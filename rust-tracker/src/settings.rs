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
                        self.add_setting(k.trim(), v.trim());
                    }
                }
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

        // OoT small key -> key-ring pairs, keyed by the spoiler dungeon label.
        const OOT_RINGS: &[(&str, u32, u32)] = &[
            ("Forest Temple", iid::OOT_SMALL_KEY_FOREST, iid::OOT_KEY_RING_FOREST),
            ("Fire Temple", iid::OOT_SMALL_KEY_FIRE, iid::OOT_KEY_RING_FIRE),
            ("Water Temple", iid::OOT_SMALL_KEY_WATER, iid::OOT_KEY_RING_WATER),
            ("Shadow Temple", iid::OOT_SMALL_KEY_SHADOW, iid::OOT_KEY_RING_SHADOW),
            ("Spirit Temple", iid::OOT_SMALL_KEY_SPIRIT, iid::OOT_KEY_RING_SPIRIT),
            ("Bottom of the Well", iid::OOT_SMALL_KEY_BOTW, iid::OOT_KEY_RING_BOTW),
            ("Gerudo Training Grounds", iid::OOT_SMALL_KEY_GTG, iid::OOT_KEY_RING_GTG),
            ("Ganon's Castle", iid::OOT_SMALL_KEY_GANON, iid::OOT_KEY_RING_GANON),
            ("Hideout", iid::OOT_SMALL_KEY_GF, iid::OOT_KEY_RING_GF),
            ("Chest Game", iid::OOT_SMALL_KEY_TCG, iid::OOT_KEY_RING_TCG),
        ];
        const MM_RINGS: &[(&str, u32, u32)] = &[
            ("Woodfall Temple", iid::MM_SMALL_KEY_WF, iid::MM_KEY_RING_WF),
            ("Snowhead Temple", iid::MM_SMALL_KEY_SH, iid::MM_KEY_RING_SH),
            ("Great Bay Temple", iid::MM_SMALL_KEY_GB, iid::MM_KEY_RING_GB),
            ("Stone Tower Temple", iid::MM_SMALL_KEY_ST, iid::MM_KEY_RING_ST),
        ];

        let enable_all = |slf: &mut Self, rings: &[(&str, u32, u32)]| {
            for &(_, small, ring) in rings {
                slf.base_disabled.insert(small);
                slf.base_disabled.remove(&ring);
            }
        };

        if let Some(list) = read_list(section, "Small Key Ring (OoT)") {
            match list {
                ListValue::Inline(v) if v == "all" => {
                    enable_all(self, OOT_RINGS);
                    if self.value("smallKeyShuffleChestGame") != ShuffleSetting::vanilla {
                        self.base_disabled.insert(iid::OOT_SMALL_KEY_TCG);
                        self.base_disabled.remove(&iid::OOT_KEY_RING_TCG);
                    }
                }
                ListValue::Items(items) => self.enable_rings(&items, OOT_RINGS),
                _ => {}
            }
        }
        if let Some(list) = read_list(section, "Small Key Ring (MM)") {
            match list {
                ListValue::Inline(v) if v == "all" => enable_all(self, MM_RINGS),
                ListValue::Items(items) => self.enable_rings(&items, MM_RINGS),
                _ => {}
            }
        }
    }

    fn enable_rings(&mut self, items: &[String], rings: &[(&str, u32, u32)]) {
        for name in items {
            if let Some(&(_, small, ring)) = rings.iter().find(|(n, _, _)| n == name) {
                self.base_disabled.insert(small);
                self.base_disabled.remove(&ring);
            }
        }
    }

    /// ParseSilverPouches: disable every silver rupee and pouch, then re-enable
    /// each cluster that exists in its scene's active layout, as a pouch when the
    /// seed selected it, otherwise as individual rupees.
    fn parse_silver_pouches(&mut self, section: &str, mq: &HashSet<(Game, u16)>) {
        // (label, rupee id, pouch id, scene, layout) — layout: 0 = Vanilla, 1 = MQ, 2 = Both.
        const AREAS: &[(&str, u32, u32, u16, u8)] = &[
            ("Dodongo's Cavern", iid::OOT_RUPEE_SILVER_DC, iid::OOT_POUCH_SILVER_DC, s::OOT_DODONGO_CAVERN, 1),
            ("Bottom of the Well", iid::OOT_RUPEE_SILVER_BOTW, iid::OOT_POUCH_SILVER_BOTW, s::OOT_BOTTOM_OF_THE_WELL, 0),
            ("Spirit Temple (Child)", iid::OOT_RUPEE_SILVER_SPIRIT_CHILD, iid::OOT_POUCH_SILVER_SPIRIT_CHILD, s::OOT_TEMPLE_SPIRIT, 0),
            ("Spirit Temple (Sun)", iid::OOT_RUPEE_SILVER_SPIRIT_SUN, iid::OOT_POUCH_SILVER_SPIRIT_SUN, s::OOT_TEMPLE_SPIRIT, 0),
            ("Spirit Temple (Boulders)", iid::OOT_RUPEE_SILVER_SPIRIT_BOULDERS, iid::OOT_POUCH_SILVER_SPIRIT_BOULDERS, s::OOT_TEMPLE_SPIRIT, 0),
            ("Spirit Temple (Lobby)", iid::OOT_RUPEE_SILVER_SPIRIT_LOBBY, iid::OOT_POUCH_SILVER_SPIRIT_LOBBY, s::OOT_TEMPLE_SPIRIT, 1),
            ("Spirit Temple (Adult)", iid::OOT_RUPEE_SILVER_SPIRIT_ADULT, iid::OOT_POUCH_SILVER_SPIRIT_ADULT, s::OOT_TEMPLE_SPIRIT, 1),
            ("Shadow Temple (Scythe)", iid::OOT_RUPEE_SILVER_SHADOW_SCYTHE, iid::OOT_POUCH_SILVER_SHADOW_SCYTHE, s::OOT_TEMPLE_SHADOW, 2),
            ("Shadow Temple (Pit)", iid::OOT_RUPEE_SILVER_SHADOW_PIT, iid::OOT_POUCH_SILVER_SHADOW_PIT, s::OOT_TEMPLE_SHADOW, 2),
            ("Shadow Temple (Spikes)", iid::OOT_RUPEE_SILVER_SHADOW_SPIKES, iid::OOT_POUCH_SILVER_SHADOW_SPIKES, s::OOT_TEMPLE_SHADOW, 2),
            ("Shadow Temple (Blades)", iid::OOT_RUPEE_SILVER_SHADOW_BLADES, iid::OOT_POUCH_SILVER_SHADOW_BLADES, s::OOT_TEMPLE_SHADOW, 1),
            ("Ice Cavern (Scythe)", iid::OOT_RUPEE_SILVER_IC_SCYTHE, iid::OOT_POUCH_SILVER_IC_SCYTHE, s::OOT_ICE_CAVERN, 0),
            ("Ice Cavern (Block)", iid::OOT_RUPEE_SILVER_IC_BLOCK, iid::OOT_POUCH_SILVER_IC_BLOCK, s::OOT_ICE_CAVERN, 0),
            ("GTG (Slopes)", iid::OOT_RUPEE_SILVER_GTG_SLOPES, iid::OOT_POUCH_SILVER_GTG_SLOPES, s::OOT_GERUDO_TRAINING_GROUND, 2),
            ("GTG (Lava)", iid::OOT_RUPEE_SILVER_GTG_LAVA, iid::OOT_POUCH_SILVER_GTG_LAVA, s::OOT_GERUDO_TRAINING_GROUND, 2),
            ("GTG (Water)", iid::OOT_RUPEE_SILVER_GTG_WATER, iid::OOT_POUCH_SILVER_GTG_WATER, s::OOT_GERUDO_TRAINING_GROUND, 2),
            ("Ganon's Castle (Light)", iid::OOT_RUPEE_SILVER_GANON_LIGHT, iid::OOT_POUCH_SILVER_GANON_LIGHT, s::OOT_INSIDE_GANON_CASTLE, 0),
            ("Ganon's Castle (Forest)", iid::OOT_RUPEE_SILVER_GANON_FOREST, iid::OOT_POUCH_SILVER_GANON_FOREST, s::OOT_INSIDE_GANON_CASTLE, 0),
            ("Ganon's Castle (Fire)", iid::OOT_RUPEE_SILVER_GANON_FIRE, iid::OOT_POUCH_SILVER_GANON_FIRE, s::OOT_INSIDE_GANON_CASTLE, 2),
            ("Ganon's Castle (Water)", iid::OOT_RUPEE_SILVER_GANON_WATER, iid::OOT_POUCH_SILVER_GANON_WATER, s::OOT_INSIDE_GANON_CASTLE, 1),
            ("Ganon's Castle (Shadow)", iid::OOT_RUPEE_SILVER_GANON_SHADOW, iid::OOT_POUCH_SILVER_GANON_SHADOW, s::OOT_INSIDE_GANON_CASTLE, 1),
            ("Ganon's Castle (Spirit)", iid::OOT_RUPEE_SILVER_GANON_SPIRIT, iid::OOT_POUCH_SILVER_GANON_SPIRIT, s::OOT_INSIDE_GANON_CASTLE, 0),
        ];

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

        for &(label, rupee, pouch, scene, layout) in AREAS {
            let is_mq = mq.contains(&(Game::Oot, scene));
            let exists = layout == 2 || (layout == 1 && is_mq) || (layout == 0 && !is_mq);
            if !exists {
                continue;
            }
            if all_pouches || pouch_areas.contains(label) {
                self.base_disabled.remove(&pouch);
            } else {
                self.base_disabled.remove(&rupee);
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
        const OWLS: &[(&str, u32)] = &[
            ("Clock Town", iid::MM_OWL_CLOCK_TOWN),
            ("Milk Road", iid::MM_OWL_MILK_ROAD),
            ("Southern Swamp", iid::MM_OWL_SOUTHERN_SWAMP),
            ("Woodfall", iid::MM_OWL_WOODFALL),
            ("Mountain Village", iid::MM_OWL_MOUNTAIN_VILLAGE),
            ("Snowhead", iid::MM_OWL_SNOWHEAD),
            ("Great Bay Coast", iid::MM_OWL_GREAT_BAY),
            ("Zora Cape", iid::MM_OWL_ZORA_CAPE),
            ("Ikana Canyon", iid::MM_OWL_IKANA_CANYON),
            ("Stone Tower", iid::MM_OWL_STONE_TOWER),
        ];
        if let Some(list) = read_list(section, "Pre-Activated Owl Statues") {
            match list {
                ListValue::Inline(v) if v == "all" => {
                    for i in iid::MM_OWL_GREAT_BAY..=iid::MM_OWL_STONE_TOWER {
                        self.base_starting.insert(i, 1);
                    }
                }
                ListValue::Items(items) => {
                    for name in items {
                        if let Some(&(_, id)) = OWLS.iter().find(|(n, _)| *n == name) {
                            self.base_starting.insert(id, 1);
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
