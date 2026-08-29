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
    /// The value a setting is fixed to (`Some(value_id)` into `SETTING_VALUES`).
    fn setting_value(&self, key: u32) -> Option<u32>;
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
    /// Entrance-randomizer edge redirects: maps `(from_region, vanilla_to_region)`
    /// to the shuffled destination region(s). `None` (the default) means vanilla
    /// entrances — the solver then follows every edge to its compiled target.
    fn exit_redirects(&self) -> Option<&HashMap<(u32, u32), Vec<u32>>> {
        None
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
    fn mask_count(&self) -> u16 {
        self.inp.mask_count()
    }
    fn setting_value(&self, key: u32) -> Option<u32> {
        self.inp.setting_value(key)
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
                    // a vanilla edge keeps its single compiled target.
                    let targets: &[u32] = redirects
                        .and_then(|m| m.get(&(ri as u32, e.to)))
                        .map_or(std::slice::from_ref(&e.to), Vec::as_slice);
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
    }

    impl Inputs for Cfg {
        fn item_count(&self, _id: u32) -> u32 {
            self.items_all
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
        }
    }

    fn empty() -> Cfg {
        let mut settings = HashMap::new();
        settings.insert(setting_idx("startingAgeOot"), value_idx("child"));
        Cfg { items_all: 0, settings, enabled: HashSet::new(), masks: 0, specials: false }
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
}
