//! GPS route finder — a faithful port of the Qt `GPSPathfinder`.
//!
//! A weighted, cross-game directed graph over entrance nodes:
//!   * intra-scene **walk** edges carry measured travel times (data
//!     `*_ENTRANCE_COSTS`), a high default for unmeasured pairs, and the special
//!     warp-song / owl-menu costs;
//!   * **portal** edges (cost 0) come exclusively from the player's discovered
//!     `out_links`, so the route reflects the seed's real randomization and can
//!     cross OoT <-> MM;
//!   * optional cross-game warp access (gated by the ROM cross-warp settings).
//!
//! Yen's K-shortest-paths yields up to `max` cheapest scene routes; each is then
//! collapsed into scene stations with a per-hop time and the exit taken, plus a
//! walking-only fallback so a non-warp route always appears when one exists.

use std::cmp::Reverse;
use std::collections::{BinaryHeap, HashMap, HashSet};
use std::hash::{Hash, Hasher};

use crate::data::{self, scenes as sc, EntranceType};
use crate::scene::Game;

// ── Cost constants (Entrances.cpp) ────────────────────────────────────────────
const DEFAULT_WALK: u32 = 999; // unmeasured intra-scene pair (Dijkstra avoids it)
const WARP_USE: u32 = 10; // ocarina out + song input, into a warp choice node
const WARP_MENU: u32 = 1; // picking a destination inside the song / owl menu
const CROSS_WARP: u32 = 10; // playing the OTHER game's warp from any entrance

const VSRC: u64 = u64::MAX - 1;
const VSINK: u64 = u64::MAX;

// ── Node encoding (game in bit 32, entrance id in the low 32 bits) ────────────
fn node(game: Game, id: u32) -> u64 {
    ((game.idx() as u64) << 32) | id as u64
}
fn node_game(n: u64) -> Game {
    if (n >> 32) & 1 == 1 { Game::Mm } else { Game::Oot }
}
fn node_id(n: u64) -> u32 {
    (n & 0xFFFF_FFFF) as u32
}

/// The synthetic warp scene of a game (OoT warp songs / MM owl statues).
fn warp_scene(game: Game) -> u16 {
    match game {
        Game::Oot => sc::OOT_SONGS,
        Game::Mm => sc::MM_OWLS,
    }
}

/// The buckets holding non-physical nodes (warp menus, generic grottos, cutscene
/// maps): "walking" between two of their entrances is meaningless. Mirrors
/// `IsSyntheticScene`.
fn is_synthetic(game: Game, scene: u16) -> bool {
    match game {
        Game::Oot => scene == sc::OOT_GROTTOS || scene == sc::OOT_CUTSCENE_MAP || scene == sc::OOT_SONGS,
        Game::Mm => {
            scene == sc::MM_GROTTOS
                || scene == sc::MM_CUTSCENE_MAP
                || scene == sc::MM_EXTRA
                || scene == sc::MM_OWLS
        }
    }
}

/// `ContributesLayoutWalks`: an entry's layout costs are real walks unless it is
/// a `None`-type entry sitting inside a synthetic scene.
fn contributes(game: Game, v: &data::EntranceDef) -> bool {
    v.type_ != EntranceType::None || !is_synthetic(game, v.from_scene)
}

// ── Graph ─────────────────────────────────────────────────────────────────────
#[derive(Clone, Copy)]
struct Edge {
    to: u64,
    cost: u32,
    portal: bool,
}

#[derive(Default)]
struct Graph {
    adj: HashMap<u64, Vec<Edge>>,
    scene_of: HashMap<u64, u16>,
}

impl Graph {
    fn add(&mut self, from: u64, to: u64, cost: u32, portal: bool) {
        self.adj.entry(from).or_default().push(Edge { to, cost, portal });
    }
}

/// Build the walk graph of both games: the intra-scene walk edges (with measured
/// / default / warp / menu costs) plus optional cross-game warp access. Portals
/// and virtual endpoints are attached per query afterwards.
fn build_walk_graph(cross_oot: bool, cross_mm: bool) -> Graph {
    let mut g = Graph::default();
    // A single cost per directed (from, to) walk, applied in the Qt precedence
    // (default -> measured -> warp-use -> menu, last write wins).
    let mut walks: HashMap<(u64, u64), u32> = HashMap::new();

    for game in [Game::Oot, Game::Mm] {
        let ents = game.entrances();

        // Entrances physically living in each scene, and the sources (from-ids)
        // that anchor a walk. Registration also records each node's scene.
        let mut in_scene: HashMap<u16, HashSet<u32>> = HashMap::new();
        let mut froms: HashMap<u16, HashSet<u32>> = HashMap::new();
        for v in ents {
            if !contributes(game, v) {
                continue;
            }
            g.scene_of.insert(node(game, v.from_id), v.from_scene);
            g.scene_of.insert(node(game, v.to_id), v.to_scene);
            in_scene.entry(v.from_scene).or_default().insert(v.from_id);
            in_scene.entry(v.to_scene).or_default().insert(v.to_id);
            froms.entry(v.from_scene).or_default().insert(v.from_id);
        }

        // 1) Default fill: every source reaches every other entrance in its scene.
        for (scene, sources) in &froms {
            let targets = &in_scene[scene];
            for &f in sources {
                for &t in targets {
                    if t != f {
                        walks.insert((node(game, f), node(game, t)), DEFAULT_WALK);
                    }
                }
            }
        }

        // 2) Measured overrides. A measurement's `to` is an entrance key (to-id);
        //    the physical walk target is that entry's from-id (`InitializeMeasuredCosts`).
        let costs = match game {
            Game::Oot => data::OOT_ENTRANCE_COSTS,
            Game::Mm => data::MM_ENTRANCE_COSTS,
        };
        for &(scene, from, to, cost) in costs {
            if !froms.get(&scene).is_some_and(|s| s.contains(&from)) {
                continue;
            }
            let Some(m) = crate::entrance::lookup(game, to) else { continue };
            walks.insert((node(game, from), node(game, m.from_id)), cost);
        }

        // 3) Warp-song use: from any entrance outside the warp scene, reach every
        //    warp choice node at WARP_USE.
        let ws = warp_scene(game);
        let choices: Vec<u32> = ents
            .iter()
            .filter(|v| v.type_ == EntranceType::One_Way_Out && v.to_scene == ws)
            .map(|v| v.to_id)
            .collect();
        for (&scene, sources) in &froms {
            if scene == ws {
                continue;
            }
            for &f in sources {
                for &c in &choices {
                    if c != f {
                        walks.insert((node(game, f), node(game, c)), WARP_USE);
                    }
                }
            }
        }

        // 4) Menu costs: walking between two nodes inside the warp scene is just a
        //    menu pick, not a real walk.
        if let (Some(sources), Some(targets)) = (froms.get(&ws), in_scene.get(&ws)) {
            for &f in sources {
                for &t in targets {
                    if t != f {
                        walks.insert((node(game, f), node(game, t)), WARP_MENU);
                    }
                }
            }
        }
    }

    // Cross-game warp: while the setting is on, any entrance can play the OTHER
    // game's warp, reaching that game's choice nodes at CROSS_WARP.
    let choices_of = |game: Game| -> Vec<u32> {
        let ws = warp_scene(game);
        game.entrances()
            .iter()
            .filter(|v| v.type_ == EntranceType::One_Way_Out && v.to_scene == ws)
            .map(|v| v.to_id)
            .collect()
    };
    if cross_oot {
        let mm = choices_of(Game::Mm);
        for v in Game::Oot.entrances() {
            if v.type_ == EntranceType::None || v.from_scene == warp_scene(Game::Oot) {
                continue;
            }
            for &c in &mm {
                walks.insert((node(Game::Oot, v.from_id), node(Game::Mm, c)), CROSS_WARP);
            }
        }
    }
    if cross_mm {
        let oot = choices_of(Game::Oot);
        for v in Game::Mm.entrances() {
            if v.type_ == EntranceType::None || v.from_scene == warp_scene(Game::Mm) {
                continue;
            }
            for &c in &oot {
                walks.insert((node(Game::Mm, v.from_id), node(Game::Oot, c)), CROSS_WARP);
            }
        }
    }

    for ((from, to), cost) in walks {
        g.add(from, to, cost, false);
    }
    g
}

/// Attach the player's discovered portals (entrance -> destination entrance,
/// cost 0), which may cross OoT <-> MM.
fn add_portals(g: &mut Graph, out_links: &HashMap<(Game, u32), (Game, u32)>) {
    for (&(fg, fid), &(tg, tid)) in out_links {
        g.add(node(fg, fid), node(tg, tid), 0, true);
    }
}

/// Wire the virtual source to every start-scene entrance and every end-scene
/// entrance to the virtual sink (pinned to a single entrance when given).
fn attach_endpoints(
    g: &mut Graph,
    start: (Game, u16),
    end: (Game, u16),
    from_ent: Option<u32>,
    to_ent: Option<u32>,
) -> bool {
    let mut has_start = false;
    let mut has_end = false;
    // Snapshot the nodes first (we mutate `g.adj` while reading `g.scene_of`).
    let nodes: Vec<(u64, u16)> = g.scene_of.iter().map(|(&n, &s)| (n, s)).collect();
    for (n, scene) in nodes {
        if node_game(n) == start.0 && scene == start.1 && from_ent.is_none_or(|e| node_id(n) == e) {
            g.add(VSRC, n, 0, false);
            has_start = true;
        }
        if node_game(n) == end.0 && scene == end.1 && to_ent.is_none_or(|e| node_id(n) == e) {
            g.add(n, VSINK, 0, false);
            has_end = true;
        }
    }
    has_start && has_end
}

// ── Dijkstra + Yen's K-shortest ───────────────────────────────────────────────
/// Shortest path source -> sink (as a node sequence), ignoring `forbid_edges`
/// and `forbid_nodes` (the source is never forbidden). None when the sink is
/// unreachable.
fn dijkstra(
    g: &Graph,
    source: u64,
    sink: u64,
    forbid_edges: &HashSet<(u64, u64)>,
    forbid_nodes: &HashSet<u64>,
) -> Option<Vec<u64>> {
    let mut dist: HashMap<u64, u32> = HashMap::from([(source, 0)]);
    let mut prev: HashMap<u64, u64> = HashMap::new();
    let mut heap: BinaryHeap<Reverse<(u32, u64)>> = BinaryHeap::from([Reverse((0, source))]);

    while let Some(Reverse((d, u))) = heap.pop() {
        if u == sink {
            break;
        }
        if dist.get(&u).is_some_and(|&best| d > best) {
            continue;
        }
        let Some(edges) = g.adj.get(&u) else { continue };
        for e in edges {
            if (e.to != source && forbid_nodes.contains(&e.to)) || forbid_edges.contains(&(u, e.to)) {
                continue;
            }
            let nd = d + e.cost;
            if dist.get(&e.to).is_none_or(|&best| nd < best) {
                dist.insert(e.to, nd);
                prev.insert(e.to, u);
                heap.push(Reverse((nd, e.to)));
            }
        }
    }

    dist.get(&sink)?; // unreachable -> None
    let mut path = vec![sink];
    let mut cur = sink;
    while cur != source {
        cur = *prev.get(&cur)?;
        path.push(cur);
    }
    path.reverse();
    Some(path)
}

/// Recompute a node sequence's cost by walking the graph's min edges.
fn path_cost(g: &Graph, path: &[u64]) -> Option<u32> {
    let mut total = 0;
    for w in path.windows(2) {
        let best = g.adj.get(&w[0])?.iter().filter(|e| e.to == w[1]).map(|e| e.cost).min()?;
        total += best;
    }
    Some(total)
}

/// Yen's K-shortest loopless paths, ascending by total cost. Each path is the
/// full node sequence including source and sink.
fn yen(g: &Graph, source: u64, sink: u64, k: usize) -> Vec<Vec<u64>> {
    let mut a: Vec<Vec<u64>> = Vec::new();
    if k == 0 {
        return a;
    }
    let Some(first) = dijkstra(g, source, sink, &HashSet::new(), &HashSet::new()) else {
        return a;
    };
    a.push(first);

    let mut candidates: Vec<(u32, Vec<u64>)> = Vec::new();
    let mut seen: HashSet<Vec<u64>> = HashSet::from_iter(a.iter().cloned());

    for _ in 1..k {
        let prev = a.last().unwrap().clone();
        for i in 0..prev.len().saturating_sub(1) {
            let spur = prev[i];
            let root = &prev[..=i];

            let mut forbid_edges: HashSet<(u64, u64)> = HashSet::new();
            let mut forbid_nodes: HashSet<u64> = HashSet::new();
            for p in &a {
                if p.len() > i + 1 && p[..=i] == *root {
                    forbid_edges.insert((p[i], p[i + 1]));
                }
            }
            for &n in &root[..i] {
                forbid_nodes.insert(n);
            }

            let Some(spur_path) = dijkstra(g, spur, sink, &forbid_edges, &forbid_nodes) else {
                continue;
            };
            let mut total: Vec<u64> = root[..i].to_vec();
            total.extend(spur_path);
            if seen.contains(&total) {
                continue;
            }
            let Some(tc) = path_cost(g, &total) else { continue };
            candidates.push((tc, total.clone()));
            seen.insert(total);
        }
        if candidates.is_empty() {
            break;
        }
        let best = (0..candidates.len()).min_by_key(|&i| candidates[i].0).unwrap();
        a.push(candidates.remove(best).1);
    }
    a
}

// ── Public result types ───────────────────────────────────────────────────────
/// One scene station of a route: the scene reached, the exit taken to leave it
/// (`via`, empty on the final station) and the seconds that hop costs.
pub struct GpsStep {
    pub game: Game,
    pub scene: u16,
    pub via: String,
    pub cost: u32,
}

/// A full route: ordered scene stations and the accumulated travel time.
pub struct GpsRoute {
    pub steps: Vec<GpsStep>,
    pub total: u32,
}

impl GpsRoute {
    /// Number of scene-to-scene transitions (stations minus one).
    pub fn transitions(&self) -> usize {
        self.steps.len().saturating_sub(1)
    }
}

pub enum GpsStatus {
    Ok,
    SameScene,
    NoRoute,
}

pub struct GpsResult {
    pub status: GpsStatus,
    pub routes: Vec<GpsRoute>,
}

/// The inputs a routing result depends on, so the UI can cache a result and only
/// recompute when something actually changed.
#[derive(Clone, PartialEq)]
pub struct GpsKey {
    pub from: (Game, u16),
    pub to: (Game, u16),
    pub from_ent: Option<u32>,
    pub to_ent: Option<u32>,
    pub cross_oot: bool,
    pub cross_mm: bool,
    pub links: u64,
}

/// A cheap order-independent fingerprint of the discovered portals, so a changed
/// link map invalidates the cached route without tracking a version counter.
pub fn links_hash(out_links: &HashMap<(Game, u32), (Game, u32)>) -> u64 {
    let mut acc = out_links.len() as u64;
    for (&(fg, fid), &(tg, tid)) in out_links {
        let mut h = std::collections::hash_map::DefaultHasher::new();
        (fg.idx(), fid, tg.idx(), tid).hash(&mut h);
        acc ^= h.finish();
    }
    acc
}

/// Whether a scene is a warp menu (its exit reads "Warp Song" / "Warp Owl").
fn is_warp_scene(game: Game, scene: u16) -> bool {
    scene == warp_scene(game)
}

/// A route is a warp detour when a real scene sits between the start and its
/// first warp station (index >= 2) — warping from the start reaches the same
/// place for less, so the detour is always dominated.
fn is_warp_detour(r: &GpsRoute) -> bool {
    r.steps
        .iter()
        .position(|s| is_warp_scene(s.game, s.scene))
        .is_some_and(|i| i >= 2)
}

/// The name of the other side of an entrance node relative to `current`, picking
/// the side opposite to it (`ExitDestinationName`).
fn exit_destination_name(n: u64, current: u16) -> &'static str {
    match crate::entrance::lookup(node_game(n), node_id(n)) {
        Some(m) if m.from_scene == current => m.to_name,
        Some(m) if m.to_scene == current => m.from_name,
        Some(m) => m.to_name,
        None => "",
    }
}

/// Collapse a raw node sequence into scene stations, accumulating each hop's
/// walk cost and labelling the exit taken (`ToScenePath`).
fn to_scene_path(g: &Graph, raw: &[u64]) -> GpsRoute {
    let mut out = GpsRoute { steps: Vec::new(), total: 0 };
    let (mut last_scene, mut last_game): (Option<u16>, Option<Game>) = (None, None);
    let mut hop = 0u32;
    let mut prev: Option<u64> = None;

    // The (non-portal) walk cost of the edge prev -> n, if any.
    let walk_cost = |prev: u64, n: u64| -> u32 {
        g.adj
            .get(&prev)
            .and_then(|es| es.iter().find(|e| e.to == n && !e.portal).map(|e| e.cost))
            .unwrap_or(0)
    };

    for &n in raw {
        if n == VSRC || n == VSINK {
            continue;
        }
        let Some(&scene) = g.scene_of.get(&n) else { continue };
        let game = node_game(n);

        if Some(scene) != last_scene || Some(game) != last_game {
            if let (Some(prev), Some(ls), Some(lg), Some(step)) =
                (prev, last_scene, last_game, out.steps.last_mut())
            {
                hop += walk_cost(prev, n);
                step.cost = hop;
                let via = if is_warp_scene(lg, ls) {
                    crate::entrance::lookup(node_game(prev), node_id(prev))
                        .map(|m| m.from_name)
                        .unwrap_or("")
                } else if is_warp_scene(game, scene) {
                    if game == Game::Oot { "Warp Song" } else { "Warp Owl" }
                } else {
                    exit_destination_name(prev, ls)
                };
                step.via = if via.is_empty() { "Walk".to_string() } else { via.to_string() };
                out.total += hop;
            }
            // The UI resolves + translates the scene name itself from (game, scene).
            out.steps.push(GpsStep { game, scene, via: String::new(), cost: 0 });
            last_scene = Some(scene);
            last_game = Some(game);
            hop = 0;
        } else if let Some(prev) = prev {
            hop += walk_cost(prev, n);
        }
        prev = Some(n);
    }
    out
}

/// Find up to `max` cheapest scene routes from `start` to `end`, over the walk
/// graph plus the player's discovered `out_links`. `from_ent` / `to_ent` pin a
/// specific departure / arrival entrance when set.
#[allow(clippy::too_many_arguments)]
pub fn find_routes(
    start: (Game, u16),
    end: (Game, u16),
    from_ent: Option<u32>,
    to_ent: Option<u32>,
    max: usize,
    cross_oot: bool,
    cross_mm: bool,
    out_links: &HashMap<(Game, u32), (Game, u32)>,
) -> GpsResult {
    // Same scene with no distinct pinned entrances is a no-op.
    if start == end && (from_ent.is_none() || to_ent.is_none() || from_ent == to_ent) {
        return GpsResult { status: GpsStatus::SameScene, routes: Vec::new() };
    }

    let mut g = build_walk_graph(cross_oot, cross_mm);
    add_portals(&mut g, out_links);
    if !attach_endpoints(&mut g, start, end, from_ent, to_ent) {
        return GpsResult { status: GpsStatus::NoRoute, routes: Vec::new() };
    }

    let mut raw = yen(&g, VSRC, VSINK, max);

    // Walking-only fallback: guarantee at least one route that avoids the warp
    // menus, even when Yen's naturally prefers warp variants.
    let warp_nodes: HashSet<u64> = g
        .scene_of
        .iter()
        .filter(|(&n, &s)| is_warp_scene(node_game(n), s))
        .map(|(&n, _)| n)
        .collect();
    if !warp_nodes.is_empty() {
        if let Some(p) = dijkstra(&g, VSRC, VSINK, &HashSet::new(), &warp_nodes) {
            raw.push(p);
        }
    }

    let mut routes: Vec<GpsRoute> = raw
        .into_iter()
        .map(|r| to_scene_path(&g, &r))
        .filter(|p| p.steps.len() >= 2)
        .collect();
    if routes.is_empty() {
        return GpsResult { status: GpsStatus::NoRoute, routes: Vec::new() };
    }

    // Cheapest first, so the dedup below keeps the best variant of each route.
    routes.sort_by_key(|r| r.total);

    // Collapse routes that show the *same* scene stations and exits: two node-level
    // paths often differ only by a 1s warp-menu hop yet render identically, which
    // looks like a bogus near-duplicate. Key on (game, scene, via) only — never the
    // per-hop cost — so those merge, keeping the cheapest (first after the sort).
    let mut seen: HashSet<Vec<(u8, u16, String)>> = HashSet::new();
    routes.retain(|r| {
        seen.insert(r.steps.iter().map(|s| (s.game.idx() as u8, s.scene, s.via.clone())).collect())
    });

    // Warp-detour pruning: a warp song / owl is playable from *any* entrance, so
    // a route that walks to some intermediate scene and only then warps is always
    // dominated by warping straight from the start (which reaches the same warp
    // choice, cheaper). Drop such routes, as long as a non-detour route survives —
    // never leave the user with nothing.
    if routes.iter().any(|r| !is_warp_detour(r)) {
        routes.retain(|r| !is_warp_detour(r));
    }

    routes.truncate(max);

    GpsResult { status: GpsStatus::Ok, routes }
}

#[cfg(test)]
mod tests {
    use super::*;

    /// A tiny hand-built graph to exercise Dijkstra / Yen without the real data.
    fn toy() -> Graph {
        let mut g = Graph::default();
        // src -> a -> b -> sink (cost 2) and src -> a -> c -> sink (cost 3).
        g.add(VSRC, 1, 0, false);
        g.add(1, 2, 1, false);
        g.add(2, VSINK, 1, false);
        g.add(1, 3, 1, false);
        g.add(3, VSINK, 2, false);
        g
    }

    #[test]
    fn dijkstra_finds_cheapest() {
        let g = toy();
        let path = dijkstra(&g, VSRC, VSINK, &HashSet::new(), &HashSet::new()).unwrap();
        assert_eq!(path, vec![VSRC, 1, 2, VSINK]);
        assert_eq!(path_cost(&g, &path), Some(2));
    }

    #[test]
    fn yen_returns_both_paths_in_order() {
        let g = toy();
        let paths = yen(&g, VSRC, VSINK, 3);
        assert_eq!(paths.len(), 2, "only two loopless paths exist");
        assert_eq!(path_cost(&g, &paths[0]), Some(2));
        assert_eq!(path_cost(&g, &paths[1]), Some(3));
    }

    fn step(game: Game, scene: u16) -> GpsStep {
        GpsStep { game, scene, via: String::new(), cost: 0 }
    }

    #[test]
    fn warp_detour_is_recognized() {
        // start -> some scene -> Warp Songs -> ... : a detour (warp at index 2).
        let detour = GpsRoute {
            steps: vec![
                step(Game::Oot, sc::OOT_DEKU_TREE),
                step(Game::Oot, sc::OOT_KOKIRI_FOREST),
                step(Game::Oot, sc::OOT_SONGS),
                step(Game::Oot, sc::OOT_TEMPLE_OF_TIME),
            ],
            total: 30,
        };
        // start -> Warp Songs -> ... : direct, not a detour (warp at index 1).
        let direct = GpsRoute {
            steps: vec![
                step(Game::Oot, sc::OOT_DEKU_TREE),
                step(Game::Oot, sc::OOT_SONGS),
                step(Game::Oot, sc::OOT_TEMPLE_OF_TIME),
            ],
            total: 20,
        };
        // A pure walking route never warps: not a detour.
        let walk = GpsRoute {
            steps: vec![
                step(Game::Oot, sc::OOT_DEKU_TREE),
                step(Game::Oot, sc::OOT_KOKIRI_FOREST),
            ],
            total: 5,
        };
        assert!(is_warp_detour(&detour));
        assert!(!is_warp_detour(&direct));
        assert!(!is_warp_detour(&walk));
    }

    #[test]
    fn node_encoding_roundtrips() {
        let n = node(Game::Mm, 0x1234);
        assert_eq!(node_game(n), Game::Mm);
        assert_eq!(node_id(n), 0x1234);
        let n = node(Game::Oot, 0x209);
        assert_eq!(node_game(n), Game::Oot);
        assert_eq!(node_id(n), 0x209);
    }

    #[test]
    fn real_graph_builds_and_has_edges() {
        let g = build_walk_graph(false, false);
        assert!(!g.adj.is_empty());
        assert!(!g.scene_of.is_empty());
    }

    /// A single discovered portal between two real OoT scenes yields a two-station
    /// route from the departure scene to the arrival scene.
    #[test]
    fn portal_enables_a_cross_scene_route() {
        let ents = Game::Oot.entrances();
        let a = ents
            .iter()
            .find(|v| v.type_ != EntranceType::None && !is_synthetic(Game::Oot, v.from_scene))
            .expect("an anchored OoT entrance");
        let b = ents
            .iter()
            .find(|v| {
                v.type_ != EntranceType::None
                    && !is_synthetic(Game::Oot, v.from_scene)
                    && v.from_scene != a.from_scene
            })
            .expect("a second scene");

        let start = (Game::Oot, a.from_scene);
        let end = (Game::Oot, b.from_scene);
        let links = HashMap::from([((Game::Oot, a.from_id), (Game::Oot, b.from_id))]);

        let res = find_routes(start, end, None, None, 3, false, false, &links);
        assert!(matches!(res.status, GpsStatus::Ok), "portal should make a route");
        let r = &res.routes[0];
        assert!(r.steps.len() >= 2);
        assert_eq!((r.steps.first().unwrap().game, r.steps.first().unwrap().scene), start);
        assert_eq!((r.steps.last().unwrap().game, r.steps.last().unwrap().scene), end);
    }
}
