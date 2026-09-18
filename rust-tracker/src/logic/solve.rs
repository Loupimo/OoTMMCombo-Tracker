//! Fixed-point reachability solver.
//!
//! Given the player's inventory / settings / tricks ([`Inputs`]), compute the set
//! of reachable check locations by exploring the region graph until nothing new
//! opens up. Reachability is tracked per `(region, age)` — the access rules gate
//! on age via `Op::Age` (`is_child` / `is_adult`), so a region can be reachable
//! as child, as adult, or both. Events are pseudo-items: reaching a region can
//! set an event, which unlocks further exits/events on a later pass (that is what
//! makes this a fixed point rather than a single graph walk).
//!
//! ## Model (M2 v1, glitchless, vanilla entrances, single world)
//! - **Seeds**: OoT `SPAWN` + `GLOBAL` and MM `GLOBAL`, at both ages. `SPAWN`'s
//!   own exits then gate the real starting age via `setting(startingAgeOot, …)` and
//!   the `TIME_TRAVEL` event, so seeding both ages is safe.
//! - **Age change** is emergent: once the `TIME_TRAVEL` event fires (Temple of
//!   Time, computed by the fixed point), `SPAWN`'s other-age exit opens.
//! - **Extra roots** (progressive entrance mode: the player's visited scenes) are
//!   seeded *inside* the fixed point and, for OoT, only at an age the player can
//!   actually reach — read straight off the `SPAWN CHILD` / `SPAWN ADULT` gate. A
//!   scene walked as a child must not light its adult-only checks (or an exit's
//!   `is_adult` branch) while adult is unreachable. MM has no age gate, so its extra
//!   roots stay both-age.
//! - **Layout**: only regions whose `layout` is active count (base dungeons /
//!   US MM by default; MQ / JP wired later).
//! - **MM region flags** (`Op::Flag`) are treated optimistically (satisfiable),
//!   and `special(x)` / renewable / license fall back to their `Inputs` / trait
//!   defaults. These are the known v1 approximations to refine later.

use std::collections::{HashMap, HashSet};

use crate::data::{self, GameLayout};

use super::eval::{eval, WorldState};

/// Age-independent, per-solve inputs. The solver supplies the evolving events and
/// the evaluation age on top of this. Implemented by the app over the tracked
/// inventory + the seed settings; the defaults keep a minimal impl short.
pub trait Inputs {
    /// Count of an item id currently owned.
    fn item_count(&self, id: u32) -> u32;
    /// The per-seed count for a `has(item, var(NAME))` requirement (index into
    /// `data::VAR_NAMES`). Default is the OoTMM default; the app inputs read the
    /// seed's setting.
    fn var_count(&self, var: u8) -> u16 {
        crate::data::VAR_DEFAULTS.get(var as usize).copied().unwrap_or(1)
    }
    /// The value a setting is fixed to (`Some(value_id)` into `SETTING_VALUES`).
    fn setting_value(&self, key: u32) -> Option<u32>;
    /// Whether setting `key` holds `val` — enum equality, or set membership for a
    /// set-valued setting. Default single-value equality; overridden by the app inputs.
    fn setting_has(&self, key: u32, val: u32) -> bool {
        self.setting_value(key) == Some(val)
    }
    /// Whether a boolean-form setting is enabled.
    fn setting_enabled(&self, key: u32) -> bool;
    /// Whether a region layout is active for this seed (base vs MQ / US vs JP).
    fn layout_active(&self, layout: GameLayout) -> bool;
    fn mask_count(&self) -> u16 {
        0
    }
    fn trick(&self, _id: u32) -> bool {
        false
    }
    fn special(&self, _id: u32) -> bool {
        false
    }
    /// Whether the seed placed a given song at a given event slot (from the
    /// spoiler). Default `true` (optimistic) until the app supplies the map.
    fn song_event(&self, _game: u8, _slot: u8, _song: u8) -> bool {
        true
    }
    /// The set of reachable MM time slices (bitmask over `data::MM_TIME_SLICES`).
    /// Default all-bits (optimistic); the app inputs narrow it under clock shuffle.
    fn mm_time_slices(&self) -> u64 {
        u64::MAX
    }
    /// Entrance-randomizer edge redirects: maps `(from_region, vanilla_to_region)`
    /// to the shuffled destination region(s). `None` (the default) means vanilla
    /// entrances — the solver then follows every edge to its compiled target.
    fn exit_redirects(&self) -> Option<&HashMap<(u32, u32), Vec<u32>>> {
        None
    }
    /// Extra region roots to seed as reachable, on top of SPAWN/GLOBAL. Default none;
    /// the app inputs supply the player's visited regions in progressive entrance mode
    /// (see `WorldInputs::seed_from_visited`) so a scene the player has physically
    /// reached is reachable even without a discovered entrance chain back to SPAWN.
    fn extra_seed_regions(&self) -> &[u32] {
        &[]
    }
    /// Progressive entrance mode: whether a *vanilla* (non-redirected) edge between
    /// two regions crosses a real entrance the player has not discovered yet, and so
    /// must be treated as impassable. Default `false` (full-knowledge: every vanilla
    /// edge open). Only consulted for edges with no `exit_redirects` entry — shuffled
    /// entrances are already gated by their (possibly empty) redirect. The app impl
    /// (`WorldInputs::edge_blocked`) walls only cross-scene edges that map to a real
    /// entrance whose scene transition is not in the discovered set, so intra-scene
    /// movement and non-entrance scene links (boss doors) stay free.
    fn edge_blocked(&self, _from_region: u32, _to_region: u32) -> bool {
        false
    }
}

/// A `WorldState` view over the inputs at a fixed age, reading the solver's
/// current event set. Rebuilt cheaply per (region, age) evaluation.
struct View<'a, I: Inputs> {
    inp: &'a I,
    events: &'a HashSet<u32>,
    age: u8,
}

impl<I: Inputs> WorldState for View<'_, I> {
    fn item_count(&self, id: u32) -> u32 {
        self.inp.item_count(id)
    }
    fn var_count(&self, var: u8) -> u16 {
        self.inp.var_count(var)
    }
    fn mask_count(&self) -> u16 {
        self.inp.mask_count()
    }
    fn setting_value(&self, key: u32) -> Option<u32> {
        self.inp.setting_value(key)
    }
    fn setting_has(&self, key: u32, val: u32) -> bool {
        self.inp.setting_has(key, val)
    }
    fn setting_enabled(&self, key: u32) -> bool {
        self.inp.setting_enabled(key)
    }
    fn trick(&self, id: u32) -> bool {
        self.inp.trick(id)
    }
    fn special(&self, id: u32) -> bool {
        self.inp.special(id)
    }
    fn song_event(&self, game: u8, slot: u8, song: u8) -> bool {
        self.inp.song_event(game, slot, song)
    }
    fn mm_time_slices(&self) -> u64 {
        self.inp.mm_time_slices()
    }
    fn event(&self, id: u32) -> bool {
        self.events.contains(&id)
    }
    fn flag_on(&self, _id: u32) -> bool {
        false
    }
    // MM region state is treated as always satisfiable in v1 (see module docs).
    fn flag(&self, _id: u32, _want: bool) -> bool {
        true
    }
    fn age(&self) -> u8 {
        self.age
    }
}

/// Evaluate an access rule that depends only on settings and inventory (no age,
/// events, flags, or time) against `inp`. Used to precompute the MM reachable
/// time-slice set from the six clock-period rules (`MM_CLOCK_PERIOD_EXPRS`) at
/// input-build time — those rules are pure `has`/`setting`, so the age and empty
/// event set fed here never affect the result.
pub fn eval_settings_only<I: Inputs>(expr: &[data::Op], inp: &I) -> bool {
    let events = HashSet::new();
    eval(expr, &View { inp, events: &events, age: 0 })
}

/// The result of a solve: which checks are reachable.
pub struct Reachability {
    pub locations: HashSet<&'static str>,
}

impl Reachability {
    /// Whether a check location (its `ObjectDef.location` string) is reachable.
    pub fn reachable(&self, location: &str) -> bool {
        self.locations.contains(location)
    }
}

/// Region indices to seed as reachable at both ages (the world roots).
fn seed_regions() -> Vec<usize> {
    let wanted = [(0u8, "SPAWN"), (0, "GLOBAL"), (1, "GLOBAL")];
    data::LOGIC_REGIONS
        .iter()
        .enumerate()
        .filter(|(_, r)| wanted.iter().any(|&(g, n)| r.game == g && r.name == n))
        .map(|(i, _)| i)
        .collect()
}

/// Run the reachability fixed point for the given inputs.
pub fn solve<I: Inputs>(inp: &I) -> Reachability {
    let regions = data::LOGIC_REGIONS;
    let n = regions.len();
    let mut reached = vec![[false; 2]; n];
    let mut events: HashSet<u32> = HashSet::new();
    let redirects = inp.exit_redirects();

    for i in seed_regions() {
        if inp.layout_active(regions[i].layout) {
            reached[i] = [true, true];
        }
    }

    // Extra roots (progressive entrance mode) are the player's visited scenes; they
    // are seeded *inside* the loop below so their age gating can track the fixed point.
    // For OoT they must open only at an age the player can actually reach: seeding a
    // scene walked as child at the adult age too would light every adult-only check in
    // it even with no way to time travel. That achievable-age set is exactly what the
    // SPAWN age gate computes (`SPAWN CHILD`/`SPAWN ADULT` = starting age or TIME_TRAVEL),
    // so we read those two nodes' reachability rather than re-deriving the rule. MM has
    // no such gate (its GLOBAL seeds both ages), so MM extra roots stay both-age.
    let spawn_child_idx = regions
        .iter()
        .position(|r| r.game == 0 && r.name == "SPAWN CHILD");
    let spawn_adult_idx = regions
        .iter()
        .position(|r| r.game == 0 && r.name == "SPAWN ADULT");

    // Iterate to a fixed point. Region reachability is updated in place (so later
    // regions in the same pass see it); events are collected and applied at the
    // end of each pass, which keeps the borrow of `events` immutable during eval.
    loop {
        let mut changed = false;
        let mut new_events: Vec<u32> = Vec::new();
        for (ri, r) in regions.iter().enumerate() {
            if !inp.layout_active(r.layout) {
                continue;
            }
            for age in 0u8..2 {
                if !reached[ri][age as usize] {
                    continue;
                }
                let view = View { inp, events: &events, age };
                for e in r.exits {
                    // Under entrance rando a shuffled edge points at new target(s);
                    // a vanilla edge keeps its single compiled target — unless
                    // progressive mode walls it as an undiscovered entrance.
                    let targets: &[u32] = match redirects.and_then(|m| m.get(&(ri as u32, e.to))) {
                        Some(v) => v.as_slice(), // shuffled: open iff discovered (non-empty)
                        None if inp.edge_blocked(ri as u32, e.to) => &[], // undiscovered vanilla entrance
                        None => std::slice::from_ref(&e.to),
                    };
                    // The access rule gates taking the entrance, not its target, so
                    // evaluate it once and apply to every destination.
                    if !eval(&data::EXPRS[e.expr as usize], &view) {
                        continue;
                    }
                    for &to in targets {
                        let to = to as usize;
                        if reached[to][age as usize] || !inp.layout_active(regions[to].layout) {
                            continue;
                        }
                        reached[to][age as usize] = true;
                        changed = true;
                    }
                }
                for ev in r.events {
                    if !events.contains(&ev.event)
                        && eval(&data::EXPRS[ev.expr as usize], &view)
                    {
                        new_events.push(ev.event);
                    }
                }
            }
        }
        for e in new_events {
            if events.insert(e) {
                changed = true;
            }
        }

        // (Re)seed the extra roots now that this pass has updated the SPAWN age gate.
        // OoT roots open only at an achievable age (child/adult per `SPAWN CHILD`/
        // `SPAWN ADULT`); MM has no age gate, so both ages. If the SPAWN nodes are
        // somehow absent, fall back to both ages (old behaviour) rather than hiding
        // everything. TIME_TRAVEL flips the adult gate mid-solve, so re-running each
        // pass lets a scene's adult checks appear the moment time travel is available.
        let oot_child_ok = spawn_child_idx.map_or(true, |i| reached[i][0]);
        let oot_adult_ok = spawn_adult_idx.map_or(true, |i| reached[i][1]);
        for &ri in inp.extra_seed_regions() {
            let i = ri as usize;
            if i >= n || !inp.layout_active(regions[i].layout) {
                continue;
            }
            let ages = if regions[i].game == 0 {
                [oot_child_ok, oot_adult_ok]
            } else {
                [true, true]
            };
            for (age, &ok) in ages.iter().enumerate() {
                if ok && !reached[i][age] {
                    reached[i][age] = true;
                    changed = true;
                }
            }
        }

        if !changed {
            break;
        }
    }

    // Collect the reachable checks: a location is reachable if any hosting region
    // is reachable at an age where its rule holds.
    let mut locations = HashSet::new();
    for (ri, r) in regions.iter().enumerate() {
        if !inp.layout_active(r.layout) {
            continue;
        }
        for age in 0u8..2 {
            if !reached[ri][age as usize] {
                continue;
            }
            let view = View { inp, events: &events, age };
            for l in r.locations {
                if !locations.contains(l.loc) && eval(&data::EXPRS[l.expr as usize], &view) {
                    locations.insert(l.loc);
                }
            }
        }
    }

    Reachability { locations }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::collections::{HashMap, HashSet};

    /// A configurable `Inputs` for tests: an owned-item set (unlisted = 0 or a
    /// blanket count), a resolved setting config, and the active layouts.
    struct Cfg {
        items_all: u32,               // blanket count for every item id
        settings: HashMap<u32, u32>,  // key idx -> value idx
        enabled: HashSet<u32>,        // boolean settings that are on
        masks: u16,
        specials: bool,
        extra_roots: Vec<u32>,        // extra reachable region roots (progressive seeding)
    }

    impl Inputs for Cfg {
        fn item_count(&self, _id: u32) -> u32 {
            self.items_all
        }
        fn extra_seed_regions(&self) -> &[u32] {
            &self.extra_roots
        }
        fn setting_value(&self, key: u32) -> Option<u32> {
            self.settings.get(&key).copied()
        }
        fn setting_enabled(&self, key: u32) -> bool {
            self.enabled.contains(&key)
        }
        fn layout_active(&self, layout: GameLayout) -> bool {
            matches!(layout, GameLayout::all | GameLayout::oot | GameLayout::mm)
        }
        fn mask_count(&self) -> u16 {
            self.masks
        }
        fn special(&self, _id: u32) -> bool {
            self.specials
        }
    }

    fn setting_idx(name: &str) -> u32 {
        data::SETTING_KEYS.iter().position(|&k| k == name).unwrap_or_else(|| {
            panic!("unknown setting key {name}")
        }) as u32
    }
    fn value_idx(name: &str) -> u32 {
        data::SETTING_VALUES.iter().position(|&v| v == name).unwrap_or_else(|| {
            panic!("unknown setting value {name}")
        }) as u32
    }

    /// Owns everything, child start with the Door of Time open — a broadly open
    /// world for reachability tests.
    fn full() -> Cfg {
        let mut settings = HashMap::new();
        settings.insert(setting_idx("startingAgeOot"), value_idx("child"));
        settings.insert(setting_idx("doorOfTime"), value_idx("open"));
        Cfg {
            items_all: 99,
            settings,
            enabled: HashSet::new(),
            masks: 24,
            specials: true,
            extra_roots: Vec::new(),
        }
    }

    fn empty() -> Cfg {
        let mut settings = HashMap::new();
        settings.insert(setting_idx("startingAgeOot"), value_idx("child"));
        Cfg { items_all: 0, settings, enabled: HashSet::new(), masks: 0, specials: false, extra_roots: Vec::new() }
    }

    /// Progressive seeding: regions handed to the solver as extra reachable roots
    /// (the player's visited scenes) open their checks even when no path from SPAWN
    /// reaches them. Rooting the whole graph with no items must therefore expose far
    /// more checks than the SPAWN-only baseline.
    #[test]
    fn extra_seed_regions_root_the_solver() {
        let base = solve(&empty());
        let mut all = empty();
        all.extra_roots = (0..data::LOGIC_REGIONS.len() as u32).collect();
        let seeded = solve(&all);
        assert!(
            seeded.locations.len() > base.locations.len(),
            "seeding regions as roots must open checks unreachable from SPAWN alone \
             (base {}, seeded {})",
            base.locations.len(),
            seeded.locations.len()
        );
    }

    /// Regression: an extra root (a scene the player has physically walked, progressive
    /// mode) must open only at an age the player can actually reach. Standing at the
    /// Sacred Forest Meadow entryway as a child with no items and no way to time travel,
    /// the maze past the gate must stay hidden — its only child path needs Wolfos/damage
    /// and the entryway→meadow exit's `is_adult` branch must not fire. Before the fix,
    /// extra roots were seeded at BOTH ages, so that adult branch lit every `true`-ruled
    /// wonder item in the maze even though the player could never become adult.
    #[test]
    fn extra_root_scene_does_not_leak_adult_only_access() {
        let entryway = data::LOGIC_REGIONS
            .iter()
            .position(|r| r.game == 0 && r.name == "Sacred Meadow Entryway")
            .expect("Sacred Meadow Entryway region") as u32;

        // Child start, no items: only the entryway itself opens, not the gated maze.
        let mut child = empty();
        child.extra_roots = vec![entryway];
        let rc = solve(&child);
        assert!(
            rc.reachable("OOT Sacred Meadow Wonder Item Entrance"),
            "the entryway check we are standing on must show"
        );
        assert!(
            !rc.reachable("OOT Sacred Meadow Wonder Item Maze 1"),
            "the maze past the closed gate must stay hidden for a child who cannot time travel"
        );

        // Adult start: the same seeding reaches the maze through the entryway's adult
        // branch — proving the gate is age-conditioned, not simply always shut.
        let mut adult = empty();
        adult.settings.insert(setting_idx("startingAgeOot"), value_idx("adult"));
        adult.extra_roots = vec![entryway];
        let ra = solve(&adult);
        assert!(
            ra.reachable("OOT Sacred Meadow Wonder Item Maze 1"),
            "an adult-start player standing at the entryway reaches the maze"
        );
    }

    #[test]
    fn solver_terminates_and_reaches_a_lot_when_fully_equipped() {
        let r = solve(&full());
        // A well-equipped child-start seed reaches most of the ~6000 checks.
        assert!(
            r.locations.len() > 3000,
            "expected broad reachability, got {}",
            r.locations.len()
        );
    }

    /// The Sacred Forest Meadow gossip-fairy checks resolve and are reachable once the
    /// meadow's access is owned. OoTMM gates the ADULT meadow entry behind Saria's Song
    /// (`Lost Woods Deep: is_child || can_play_saria || …`, lost_woods.yml), so a fully
    /// equipped solve reaches them while a child-only / song-less inventory would not —
    /// which is why the tracker (correctly) hides them for a player who has walked there
    /// as adult without Saria's Song. Guards the location strings + the region graph
    /// against a regression that would drop the whole meadow (or its gossip stones).
    #[test]
    fn sacred_meadow_gossip_fairy_reachable_when_equipped() {
        let r = solve(&full());
        for loc in [
            "OOT Sacred Meadow Gossip Fairy Maze 1",
            "OOT Sacred Meadow Gossip Fairy Maze 2",
            "OOT Sacred Meadow Gossip Fairy Deep",
            "OOT Sacred Meadow Wonder Item Maze 1",
            "OOT Temple of Time Exterior Gossip Fairy 1",
        ] {
            assert!(r.reachable(loc), "fully equipped should reach {loc}");
        }
    }

    /// Regression: OoTMM overloads the name `SONG_STORMS` for both the Song of Storms
    /// ITEM (OOT_SONG_STORMS 0x8E) and the windmill NPC (renamed OOT_SONG_OF_STORMS 0x06,
    /// which also equals OOT_BOOMERANG). A stale gen_logic alias mapped the logic's
    /// `has(SONG_STORMS)` to the NPC id 0x06, so collecting the BOOMERANG wrongly satisfied
    /// `can_play_storms` and lit the gossip "big fairy". `can_play_storms` must key on the
    /// real song item only. (Ocarina buttons aren't shuffled here, so they fold to `true`.)
    #[test]
    fn boomerang_does_not_satisfy_song_of_storms() {
        use crate::data::iid;
        let mq = std::collections::HashSet::new();
        // Adult start puts Link at the Temple of Time, so the ToT-Exterior gossip
        // region is reachable with almost no items — isolating `can_play_storms`
        // (the only remaining gate on the "big fairy") from world-traversal noise.
        let base = |extra: &[(u32, u32)]| {
            let mut s = crate::settings::Settings::default();
            s.parse_spoiler("Settings\n  startingAgeOot: adult\n  doorOfTime: open\n", &mq);
            s.apply(&mq);
            s.starting_item_ids.insert(iid::OOT_OCARINA, 1); // has_ocarina
            for &(id, n) in extra {
                s.starting_item_ids.insert(id, n);
            }
            crate::logic::solve_world(&s, &[crate::WorldData::default()], 1, &Default::default(), false)
        };
        let big = "OOT Temple of Time Exterior Gossip Big Fairy 1"; // gossip_fairy_big = can_play_storms

        // Boomerang (id 0x06) must NOT masquerade as the Song of Storms item.
        assert!(
            !base(&[(iid::OOT_BOOMERANG, 1)]).reachable(big),
            "the Boomerang must not satisfy can_play_storms"
        );
        // The actual Song of Storms item (0x8E) opens it.
        assert!(
            base(&[(iid::OOT_SONG_STORMS, 1)]).reachable(big),
            "Song of Storms opens the gossip big fairy"
        );
    }

    /// Regression for **song-note partiality** (`songs: notes` shuffle). A song is only
    /// playable once ALL its notes are owned: OoTMM gates `has_song_storms` behind
    /// `has_shared_count(SONG_NOTE_STORMS, …, 6)` — six copies of the note item — so a
    /// single collected note must NOT light the storms-gated gossip "big fairy", and six
    /// must. Confirms the tracker counts note items at their own id (0xA2) with the real
    /// threshold rather than crediting the song on the first note.
    #[test]
    fn song_notes_unlock_only_when_all_are_collected() {
        use crate::data::iid;
        let mq = std::collections::HashSet::new();
        let base = |notes: u32| {
            let mut s = crate::settings::Settings::default();
            s.parse_spoiler("Settings\n  startingAgeOot: adult\n  doorOfTime: open\n  songs: notes\n", &mq);
            s.apply(&mq);
            s.starting_item_ids.insert(iid::OOT_OCARINA, 1);
            s.starting_item_ids.insert(iid::OOT_SONG_NOTE_STORMS, notes);
            crate::logic::solve_world(&s, &[crate::WorldData::default()], 1, &Default::default(), false)
        };
        let big = "OOT Temple of Time Exterior Gossip Big Fairy 1";
        assert!(!base(1).reachable(big), "one storm note must not unlock the song");
        assert!(base(6).reachable(big), "six storm notes complete the Song of Storms");
    }

    #[test]
    fn deku_tree_reachable_when_equipped() {
        let r = solve(&full());
        assert!(
            r.reachable("OOT Deku Tree Map Chest"),
            "Deku Tree Map Chest should be reachable fully equipped"
        );
    }

    #[test]
    fn reachability_is_monotonic_in_items() {
        // Everything reachable with no items must still be reachable with all
        // items (adding items never closes a door).
        let poor = solve(&empty());
        let rich = solve(&full());
        for loc in &poor.locations {
            assert!(
                rich.locations.contains(loc),
                "{loc} reachable while poor but not while rich"
            );
        }
        assert!(rich.locations.len() >= poor.locations.len());
    }

    /// With everything owned (child start, Door of Time open) the solver reaches
    /// the large majority of each game's ACTIVE-layout checks. A sharp drop here
    /// means the region graph or the fixed point regressed (e.g. a whole dungeon
    /// went unreachable). The remaining tail is endgame / setting-specific gates
    /// this arbitrary config does not open.
    #[test]
    fn full_equip_reaches_most_active_checks() {
        let full = solve(&full());
        let logic_set = crate::logic::logic_location_set();
        let active = |l| matches!(l, GameLayout::all | GameLayout::oot | GameLayout::mm);
        for (label, objs, floor) in
            [("OoT", data::OOT_OBJECTS, 0.90), ("MM", data::MM_OBJECTS, 0.85)]
        {
            let (mut tot, mut reach) = (0.0f64, 0.0f64);
            for o in objs {
                if o.type_ == data::ObjectType::none
                    || !active(o.layout)
                    || !logic_set.contains(o.location)
                {
                    continue;
                }
                tot += 1.0;
                if full.locations.contains(o.location) {
                    reach += 1.0;
                }
            }
            assert!(
                reach / tot >= floor,
                "{label}: only {:.1}% of active checks reachable ({reach}/{tot})",
                100.0 * reach / tot
            );
        }
    }

    /// A child with a bronze scale, rupees available, and the Fishing Pond owner's
    /// soul reaches the shuffled pond fish — the exact inventory a player fishing at
    /// the pond has. Guards the macro chain `soul_fishing_pond_owner` /
    /// `can_use_wallet` and the child `Lake Hylia -> Near Pond -> Fishing Pond`
    /// route (a regression here would wrongly dim every pond-fish check).
    #[test]
    fn fishing_pond_fish_reachable_with_scale_soul_and_rupees() {
        use crate::data::iid;
        let mq = std::collections::HashSet::new();
        let mut s = crate::settings::Settings::default();
        s.parse_spoiler(
            "Settings\n  startingAgeOot: child\n  doorOfTime: open\n  bronzeScale: true\n  soulsNpcOot: true\n  rustyKeysOot: false\n",
            &mq,
        );
        s.apply(&mq);
        // Bronze scale (opens the child swim into Near Pond) + the pond owner's soul.
        s.starting_item_ids.insert(iid::OOT_SCALE, 1);
        s.starting_item_ids.insert(iid::OOT_SOUL_NPC_FISHING_POND_OWNER, 1);
        let r = crate::logic::solve_world(
            &s,
            std::slice::from_ref(&crate::WorldData::default()),
            1,
            &Default::default(),
            false,
        );
        assert!(r.reachable("OOT Fishing Pond Child Fish 1"), "pond fish should be reachable");
        assert!(r.reachable("OOT Fishing Pond Child"), "pond prize should be reachable");
    }

    /// With `rustyKeysOot` on, the pond entrance needs `has(RUSTY_KEY_FISHING_POND)`.
    /// The key obtained through a COLLECTED check (its spoiler placement name resolved
    /// by `find_item_id`, the real app inventory path) must credit id 0x1f0 and open
    /// the pond — guards against a rusty-key name/id drift silencing the credit.
    #[test]
    fn rusty_key_from_collected_check_opens_pond() {
        use crate::data::iid;
        use crate::scene::Game;
        let mq = std::collections::HashSet::new();
        let mut s = crate::settings::Settings::default();
        s.parse_spoiler(
            "Settings\n  startingAgeOot: child\n  doorOfTime: open\n  bronzeScale: true\n  soulsNpcOot: true\n  rustyKeysOot: true\n",
            &mq,
        );
        s.apply(&mq);
        s.starting_item_ids.insert(iid::OOT_SCALE, 1);
        s.starting_item_ids.insert(iid::OOT_SOUL_NPC_FISHING_POND_OWNER, 1);

        let loc = "OOT Kokiri Forest Kokiri Sword Chest";
        let idx = data::OOT_OBJECTS.iter().position(|o| o.location == loc).unwrap();
        let mut w = crate::WorldData::default();
        w.items.insert(loc.to_string(), "Rusty Key (Fishing Pond)".to_string());
        w.collected.insert((Game::Oot, idx));

        // Without the key: pond walled. With it: reachable.
        let no_key = crate::logic::solve_world(&s, std::slice::from_ref(&crate::WorldData::default()), 1, &Default::default(), false);
        assert!(!no_key.reachable("OOT Fishing Pond Child Fish 1"), "no rusty key => pond walled");
        let with_key = crate::logic::solve_world(&s, std::slice::from_ref(&w), 1, &Default::default(), false);
        assert!(with_key.reachable("OOT Fishing Pond Child Fish 1"), "collected rusty key must open the pond");
    }

    /// With `childWallets` on, `can_use_wallet(n)` needs the wallet **tier counter**
    /// (`has(OOT_WALLET, n)`). The shuffled pool hands out "Progressive Wallet (OoT)"
    /// (`OOT_PROG_WALLET`, a different id) or specific tier wallets, none of which move
    /// the counter through the naive tally — so without the wallet-tier fold every
    /// wallet-gated check (the whole Fishing Pond) stays dark despite wallets in hand.
    /// Covers the exact reported seed: child, rusty keys on, pond key + owner soul +
    /// bronze scale collected, wallets progressive.
    #[test]
    fn progressive_wallet_satisfies_can_use_wallet_at_pond() {
        use crate::data::iid;
        let mq = std::collections::HashSet::new();
        let mut s = crate::settings::Settings::default();
        s.parse_spoiler(
            "Settings\n  startingAgeOot: child\n  doorOfTime: open\n  bronzeScale: true\n  soulsNpcOot: true\n  rustyKeysOot: true\n  childWallets: true\n",
            &mq,
        );
        s.apply(&mq);
        s.starting_item_ids.insert(iid::OOT_SCALE, 1);
        s.starting_item_ids.insert(iid::OOT_SOUL_NPC_FISHING_POND_OWNER, 1);
        s.starting_item_ids.insert(iid::OOT_RUSTY_KEY_FISHING_POND, 1);

        // No wallet yet: childWallets gates paying, so the whole pond is dark.
        let none = crate::logic::solve_world(&s, std::slice::from_ref(&crate::WorldData::default()), 1, &Default::default(), false);
        assert!(!none.reachable("OOT Fishing Pond Child Fish 1"), "no wallet + childWallets => pond dark");

        // Two progressive wallets collected (tier 2): the counter must reflect it.
        s.starting_item_ids.insert(iid::OOT_PROG_WALLET, 2);
        let inp = crate::logic::inputs::WorldInputs::build(&s, std::slice::from_ref(&crate::WorldData::default()), 1, &Default::default(), false);
        assert_eq!(inp.item_count(iid::OOT_WALLET), 2, "progressive wallets must fold onto the tier counter");
        let with = crate::logic::solve_world(&s, std::slice::from_ref(&crate::WorldData::default()), 1, &Default::default(), false);
        assert!(with.reachable("OOT Fishing Pond Child Fish 1"), "progressive wallet must open the pond");
    }

    /// `pondFishShuffle`: the weighted fish become real, uniquely-ided shuffled items,
    /// so the pond PRIZE (heart piece / scale) needs one heavy enough. `has_pond_fish`
    /// is compiled to a real OR of `has(<weighted fish>)` (not optimistic): with the
    /// setting on and no fish the prize is dark (while the fish CHECKS, gated only by
    /// pond access, stay lit); an in-range fish collected through the real inventory
    /// path lights it, a too-light one does not. With the setting off the prize is
    /// reachable regardless (vanilla: any weight is catchable). Covers the reported
    /// seed (child, pond accessible, no fish -> prize wrongly shown before this fix).
    #[test]
    fn pond_prize_needs_a_heavy_enough_shuffled_fish() {
        use crate::data::iid;
        use crate::scene::Game;
        let mq = std::collections::HashSet::new();
        let base = "Settings\n  startingAgeOot: child\n  doorOfTime: open\n  bronzeScale: true\n  soulsNpcOot: true\n  rustyKeysOot: false\n";

        // Solve with pondFishShuffle on, optionally having collected one named fish
        // (placed at a real check, exercising the find_item_id crediting path).
        let solve_with_fish = |fish: Option<&str>| {
            let mut s = crate::settings::Settings::default();
            s.parse_spoiler(&format!("{base}  pondFishShuffle: true\n"), &mq);
            s.apply(&mq);
            s.starting_item_ids.insert(iid::OOT_SCALE, 1);
            s.starting_item_ids.insert(iid::OOT_SOUL_NPC_FISHING_POND_OWNER, 1);
            let w = match fish {
                Some(name) => {
                    let loc = "OOT Kokiri Forest Kokiri Sword Chest";
                    let idx = data::OOT_OBJECTS.iter().position(|o| o.location == loc).unwrap();
                    let mut w = crate::WorldData::default();
                    w.items.insert(loc.to_string(), name.to_string());
                    w.collected.insert((Game::Oot, idx));
                    w
                }
                None => crate::WorldData::default(),
            };
            crate::logic::solve_world(&s, std::slice::from_ref(&w), 1, &Default::default(), false)
        };

        // No fish: pond access is fine (fish checks lit) but the prize is dark.
        let none = solve_with_fish(None);
        assert!(none.reachable("OOT Fishing Pond Child Fish 1"), "pond access unaffected: fish check stays reachable");
        assert!(!none.reachable("OOT Fishing Pond Child"), "pondFishShuffle on + no fish => prize dark");

        // Too light (3 lbs < the 7 lbs the child prize needs): must not open it.
        assert!(!solve_with_fish(Some("Child Fish (3 pounds)")).reachable("OOT Fishing Pond Child"),
            "a too-light fish must not satisfy the prize");

        // Heavy enough (7 lbs, in [7,14]) collected through the real inventory path: lit.
        let child_fish = solve_with_fish(Some("Child Fish (7 pounds)"));
        assert!(child_fish.reachable("OOT Fishing Pond Child"),
            "an in-range shuffled fish must satisfy the prize");
        // Age separation: the child and adult prizes compile to DISJOINT fish item
        // sets (child 0x199..=0x1a0 / adult 0x1a5..=0x1b6) plus an `is_child` /
        // `is_adult` gate, so a fish caught as one age can never open the other
        // age's prize. A child fish must not open the adult prize.
        assert!(!child_fish.reachable("OOT Fishing Pond Adult"),
            "a CHILD fish must not open the ADULT prize");

        // pondFishShuffle off: prize reachable with no fish at all (vanilla pond).
        let mut off = crate::settings::Settings::default();
        off.parse_spoiler(base, &mq);
        off.apply(&mq);
        off.starting_item_ids.insert(iid::OOT_SCALE, 1);
        off.starting_item_ids.insert(iid::OOT_SOUL_NPC_FISHING_POND_OWNER, 1);
        let off_r = crate::logic::solve_world(&off, std::slice::from_ref(&crate::WorldData::default()), 1, &Default::default(), false);
        assert!(off_r.reachable("OOT Fishing Pond Child"), "pondFishShuffle off => prize reachable regardless of fish");
    }

    /// A specific-tier wallet (Adult's Wallet, not the progressive pool item) must also
    /// fold onto the counter so `can_use_wallet` sees it.
    #[test]
    fn specific_tier_wallet_satisfies_can_use_wallet() {
        use crate::data::iid;
        let mq = std::collections::HashSet::new();
        let mut s = crate::settings::Settings::default();
        s.parse_spoiler(
            "Settings\n  startingAgeOot: child\n  childWallets: true\n",
            &mq,
        );
        s.apply(&mq);
        s.starting_item_ids.insert(iid::OOT_WALLET2, 1); // Adult's Wallet = tier 2
        let inp = crate::logic::inputs::WorldInputs::build(&s, std::slice::from_ref(&crate::WorldData::default()), 1, &Default::default(), false);
        assert_eq!(inp.item_count(iid::OOT_WALLET), 2, "Adult's Wallet must raise the counter to tier 2");
    }

    /// Separate-tier upgrades (not the progressive pool item) must fold onto the tier
    /// counter the logic tests, across every family: scale, strength, ocarina, magic
    /// and the OoT longshot. Guards `normalize_tier_counters` against a missed family.
    #[test]
    fn separate_tier_upgrades_fold_onto_counter() {
        use crate::data::iid;
        let mq = std::collections::HashSet::new();
        let build = |id: u32, n: u32| {
            let mut s = crate::settings::Settings::default();
            s.apply(&mq);
            s.starting_item_ids.insert(id, n);
            crate::logic::inputs::WorldInputs::build(
                &s,
                std::slice::from_ref(&crate::WorldData::default()),
                1,
                &Default::default(),
                false,
            )
        };
        // (separate tier item, its counter, expected tier).
        let cases = [
            (iid::OOT_SCALE_GOLDEN, iid::OOT_SCALE, 3),
            (iid::OOT_SCALE_SILVER, iid::OOT_SCALE, 2),
            (iid::MM_SCALE_GOLDEN, iid::MM_SCALE, 3),
            (iid::OOT_GOLDEN_GAUNTLETS, iid::OOT_STRENGTH, 3),
            (iid::OOT_GORON_BRACELET, iid::OOT_STRENGTH, 1),
            (iid::MM_SILVER_GAUNTLETS, iid::MM_STRENGTH, 2),
            (iid::OOT_OCARINA_TIME, iid::OOT_OCARINA, 2),
            (iid::MM_OCARINA_OF_TIME, iid::MM_OCARINA, 2),
            (iid::OOT_MAGIC_UPGRADE2, iid::OOT_MAGIC_UPGRADE, 2),
            (iid::MM_MAGIC_UPGRADE2, iid::MM_MAGIC_UPGRADE, 2),
            (iid::OOT_LONGSHOT, iid::OOT_HOOKSHOT, 2),
        ];
        for (item, counter, tier) in cases {
            let inp = build(item, 1);
            assert_eq!(
                inp.item_count(counter),
                tier,
                "item {item:#x} should fold onto counter {counter:#x} as tier {tier}"
            );
        }

        // The progressive pool item still counts directly (counter == its id), and the
        // fold must not lower it: three progressive scales stay tier 3.
        let prog = build(iid::OOT_SCALE, 3);
        assert_eq!(prog.item_count(iid::OOT_SCALE), 3, "progressive scales keep their tier");
    }

    #[test]
    fn empty_inventory_reaches_little() {
        // With nothing, only the free/near-spawn checks open; far less than full.
        let poor = solve(&empty());
        let rich = solve(&full());
        assert!(
            poor.locations.len() < rich.locations.len(),
            "empty={} full={}",
            poor.locations.len(),
            rich.locations.len()
        );
    }

    /// End-to-end check on the real "all open" spoiler fixture: it loads, the access /
    /// win-condition settings land in `raw_settings` (so the solver reads them), and once
    /// every placement is collected the seed reaches ~all of its active, logic-gated
    /// checks — i.e. it is clearable and the access logic did not wrongly wall anything
    /// off. Skips gracefully when the fixture is absent (it is a hand-supplied file).
    /// Run with `cargo test all_open_spoiler -- --nocapture` to see the numbers.
    #[test]
    fn all_open_spoiler_is_clearable() {
        use crate::scene::Game;
        let path = concat!(env!("CARGO_MANIFEST_DIR"), "/OoTMM-Spoiler-all-open.txt");
        let Ok(text) = std::fs::read_to_string(path) else {
            eprintln!("skip all_open_spoiler_is_clearable: {path} absent");
            return;
        };

        let sp = crate::spoiler::parse(&text);
        let mut settings = crate::settings::Settings::default();
        settings.parse_spoiler(&text, &sp.mq_scenes);
        settings.apply(&sp.mq_scenes);

        // The access / condition settings reached the logic feed (raw_settings).
        let got = |k: &str| settings.raw_settings.get(k).cloned().unwrap_or_default();
        assert_eq!(got("doorOfTime"), "open");
        assert_eq!(got("beneathWell"), "open");
        assert_eq!(got("rainbowBridge"), "custom");
        assert_eq!(got("ganonBossKey"), "anywhere");
        // The open-dungeon / trial SETS live in the World Flags section (display-label
        // lists) and are folded into raw_settings as their raw members.
        let members = |k: &str| got(k).split(',').filter(|t| !t.is_empty() && *t != "none").count();
        eprintln!(
            "sets: openDungeonsOot={} openDungeonsMm={} ganonTrials={} clearStateDungeonsMm={}",
            members("openDungeonsOot"), members("openDungeonsMm"),
            members("ganonTrials"), members("clearStateDungeonsMm"),
        );
        assert_eq!(members("openDungeonsOot"), 8, "all 8 OoT dungeons open");
        assert_eq!(members("openDungeonsMm"), 4, "all 4 MM temples open");
        assert_eq!(members("ganonTrials"), 6, "all 6 Ganon trials");
        assert_eq!(members("clearStateDungeonsMm"), 2, "WF + GB clear-state (World Flags)");
        // The dual setting fire_temple_open_as_child is DERIVED from openDungeonsOot's
        // `fireChild` member, so the one dungeon toggle drives display + logic.
        assert!(settings.fire_temple_open_as_child, "fireChild open => the map flag is set");
        // The Song Events section (shuffled event->song) reached the logic feed: every
        // slot resolved to a real song index (no u8::MAX), and the slot counts match the
        // spoiler (18 OoT event slots, 13 MM), so `_song_event_<game>(slot, song)` gates
        // are evaluated against the actual placement rather than the optimistic default.
        eprintln!(
            "song_events: OoT={} MM={} (unresolved={})",
            settings.song_events[0].len(), settings.song_events[1].len(),
            settings.song_events.iter().flatten().filter(|&&n| n == u8::MAX).count(),
        );
        assert_eq!(settings.song_events[0].len(), 18, "18 OoT song-event slots");
        assert_eq!(settings.song_events[1].len(), 13, "13 MM song-event slots");
        assert!(
            settings.song_events.iter().flatten().all(|&n| n != u8::MAX),
            "every song-event name resolved to an index",
        );
        let n_access = data::ACCESS_SETTINGS
            .iter()
            .filter(|a| settings.raw_settings.contains_key(a.key))
            .count();
        eprintln!("rom={:?}  mq_scenes={}", sp.rom, sp.mq_scenes.len());
        eprintln!("access/condition settings present: {n_access}/{}", data::ACCESS_SETTINGS.len());

        // The `custom` win conditions parsed their Special Conditions thresholds (these
        // back `special(BRIDGE/MOON/LACS)` when the setting is `custom`).
        for name in ["BRIDGE", "MOON", "LACS"] {
            let c = settings.special_conds.get(name).unwrap_or_else(|| panic!("{name} custom cond parsed"));
            eprintln!("special {name}: count={} cats={}", c.count, c.cats.len());
            assert!(c.count > 0 && !c.cats.is_empty(), "{name} threshold parsed");
        }

        // Build the (single) world from the seed's placements.
        let mut world = crate::WorldData::default();
        world.items = sp.worlds.into_iter().next().map(|w| w.items).unwrap_or_default();
        eprintln!("placements: {}", world.items.len());

        // Sphere 0 (starting items only), all-open settings.
        let start = crate::logic::solve_world(&settings, std::slice::from_ref(&world), 1, &Default::default(), false);

        // The access settings genuinely drive the logic: at sphere 0 the all-open config
        // opens strictly more than the SAME seed with every access / condition setting
        // stripped back to its logic default (door closed, bridge vanilla, …).
        let mut defaults = crate::settings::Settings::default();
        defaults.parse_spoiler(&text, &sp.mq_scenes);
        defaults.apply(&sp.mq_scenes);
        for a in data::ACCESS_SETTINGS {
            defaults.raw_settings.remove(a.key);
        }
        let start_closed = crate::logic::solve_world(&defaults, std::slice::from_ref(&world), 1, &Default::default(), false);
        eprintln!(
            "sphere0 reachable: all-open={}  access-defaults={}  (delta={})",
            start.locations.len(),
            start_closed.locations.len(),
            start.locations.len() as i64 - start_closed.locations.len() as i64,
        );
        assert!(
            start.locations.len() > start_closed.locations.len(),
            "the open-access settings must open more at sphere 0 (open={}, defaults={})",
            start.locations.len(),
            start_closed.locations.len(),
        );

        // Full clear (every placement collected).
        for (game, objs) in [(Game::Oot, data::OOT_OBJECTS), (Game::Mm, data::MM_OBJECTS)] {
            for (i, o) in objs.iter().enumerate() {
                if world.items.contains_key(o.location) {
                    world.collected.insert((game, i));
                }
            }
        }
        let full = crate::logic::solve_world(&settings, std::slice::from_ref(&world), 1, &Default::default(), false);
        eprintln!("reachable locations: sphere0={}  full-clear={}", start.locations.len(), full.locations.len());
        assert!(
            full.locations.len() > start.locations.len(),
            "collecting the seed's items must open more checks (start={}, full={})",
            start.locations.len(),
            full.locations.len()
        );

        // On a full clear, ~all active logic-gated checks are reachable (the seed clears).
        let logic_set = crate::logic::logic_location_set();
        let active = |l| matches!(l, GameLayout::all | GameLayout::oot | GameLayout::mm);
        for (label, objs, floor) in [("OoT", data::OOT_OBJECTS, 0.90), ("MM", data::MM_OBJECTS, 0.85)] {
            let (mut tot, mut reach) = (0.0f64, 0.0f64);
            for o in objs {
                if o.type_ == data::ObjectType::none || !active(o.layout) || !logic_set.contains(o.location) {
                    continue;
                }
                tot += 1.0;
                if full.locations.contains(o.location) {
                    reach += 1.0;
                }
            }
            eprintln!(
                "{label}: {:.1}% of active checks reachable on full clear ({}/{})",
                100.0 * reach / tot,
                reach as u32,
                tot as u32
            );
            assert!(
                reach / tot >= floor,
                "{label}: only {:.1}% reachable on full clear ({}/{})",
                100.0 * reach / tot,
                reach as u32,
                tot as u32
            );
        }
    }
}
