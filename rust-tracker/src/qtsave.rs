//! Import of the **Qt tracker's binary `.trck` saves** (the C++ reference app).
//!
//! The Rust tracker writes a text save (`TRACKER_SAVE`), but the user's history
//! is full of Qt binary saves. Those are little-endian struct dumps whose layout
//! is versioned by a leading `u32` (`TrackerVersion`): 0 = V1_0, 1 = V2_0,
//! 2 = V2_1. A fourth on-disk value (3) exists in the wild from a newer local Qt
//! build; per the maintainer it is not worth decoding exactly, so we parse it as
//! V2_1 (best effort) and fall back gracefully if it desyncs.
//!
//! Layout (V2_1), all integers little-endian:
//! ```text
//! u32  version
//! u64  paramCount              (settings block — only a count is persisted)
//! u32  numWorlds
//!   per world, per game (OoT then MM):
//!     u32 sceneCount
//!       per scene: u32 sceneID, u64 objCount
//!         per object: u32 objID, u32 status, u32 itemID, u32 targetWorld
//! entrances (OoT then MM):
//!   u64 sceneCount
//!     per scene: u32 sceneID, u64 linkCount
//!       per link: u32 entranceID, u32 outLink, u8 outGame, u32 inCount,
//!                 inCount * (u32 srcEntrance, u8 srcGame)
//! ```
//! V2_0 is identical except each world/game block has **no** `sceneCount` header
//! (one block per scene slot, in id order), so we read `maxSceneId + 1` blocks.

use crate::data;
use crate::scene::Game;

const NO_LINK: u32 = u32::MAX;
/// `ObjectState`: Hidden = 0, Collected = 1, Forced = 2 (Objects.h).
pub const ST_COLLECTED: u32 = 1;
pub const ST_FORCED: u32 = 2;
const MAX_COUNT: u64 = 200_000; // sanity cap: any larger count means a desync

/// One saved object placement, as read from the file (ids, not resolved yet).
pub struct QtObj {
    pub game: Game,
    pub scene: u32,
    pub obj_id: u32,
    pub status: u32,
    pub item_id: u32,
}

/// A parsed Qt save: per-world object records, the discovered entrance graph
/// (out-links, per-target in-links and the set of entrances that count as
/// "visited"), and whether the parse had to stop early (older/odd formats are
/// read best-effort).
pub struct QtSave {
    pub version: u32,
    pub worlds: Vec<Vec<QtObj>>,
    pub out_links: Vec<((Game, u32), (Game, u32))>,
    pub in_links: Vec<((Game, u32), Vec<(Game, u32)>)>,
    /// Every entrance id touched by a discovered link (either end), so the
    /// found/total counters light up on load exactly as during live tracking.
    pub visited: Vec<(Game, u32)>,
    pub partial: bool,
}

/// A little-endian cursor over the file bytes; every read is bounds-checked and
/// yields `None` at end-of-buffer so a truncated/odd file can never panic.
struct Rd<'a> {
    d: &'a [u8],
    o: usize,
}
impl Rd<'_> {
    fn u8(&mut self) -> Option<u8> {
        let v = *self.d.get(self.o)?;
        self.o += 1;
        Some(v)
    }
    fn u32(&mut self) -> Option<u32> {
        let b = self.d.get(self.o..self.o + 4)?;
        self.o += 4;
        Some(u32::from_le_bytes(b.try_into().unwrap()))
    }
    fn u64(&mut self) -> Option<u64> {
        let b = self.d.get(self.o..self.o + 8)?;
        self.o += 8;
        Some(u64::from_le_bytes(b.try_into().unwrap()))
    }
}

fn game_of(v: u8) -> Option<Game> {
    match v {
        0 => Some(Game::Oot),
        1 => Some(Game::Mm),
        _ => None, // NO_GAME (2) or garbage
    }
}

/// Parse a Qt binary `.trck`. Returns `None` only for formats we don't decode at
/// all (V1_0 and truly foreign data); V2_0/V2_1/v3 return a (possibly partial)
/// save.
pub fn parse(bytes: &[u8]) -> Option<QtSave> {
    let mut r = Rd { d: bytes, o: 0 };
    let version = r.u32()?;
    match version {
        1 => parse_worlds(&mut r, version, WorldFraming::Lockstep),
        2 | 3 => parse_worlds(&mut r, version, WorldFraming::Counted),
        _ => None, // V1_0 (0) uses a different pre-multiworld layout — unsupported.
    }
}

enum WorldFraming {
    /// V2_1: each game block starts with a `u32` scene count.
    Counted,
    /// V2_0: no count — one block per scene slot (`maxSceneId + 1` of them).
    Lockstep,
}

fn parse_worlds(r: &mut Rd, version: u32, framing: WorldFraming) -> Option<QtSave> {
    let _param_count = r.u64()?; // settings block: only a count is stored
    let num_worlds = r.u32()?.clamp(1, 64) as usize;

    let slots = |g: Game| g.scenes().iter().map(|s| s.id as u32).max().unwrap_or(0) + 1;

    let mut worlds = Vec::new();
    let mut partial = false;
    'outer: for _ in 0..num_worlds {
        let mut objs = Vec::new();
        for game in [Game::Oot, Game::Mm] {
            let scene_count = match framing {
                WorldFraming::Counted => match r.u32() {
                    Some(c) if (c as u64) <= MAX_COUNT => c,
                    _ => {
                        partial = true;
                        break 'outer;
                    }
                },
                WorldFraming::Lockstep => slots(game),
            };
            for _ in 0..scene_count {
                let (Some(scene), Some(nobj)) = (r.u32(), r.u64()) else {
                    partial = true;
                    break 'outer;
                };
                if nobj > MAX_COUNT {
                    partial = true;
                    break 'outer;
                }
                for _ in 0..nobj {
                    let (Some(obj_id), Some(status), Some(item_id), Some(_tw)) =
                        (r.u32(), r.u32(), r.u32(), r.u32())
                    else {
                        partial = true;
                        break 'outer;
                    };
                    objs.push(QtObj { game, scene, obj_id, status, item_id });
                }
            }
        }
        worlds.push(objs);
    }

    // Entrances only make sense if the world section stayed aligned.
    let ent = if partial { Entrances::default() } else { parse_entrances(r).unwrap_or_default() };
    Some(QtSave {
        version,
        worlds,
        out_links: ent.out_links,
        in_links: ent.in_links,
        visited: ent.visited,
        partial,
    })
}

#[derive(Default)]
struct Entrances {
    out_links: Vec<((Game, u32), (Game, u32))>,
    in_links: Vec<((Game, u32), Vec<(Game, u32)>)>,
    visited: Vec<(Game, u32)>,
}

/// Read the OoT-then-MM entrance sections, reconstructing the discovered graph:
/// each entrance's OutLink (portal), its InLinks (sources leading in) and the set
/// of entrances that any discovered link touches (which the counters treat as
/// "visited", matching live tracking's out+in marking).
fn parse_entrances(r: &mut Rd) -> Option<Entrances> {
    let mut e = Entrances::default();
    for game in [Game::Oot, Game::Mm] {
        let num_scenes = r.u64()?;
        if num_scenes > MAX_COUNT {
            return Some(e);
        }
        for _ in 0..num_scenes {
            let _scene_id = r.u32()?;
            let num_links = r.u64()?;
            if num_links > MAX_COUNT {
                return Some(e);
            }
            for _ in 0..num_links {
                let ent_id = r.u32()?;
                let out_link = r.u32()?;
                let out_game = r.u8()?;
                let num_in = r.u32()?;
                if num_in as u64 > MAX_COUNT {
                    return Some(e);
                }
                // Read every InLink source, dropping the seeded "?" placeholder
                // ({UINT32_MAX, NO_GAME}) that marks an undiscovered source.
                let mut sources = Vec::new();
                for _ in 0..num_in {
                    let src_id = r.u32()?;
                    let src_game = r.u8()?;
                    if src_id != NO_LINK {
                        if let Some(sg) = game_of(src_game) {
                            sources.push((sg, src_id));
                            e.visited.push((sg, src_id)); // source was left through
                            e.visited.push((game, ent_id)); // this entrance was reached
                        }
                    }
                }
                if !sources.is_empty() {
                    e.in_links.push(((game, ent_id), sources));
                }
                if out_link != NO_LINK {
                    if let Some(og) = game_of(out_game) {
                        e.out_links.push(((game, ent_id), (og, out_link)));
                        e.visited.push((game, ent_id)); // left through this entrance
                        e.visited.push((og, out_link)); // arrived at its destination
                    }
                }
            }
        }
    }
    Some(e)
}

/// Resolve a Qt item id to its display name (dev numbering, shared with the Rust
/// `ITEMS` table). Id 0 means "no item".
pub fn item_name(id: u32) -> Option<&'static str> {
    if id == 0 {
        return None;
    }
    data::ITEMS.iter().find(|it| it.id == id).map(|it| it.name)
}

#[cfg(test)]
mod tests {
    use super::*;

    /// Assemble a minimal V2_1 buffer by hand and check the parser extracts it.
    fn v21_bytes() -> Vec<u8> {
        let mut b = Vec::new();
        b.extend_from_slice(&2u32.to_le_bytes()); // version V2_1
        b.extend_from_slice(&0u64.to_le_bytes()); // param count
        b.extend_from_slice(&1u32.to_le_bytes()); // 1 world
                                                  // OoT: 1 scene, 2 objects
        b.extend_from_slice(&1u32.to_le_bytes()); // sceneCount
        b.extend_from_slice(&0x10u32.to_le_bytes()); // sceneID
        b.extend_from_slice(&2u64.to_le_bytes()); // objCount
        for (obj, status, item) in [(0xAAu32, ST_COLLECTED, 0u32), (0xBB, ST_FORCED, 0x4)] {
            b.extend_from_slice(&obj.to_le_bytes());
            b.extend_from_slice(&status.to_le_bytes());
            b.extend_from_slice(&item.to_le_bytes());
            b.extend_from_slice(&0u32.to_le_bytes()); // targetWorld
        }
        // MM: 0 scenes
        b.extend_from_slice(&0u32.to_le_bytes());
        // Entrances: OoT one scene with one link (outLink 0x77 -> MM), MM none.
        b.extend_from_slice(&1u64.to_le_bytes()); // OoT scene count
        b.extend_from_slice(&0x10u32.to_le_bytes()); // sceneID
        b.extend_from_slice(&1u64.to_le_bytes()); // link count
        b.extend_from_slice(&0x55u32.to_le_bytes()); // entranceID
        b.extend_from_slice(&0x77u32.to_le_bytes()); // outLink
        b.push(1u8); // outGame = MM
        b.extend_from_slice(&0u32.to_le_bytes()); // inCount
        b.extend_from_slice(&0u64.to_le_bytes()); // MM entrance scene count
        b
    }

    #[test]
    fn parses_v21_objects_and_links() {
        let save = parse(&v21_bytes()).expect("v21 parses");
        assert!(!save.partial);
        assert_eq!(save.version, 2);
        assert_eq!(save.worlds.len(), 1);
        let w = &save.worlds[0];
        assert_eq!(w.len(), 2);
        assert_eq!(w[0].obj_id, 0xAA);
        assert_eq!(w[0].status, ST_COLLECTED);
        assert_eq!(w[1].status, ST_FORCED);
        assert_eq!(w[1].item_id, 0x4);
        assert_eq!(save.out_links, vec![((Game::Oot, 0x55), (Game::Mm, 0x77))]);
        // The out-link's two endpoints both count as visited.
        assert!(save.visited.contains(&(Game::Oot, 0x55)));
        assert!(save.visited.contains(&(Game::Mm, 0x77)));
    }

    #[test]
    fn item_name_resolves_and_rejects_zero() {
        assert!(item_name(0).is_none());
        assert_eq!(item_name(0x4), Some("Fairy Bow (OoT)"));
    }

    #[test]
    fn foreign_data_is_rejected() {
        // "TRAC" (a Rust text save) decodes to a huge version -> None.
        assert!(parse(b"TRACKER_SAVE 4\n").is_none());
        assert!(parse(&[]).is_none());
    }

    /// Validate the reverse-engineered layout against the maintainer's real Qt
    /// saves: parse the newest V2_1 (or V2_0) `.trck` under `../AutoSave` and
    /// confirm it consumes cleanly and its collected objects resolve to the
    /// current data. Skips silently when the folder isn't present (CI, clones).
    #[test]
    fn real_qt_save_parses_and_resolves() {
        use std::collections::HashMap;
        let dir = format!("{}/../AutoSave", env!("CARGO_MANIFEST_DIR"));
        let Ok(entries) = std::fs::read_dir(&dir) else {
            eprintln!("skip: no {dir}");
            return;
        };
        // Prefer a V2_1 (version 2) file; fall back to V2_0 (version 1).
        let mut best: Option<(u32, std::path::PathBuf)> = None;
        for e in entries.flatten() {
            let p = e.path();
            if p.extension().and_then(|s| s.to_str()) != Some("trck") {
                continue;
            }
            let Ok(bytes) = std::fs::read(&p) else { continue };
            if bytes.len() < 4 {
                continue;
            }
            let v = u32::from_le_bytes([bytes[0], bytes[1], bytes[2], bytes[3]]);
            if v == 2 && !matches!(best, Some((2, _))) {
                best = Some((2, p));
            } else if v == 1 && best.is_none() {
                best = Some((1, p));
            }
        }
        let Some((_, path)) = best else {
            eprintln!("skip: no V2_0/V2_1 .trck in {dir}");
            return;
        };

        let bytes = std::fs::read(&path).unwrap();
        let save = parse(&bytes).expect("real save parses");

        // King Dodongo's Lair (scene 0x12, region 0) must count its discovered
        // links, not read 0 — replicate `entrance_scene_counts` over the imported
        // graph and assert found > 0 (its boss-in / warp-out links are set here).
        {
            use std::collections::{HashMap as Map, HashSet as Set};
            let in_ct: Map<(Game, u32), usize> =
                save.in_links.iter().map(|(k, v)| (*k, v.len())).collect();
            let out_set: Set<(Game, u32)> = save.out_links.iter().map(|(k, _)| *k).collect();
            let ids = crate::entrance::scene_entrances(Game::Oot, 0x12).unwrap_or(&[]);
            let (mut found, mut total) = (0usize, 0usize);
            for &eid in ids {
                let Some(en) = crate::entrance::lookup(Game::Oot, eid) else { continue };
                let inf = in_ct.get(&(Game::Oot, eid)).copied().unwrap_or(0);
                let outs = out_set.contains(&(Game::Oot, eid)) as usize;
                match en.type_ {
                    data::EntranceType::Normal => {
                        total += inf.max(1) + 1;
                        found += inf + outs;
                    }
                    data::EntranceType::One_Way_In => {
                        total += inf.max(1);
                        found += inf;
                    }
                    data::EntranceType::One_Way_Out => {
                        total += 1;
                        found += outs;
                    }
                    data::EntranceType::None => {}
                }
            }
            eprintln!("  King Dodongo (0x12): {found}/{total}");
            assert!(found > 0, "King Dodongo's discovered links must count");
        }

        let mut lut: [HashMap<(u32, u32), usize>; 2] = [HashMap::new(), HashMap::new()];
        for game in [Game::Oot, Game::Mm] {
            for (i, o) in game.objects().iter().enumerate() {
                lut[game.idx()].insert((o.scene as u32, o.object_id), i);
            }
        }

        // Scenes present in the current data — objects in a *renumbered/removed*
        // scene (e.g. per-grotto scenes long since folded into MM_GROTTOS) are
        // data drift across builds, not a parser fault, so we score them apart.
        let mut scenes_present: [std::collections::HashSet<u32>; 2] =
            [std::collections::HashSet::new(), std::collections::HashSet::new()];
        for game in [Game::Oot, Game::Mm] {
            for o in game.objects() {
                scenes_present[game.idx()].insert(o.scene as u32);
            }
        }

        let (mut collected, mut resolvable, mut resolved, mut drift, mut items_ok) =
            (0u32, 0u32, 0u32, 0u32, 0u32);
        for w in &save.worlds {
            for ob in w {
                if ob.status == ST_COLLECTED || ob.status == ST_FORCED {
                    collected += 1;
                    let hit = lut[ob.game.idx()].contains_key(&(ob.scene, ob.obj_id));
                    if hit {
                        resolved += 1;
                    }
                    if scenes_present[ob.game.idx()].contains(&ob.scene) {
                        resolvable += 1;
                    } else {
                        drift += 1; // its whole scene id is gone from current data
                    }
                }
                if item_name(ob.item_id).is_some() {
                    items_ok += 1;
                }
            }
        }
        eprintln!(
            "{}: v{} partial={} worlds={} collected={collected} resolved={resolved} \
             resolvable={resolvable} drift={drift} items_ok={items_ok} out_links={}",
            path.file_name().unwrap().to_string_lossy(),
            save.version,
            save.partial,
            save.worlds.len(),
            save.out_links.len(),
        );
        // Format correctness: a V2_1 parse is complete, and every collected object
        // whose scene still exists resolves by (scene, id). Scene-level drift is
        // reported but not a failure — the numbering changed between builds.
        if save.version == 2 {
            assert!(!save.partial, "V2_1 should parse fully");
        }
        assert!(resolved >= resolvable, "objects in existing scenes must all resolve");
    }
}
