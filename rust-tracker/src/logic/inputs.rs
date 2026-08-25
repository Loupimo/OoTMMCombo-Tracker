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
    /// `TRICK_NAMES` indices enabled by the seed.
    tricks: HashSet<u32>,
    /// Per game (`[OoT, MM]`) the song index placed at each event slot (indexed by
    /// slot), from the spoiler's `Song Events` section. `u8::MAX` = unresolved.
    song_events: [Vec<u8>; 2],
    /// Custom win-condition definitions (`special(X)`), keyed by SPECIAL name.
    special_conds: std::collections::HashMap<String, crate::settings::SpecialCond>,
    /// Entrance-randomizer edge redirects: `(from_region, vanilla_to_region)` ->
    /// shuffled destination region index(es). Empty when the seed has no ER.
    exit_redirects: HashMap<(u32, u32), Vec<u32>>,
}

impl WorldInputs {
    /// Build the inputs for one player's reachability. `settings` must already be
    /// `apply`-ed (so `starting_item_ids` is populated). `worlds` are all physical
    /// worlds and `player` is the 1-based player whose map is shown: their
    /// inventory is every collected check, across all worlds, whose item is
    /// destined to that player (multiworld routing). Single / coop seeds have one
    /// world whose placements default to player 1, so this counts everything.
    pub fn build(settings: &Settings, worlds: &[WorldData], player: u8) -> Self {
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
        for (name, val) in &settings.raw_settings {
            let Some(&ki) = skeys.get(name.as_str()) else { continue };
            if val.eq_ignore_ascii_case("true") {
                settings_enabled.insert(ki);
            }
            if let Some(&vi) = svals.get(val.as_str()) {
                settings_value.insert(ki, vi);
            }
        }

        // Tricks: id string -> TRICK_NAMES index.
        let tri = trick_index();
        let tricks = settings
            .enabled_trick_ids
            .iter()
            .filter_map(|id| tri.get(id).copied())
            .collect();

        WorldInputs {
            items,
            masks,
            settings_enabled,
            settings_value,
            tricks,
            song_events: settings.song_events.clone(),
            special_conds: settings.special_conds.clone(),
            exit_redirects: build_exit_redirects(&settings.entrance_remap),
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

/// Turn the spoiler's name-based entrance remaps into solver edge redirects: for
/// each `from -> via` vanilla edge in the region graph, redirect it to the shuffled
/// destination region(s). Dungeon destinations resolve to every layout variant; the
/// solver keeps only the active one.
fn build_exit_redirects(
    remaps: &[crate::settings::EntranceRemap],
) -> HashMap<(u32, u32), Vec<u32>> {
    let names = region_name_index();
    let mut out: HashMap<(u32, u32), Vec<u32>> = HashMap::new();
    for m in remaps {
        let Some(dests) = names[m.dest_game as usize].get(m.dest.as_str()) else { continue };
        let Some(froms) = names[m.game as usize].get(m.from.as_str()) else { continue };
        for &fi in froms {
            for e in data::LOGIC_REGIONS[fi as usize].exits {
                let to = &data::LOGIC_REGIONS[e.to as usize];
                if to.game == m.game && to.name == m.via {
                    out.insert((fi, e.to), dests.clone());
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
    fn setting_value(&self, key: u32) -> Option<u32> {
        self.settings_value.get(&key).copied()
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

        let inp = WorldInputs::build(&settings, std::slice::from_ref(&world), 1);

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
        let inp = WorldInputs::build(&settings, &[WorldData::default()], 1);

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
        let inp = WorldInputs::build(&settings, &[WorldData::default()], 1);
        let red = inp.exit_redirects().expect("redirects present");
        assert_eq!(red.get(&(kokiri, lost)).map(|v| v.contains(&zora)), Some(true));

        // A no-entrance seed leaves the redirect map empty (vanilla graph).
        let mut vanilla = Settings::default();
        vanilla.parse_spoiler("Settings\n  startingAge: child\n", &mq);
        vanilla.apply(&mq);
        assert!(WorldInputs::build(&vanilla, &[WorldData::default()], 1).exit_redirects().is_none());
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
        assert_eq!(WorldInputs::build(&settings, &worlds, 1).item_count(sword), 0);
        assert_eq!(WorldInputs::build(&settings, &worlds, 2).item_count(sword), 1);
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
        assert!(!WorldInputs::build(&settings, &[WorldData::default()], 1).special(bridge));

        // One medallion -> still short.
        settings.starting_item_ids.insert(iid::OOT_MEDALLION_FOREST, 1);
        assert!(!WorldInputs::build(&settings, &[WorldData::default()], 1).special(bridge));

        // Two medallions -> satisfied.
        settings.starting_item_ids.insert(iid::OOT_MEDALLION_FIRE, 1);
        let inp = WorldInputs::build(&settings, &[WorldData::default()], 1);
        assert!(inp.special(bridge));

        // LACS requires a mask category the tracker does not model -> optimistic.
        assert!(inp.special(lacs));
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
        let r0 = super::super::solve_world(&settings, std::slice::from_ref(&empty), 1);

        let loc = "OOT Kokiri Forest Kokiri Sword Chest";
        let mut world = WorldData::default();
        world.items.insert(loc.to_string(), "Kokiri Sword (OoT)".to_string());
        world.collected.insert((Game::Oot, obj_idx(Game::Oot, loc)));
        let r1 = super::super::solve_world(&settings, std::slice::from_ref(&world), 1);

        for l in &r0.locations {
            assert!(r1.locations.contains(l), "{l} lost after collecting a check");
        }
        assert!(r1.locations.len() >= r0.locations.len());
    }
}
