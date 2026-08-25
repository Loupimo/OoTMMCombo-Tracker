//! ROM build settings (port of Settings.cpp).
//!
//! The spoiler-parsed / user-edited parameters that decide which objects render
//! on the map (via per-render-type shuffle settings) and which items the
//! progression tab tracks. `Settings` owns the current parameter values and the
//! derived item-id sets; `apply` rebuilds the excluded-object set and the
//! progression sets from those values.

use std::collections::{HashMap, HashSet};

use crate::data::{
    iid, scenes as s, ObjectDef, ObjectType, ParamCategory, ShuffleSetting, FILTER_SETTINGS,
    ITEM_SETTINGS,
};
use crate::scene::Game;

/// Mirror of ROMGame (Settings.h).
#[derive(Clone, Copy, PartialEq, Eq, Debug)]
pub enum RomGame {
    Oot,
    Mm,
    OotMm,
}

impl RomGame {
    pub const ALL: [RomGame; 3] = [RomGame::Oot, RomGame::Mm, RomGame::OotMm];
    pub fn label(self) -> &'static str {
        match self {
            RomGame::Oot => "Ocarina of Time",
            RomGame::Mm => "Majora's Mask",
            RomGame::OotMm => "OoTMM (combo)",
        }
    }
}

/// Mirror of GameMode (Settings.h).
#[derive(Clone, Copy, PartialEq, Eq, Debug)]
pub enum GameMode {
    Single,
    Coop,
    Multi,
}

impl GameMode {
    pub const ALL: [GameMode; 3] = [GameMode::Single, GameMode::Coop, GameMode::Multi];
    pub fn label(self) -> &'static str {
        match self {
            GameMode::Single => "Solo",
            GameMode::Coop => "Coop",
            GameMode::Multi => "Multiworld",
        }
    }
}

/// Mirror of GoalMode (Settings.h).
#[derive(Clone, Copy, PartialEq, Eq, Debug)]
pub enum GoalMode {
    Boss,
    Triforce3,
    Triforce,
}

impl GoalMode {
    pub const ALL: [GoalMode; 3] = [GoalMode::Boss, GoalMode::Triforce3, GoalMode::Triforce];
    pub fn label(self) -> &'static str {
        match self {
            GoalMode::Boss => "Boss (Ganon / Majora)",
            GoalMode::Triforce3 => "Triforce Hunt",
            GoalMode::Triforce => "Triforce Pieces",
        }
    }
}

/// The OoT dungeons with a Master Quest variant: (label, scene id).
pub const OOT_MQ_DUNGEONS: &[(&str, u16)] = &[
    ("Deku Tree", s::OOT_DEKU_TREE),
    ("Dodongo's Cavern", s::OOT_DODONGO_CAVERN),
    ("Jabu-Jabu", s::OOT_INSIDE_JABU_JABU),
    ("Forest Temple", s::OOT_TEMPLE_FOREST),
    ("Fire Temple", s::OOT_TEMPLE_FIRE),
    ("Water Temple", s::OOT_TEMPLE_WATER),
    ("Shadow Temple", s::OOT_TEMPLE_SHADOW),
    ("Spirit Temple", s::OOT_TEMPLE_SPIRIT),
    ("Bottom of the Well", s::OOT_BOTTOM_OF_THE_WELL),
    ("Ice Cavern", s::OOT_ICE_CAVERN),
    ("Gerudo Training Grounds", s::OOT_GERUDO_TRAINING_GROUND),
    ("Ganon's Castle", s::OOT_INSIDE_GANON_CASTLE),
];

/// The MM scenes swapped to the JP layout by the Deku Palace toggle.
pub const MM_JP_SCENES: &[u16] = &[
    s::MM_GROTTOS,
    s::MM_DEKU_PALACE,
    s::MM_GROTTO_DEKU_PALACE_GENERIC,
    s::MM_GROTTO_DEKU_PALACE_CLIMB,
];

/// A dungeon's small-key / key-ring item id pair (SettingsTab OoTKeyRings /
/// MMKeyRings). The World Items editor toggles between individual small keys and
/// a single ring by moving these ids in / out of the disabled set.
pub struct KeyRing {
    pub label: &'static str,
    pub small: u32,
    pub ring: u32,
}

/// OoT small-key / key-ring pairs, keyed by the spoiler dungeon label.
pub const KEY_RINGS_OOT: &[KeyRing] = &[
    KeyRing { label: "Forest Temple", small: iid::OOT_SMALL_KEY_FOREST, ring: iid::OOT_KEY_RING_FOREST },
    KeyRing { label: "Fire Temple", small: iid::OOT_SMALL_KEY_FIRE, ring: iid::OOT_KEY_RING_FIRE },
    KeyRing { label: "Water Temple", small: iid::OOT_SMALL_KEY_WATER, ring: iid::OOT_KEY_RING_WATER },
    KeyRing { label: "Shadow Temple", small: iid::OOT_SMALL_KEY_SHADOW, ring: iid::OOT_KEY_RING_SHADOW },
    KeyRing { label: "Spirit Temple", small: iid::OOT_SMALL_KEY_SPIRIT, ring: iid::OOT_KEY_RING_SPIRIT },
    KeyRing { label: "Bottom of the Well", small: iid::OOT_SMALL_KEY_BOTW, ring: iid::OOT_KEY_RING_BOTW },
    KeyRing { label: "Gerudo Training Grounds", small: iid::OOT_SMALL_KEY_GTG, ring: iid::OOT_KEY_RING_GTG },
    KeyRing { label: "Ganon's Castle", small: iid::OOT_SMALL_KEY_GANON, ring: iid::OOT_KEY_RING_GANON },
    KeyRing { label: "Hideout", small: iid::OOT_SMALL_KEY_GF, ring: iid::OOT_KEY_RING_GF },
    KeyRing { label: "Chest Game", small: iid::OOT_SMALL_KEY_TCG, ring: iid::OOT_KEY_RING_TCG },
];

/// MM small-key / key-ring pairs, keyed by the spoiler dungeon label.
pub const KEY_RINGS_MM: &[KeyRing] = &[
    KeyRing { label: "Woodfall Temple", small: iid::MM_SMALL_KEY_WF, ring: iid::MM_KEY_RING_WF },
    KeyRing { label: "Snowhead Temple", small: iid::MM_SMALL_KEY_SH, ring: iid::MM_KEY_RING_SH },
    KeyRing { label: "Great Bay Temple", small: iid::MM_SMALL_KEY_GB, ring: iid::MM_KEY_RING_GB },
    KeyRing { label: "Stone Tower Temple", small: iid::MM_SMALL_KEY_ST, ring: iid::MM_KEY_RING_ST },
];

/// A silver-rupee cluster: its rupee / pouch ids, the scene it lives in and the
/// layout that owns it (`layout`: 0 = Vanilla only, 1 = Master Quest only,
/// 2 = Both). Mirrors the OoTSilverPouches table in the Qt SettingsTab.
pub struct SilverArea {
    pub label: &'static str,
    pub rupee: u32,
    pub pouch: u32,
    pub scene: u16,
    pub layout: u8,
}

/// Per-area silver-rupee / silver-pouch descriptors, gated by the scene layout.
pub const SILVER_AREAS: &[SilverArea] = &[
    SilverArea { label: "Dodongo's Cavern", rupee: iid::OOT_RUPEE_SILVER_DC, pouch: iid::OOT_POUCH_SILVER_DC, scene: s::OOT_DODONGO_CAVERN, layout: 1 },
    SilverArea { label: "Bottom of the Well", rupee: iid::OOT_RUPEE_SILVER_BOTW, pouch: iid::OOT_POUCH_SILVER_BOTW, scene: s::OOT_BOTTOM_OF_THE_WELL, layout: 0 },
    SilverArea { label: "Spirit Temple (Child)", rupee: iid::OOT_RUPEE_SILVER_SPIRIT_CHILD, pouch: iid::OOT_POUCH_SILVER_SPIRIT_CHILD, scene: s::OOT_TEMPLE_SPIRIT, layout: 0 },
    SilverArea { label: "Spirit Temple (Sun)", rupee: iid::OOT_RUPEE_SILVER_SPIRIT_SUN, pouch: iid::OOT_POUCH_SILVER_SPIRIT_SUN, scene: s::OOT_TEMPLE_SPIRIT, layout: 0 },
    SilverArea { label: "Spirit Temple (Boulders)", rupee: iid::OOT_RUPEE_SILVER_SPIRIT_BOULDERS, pouch: iid::OOT_POUCH_SILVER_SPIRIT_BOULDERS, scene: s::OOT_TEMPLE_SPIRIT, layout: 0 },
    SilverArea { label: "Spirit Temple (Lobby)", rupee: iid::OOT_RUPEE_SILVER_SPIRIT_LOBBY, pouch: iid::OOT_POUCH_SILVER_SPIRIT_LOBBY, scene: s::OOT_TEMPLE_SPIRIT, layout: 1 },
    SilverArea { label: "Spirit Temple (Adult)", rupee: iid::OOT_RUPEE_SILVER_SPIRIT_ADULT, pouch: iid::OOT_POUCH_SILVER_SPIRIT_ADULT, scene: s::OOT_TEMPLE_SPIRIT, layout: 1 },
    SilverArea { label: "Shadow Temple (Scythe)", rupee: iid::OOT_RUPEE_SILVER_SHADOW_SCYTHE, pouch: iid::OOT_POUCH_SILVER_SHADOW_SCYTHE, scene: s::OOT_TEMPLE_SHADOW, layout: 2 },
    SilverArea { label: "Shadow Temple (Pit)", rupee: iid::OOT_RUPEE_SILVER_SHADOW_PIT, pouch: iid::OOT_POUCH_SILVER_SHADOW_PIT, scene: s::OOT_TEMPLE_SHADOW, layout: 2 },
    SilverArea { label: "Shadow Temple (Spikes)", rupee: iid::OOT_RUPEE_SILVER_SHADOW_SPIKES, pouch: iid::OOT_POUCH_SILVER_SHADOW_SPIKES, scene: s::OOT_TEMPLE_SHADOW, layout: 2 },
    SilverArea { label: "Shadow Temple (Blades)", rupee: iid::OOT_RUPEE_SILVER_SHADOW_BLADES, pouch: iid::OOT_POUCH_SILVER_SHADOW_BLADES, scene: s::OOT_TEMPLE_SHADOW, layout: 1 },
    SilverArea { label: "Ice Cavern (Scythe)", rupee: iid::OOT_RUPEE_SILVER_IC_SCYTHE, pouch: iid::OOT_POUCH_SILVER_IC_SCYTHE, scene: s::OOT_ICE_CAVERN, layout: 0 },
    SilverArea { label: "Ice Cavern (Block)", rupee: iid::OOT_RUPEE_SILVER_IC_BLOCK, pouch: iid::OOT_POUCH_SILVER_IC_BLOCK, scene: s::OOT_ICE_CAVERN, layout: 0 },
    SilverArea { label: "GTG (Slopes)", rupee: iid::OOT_RUPEE_SILVER_GTG_SLOPES, pouch: iid::OOT_POUCH_SILVER_GTG_SLOPES, scene: s::OOT_GERUDO_TRAINING_GROUND, layout: 2 },
    SilverArea { label: "GTG (Lava)", rupee: iid::OOT_RUPEE_SILVER_GTG_LAVA, pouch: iid::OOT_POUCH_SILVER_GTG_LAVA, scene: s::OOT_GERUDO_TRAINING_GROUND, layout: 2 },
    SilverArea { label: "GTG (Water)", rupee: iid::OOT_RUPEE_SILVER_GTG_WATER, pouch: iid::OOT_POUCH_SILVER_GTG_WATER, scene: s::OOT_GERUDO_TRAINING_GROUND, layout: 2 },
    SilverArea { label: "Ganon's Castle (Light)", rupee: iid::OOT_RUPEE_SILVER_GANON_LIGHT, pouch: iid::OOT_POUCH_SILVER_GANON_LIGHT, scene: s::OOT_INSIDE_GANON_CASTLE, layout: 0 },
    SilverArea { label: "Ganon's Castle (Forest)", rupee: iid::OOT_RUPEE_SILVER_GANON_FOREST, pouch: iid::OOT_POUCH_SILVER_GANON_FOREST, scene: s::OOT_INSIDE_GANON_CASTLE, layout: 0 },
    SilverArea { label: "Ganon's Castle (Fire)", rupee: iid::OOT_RUPEE_SILVER_GANON_FIRE, pouch: iid::OOT_POUCH_SILVER_GANON_FIRE, scene: s::OOT_INSIDE_GANON_CASTLE, layout: 2 },
    SilverArea { label: "Ganon's Castle (Water)", rupee: iid::OOT_RUPEE_SILVER_GANON_WATER, pouch: iid::OOT_POUCH_SILVER_GANON_WATER, scene: s::OOT_INSIDE_GANON_CASTLE, layout: 1 },
    SilverArea { label: "Ganon's Castle (Shadow)", rupee: iid::OOT_RUPEE_SILVER_GANON_SHADOW, pouch: iid::OOT_POUCH_SILVER_GANON_SHADOW, scene: s::OOT_INSIDE_GANON_CASTLE, layout: 1 },
    SilverArea { label: "Ganon's Castle (Spirit)", rupee: iid::OOT_RUPEE_SILVER_GANON_SPIRIT, pouch: iid::OOT_POUCH_SILVER_GANON_SPIRIT, scene: s::OOT_INSIDE_GANON_CASTLE, layout: 0 },
];

/// A pre-activated owl statue choice (SettingsTab MMOwlStatues): checking one
/// adds the owl as an owned starting item.
pub struct OwlStatue {
    pub label: &'static str,
    pub id: u32,
}

/// The MM owl statues that can start pre-activated.
pub const OWL_STATUES: &[OwlStatue] = &[
    OwlStatue { label: "Clock Town", id: iid::MM_OWL_CLOCK_TOWN },
    OwlStatue { label: "Milk Road", id: iid::MM_OWL_MILK_ROAD },
    OwlStatue { label: "Southern Swamp", id: iid::MM_OWL_SOUTHERN_SWAMP },
    OwlStatue { label: "Woodfall", id: iid::MM_OWL_WOODFALL },
    OwlStatue { label: "Mountain Village", id: iid::MM_OWL_MOUNTAIN_VILLAGE },
    OwlStatue { label: "Snowhead", id: iid::MM_OWL_SNOWHEAD },
    OwlStatue { label: "Great Bay Coast", id: iid::MM_OWL_GREAT_BAY },
    OwlStatue { label: "Zora Cape", id: iid::MM_OWL_ZORA_CAPE },
    OwlStatue { label: "Ikana Canyon", id: iid::MM_OWL_IKANA_CANYON },
    OwlStatue { label: "Stone Tower", id: iid::MM_OWL_STONE_TOWER },
];

/// The objects excluded from the map by the ROM settings, as global indices into
/// `game.objects()`, per game (FilterManager::ExcludedObj).
#[derive(Default, Clone)]
pub struct Excluded {
    pub oot: HashSet<usize>,
    pub mm: HashSet<usize>,
}

impl Excluded {
    fn set(&mut self, game: Game) -> &mut HashSet<usize> {
        match game {
            Game::Oot => &mut self.oot,
            Game::Mm => &mut self.mm,
        }
    }

    /// Whether the object at `idx` in `game` is excluded.
    pub fn contains(&self, game: Game, idx: usize) -> bool {
        match game {
            Game::Oot => &self.oot,
            Game::Mm => &self.mm,
        }
        .contains(&idx)
    }
}

/// The full ROM settings state (mirror of the Settings class).
pub struct Settings {
    pub game: RomGame,
    pub mode: GameMode,
    pub goal: GoalMode,
    pub num_teams: usize,
    pub fire_temple_open_as_child: bool,
    /// Current shuffle value per parameter key (both filter and item settings).
    values: HashMap<&'static str, ShuffleSetting>,
    /// Item flags parsed from the spoiler world sections, which persist across a
    /// SettingsTab re-apply (key rings, silver pouches, pre-activated owls).
    base_disabled: HashSet<u32>,
    base_starting: HashMap<u32, u32>,
    // Derived by `apply`, consumed by the progression tab.
    pub disabled_item_ids: HashSet<u32>,
    pub shared_item_ids: HashSet<u32>,
    pub progressive_item_ids: HashSet<u32>,
    pub starting_item_ids: HashMap<u32, u32>,
    /// Raw `key: value` of the spoiler's `Settings` section (every key, verbatim),
    /// the source of truth for the reachability logic's `setting(k)` / `setting(k, v)`.
    /// Kept alongside `values` (which only carries the tracker's filter settings).
    pub raw_settings: HashMap<String, String>,
    /// Trick ids enabled by the seed (into `data::TRICK_NAMES`), parsed from the
    /// spoiler's `Tricks` / `Glitches` sections — the logic's `trick(id)` source.
    pub enabled_trick_ids: HashSet<&'static str>,
    /// Per game (`[OoT, MM]`) the song placed at each event slot, indexed by slot:
    /// the song index (0..=19, the macro song ordering) the spoiler's `Song Events`
    /// section assigns to that slot, or `u8::MAX` for an unresolved name. Drives
    /// the logic's `_song_event_<game>(slot, song)`.
    pub song_events: [Vec<u8>; 2],
    /// Custom win-condition definitions (`special(X)`), keyed by the SPECIAL name
    /// (BRIDGE / MOON / LACS / GANON_BK / MAJORA), from the spoiler's
    /// `Special Conditions` section. Only consulted when the seed sets the matching
    /// setting to `custom`.
    pub special_conds: HashMap<String, SpecialCond>,
    /// Shuffled entrances (from the spoiler's `Entrances` section): each remaps a
    /// vanilla region edge to a new destination for the reachability solver. Empty
    /// when the seed has no entrance randomizer.
    pub entrance_remap: Vec<EntranceRemap>,
}

/// One custom win-condition: the token threshold (`count`) and which item
/// categories count toward it (the `true` flags in the spoiler block).
#[derive(Default, Clone)]
pub struct SpecialCond {
    pub count: u32,
    /// Enabled category keys, e.g. `medallions`, `stones`, `skullsGold`.
    pub cats: Vec<String>,
}

/// One shuffled entrance (from the spoiler's `Entrances` section): the vanilla
/// edge `from -> via` (both in game `game`) now leads to region `dest` in game
/// `dest_game` instead. The reachability solver redirects that graph edge.
#[derive(Clone)]
pub struct EntranceRemap {
    pub game: u8,
    pub from: String,
    pub via: String,
    pub dest_game: u8,
    pub dest: String,
}

impl Default for Settings {
    fn default() -> Self {
        let mut values = HashMap::new();
        for m in FILTER_SETTINGS.iter().chain(ITEM_SETTINGS.iter()) {
            values.insert(m.key, m.default);
        }
        Settings {
            game: RomGame::OotMm,
            mode: GameMode::Single,
            goal: GoalMode::Boss,
            num_teams: 1,
            fire_temple_open_as_child: false,
            values,
            base_disabled: HashSet::new(),
            base_starting: HashMap::new(),
            disabled_item_ids: HashSet::new(),
            shared_item_ids: HashSet::new(),
            progressive_item_ids: HashSet::new(),
            starting_item_ids: HashMap::new(),
            raw_settings: HashMap::new(),
            enabled_trick_ids: HashSet::new(),
            song_events: [Vec::new(), Vec::new()],
            special_conds: HashMap::new(),
            entrance_remap: Vec::new(),
        }
    }
}

impl Settings {
    // ── Value accessors ──────────────────────────────────────────────────────

    /// The current shuffle value of a parameter (falls back to its default).
    pub fn value(&self, key: &str) -> ShuffleSetting {
        self.values.get(key).copied().unwrap_or(ShuffleSetting::vanilla)
    }

    /// Set a parameter's shuffle value (used by the SettingsTab editors).
    pub fn set_value(&mut self, key: &'static str, v: ShuffleSetting) {
        self.values.insert(key, v);
    }

    fn is_filter_key(key: &str) -> bool {
        FILTER_SETTINGS.iter().any(|m| m.key == key)
    }
    fn is_item_key(key: &str) -> bool {
        ITEM_SETTINGS.iter().any(|m| m.key == key)
    }

    // ── World Items editors (SettingsTab per-dungeon toggles) ────────────────
    // These edit the `base_*` sets that survive a re-`apply` (mirror of the Qt
    // SettingsTab checkboxes that write DisabledItemIDs / StartingItemIDs).

    /// Whether a dungeon's key ring is delivered (ring id enabled).
    pub fn key_ring_on(&self, ring: u32) -> bool {
        !self.base_disabled.contains(&ring)
    }

    /// Toggle a dungeon's key ring: on => keep the ring, drop the small keys.
    pub fn set_key_ring(&mut self, small: u32, ring: u32, on: bool) {
        if on {
            self.base_disabled.insert(small);
            self.base_disabled.remove(&ring);
        } else {
            self.base_disabled.remove(&small);
            self.base_disabled.insert(ring);
        }
    }

    /// Whether a silver cluster exists in its scene's currently active layout.
    pub fn silver_area_exists(&self, area: &SilverArea, mq: &HashSet<(Game, u16)>) -> bool {
        let is_mq = mq.contains(&(Game::Oot, area.scene));
        area.layout == 2 || (area.layout == 1 && is_mq) || (area.layout == 0 && !is_mq)
    }

    /// Whether a silver cluster currently delivers a pouch (pouch id enabled).
    pub fn silver_pouch_on(&self, pouch: u32) -> bool {
        !self.base_disabled.contains(&pouch)
    }

    /// Toggle a silver cluster: on => a pouch, off => the individual rupees.
    pub fn set_silver_pouch(&mut self, rupee: u32, pouch: u32, on: bool) {
        if on {
            self.base_disabled.insert(rupee);
            self.base_disabled.remove(&pouch);
        } else {
            self.base_disabled.remove(&rupee);
            self.base_disabled.insert(pouch);
        }
    }

    /// Whether an owl statue starts pre-activated (owned as a starting item).
    pub fn owl_on(&self, owl: u32) -> bool {
        self.base_starting.contains_key(&owl)
    }

    /// Toggle a pre-activated owl statue.
    pub fn set_owl(&mut self, owl: u32, on: bool) {
        if on {
            self.base_starting.insert(owl, 1);
        } else {
            self.base_starting.remove(&owl);
        }
    }

    // ── Spoiler parsing (ParseSettings + world flags) ────────────────────────

    /// Parse the settings / world / starting-item sections of a spoiler log,
    /// resetting to defaults first. `mq` is the Master-Quest scene set already
    /// resolved by the spoiler parser (used by the silver-pouch layout logic).
    pub fn parse_spoiler(&mut self, text: &str, mq: &HashSet<(Game, u16)>) {
        *self = Settings::default();

        for section in split_sections(text) {
            if section.starts_with("Settings") {
                // Two-space indented "key: value" lines.
                for line in section.lines() {
                    let body = match line.strip_prefix("  ") {
                        Some(b) if !b.starts_with(' ') => b,
                        _ => continue,
                    };
                    if let Some((k, v)) = body.split_once(": ") {
                        let (k, v) = (k.trim(), v.trim());
                        // Keep every raw value for the reachability logic; the
                        // tracker's own filter/item settings are also folded in.
                        self.raw_settings.insert(k.to_string(), v.to_string());
                        self.add_setting(k, v);
                    }
                }
            } else if section.starts_with("Tricks") || section.starts_with("Glitches") {
                // Each `  <display name>` line names one enabled trick / glitch.
                for line in section.lines() {
                    let name = match line.strip_prefix("  ") {
                        Some(n) if !n.starts_with(' ') => n.trim(),
                        _ => continue,
                    };
                    if let Some(id) = resolve_trick_name(name) {
                        self.enabled_trick_ids.insert(id);
                    }
                }
            } else if section.starts_with("Song Events") {
                self.parse_song_events(&section);
            } else if section.starts_with("Special Conditions") {
                self.parse_special_conds(&section);
            } else if section.starts_with("Entrances") {
                self.parse_entrances(&section);
            } else if section.starts_with("World") {
                self.parse_key_rings(&section);
                self.parse_silver_pouches(&section, mq);
                self.parse_open_dungeons_oot(&section);
                self.parse_pre_activated_owl(&section);
            } else if section.starts_with("Starting Items") {
                self.parse_starting_items(&section);
            }
        }
    }

    /// ParseStartingItems: read the "Starting Items" section into `base_starting`
    /// (item id -> starting count). Handles the flat (single / coop) layout and
    /// the per-player multiworld layout, folding every world into the combined
    /// map — the per-world split is a multiworld refinement left unported.
    fn parse_starting_items(&mut self, section: &str) {
        // Multiworld spoilers prefix each block with a "  Player N" header; the
        // items then sit one extra indent level in (four spaces).
        let multiworld = section.lines().any(|l| {
            l.strip_prefix("  Player ")
                .and_then(|r| r.chars().next())
                .is_some_and(|c| c.is_ascii_digit())
        });
        let indent = if multiworld { "    " } else { "  " };

        for line in section.lines() {
            let body = match line.strip_prefix(indent) {
                Some(b) if !b.starts_with(' ') => b,
                _ => continue,
            };
            let Some((name, count)) = body.split_once(": ") else { continue };
            let Ok(count) = count.trim().parse::<u32>() else { continue };
            if let Some(id) = crate::progression::find_item_id(name.trim()) {
                *self.base_starting.entry(id).or_insert(0) += count;
            }
        }
    }

    /// ParseSongEvents: read the spoiler's `Song Events` section into
    /// `song_events`. The section groups slots by game (`  Ocarina of Time` /
    /// `  Majora's Mask` sub-headers) and lists `    SONG_EVENT_X : Song` in enum
    /// order, so the push order IS the slot id the logic's `_song_event_<game>`
    /// uses. An unresolved song name is stored as `u8::MAX` (treated as "any" =
    /// optimistic) to keep the slot alignment intact.
    fn parse_song_events(&mut self, section: &str) {
        let mut game: Option<usize> = None;
        for line in section.lines() {
            if let Some(rest) = line.strip_prefix("    ") {
                // `SONG_EVENT_X            : Song Name`
                let Some((_, song)) = rest.split_once(':') else { continue };
                let n = resolve_song_name(song.trim()).unwrap_or(u8::MAX);
                if let Some(g) = game {
                    self.song_events[g].push(n);
                }
            } else if let Some(rest) = line.strip_prefix("  ") {
                let head = rest.trim();
                game = if head.starts_with("Ocarina") {
                    Some(0)
                } else if head.starts_with("Majora") {
                    Some(1)
                } else {
                    game
                };
            }
        }
    }

    /// ParseSpecialConds: read the spoiler's `Special Conditions` section into
    /// `special_conds`. Each `  NAME:` block holds `    count: N` and a set of
    /// `    category: true|false` flags; we keep the count and the enabled
    /// categories, which the logic's `special(NAME)` sums against `count`.
    fn parse_special_conds(&mut self, section: &str) {
        let mut cur: Option<String> = None;
        for line in section.lines() {
            if let Some(rest) = line.strip_prefix("    ") {
                let Some((k, v)) = rest.split_once(':') else { continue };
                let (k, v) = (k.trim(), v.trim());
                let Some(name) = cur.as_ref() else { continue };
                let cond = self.special_conds.entry(name.clone()).or_default();
                if k == "count" {
                    cond.count = v.parse().unwrap_or(0);
                } else if v == "true" {
                    cond.cats.push(k.to_string());
                }
            } else if let Some(rest) = line.strip_prefix("  ") {
                cur = Some(rest.trim().trim_end_matches(':').to_string());
            }
        }
    }

    /// ParseEntrances: read the spoiler's `Entrances` section into `entrance_remap`.
    /// Each line is `<A> to <B> (SRC) -> <C> from <D> (DST)`, meaning the vanilla
    /// entrance edge `A -> B` now leads to region `C`. Region names carry their
    /// `OOT `/`MM ` game prefix; names themselves can contain ` to `/` from `, so
    /// the A|B and C|D splits anchor on the game prefix that begins the right half.
    fn parse_entrances(&mut self, section: &str) {
        for line in section.lines() {
            let Some((left, right)) = line.split_once("->") else { continue };
            // Drop the trailing ` (ENTRANCE_ID)` on each half (names have no ` (`).
            let left = left.trim().rsplit_once(" (").map_or(left.trim(), |(n, _)| n);
            let right = right.trim().rsplit_once(" (").map_or(right.trim(), |(n, _)| n);
            let (Some((a, b)), Some((c, _d))) =
                (split_on_prefixed(left, " to "), split_on_prefixed(right, " from "))
            else {
                continue;
            };
            let (Some((game, from)), Some((bg, via)), Some((dest_game, dest))) =
                (strip_game(a), strip_game(b), strip_game(c))
            else {
                continue;
            };
            // A and B are the same vanilla world; guard against a malformed split.
            if game != bg {
                continue;
            }
            self.entrance_remap.push(EntranceRemap {
                game,
                from: from.to_string(),
                via: via.to_string(),
                dest_game,
                dest: dest.to_string(),
            });
        }
    }

    /// AddSetting: map a spoiler `key: value` pair onto a parameter value or a
    /// top-level field (game / mode / goal / teams).
    fn add_setting(&mut self, name: &str, value: &str) {
        match name {
            "games" => {
                self.game = match value {
                    "oot" => RomGame::Oot,
                    "mm" => RomGame::Mm,
                    _ => RomGame::OotMm,
                };
            }
            "mode" => {
                self.mode = match value {
                    "coop" => GameMode::Coop,
                    "multi" => GameMode::Multi,
                    _ => GameMode::Single,
                };
            }
            "goal" => {
                self.goal = match value {
                    "triforce3" => GoalMode::Triforce3,
                    "triforce" => GoalMode::Triforce,
                    _ => GoalMode::Boss,
                };
            }
            "teams" => {
                self.num_teams = value.parse().unwrap_or(1);
            }
            "songs" => {
                let v = if value == "notes" {
                    ShuffleSetting::all
                } else {
                    ShuffleSetting::vanilla
                };
                self.set_value("songs", v);
            }
            _ if Self::is_filter_key(name) => {
                let v = filter_value(value);
                self.set_value(filter_key(name), v);
            }
            _ if Self::is_item_key(name) => {
                let v = item_value(value);
                self.set_value(item_key(name), v);
            }
            _ => {}
        }
    }

    // ── World flags ──────────────────────────────────────────────────────────

    /// ParseKeyRings: disable every small key / key ring by default, then
    /// re-enable the rings the seed rolled (all, or a per-dungeon list).
    fn parse_key_rings(&mut self, section: &str) {
        for i in iid::OOT_KEY_RING_FOREST..=iid::OOT_KEY_RING_GTG {
            self.base_disabled.insert(i);
        }
        self.base_disabled.insert(iid::OOT_KEY_RING_TCG);
        for i in iid::MM_KEY_RING_WF..=iid::MM_KEY_RING_ST {
            self.base_disabled.insert(i);
        }

        let enable_all = |slf: &mut Self, rings: &[KeyRing]| {
            for r in rings {
                slf.base_disabled.insert(r.small);
                slf.base_disabled.remove(&r.ring);
            }
        };

        if let Some(list) = read_list(section, "Small Key Ring (OoT)") {
            match list {
                ListValue::Inline(v) if v == "all" => {
                    enable_all(self, KEY_RINGS_OOT);
                    if self.value("smallKeyShuffleChestGame") != ShuffleSetting::vanilla {
                        self.base_disabled.insert(iid::OOT_SMALL_KEY_TCG);
                        self.base_disabled.remove(&iid::OOT_KEY_RING_TCG);
                    }
                }
                ListValue::Items(items) => self.enable_rings(&items, KEY_RINGS_OOT),
                _ => {}
            }
        }
        if let Some(list) = read_list(section, "Small Key Ring (MM)") {
            match list {
                ListValue::Inline(v) if v == "all" => enable_all(self, KEY_RINGS_MM),
                ListValue::Items(items) => self.enable_rings(&items, KEY_RINGS_MM),
                _ => {}
            }
        }
    }

    fn enable_rings(&mut self, items: &[String], rings: &[KeyRing]) {
        for name in items {
            if let Some(r) = rings.iter().find(|r| r.label == name) {
                self.base_disabled.insert(r.small);
                self.base_disabled.remove(&r.ring);
            }
        }
    }

    /// ParseSilverPouches: disable every silver rupee and pouch, then re-enable
    /// each cluster that exists in its scene's active layout, as a pouch when the
    /// seed selected it, otherwise as individual rupees.
    fn parse_silver_pouches(&mut self, section: &str, mq: &HashSet<(Game, u16)>) {
        for i in iid::OOT_RUPEE_SILVER_DC..=iid::OOT_RUPEE_SILVER_GANON_WATER {
            self.base_disabled.insert(i);
        }
        for i in iid::OOT_POUCH_SILVER_DC..=iid::OOT_POUCH_SILVER_GANON_WATER {
            self.base_disabled.insert(i);
        }

        let mut all_pouches = false;
        let mut pouch_areas: HashSet<String> = HashSet::new();
        if let Some(list) = read_list(section, "Silver Rupee Pouches") {
            match list {
                ListValue::Inline(v) if v == "all" => all_pouches = true,
                ListValue::Items(items) => pouch_areas.extend(items),
                _ => {}
            }
        }

        for a in SILVER_AREAS {
            if !self.silver_area_exists(a, mq) {
                continue;
            }
            if all_pouches || pouch_areas.contains(a.label) {
                self.base_disabled.remove(&a.pouch);
            } else {
                self.base_disabled.remove(&a.rupee);
            }
        }
    }

    /// ParseOpenDungeonsOoT: only the "Fire Temple as Child" flag is tracked.
    fn parse_open_dungeons_oot(&mut self, section: &str) {
        if let Some(list) = read_list(section, "Open Dungeons (OoT)") {
            match list {
                ListValue::Inline(v) if v == "all" => self.fire_temple_open_as_child = true,
                ListValue::Items(items) => {
                    if items.iter().any(|i| i == "Fire Temple as Child") {
                        self.fire_temple_open_as_child = true;
                    }
                }
                _ => {}
            }
        }
    }

    /// ParsePreActivatedOwl: pre-activated MM owl statues start as owned items.
    fn parse_pre_activated_owl(&mut self, section: &str) {
        if let Some(list) = read_list(section, "Pre-Activated Owl Statues") {
            match list {
                ListValue::Inline(v) if v == "all" => {
                    for i in iid::MM_OWL_GREAT_BAY..=iid::MM_OWL_STONE_TOWER {
                        self.base_starting.insert(i, 1);
                    }
                }
                ListValue::Items(items) => {
                    for name in items {
                        if let Some(o) = OWL_STATUES.iter().find(|o| o.label == name) {
                            self.base_starting.insert(o.id, 1);
                        }
                    }
                }
                _ => {}
            }
        }
    }

    // ── Apply (ApplySettings) ────────────────────────────────────────────────

    /// Rebuild the excluded-object set and the progression item sets from the
    /// current parameter values. `mq` gates the per-scene layout (Master Quest).
    pub fn apply(&mut self, mq: &HashSet<(Game, u16)>) -> Excluded {
        let mut excluded = Excluded::default();
        // Seed the derived sets from what the spoiler world sections rolled.
        self.disabled_item_ids = self.base_disabled.clone();
        self.starting_item_ids = self.base_starting.clone();
        self.shared_item_ids.clear();
        self.progressive_item_ids.clear();

        match self.game {
            RomGame::Oot => self.apply_oot(&mut excluded),
            RomGame::Mm => self.apply_mm(&mut excluded),
            RomGame::OotMm => {
                self.apply_oot(&mut excluded);
                self.apply_mm(&mut excluded);
            }
        }
        self.apply_item_settings();
        let _ = mq; // MQ already reflected via LiveScene layout filtering.
        excluded
    }

    /// CheckObjectExclusion: exclude the object when its shuffle setting hides it.
    fn exclude(&self, excluded: &mut Excluded, game: Game, idx: usize, o: &ObjectDef, setting: ShuffleSetting) {
        let hide = match setting {
            ShuffleSetting::removed | ShuffleSetting::vanilla => true,
            ShuffleSetting::overworld => o.loc_type != crate::data::LocType::overworld,
            ShuffleSetting::dungeons => o.loc_type != crate::data::LocType::dungeon,
            _ => false,
        };
        if hide {
            excluded.set(game).insert(idx);
        }
    }

    /// ApplyOoTSettingsToFilter.
    fn apply_oot(&mut self, excluded: &mut Excluded) {
        use ObjectType as T;
        let game = Game::Oot;
        for (idx, o) in game.objects().iter().enumerate() {
            match o.render_type {
                T::gs => self.exclude(excluded, game, idx, o, self.value("goldSkulltulaTokens")),
                T::map | T::compass => self.exclude(excluded, game, idx, o, self.value("mapCompassShuffle")),
                T::small_key => {
                    let key = if o.scene == s::OOT_THIEVES_HIDEOUT {
                        "smallKeyShuffleHideout"
                    } else if o.loc_type == crate::data::LocType::minigame {
                        "smallKeyShuffleChestGame"
                    } else {
                        "smallKeyShuffleOot"
                    };
                    self.exclude(excluded, game, idx, o, self.value(key));
                }
                T::boss_key => {
                    let key = if o.scene == s::OOT_GANON_TOWER {
                        "ganonBossKey"
                    } else {
                        "bossKeyShuffleOot"
                    };
                    self.exclude(excluded, game, idx, o, self.value(key));
                }
                T::sr => self.exclude(excluded, game, idx, o, self.value("silverRupeeShuffle")),
                T::scrub => self.exclude(excluded, game, idx, o, self.value("scrubShuffleOot")),
                T::cow => self.exclude(excluded, game, idx, o, self.value("cowShuffleOot")),
                T::shop => self.exclude(excluded, game, idx, o, self.value("shopShuffleOot")),
                T::pot => self.exclude(excluded, game, idx, o, self.value("shufflePotsOot")),
                T::crate_ => self.exclude(excluded, game, idx, o, self.value("shuffleCratesOot")),
                T::hive => self.exclude(excluded, game, idx, o, self.value("shuffleHivesOot")),
                T::grass => self.exclude(excluded, game, idx, o, self.value("shuffleGrassOot")),
                T::rock => self.exclude(excluded, game, idx, o, self.value("shuffleRocksOot")),
                T::tree => self.exclude(excluded, game, idx, o, self.value("shuffleTreesOot")),
                T::bush => self.exclude(excluded, game, idx, o, self.value("shuffleBushOot")),
                T::soil => self.exclude(excluded, game, idx, o, self.value("shuffleSoilOot")),
                T::rupee => {
                    let key = if o.loc_type == crate::data::LocType::minigame {
                        "divingGameRupeeShuffle"
                    } else {
                        "shuffleFreeRupeesOot"
                    };
                    self.exclude(excluded, game, idx, o, self.value(key));
                }
                T::heart => self.exclude(excluded, game, idx, o, self.value("shuffleFreeHeartsOot")),
                T::wonder => self.exclude(excluded, game, idx, o, self.value("shuffleWonderItemsOot")),
                T::butterfly => self.exclude(excluded, game, idx, o, self.value("shuffleButterfliesOot")),
                T::boulder => self.exclude(excluded, game, idx, o, self.value("shuffleBouldersOot")),
                T::silverboulder => {
                    // Death Mountain Crater silver boulder only appears as child and
                    // needs Silver / Golden Gauntlets.
                    if o.scene == s::OOT_DEATH_MOUNTAIN_CRATER
                        && (self.value("agelessStrength") != ShuffleSetting::all
                            || self.fire_temple_open_as_child)
                    {
                        self.exclude(excluded, game, idx, o, ShuffleSetting::vanilla);
                    } else {
                        self.exclude(excluded, game, idx, o, self.value("shuffleSilverBouldersOot"));
                    }
                }
                T::redboulder => self.exclude(excluded, game, idx, o, self.value("shuffleRedBouldersOot")),
                T::icicle => self.exclude(excluded, game, idx, o, self.value("shuffleIciclesOot")),
                T::redice => self.exclude(excluded, game, idx, o, self.value("shuffleRedIceOot")),
                T::ocarina => self.exclude(excluded, game, idx, o, self.value("shuffleOcarinasOot")),
                T::sword => self.exclude(excluded, game, idx, o, self.value("shuffleMasterSword")),
                T::mask => self.exclude(excluded, game, idx, o, self.value("shuffleMaskTrades")),
                T::merchant => self.exclude(excluded, game, idx, o, self.value("shuffleMerchantsOot")),
                T::fish => self.exclude(excluded, game, idx, o, self.value("pondFishShuffle")),
                T::fairy => self.exclude(excluded, game, idx, o, self.value("fairyFountainFairyShuffleOot")),
                T::fairy_spot => self.exclude(excluded, game, idx, o, self.value("fairySpotShuffleOot")),
                T::egg => self.exclude(excluded, game, idx, o, self.value("eggShuffle")),
                T::npc => {
                    match o.map_icon {
                        "card" => self.exclude(excluded, game, idx, o, self.value("shuffleGerudoCard")),
                        "frog" => {
                            if o.loc_type == crate::data::LocType::minigame {
                                self.exclude(excluded, game, idx, o, self.value("shuffleFrogsRupeesOot"));
                            }
                        }
                        "letter" => self.exclude(excluded, game, idx, o, self.skip_zelda_setting()),
                        "gold_rupee" => self.exclude(excluded, game, idx, o, self.value("shuffleSkulltulaFinalReward")),
                        _ => {}
                    }
                }
                T::song => {
                    if o.render_scene == s::OOT_CASTLE_COURTYARD {
                        self.exclude(excluded, game, idx, o, self.skip_zelda_setting());
                    }
                }
                _ => {}
            }
        }

        // Restore Broken Actors off -> hide broken actors in the affected scenes.
        if self.value("restoreBrokenActors") != ShuffleSetting::all {
            const BROKEN_SCENES: [u16; 3] =
                [s::OOT_DODONGO_CAVERN, s::OOT_HYRULE_CASTLE, s::OOT_LAKE_HYLIA];
            for (idx, o) in game.objects().iter().enumerate() {
                if o.loc_type == crate::data::LocType::broken && BROKEN_SCENES.contains(&o.scene) {
                    excluded.set(game).insert(idx);
                }
            }
        }
    }

    /// Zelda's Letter / Song: shown only when Skip Zelda is off (inverted).
    fn skip_zelda_setting(&self) -> ShuffleSetting {
        if self.value("skipZelda") == ShuffleSetting::vanilla {
            ShuffleSetting::all
        } else {
            ShuffleSetting::vanilla
        }
    }

    /// ApplyMMSettingsToFilter.
    fn apply_mm(&mut self, excluded: &mut Excluded) {
        use ObjectType as T;
        let game = Game::Mm;
        for (idx, o) in game.objects().iter().enumerate() {
            match o.render_type {
                T::gs => self.exclude(excluded, game, idx, o, self.value("housesSkulltulaTokens")),
                T::map => {
                    if o.type_ == T::npc {
                        self.exclude(excluded, game, idx, o, self.value("tingleShuffle"));
                    } else {
                        // Fall-through to compass in the C++.
                        self.exclude(excluded, game, idx, o, self.value("mapCompassShuffle"));
                    }
                }
                T::compass => self.exclude(excluded, game, idx, o, self.value("mapCompassShuffle")),
                T::small_key => self.exclude(excluded, game, idx, o, self.value("smallKeyShuffleMm")),
                T::boss_key => self.exclude(excluded, game, idx, o, self.value("bossKeyShuffleMm")),
                T::sf => {
                    let key = if o.object_id == iid::STRAY_FAIRY_TOWN {
                        "townFairyShuffle"
                    } else if o.type_ == T::chest {
                        "strayFairyChestShuffle"
                    } else {
                        "strayFairyOtherShuffle"
                    };
                    self.exclude(excluded, game, idx, o, self.value(key));
                    // Starting stray fairies (setting == starting) add to the pool.
                    if o.object_id != iid::STRAY_FAIRY_TOWN
                        && self.value(key) == ShuffleSetting::starting
                    {
                        if let Some(fairy) = mm_stray_fairy_for_scene(o.scene) {
                            *self.base_starting.entry(fairy).or_insert(0) += 1;
                        }
                    }
                }
                T::scrub => self.exclude(excluded, game, idx, o, self.value("scrubShuffleMm")),
                T::cow => self.exclude(excluded, game, idx, o, self.value("cowShuffleMm")),
                T::shop => self.exclude(excluded, game, idx, o, self.value("shopShuffleMm")),
                T::owl => self.exclude(excluded, game, idx, o, self.value("owlShuffle")),
                T::pot => self.exclude(excluded, game, idx, o, self.value("shufflePotsMm")),
                T::crate_ => self.exclude(excluded, game, idx, o, self.value("shuffleCratesMm")),
                T::barrel => self.exclude(excluded, game, idx, o, self.value("shuffleBarrelsMm")),
                T::hive => self.exclude(excluded, game, idx, o, self.value("shuffleHivesMm")),
                T::rock => self.exclude(excluded, game, idx, o, self.value("shuffleRocksMm")),
                T::grass => {
                    let key = if o.scene == s::MM_TERMINA_FIELD {
                        "shuffleTFGrassMm"
                    } else {
                        "shuffleGrassMm"
                    };
                    self.exclude(excluded, game, idx, o, self.value(key));
                }
                T::tree => self.exclude(excluded, game, idx, o, self.value("shuffleTreesMm")),
                T::bush => self.exclude(excluded, game, idx, o, self.value("shuffleBushMm")),
                T::soil => self.exclude(excluded, game, idx, o, self.value("shuffleSoilMm")),
                T::rupee => self.exclude(excluded, game, idx, o, self.value("shuffleFreeRupeesMm")),
                T::heart => self.exclude(excluded, game, idx, o, self.value("shuffleFreeHeartsMm")),
                T::wonder => self.exclude(excluded, game, idx, o, self.value("shuffleWonderItemsMm")),
                T::snowball => self.exclude(excluded, game, idx, o, self.value("shuffleSnowballsMm")),
                T::butterfly => self.exclude(excluded, game, idx, o, self.value("shuffleButterfliesMm")),
                T::boulder => self.exclude(excluded, game, idx, o, self.value("shuffleBouldersMm")),
                T::redboulder => self.exclude(excluded, game, idx, o, self.value("shuffleRedBouldersMm")),
                T::icicle => self.exclude(excluded, game, idx, o, self.value("shuffleIciclesMm")),
                T::merchant => self.exclude(excluded, game, idx, o, self.value("shuffleMerchantsMm")),
                T::fairy => self.exclude(excluded, game, idx, o, self.value("fairyFountainFairyShuffleMm")),
                T::npc => {
                    if o.scene == s::MM_LOTTERY {
                        self.exclude(excluded, game, idx, o, self.value("shuffleLotteryMm"));
                    }
                }
                _ => {}
            }
        }
    }

    /// ApplyItemSettings: derive disabled / shared / progressive item ids from
    /// the goal and the item-setting parameter values.
    fn apply_item_settings(&mut self) {
        match self.goal {
            GoalMode::Triforce => {
                self.disabled_item_ids.insert(iid::OOT_TRIFORCE_POWER);
                self.disabled_item_ids.insert(iid::OOT_TRIFORCE_COURAGE);
                self.disabled_item_ids.insert(iid::OOT_TRIFORCE_WISDOM);
            }
            GoalMode::Triforce3 => {
                self.disabled_item_ids.insert(iid::OOT_TRIFORCE);
                self.disabled_item_ids.insert(iid::OOT_TRIFORCE_FULL);
            }
            GoalMode::Boss => {
                self.disabled_item_ids.insert(iid::OOT_TRIFORCE);
                self.disabled_item_ids.insert(iid::OOT_TRIFORCE_FULL);
                self.disabled_item_ids.insert(iid::OOT_TRIFORCE_POWER);
                self.disabled_item_ids.insert(iid::OOT_TRIFORCE_COURAGE);
                self.disabled_item_ids.insert(iid::OOT_TRIFORCE_WISDOM);
            }
        }

        if self.value("mapCompassShuffle") == ShuffleSetting::removed {
            self.disable_range(iid::OOT_MAP_DT, iid::OOT_COMPASS_IC);
            self.disable_range(iid::MM_MAP_WF, iid::MM_COMPASS_ST);
        }
        if self.value("tingleShuffle") == ShuffleSetting::removed {
            self.disable_range(iid::MM_WORLD_MAP_CLOCK_TOWN, iid::MM_WORLD_MAP_STONE_TOWER);
        }
        if self.value("ganonBossKey") == ShuffleSetting::removed {
            self.disabled_item_ids.insert(iid::OOT_BOSS_KEY_GANON);
        }
        if self.value("bossKeyShuffleOot") == ShuffleSetting::removed {
            self.disable_range(iid::OOT_BOSS_KEY_FOREST, iid::OOT_BOSS_KEY_SHADOW);
        }
        if self.value("bossKeyShuffleMm") == ShuffleSetting::removed {
            self.disable_range(iid::MM_BOSS_KEY_WF, iid::MM_BOSS_KEY_ST);
        }
        if self.value("smallKeyShuffleOot") == ShuffleSetting::removed {
            self.disable_range(iid::OOT_SMALL_KEY_FOREST, iid::OOT_SMALL_KEY_GTG);
            self.disable_range(iid::OOT_KEY_RING_FOREST, iid::OOT_KEY_RING_GTG);
        }
        if self.value("smallKeyShuffleMm") == ShuffleSetting::removed {
            self.disable_range(iid::MM_SMALL_KEY_WF, iid::MM_SMALL_KEY_ST);
            self.disable_range(iid::MM_KEY_RING_WF, iid::MM_KEY_RING_ST);
        }
        if self.value("skipZelda") == ShuffleSetting::all {
            self.disabled_item_ids.insert(iid::OOT_CHICKEN);
            self.starting_item_ids.insert(iid::OOT_SONG_ZELDA, 1);
            self.starting_item_ids.insert(iid::OOT_ZELDA_LETTER, 1);
        }

        for m in ITEM_SETTINGS {
            let setting = self.value(m.key);
            match m.cat {
                ParamCategory::standard => {
                    if !m.affected.is_empty() && self.check_item_enabled(setting, m.affected[0]) == false {
                        for &id in &m.affected[1..] {
                            self.disabled_item_ids.insert(id);
                        }
                    }
                }
                ParamCategory::progressive => {
                    for &id in m.affected {
                        self.progressive_item_ids.insert(id);
                    }
                }
                ParamCategory::souls => {
                    if m.affected.len() == 2 && self.check_item_enabled(setting, m.affected[0]) == false {
                        for id in (m.affected[0] + 1)..=m.affected[1] {
                            self.disabled_item_ids.insert(id);
                        }
                    }
                }
                ParamCategory::shared => {
                    for &id in m.affected {
                        self.shared_item_ids.insert(id);
                    }
                }
            }
        }
    }

    /// CheckItemEnabled: a vanilla item is disabled (and reported not enabled).
    fn check_item_enabled(&mut self, setting: ShuffleSetting, item_id: u32) -> bool {
        if setting == ShuffleSetting::vanilla {
            self.disabled_item_ids.insert(item_id);
            return false;
        }
        true
    }

    fn disable_range(&mut self, from: u32, to: u32) {
        for i in from..=to {
            self.disabled_item_ids.insert(i);
        }
    }
}

/// The dungeon-specific stray-fairy pool item for an MM temple scene.
fn mm_stray_fairy_for_scene(scene: u16) -> Option<u32> {
    Some(match scene {
        s::MM_TEMPLE_WOODFALL => iid::MM_STRAY_FAIRY_WF,
        s::MM_TEMPLE_SNOWHEAD => iid::MM_STRAY_FAIRY_SH,
        s::MM_TEMPLE_GREAT_BAY => iid::MM_STRAY_FAIRY_GB,
        s::MM_STONE_TOWER | s::MM_STONE_TOWER_INVERTED => iid::MM_STRAY_FAIRY_ST,
        _ => return None,
    })
}

// ── AddSetting value mapping (spoiler string -> ShuffleSetting) ───────────────

/// The `key` FilterSettings entry name (identity — spoiler keys match directly).
fn filter_key(name: &str) -> &'static str {
    FILTER_SETTINGS.iter().find(|m| m.key == name).map(|m| m.key).unwrap_or("")
}
fn item_key(name: &str) -> &'static str {
    ITEM_SETTINGS.iter().find(|m| m.key == name).map(|m| m.key).unwrap_or("")
}

/// The filter-parameter value mapping of AddSetting.
fn filter_value(value: &str) -> ShuffleSetting {
    let int_pos = value.parse::<i64>().map(|n| n > 0).unwrap_or(false);
    match value {
        "all" | "true" | "full" | "anywhere" | "ganon" | "child" | "cross" => ShuffleSetting::all,
        _ if int_pos => ShuffleSetting::all,
        "starting" => ShuffleSetting::starting,
        "dungeons" | "ownDungeon" => ShuffleSetting::dungeons,
        "overworld" => ShuffleSetting::overworld,
        "removed" => ShuffleSetting::removed,
        _ => ShuffleSetting::vanilla,
    }
}

/// The item-parameter value mapping of AddSetting.
fn item_value(value: &str) -> ShuffleSetting {
    let int_pos = value.parse::<i64>().map(|n| n > 0).unwrap_or(false);
    match value {
        "progressive" | "all" | "true" | "ascending" => ShuffleSetting::all,
        _ if int_pos => ShuffleSetting::all,
        "goron" | "descending" => ShuffleSetting::overworld,
        _ => ShuffleSetting::vanilla,
    }
}

// ── Spoiler text helpers ─────────────────────────────────────────────────────

/// A parsed `  Label: ...` entry: an inline value or an indented `- item` list.
enum ListValue {
    Inline(String),
    Items(Vec<String>),
}

/// Split the spoiler into top-level sections: each starts at a column-0,
/// non-empty line and includes the following indented lines (mirror of the
/// ParseSettings section regex).
/// Resolve a spoiler `Tricks` / `Glitches` display name to its logic trick id
/// (an entry of `data::TRICK_NAMES`). Both sections list tricks by OoTMM display
/// name; `data::TRICK_NAME_TO_ID` (vendored from OoTMM's tricks.ts, sorted by
/// name) maps them back to ids — covering every trick the logic references. An
/// unknown name (e.g. a trick the logic never checks) resolves to `None` and is
/// simply ignored.
fn resolve_trick_name(name: &str) -> Option<&'static str> {
    let table = crate::data::TRICK_NAME_TO_ID;
    table
        .binary_search_by(|(n, _)| n.cmp(&name))
        .ok()
        .map(|i| table[i].1)
}

/// Resolve a spoiler `Song Events` song display name to its song index (0..=19),
/// the ordering the logic macros use (`_song_event_<game>(slot, song)`): zelda=0,
/// epona=1, saria=2, storms=3, sun=4, time=5, tp_forest=6, tp_fire=7, tp_water=8,
/// tp_spirit=9, tp_shadow=10, tp_light=11, healing=12, soaring=13, awakening=14,
/// goron=15, goron_half=16, zora=17, elegy=18, order=19.
fn resolve_song_name(name: &str) -> Option<u8> {
    Some(match name {
        "Zelda's Lullaby" => 0,
        "Epona's Song" => 1,
        "Saria's Song" => 2,
        "Song of Storms" => 3,
        "Sun's Song" => 4,
        "Song of Time" => 5,
        "Minuet of Forest" => 6,
        "Bolero of Fire" => 7,
        "Serenade of Water" => 8,
        "Requiem of Spirit" => 9,
        "Nocturne of Shadow" => 10,
        "Prelude of Light" => 11,
        "Song of Healing" => 12,
        "Song of Soaring" => 13,
        "Sonata of Awakening" => 14,
        "Goron Lullaby" => 15,
        "Goron Lullaby Intro" => 16,
        "New Wave Bossa Nova" => 17,
        "Elegy of Emptiness" => 18,
        "Oath to Order" => 19,
        _ => return None,
    })
}

/// Strip a spoiler region's `OOT `/`MM ` prefix -> `(game, bare name)`.
fn strip_game(s: &str) -> Option<(u8, &str)> {
    if let Some(r) = s.strip_prefix("OOT ") {
        Some((0, r))
    } else {
        s.strip_prefix("MM ").map(|r| (1, r))
    }
}

/// Split `"<left><sep><right>"` at the first `sep` that is immediately followed by
/// a game prefix, so a region name containing `sep` itself (e.g. "Road to Southern
/// Swamp") does not break the split. The returned right half keeps its prefix.
fn split_on_prefixed<'a>(s: &'a str, sep: &str) -> Option<(&'a str, &'a str)> {
    let mut best: Option<usize> = None;
    for prefix in ["OOT ", "MM "] {
        if let Some(i) = s.find(&format!("{sep}{prefix}")) {
            best = Some(best.map_or(i, |b| b.min(i)));
        }
    }
    let i = best?;
    Some((&s[..i], &s[i + sep.len()..]))
}

fn split_sections(text: &str) -> Vec<String> {
    let mut sections: Vec<String> = Vec::new();
    let mut cur: Option<String> = None;
    for line in text.lines() {
        let starts_col0 = !line.is_empty() && !line.starts_with(char::is_whitespace);
        if starts_col0 {
            if let Some(sec) = cur.take() {
                sections.push(sec);
            }
            cur = Some(line.to_string());
        } else if let Some(sec) = cur.as_mut() {
            sec.push('\n');
            sec.push_str(line);
        }
    }
    if let Some(sec) = cur {
        sections.push(sec);
    }
    sections
}

/// Read a `  <label>: value` / `  <label>:` + `    - item` block from a section.
fn read_list(section: &str, label: &str) -> Option<ListValue> {
    let lines: Vec<&str> = section.lines().collect();
    for (i, line) in lines.iter().enumerate() {
        let body = match line.strip_prefix("  ") {
            Some(b) if !b.starts_with(' ') => b,
            _ => continue,
        };
        if let Some(rest) = body.strip_prefix(label) {
            if let Some(v) = rest.strip_prefix(": ") {
                return Some(ListValue::Inline(v.trim().to_string()));
            }
            if rest == ":" {
                // Following `    - item` lines.
                let mut items = Vec::new();
                for l in &lines[i + 1..] {
                    match l.strip_prefix("    - ") {
                        Some(item) => items.push(item.trim().to_string()),
                        None => break,
                    }
                }
                return Some(ListValue::Items(items));
            }
        }
    }
    None
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::data::OOT_OBJECTS;

    #[test]
    fn spoiler_settings_exclude_map_objects() {
        let mq = HashSet::new();
        let mut s = Settings::default();
        let base = s.apply(&mq);

        // A spoiler that leaves gold skulltulas and OoT shops in vanilla should
        // hide every GS + shop object (default is `all` = shuffled = shown).
        let spoiler = "Settings\n  goldSkulltulaTokens: vanilla\n  shopShuffleOot: vanilla\n";
        s.parse_spoiler(spoiler, &mq);
        let after = s.apply(&mq);

        assert!(after.oot.len() > base.oot.len(), "vanilla settings should exclude more");
        let gs_excluded = OOT_OBJECTS
            .iter()
            .enumerate()
            .any(|(i, o)| o.render_type == ObjectType::gs && after.oot.contains(&i));
        assert!(gs_excluded, "GS objects excluded when goldSkulltulaTokens=vanilla");
    }

    /// The spoiler's `Tricks` / `Glitches` sections resolve to real logic trick
    /// ids via the vendored name table, and the table stays sorted (the binary
    /// search precondition).
    #[test]
    fn spoiler_tricks_and_glitches_resolve_to_ids() {
        let mq = HashSet::new();
        let mut s = Settings::default();
        let spoiler = "Tricks\n  Backflip Over Mido\n  Fewer Lens Requirements (OoT)\n\
                       Glitches\n  Equip Swap (OoT)\n  Broken Deku Stick (OoT)\n";
        s.parse_spoiler(spoiler, &mq);

        for id in ["OOT_MIDO_SKIP", "OOT_LENS", "GLITCH_OOT_EQUIP_SWAP",
                   "GLITCH_OOT_BROKEN_STICK"] {
            assert!(s.enabled_trick_ids.contains(id), "trick {id} not enabled");
        }
        // Everything resolved must be a real logic trick id.
        for id in &s.enabled_trick_ids {
            assert!(crate::data::TRICK_NAMES.contains(id), "unknown trick id {id}");
        }
        assert!(
            crate::data::TRICK_NAME_TO_ID.windows(2).all(|w| w[0].0 <= w[1].0),
            "TRICK_NAME_TO_ID must be sorted by name for binary search"
        );
    }

    /// The `Song Events` section maps each slot (in push order = enum order) to
    /// its song index, per game, resolving the display names.
    #[test]
    fn spoiler_song_events_map_slots_in_order() {
        let mq = HashSet::new();
        let mut s = Settings::default();
        let spoiler = "Song Events\n  Ocarina of Time\n\
                       \x20\x20\x20\x20SONG_EVENT_A : Prelude of Light\n\
                       \x20\x20\x20\x20SONG_EVENT_B : Song of Time\n\
                       \x20\x20Majora's Mask\n\
                       \x20\x20\x20\x20SONG_EVENT_C : Goron Lullaby\n";
        s.parse_spoiler(spoiler, &mq);
        // Prelude of Light = 11, Song of Time = 5 (OoT slots 0,1).
        assert_eq!(s.song_events[0], vec![11, 5]);
        // Goron Lullaby = 15 (MM slot 0).
        assert_eq!(s.song_events[1], vec![15]);
    }

    /// The `Entrances` split anchors on the game prefix, so a region name that
    /// itself contains ` to ` / ` from ` (e.g. "Road to Southern Swamp") is kept
    /// whole rather than split at its internal separator.
    #[test]
    fn spoiler_entrances_split_handles_names_with_to() {
        let mq = HashSet::new();
        let mut s = Settings::default();
        s.parse_spoiler(
            "Entrances\n  MM Road to Southern Swamp to MM Swamp Archery (MM_A) \
             -> MM Path to Snowhead from MM Snowhead (MM_B)\n",
            &mq,
        );
        assert_eq!(s.entrance_remap.len(), 1);
        let m = &s.entrance_remap[0];
        assert_eq!(m.game, 1);
        assert_eq!(m.from, "Road to Southern Swamp");
        assert_eq!(m.via, "Swamp Archery");
        assert_eq!(m.dest_game, 1);
        assert_eq!(m.dest, "Path to Snowhead");
    }

    /// The exclusion actually removes objects from what a loaded scene would
    /// render: a scene's GS markers all vanish once GS shuffle is vanilla.
    #[test]
    fn exclusion_hides_scene_gs_markers() {
        use crate::scene::LiveScene;
        let mq = HashSet::new();

        // Pick an OoT scene that renders at least one GS object.
        let scene_id = OOT_OBJECTS
            .iter()
            .find(|o| o.render_type == ObjectType::gs)
            .map(|o| o.render_scene)
            .expect("a scene with a GS");
        let def = Game::Oot.scenes().iter().find(|s| s.id == scene_id).unwrap();

        // Default settings: the scene has visible (non-excluded) GS markers.
        let mut s = Settings::default();
        let base = s.apply(&mq);
        let scene = LiveScene::load(Game::Oot, def, &mq);
        let visible_gs = |ex: &Excluded| {
            scene
                .objects
                .iter()
                .filter(|o| o.type_ == ObjectType::gs && !ex.contains(Game::Oot, o.index))
                .count()
        };
        assert!(visible_gs(&base) > 0, "default: GS markers are shown");

        // With GS shuffle vanilla, every GS marker on that scene is excluded.
        s.parse_spoiler("Settings\n  goldSkulltulaTokens: vanilla\n", &mq);
        let after = s.apply(&mq);
        assert_eq!(visible_gs(&after), 0, "vanilla GS: no GS marker rendered");
    }

    /// A realistic spoiler split by `===` separators (Settings block + World/MQ
    /// block + a locations block) drives both the MQ layout set and the exclusion.
    #[test]
    fn realistic_spoiler_drives_mq_and_exclusion() {
        let sep = "===========================================================================";
        let spoiler = format!(
            "Version: dev-test\nSeed: TEST\n\n\
             Settings\n  \
             goldSkulltulaTokens: vanilla\n  \
             mapCompassShuffle: vanilla\n\n\
             World\n  \
             Master Quest Dungeons: all\n\n\
             {sep}\nLocations\n{sep}\n  Kokiri Forest:\n    OOT Kokiri Forest Chest: Kokiri Sword\n"
        );

        // spoiler::parse resolves the MQ layout set (12 OoT dungeons).
        let sp = crate::spoiler::parse(&spoiler);
        assert_eq!(sp.mq_scenes.len(), 12, "Master Quest Dungeons: all -> 12 scenes");

        // Settings::parse_spoiler + apply produce the exclusion set.
        let mut s = Settings::default();
        s.parse_spoiler(&spoiler, &sp.mq_scenes);
        let excluded = s.apply(&sp.mq_scenes);
        let gs_excluded = OOT_OBJECTS
            .iter()
            .enumerate()
            .any(|(i, o)| o.render_type == ObjectType::gs && excluded.oot.contains(&i));
        assert!(gs_excluded, "vanilla GS -> excluded even through the full spoiler");
    }
}
