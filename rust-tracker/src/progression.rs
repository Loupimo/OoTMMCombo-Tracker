//! Faithful port of the Qt progression dashboard (Sources/UI/ProgressionTab.cpp).
//!
//! The Qt version tracks every collected item on a grid of icon widgets spread
//! over four pages (OoT / MM / Souls / Collectibles). We keep the same data
//! model — the generated `PROG_PAGES` — and replicate the runtime semantics
//! (counters, shared items, progressive stages, spoiler-derived totals,
//! starting items) in a single `rebuild` pass. The UI layer (main.rs) then
//! paints the grids and the detail panel from the resulting per-entry state,
//! the way `RefreshVisual` / `BuildLocationTree` do.
//!
//! The one impedance mismatch with the C++: there an object already carries an
//! `ItemInfo*` (numeric `ItemID`); here a collected object is keyed by its
//! `Location`, so we resolve `Location -> item name -> id` through the spoiler
//! and the generated `ITEM_BY_NAME_LC` table (a port of `FindItemByName`).

use std::collections::{HashMap, HashSet};

use crate::data::{self, ProgEntry};
use crate::scene::Game;
use crate::settings::Settings;
use crate::WorldData;

/// Live state of one dashboard entry (the mutable `ItemIconWidget` fields).
#[derive(Default, Clone)]
pub struct ProgState {
    /// At least one matching item collected (or a starting item).
    pub found: bool,
    /// Running total for counter entries (song notes, tokens, collectables).
    pub count: i32,
    /// Total needed to complete: the static `ProgEntry.max_count`, or — for
    /// spoiler-derived collectables — the tally of matching placements.
    pub max_count: i32,
    /// The player begins the run owning this item.
    pub is_starting: bool,
}

/// One flattened dashboard entry with its page/section coordinates.
pub struct FlatEntry {
    pub page: usize,
    pub section: usize,
    pub section_title: &'static str,
    pub entry: &'static ProgEntry,
}

/// A scene bucket of the detail-panel location tree.
pub struct LocScene {
    pub game: Game,
    pub title: String,
    pub leaves: Vec<LocLeaf>,
}

/// One object leaf under a scene bucket.
pub struct LocLeaf {
    pub game: Game,
    pub render_scene: u16,
    pub name: &'static str,
    pub collected: bool,
    /// Resolved map-icon path for this check's type (chest / pot / GS / …), shown
    /// ahead of the location name in the detail panel. `None` when the type has no
    /// icon (the caller then reserves the same width so names stay aligned).
    pub icon: Option<&'static str>,
}

pub struct Dashboard {
    /// Flattened entries in declaration order (page, then section, then entry).
    flat: Vec<FlatEntry>,
    /// item id -> flat indices whose `lookup_keys` contain it (declaration order).
    by_item: HashMap<u32, Vec<usize>>,
    /// Live per-entry state, aligned with `flat`.
    states: Vec<ProgState>,
    /// Whether the 'songs' setting shuffles notes individually (so a song widget
    /// reads as a counter). Resolved from the settings on every `rebuild`.
    songs_counter: bool,
    /// Selected sub-tab (page index) and selected entry (flat index).
    pub sub_tab: usize,
    pub selected: Option<usize>,
    /// Whether the detail location tree also lists uncollected placements.
    pub reveal: bool,
    /// Multiworld: the world (1-based) whose progression is shown. Placements are
    /// counted only when their destination world matches (ParseWorldLocations'
    /// TargetWorld). 1 = the local world (the only one for single / coop seeds).
    pub active_world: u8,
    /// Cached detail location tree (recomputed only when the inputs change).
    tree_cache: Vec<LocScene>,
    tree_cache_key: Option<usize>,
    tree_dirty: bool,
}

impl Dashboard {
    pub fn new() -> Self {
        // Flatten the generated pages into one indexable list.
        let mut flat = Vec::new();
        for (p, page) in data::PROG_PAGES.iter().enumerate() {
            for (s, sec) in page.sections.iter().enumerate() {
                for e in sec.entries {
                    flat.push(FlatEntry { page: p, section: s, section_title: sec.title, entry: e });
                }
            }
        }
        // Index every entry by the item ids it stands for (declaration order,
        // which the progressive-stage walk relies on).
        let mut by_item: HashMap<u32, Vec<usize>> = HashMap::new();
        for (i, fe) in flat.iter().enumerate() {
            for &k in fe.entry.lookup_keys {
                let list = by_item.entry(k).or_default();
                if list.last() != Some(&i) {
                    list.push(i);
                }
            }
        }
        let states = vec![ProgState::default(); flat.len()];
        Dashboard {
            flat,
            by_item,
            states,
            songs_counter: false,
            sub_tab: 0,
            selected: None,
            reveal: true,
            active_world: 1,
            tree_cache: Vec::new(),
            tree_cache_key: None,
            tree_dirty: true,
        }
    }

    /// Switch the world whose progression is shown (multiworld). Invalidates the
    /// detail tree; the caller re-runs `rebuild` to recompute the per-entry state.
    pub fn set_active_world(&mut self, w: u8) {
        if self.active_world != w {
            self.active_world = w;
            self.tree_dirty = true;
        }
    }

    pub fn flat(&self) -> &[FlatEntry] {
        &self.flat
    }

    pub fn state(&self, i: usize) -> &ProgState {
        &self.states[i]
    }

    // ── Rebuild (RebuildFromSceneObjects) ─────────────────────────────────────

    /// Recompute every entry's live state from all worlds' placements + collected
    /// sets + settings. Cheap enough to run whenever one of those changes.
    ///
    /// Multiworld (Qt "option b"): the active world's progression is every
    /// placement DESTINED to that world, wherever it is physically placed. So we
    /// scan every world and keep only the placements whose destination player
    /// matches `active_world` (1-based). Single / coop seeds have one world whose
    /// placements all default to destination 1, so this matches everything.
    pub fn rebuild(
        &mut self,
        worlds: &[WorldData],
        settings: &Settings,
        mq: &HashSet<(Game, u16)>,
    ) {
        self.songs_counter = settings.value("songs") == data::ShuffleSetting::all;
        self.tree_dirty = true; // collected / spoiler changed → detail tree stale

        // Reset. Spoiler-derived counters start at 0 (tallied below); static
        // counters keep their declared max.
        for (i, fe) in self.flat.iter().enumerate() {
            let e = fe.entry;
            self.states[i] = ProgState {
                found: false,
                count: 0,
                max_count: if e.max_from_spoiler { 0 } else { e.max_count },
                is_starting: false,
            };
        }

        // Starting items first, so the collected replay sees them as already
        // found and progressive items advance to the right stage.
        for i in 0..self.flat.len() {
            for &key in self.flat[i].entry.lookup_keys {
                if let Some(&c) = settings.starting_item_ids.get(&key) {
                    self.states[i].is_starting = true;
                    for _ in 0..c {
                        self.mark_found(i);
                    }
                    break;
                }
                // Progressive capacity upgrade: the starting section only lists the
                // base (bow / slingshot) with a count; a count >= 2 grants the first
                // upgrade, >= 3 the second (GetProgressiveUpgradeRequirement).
                if let Some((base, req)) = progressive_upgrade_requirement(key) {
                    if settings.starting_item_ids.get(&base).copied().unwrap_or(0) >= req {
                        self.states[i].is_starting = true;
                        self.mark_found(i);
                        break;
                    }
                }
            }
        }

        // Tally spoiler-derived totals: every active placement (across all worlds)
        // destined to the active world counts toward the max, collected or not.
        // The dedup key carries the physical world so the same coordinate in two
        // world clones counts twice (they are distinct placements).
        let mut seen = HashSet::new();
        for (wi, w) in worlds.iter().enumerate() {
            for (game, objs) in [(Game::Oot, data::OOT_OBJECTS), (Game::Mm, data::MM_OBJECTS)] {
                for o in objs {
                    if o.type_ == data::ObjectType::none
                        || !crate::tracking::object_active(o, game, mq)
                    {
                        continue;
                    }
                    let Some(name) = w.items.get(o.location) else { continue };
                    if dest_world(w, wi, o.location) != self.active_world {
                        continue;
                    }
                    let Some(id) = find_item_id(name) else { continue };
                    if !seen.insert((wi, game.idx(), o.object_id, o.render_scene, o.type_ as u8)) {
                        continue;
                    }
                    self.tally_max(id);
                }
            }
        }

        // Replay every collected placement across all worlds. Each pickup resolves
        // to a single object index (the tracker never records a paired duplicate),
        // so a plain walk of each world's collected set matches the C++ deduped
        // placement replay. A pickup destined to another player does not advance
        // the active world's progression.
        for (wi, w) in worlds.iter().enumerate() {
            for &(game, idx) in &w.collected {
                let o = &game.objects()[idx];
                let Some(name) = w.items.get(o.location) else { continue };
                if dest_world(w, wi, o.location) != self.active_world {
                    continue;
                }
                let Some(id) = find_item_id(name) else { continue };
                self.on_item_found(id, settings);
            }
        }
    }

    /// Whether an entry behaves as a counter right now (songs flip with the
    /// 'songs' setting; every other entry keeps its declared flag).
    fn effective_is_counter(&self, e: &ProgEntry) -> bool {
        if is_song_icon(e.icon) {
            self.songs_counter
        } else {
            e.is_counter
        }
    }

    /// MarkFound: flag the entry found and bump its counter when applicable.
    fn mark_found(&mut self, i: usize) {
        self.states[i].found = true;
        if self.effective_is_counter(self.flat[i].entry) {
            self.states[i].count += 1;
        }
    }

    /// TallySpoilerMax: bump the max of every spoiler-derived entry matching id.
    fn tally_max(&mut self, id: u32) {
        let Some(list) = self.by_item.get(&id) else { return };
        let idxs: Vec<usize> =
            list.iter().copied().filter(|&i| self.flat[i].entry.max_from_spoiler).collect();
        for i in idxs {
            self.states[i].max_count += 1;
        }
    }

    /// OnItemFound (add path): route a collected item id to the matching entries,
    /// honouring shared (propagate to every mirror) and progressive (advance one
    /// stage) semantics.
    fn on_item_found(&mut self, id: u32, settings: &Settings) {
        let Some(matches) = self.by_item.get(&id).cloned() else { return };
        let shared = item_can_be_shared(id) && settings.shared_item_ids.contains(&id);
        let progressive = settings.progressive_item_ids.contains(&id);

        if shared && progressive {
            // Shared progressive items (shields) live on both pages: advance each
            // page independently so every mirror moves in lockstep.
            for page in 0..data::PROG_PAGES.len() {
                let stages: Vec<usize> =
                    matches.iter().copied().filter(|&i| self.flat[i].page == page).collect();
                self.walk_stages(&stages);
            }
        } else if shared || !progressive {
            // Propagate to every mirror (shared) or mark every match the same way
            // (non-progressive: one widget per game, counters accumulate).
            for &i in &matches {
                self.mark_found(i);
            }
        } else {
            // Progressive only: advance the first not-yet-found stage.
            self.walk_stages(&matches);
        }
    }

    /// Advance the first not-yet-found stage of a declaration-ordered list.
    fn walk_stages(&mut self, stages: &[usize]) {
        for &i in stages {
            if !self.states[i].found {
                self.mark_found(i);
                return;
            }
        }
    }

    // ── Per-entry render queries (RefreshVisual) ─────────────────────────────

    /// Whether an entry is hidden by the ROM settings (any of its ids disabled).
    pub fn entry_hidden(&self, i: usize, settings: &Settings) -> bool {
        self.flat[i]
            .entry
            .lookup_keys
            .iter()
            .any(|k| settings.disabled_item_ids.contains(k))
    }

    /// Whether a page still has at least one visible entry (else its tab hides).
    pub fn page_visible(&self, page: usize, settings: &Settings) -> bool {
        self.flat
            .iter()
            .enumerate()
            .any(|(i, fe)| fe.page == page && !self.entry_hidden(i, settings))
    }

    /// Whether the entry's icon should light up (a "full set" song counter only
    /// completes once every note is gathered; everything else on first pickup).
    pub fn complete(&self, i: usize) -> bool {
        let e = self.flat[i].entry;
        let st = &self.states[i];
        if self.effective_is_counter(e) && e.max_count > 0 && !e.max_from_spoiler {
            st.count >= e.max_count
        } else {
            st.found
        }
    }

    /// The counter badge text ("count" or "count/total"), or None when hidden.
    pub fn badge_text(&self, i: usize) -> Option<String> {
        let e = self.flat[i].entry;
        let st = &self.states[i];
        if self.effective_is_counter(e) && st.count > 0 {
            Some(if st.max_count > 0 {
                format!("{}/{}", st.count, st.max_count)
            } else {
                st.count.to_string()
            })
        } else {
            None
        }
    }

    // ── Detail panel location tree (BuildLocationTree) ────────────────────────

    /// Toggle the "reveal uncollected placements" option (invalidates the tree).
    pub fn set_reveal(&mut self, v: bool) {
        if self.reveal != v {
            self.reveal = v;
            self.tree_dirty = true;
        }
    }

    /// Refresh the cached location tree of the selected entry when it went stale
    /// (selection / reveal / collected changed). Cheap no-op otherwise, so it can
    /// run every frame before the detail panel reads `tree`.
    pub fn ensure_tree(&mut self, worlds: &[WorldData], mq: &HashSet<(Game, u16)>) {
        if !self.tree_dirty && self.tree_cache_key == self.selected {
            return;
        }
        self.tree_cache = match self.selected {
            Some(i) => self.location_tree(i, worlds, mq),
            None => Vec::new(),
        };
        self.tree_cache_key = self.selected;
        self.tree_dirty = false;
    }

    /// The cached location tree (see `ensure_tree`).
    pub fn tree(&self) -> &[LocScene] {
        &self.tree_cache
    }

    /// Every placement of the selected entry's item(s) DESTINED to the active
    /// world (across all physical worlds), grouped by scene and sorted (OoT before
    /// MM, then by scene name; uncollected leaves first). A placement that lives in
    /// another world is tagged "World N —" so it reads apart from own-world ones.
    fn location_tree(
        &self,
        i: usize,
        worlds: &[WorldData],
        mq: &HashSet<(Game, u16)>,
    ) -> Vec<LocScene> {
        let e = self.flat[i].entry;
        let multiworld = worlds.len() > 1;
        // Bucket per (physical world, game, scene) so a foreign world's copy of a
        // scene is a distinct group from the active world's.
        let mut buckets: HashMap<(usize, usize, u16), LocScene> = HashMap::new();
        let mut seen = HashSet::new();

        for (wi, w) in worlds.iter().enumerate() {
            for (game, objs) in [(Game::Oot, data::OOT_OBJECTS), (Game::Mm, data::MM_OBJECTS)] {
                for (idx, o) in objs.iter().enumerate() {
                    if o.type_ == data::ObjectType::none
                        || !crate::tracking::object_active(o, game, mq)
                    {
                        continue;
                    }
                    let Some(name) = w.items.get(o.location) else { continue };
                    if dest_world(w, wi, o.location) != self.active_world {
                        continue;
                    }
                    let Some(id) = find_item_id(name) else { continue };
                    if !item_matches(e, id) {
                        continue;
                    }
                    let coll = w.collected.contains(&(game, idx));
                    if !coll && !self.reveal {
                        continue;
                    }
                    if !seen.insert((wi, game.idx(), o.object_id, o.render_scene, o.type_ as u8)) {
                        continue;
                    }
                    let bucket =
                        buckets.entry((wi, game.idx(), o.render_scene)).or_insert_with(|| {
                            let base = scene_name(game, o.render_scene);
                            let title = if multiworld && (wi as u8 + 1) != self.active_world {
                                format!("World {} — {}", wi + 1, base)
                            } else {
                                base
                            };
                            LocScene { game, title, leaves: Vec::new() }
                        });
                    bucket.leaves.push(LocLeaf {
                        game,
                        render_scene: o.render_scene,
                        name: o.name,
                        collected: coll,
                        icon: crate::scene::icon_path_for(o.map_icon, o.type_),
                    });
                }
            }
        }

        let mut out: Vec<LocScene> = buckets.into_values().collect();
        for s in &mut out {
            // Uncollected ("to-find") leaves first, then alphabetical.
            s.leaves.sort_by(|a, b| {
                (a.collected as u8, a.name.to_lowercase()).cmp(&(b.collected as u8, b.name.to_lowercase()))
            });
        }
        out.sort_by(|a, b| {
            (a.game.idx(), a.title.to_lowercase()).cmp(&(b.game.idx(), b.title.to_lowercase()))
        });
        out
    }
}

impl Default for Dashboard {
    fn default() -> Self {
        Self::new()
    }
}

// ── Free helpers (Items.cpp ports) ────────────────────────────────────────────

/// FindItemByName: resolve a spoiler item name to its internal (dev) item id.
/// Strips newlines and the "cloaked as …" wrapper, then matches case-insensitively.
pub fn find_item_id(name: &str) -> Option<u32> {
    let mut n = name.replace('\n', "");
    if n.contains("cloaked") {
        if let Some(p) = n.find('(') {
            n.truncate(p.saturating_sub(1));
        }
    }
    let key = n.trim().to_lowercase();
    if let Ok(idx) = data::ITEM_BY_NAME_LC.binary_search_by(|&(nm, _)| nm.cmp(key.as_str())) {
        return Some(data::ITEM_BY_NAME_LC[idx].1);
    }
    // Fallback: OoTMM spoilers name rusty keys by their internal door/location
    // string ("Rusty Key (Silo)"), while the tracker's item table uses the name
    // shown in-game ("Rusty Key (Lon Lon Silo)"). Map the spoiler-only spellings
    // onto the tracker id so a collected rusty key still shows in progression.
    rusty_key_alias(&key).or_else(|| clock_alias(&key))
}

/// Since OoTMM added an OoT clock, newer spoilers / logs disambiguate the Majora's
/// Mask clocks as "Clock (MM, Day 1)" where they used to read "Clock (Day 1)". The
/// item table only carries one spelling, so on a direct miss try the other form so
/// BOTH the old and new names resolve to the same MM clock. "Clock (OoT)" already
/// matches directly and is never rewritten. Only reached after `ITEM_BY_NAME_LC`
/// misses, so there is no ambiguity with whichever spelling the table itself uses.
fn clock_alias(key_lc: &str) -> Option<u32> {
    let lookup = |k: &str| {
        data::ITEM_BY_NAME_LC
            .binary_search_by(|&(nm, _)| nm.cmp(k))
            .ok()
            .map(|i| data::ITEM_BY_NAME_LC[i].1)
    };
    // New "clock (mm, X)" -> old "clock (X)".
    if let Some(inner) = key_lc.strip_prefix("clock (mm, ").and_then(|s| s.strip_suffix(')')) {
        return lookup(&format!("clock ({inner})"));
    }
    // Old "clock (X)" -> new "clock (mm, X)" (the OoT clock keeps its own name).
    if let Some(inner) = key_lc.strip_prefix("clock (").and_then(|s| s.strip_suffix(')')) {
        if !inner.eq_ignore_ascii_case("oot") {
            return lookup(&format!("clock (mm, {inner})"));
        }
    }
    None
}

/// Resolve the OoTMM spoiler spelling of a rusty key (lowercased) to the tracker
/// item id whose in-game name differs. Only the diverging names are listed; every
/// matching one already resolves through `ITEM_BY_NAME_LC`. Returns `None` for a
/// non-rusty-key string.
fn rusty_key_alias(key_lc: &str) -> Option<u32> {
    use data::iid::*;
    Some(match key_lc {
        // OoT — the tracker prefixes the region (Market / Kakariko / Lon Lon).
        "rusty key (treasure chest game)" => OOT_RUSTY_KEY_TREASURE_CHEST_GAME,
        "rusty key (hyrule castle)" => OOT_RUSTY_KEY_HYRULE_CASTLE,
        "rusty key (child bazaar)" => OOT_RUSTY_KEY_CHILD_BAZAAR,
        "rusty key (child potion shop)" => OOT_RUSTY_KEY_CHILD_POTION_SHOP,
        "rusty key (child shooting gallery)" => OOT_RUSTY_KEY_CHILD_SHOOTING_GALLERY,
        "rusty key (laboratory)" => OOT_RUSTY_KEY_LABORATORY,
        "rusty key (silo)" => OOT_RUSTY_KEY_SILO,
        "rusty key (ranch stable)" => OOT_RUSTY_KEY_RANCH_STABLE,
        "rusty key (ranch house)" => OOT_RUSTY_KEY_RANCH_HOUSE,
        "rusty key (adult shooting gallery)" => OOT_RUSTY_KEY_ADULT_SHOOTING_GALLERY,
        "rusty key (skulltula house)" => OOT_RUSTY_KEY_SKULLTULA_HOUSE,
        "rusty key (adult bazaar)" => OOT_RUSTY_KEY_ADULT_BAZAAR,
        "rusty key (adult potion shop)" => OOT_RUSTY_KEY_ADULT_POTION_SHOP,
        "rusty key (adult potion shop back)" => OOT_RUSTY_KEY_ADULT_POTION_SHOP_BACK,
        // MM.
        "rusty key (potion shop)" => MM_RUSTY_KEY_POTION_SHOP,
        "rusty key (swordsman school)" => MM_RUSTY_KEY_SWORDSMAN_SCHOOL,
        "rusty key (town archery)" => MM_RUSTY_KEY_TOWN_ARCHERY,
        "rusty key (swamp archery)" => MM_RUSTY_KEY_SWAMP_ARCHERY,
        "rusty key (observatory)" => MM_RUSTY_KEY_OBSERVATORY,
        "rusty key (blacksmith)" => MM_RUSTY_KEY_BLACKSMITH,
        "rusty key (music house)" => MM_RUSTY_KEY_MUSIC_HOUSE,
        "rusty key (oceanic laboratory)" => MM_RUSTY_KEY_LABORATORY,
        "rusty key (treasure game)" => MM_RUSTY_KEY_TREASURE_CHEST_GAME,
        "rusty key (romani's room)" => MM_RUSTY_KEY_RANCH_HOUSE_ROOM,
        "rusty key (kafei's room)" => MM_RUSTY_KEY_MAYOR_RESIDENCE_KAFEI,
        _ => return None,
    })
}

/// ItemInfo::CanBeShared for the given id (ITEMS is dense, id-ordered).
fn item_can_be_shared(id: u32) -> bool {
    let dense = (id as usize).checked_sub(1).and_then(|i| data::ITEMS.get(i));
    match dense {
        Some(d) if d.id == id => d.can_be_shared,
        _ => data::ITEMS.iter().find(|d| d.id == id).is_some_and(|d| d.can_be_shared),
    }
}

/// ItemMatchesWidget: an entry stands for the item if it lists the id directly,
/// or the id shares a progressive capacity family with one of its keys.
fn item_matches(e: &ProgEntry, id: u32) -> bool {
    e.lookup_keys.contains(&id) || e.lookup_keys.iter().any(|&k| items_share_family(id, k))
}

/// Deku stick / nut upgrade families the generated `PROGRESSIVE_FAMILIES` omits.
/// OoTMM places a progressive stick/nut upgrade under a SINGLE item name (every tier
/// is e.g. "Deku Stick Upgrade (OoT)"), so all placements resolve to the middle
/// tier's id. Their three prog entries (capacity / upgrade / second upgrade) don't
/// share a common base id either — the capacity id was removed from the upgrade
/// tiers to make the visual cope with the capacity × upgrade setting cross — so
/// without a family the detail panel shows the shared pool only under the middle
/// entry and "No Known Location" under the other two. Grouping [capacity, upgrade,
/// second upgrade] pools the locations under all three, exactly like the bow/quiver
/// and slingshot/bullet-bag families. Kept hand-written (not in the generated data)
/// so it survives a regeneration, and only consulted by `items_share_family` → the
/// location detail; the visual walk and starting-item logic are untouched.
const STICK_NUT_FAMILIES: &[[u32; 3]] = &[
    [data::iid::OOT_STICK_UPGRADE, data::iid::OOT_STICK_UPGRADE2, data::iid::OOT_STICK_UPGRADE3],
    [data::iid::MM_STICK_UPGRADE, data::iid::MM_STICK_UPGRADE2, data::iid::MM_STICK_UPGRADE3],
    [data::iid::OOT_NUT_UPGRADE, data::iid::OOT_NUT_UPGRADE2, data::iid::OOT_NUT_UPGRADE3],
    [data::iid::MM_NUT_UPGRADE, data::iid::MM_NUT_UPGRADE2, data::iid::MM_NUT_UPGRADE3],
];

/// ItemsShareProgressiveFamily: both ids in the same [base, up1, up2] family.
fn items_share_family(a: u32, b: u32) -> bool {
    data::PROGRESSIVE_FAMILIES
        .iter()
        .chain(STICK_NUT_FAMILIES.iter())
        .any(|f| f.contains(&a) && f.contains(&b))
}

/// GetProgressiveUpgradeRequirement: for an upgrade tier, its base id and the
/// base starting count that grants it (2 for the first upgrade, 3 for the second).
fn progressive_upgrade_requirement(id: u32) -> Option<(u32, u32)> {
    for f in data::PROGRESSIVE_FAMILIES {
        for pos in 1..3 {
            if f[pos] == id {
                return Some((f[0], (pos + 1) as u32));
            }
        }
    }
    None
}

/// The destination world (1-based) of a placement physically in world `wi`: the
/// spoiler's "Player N" prefix when present, else that world's own player.
/// Mirrors ParseWorldLocations' `TargetWorld = WorldIndex + 1` default.
fn dest_world(world: &WorldData, wi: usize, location: &str) -> u8 {
    world.dest.get(location).copied().unwrap_or((wi + 1) as u8)
}

/// The song-family icons whose counter behaviour follows the 'songs' setting.
fn is_song_icon(icon: &str) -> bool {
    matches!(
        icon,
        "song" | "song_green" | "song_red" | "song_blue" | "song_purple" | "song_orange" | "song_yellow"
    )
}

/// A scene's display name, or a hex fallback for an unknown id.
fn scene_name(game: Game, scene_id: u16) -> String {
    game.scenes()
        .iter()
        .find(|s| s.id == scene_id)
        .map(|s| s.name.to_string())
        .unwrap_or_else(|| format!("Scene {scene_id:#x}"))
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn resolves_known_item_names() {
        // A game-suffixed name and a suffix-less one both resolve.
        assert_eq!(find_item_id("Fairy Bow (OoT)"), Some(data::iid::OOT_BOW));
        assert_eq!(find_item_id("Zelda's Letter"), Some(data::iid::OOT_ZELDA_LETTER));
        // Case-insensitive suffix.
        assert_eq!(find_item_id("Boomerang (OOT)"), Some(data::iid::OOT_BOOMERANG));
        // Unknown -> None (the C++ synthesises an id=-1 placeholder we skip).
        assert_eq!(find_item_id("Definitely Not An Item"), None);
    }

    #[test]
    fn every_prog_icon_and_key_is_sane() {
        let d = Dashboard::new();
        assert!(!d.flat().is_empty());
        // Every lookup key resolves to a real item id.
        for fe in d.flat() {
            for &k in fe.entry.lookup_keys {
                assert!(
                    data::ITEMS.iter().any(|it| it.id == k),
                    "unknown item id {k:#x} in {}",
                    fe.entry.name
                );
            }
        }
    }

    /// Flat index of the first entry with the given display name.
    fn entry_by_name(d: &Dashboard, name: &str) -> usize {
        d.flat().iter().position(|fe| fe.entry.name == name).expect("entry exists")
    }

    /// Build a single local world from (location -> item) placements plus a
    /// collected set, for the tests below.
    fn one_world(
        items: &[(&str, &str)],
        dest: &[(&str, u8)],
        collected: &[(Game, usize)],
    ) -> Vec<WorldData> {
        let mut w = WorldData::default();
        for &(loc, it) in items {
            w.items.insert(loc.to_string(), it.to_string());
        }
        for &(loc, d) in dest {
            w.dest.insert(loc.to_string(), d);
        }
        w.collected = collected.iter().copied().collect();
        vec![w]
    }

    #[test]
    fn collecting_marks_the_matching_entry() {
        // Map an OoT object's location to "Fairy Bow (OoT)" and collect it.
        let obj = &data::OOT_OBJECTS[0];
        let worlds = one_world(&[(obj.location, "Fairy Bow (OoT)")], &[], &[(Game::Oot, 0)]);

        let mut d = Dashboard::new();
        d.rebuild(&worlds, &Settings::default(), &HashSet::new());

        let bow = entry_by_name(&d, "Fairy Bow");
        assert!(d.state(bow).found, "the bow widget should light up");
        assert!(d.complete(bow));
    }

    #[test]
    fn counter_entry_accumulates_per_placement() {
        // Two distinct GS placements both hold a Gold Skulltula Token: count == 2.
        let a = data::OOT_OBJECTS.iter().position(|o| o.type_ == data::ObjectType::gs).unwrap();
        let b = data::OOT_OBJECTS
            .iter()
            .enumerate()
            .position(|(i, o)| i != a && o.type_ == data::ObjectType::gs)
            .unwrap();

        let worlds = one_world(
            &[
                (data::OOT_OBJECTS[a].location, "Gold Skulltula Token"),
                (data::OOT_OBJECTS[b].location, "Gold Skulltula Token"),
            ],
            &[],
            &[(Game::Oot, a), (Game::Oot, b)],
        );

        let mut d = Dashboard::new();
        d.rebuild(&worlds, &Settings::default(), &HashSet::new());

        let gs = entry_by_name(&d, "Gold Skulltula Token");
        assert_eq!(d.state(gs).count, 2, "each collected token bumps the counter");
    }

    #[test]
    fn multiworld_routes_by_destination_player() {
        // Two GS coordinates. In world 1, coord `a` holds a token for player 1
        // (own, no prefix) and coord `b` holds one destined to player 2. Both are
        // collected in world 1. World 2 is present but physically empty.
        let a = data::OOT_OBJECTS.iter().position(|o| o.type_ == data::ObjectType::gs).unwrap();
        let b = data::OOT_OBJECTS
            .iter()
            .enumerate()
            .position(|(i, o)| i != a && o.type_ == data::ObjectType::gs)
            .unwrap();

        let mut w1 = WorldData::default();
        w1.items.insert(data::OOT_OBJECTS[a].location.to_string(), "Gold Skulltula Token".to_string());
        w1.items.insert(data::OOT_OBJECTS[b].location.to_string(), "Gold Skulltula Token".to_string());
        w1.dest.insert(data::OOT_OBJECTS[b].location.to_string(), 2u8);
        w1.collected.insert((Game::Oot, a));
        w1.collected.insert((Game::Oot, b));
        let worlds = vec![w1, WorldData::default()];

        let gs = |d: &Dashboard| entry_by_name(d, "Gold Skulltula Token");
        let mut d = Dashboard::new();

        // World 1 (default): only the own token (a) counts; b is sent to player 2.
        d.rebuild(&worlds, &Settings::default(), &HashSet::new());
        assert_eq!(d.state(gs(&d)).count, 1, "player-2 token excluded from world 1");

        // World 2: only the placement destined to player 2 (coord b) counts,
        // even though it physically lives in world 1 (Qt "option b").
        d.set_active_world(2);
        d.rebuild(&worlds, &Settings::default(), &HashSet::new());
        assert_eq!(d.state(gs(&d)).count, 1, "world 2 sees the token routed to it");
    }

    #[test]
    fn progressive_upgrade_requirements() {
        // Big Quiver needs 2 bows, Biggest Quiver 3.
        assert_eq!(
            progressive_upgrade_requirement(data::iid::OOT_QUIVER2),
            Some((data::iid::OOT_BOW, 2))
        );
        assert_eq!(
            progressive_upgrade_requirement(data::iid::OOT_QUIVER3),
            Some((data::iid::OOT_BOW, 3))
        );
        // The base itself is not an upgrade.
        assert_eq!(progressive_upgrade_requirement(data::iid::OOT_BOW), None);
    }

    #[test]
    fn progressive_ocarina_lights_stages_in_order() {
        // Two placements both holding a "Progressive Ocarina". Collecting the
        // first must light ONLY the Fairy Ocarina, not Ocarina of Time — the
        // regression being that the shared OOT_OCARINA id sits in both widgets'
        // lookup keys and, when the ocarina wasn't treated as progressive, lit
        // them both on the first pickup.
        let gs: Vec<usize> = data::OOT_OBJECTS
            .iter()
            .enumerate()
            .filter(|(_, o)| o.type_ == data::ObjectType::gs)
            .map(|(i, _)| i)
            .take(2)
            .collect();
        let (a, b) = (gs[0], gs[1]);
        let places = [
            (data::OOT_OBJECTS[a].location, "Progressive Ocarina (OoT)"),
            (data::OOT_OBJECTS[b].location, "Progressive Ocarina (OoT)"),
        ];
        // apply() derives progressive_item_ids (incl. the always-progressive ocarina).
        let mut settings = Settings::default();
        settings.apply(&HashSet::new());

        let mut d = Dashboard::new();
        let fairy = entry_by_name(&d, "Fairy Ocarina");
        let time = entry_by_name(&d, "Ocarina of Time"); // first match = the OoT page

        // Only the first placement collected.
        d.rebuild(&one_world(&places, &[], &[(Game::Oot, a)]), &settings, &HashSet::new());
        assert!(d.state(fairy).found, "first Progressive Ocarina lights Fairy Ocarina");
        assert!(!d.state(time).found, "and not Ocarina of Time yet (progressive walk)");

        // Both collected → the second advances to Ocarina of Time.
        d.rebuild(
            &one_world(&places, &[], &[(Game::Oot, a), (Game::Oot, b)]),
            &settings,
            &HashSet::new(),
        );
        assert!(d.state(time).found, "second Progressive Ocarina lights Ocarina of Time");
    }

    #[test]
    fn resolves_rusty_key_spoiler_alias() {
        // The spoiler names rusty keys by their internal door/location string,
        // which differs from the tracker's in-game item name; the alias bridges
        // them so a collected rusty key still resolves (and shows in progression).
        assert_eq!(find_item_id("Rusty Key (Silo)"), Some(data::iid::OOT_RUSTY_KEY_SILO));
        assert_eq!(
            find_item_id("Rusty Key (Swordsman School)"),
            Some(data::iid::MM_RUSTY_KEY_SWORDSMAN_SCHOOL)
        );
        // A name already matching the item table still resolves through it.
        assert_eq!(find_item_id("Rusty Key (Windmill)"), Some(data::iid::OOT_RUSTY_KEY_WINDMILL));
    }

    #[test]
    fn resolves_clock_both_formats() {
        use data::iid::*;
        // Old ("Clock (Night 3)") and new ("Clock (MM, Night 3)") MM clock spellings
        // must resolve to the same item, whichever the current data table uses.
        assert_eq!(find_item_id("Clock (Night 3)"), Some(MM_CLOCK6));
        assert_eq!(find_item_id("Clock (MM, Night 3)"), Some(MM_CLOCK6));
        assert_eq!(find_item_id("Clock (Day 1)"), Some(MM_CLOCK1));
        assert_eq!(find_item_id("Clock (MM, Day 1)"), Some(MM_CLOCK1));
        // Case-insensitive, like the rest of the resolver.
        assert_eq!(find_item_id("clock (mm, night 1)"), Some(MM_CLOCK2));
        // The OoT clock keeps its own name and is never rewritten to an MM clock.
        assert_eq!(find_item_id("Clock (OoT)"), Some(OOT_CLOCK));
        // A genuinely unknown clock stays unresolved.
        assert_eq!(find_item_id("Clock (MM, Night 9)"), None);
    }

    #[test]
    fn stick_nut_upgrade_family_pools_locations() {
        use data::iid::*;
        // OoTMM places the whole progressive stick/nut chain under the middle tier's
        // id (e.g. every stick placement is "Deku Stick Upgrade (OoT)" = 0x77). The
        // family must make the capacity and second-upgrade entries — which list only
        // their own tier — still match that placement id, so the detail panel pools
        // the locations under all three instead of showing "No Known Location".
        let entry = |keys: &'static [u32]| ProgEntry {
            icon: "stick",
            name: "x",
            lookup_keys: keys,
            is_counter: false,
            max_count: 0,
            max_from_spoiler: false,
        };
        let cap = entry(&[OOT_STICK_UPGRADE, SHARED_STICK_UPGRADE]);
        let second = entry(&[OOT_STICK_UPGRADE3, SHARED_STICK_UPGRADE]);
        assert!(item_matches(&cap, OOT_STICK_UPGRADE2), "capacity pools the middle-tier placement");
        assert!(item_matches(&second, OOT_STICK_UPGRADE2), "second upgrade pools it too");
        // MM nuts behave the same.
        let mm_nut_cap = entry(&[MM_NUT_UPGRADE, SHARED_NUT_UPGRADE]);
        assert!(item_matches(&mm_nut_cap, MM_NUT_UPGRADE2));
        assert!(item_matches(&mm_nut_cap, MM_NUT_UPGRADE3));
        // Families don't bleed across item type or game.
        assert!(!item_matches(&mm_nut_cap, OOT_STICK_UPGRADE2));
        assert!(!item_matches(&cap, MM_STICK_UPGRADE2));
    }
}
