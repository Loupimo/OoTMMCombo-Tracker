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
//! ## `region_active` gates each dungeon's dead layout variant (per dungeon)
//! Every OoT dungeon exists as two region graphs — vanilla (`oot`) and Master Quest
//! (`oot_mq`) — and the MM Deku Palace as `mm` / `mm_jp`; both variants coexist in
//! `LOGIC_REGIONS`. They are NOT fully disjoint: a dungeon's two variants **share its
//! boss room** (`GameLayout::all`), and both variants' pre-boss rooms carry an edge into
//! it. So exploring the dead variant is not harmless — its (differently gated) path to
//! the shared boss lights that dungeon's boss checks even when the live variant walls
//! them (reported: King Dodongo's chest / heart container / stone shown though the boss
//! was unreachable). [`compute_active_regions`] therefore marks each region live or dead
//! from `settings.mq_scenes`, and the solver skips dead ones. The dungeon's inbound
//! entrance edge lands on *both* variants' entry regions, so gating the dead one keeps
//! the live one reachable.

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
    /// Per region index (into [`data::LOGIC_REGIONS`]): whether its layout is live this
    /// seed (`Inputs::region_active`). A dungeon's vanilla and Master-Quest / JP region
    /// graphs coexist in the data and share the boss room, so the dead variant must be
    /// gated out or its path leaks reachability into the shared boss checks. Built once
    /// in [`Self::build`] from `settings.mq_scenes`.
    active_regions: Vec<bool>,
    /// The `[child, adult]` ages MM can carry into OoT across a game boundary under
    /// Cross-Age Items (`crossAge`). `[false, false]` when the setting is off; otherwise
    /// the ages the player can actually be in MM (owning the MM Adult mask, or the MM
    /// starting age). See [`Inputs::cross_game_ages`]. Built once in [`Self::build`].
    cross_game_ages: [bool; 2],
    /// Renewable sources: renewable check location (`data::RENEWABLE_LOCATIONS`) ->
    /// the item it restocks for this player. See [`Inputs::renewable_item`].
    renewable_at: HashMap<&'static str, u32>,
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

        // OoT starting age. `SPAWN` only opens `SPAWN CHILD` / `SPAWN ADULT` through
        // `setting(startingAgeOot, …)` (or TIME_TRAVEL, itself behind a spawn), so
        // without that value no OoT age ever spawns and every OoT check reads
        // unreachable — grass, chests, everything. Stable v32.3 spoilers name it
        // `startingAge` (MM had no starting age yet), and with no spoiler loaded the
        // raw block is empty. Fall back to the old key, then to OoTMM's default.
        if !settings.raw_settings.contains_key("startingAgeOot") {
            let age = settings.raw_settings.get("startingAge").map(String::as_str).unwrap_or("child");
            if let (Some(&ki), Some(&vi)) = (skeys.get("startingAgeOot"), svals.get(age)) {
                settings_value.insert(ki, vi);
                settings_multi.entry(ki).or_default().insert(vi);
            }
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

        // Cross-Age Items: an age the player can be in MM carries across a game boundary
        // into OoT. MM `is_adult` is `has(MM_MASK_ADULT) || startingAgeMm == adult` (the
        // solver's mask-based MM age model, gen_logic.py); `is_child` its mirror. Only the
        // OoT direction needs this — MM's plane is seeded both ages regardless. Off (or no
        // MM adult access) contributes nothing, preserving the child-locked isolation.
        let cross_game_ages = if settings
            .raw_settings
            .get("crossAge")
            .is_some_and(|v| v.eq_ignore_ascii_case("true"))
        {
            let has_adult_mask = items.get(&data::iid::MM_MASK_ADULT).copied().unwrap_or(0) > 0;
            let mm_start_adult = settings
                .raw_settings
                .get("startingAgeMm")
                .is_some_and(|v| v.eq_ignore_ascii_case("adult"));
            [has_adult_mask || !mm_start_adult, has_adult_mask || mm_start_adult]
        } else {
            [false, false]
        };

        // Renewable sources: what each renewable location restocks for THIS player —
        // the spoiler placement when the seed shuffled it, else the vanilla item (the
        // spoiler lists only shuffled locations, so a vanilla shop / cow is absent).
        // A placement destined to another player restocks nothing here. Only the
        // player's own world counts: the solver walks that world's graph, so another
        // world's shop cannot be reached from it. Starting items are not renewable
        // (OoTMM seeds `renewables` empty).
        let own = worlds.get((player as usize).saturating_sub(1));
        let mut renewable_at = HashMap::new();
        for &(loc, vanilla) in data::RENEWABLE_LOCATIONS {
            let id = match own.and_then(|w| w.items.get(loc).map(|name| (w, name))) {
                Some((w, name)) => {
                    if w.dest.get(loc).copied().unwrap_or(player) != player {
                        continue;
                    }
                    find_item_id(name).unwrap_or(0)
                }
                None => vanilla,
            };
            if id != 0 {
                renewable_at.insert(loc, id);
            }
        }

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
            active_regions: compute_active_regions(&settings.mq_scenes),
            cross_game_ages,
            renewable_at,
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
    /// Each entrance is rooted at the region the player ACTUALLY lands in. The primary
    /// source is the entrance's exact destination NODE — `EntranceDef.to_area`, taken
    /// from OoTMM's `entrances.yml` `areas[1]`, which names the very logic region the
    /// entrance drops into (unlike the display `to_name`, e.g. "Impa's House" for the
    /// node "Impa House Front"). This distinguishes several entrances that lead from one
    /// neighbour into different, separately gated pockets of the SAME scene: Impa's
    /// House is entered by the free balcony window ("Impa House Back", the Piece of
    /// Heart) OR the rusty-key front door ("Impa House Front", the Wonder Item), so
    /// rooting only the walked pocket keeps the locked sibling hidden (reported: the
    /// Wonder Item showed after entering the back). Only that node is rooted — even when
    /// it is a scene-less hub or waypoint ("Market Pot House", "Near Romani Ranch") — and
    /// its destination scene is marked visited so the solver may extend inward across the
    /// node's real (gated) edges. It must NOT root those inner regions topologically: a
    /// hub that forks by age (the child-only vs adult-only Market pot rooms) would then
    /// light the adult room for a child, ignoring the age gate (reported).
    ///
    /// When `to_area` does not resolve to a destination-scene region (an entrance with
    /// no `areas`, or a name the logic graph lacks), it falls back to a SOURCE-side walk
    /// (from `from_name` + the whole source scene, forward to any destination-scene
    /// region one edge away) and finally to a direct `to_name` match. The solver then
    /// extends from each root through vanilla edges, discovered redirects and in-region
    /// logic. Names that resolve to no logic region are skipped.
    pub fn seed_from_visited(&mut self, visited: &HashSet<(u8, u32)>) {
        let names = region_name_index();
        let scenes = region_scenes();
        let mut regions = Vec::new();
        // Destination scenes reached through a scene-less `to_area` hub/waypoint. Rooting
        // the (scene-less) node contributes no scene of its own, but the player physically
        // arrived in this scene, so it must be marked visited for `edge_blocked` to pass
        // the hub's inward edges. Merged into `visited_scenes` below.
        let mut extra_visited: HashSet<(u8, u32)> = HashSet::new();
        for &(game, id) in visited {
            let g = if game == 0 { crate::scene::Game::Oot } else { crate::scene::Game::Mm };
            let Some(meta) = crate::entrance::lookup(g, id) else { continue };
            let dest_scene = canon_entrance_scene(game, meta.to_scene as u32);
            let src_scene = canon_entrance_scene(game, meta.from_scene as u32);
            let before = regions.len();
            let mut stack: Vec<u32> = Vec::new();
            let mut seen: HashSet<u32> = HashSet::new();
            // Follow scene-less connectors on `stack` forward, rooting every
            // destination-scene region reached. Never steps into another real scene, so
            // an in-scene barrier between the arrival pocket and the rest of the scene
            // stays gated (the barrier is a real-scene -> real-scene edge, never taken).
            let walk = |regions: &mut Vec<u32>, stack: &mut Vec<u32>, seen: &mut HashSet<u32>| {
                while let Some(s) = stack.pop() {
                    for edge in data::LOGIC_REGIONS[s as usize].exits {
                        let d = edge.to;
                        if data::LOGIC_REGIONS[d as usize].game != game {
                            continue;
                        }
                        match scenes[d as usize] {
                            Some(sc) if sc == dest_scene => regions.push(d),
                            None if seen.insert(d) => stack.push(d),
                            _ => {}
                        }
                    }
                }
            };
            // Primary: the entrance's exact destination NODE (`to_area`). Root that node
            // itself — a located pocket of the destination scene, or a scene-less hub /
            // waypoint in front of it — and let the solver extend inward across its real
            // (gated) edges. A scene-less hub is NOT walked forward to root its
            // destination-scene children: that ignored the age/item gate on those inward
            // edges and lit the adult Market pot room for a child (reported). Marking the
            // destination scene visited lets `edge_blocked` pass the hub's inward edges so
            // the solver takes only the ones whose rule the player satisfies.
            if let Some(dsts) = names[game as usize].get(meta.to_area) {
                let mut rooted = false;
                for &d in dsts {
                    if data::LOGIC_REGIONS[d as usize].game != game {
                        continue;
                    }
                    regions.push(d);
                    rooted = true;
                }
                if rooted {
                    extra_visited.insert((game, dest_scene));
                }
            }
            // Fallback 1: `to_area` gave nothing — walk the SOURCE side. Seed from the
            // named `from` region AND every region of the source scene (the table's names
            // do not always match the logic region names — "North Clock Town" vs "Clock
            // Town North" — so the scene id is the reliable source-side link), then walk
            // forward. Broader (can root sibling pockets), hence only a fallback.
            //
            // Only for a CROSS-scene entrance: when the destination scene IS the source
            // scene (an intra-scene warp with no `to_area` — the Deku Palace "Caught" guard
            // toss, a bridge), seeding every source-scene region and rooting each one's
            // in-scene neighbours would light the entire scene through its own internal
            // edges, ignoring every gate — it rooted the mask-gated "Deku Palace Upper"
            // pots off the Caught warp (reported). The player is already in that scene via
            // the entrance that brought them in, so nothing extra needs rooting here.
            if regions.len() == before && src_scene != dest_scene {
                seen.clear();
                if let Some(srcs) = names[game as usize].get(meta.from_name) {
                    for &s in srcs {
                        if seen.insert(s) {
                            stack.push(s);
                        }
                    }
                }
                for (i, r) in data::LOGIC_REGIONS.iter().enumerate() {
                    if r.game == game && scenes[i] == Some(src_scene) && seen.insert(i as u32) {
                        stack.push(i as u32);
                    }
                }
                walk(&mut regions, &mut stack, &mut seen);
            }
            // Fallback 2: match `to_name` directly when neither walk found anything.
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
        let mut visited_scenes: HashSet<(u8, u32)> = regions
            .iter()
            .filter_map(|&i| scenes[i as usize].map(|s| (data::LOGIC_REGIONS[i as usize].game, s)))
            .collect();
        // Scenes reached only through a scene-less `to_area` hub contribute no region of
        // their own but were physically entered; add them so their inward edges open.
        visited_scenes.extend(extra_visited);
        self.visited_scenes = visited_scenes;
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
        let areas = entrance_area_scene();
        // Did seed_from_visited already root a region of this exact scene? Then the
        // arrival point is known; do not also force the hub (see the doc above). A
        // scene's arrival region often has no check of its own, so `region_scenes` leaves
        // it `None` — but it IS a region of that scene, named by some entrance's
        // `to_area`. The Deku King's Chamber (scene 0x3e) is entered at "Deku Palace
        // Throne" (no check) via the throne door, or at "Deku Palace Cage" (the Sonata
        // check) via the jail door; walking the throne door roots "Throne", yet matching
        // on `region_scenes` alone missed it and force-rooted the gated "Cage" hub, whose
        // free Cage -> Near Cage -> Upper (Deku mask) path lit the upper pots (reported).
        // So also credit a seeded region that an entrance's `to_area` files under `scene`.
        let arrival_known = self.seed_regions.iter().any(|&i| {
            let r = &data::LOGIC_REGIONS[i as usize];
            r.game == game
                && (scenes[i as usize] == Some(scene)
                    || areas[game as usize].get(r.name) == Some(&scene))
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

/// Per region, whether a *walk-in* entrance can land the player in it: it has at least
/// one incoming edge, from a region in a different scene (or a scene-less plumbing
/// node), that is not a WARP. These are the arrival points of a scene; a region
/// reachable only from within its own scene (a gated pond ledge, a bean spot) is not
/// one. [`WorldInputs::seed_scene`] roots the busiest of them (the hub) and lets
/// in-scene rules gate the rest.
///
/// Warp-song / owl landings are excluded: a region reached only from a scene-less warp
/// node (the nodes `GLOBAL` warps to — `SONG_TP_*`, `MM SOARING`, `EGGS`) is a warp pad,
/// not a walk-in entry. Being physically in a scene does not mean the player warped to
/// its pad; treating "Death Mountain Crater Warp" (fed only by `SONG_TP_FIRE`) as the
/// hub seeded it at child age and lit the tunic-gated child rupees reached from it via
/// `is_child` (reported). With it excluded, `seed_scene` falls back to a real walk-in
/// entry (Crater Top/Bridges), whose own rules keep the tunic gate honest.
/// Static: depends only on the region graph + `region_scenes`.
fn entry_regions() -> &'static [bool] {
    static E: OnceLock<Vec<bool>> = OnceLock::new();
    E.get_or_init(|| {
        let scenes = region_scenes();
        // Warp nodes: every region a `GLOBAL` node exits to (the warp songs, MM soaring,
        // eggs). An edge from one is a warp, not a walk-in, so it must not mark an entry.
        let mut warp_node = vec![false; data::LOGIC_REGIONS.len()];
        for r in data::LOGIC_REGIONS.iter() {
            if r.name == "GLOBAL" {
                for e in r.exits {
                    warp_node[e.to as usize] = true;
                }
            }
        }
        let mut entry = vec![false; data::LOGIC_REGIONS.len()];
        for (j, r) in data::LOGIC_REGIONS.iter().enumerate() {
            if warp_node[j] {
                continue;
            }
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

/// Per game, an entrance `to_area` region name -> the (canonicalized) scene that entrance
/// drops into. A scene's arrival region often carries no check of its own (e.g. "Deku
/// Palace Throne", the Deku King's Chamber entry), so [`region_scenes`] leaves it `None`
/// even though it is a region of that scene. [`WorldInputs::seed_scene`] consults this so
/// a scene whose arrival [`WorldInputs::seed_from_visited`] already rooted counts as known
/// — instead of force-rooting a different, gated hub region of the same scene. First
/// writer wins if two entrances share a `to_area` (they resolve to the same scene anyway).
fn entrance_area_scene() -> &'static [HashMap<&'static str, u32>; 2] {
    static M: OnceLock<[HashMap<&'static str, u32>; 2]> = OnceLock::new();
    M.get_or_init(|| {
        let mut m: [HashMap<&'static str, u32>; 2] = [HashMap::new(), HashMap::new()];
        for (g, ents) in [(0usize, data::OOT_ENTRANCES), (1usize, data::MM_ENTRANCES)] {
            for e in ents {
                if !e.to_area.is_empty() {
                    let s = canon_entrance_scene(g as u8, e.to_scene as u32);
                    m[g].entry(e.to_area).or_insert(s);
                }
            }
        }
        m
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
        // the object table — but keyed to the scene the ENTRANCE table and the map use,
        // which the object table sometimes files a check under a different id from:
        //   * Context variants (`context != All`): MM season copies (Mountain Village
        //     Winter/Spring) and OoT day/age copies are separate object scenes, while
        //     the map shows one scene toggled by context and every entrance targets that
        //     base — which is the check's `render_scene`. Fold onto it.
        //   * Generic groupers: grottos and fairy fountains file every check under one
        //     lumping scene (OOT_GROTTOS / OOT_FAIRY_FOUNTAIN / MM_GROTTOS / …) that is
        //     not itself an entrance destination, while each instance's entrance keys the
        //     specific `render_scene`. Fold onto it too.
        // Without this the entrance boundary is not recognised and `edge_blocked` leaves
        // it open, so a bomb-openable grotto, a fairy fountain, or a whole season copy of
        // an area lit up from a neighbour the player had merely visited (reported after a
        // Bomb Bag let the explosives rules fire). A boss lair keeps its own object scene:
        // it is `context: All` and its lair IS an entrance destination, so neither fold
        // triggers and its boss-door boundary stays gated.
        let dest = entrance_dest_scenes();
        let mut scene: Vec<Option<u32>> = data::LOGIC_REGIONS
            .iter()
            .map(|r| {
                let objs = if r.game == 0 { data::OOT_OBJECTS } else { data::MM_OBJECTS };
                let d = &dest[r.game as usize];
                // Majority vote of the folded scene across ALL resolving locations, not
                // just the first: a region's scene is where most of its checks live. A
                // single shared reward filed under another instance's render scene — the
                // "Termina Field Gossip Stones HP", listed in all four gossip-stone
                // grottos but rendering in just one — must not hijack the region. With the
                // first location it folded every gossip grotto onto that one scene, so
                // `edge_blocked` saw them as one scene and visiting one lit the others.
                let mut counts: Vec<(u32, u32)> = Vec::new(); // (scene, count), first-seen order
                for l in r.locations {
                    if let Some(o) = objs.iter().find(|o| o.location == l.loc) {
                        let (s, rs) = (o.scene as u32, o.render_scene as u32);
                        let context_variant = o.context != data::ObjectContext::All;
                        let generic_grouper = !d.contains(&s) && d.contains(&rs);
                        let folded = if s != rs && (context_variant || generic_grouper) { rs } else { s };
                        match counts.iter_mut().find(|(sc, _)| *sc == folded) {
                            Some((_, c)) => *c += 1,
                            None => counts.push((folded, 1)),
                        }
                    }
                }
                // The strictly-most-common folded scene; ties keep the first-seen (stable),
                // which matches the old first-location behaviour when every check agrees.
                let mut best: Option<(u32, u32)> = None;
                for &(sc, c) in &counts {
                    if best.map_or(true, |(_, bc)| c > bc) {
                        best = Some((sc, c));
                    }
                }
                best.map(|(sc, _)| sc)
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

        // Pass 2b: a location-less DUNGEON region (a dungeon's entry hall or an inner
        // transition node that hosts no checks of its own) inherits its dungeon's scene.
        // Dungeon interiors carry the OoTMM `dungeon:` tag instead of an overworld
        // `area:`, so Pass 2 skipped them and they stayed scene-less — and `edge_blocked`
        // cannot wall an entrance into a `None` scene, so an undiscovered dungeon lit up
        // the instant its overworld scene was reached with the access items (Bottom of the
        // Well showed from Kakariko though its entrance was unfound — reported; the same
        // latent hole affected every dungeon whose entry hall carries no check, e.g. the
        // Deku Tree lobby). Learn `(game, dungeon) -> scene` from the RENDER scene of the
        // dungeon's checks: a boss lair's checks keep the dungeon's render scene even
        // though their own object scene is the lair, so the whole dungeon maps to one
        // scene here, while the lair regions still take their real lair scene from Pass 1
        // (they host checks) and stay gated behind their own boss-door entrance. Ambiguous
        // dungeons (none expected) stay `None`.
        let mut dungeon_scene: HashMap<(u8, &str), Option<u32>> = HashMap::new();
        for r in data::LOGIC_REGIONS.iter() {
            if r.dungeon.is_empty() {
                continue;
            }
            let objs = if r.game == 0 { data::OOT_OBJECTS } else { data::MM_OBJECTS };
            for l in r.locations {
                if let Some(o) = objs.iter().find(|o| o.location == l.loc) {
                    let s = o.render_scene as u32;
                    dungeon_scene
                        .entry((r.game, r.dungeon))
                        .and_modify(|e| {
                            if *e != Some(s) {
                                *e = None;
                            }
                        })
                        .or_insert(Some(s));
                }
            }
        }
        for (i, r) in data::LOGIC_REGIONS.iter().enumerate() {
            if scene[i].is_none() && !r.dungeon.is_empty() {
                if let Some(&Some(s)) = dungeon_scene.get(&(r.game, r.dungeon)) {
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

/// Per game, every scene id that appears as an entrance endpoint (`to_scene` or
/// `from_scene`) in the entrance table — i.e. a scene you can walk into through a real
/// loading zone. Used by `region_scenes` to tell a grotto / fairy-fountain check (filed
/// under a generic grouping scene that is NOT an entrance endpoint, its render scene the
/// specific one that IS) apart from a boss-lair check (its own scene already an entrance
/// endpoint). Raw scene ids (no Market fold — grottos and lairs need no folding).
fn entrance_dest_scenes() -> &'static [HashSet<u32>; 2] {
    static D: OnceLock<[HashSet<u32>; 2]> = OnceLock::new();
    D.get_or_init(|| {
        let mut d: [HashSet<u32>; 2] = [HashSet::new(), HashSet::new()];
        for (game, table) in [(0usize, data::OOT_ENTRANCES), (1usize, data::MM_ENTRANCES)] {
            for e in table {
                if e.type_ == crate::data::EntranceType::None {
                    continue;
                }
                d[game].insert(e.to_scene as u32);
                d[game].insert(e.from_scene as u32);
            }
        }
        d
    })
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

/// OoT dungeon `LogicRegion.dungeon` code -> its base scene id, for the 12 dungeons that
/// have a Master Quest variant. Both the vanilla (`oot`) and MQ (`oot_mq`) region graphs
/// of such a dungeon carry this code; which one is live depends on whether this scene
/// runs MQ ([`compute_active_regions`]). A code absent here — the overworld (`""`),
/// Ganon's Tower, Gerudo Fortress, Thieves' Hideout — has no MQ variant, so its regions
/// are always live. Scene ids match `settings.mq_scenes` (base ids, see `parse_mq`).
fn oot_mq_dungeon_scene(code: &str) -> Option<u16> {
    use crate::data::scenes as s;
    Some(match code {
        "DT" => s::OOT_DEKU_TREE,
        "DC" => s::OOT_DODONGO_CAVERN,
        "JJ" => s::OOT_INSIDE_JABU_JABU,
        "Forest" => s::OOT_TEMPLE_FOREST,
        "Fire" => s::OOT_TEMPLE_FIRE,
        "Water" => s::OOT_TEMPLE_WATER,
        "Shadow" => s::OOT_TEMPLE_SHADOW,
        "Spirit" => s::OOT_TEMPLE_SPIRIT,
        "BotW" => s::OOT_BOTTOM_OF_THE_WELL,
        "IC" => s::OOT_ICE_CAVERN,
        "GTG" => s::OOT_GERUDO_TRAINING_GROUND,
        "Ganon" => s::OOT_INSIDE_GANON_CASTLE,
        _ => return None,
    })
}

/// Per region index, whether its layout is live this seed — backs
/// [`Inputs::region_active`]. `mq` is the seed's Master-Quest (OoT) / JP (MM) scene set
/// (`Settings::mq_scenes`; empty = base game everywhere). `all` regions are always live.
/// An OoT dungeon's vanilla (`oot`) / MQ (`oot_mq`) variants are gated by whether that
/// dungeon runs MQ; the MM Deku-Palace `mm` / `mm_jp` variants by the all-or-nothing JP
/// toggle. Both variants of a dungeon share its boss room and both receive the dungeon's
/// inbound entrance edge, so gating the dead one keeps the entrance connected through the
/// live one while stopping the dead path from lighting the shared boss checks.
fn compute_active_regions(mq: &HashSet<(Game, u16)>) -> Vec<bool> {
    let jp_on = mq.iter().any(|&(g, _)| g == Game::Mm);
    data::LOGIC_REGIONS
        .iter()
        .map(|r| match r.layout {
            GameLayout::all => true,
            GameLayout::mm => !jp_on,
            GameLayout::mm_jp => jp_on,
            GameLayout::oot => match oot_mq_dungeon_scene(r.dungeon) {
                Some(scene) => !mq.contains(&(Game::Oot, scene)),
                None => true, // overworld / no-MQ dungeon: always live
            },
            GameLayout::oot_mq => match oot_mq_dungeon_scene(r.dungeon) {
                Some(scene) => mq.contains(&(Game::Oot, scene)),
                None => false, // an oot_mq region must belong to an MQ-capable dungeon
            },
        })
        .collect()
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
    fn region_active(&self, idx: usize) -> bool {
        self.active_regions.get(idx).copied().unwrap_or(true)
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
    fn renewable_item(&self, location: &str) -> Option<u32> {
        self.renewable_at.get(location).copied()
    }
    fn extra_seed_regions(&self) -> &[u32] {
        &self.seed_regions
    }
    fn cross_game_ages(&self) -> [bool; 2] {
        self.cross_game_ages
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
        // Entering scene-less plumbing (SPAWN / GLOBAL / warp / junction / event nodes)
        // is never a wall — it carries no checks and is gated by its own access rule.
        let Some(sb) = scenes[to as usize] else {
            return false;
        };
        let sa = scenes[from as usize];
        // Same scene → intra-scene movement, always free.
        if sa == Some(sb) {
            return false;
        }
        // A real-scene → real-scene edge that is not a trackable entrance (e.g. a dungeon
        // → boss-lair door that is not a One_Way_In entrance) is free — crossing it is
        // implied by reaching the source.
        if let Some(sa) = sa {
            if !entrance_scene_pairs().contains(&(rf.game, sa, sb)) {
                return false;
            }
        }
        // Otherwise this crosses INTO real scene `sb` — either from another real scene over
        // a trackable entrance, or from a scene-less waypoint / warp hub whose edge IS the
        // loading zone ("Near Swamp Spider House" → the spider house, the Song-of-Soaring
        // hub → an owl statue, a bombable "Behind Large Icicles" approach → the path). It
        // may only be crossed into a scene the player has actually walked into; stepping
        // into an undiscovered scene is walled. Without gating the scene-less case, crediting
        // an item (explosives / a warp song) opened un-entered scenes (reported leak).
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

    /// A shared Bomb Bag fires the explosives rules; in progressive mode that must NOT
    /// open grottos / fairy fountains the player has not walked into. Those file their
    /// checks under a generic grouping scene (OOT_GROTTOS / OOT_FAIRY_FOUNTAIN), which is
    /// not an entrance destination, while the map keys each instance by the specific
    /// scene it renders in — so `region_scenes` must resolve them to that specific scene
    /// for `edge_blocked` to wall the boundary. Reported: after picking up a shared Bomb
    /// Bag, grottos and fairy fountains lit up from a merely-visited Hyrule Field.
    #[test]
    fn bomb_openable_grottos_and_fountains_stay_gated_until_visited() {
        // A grotto region resolves to its specific render scene, not the generic grouper.
        let grotto_obj = data::OOT_OBJECTS
            .iter()
            .find(|o| o.scene as u32 == data::scenes::OOT_GROTTOS as u32)
            .expect("a grotto object exists");
        let grotto_region = data::LOGIC_REGIONS
            .iter()
            .position(|r| r.game == 0 && r.locations.iter().any(|l| l.loc == grotto_obj.location))
            .expect("its logic region exists");
        assert_ne!(
            region_scenes()[grotto_region],
            Some(data::scenes::OOT_GROTTOS as u32),
            "a grotto region must not stay on the generic grouping scene"
        );
        assert_eq!(region_scenes()[grotto_region], Some(grotto_obj.render_scene as u32));

        let mq = std::collections::HashSet::new();
        let mut settings = Settings::default();
        settings.raw_settings.insert("startingAgeOot".into(), "child".into());
        settings.apply(&mq);

        // Stand in Hyrule Field with a shared Bomb Bag collected there.
        let hf = data::scenes::OOT_HYRULE_FIELD as u32;
        let loc = data::OOT_OBJECTS
            .iter()
            .find(|o| o.scene as u32 == hf && o.type_ != data::ObjectType::none)
            .map(|o| o.location)
            .expect("a HF object");
        let mut world = WorldData::default();
        world.items.insert(loc.to_string(), "Bomb Bag".to_string());
        world.collected.insert((Game::Oot, obj_idx(Game::Oot, loc)));

        let mut inp =
            WorldInputs::build(&settings, std::slice::from_ref(&world), 1, &Default::default(), true);
        inp.visited_scenes = [(0u8, hf)].into_iter().collect();
        inp.seed_scene(0, hf);
        assert_eq!(inp.item_count(data::iid::SHARED_BOMB_BAG), 1, "the shared Bomb Bag is credited");

        let r = crate::logic::solve::solve(&inp);
        // No grotto / fairy-fountain check leaks in — none has been entered.
        let generic =
            [data::scenes::OOT_GROTTOS as u32, data::scenes::OOT_FAIRY_FOUNTAIN as u32];
        let leaked = data::OOT_OBJECTS
            .iter()
            .filter(|o| generic.contains(&(o.scene as u32)) && r.reachable(o.location))
            .count();
        assert_eq!(leaked, 0, "no bomb-openable grotto / fountain check leaks from a visited overworld");
        // Sanity: the visited overworld's own checks are reachable.
        assert!(r.reachable("OOT Hyrule Field Tree 01"), "Hyrule Field itself stays reachable");
    }

    /// In progressive mode a Bomb Bag (explosives) must not open MM areas the player has
    /// not walked into. MM season copies (Mountain Village / Twin Islands Winter/Spring)
    /// are separate OBJECT scenes the map folds onto one base via the check's
    /// `render_scene`; the connector "Path to Mountain Village" and the spider houses are
    /// their own scenes. Reported: after a Bomb Bag they lit up from a merely-visited
    /// Termina Field / Great Bay Coast — the season copies because `region_scenes` had
    /// filed them under the un-entered variant id instead of the folded base.
    #[test]
    fn mm_context_and_path_scenes_stay_gated_until_visited() {
        let mq = std::collections::HashSet::new();
        let mut settings = Settings::default();
        settings.apply(&mq);

        let visited =
            [data::scenes::MM_TERMINA_FIELD as u32, data::scenes::MM_GREAT_BAY_COAST as u32];
        let loc = data::MM_OBJECTS
            .iter()
            .find(|o| {
                o.scene as u32 == data::scenes::MM_GREAT_BAY_COAST as u32
                    && o.type_ != data::ObjectType::none
            })
            .map(|o| o.location)
            .expect("a Great Bay Coast object");
        let mut world = WorldData::default();
        world.items.insert(loc.to_string(), "Bomb Bag".to_string());
        world.collected.insert((Game::Mm, obj_idx(Game::Mm, loc)));

        let mut inp =
            WorldInputs::build(&settings, std::slice::from_ref(&world), 1, &Default::default(), true);
        inp.visited_scenes = visited.iter().map(|&s| (1u8, s)).collect();
        for &s in &visited {
            inp.seed_scene(1, s);
        }
        assert_eq!(inp.item_count(data::iid::SHARED_BOMB_BAG), 1, "the shared Bomb Bag is credited");

        let r = crate::logic::solve::solve(&inp);
        let reachable_in =
            |scene: u32| data::MM_OBJECTS.iter().any(|o| o.scene as u32 == scene && r.reachable(o.location));

        use data::scenes as sc;
        for (scene, name) in [
            (sc::MM_PATH_MOUNTAIN_VILLAGE as u32, "Path to Mountain Village"),
            (sc::MM_MOUNTAIN_VILLAGE_WINTER as u32, "Mountain Village (Winter)"),
            (sc::MM_TWIN_ISLANDS_WINTER as u32, "Twin Islands (Winter)"),
            (sc::MM_SPIDER_HOUSE_OCEAN as u32, "Ocean Spider House"),
        ] {
            assert!(!reachable_in(scene), "{name} must stay gated until its entrance is discovered");
        }
        assert!(reachable_in(sc::MM_TERMINA_FIELD as u32), "Termina Field itself stays reachable");
    }

    /// MM `is_adult` must be gated by the ability to actually become adult — owning the
    /// MM Adult mask (crossAge) or starting adult — not by the solver's age plane. The
    /// solver seeds MM at both ages unconditionally, so OoTMM's `age(adult)` was trivially
    /// true and lit every adult-only MM check; the tracker's MM macros drop the age()
    /// dependency (gen_logic.py override). Reported: the Doggy Racetrack Chest
    /// (`… || is_tall || …`, is_tall = has_mask_zora || is_adult) showed with no way to
    /// reach it. With an empty inventory its only satisfiable term is is_adult.
    #[test]
    fn mm_is_adult_needs_the_adult_mask_not_the_age_plane() {
        let mq = std::collections::HashSet::new();
        let mut settings = Settings::default();
        settings.raw_settings.insert("crossAge".into(), "true".into());
        settings.apply(&mq);

        let region = data::LOGIC_REGIONS
            .iter()
            .position(|r| r.game == 1 && r.name == "Doggy Racetrack")
            .expect("Doggy Racetrack region exists") as u32;
        let loc = "MM Doggy Racetrack Chest";

        let solved = |mask: bool| {
            let world = WorldData::default();
            let mut inp = WorldInputs::build(
                &settings, std::slice::from_ref(&world), 1, &Default::default(), false);
            // Seed the race track directly; an empty inventory leaves is_adult as the
            // chest's only satisfiable access term (no beans / hookshot / Zora mask / trick).
            inp.seed_regions = vec![region];
            if mask {
                inp.items.insert(data::iid::MM_MASK_ADULT, 1);
            }
            crate::logic::solve::solve(&inp).reachable(loc)
        };

        assert!(!solved(false),
            "adult-only MM chest must stay unreachable without the means to become adult");
        assert!(solved(true),
            "the MM Adult mask makes the adult-only chest reachable");
    }

    /// The four MM gossip-stone grottos (Swamp / Mountain / Ocean / Canyon) each share the
    /// single "Termina Field Gossip Stones HP" reward, listed in all four logic regions but
    /// rendering in just one grotto's scene. Taking the FIRST resolving location folded all
    /// four regions onto that one scene, so `edge_blocked` saw them as a single scene and
    /// walking into one lit the others (reported: only the Swamp grotto visited, but the
    /// Ocean/Mountain/Canyon grottos showed their grass/hive/butterfly checks). The
    /// majority-vote fold in `region_scenes` keeps each grotto on its own render scene.
    #[test]
    fn mm_gossip_grottos_keep_distinct_scenes() {
        use crate::data::scenes as sc;
        let rs = region_scenes();
        let region_scene = |first_loc: &str| {
            let ri = data::LOGIC_REGIONS
                .iter()
                .position(|r| r.game == 1 && r.locations.iter().any(|l| l.loc == first_loc))
                .expect("grotto region exists");
            rs[ri]
        };
        // Each grotto region resolves to its OWN render scene, not a single shared one.
        assert_eq!(region_scene("MM Swamp Gossip Grotto Gossip Fairy 1"),
            Some(sc::MM_GROTTO_TERMINA_SWAMP_GOSSIP as u32));
        assert_eq!(region_scene("MM Ocean Gossip Grotto Grass 1"),
            Some(sc::MM_GROTTO_TERMINA_OCEAN_GOSSIP as u32));
        assert_eq!(region_scene("MM Mountain Gossip Grotto Gossip Fairy 1"),
            Some(sc::MM_GROTTO_TERMINA_MOUNTAIN_GOSSIP as u32));
        assert_eq!(region_scene("MM Canyon Gossip Grotto Grass 1"),
            Some(sc::MM_GROTTO_TERMINA_CANYON_GOSSIP as u32));

        // Progressive mode: having walked only the Swamp grotto, the others stay hidden.
        let mq = std::collections::HashSet::new();
        let mut settings = Settings::default();
        settings.apply(&mq);
        let visited: HashSet<(u8, u32)> = [
            (1u8, data::entr::MM_GROTTO_GOSSIPS_SWAMP_ENTR),
            (1u8, data::entr::MM_GROTTO_EXIT_GOSSIPS_SWAMP),
        ]
        .into_iter()
        .collect();
        let mut inp = WorldInputs::build(&settings, &[WorldData::default()], 1, &Default::default(), true);
        inp.seed_from_visited(&visited);
        let r = crate::logic::solve::solve(&inp);
        for loc in [
            "MM Ocean Gossip Grotto Grass 1",
            "MM Ocean Gossip Grotto Hive",
            "MM Mountain Gossip Grotto Grass 1",
            "MM Canyon Gossip Grotto Grass 1",
        ] {
            assert!(!r.reachable(loc), "{loc} must stay hidden until its own grotto is visited");
        }
    }

    /// An intra-scene warp with no `to_area` (the Deku Palace "Caught" guard toss, whose
    /// `from_scene == to_scene`) must not fall into the source-side walk: seeding every
    /// region of the scene and rooting each one's in-scene neighbours lights the whole
    /// palace through its own gated edges — it rooted the mask-gated "Deku Palace Upper"
    /// pots (reported: pots shown in progressive mode with no way to reach Upper). The
    /// scene is already rooted by the entrance that brought the player in.
    #[test]
    fn intra_scene_warp_does_not_root_the_whole_scene() {
        let mq = HashSet::new();
        let mut settings = Settings::default();
        settings.apply(&mq);
        let mut inp = WorldInputs::build(&settings, &[WorldData::default()], 1, &Default::default(), true);
        inp.items.insert(data::iid::MM_MASK_DEKU, 1); // the mask that would open Upper if rooted

        let visited: HashSet<(u8, u32)> = [
            (1u8, data::entr::MM_DEKU_PALACE_MAIN_ENTRANCE_ENTR), // normal entry -> "Deku Palace Front"
            (1u8, data::entr::MM_DEKU_PALACE_CAUGHT),             // intra-scene guard toss (no to_area)
        ]
        .into_iter()
        .collect();
        inp.seed_from_visited(&visited);

        let idx = |n: &str| region_name_index()[1][n][0];
        assert!(
            inp.seed_regions.contains(&idx("Deku Palace Front")),
            "the normal entrance still roots the palace arrival (Front)",
        );
        assert!(
            !inp.seed_regions.contains(&idx("Deku Palace Upper")),
            "the intra-scene Caught warp must not root the gated Upper pocket",
        );
        let r = crate::logic::solve::solve(&inp);
        assert!(
            !r.reachable("MM Deku Palace Pot 1"),
            "the upper pots stay hidden with no real path to Upper",
        );
    }

    /// The Deku King's Chamber (scene 0x3e) is entered at "Deku Palace Throne" (no check
    /// of its own, so `region_scenes` = None) via the throne door, or at "Deku Palace
    /// Cage" (the Sonata check) via the jail door. Standing in the chamber after the
    /// throne door must not root the gated "Cage" hub — its free Cage -> Near Cage ->
    /// Upper (Deku mask) path would light the upper pots (reported: pots shown in
    /// progressive mode though the player could not grow the beans to reach Upper).
    #[test]
    fn chamber_throne_arrival_does_not_leak_to_the_upper_pots() {
        let mq = HashSet::new();
        let mut settings = Settings::default();
        settings.apply(&mq);
        let mut inp = WorldInputs::build(&settings, &[WorldData::default()], 1, &Default::default(), true);
        inp.items.insert(data::iid::MM_MASK_DEKU, 1); // owns the Deku mask (the leak's key)

        // Walk the throne door: `seed_from_visited` roots its `to_area` "Deku Palace
        // Throne", which has no check, so `region_scenes` leaves it None.
        let visited: HashSet<(u8, u32)> =
            [(1u8, data::entr::MM_DEKU_PALACE_THRONE_ENTR)].into_iter().collect();
        inp.seed_from_visited(&visited);
        assert!(
            inp.seed_regions.contains(&(region_name_index()[1]["Deku Palace Throne"][0])),
            "the throne door roots the Throne arrival region",
        );

        // Standing in the chamber must recognise the Throne arrival and NOT force the Cage
        // hub, so the mask-gated upper pots stay hidden.
        inp.seed_scene(1, data::scenes::MM_DEKU_KING_CHAMBER as u32);
        let cage = region_name_index()[1]["Deku Palace Cage"][0];
        assert!(
            !inp.seed_regions.contains(&cage),
            "the gated Cage hub must not be force-rooted when the Throne arrival is known",
        );
        let r = crate::logic::solve::solve(&inp);
        assert!(
            !r.reachable("MM Deku Palace Pot 1"),
            "the upper pots stay hidden when the chamber was entered at the throne",
        );
    }

    /// A dungeon's vanilla and Master Quest region graphs share the boss room, so the
    /// dead variant must be gated per dungeon: with no MQ dungeons the MQ graph is off
    /// (and vice-versa). Without this the MQ Pre-Boss Lobby stayed reachable and lit King
    /// Dodongo's chest / heart container / stone even when the vanilla path to the boss
    /// was walled (reported).
    #[test]
    fn dungeon_variant_regions_are_gated_by_the_seed_layout() {
        use crate::data::scenes as s;
        let dc: Vec<(usize, GameLayout)> = data::LOGIC_REGIONS
            .iter()
            .enumerate()
            .filter(|(_, r)| r.game == 0 && r.name == "Dodongo Cavern")
            .map(|(i, r)| (i, r.layout))
            .collect();
        let vanilla = dc.iter().find(|(_, l)| *l == GameLayout::oot).expect("vanilla DC entry").0;
        let mq = dc.iter().find(|(_, l)| *l == GameLayout::oot_mq).expect("MQ DC entry").0;

        // No Master Quest dungeons: vanilla graph live, MQ graph dead.
        let mut base = Settings::default();
        base.apply(&Default::default());
        let inp = WorldInputs::build(&base, &[WorldData::default()], 1, &Default::default(), false);
        assert!(inp.region_active(vanilla), "vanilla DC is live when DC is not Master Quest");
        assert!(!inp.region_active(mq), "MQ DC is dead when DC is not Master Quest");

        // Dodongo's Cavern set to Master Quest: the two graphs swap.
        let mut mqset = HashSet::new();
        mqset.insert((Game::Oot, s::OOT_DODONGO_CAVERN));
        let mut mqs = Settings::default();
        mqs.apply(&mqset);
        let inp = WorldInputs::build(&mqs, &[WorldData::default()], 1, &Default::default(), false);
        assert!(!inp.region_active(vanilla), "vanilla DC is dead when DC is Master Quest");
        assert!(inp.region_active(mq), "MQ DC is live when DC is Master Quest");

        // The always-shared boss room stays live regardless of the layout.
        let boss = data::LOGIC_REGIONS
            .iter()
            .position(|r| r.game == 0 && r.name == "Dodongo Cavern Boss")
            .unwrap();
        assert!(inp.region_active(boss), "the shared boss room is always live");
    }

    /// A warp-song / owl landing region must not be picked as a scene's `seed_scene` hub.
    /// "Death Mountain Crater Warp" is fed only by the scene-less `SONG_TP_FIRE` node
    /// (the Bolero warp pad), so it looked like the crater's busiest "entry" and got
    /// seeded when the player merely stood in the crater — which lit the tunic-gated
    /// child rupees reached from it via `is_child`, with no Goron Tunic (reported).
    /// `entry_regions` now excludes warp-node sources, so the hub is a real walk-in entry
    /// whose own rules keep the tunic gate honest.
    #[test]
    fn warp_pad_is_not_a_seed_scene_hub() {
        let idx = |n: &str| region_name_index()[0].get(n).expect("region")[0] as usize;
        let entry = entry_regions();
        assert!(
            !entry[idx("Death Mountain Crater Warp")],
            "the Bolero warp pad must not count as a walk-in entry"
        );
        // The crater still has real walk-in entries (from Summit / Goron City), so a hub
        // can still be picked.
        assert!(
            entry[idx("Death Mountain Crater Bridges")] || entry[idx("Death Mountain Crater Top")],
            "a real walk-in crater entry must remain"
        );

        // End to end: a tunic-less child physically in the crater (seed_scene) must NOT
        // see the child rupees — reaching them needs the Goron Tunic (or the Bolero warp).
        let mq = std::collections::HashSet::new();
        let mut s = Settings::default();
        s.raw_settings.insert("startingAgeOot".into(), "child".into());
        s.raw_settings.insert("doorOfTime".into(), "open".into());
        s.apply(&mq);
        let mut inp = WorldInputs::build(&s, &[WorldData::default()], 1, &Default::default(), true);
        inp.seed_scene(0, data::scenes::OOT_DEATH_MOUNTAIN_CRATER as u32);
        let r = crate::logic::solve::solve(&inp);
        assert!(
            !r.reachable("OOT Death Mountain Crater Rupee Child 1"),
            "standing in the crater without the tunic must not reveal the child rupees"
        );
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

    /// Progressive seeding must root only the pocket a walked entrance lands in, not
    /// every separately gated pocket of the same scene. Impa's House (scene 0x37) is one
    /// interior split into two disconnected nodes reached from Kakariko by DIFFERENT
    /// entrances: the free balcony window -> "Impa House Back" (the Piece of Heart) and
    /// the rusty-key front door -> "Impa House Front" (the Wonder Item). Walking the back
    /// entrance used to root both — the old seed walked ungated edges from the whole
    /// source scene, and Kakariko has an (ungated-in-walk) edge to the front node — so it
    /// lit the locked Wonder Item (reported). `EntranceDef.to_area` roots exactly the
    /// node walked, leaving the front gated behind its rusty key.
    #[test]
    fn walked_entrance_roots_only_its_pocket_not_the_locked_sibling() {
        use crate::data::entr as e;

        let mq = std::collections::HashSet::new();
        let mut settings = Settings::default();
        // Rusty keys shuffled: the front door needs the (unowned) Impa's House rusty key.
        settings.raw_settings.insert("rustyKeysOot".into(), "true".into());
        settings.raw_settings.insert("startingAgeOot".into(), "child".into());
        settings.apply(&mq);

        let idx = |n: &str| region_name_index()[0].get(n).expect("region")[0];
        let (front, back) = (idx("Impa House Front"), idx("Impa House Back"));
        let impa = data::scenes::OOT_IMPA_HOUSE as u32;

        // Walk the BACK entrance (balcony window) and stand in the scene.
        let visited: std::collections::HashSet<(u8, u32)> = [
            (0u8, e::OOT_HOUSE_IMPA_BACK_ENTR),
            (0u8, e::OOT_KAKARIKO_FROM_IMPA_BACK_ENTR),
        ]
        .into_iter()
        .collect();
        let mut inp = WorldInputs::build(&settings, &[WorldData::default()], 1, &visited, true);
        inp.seed_from_visited(&visited);
        inp.seed_scene(0, impa);
        assert!(inp.extra_seed_regions().contains(&back), "the walked back node is rooted");
        assert!(
            !inp.extra_seed_regions().contains(&front),
            "the locked front node (rusty-key door) must NOT be rooted by the back entrance"
        );

        let r = crate::logic::solve::solve(&inp);
        assert!(r.reachable("OOT Kakariko Impa House HP"), "the HP (back) is reachable");
        assert!(
            !r.reachable("OOT Kakariko Impa House Wonder Item"),
            "the Wonder Item (front) stays hidden without the rusty key"
        );

        // Walking the FRONT door instead roots the front node, so its Wonder Item shows —
        // the player physically entered that pocket, key or not.
        let visited_front: std::collections::HashSet<(u8, u32)> = [
            (0u8, e::OOT_HOUSE_IMPA_ENTR),
            (0u8, e::OOT_KAKARIKO_FROM_IMPA_ENTR),
        ]
        .into_iter()
        .collect();
        let mut inp2 = WorldInputs::build(&settings, &[WorldData::default()], 1, &visited_front, true);
        inp2.seed_from_visited(&visited_front);
        assert!(inp2.extra_seed_regions().contains(&front), "the walked front node is rooted");
        let r2 = crate::logic::solve::solve(&inp2);
        assert!(
            r2.reachable("OOT Kakariko Impa House Wonder Item"),
            "the Wonder Item shows once the front door is actually walked"
        );
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

    /// A dungeon whose entry hall hosts no checks (Bottom of the Well, the Deku Tree
    /// lobby) used to leave that region scene-less, so `edge_blocked` could not wall its
    /// entrance and the dungeon lit up the instant its overworld scene was reached with
    /// the access items though the entrance was still unfound (reported: the Bottom of
    /// the Well showed from Kakariko). `region_scenes` now resolves a location-less
    /// dungeon region to its dungeon's scene, so the entrance is gated like any other.
    #[test]
    fn location_less_dungeon_entry_is_scened_and_gated() {
        let scenes = region_scenes();
        let idx = |n: &str| region_name_index()[0].get(n).unwrap()[0];

        // The check-less entry halls now carry their dungeon's (render) scene — even the
        // Deku Tree, whose dungeon also spans the Gohma lair (a boss lair keeps the
        // dungeon's render scene, so the dungeon still maps to one scene here).
        let botw = data::scenes::OOT_BOTTOM_OF_THE_WELL as u32;
        assert_eq!(scenes[idx("Bottom of the Well") as usize], Some(botw));
        assert_eq!(
            scenes[idx("Deku Tree") as usize],
            Some(data::scenes::OOT_DEKU_TREE as u32),
            "the Deku Tree lobby resolves despite the dungeon also containing Gohma's lair"
        );

        let mut settings = Settings::default();
        settings.apply(&std::collections::HashSet::new());
        let mut inp =
            WorldInputs::build(&settings, &[WorldData::default()], 1, &Default::default(), true);

        let well = idx("Kakariko Well");
        let entry = idx("Bottom of the Well");
        let kak = scenes[well as usize].expect("Kakariko Well resolves to a scene");
        assert!(
            entrance_scene_pairs().contains(&(0, kak, botw)),
            "Kakariko <-> Bottom of the Well is a real entrance"
        );

        // Reached Kakariko, but not the well: the entrance is walled.
        inp.visited_scenes = [(0u8, kak)].into_iter().collect();
        assert!(
            Inputs::edge_blocked(&inp, well, entry),
            "the well entrance stays walled until the Bottom of the Well is discovered"
        );
        // Once discovered, it opens.
        inp.visited_scenes.insert((0, botw));
        assert!(
            !Inputs::edge_blocked(&inp, well, entry),
            "a discovered dungeon is crossable"
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

    /// Progressive discovery, Fix B (scene-less bridge gate): the entrance graph routes
    /// many loading zones through a *scene-less* waypoint region ("Near Swamp Spider
    /// House" → the spider house, the Song-of-Soaring hub → an owl statue, a bombable
    /// "Behind Large Icicles" approach → the mountain path). `edge_blocked` used to treat
    /// any edge touching a scene-less region as free, so crediting an item (explosives, a
    /// warp song) let those bridges spill into scenes the player had never walked into
    /// (reported: Path to Mountain Village and the Swamp Spider House showing un-entered).
    /// The hop from a scene-less waypoint INTO a real scene is the loading zone and must be
    /// walled until that scene is discovered.
    #[test]
    fn scene_less_waypoint_into_unvisited_scene_is_walled() {
        let mq = std::collections::HashSet::new();
        let mut settings = Settings::default();
        settings.apply(&mq);
        let mut inp = WorldInputs::build(&settings, &[WorldData::default()], 1, &Default::default(), true);

        let mm = |n: &str| region_name_index()[1].get(n).map(|v| v[0]);
        let scenes = region_scenes();
        let (Some(w), Some(t)) = (mm("Near Swamp Spider House"), mm("Swamp Spider House")) else {
            panic!("anchor regions must exist");
        };
        // Precondition: W is scene-less plumbing, T is the real spider-house scene, W → T.
        assert_eq!(scenes[w as usize], None, "the waypoint must be scene-less");
        assert_eq!(scenes[t as usize], Some(data::scenes::MM_SPIDER_HOUSE_SWAMP as u32));
        assert!(data::LOGIC_REGIONS[w as usize].exits.iter().any(|e| e.to == t),
            "the waypoint must bridge into the spider house");

        // Un-entered: the scene-less bridge into it is walled.
        assert!(Inputs::edge_blocked(&inp, w, t),
            "a scene-less waypoint must not bridge into an un-entered scene");
        // Once the player has actually walked into the spider house, the bridge opens.
        inp.visited_scenes.insert((1, data::scenes::MM_SPIDER_HOUSE_SWAMP as u32));
        assert!(!Inputs::edge_blocked(&inp, w, t), "a discovered scene is crossable");
    }

    /// Progressive discovery, Fix A (root through scene-less waypoints): `seed_from_visited`
    /// roots the region a discovered entrance actually lands in. When the entrance graph
    /// puts a scene-less waypoint ("Near Romani Ranch") between the `from_name` region and
    /// the arrival scene, a single-hop match missed it and the walked scene was never
    /// marked visited — so its own checks hid AND (with Fix B) its scene-less bridges would
    /// wrongly wall. Following scene-less connectors (but never another real scene, so an
    /// intra-scene barrier is still respected) roots the true landing scene.
    #[test]
    fn entrance_through_scene_less_waypoint_roots_its_scene() {
        let mq = std::collections::HashSet::new();
        let mut settings = Settings::default();
        settings.apply(&mq);
        let mut inp = WorldInputs::build(&settings, &[WorldData::default()], 1, &Default::default(), true);

        // The Milk Road → Romani Ranch loading zone lands behind the scene-less
        // "Near Romani Ranch" waypoint. Its `to_id` is the map key `lookup` matches.
        let id = data::entr::MM_ROMANI_RANCH_FROM_MILK_ROAD_ENTR;
        let ranch = data::scenes::MM_ROMANI_RANCH as u32;
        assert_eq!(
            crate::entrance::lookup(Game::Mm, id).map(|m| m.to_scene as u32),
            Some(ranch),
            "the anchor entrance must land in Romani Ranch",
        );

        let visited: std::collections::HashSet<(u8, u32)> = [(1u8, id)].into_iter().collect();
        inp.seed_from_visited(&visited);
        assert!(inp.visited_scenes.contains(&(1, ranch)),
            "the scene reached through the scene-less waypoint must be rooted as visited");
    }

    /// What a renewable location restocks for the shown player: the spoiler placement
    /// when the seed shuffled it, else its vanilla item (the spoiler lists only
    /// shuffled locations), and nothing when the placement is destined to another
    /// player. Starting items never count as a renewable source.
    #[test]
    fn renewable_sources_follow_spoiler_then_vanilla() {
        let mq = std::collections::HashSet::new();
        let mut settings = Settings::default();
        settings.apply(&mq);
        let shop = "MM Swamp Potion Shop Item 3"; // vanilla: Red Potion (MM)
        let build = |worlds: &[WorldData], player: u8| {
            WorldInputs::build(&settings, worlds, player, &Default::default(), false)
        };

        // Not in the spoiler (vanilla shop): the vanilla item.
        let vanilla = build(&[WorldData::default()], 1);
        assert_eq!(vanilla.renewable_item(shop), Some(data::iid::MM_POTION_RED));
        assert_eq!(vanilla.renewable_item("MM Clock Town Chest"), None, "a chest is not renewable");

        // Shuffled: the spoiler placement wins over the vanilla item.
        let mut shuffled = WorldData::default();
        shuffled.items.insert(shop.to_string(), "Milk".to_string());
        assert_eq!(build(std::slice::from_ref(&shuffled), 1).renewable_item(shop), Some(data::iid::SHARED_MILK));

        // Multiworld: a placement destined to player 2 restocks nothing for player 1,
        // and player 2 does not walk world 1's shop either.
        shuffled.dest.insert(shop.to_string(), 2);
        let worlds = [shuffled, WorldData::default()];
        assert_eq!(build(&worlds, 1).renewable_item(shop), None);
        assert_eq!(build(&worlds, 2).renewable_item(shop), Some(data::iid::MM_POTION_RED), "world 2's own vanilla shop");
    }

    /// A spoiler without `startingAgeOot` must still spawn the OoT starting age.
    /// `SPAWN` gates both ages on `setting(startingAgeOot, …)`, so a stable v32.3
    /// spoiler (which calls it `startingAge`) or no spoiler at all left every OoT
    /// check unreachable. Reported as "Kokiri Sword / bombs don't reveal the grass
    /// checks": the grass is what the sword should light first on a new game.
    #[test]
    fn missing_starting_age_oot_falls_back_to_old_key_then_child() {
        let mq = std::collections::HashSet::new();
        let loc = "OOT Kokiri Forest Kokiri Sword Chest";
        let mut world = WorldData::default();
        world.items.insert(loc.to_string(), "Kokiri Sword (OoT)".to_string());
        world.collected.insert((Game::Oot, obj_idx(Game::Oot, loc)));

        for (label, text) in [
            ("v32.3 key", Some("Settings
  startingAge: child
  progressiveSwordsOot: separate
")),
            ("no spoiler", None),
        ] {
            let mut settings = Settings::default();
            if let Some(t) = text {
                settings.parse_spoiler(t, &mq);
            }
            settings.apply(&mq);
            assert!(!settings.raw_settings.contains_key("startingAgeOot"), "{label}: precondition");

            let r = crate::logic::solve_world(&settings, std::slice::from_ref(&world), 1, &Default::default(), false);
            assert!(r.reachable(loc), "{label}: the child spawn is reachable");
            assert!(
                r.reachable("OOT Kokiri Forest Grass Child 1"),
                "{label}: the Kokiri Sword cuts the child Kokiri Forest grass"
            );
            assert!(
                !r.reachable("OOT Kokiri Forest Grass Adult 01"),
                "{label}: still child only — adult grass stays out of reach"
            );

            // Progressive entrances: the player's scene is seeded at the spawned age.
            let mut inp = WorldInputs::build(&settings, std::slice::from_ref(&world), 1, &Default::default(), true);
            inp.seed_scene(0, data::scenes::OOT_KOKIRI_FOREST as u32);
            let rp = crate::logic::solve::solve(&inp);
            assert!(
                rp.reachable("OOT Kokiri Forest Grass Child 1"),
                "{label}: progressive mode reveals the grass too"
            );
        }

        // An adult start in the old key is honoured, not overridden by the default.
        let mut adult = Settings::default();
        adult.parse_spoiler("Settings
  startingAge: adult
", &mq);
        adult.apply(&mq);
        let r = crate::logic::solve_world(&adult, &[WorldData::default()], 1, &Default::default(), false);
        assert!(!r.reachable(loc), "an adult start cannot open the child-only sword chest");
    }
}
