//! Bridge the tracked game state to the solver's [`Inputs`].
//!
//! The solver speaks in the compiled logic's index spaces (item ids, setting-key
//! / setting-value / trick indices); the app tracks a spoiler (locations -> item
//! names + settings + tricks) and a set of collected checks. [`WorldInputs`]
//! resolves one into the other, once, before a solve:
//!
//! - **Inventory** = the seed's starting items plus, for every collected check,
//!   the item the spoiler placed there — resolved to its internal (dev) id with
//!   [`crate::progression::find_item_id`]. That id space is the one the logic's
//!   `has(item, n)` uses (`Items.h` == `data::ITEMS` == `Op::Has` operands), so
//!   progressive items (Hookshot -> Longshot, Strength tiers, …) just count.
//! - **Settings** come from the spoiler's verbatim `key: value` block
//!   ([`Settings::raw_settings`]): a boolean `true` enables `setting(k)`, and the
//!   value string, when it names a known `SETTING_VALUES` entry, backs
//!   `setting(k, v)`.
//! - **Tricks** are the ids parsed from the `Tricks` / `Glitches` sections.
//!
//! ## Song events & specials (precise, from the spoiler)
//! - `song_event(x)` resolves against the spoiler's `Song Events` slot->song map
//!   (see [`Settings::song_events`]); the `&& can_play_<song>` half is ordinary
//!   item leaves the solver already evaluates.
//! - `special(x)` (BRIDGE / GANON_BK / LACS / MAJORA / MOON) sums the owned items
//!   in the categories the spoiler's `Special Conditions` block enables against
//!   its `count`. It is only ever reached behind `setting(cond, custom)`; a
//!   non-custom seed short-circuits it. The mask categories are not modelled yet
//!   and fall back to optimistic (`true`) so they never hide a check.
//!
//! ## `layout_active` is always `true` (correct, not an approximation)
//! The solver explores both the base and the alternate (MQ / JP) variant of every
//! dungeon. This never mis-renders: the tracker's object-layout filter draws only
//! the seed's real variant, and the two variants use disjoint location strings, so
//! a dead variant's reachable locations are simply never shown. Exploring the extra
//! regions only ever *adds* reachability to invisible checks — it can never hide a
//! real one — so per-dungeon gating would change nothing observable.

use std::collections::{HashMap, HashSet};
use std::sync::OnceLock;

use crate::data::{self, GameLayout};
use crate::progression::find_item_id;
use crate::scene::Game;
use crate::settings::Settings;
use crate::WorldData;

use super::solve::Inputs;

/// Resolved, solver-ready view of one player's inventory + the seed settings.
pub struct WorldInputs {
    /// Internal item id -> count owned (starting items + collected placements).
    items: HashMap<u32, u32>,
    /// Number of masks owned (`masks(n)`, MM Moon trials).
    masks: u16,
    /// `SETTING_KEYS` indices whose spoiler value is boolean-true (`setting(k)`).
    settings_enabled: HashSet<u32>,
    /// `SETTING_KEYS` index -> `SETTING_VALUES` index (`setting(k, v)`).
    settings_value: HashMap<u32, u32>,
    /// `SETTING_KEYS` index -> the set of `SETTING_VALUES` indices selected for a
    /// set-valued setting (`openDungeonsOot/Mm`, `ganonTrials`), so `setting(k, member)`
    /// resolves by membership. A single-valued enum lands here too (a one-member set).
    settings_multi: HashMap<u32, HashSet<u32>>,
    /// `TRICK_NAMES` indices enabled by the seed.
    tricks: HashSet<u32>,
    /// Per-seed `var(NAME)` counts (index = `data::VAR_NAMES` id), read from the
    /// spoiler setting `data::VAR_SETTING_KEYS[i]` (default `data::VAR_DEFAULTS`).
    var_counts: Vec<u16>,
    /// Per game (`[OoT, MM]`) the song index placed at each event slot (indexed by
    /// slot), from the spoiler's `Song Events` section. `u8::MAX` = unresolved.
    song_events: [Vec<u8>; 2],
    /// Custom win-condition definitions (`special(X)`), keyed by SPECIAL name.
    special_conds: std::collections::HashMap<String, crate::settings::SpecialCond>,
    /// Entrance-randomizer edge redirects: `(from_region, vanilla_to_region)` ->
    /// shuffled destination region index(es). Empty when the seed has no ER.
    exit_redirects: HashMap<(u32, u32), Vec<u32>>,
    /// Reachable MM time slices (bitmask over `data::MM_TIME_SLICES`), precomputed
    /// from the clock-period rules. All-bits when clock shuffle (`clocksMm`) is
    /// off; under it, only the periods whose clock the player owns contribute.
    mm_slices: u64,
    /// Extra region roots seeded as reachable on top of SPAWN/GLOBAL. In progressive
    /// entrance mode these are the logic regions the player has physically been to
    /// (derived from the discovered entrance graph via [`Self::seed_from_visited`]):
    /// a live auto-tracker treats "where you have walked" as reachable and lets the
    /// solver extend from there through discovered entrances + logic. Empty otherwise
    /// (full-knowledge mode seeds only SPAWN, the true from-start reachability).
    seed_regions: Vec<u32>,
    /// Progressive entrance mode is on. Gates [`Inputs::edge_blocked`]: only here do
    /// undiscovered entrance edges wall reachability. Off = full-knowledge (every
    /// vanilla edge open).
    progressive: bool,
    /// Progressive mode: the scenes the player has physically discovered (landed in),
    /// as `(game, scene_id)` — the scenes hosting the seeded regions. A vanilla (non-
    /// redirected) real-entrance edge may only be crossed *into* a scene in this set;
    /// crossing into an undiscovered scene is walled. Populated by
    /// [`Self::seed_from_visited`]. Empty outside progressive mode.
    visited_scenes: HashSet<(u8, u32)>,
}

impl WorldInputs {
    /// Build the inputs for one player's reachability. `settings` must already be
    /// `apply`-ed (so `starting_item_ids` is populated). `worlds` are all physical
    /// worlds and `player` is the 1-based player whose map is shown: their
    /// inventory is every collected check, across all worlds, whose item is
    /// destined to that player (multiworld routing). Single / coop seeds have one
    /// world whose placements default to player 1, so this counts everything.
    pub fn build(
        settings: &Settings,
        worlds: &[WorldData],
        player: u8,
        discovered: &HashSet<(u8, u32)>,
        progressive: bool,
    ) -> Self {
        // Inventory: starting items, then +1 per collected check destined to us.
        let mut items: HashMap<u32, u32> = settings.starting_item_ids.clone();
        for (wi, w) in worlds.iter().enumerate() {
            let owner = (wi + 1) as u8; // this world's own player (1-based)
            for &(game, idx) in &w.collected {
                let objs = match game {
                    Game::Oot => data::OOT_OBJECTS,
                    Game::Mm => data::MM_OBJECTS,
                };
                let Some(obj) = objs.get(idx) else { continue };
                let Some(name) = w.items.get(obj.location) else { continue };
                // The pickup goes to its destination player (default: this owner);
                // only items routed to `player` enter that player's inventory.
                if w.dest.get(obj.location).copied().unwrap_or(owner) != player {
                    continue;
                }
                if let Some(id) = find_item_id(name) {
                    *items.entry(id).or_insert(0) += 1;
                }
            }
        }

        // OoTMM funnels each tiered item family (wallet / scale / strength / ocarina /
        // magic / hookshot) into one counter that the logic tests; re-derive those
        // counters from every collected tier source (see `normalize_tier_counters`).
        normalize_tier_counters(&mut items);

        // Masks: sum the counts of every owned mask item (generous on purpose —
        // masks() only gates the optimistic Moon trials).
        let masks = mask_item_ids()
            .iter()
            .filter_map(|id| items.get(id))
            .sum::<u32>()
            .min(u16::MAX as u32) as u16;

        // Settings: resolve the raw `key: value` block into the two index spaces.
        let (skeys, svals) = (setting_key_index(), setting_value_index());
        let mut settings_enabled = HashSet::new();
        let mut settings_value = HashMap::new();
        let mut settings_multi: HashMap<u32, HashSet<u32>> = HashMap::new();
        for (name, val) in &settings.raw_settings {
            let Some(&ki) = skeys.get(name.as_str()) else { continue };
            if val.eq_ignore_ascii_case("true") {
                settings_enabled.insert(ki);
            }
            if let Some(&vi) = svals.get(val.as_str()) {
                settings_value.insert(ki, vi);
            }
            // A set-valued setting stores a list of members (comma / whitespace
            // separated); index each recognised member so `setting(k, member)` holds.
            for tok in val.split(|c: char| c == ',' || c.is_whitespace()).filter(|t| !t.is_empty()) {
                if let Some(&vi) = svals.get(tok) {
                    settings_multi.entry(ki).or_default().insert(vi);
                }
            }
        }

        // Defaults for settings the seed omits. OoTMM applies each setting's default
        // when it is absent; an older / cut-down ROM version drops whole settings
        // (e.g. one without OoT clocks has no `clocksOot`). A missing enum setting
        // whose default is the unshuffled/"vanilla" state must still satisfy
        // `setting(k, <off value>)` — `is_day`/`is_night` gate on
        // `setting(clocksOot, none)`, so without this every OoT day/night check reads
        // unreachable on such a version. The first option is that off value (OoTMM
        // lists options default-first). Only vanilla-default settings are seeded, and
        // only when absent, so a present or actively-shuffled setting is untouched.
        for meta in data::FILTER_SETTINGS.iter().chain(data::ITEM_SETTINGS.iter()) {
            if meta.default != data::ShuffleSetting::vanilla || settings.raw_settings.contains_key(meta.key) {
                continue;
            }
            let Some(opt) = meta.options.first() else { continue };
            let (Some(&ki), Some(&vi)) = (skeys.get(meta.key), svals.get(opt.value)) else { continue };
            settings_value.entry(ki).or_insert(vi);
            settings_multi.entry(ki).or_default().insert(vi);
        }

        // Tricks: id string -> TRICK_NAMES index.
        let tri = trick_index();
        let tricks = settings
            .enabled_trick_ids
            .iter()
            .filter_map(|id| tri.get(id).copied())
            .collect();

        // `var(NAME)` counts (`Op::HasVar`): read each from its spoiler setting
        // (`VAR_SETTING_KEYS`), falling back to the OoTMM default (`VAR_DEFAULTS`).
        let var_counts: Vec<u16> = (0..data::VAR_NAMES.len())
            .map(|i| {
                data::VAR_SETTING_KEYS
                    .get(i)
                    .and_then(|k| settings.raw_settings.get(*k))
                    .and_then(|v| v.trim().parse::<u16>().ok())
                    .unwrap_or_else(|| data::VAR_DEFAULTS.get(i).copied().unwrap_or(1))
            })
            .collect();

        let mut wi = WorldInputs {
            items,
            masks,
            settings_enabled,
            settings_value,
            settings_multi,
            tricks,
            var_counts,
            song_events: settings.song_events.clone(),
            special_conds: settings.special_conds.clone(),
            exit_redirects: build_exit_redirects(&settings.entrance_remap, discovered, progressive),
            // Placeholder; the clock-period rules read only settings/inventory
            // (never `mm_time`), so computing over `wi` below is well-defined.
            mm_slices: u64::MAX,
            // Set by `seed_from_visited` after build (progressive mode only).
            seed_regions: Vec::new(),
            progressive,
            visited_scenes: HashSet::new(),
        };
        wi.mm_slices = compute_mm_slices(&wi);
        wi
    }

    /// Seed the reachability roots from the player's discovered entrance graph
    /// (progressive mode). `visited` holds every entrance id the player has actually
    /// walked through — BOTH the departure ids (`out_links` keys) and the arrival ids
    /// (`out_links` values). Each entrance's def gives the destination scene and the
    /// facing (`from_name`) side; the tracker detected the true arriving entrance, so
    /// this is correct under entrance rando and decoupling, not the vanilla assumption.
    ///
    /// Each entrance is rooted at the region the player ACTUALLY lands in, taken from
    /// the logic region graph: the destination-scene region(s) a `from_name` region
    /// reaches by one logic edge. The entrance table's `to_name` is only the
    /// destination AREA, which is the scene's hub even when the entrance really drops
    /// the player in a finer pocket walled off by an in-scene barrier — Lost Woods ->
    /// Goron City lands in the gated "Goron City Shortcut" (behind the three rocks),
    /// not the "Goron City" hub, so rooting `to_name` lit the whole city for free
    /// (reported). Reading the arrival off the graph edge keeps that barrier honest,
    /// and the solver then extends from each root through vanilla edges, discovered
    /// redirects and in-region logic. Falls back to matching `to_name` when the graph
    /// does not line up (names differ, cross-game portal, an entrance with no modelled
    /// edge). Names that resolve to no logic region are skipped.
    pub fn seed_from_visited(&mut self, visited: &HashSet<(u8, u32)>) {
        let names = region_name_index();
        let scenes = region_scenes();
        let mut regions = Vec::new();
        for &(game, id) in visited {
            let g = if game == 0 { crate::scene::Game::Oot } else { crate::scene::Game::Mm };
            let Some(meta) = crate::entrance::lookup(g, id) else { continue };
            let dest_scene = canon_entrance_scene(game, meta.to_scene as u32);
            // Primary: the region(s) in the destination scene that a `from_name` region
            // steps into over one edge — the true landing spot, barrier-aware.
            let before = regions.len();
            if let Some(srcs) = names[game as usize].get(meta.from_name) {
                for &s in srcs {
                    for edge in data::LOGIC_REGIONS[s as usize].exits {
                        let d = edge.to;
                        if data::LOGIC_REGIONS[d as usize].game == game
                            && scenes[d as usize] == Some(dest_scene)
                        {
                            regions.push(d);
                        }
                    }
                }
            }
            // Fallback: match `to_name` directly when the edge lookup found nothing.
            if regions.len() == before {
                if let Some(idxs) = names[game as usize].get(meta.to_name) {
                    regions.extend_from_slice(idxs);
                }
            }
        }
        regions.sort_unstable();
        regions.dedup();
        // The scenes those regions live in are the "discovered" scenes: a real
        // entrance may be crossed into one of them, but not into any other scene
        // (see `edge_blocked`). Derived from the regions so it tracks exactly what
        // the player has physically reached, ER and vanilla alike.
        self.visited_scenes = regions
            .iter()
            .filter_map(|&i| scenes[i as usize].map(|s| (data::LOGIC_REGIONS[i as usize].game, s)))
            .collect();
        self.seed_regions = regions;
    }

    /// Also seed the scene the player is physically standing in *right now* (the
    /// tracker's `player_obj_scene`), on top of [`Self::seed_from_visited`]. On
    /// arrival the entrance's `to_name` does not always resolve to the scene actually
    /// loaded — an owl flight or the reverse-pair OUT id can name the far side — so
    /// without this the current scene's checks only lit up one entrance later (the
    /// reported one-step lag). Seeding the live scene roots it immediately and marks
    /// it discovered, so `edge_blocked` lets movement fan out from it at once.
    /// Must be called after `seed_from_visited` (which overwrites both fields).
    ///
    /// Roots only the scene's **hub** region — the entry region with the most exits,
    /// i.e. the main area a player lands in — not every region of the scene. A scene's
    /// interior sub-areas (Lake Hylia's pond ledge, a bean-only spot) and its other
    /// entrance-fed pockets (the fishing-pond exit) are reachable solely through their
    /// own access rules / entrances, so seeding them would wrongly show them for free.
    /// Seeding just the hub lights the scene immediately while the normal fixed point
    /// gates the rest by the player's items.
    ///
    /// The hub is rooted **only as a fallback** — when `seed_from_visited` did not
    /// already root a region of this scene from a discovered entrance's `to_name`
    /// (owl flights, reverse-pair OUT ids whose `to_name` does not resolve to the
    /// loaded scene). When the real arrival region IS known, forcing the hub on top
    /// would jump the player past an intra-scene barrier standing between the arrival
    /// region and the hub: entering Goron City from the Lost Woods lands in the gated
    /// "Goron City Shortcut" pocket, whose only edge to the main city needs explosives
    /// / hammer / Din (the three blocking rocks), so with no bombs the nine city checks
    /// must stay hidden. Rooting the hub unconditionally showed them as reachable
    /// (reported). Letting the fixed point extend from the true arrival region alone
    /// keeps the barrier honest.
    pub fn seed_scene(&mut self, game: u8, scene: u32) {
        let scenes = region_scenes();
        // Did seed_from_visited already root a region of this exact scene? Then the
        // arrival point is known; do not also force the hub (see the doc above).
        let arrival_known = self.seed_regions.iter().any(|&i| {
            data::LOGIC_REGIONS[i as usize].game == game && scenes[i as usize] == Some(scene)
        });
        if !arrival_known {
            let entry = entry_regions();
            let hub = data::LOGIC_REGIONS
                .iter()
                .enumerate()
                .filter(|(i, r)| r.game == game && scenes[*i] == Some(scene) && entry[*i])
                .max_by_key(|(_, r)| r.exits.len())
                .map(|(i, _)| i as u32);
            if let Some(h) = hub {
                if !self.seed_regions.contains(&h) {
                    self.seed_regions.push(h);
                }
            }
        }
        self.visited_scenes.insert((game, scene));
    }
}

/// Per region, whether an entrance can land the player *in* it: it has at least one
/// incoming edge from a region in a different scene (or a scene-less plumbing node).
/// These are the arrival points of a scene; a region reachable only from within its
/// own scene (a gated pond ledge, a bean spot) is not one. [`WorldInputs::seed_scene`]
/// roots the busiest of them (the hub) and lets in-scene rules gate the rest. Static:
/// depends only on the region graph + `region_scenes`.
fn entry_regions() -> &'static [bool] {
    static E: OnceLock<Vec<bool>> = OnceLock::new();
    E.get_or_init(|| {
        let scenes = region_scenes();
        let mut entry = vec![false; data::LOGIC_REGIONS.len()];
        for (j, r) in data::LOGIC_REGIONS.iter().enumerate() {
            for e in r.exits {
                let i = e.to as usize;
                // A cross-scene (or plumbing -> scene) incoming edge marks an arrival.
                if scenes[j] != scenes[i] {
                    entry[i] = true;
                }
            }
        }
        entry
    })
}

/// The set of MM time slices the player can reach: the union of each period's
/// slices (`data::MM_PERIOD_SLICES`) whose reachability rule
/// (`data::MM_CLOCK_PERIOD_EXPRS`) holds. With clock shuffle off every rule holds
/// (they short-circuit on `!setting(clocksMm)`), so this is the full set — fully
/// optimistic, as the tracker was before. Under clock shuffle only the periods
/// whose clock the player owns (per `progressiveClocks` mode) contribute, so a
/// check gated on e.g. `after(NIGHT3_AM_12_00)` needs the Night 3 clock.
fn compute_mm_slices(inp: &WorldInputs) -> u64 {
    let mut mask = 0u64;
    for (i, &expr) in data::MM_CLOCK_PERIOD_EXPRS.iter().enumerate() {
        if crate::logic::solve::eval_settings_only(&data::EXPRS[expr as usize], inp) {
            mask |= data::MM_PERIOD_SLICES[i];
        }
    }
    mask
}

/// Fold every collected upgrade of a tiered item family onto the single **tier
/// counter** the logic tests with `has(<COUNTER>, n)`.
///
/// OoTMM models these families as one counter: each upgrade get-item does
/// `add: [<COUNTER>, tier]` (gi.yml), so the counter's value is the current tier and
/// the logic just tests it (`has_scale_raw`, `has_strength_raw`, `has_ocarina_of_time`,
/// `has_double_magic`, `has_wallet`, `has_hookshot`, …). The tracker inventory instead
/// tallies the raw `find_item_id` ids, and in two cases that misses the counter:
///
/// - **Separate-tier shuffles** place the individual upgrades (Silver/Golden Scale,
///   Silver/Golden Gauntlets, Ocarina of Time, Larger Magic Upgrade, Longshot,
///   Deku Stick/Nut Upgrade + Second Upgrade, Adult's/Giant's/… Wallet), each with its
///   own id ≠ the counter — so the counter never moves and e.g. `has(SCALE, 2)` stays
///   false with a Golden Scale in hand (or `has(STICK_UPGRADE)` false after a lone
///   "Deku Stick Upgrade").
/// - **Wallets specifically** place a "Progressive Wallet (OoT/MM)" whose id
///   (`OOT_PROG_WALLET` / `MM_PROG_WALLET`) also differs from the counter, so even the
///   progressive shuffle misses it.
///
/// The other families' progressive pool item id already *is* the counter id (e.g.
/// `OOT_SCALE` == "Progressive Scale (OoT)"), and the shared counters are progressive-
/// only (`SHARED_SCALE` etc. *are* the shared progressive item), so those are already
/// counted by the naive tally and need no `prog` fixup — only their separate tiers do.
///
/// Tier = max(the counter's own naive count, the progressive-pool count, the highest
/// separate tier owned). Writing it back is monotone (never lowers the counter), so a
/// single pass after the inventory is assembled is safe. The MM hookshot is
/// deliberately absent: its counter carries pickup-count semantics that already line up
/// with the shuffled pool (1 copy short-off, 2 copies short-on), so the naive tally is
/// already correct there.
fn normalize_tier_counters(items: &mut HashMap<u32, u32>) {
    use crate::data::iid;
    let count = |items: &HashMap<u32, u32>, id: u32| items.get(&id).copied().unwrap_or(0);
    // (counter, progressive-pool item [0 = none / same id as the counter],
    //  separate tier members [(item id, tier value)] — the counter's own tier is read
    //  from its naive count, so only tiers with a DIFFERENT id are listed here).
    struct Fam {
        counter: u32,
        prog: u32,
        members: &'static [(u32, u32)],
    }
    let families: &[Fam] = &[
        // Wallets: the progressive pool item id differs from the counter id.
        Fam { counter: iid::OOT_WALLET, prog: iid::OOT_PROG_WALLET, members: &[
            (iid::OOT_WALLET2, 2), (iid::OOT_WALLET3, 3), (iid::OOT_WALLET4, 4), (iid::OOT_WALLET5, 5)] },
        Fam { counter: iid::MM_WALLET, prog: iid::MM_PROG_WALLET, members: &[
            (iid::MM_WALLET2, 2), (iid::MM_WALLET3, 3), (iid::MM_WALLET4, 4), (iid::MM_WALLET5, 5)] },
        // Scale (bronze/silver/gold) — progressive id == counter, only separate tiers listed.
        Fam { counter: iid::OOT_SCALE, prog: 0, members: &[
            (iid::OOT_SCALE_BRONZE, 1), (iid::OOT_SCALE_SILVER, 2), (iid::OOT_SCALE_GOLDEN, 3)] },
        Fam { counter: iid::MM_SCALE, prog: 0, members: &[
            (iid::MM_SCALE_BRONZE, 1), (iid::MM_SCALE_SILVER, 2), (iid::MM_SCALE_GOLDEN, 3)] },
        // Strength (bracelet/silver/golden gauntlets).
        Fam { counter: iid::OOT_STRENGTH, prog: 0, members: &[
            (iid::OOT_GORON_BRACELET, 1), (iid::OOT_SILVER_GAUNTLETS, 2), (iid::OOT_GOLDEN_GAUNTLETS, 3)] },
        Fam { counter: iid::MM_STRENGTH, prog: 0, members: &[
            (iid::MM_GORON_BRACELET, 1), (iid::MM_SILVER_GAUNTLETS, 2), (iid::MM_GOLDEN_GAUNTLETS, 3)] },
        // Ocarina (fairy/of time).
        Fam { counter: iid::OOT_OCARINA, prog: 0, members: &[
            (iid::OOT_OCARINA_FAIRY, 1), (iid::OOT_OCARINA_TIME, 2)] },
        Fam { counter: iid::MM_OCARINA, prog: 0, members: &[
            (iid::MM_OCARINA_FAIRY, 1), (iid::MM_OCARINA_OF_TIME, 2)] },
        // Magic (single/double) — the single upgrade IS the counter id, tier 2 is separate.
        Fam { counter: iid::OOT_MAGIC_UPGRADE, prog: 0, members: &[(iid::OOT_MAGIC_UPGRADE2, 2)] },
        Fam { counter: iid::MM_MAGIC_UPGRADE, prog: 0, members: &[(iid::MM_MAGIC_UPGRADE2, 2)] },
        // OoT hookshot: `has(HOOKSHOT, 2)` = Longshot, whose id differs from the counter.
        Fam { counter: iid::OOT_HOOKSHOT, prog: 0, members: &[(iid::OOT_LONGSHOT, 2)] },
        // Deku stick / nut capacity: each get-item does `add: [<counter>, tier]` (gi.yml),
        // so the counter (the tier-1 "Capacity" id) is what `has(STICK_UPGRADE)` /
        // `has(NUT_UPGRADE)` reads. The shuffle can place the tier-2 "Upgrade" or tier-3
        // "Second Upgrade" get-item, each with its own id ≠ the counter, so a collected
        // upgrade never moved the counter and `has_sticks_capacity` stayed false.
        Fam { counter: iid::OOT_STICK_UPGRADE, prog: 0, members: &[
            (iid::OOT_STICK_UPGRADE2, 2), (iid::OOT_STICK_UPGRADE3, 3)] },
        Fam { counter: iid::MM_STICK_UPGRADE, prog: 0, members: &[
            (iid::MM_STICK_UPGRADE2, 2), (iid::MM_STICK_UPGRADE3, 3)] },
        Fam { counter: iid::OOT_NUT_UPGRADE, prog: 0, members: &[
            (iid::OOT_NUT_UPGRADE2, 2), (iid::OOT_NUT_UPGRADE3, 3)] },
        Fam { counter: iid::MM_NUT_UPGRADE, prog: 0, members: &[
            (iid::MM_NUT_UPGRADE2, 2), (iid::MM_NUT_UPGRADE3, 3)] },
    ];
    for fam in families {
        let mut tier = count(items, fam.counter);
        if fam.prog != 0 {
            tier = tier.max(count(items, fam.prog));
        }
        for &(id, t) in fam.members {
            if count(items, id) > 0 {
                tier = tier.max(t);
            }
        }
        if tier > 0 {
            items.insert(fam.counter, tier);
        }
    }
}

/// Region-name index per game: `name -> region indices` (a name can map to several
/// dungeon-layout variants). Queried with a `&str` since `&'static str: Borrow<str>`.
fn region_name_index() -> &'static [HashMap<&'static str, Vec<u32>>; 2] {
    static I: OnceLock<[HashMap<&'static str, Vec<u32>>; 2]> = OnceLock::new();
    I.get_or_init(|| {
        let mut idx: [HashMap<&'static str, Vec<u32>>; 2] = [HashMap::new(), HashMap::new()];
        for (i, r) in data::LOGIC_REGIONS.iter().enumerate() {
            idx[r.game as usize].entry(r.name).or_default().push(i as u32);
        }
        idx
    })
}

/// Per region index, the scene it belongs to (`Some(scene_id)`), inferred from the
/// first of its locations that resolves in the object table. Regions with no
/// locations — pure plumbing (SPAWN / GLOBAL / warp hubs / lost-woods junctions) and
/// event-only nodes — have no scene (`None`) and are never treated as an entrance
/// boundary (movement through them is free). Static: depends only on `LOGIC_REGIONS`
/// and the object tables. The scene id is game-relative, so callers pair it with the
/// region's own `game` (OoT and MM scene ids overlap).
fn region_scenes() -> &'static [Option<u32>] {
    static S: OnceLock<Vec<Option<u32>>> = OnceLock::new();
    S.get_or_init(|| {
        // Pass 1: a region's scene = the scene of its first location that resolves in
        // the object table.
        let mut scene: Vec<Option<u32>> = data::LOGIC_REGIONS
            .iter()
            .map(|r| {
                let objs = if r.game == 0 { data::OOT_OBJECTS } else { data::MM_OBJECTS };
                r.locations
                    .iter()
                    .find_map(|l| objs.iter().find(|o| o.location == l.loc).map(|o| o.scene as u32))
            })
            .collect();

        // A real overworld `area` (LAKE_HYLIA, HYRULE_FIELD, …) maps to exactly one
        // scene; the pseudo-areas (NONE / ENTRANCE / "" dungeon interiors / EGGS /
        // BUFFER_DELAYED) span many. Learn `(game, area) -> scene` only where it is
        // unambiguous, from the regions that already have a scene.
        let mut area_scene: HashMap<(u8, &str), Option<u32>> = HashMap::new();
        for (i, r) in data::LOGIC_REGIONS.iter().enumerate() {
            if r.area.is_empty() {
                continue;
            }
            if let Some(s) = scene[i] {
                area_scene
                    .entry((r.game, r.area))
                    .and_modify(|e| {
                        if *e != Some(s) {
                            *e = None;
                        }
                    })
                    .or_insert(Some(s));
            }
        }

        // Pass 2: a location-less region (owl-flight spots, the Lost Woods bridge,
        // transition nodes) inherits its area's scene when that is unambiguous. This
        // closes the leak where an entrance routes through a scene-less waypoint. The
        // pseudo-areas resolve to `None` and stay free plumbing (so boss-lair doors
        // and SPAWN/GLOBAL/warp hubs are never mistaken for entrance boundaries).
        for (i, r) in data::LOGIC_REGIONS.iter().enumerate() {
            if scene[i].is_none() && !r.area.is_empty() {
                if let Some(&Some(s)) = area_scene.get(&(r.game, r.area)) {
                    scene[i] = Some(s);
                }
            }
        }

        // Pass 3: the OoT Market complex has no single area scene (its `MARKET` area
        // spans the Market, the Back Alley and the Temple of Time exterior), so its
        // check-less transition nodes stay scene-less and act as free plumbing — which
        // let Market and Hyrule Castle show as reachable in progressive mode the moment
        // Hyrule Field was reached (reported). Pin the two nodes that gate the complex's
        // outer boundaries to the Market's own object scene, so `edge_blocked` treats
        // Hyrule Field -> Market and Market -> Hyrule Castle as the real entrances they
        // are (paired via `canon_entrance_scene`). Market Entryway is the drawbridge
        // side; Market Castle Entry is the castle-door side.
        for (i, r) in data::LOGIC_REGIONS.iter().enumerate() {
            if r.game == 0
                && scene[i].is_none()
                && matches!(r.name, "Market Entryway" | "Market Castle Entry")
            {
                scene[i] = Some(data::scenes::OOT_MARKET_CHILD_DAY as u32);
            }
        }
        scene
    })
}

/// Fold an OoTMM entrance-table scene id onto the object-scene id the tracker keys
/// regions and visited scenes by. OoTMM numbers the Market complex with "meta" scene
/// ids in its entrance table (Market Entrance `0x93`, Market `0x98`) that differ from
/// the object/render scenes the map uses (Market Child Day `0x20`); without this fold
/// the entrance pairs never match the region graph's scenes, so the Market boundary is
/// invisible to `edge_blocked`. Every other scene already agrees across both spaces.
fn canon_entrance_scene(game: u8, s: u32) -> u32 {
    use crate::data::scenes as sc;
    if game == 0 && (s == sc::OOT_MARKET as u32 || s == sc::OOT_MARKET_ENTRANCE as u32) {
        return sc::OOT_MARKET_CHILD_DAY as u32;
    }
    s
}

/// The set of scene transitions that are real, trackable entrances (loading zones),
/// as `(game, from_scene, to_scene)`. Built from the entrance tables, both directions
/// per def. Excludes `EntranceType::None` (logic-internal, never walked) and same-
/// scene defs (wallmaster drops etc.). A cross-scene region edge whose scene pair is
/// NOT in here is not an entrance — a boss-lair door, say — and is never walled.
fn entrance_scene_pairs() -> &'static HashSet<(u8, u32, u32)> {
    static P: OnceLock<HashSet<(u8, u32, u32)>> = OnceLock::new();
    P.get_or_init(|| {
        let mut set = HashSet::new();
        for (game, table) in [(0u8, data::OOT_ENTRANCES), (1u8, data::MM_ENTRANCES)] {
            for e in table {
                if e.type_ == crate::data::EntranceType::None {
                    continue;
                }
                let (a, b) = (canon_entrance_scene(game, e.from_scene as u32),
                              canon_entrance_scene(game, e.to_scene as u32));
                if a == b {
                    continue;
                }
                set.insert((game, a, b));
                set.insert((game, b, a));
            }
        }
        set
    })
}

/// Turn the spoiler's name-based entrance remaps into solver edge redirects: for
/// each `from -> via` vanilla edge in the region graph, redirect it to the shuffled
/// destination region(s). Dungeon destinations resolve to every layout variant; the
/// solver keeps only the active one.
///
/// In `progressive` mode the redirect is only laid down once the player has walked
/// that entrance (`discovered` holds `(game, src_id)`); an undiscovered entrance is
/// mapped to *no* targets, which makes `solve` treat the edge as impassable. Outside
/// progressive mode every remap is applied (full spoiler knowledge, the default).
fn build_exit_redirects(
    remaps: &[crate::settings::EntranceRemap],
    discovered: &HashSet<(u8, u32)>,
    progressive: bool,
) -> HashMap<(u32, u32), Vec<u32>> {
    let names = region_name_index();
    let mut out: HashMap<(u32, u32), Vec<u32>> = HashMap::new();
    for m in remaps {
        let Some(dests) = names[m.dest_game as usize].get(m.dest.as_str()) else { continue };
        let Some(froms) = names[m.game as usize].get(m.from.as_str()) else { continue };
        // Progressive: an entrance the player has not walked yet is a wall (empty
        // targets). An unresolved `src_id` (rare) is treated as known, so we never
        // wrongly hide reachability we cannot attribute to a specific entrance.
        let found = !progressive
            || m.src_id.map_or(true, |id| discovered.contains(&(m.game, id)));
        let targets: &[u32] = if found { dests } else { &[] };
        for &fi in froms {
            for e in data::LOGIC_REGIONS[fi as usize].exits {
                let to = &data::LOGIC_REGIONS[e.to as usize];
                if to.game == m.game && to.name == m.via {
                    out.insert((fi, e.to), targets.to_vec());
                }
            }
        }
    }
    out
}

/// Owned-item tally for one `special(X)` category, or `None` when the tracker
/// does not yet model it (the mask categories) — the caller then stays optimistic.
fn special_cat_count(cat: &str, items: &HashMap<u32, u32>) -> Option<u32> {
    use crate::data::iid;
    let sum = |ids: &[u32]| -> u32 {
        ids.iter().map(|i| items.get(i).copied().unwrap_or(0)).sum()
    };
    Some(match cat {
        "medallions" => sum(&[
            iid::OOT_MEDALLION_FOREST, iid::OOT_MEDALLION_FIRE, iid::OOT_MEDALLION_WATER,
            iid::OOT_MEDALLION_SPIRIT, iid::OOT_MEDALLION_SHADOW, iid::OOT_MEDALLION_LIGHT,
        ]),
        "stones" => sum(&[iid::OOT_STONE_EMERALD, iid::OOT_STONE_RUBY, iid::OOT_STONE_SAPPHIRE]),
        "remains" => sum(&[
            iid::MM_REMAINS_ODOLWA, iid::MM_REMAINS_GOHT,
            iid::MM_REMAINS_GYORG, iid::MM_REMAINS_TWINMOLD,
        ]),
        "skullsGold" => sum(&[iid::OOT_GS_TOKEN]),
        "skullsSwamp" => sum(&[iid::MM_GS_TOKEN_SWAMP]),
        "skullsOcean" => sum(&[iid::MM_GS_TOKEN_OCEAN]),
        // The generic MM_STRAY_FAIRY is folded in alongside each dungeon id so a
        // seed that shuffles fairies generically is never under-counted.
        "fairiesWF" => sum(&[iid::MM_STRAY_FAIRY_WF, iid::MM_STRAY_FAIRY]),
        "fairiesSH" => sum(&[iid::MM_STRAY_FAIRY_SH, iid::MM_STRAY_FAIRY]),
        "fairiesGB" => sum(&[iid::MM_STRAY_FAIRY_GB, iid::MM_STRAY_FAIRY]),
        "fairiesST" => sum(&[iid::MM_STRAY_FAIRY_ST, iid::MM_STRAY_FAIRY]),
        "fairyTown" => sum(&[iid::MM_STRAY_FAIRY_TOWN, iid::MM_STRAY_FAIRY]),
        "triforce" => sum(&[iid::OOT_TRIFORCE, iid::SHARED_TRIFORCE]),
        "coinsRed" => sum(&[iid::OOT_COIN_RED]),
        "coinsGreen" => sum(&[iid::OOT_COIN_GREEN]),
        "coinsBlue" => sum(&[iid::OOT_COIN_BLUE]),
        "coinsYellow" => sum(&[iid::OOT_COIN_YELLOW]),
        // Mask categories need a per-mask class map (not modelled yet) and unknown
        // categories: report "uncomputable" so `special` stays optimistic.
        _ => return None,
    })
}

impl Inputs for WorldInputs {
    fn item_count(&self, id: u32) -> u32 {
        self.items.get(&id).copied().unwrap_or(0)
    }
    fn var_count(&self, var: u8) -> u16 {
        self.var_counts.get(var as usize).copied().unwrap_or_else(|| {
            crate::data::VAR_DEFAULTS.get(var as usize).copied().unwrap_or(1)
        })
    }
    fn setting_value(&self, key: u32) -> Option<u32> {
        self.settings_value.get(&key).copied()
    }
    fn setting_has(&self, key: u32, val: u32) -> bool {
        self.settings_value.get(&key) == Some(&val)
            || self.settings_multi.get(&key).is_some_and(|s| s.contains(&val))
    }
    fn setting_enabled(&self, key: u32) -> bool {
        self.settings_enabled.contains(&key)
    }
    fn layout_active(&self, _layout: GameLayout) -> bool {
        // Explore every variant; the object-layout filter renders only the seed's
        // real one and the variants use disjoint locations (see module docs).
        true
    }
    fn mask_count(&self) -> u16 {
        self.masks
    }
    fn trick(&self, id: u32) -> bool {
        self.tricks.contains(&id)
    }
    fn special(&self, id: u32) -> bool {
        // `special(X)` = owned items across the enabled categories >= count. It is
        // only ever reached when the matching setting is `custom`; a missing cond
        // (non-custom seed) or an uncomputable category stays optimistic (true) so
        // we never hide an endgame check we cannot be sure is blocked.
        let Some(&name) = data::SPECIAL_NAMES.get(id as usize) else { return true };
        let Some(cond) = self.special_conds.get(name) else { return true };
        let mut tally = 0u32;
        for cat in &cond.cats {
            match special_cat_count(cat, &self.items) {
                Some(n) => tally += n,
                None => return true,
            }
        }
        tally >= cond.count
    }
    fn song_event(&self, game: u8, slot: u8, song: u8) -> bool {
        match self.song_events.get(game as usize).and_then(|v| v.get(slot as usize)) {
            // Exact placement match, or an unresolved name (u8::MAX) kept optimistic.
            Some(&n) => n == song || n == u8::MAX,
            // No song-event data (section absent / slot unknown) → optimistic.
            None => true,
        }
    }
    fn exit_redirects(&self) -> Option<&HashMap<(u32, u32), Vec<u32>>> {
        (!self.exit_redirects.is_empty()).then_some(&self.exit_redirects)
    }
    fn extra_seed_regions(&self) -> &[u32] {
        &self.seed_regions
    }
    fn mm_time_slices(&self) -> u64 {
        self.mm_slices
    }
    fn edge_blocked(&self, from: u32, to: u32) -> bool {
        // Full-knowledge mode: every vanilla edge is open.
        if !self.progressive {
            return false;
        }
        let regions = data::LOGIC_REGIONS;
        let (rf, rt) = (&regions[from as usize], &regions[to as usize]);
        // Cross-game edges (the OoTMM portal) and any edge touching a scene-less
        // plumbing region (SPAWN / GLOBAL / warps / junctions / events) are not a
        // walkable loading zone we gate — leave them to the access rule.
        if rf.game != rt.game {
            return false;
        }
        let scenes = region_scenes();
        let (Some(sa), Some(sb)) = (scenes[from as usize], scenes[to as usize]) else {
            return false;
        };
        // Same scene → intra-scene movement, always free.
        if sa == sb {
            return false;
        }
        // A cross-scene edge that is not a real entrance (e.g. a dungeon → boss-lair
        // door) is free — crossing it is implied by reaching the source. A real
        // entrance may only be crossed *into* a scene the player has discovered;
        // stepping into an undiscovered scene is walled.
        if !entrance_scene_pairs().contains(&(rf.game, sa, sb)) {
            return false;
        }
        !self.visited_scenes.contains(&(rt.game, sb))
    }
}

// ── Reverse index tables (name/id -> compiled index), built once ─────────────
fn build_index(names: &'static [&'static str]) -> HashMap<&'static str, u32> {
    names.iter().enumerate().map(|(i, &n)| (n, i as u32)).collect()
}

fn setting_key_index() -> &'static HashMap<&'static str, u32> {
    static I: OnceLock<HashMap<&'static str, u32>> = OnceLock::new();
    I.get_or_init(|| build_index(data::SETTING_KEYS))
}

fn setting_value_index() -> &'static HashMap<&'static str, u32> {
    static I: OnceLock<HashMap<&'static str, u32>> = OnceLock::new();
    I.get_or_init(|| build_index(data::SETTING_VALUES))
}

fn trick_index() -> &'static HashMap<&'static str, u32> {
    static I: OnceLock<HashMap<&'static str, u32>> = OnceLock::new();
    I.get_or_init(|| build_index(data::TRICK_NAMES))
}

/// Item ids whose display name mentions a mask, for the `masks(n)` count.
fn mask_item_ids() -> &'static HashSet<u32> {
    static I: OnceLock<HashSet<u32>> = OnceLock::new();
    I.get_or_init(|| {
        data::ITEMS
            .iter()
            .filter(|d| d.name.contains("Mask"))
            .map(|d| d.id)
            .collect()
    })
}

#[cfg(test)]
mod tests {
    use super::*;


    /// Object index of a location in a game's object table (test helper).
    fn obj_idx(game: Game, location: &str) -> usize {
        let objs = match game {
            Game::Oot => data::OOT_OBJECTS,
            Game::Mm => data::MM_OBJECTS,
        };
        objs.iter().position(|o| o.location == location).expect("known location")
    }

    /// A collected Kokiri Sword check turns into +1 of the sword's item id, and
    /// the seed settings / tricks resolve to their compiled indices.
    #[test]
    fn build_resolves_inventory_settings_and_tricks() {
        let mq = std::collections::HashSet::new();
        let mut settings = Settings::default();
        settings.parse_spoiler(
            "Settings\n  startingAge: child\n  doorOfTime: open\n\
             Tricks\n  Backflip Over Mido\n",
            &mq,
        );
        settings.apply(&mq);

        let loc = "OOT Kokiri Forest Kokiri Sword Chest";
        let mut world = WorldData::default();
        world.items.insert(loc.to_string(), "Kokiri Sword (OoT)".to_string());
        world.collected.insert((Game::Oot, obj_idx(Game::Oot, loc)));

        let inp = WorldInputs::build(&settings, std::slice::from_ref(&world), 1, &Default::default(), false);

        // The placed sword resolved to its dev id and was counted.
        let sword = find_item_id("Kokiri Sword (OoT)").expect("sword id");
        assert_eq!(inp.item_count(sword), 1);

        // startingAge=child / doorOfTime=open resolved to setting(k, v) values.
        let sk = setting_key_index();
        let sv = setting_value_index();
        if let (Some(&k), Some(&v)) = (sk.get("doorOfTime"), sv.get("open")) {
            assert_eq!(inp.setting_value(k), Some(v), "doorOfTime should map to open");
        }

        // The Mido-skip trick resolved to a real TRICK_NAMES index.
        let ti = trick_index();
        let mido = ti.get("OOT_MIDO_SKIP").copied().expect("trick indexed");
        assert!(inp.trick(mido), "Backflip Over Mido should be enabled");
    }

    /// A separate-tier stick/nut upgrade folds onto the logic counter the macros read.
    /// `has_sticks_capacity` tests `has(STICK_UPGRADE)`, i.e. the tier-1 counter id, but
    /// the shuffle places the tier-2 "Deku Stick Upgrade" (a different id), so without the
    /// fold the counter stayed 0 and sticks read as unusable. The fold lifts the counter
    /// to the owned tier; the shared counter (progressive-only) is untouched.
    #[test]
    fn stick_nut_upgrades_fold_onto_the_logic_counter() {
        use crate::data::iid;
        let mut items: HashMap<u32, u32> = HashMap::new();
        items.insert(iid::OOT_STICK_UPGRADE2, 1); // tier-2 "Deku Stick Upgrade (OoT)"
        items.insert(iid::MM_NUT_UPGRADE3, 1); // tier-3 "Second Deku Nut Upgrade (MM)"
        normalize_tier_counters(&mut items);
        assert_eq!(items.get(&iid::OOT_STICK_UPGRADE).copied(), Some(2), "stick counter lifted to tier 2");
        assert_eq!(items.get(&iid::MM_NUT_UPGRADE).copied(), Some(3), "nut counter lifted to tier 3");
        assert_eq!(items.get(&iid::SHARED_STICK_UPGRADE).copied().unwrap_or(0), 0, "shared counter untouched");
    }

    /// A set-valued setting (`openDungeonsOot`) parses its comma-separated members so
    /// `setting(k, member)` holds for each selected dungeon and no other. This is the
    /// path the manual ROM editor and a list-style spoiler value both feed.
    #[test]
    fn set_setting_resolves_each_member() {
        let mq = std::collections::HashSet::new();
        let mut settings = Settings::default();
        // The editor writes the chosen members comma-joined into raw_settings.
        settings.raw_settings.insert("openDungeonsOot".to_string(), "DC, Water".to_string());
        settings.apply(&mq);
        let inp = WorldInputs::build(&settings, &[WorldData::default()], 1, &Default::default(), false);

        let sk = setting_key_index();
        let sv = setting_value_index();
        let k = *sk.get("openDungeonsOot").expect("openDungeonsOot is a logic key");
        let member = |name: &str| *sv.get(name).expect("known member value");
        assert!(inp.setting_has(k, member("DC")), "DC is in the open set");
        assert!(inp.setting_has(k, member("Water")), "Water is in the open set");
        assert!(!inp.setting_has(k, member("JJ")), "JJ was not selected");
    }

    /// MM time under clock shuffle: `mm_time_slices` narrows to the day/night
    /// periods whose clock the player owns, so a check gated on
    /// `after(NIGHT3_AM_12_00)` needs the Night 3 clock rather than merely *some*
    /// clock (the reported over-permissive bug). With clock shuffle off it stays
    /// fully optimistic (every slice reachable), as the tracker was before.
    #[test]
    fn mm_time_slices_track_clock_shuffle() {
        use crate::data::iid;
        let mq = std::collections::HashSet::new();
        let all = data::MM_PERIOD_SLICES.iter().fold(0u64, |a, &m| a | m);
        let day1 = data::MM_PERIOD_SLICES[0];
        let night3 = data::MM_PERIOD_SLICES[5];

        let build = |clocks_mm: bool, mode: &str, owned: &[(u32, u32)]| {
            let mut s = Settings::default();
            if clocks_mm {
                s.raw_settings.insert("clocksMm".into(), "true".into());
                s.raw_settings.insert("progressiveClocks".into(), mode.into());
            }
            s.apply(&mq);
            for &(id, n) in owned {
                s.starting_item_ids.insert(id, n);
            }
            WorldInputs::build(&s, &[WorldData::default()], 1, &Default::default(), false)
        };

        // Clock shuffle OFF -> every slice reachable (optimistic, as before).
        assert_eq!(build(false, "", &[]).mm_time_slices(), all);

        // Separate mode, only the Night 3 clock (CLOCK6): night3 reachable, day1
        // is not (it needs CLOCK1) -> `after(NIGHT3_AM_12_00)` holds, day-1 checks
        // do not.
        let sep_n3 = build(true, "separate", &[(iid::MM_CLOCK6, 1)]);
        assert_eq!(sep_n3.mm_time_slices() & night3, night3, "night3 unlocked by CLOCK6");
        assert_eq!(sep_n3.mm_time_slices() & day1, 0, "day1 still needs CLOCK1");

        // Ascending mode: day1 is free (has(CLOCK, 0)), night3 needs 5 clocks.
        let asc0 = build(true, "ascending", &[]);
        assert_eq!(asc0.mm_time_slices() & day1, day1, "day1 free in ascending");
        assert_eq!(asc0.mm_time_slices() & night3, 0, "night3 needs 5 clocks (had 0)");
        let asc5 = build(true, "ascending", &[(iid::MM_CLOCK, 5)]);
        assert_eq!(asc5.mm_time_slices() & night3, night3, "5 clocks reach night3");
    }

    /// Stray-fairy Great Fairy rewards gate on `has(item, var(STRAY_FAIRY_COUNT))`.
    /// The count is `strayFairyRewardCount` (default 15), NOT 1 (the old bug where
    /// a single fairy lit the Great Fairy). It also tracks a lowered seed setting.
    #[test]
    fn stray_fairy_reward_needs_the_full_count() {
        use crate::data::{iid, Op};
        let mq = std::collections::HashSet::new();
        let st = iid::MM_STRAY_FAIRY_ST; // Ikana / Stone Tower (the "Canyon" fairy)
        let rule = [Op::HasVar(st, 0)]; // sf_stone_tower's `has(item, var(...))` half

        // Default seed: threshold is 15. 7 fairies is not enough; 15 is.
        let build = |setting: Option<&str>, owned: u32| {
            let mut s = Settings::default();
            if let Some(v) = setting {
                s.raw_settings.insert("strayFairyRewardCount".into(), v.into());
            }
            s.apply(&mq);
            s.starting_item_ids.insert(st, owned);
            WorldInputs::build(&s, &[WorldData::default()], 1, &Default::default(), false)
        };

        let def7 = build(None, 7);
        assert_eq!(def7.var_count(0), 15, "default stray fairy count is 15");
        assert!(!crate::logic::solve::eval_settings_only(&rule, &def7), "7 < 15");
        assert!(crate::logic::solve::eval_settings_only(&rule, &build(None, 15)), "15 reaches");

        // A seed that lowers the requirement to 5: 7 fairies now satisfies it.
        let low = build(Some("5"), 7);
        assert_eq!(low.var_count(0), 5, "reads the lowered setting");
        assert!(crate::logic::solve::eval_settings_only(&rule, &low), "7 >= 5");
    }

    /// `song_event` reflects the spoiler's slot->song map: the exact placement
    /// matches, other songs at that slot do not, and unknown slots stay optimistic.
    #[test]
    fn song_event_reflects_spoiler_placement() {
        let mq = std::collections::HashSet::new();
        let mut settings = Settings::default();
        settings.parse_spoiler(
            "Song Events\n  Ocarina of Time\n\
             \x20\x20\x20\x20SLOT0 : Prelude of Light\n\
             \x20\x20\x20\x20SLOT1 : Song of Time\n",
            &mq,
        );
        settings.apply(&mq);
        let inp = WorldInputs::build(&settings, &[WorldData::default()], 1, &Default::default(), false);

        // OoT slot 0 holds Prelude of Light (song 11): only song 11 matches.
        assert!(inp.song_event(0, 0, 11));
        assert!(!inp.song_event(0, 0, 5));
        // OoT slot 1 holds Song of Time (song 5).
        assert!(inp.song_event(0, 1, 5));
        assert!(!inp.song_event(0, 1, 11));
        // A slot with no data (beyond the list, or MM which was not listed) is
        // optimistic (true) so nothing is hidden for lack of a map.
        assert!(inp.song_event(0, 2, 3));
        assert!(inp.song_event(1, 0, 0));
    }

    /// The spoiler `Entrances` section parses (handling region names that contain
    /// ` to `/` from `), and its remaps become solver edge redirects: the vanilla
    /// `from -> via` edge is rerouted to the shuffled destination, and reachability
    /// follows the new target instead of the old one.
    #[test]
    fn entrance_rando_redirects_the_edge() {
        let mq = std::collections::HashSet::new();
        let mut settings = Settings::default();
        // Reroute the Kokiri Forest -> Lost Woods overworld edge to Zora River.
        settings.parse_spoiler(
            "Entrances\n  OOT Kokiri Forest to OOT Lost Woods (OOT_LW) \
             -> OOT Zora River from OOT Hyrule Field (OOT_ZORA_RIVER)\n",
            &mq,
        );
        settings.apply(&mq);
        assert_eq!(settings.entrance_remap.len(), 1);
        let m = &settings.entrance_remap[0];
        assert_eq!((m.game, m.from.as_str(), m.via.as_str()), (0, "Kokiri Forest", "Lost Woods"));
        assert_eq!((m.dest_game, m.dest.as_str()), (0, "Zora River"));

        // The (Kokiri Forest -> Lost Woods) edge now redirects to Zora River.
        let idx = |g: u8, n: &str| region_name_index()[g as usize].get(n).unwrap()[0];
        let (kokiri, lost, zora) =
            (idx(0, "Kokiri Forest"), idx(0, "Lost Woods"), idx(0, "Zora River"));
        let inp = WorldInputs::build(&settings, &[WorldData::default()], 1, &Default::default(), false);
        let red = inp.exit_redirects().expect("redirects present");
        assert_eq!(red.get(&(kokiri, lost)).map(|v| v.contains(&zora)), Some(true));

        // A no-entrance seed leaves the redirect map empty (vanilla graph).
        let mut vanilla = Settings::default();
        vanilla.parse_spoiler("Settings\n  startingAge: child\n", &mq);
        vanilla.apply(&mq);
        assert!(WorldInputs::build(&vanilla, &[WorldData::default()], 1, &Default::default(), false).exit_redirects().is_none());
    }

    /// Progressive entrance discovery: the same rerouted edge is a *wall* until the
    /// player walks it. With `progressive=true` and nothing discovered the redirect
    /// carries no targets (impassable); once the source entrance id is in the
    /// discovered set the redirect is laid down exactly as in full-knowledge mode.
    #[test]
    fn progressive_entrances_gate_until_walked() {
        let mq = std::collections::HashSet::new();
        let mut settings = Settings::default();
        // A real SRC entrance const (`..._ENTR` dropped, as the spoiler writes it)
        // on the Kokiri Forest -> Lost Woods edge, rerouted to Zora River.
        settings.parse_spoiler(
            "Entrances\n  OOT Kokiri Forest to OOT Lost Woods (OOT_KOKIRI_FOREST_FROM_LOST_WOODS) \
             -> OOT Zora River from OOT Hyrule Field (OOT_ZORA_RIVER)\n",
            &mq,
        );
        settings.apply(&mq);

        // The SRC entrance const resolved to a real id (the name<->id bridge).
        let src_id = settings.entrance_remap[0].src_id.expect("SRC entrance resolved to an id");

        let idx = |g: u8, n: &str| region_name_index()[g as usize].get(n).unwrap()[0];
        let (kokiri, lost, zora) =
            (idx(0, "Kokiri Forest"), idx(0, "Lost Woods"), idx(0, "Zora River"));
        let edge = (kokiri, lost);

        let empty = std::collections::HashSet::new();
        let walked: std::collections::HashSet<(u8, u32)> = [(0u8, src_id)].into_iter().collect();

        // Targets this edge is redirected to (empty when the edge is absent or walled).
        let targets = |inp: &WorldInputs| -> Vec<u32> {
            inp.exit_redirects().and_then(|m| m.get(&edge)).cloned().unwrap_or_default()
        };

        // Full knowledge (progressive off): rerouted to Zora River, ignoring `discovered`.
        let full = WorldInputs::build(&settings, &[WorldData::default()], 1, &empty, false);
        assert!(targets(&full).contains(&zora), "full knowledge should reroute to Zora River");

        // Progressive, entrance not yet walked: the edge carries no targets (a wall),
        // so the shuffled Zora River destination is unreachable through it.
        let blocked = WorldInputs::build(&settings, &[WorldData::default()], 1, &empty, true);
        assert!(targets(&blocked).is_empty(), "undiscovered entrance must be a wall");

        // Progressive, entrance walked: identical to full knowledge.
        let opened = WorldInputs::build(&settings, &[WorldData::default()], 1, &walked, true);
        assert!(targets(&opened).contains(&zora), "walked entrance reroutes like full knowledge");
    }

    /// Regression for the cross-game warp-song discovery bug (#3): an OoT warp song
    /// lands in `out_links` under its synthetic song-node id (`OOT_MINUET_OF_FOREST_SONG`),
    /// but the spoiler keys the shuffled warp by its real entrance (`OOT_WARP_SONG_MEADOW`).
    /// Feeding the raw synthetic id to progressive discovery never matched the remap, so
    /// a walked warp song stayed a wall. `canonical_discovered_entrance` bridges the two;
    /// this proves the bridged id opens the destination while the raw synthetic id does not.
    #[test]
    fn walked_oot_warp_song_opens_its_shuffled_destination() {
        use crate::data::entr as e;
        use crate::entrance::canonical_discovered_entrance;

        let mq = std::collections::HashSet::new();
        let mut settings = Settings::default();
        // The Minuet warp (SONG_TP_FOREST -> Sacred Meadow, src OOT_WARP_SONG_MEADOW)
        // rerouted so playing it leads to Zora River instead.
        settings.parse_spoiler(
            "Entrances\n  OOT SONG_TP_FOREST to OOT Sacred Meadow (OOT_WARP_SONG_MEADOW) \
             -> OOT Zora River from OOT Hyrule Field (OOT_ZORA_RIVER)\n",
            &mq,
        );
        settings.apply(&mq);
        // The spoiler resolves the warp song to its REAL entrance id (0x600), not the
        // synthetic song-node id (0xfff00) the runtime stores.
        assert_eq!(settings.entrance_remap[0].src_id, Some(e::OOT_WARP_SONG_MEADOW_ENTR));

        let idx = |g: u8, n: &str| region_name_index()[g as usize].get(n).unwrap()[0];
        let (song, meadow, zora) =
            (idx(0, "SONG_TP_FOREST"), idx(0, "Sacred Meadow"), idx(0, "Zora River"));
        let edge = (song, meadow);
        let targets = |inp: &WorldInputs| -> Vec<u32> {
            inp.exit_redirects().and_then(|m| m.get(&edge)).cloned().unwrap_or_default()
        };

        // What the entrance follow actually stores when the player plays the Minuet.
        let raw_node = (0u8, e::OOT_MINUET_OF_FOREST_SONG);
        let bridged = (0u8, canonical_discovered_entrance(Game::Oot, e::OOT_MINUET_OF_FOREST_SONG));

        // Progressive, discovered set holding only the RAW synthetic node id: still a wall
        // (this is the bug — the id never matches the remap's real src id).
        let raw_only: std::collections::HashSet<(u8, u32)> = [raw_node].into_iter().collect();
        let blocked = WorldInputs::build(&settings, &[WorldData::default()], 1, &raw_only, true);
        assert!(
            targets(&blocked).is_empty(),
            "the synthetic song-node id alone must not open the warp (it is the bug)"
        );

        // Progressive, discovered set bridged as `recompute_reachability` does it: opens.
        let walked: std::collections::HashSet<(u8, u32)> = [bridged].into_iter().collect();
        let opened = WorldInputs::build(&settings, &[WorldData::default()], 1, &walked, true);
        assert!(
            targets(&opened).contains(&zora),
            "a walked OoT warp song, bridged to its real entrance, reroutes to Zora River"
        );
    }

    /// Regression for the ordinary-entrance progressive-discovery bug: a Normal
    /// entrance lands in `out_links` under the def's *return-trip* `from_id`, but the
    /// spoiler keys the shuffled entrance by the FORWARD `to_id` the player walked
    /// into. Feeding the raw stored id to progressive discovery never matched the
    /// remap, so a walked entrance stayed a permanent wall (no maps ever opened).
    /// `canonical_discovered_entrance` bridges the return id back to the forward id;
    /// this proves the bridged id opens the destination while the raw stored id does not.
    #[test]
    fn walked_normal_entrance_opens_its_shuffled_destination() {
        use crate::data::entr as e;
        use crate::entrance::canonical_discovered_entrance;

        let mq = std::collections::HashSet::new();
        let mut settings = Settings::default();
        // The vanilla Kokiri Forest -> Lost Woods edge (forward entrance
        // OOT_LOST_WOODS_FROM_KOKIRI_FOREST) rerouted to Zora River.
        settings.parse_spoiler(
            "Entrances\n  OOT Kokiri Forest to OOT Lost Woods (OOT_LOST_WOODS_FROM_KOKIRI_FOREST) \
             -> OOT Zora River from OOT Hyrule Field (OOT_ZORA_RIVER)\n",
            &mq,
        );
        settings.apply(&mq);
        // The spoiler keys the entrance by the forward id the player walks into.
        assert_eq!(settings.entrance_remap[0].src_id, Some(e::OOT_LOST_WOODS_FROM_KOKIRI_FOREST_ENTR));

        let idx = |g: u8, n: &str| region_name_index()[g as usize].get(n).unwrap()[0];
        let (kokiri, lost, zora) =
            (idx(0, "Kokiri Forest"), idx(0, "Lost Woods"), idx(0, "Zora River"));
        let edge = (kokiri, lost);
        let targets = |inp: &WorldInputs| -> Vec<u32> {
            inp.exit_redirects().and_then(|m| m.get(&edge)).cloned().unwrap_or_default()
        };

        // What the entrance follow actually stores when the player walks the entrance:
        // the def's return-trip `from_id`, NOT the forward id the spoiler keys by.
        let raw_stored = (0u8, e::OOT_KOKIRI_FOREST_FROM_LOST_WOODS_ENTR);
        let bridged = (0u8, canonical_discovered_entrance(Game::Oot, e::OOT_KOKIRI_FOREST_FROM_LOST_WOODS_ENTR));

        // Progressive, discovered set holding only the RAW stored return id: still a
        // wall (this is the bug — the return id never matches the remap's forward src).
        let raw_only: std::collections::HashSet<(u8, u32)> = [raw_stored].into_iter().collect();
        let blocked = WorldInputs::build(&settings, &[WorldData::default()], 1, &raw_only, true);
        assert!(
            targets(&blocked).is_empty(),
            "the raw stored return id alone must not open the entrance (it is the bug)"
        );

        // Progressive, discovered set bridged as `recompute_reachability` does it: opens.
        let walked: std::collections::HashSet<(u8, u32)> = [bridged].into_iter().collect();
        let opened = WorldInputs::build(&settings, &[WorldData::default()], 1, &walked, true);
        assert!(
            targets(&opened).contains(&zora),
            "a walked Normal entrance, bridged to its forward id, reroutes to Zora River"
        );
    }

    /// Progressive seeding: `seed_from_visited` turns the entrance ids the player has
    /// actually walked (both `out_links` endpoints) into logic-region roots via each
    /// entrance's own `to_name`. Walking Graveyard <-> Kakariko must root exactly the
    /// "Graveyard" and "Kakariko" regions, so those scenes are reachable even with no
    /// discovered chain back to SPAWN (the reported bug: physically there, nothing shown).
    #[test]
    fn seed_from_visited_roots_the_players_regions() {
        use crate::data::entr as e;

        let mq = std::collections::HashSet::new();
        let mut settings = Settings::default();
        settings.apply(&mq);
        let mut inp = WorldInputs::build(&settings, &[WorldData::default()], 1, &Default::default(), true);
        assert!(inp.extra_seed_regions().is_empty(), "no roots before seeding");

        // Both endpoints of the discovered Graveyard <-> Kakariko link, exactly as
        // `recompute_reachability` gathers them from `out_links` (keys + values).
        let visited: std::collections::HashSet<(u8, u32)> = [
            (0u8, e::OOT_GRAVEYARD_ENTR),            // to_name "Graveyard"
            (0u8, e::OOT_KAKARIKO_FROM_GRAVEYARD_ENTR), // to_name "Kakariko"
        ]
        .into_iter()
        .collect();
        inp.seed_from_visited(&visited);

        let gy = region_name_index()[0].get("Graveyard").expect("Graveyard region")[0];
        let kak = region_name_index()[0].get("Kakariko").expect("Kakariko region")[0];
        assert!(inp.extra_seed_regions().contains(&gy), "Graveyard must be a seeded root");
        assert!(inp.extra_seed_regions().contains(&kak), "Kakariko must be a seeded root");

        // The visited SCENES follow from those regions: Graveyard and Kakariko, and
        // nothing else — this is the set the entrance wall lets you cross *into*.
        let scenes = region_scenes();
        let (gy_s, kak_s) = (scenes[gy as usize].unwrap(), scenes[kak as usize].unwrap());
        assert!(inp.visited_scenes.contains(&(0, gy_s)));
        assert!(inp.visited_scenes.contains(&(0, kak_s)));
    }

    /// Region -> scene inference (`region_scenes`). A region with checks resolves to
    /// its object scene; a *location-less* overworld waypoint (owl-flight spot, the
    /// Lost Woods bridge) inherits its `area`'s scene so an entrance routed through it
    /// is still gated; a pseudo-area plumbing node (SPAWN / GLOBAL) stays `None`; and a
    /// boss lair is a distinct scene from its dungeon (so a scene gate must NOT wall
    /// the dungeon -> boss door — that is what `entrance_scene_pairs` is for).
    #[test]
    fn region_scene_inference_covers_locationless_waypoints() {
        let scenes = region_scenes();
        let idx = |n: &str| region_name_index()[0].get(n).map(|v| v[0]);
        let sc = |n: &str| idx(n).and_then(|i| scenes[i as usize]);

        // Located overworld regions resolve to their own scene.
        assert_eq!(sc("Lake Hylia"), Some(data::scenes::OOT_LAKE_HYLIA as u32));
        assert_eq!(sc("Kokiri Forest"), Some(data::scenes::OOT_KOKIRI_FOREST as u32));

        // A location-less waypoint inside Lake Hylia inherits the Lake Hylia scene.
        if let Some(owl) = sc("Lake Hylia Owl Flight") {
            assert_eq!(owl, data::scenes::OOT_LAKE_HYLIA as u32,
                "owl-flight waypoint must inherit its area's scene, not stay None");
        }

        // Plumbing nodes have no scene (never an entrance boundary).
        assert_eq!(sc("SPAWN"), None);
        assert_eq!(sc("GLOBAL"), None);

        // A boss lair is its own scene, distinct from the dungeon it sits in.
        let deku = sc("Deku Tree Before Boss");
        let boss = sc("Deku Tree Boss");
        assert!(deku.is_some() && boss.is_some() && deku != boss,
            "boss lair should be a separate scene from the dungeon");
    }

    /// `entrance_scene_pairs` marks exactly the scene transitions that carry a real
    /// `EntranceDef` (both directions), and excludes the non-walkable `None`-type and
    /// same-scene defs. A Kokiri Forest <-> Link's House door is in; a scene pair with
    /// no def at all (Lake Hylia <-> Deku Tree — never adjacent) is out.
    /// Progressive discovery must wall the OoT Market complex behind its real
    /// entrances even though the Market's transition nodes carry no checks (and so
    /// resolved to no scene) and OoTMM numbers the Market with a different scene id in
    /// its entrance table than the map uses. Regression: standing in Hyrule Field lit
    /// up every Market and Hyrule Castle check although neither had been visited.
    #[test]
    fn progressive_market_and_castle_gated_behind_their_entrances() {
        let mq = std::collections::HashSet::new();
        let mut settings = Settings::default();
        settings.raw_settings.insert("startingAgeOot".into(), "child".into());
        settings.apply(&mq);
        let hf = data::scenes::OOT_HYRULE_FIELD as u32;
        let market = data::scenes::OOT_MARKET_CHILD_DAY as u32;

        // The Market node and its castle-door node now carry the Market scene, and the
        // entrance pairs fold onto it, so both boundaries are real entrances.
        let scenes = region_scenes();
        let idx = |n: &str| region_name_index()[0].get(n).unwrap()[0] as usize;
        assert_eq!(scenes[idx("Market Entryway")], Some(market), "Market Entryway is pinned to the Market scene");
        assert_eq!(scenes[idx("Market Castle Entry")], Some(market));
        assert!(entrance_scene_pairs().contains(&(0, hf, market)), "Hyrule Field <-> Market is a (folded) entrance");
        assert!(entrance_scene_pairs().contains(&(0, market, data::scenes::OOT_HYRULE_CASTLE as u32)));

        // Standing only in Hyrule Field: Market and Castle stay hidden.
        let mut inp = WorldInputs::build(&settings, &[WorldData::default()], 1, &Default::default(), true);
        inp.seed_scene(0, hf);
        let r = crate::logic::solve::solve(&inp);
        assert!(r.reachable("OOT Hyrule Field Tree 01"), "the visited scene itself is reachable");
        assert!(!r.reachable("OOT Market Crate 1"), "Market must stay hidden until visited");
        assert!(!r.reachable("OOT Hyrule Castle Pot 1"), "Hyrule Castle must stay hidden until visited");

        // After walking into the Market, its checks open — but Hyrule Castle, one more
        // entrance away, stays gated.
        inp.visited_scenes.insert((0, market));
        inp.seed_scene(0, market);
        let r2 = crate::logic::solve::solve(&inp);
        assert!(r2.reachable("OOT Market Crate 1"), "the Market opens once walked into");
        assert!(!r2.reachable("OOT Hyrule Castle Pot 1"), "Hyrule Castle is still a further undiscovered entrance");
    }

    #[test]
    fn entrance_scene_pairs_marks_real_entrances() {
        let p = entrance_scene_pairs();
        let (links, kokiri) = (data::scenes::OOT_LINK_HOUSE as u32, data::scenes::OOT_KOKIRI_FOREST as u32);
        assert!(p.contains(&(0, kokiri, links)), "Kokiri <-> Link's House is a real entrance");
        assert!(p.contains(&(0, links, kokiri)), "stored both directions");

        // The boss-lair door IS a trackable entrance (`One_Way_In` Deku Tree ->
        // Gohma's Lair), so in progressive mode the boss room is discovered by
        // entering it — its scene pair must be present.
        let (deku, gohma) = (data::scenes::OOT_DEKU_TREE as u32, data::scenes::OOT_LAIR_GOHMA as u32);
        assert!(p.contains(&(0, deku, gohma)), "Deku Tree -> boss lair is a real (one-way) entrance");

        // A pair with no EntranceDef between the two scenes is absent.
        let lake = data::scenes::OOT_LAKE_HYLIA as u32;
        assert!(!p.contains(&(0, lake, gohma)), "Lake Hylia and Gohma's Lair share no entrance");
    }

    /// The core progressive wall (`edge_blocked`): with only Lake Hylia discovered,
    /// crossing a real entrance *into* an undiscovered scene (Hyrule Field) is walled,
    /// while intra-scene movement, boss-lair doors, and edges touching plumbing stay
    /// free — and the whole thing is inert outside progressive mode.
    #[test]
    fn edge_blocked_walls_only_undiscovered_entrances() {
        let mq = std::collections::HashSet::new();
        let mut settings = Settings::default();
        settings.apply(&mq);
        let idx = |n: &str| region_name_index()[0].get(n).unwrap()[0];
        let scenes = region_scenes();

        let mut inp = WorldInputs::build(&settings, &[WorldData::default()], 1, &Default::default(), true);
        // Pretend the player has discovered (landed in) Lake Hylia only.
        inp.visited_scenes = [(0u8, data::scenes::OOT_LAKE_HYLIA as u32)].into_iter().collect();

        let (lake, hyrule) = (idx("Lake Hylia"), idx("Hyrule Field"));
        // Cross-scene real entrance into an UNDISCOVERED scene -> walled.
        assert!(Inputs::edge_blocked(&inp, lake, hyrule),
            "Lake Hylia -> Hyrule Field must be walled until Hyrule Field is discovered");

        // Same transition once Hyrule Field is also discovered -> open.
        inp.visited_scenes.insert((0, data::scenes::OOT_HYRULE_FIELD as u32));
        assert!(!Inputs::edge_blocked(&inp, lake, hyrule),
            "a discovered scene is crossable");

        // Intra-scene movement is always free: Lake Hylia -> a Lake Hylia waypoint.
        if scenes[idx("Lake Hylia Owl Flight") as usize] == Some(data::scenes::OOT_LAKE_HYLIA as u32) {
            assert!(!Inputs::edge_blocked(&inp, lake, idx("Lake Hylia Owl Flight")));
        }

        // An edge touching a scene-less plumbing region (GLOBAL) is never a wall.
        assert!(!Inputs::edge_blocked(&inp, idx("Kokiri Forest"), idx("GLOBAL")),
            "an edge into plumbing must stay free");

        // Outside progressive mode the wall is inert.
        let full = WorldInputs::build(&settings, &[WorldData::default()], 1, &Default::default(), false);
        assert!(!Inputs::edge_blocked(&full, lake, hyrule),
            "full-knowledge mode never walls a vanilla edge");
    }

    /// `seed_scene` roots the scene the player is standing in right now and marks it
    /// discovered — the fix for the one-step lag where a freshly loaded scene's checks
    /// only lit up one entrance later. Crucially it roots ONLY the scene's entry
    /// regions (arrival points), not its interior sub-areas: seeding Lake Hylia roots
    /// the main "Lake Hylia" region but NOT "Lake Hylia Near Pond" (the pond ledge,
    /// reachable only through an in-scene access rule), so the interior stays gated.
    #[test]
    fn seed_scene_roots_only_entry_regions_of_the_live_scene() {
        let mq = std::collections::HashSet::new();
        let mut settings = Settings::default();
        settings.apply(&mq);
        let mut inp = WorldInputs::build(&settings, &[WorldData::default()], 1, &Default::default(), true);
        inp.seed_from_visited(&std::collections::HashSet::new());
        assert!(inp.extra_seed_regions().is_empty() && inp.visited_scenes.is_empty());

        let lake = data::scenes::OOT_LAKE_HYLIA as u32;
        inp.seed_scene(0, lake);

        assert!(inp.visited_scenes.contains(&(0, lake)), "the live scene is now discovered");
        let main = region_name_index()[0].get("Lake Hylia").unwrap()[0];
        assert!(inp.extra_seed_regions().contains(&main),
            "the scene's entry region is seeded as a root");
        let pond = region_name_index()[0].get("Lake Hylia Near Pond").unwrap()[0];
        assert!(!inp.extra_seed_regions().contains(&pond),
            "an interior gated sub-region must NOT be seeded (kept behind its access rule)");
    }

    /// Progressive discovery: arriving through a SECONDARY entrance that opens into a
    /// pocket walled off from the rest of the scene by an in-scene barrier must not
    /// light the whole scene. Reported: discovering Goron City from the Lost Woods
    /// drops the player in the gated "Goron City Shortcut" pocket, whose only edge to
    /// the main city needs explosives / hammer / Din (the three blocking rocks), so
    /// with no bombs the city's checks must stay hidden. The bug was `seed_scene`
    /// force-rooting the busiest hub on top of the real (known) arrival region.
    #[test]
    fn secondary_entrance_pocket_does_not_light_the_barred_hub() {
        use crate::data::entr as e;
        let mq = std::collections::HashSet::new();
        let mut settings = Settings::default();
        settings.apply(&mq);
        let idx = |n: &str| region_name_index()[0].get(n).unwrap()[0];
        let gc_scene =
            region_scenes()[idx("Goron City") as usize].expect("Goron City resolves to a scene");
        let (shortcut, hub) = (idx("Goron City Shortcut"), idx("Goron City"));

        // Drive the real seeding path: the player has walked ONLY the Lost Woods <->
        // Goron City link. `recompute_reachability` feeds both endpoint ids to
        // `seed_from_visited`, exactly as gathered from `out_links`.
        let visited: std::collections::HashSet<(u8, u32)> = [
            (0u8, e::OOT_GORON_CITY_FROM_LOST_WOODS_ENTR), // arrival: Goron City side
            (0u8, e::OOT_LOST_WOODS_FROM_GORON_CITY_ENTR), // departure: Lost Woods side
        ]
        .into_iter()
        .collect();
        let mut inp =
            WorldInputs::build(&settings, &[WorldData::default()], 1, &Default::default(), true);
        inp.seed_from_visited(&visited);
        // The arrival must be the gated pocket, not the hub past the rocks.
        assert!(
            inp.extra_seed_regions().contains(&shortcut),
            "the Lost Woods entrance roots the 'Goron City Shortcut' pocket it truly lands in"
        );
        assert!(
            !inp.extra_seed_regions().contains(&hub),
            "the 'Goron City' hub (its coarse to_name) must NOT be rooted — the rocks are between"
        );

        // Standing in Goron City now: the seed_scene fallback must not re-add the hub,
        // because the arrival pocket is already a known root of this scene.
        inp.seed_scene(0, gc_scene);
        assert!(!inp.extra_seed_regions().contains(&hub), "seed_scene must not force the hub either");

        // With no explosives the shortcut event never fires, so the main city — and its
        // pots — stay behind the unbroken rocks (0 checks, as reported).
        let r = crate::logic::solve::solve(&inp);
        assert!(
            !r.reachable("OOT Goron City Pot Stairs 1"),
            "a hub check stays hidden while the rocks block the only way in"
        );

        // The seed_scene fallback still works when NO arrival resolves (owl flight /
        // unresolved OUT id): it roots the hub so the loaded scene still lights.
        let mut fb =
            WorldInputs::build(&settings, &[WorldData::default()], 1, &Default::default(), true);
        fb.seed_scene(0, gc_scene);
        assert!(
            fb.extra_seed_regions().contains(&hub),
            "with no known arrival the hub is still seeded as a fallback"
        );
    }

    /// An enum setting the seed omits defaults to its unshuffled ("off") value, so
    /// `setting(k, <off>)` still holds. Regression: a ROM version without OoT clocks
    /// drops `clocksOot`, and `is_day`/`is_night` gate on `setting(clocksOot, none)` —
    /// without the default every OoT day/night check (Hyrule Field / Market wonder
    /// items…) read unreachable.
    #[test]
    fn absent_vanilla_enum_setting_defaults_to_its_off_value() {
        let mq = std::collections::HashSet::new();
        let mut settings = Settings::default();
        settings.apply(&mq); // no spoiler loaded -> clocksOot absent
        assert!(!settings.raw_settings.contains_key("clocksOot"), "precondition: clocksOot omitted");

        let inp = WorldInputs::build(&settings, &[WorldData::default()], 1, &Default::default(), false);
        let ki = *setting_key_index().get("clocksOot").expect("clocksOot in SETTING_KEYS");
        let vi = *setting_value_index().get("none").expect("none in SETTING_VALUES");
        assert!(inp.setting_has(ki, vi),
            "absent clocksOot must default to `none` so has_clock / is_day hold");
    }

    /// Multiworld: a collected check's item goes to its destination player, so it
    /// enters that player's inventory — not the world it was physically found in.
    #[test]
    fn multiworld_inventory_routes_by_destination() {
        let mq = std::collections::HashSet::new();
        let mut settings = Settings::default();
        settings.apply(&mq);
        let sword = find_item_id("Kokiri Sword (OoT)").expect("sword id");
        let loc = "OOT Kokiri Forest Kokiri Sword Chest";
        let oi = obj_idx(Game::Oot, loc);

        // World 1 collected a Kokiri Sword physically, but it is destined to player 2.
        let mut w1 = WorldData::default();
        w1.items.insert(loc.to_string(), "Kokiri Sword (OoT)".to_string());
        w1.dest.insert(loc.to_string(), 2);
        w1.collected.insert((Game::Oot, oi));
        let worlds = vec![w1, WorldData::default()];

        // Player 1 does not receive it; player 2 does.
        assert_eq!(WorldInputs::build(&settings, &worlds, 1, &Default::default(), false).item_count(sword), 0);
        assert_eq!(WorldInputs::build(&settings, &worlds, 2, &Default::default(), false).item_count(sword), 1);
    }

    /// A custom `special(X)` counts owned items across the enabled categories
    /// against `count`; an uncomputable (mask) category stays optimistic.
    #[test]
    fn special_custom_condition_counts_categories() {
        use crate::data::iid;
        let mq = std::collections::HashSet::new();
        let mut settings = Settings::default();
        settings.parse_spoiler(
            "Special Conditions\n  BRIDGE:\n    count: 2\n    medallions: true\n    stones: false\n\
             \x20\x20LACS:\n    count: 3\n    masksTransform: true\n",
            &mq,
        );
        settings.apply(&mq);
        let idx = |n: &str| data::SPECIAL_NAMES.iter().position(|&x| x == n).unwrap() as u32;
        let (bridge, lacs) = (idx("BRIDGE"), idx("LACS"));

        // No medallions -> BRIDGE (needs 2) unsatisfied.
        assert!(!WorldInputs::build(&settings, &[WorldData::default()], 1, &Default::default(), false).special(bridge));

        // One medallion -> still short.
        settings.starting_item_ids.insert(iid::OOT_MEDALLION_FOREST, 1);
        assert!(!WorldInputs::build(&settings, &[WorldData::default()], 1, &Default::default(), false).special(bridge));

        // Two medallions -> satisfied.
        settings.starting_item_ids.insert(iid::OOT_MEDALLION_FIRE, 1);
        let inp = WorldInputs::build(&settings, &[WorldData::default()], 1, &Default::default(), false);
        assert!(inp.special(bridge));

        // LACS requires a mask category the tracker does not model -> optimistic.
        assert!(inp.special(lacs));
    }

    /// Key-ring / silver-pouch set flags reach the reachability logic. OoTMM writes
    /// them in the World Flags section by display LABEL (`- Fire Temple`), and
    /// `Settings::parse_logic_sets` maps the labels to the logic TOKENS so
    /// `setting(smallKeyRingOot, Fire)` resolves — the branch of `small_keys(...)`
    /// that lets one owned KEY_RING satisfy a locked door instead of `count`
    /// individual small keys. Regression: the set was never parsed, so the flag read
    /// false and a Fire Temple key ring left the rooms behind key doors unreachable.
    #[test]
    fn key_ring_and_silver_pouch_set_flags_resolve_for_the_logic() {
        let mq = std::collections::HashSet::new();
        let mut s = Settings::default();
        s.parse_spoiler(
            "World Flags\n  Small Key Ring (OoT):\n    - Fire Temple\n    - Water Temple\n\
             \x20\x20Silver Rupee Pouches:\n    - Ganon's Castle (Light)\n",
            &mq,
        );
        s.apply(&mq);
        let inp = WorldInputs::build(&s, &[WorldData::default()], 1, &Default::default(), false);
        let skey = |n: &str| data::SETTING_KEYS.iter().position(|&x| x == n).unwrap() as u32;
        let sval = |n: &str| data::SETTING_VALUES.iter().position(|&x| x == n).unwrap() as u32;

        // The two listed dungeons flip to the key-ring branch; an unlisted one does not.
        assert!(inp.setting_has(skey("smallKeyRingOot"), sval("Fire")), "Fire ring listed");
        assert!(inp.setting_has(skey("smallKeyRingOot"), sval("Water")), "Water ring listed");
        assert!(!inp.setting_has(skey("smallKeyRingOot"), sval("Shadow")), "Shadow not listed");
        // Silver rupee pouches share the exact same mechanism.
        assert!(inp.setting_has(skey("silverRupeePouches"), sval("Ganon_Light")), "silver pouch listed");
    }

    /// `solve_world` runs end to end and is monotonic: collecting a check never
    /// removes reachability.
    #[test]
    fn solve_world_is_monotonic_in_collected() {
        let mq = std::collections::HashSet::new();
        let mut settings = Settings::default();
        settings.parse_spoiler("Settings\n  startingAge: child\n  doorOfTime: open\n", &mq);
        settings.apply(&mq);

        let empty = WorldData::default();
        let r0 = super::super::solve_world(&settings, std::slice::from_ref(&empty), 1, &Default::default(), false);

        let loc = "OOT Kokiri Forest Kokiri Sword Chest";
        let mut world = WorldData::default();
        world.items.insert(loc.to_string(), "Kokiri Sword (OoT)".to_string());
        world.collected.insert((Game::Oot, obj_idx(Game::Oot, loc)));
        let r1 = super::super::solve_world(&settings, std::slice::from_ref(&world), 1, &Default::default(), false);

        for l in &r0.locations {
            assert!(r1.locations.contains(l), "{l} lost after collecting a check");
        }
        assert!(r1.locations.len() >= r0.locations.len());
    }
}
