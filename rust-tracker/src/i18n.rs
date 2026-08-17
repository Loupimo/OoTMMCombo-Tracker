use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use std::path::Path;

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum Language {
    #[serde(rename = "fr")]
    French,
    #[serde(rename = "en")]
    English,
}

impl Language {
    pub const ALL: [Language; 2] = [
        Language::French,
        Language::English,
    ];

    pub fn code(self) -> &'static str {
        match self {
            Language::French => "fr",
            Language::English => "en",
        }
    }

    pub fn label(self) -> &'static str {
        match self {
            Language::French => "Français",
            Language::English => "English",
        }
    }

    /*pub fn from_code(code: &str) -> Self {
        match code {
            "fr" => Self::French,
            _ => Self::English,
        }
    }*/
}

#[derive(Debug, Deserialize)]
struct LocaleFile {
    menu: HashMap<String, String>,
    main_tab: HashMap<String, String>,
    tracking: HashMap<String, String>,
    objects: HashMap<String, String>,
    launch: HashMap<String, String>,
    common: HashMap<String, String>,
    entrance: HashMap<String, String>,
    progression: HashMap<String, String>,
    gps: HashMap<String, String>,
    settings: HashMap<String, String>,
    shuffle: HashMap<String, String>,
    spoiler: HashMap<String, String>,
    logs: HashMap<String, String>,
    map: HashMap<String, String>,
    /// Optional display-name overlays keyed by the ENGLISH name from `data.rs`
    /// (which stays the stable id / fallback). Absent keys fall back to English.
    #[serde(default)]
    scene_names: HashMap<String, String>,
    #[serde(default)]
    region_names: HashMap<String, String>,
    #[serde(default)]
    object_names: HashMap<String, String>,
    /// Keyed by the canonical `ItemDef.name` (spoiler strings resolve to it via
    /// `progression::find_item_id`). Absent / empty keys fall back to the raw
    /// spoiler string.
    #[serde(default)]
    item_names: HashMap<String, String>,
}

fn default_true() -> bool {
    true
}

#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct AppSettings {
    pub language: Language,
    // Persisted option toggles (Qt Options menu). `#[serde(default)]` keeps older
    // settings files (language only) loadable.
    #[serde(default)]
    pub hide_collected_map: bool,
    #[serde(default)]
    pub hide_collected_list: bool,
    #[serde(default)]
    pub auto_snap: bool,
    #[serde(default)]
    pub auto_zoom: bool,
    #[serde(default)]
    pub backup_on_save: bool,
    #[serde(default)]
    pub auto_follow_item: bool,
    #[serde(default)]
    pub auto_follow_entrance: bool,
    #[serde(default)]
    pub auto_gps_start: bool,
    #[serde(default = "default_true")]
    pub auto_load_tracking: bool,
    #[serde(default = "default_true")]
    pub auto_load_spoiler: bool,
}

impl Default for AppSettings {
    /// First-launch defaults: follow the OS UI language, English if unsupported;
    /// options off except the two auto-loads (which mirror the current behaviour).
    fn default() -> Self {
        Self {
            language: system_language(),
            hide_collected_map: false,
            hide_collected_list: false,
            auto_snap: false,
            auto_zoom: false,
            backup_on_save: false,
            auto_follow_item: false,
            auto_follow_entrance: false,
            auto_gps_start: false,
            auto_load_tracking: true,
            auto_load_spoiler: true,
        }
    }
}

impl AppSettings {
    /// Load the persisted settings, falling back to the system-language defaults
    /// when the file is missing or unreadable (first launch / corrupted file).
    /*
     * @param path Fichier de configuration (tracker_settings.toml).
     * @return Les réglages chargés, ou les valeurs par défaut.
     */
    pub fn load(path: &Path) -> Self {
        std::fs::read_to_string(path)
            .ok()
            .and_then(|text| toml::from_str(&text).ok())
            .unwrap_or_default()
    }

    /// Persist the settings so the chosen language survives across sessions.
    /*
     * @param path Fichier de configuration où écrire les réglages.
     */
    pub fn save(&self, path: &Path) {
        if let Ok(text) = toml::to_string(self) {
            let _ = std::fs::write(path, text);
        }
    }
}

/// The OS UI language mapped onto a supported `Language` (English if the system
/// language isn't one we ship). Used for the first-launch default.
/*
 * @return La langue système si supportée, sinon l'anglais.
 */
fn system_language() -> Language {
    #[cfg(windows)]
    {
        extern "system" {
            fn GetUserDefaultUILanguage() -> u16;
        }
        // The low 10 bits of the LANGID hold the primary language (LANG_FRENCH = 0x0C).
        const LANG_FRENCH: u16 = 0x0C;
        if unsafe { GetUserDefaultUILanguage() } & 0x03FF == LANG_FRENCH {
            return Language::French;
        }
    }
    #[cfg(not(windows))]
    {
        if std::env::var("LANG").is_ok_and(|l| l.starts_with("fr")) {
            return Language::French;
        }
    }
    Language::English
}

pub struct I18n {
    language: Language,
    strings: LocaleFile,
}

impl I18n {
    pub fn new(language: Language) -> Self {
        let strings = Self::load(language);
        Self { language, strings }
    }

    pub fn language(&self) -> Language {
        self.language
    }

    pub fn set_language(&mut self, language: Language) {
        self.language = language;
        self.strings = Self::load(language);
    }

    fn load(language: Language) -> LocaleFile {
        let text = match language {
            Language::French => include_str!("../locales/fr.toml"),
            Language::English => include_str!("../locales/en.toml"),
        };

        toml::from_str(text)
            .unwrap_or_else(|e| panic!("Invalid {} locale: {e}", language.code()))
    }

    fn get<'a>(map: &'a HashMap<String, String>, key: &str) -> &'a str {
        map.get(key)
            .map(String::as_str)
            .unwrap_or_else(|| panic!("Missing translation key: {key}"))
    }

    // ---------------------------------------------------------------------
    // Menu
    // ---------------------------------------------------------------------

    pub fn options(&self) -> &str {
        Self::get(&self.strings.menu, "options")
    }

    pub fn recenter_view(&self) -> &str {
        Self::get(&self.strings.menu, "recenter_view")
    }

    pub fn rom_settings(&self) -> &str {
        Self::get(&self.strings.menu, "rom_settings")
    }

    pub fn menu_tracking(&self) -> &str {
        Self::get(&self.strings.menu, "tracking")
    }

    pub fn reveal_items(&self) -> &str {
        Self::get(&self.strings.menu, "reveal_items")
    }

    pub fn auto_saving(&self) -> &str {
        Self::get(&self.strings.menu, "auto_saving")
    }

    pub fn about(&self) -> &str {
        Self::get(&self.strings.menu, "about")
    }

    pub fn opt_hide_collected(&self) -> &str {
        Self::get(&self.strings.menu, "hide_collected")
    }

    pub fn opt_from_map(&self) -> &str {
        Self::get(&self.strings.menu, "from_map")
    }

    pub fn opt_from_list(&self) -> &str {
        Self::get(&self.strings.menu, "from_list")
    }

    pub fn opt_auto_snap(&self) -> &str {
        Self::get(&self.strings.menu, "auto_snap")
    }

    pub fn opt_auto_zoom(&self) -> &str {
        Self::get(&self.strings.menu, "auto_zoom")
    }

    pub fn opt_backup(&self) -> &str {
        Self::get(&self.strings.menu, "backup_on_save")
    }

    pub fn opt_follow_item(&self) -> &str {
        Self::get(&self.strings.menu, "auto_follow_item")
    }

    pub fn opt_follow_entrance(&self) -> &str {
        Self::get(&self.strings.menu, "auto_follow_entrance")
    }

    pub fn opt_gps_start(&self) -> &str {
        Self::get(&self.strings.menu, "auto_gps_start")
    }

    pub fn opt_auto_load(&self) -> &str {
        Self::get(&self.strings.menu, "auto_load")
    }

    pub fn opt_auto_load_tracking(&self) -> &str {
        Self::get(&self.strings.menu, "auto_load_tracking")
    }

    pub fn opt_auto_load_spoiler(&self) -> &str {
        Self::get(&self.strings.menu, "auto_load_spoiler")
    }

    // ---------------------------------------------------------------------
    // Main Tabs
    // ---------------------------------------------------------------------

    pub fn launch(&self) -> &str {
        Self::get(&self.strings.main_tab, "launch")
    }

    pub fn entrance(&self) -> &str {
        Self::get(&self.strings.main_tab, "entrance")
    }

    pub fn progress(&self) -> &str {
        Self::get(&self.strings.main_tab, "progress")
    }

    // ---------------------------------------------------------------------
    // Tracking
    // ---------------------------------------------------------------------

    pub fn tracking_active(&self) -> &str {
        Self::get(&self.strings.tracking, "active")
    }

    pub fn tracking_inactive(&self) -> &str {
        Self::get(&self.strings.tracking, "inactive")
    }

    pub fn tracker_state_tip(&self) -> &str {
        Self::get(&self.strings.tracking, "tracker_state_tip")
    }

    pub fn start_tracking(&self) -> &str {
        Self::get(&self.strings.tracking, "start")
    }

    pub fn stop_tracking(&self) -> &str {
        Self::get(&self.strings.tracking, "stop")
    }

    pub fn auto_save_tip(&self) -> &str {
        Self::get(&self.strings.tracking, "auto_save_tip")
    }

    pub fn auto_save(&self, status: bool) -> &str {
        if status
        {
            Self::get(&self.strings.tracking, "auto_save_on")
        }
        else
        {
            Self::get(&self.strings.tracking, "auto_save_off")
        }
        
    }

    pub fn items_revealed_tip(&self) -> &str {
        Self::get(&self.strings.tracking, "items_revealed_tip")
    }

    pub fn items_revealed(&self, status: bool) -> &str {
        if status
        {
            Self::get(&self.strings.tracking, "items_revealed")
        }
        else
        {
            Self::get(&self.strings.tracking, "items_hidden")
        }
    }

    pub fn total(&self) -> &str {
        Self::get(&self.strings.tracking, "total")
    }

    // ---------------------------------------------------------------------
    // Objects
    // ---------------------------------------------------------------------

    pub fn objects(&self) -> &str {
        Self::get(&self.strings.objects, "title")
    }

    pub fn expand_all(&self) -> &str {
        Self::get(&self.strings.objects, "expand_all")
    }

    pub fn collapse_all(&self) -> &str {
        Self::get(&self.strings.objects, "collapse_all")
    }

    pub fn item(&self) -> &str {
        Self::get(&self.strings.objects, "item")
    }

    pub fn filter_tooltip(&self) -> &str {
        Self::get(&self.strings.objects, "filter_tooltip")
    }

    pub fn filter_needs_game(&self) -> &str {
        Self::get(&self.strings.objects, "filter_needs_game")
    }

    // ---------------------------------------------------------------------
    // Launch
    // ---------------------------------------------------------------------

    pub fn launch_options(&self) -> &str {
        Self::get(&self.strings.launch, "options")
    }

    pub fn save_tracking(&self) -> &str {
        Self::get(&self.strings.launch, "save_tracking")
    }

    pub fn load_tracking(&self) -> &str {
        Self::get(&self.strings.launch, "load_tracking")
    }

    pub fn load_spoiler(&self) -> &str {
        Self::get(&self.strings.launch, "load_spoiler")
    }

    pub fn reset_tracking(&self) -> &str {
        Self::get(&self.strings.launch, "reset_tracking")
    }

    pub fn use_multiplayer(&self) -> &str {
        Self::get(&self.strings.launch, "use_multiplayer")
    }

    pub fn journal(&self) -> &str {
        Self::get(&self.strings.launch, "journal")
    }

    pub fn drop_spoiler_hint(&self) -> &str {
        Self::get(&self.strings.launch, "drop_spoiler_hint")
    }

    pub fn simulate_event(&self) -> &str {
        Self::get(&self.strings.launch, "simulate_event")
    }

    pub fn no_event(&self) -> &str {
        Self::get(&self.strings.launch, "no_event")
    }

    pub fn address_placeholder(&self) -> &str {
        Self::get(&self.strings.launch, "address_placeholder")
    }

    pub fn port_placeholder(&self) -> &str {
        Self::get(&self.strings.launch, "port_placeholder")
    }

    // ---------------------------------------------------------------------
    // Common
    // ---------------------------------------------------------------------

    pub fn all(&self) -> &str {
        Self::get(&self.strings.common, "all")
    }

    pub fn none(&self) -> &str {
        Self::get(&self.strings.common, "none")
    }

    pub fn choose(&self) -> &str {
        Self::get(&self.strings.common, "choose")
    }

    pub fn search(&self) -> &str {
        Self::get(&self.strings.common, "search")
    }

    pub fn apply(&self) -> &str {
        Self::get(&self.strings.common, "apply")
    }

    pub fn lang(&self) -> &str {
        Self::get(&self.strings.common, "language")
    }

    pub fn trck_file(&self) -> &str {
        Self::get(&self.strings.common, "trck_file")
    }

    pub fn txt_file(&self) -> &str {
        Self::get(&self.strings.common, "txt_file")
    }

    pub fn choose_name(&self) -> &str {
        Self::get(&self.strings.common, "choose_name")
    }

    pub fn choose_trck(&self) -> &str {
        Self::get(&self.strings.common, "choose_trck")
    }

    pub fn choose_spoiler(&self) -> &str {
        Self::get(&self.strings.common, "choose_spoiler")
    }

    pub fn scenes(&self) -> &str {
        Self::get(&self.strings.common, "scenes")
    }

    pub fn scenes_title(&self) -> &str {
        Self::get(&self.strings.common, "scenes_title")
    }

    pub fn player(&self) -> &str {
        Self::get(&self.strings.common, "player")
    }

    pub fn filters(&self) -> &str {
        Self::get(&self.strings.common, "filters")
    }


    // ---------------------------------------------------------------------
    // Entrance
    // ---------------------------------------------------------------------

    pub fn all_entrances(&self) -> &str {
        Self::get(&self.strings.entrance, "all")
    }

    pub fn gps(&self) -> &str {
        Self::get(&self.strings.entrance, "gps")
    }

    pub fn entry(&self) -> &str {
        Self::get(&self.strings.entrance, "entry")
    }

    pub fn entrance_detect(&self, from: &str, to: &str) -> String {
        let template = Self::get(&self.strings.entrance, "entrance_detect");

        template
            .replace("{from}", from)
            .replace("{to}", to)
    }

    pub fn entrance_all_game(&self, game: &str) -> String {
        Self::get(&self.strings.entrance, "all_game").replace("{game}", game)
    }

    pub fn entrance_count(&self, n: usize) -> String {
        Self::get(&self.strings.entrance, "count").replace("{n}", &n.to_string())
    }

    pub fn entrance_col_scene(&self) -> &str {
        Self::get(&self.strings.entrance, "col_scene")
    }

    pub fn entrance_col_spawn(&self) -> &str {
        Self::get(&self.strings.entrance, "col_spawn")
    }

    pub fn entrance_col_leads(&self) -> &str {
        Self::get(&self.strings.entrance, "col_leads")
    }

    // ---------------------------------------------------------------------
    // Progression
    // ---------------------------------------------------------------------

    pub fn prog_world(&self) -> &str {
        Self::get(&self.strings.progression, "world")
    }

    pub fn prog_reveal(&self) -> &str {
        Self::get(&self.strings.progression, "reveal_uncollected")
    }

    pub fn prog_select_item(&self) -> &str {
        Self::get(&self.strings.progression, "select_item")
    }

    pub fn prog_found(&self) -> &str {
        Self::get(&self.strings.progression, "found")
    }

    pub fn prog_not_found(&self) -> &str {
        Self::get(&self.strings.progression, "not_found")
    }

    pub fn prog_starting_item(&self) -> &str {
        Self::get(&self.strings.progression, "starting_item")
    }

    pub fn prog_locations(&self) -> &str {
        Self::get(&self.strings.progression, "locations")
    }

    pub fn prog_no_location(&self) -> &str {
        Self::get(&self.strings.progression, "no_location")
    }

    pub fn prog_not_found_yet(&self) -> &str {
        Self::get(&self.strings.progression, "not_found_yet")
    }

    // ---------------------------------------------------------------------
    // GPS
    // ---------------------------------------------------------------------

    pub fn departure(&self) -> &str {
        Self::get(&self.strings.gps, "departure")
    }

    pub fn arrival(&self) -> &str {
        Self::get(&self.strings.gps, "arrival")
    }

    pub fn choose_route_scenes(&self) -> &str {
        Self::get(&self.strings.gps, "choose_route_scenes")
    }

    pub fn gps_route_title(&self) -> &str {
        Self::get(&self.strings.gps, "route_title")
    }

    pub fn gps_from_entrance(&self, ent: &str, scene: &str) -> String {
        Self::get(&self.strings.gps, "from_entrance")
            .replace("{ent}", ent)
            .replace("{scene}", scene)
    }

    pub fn gps_already_there(&self) -> &str {
        Self::get(&self.strings.gps, "already_there")
    }

    pub fn gps_steps(&self, count: usize) -> String {
        Self::get(&self.strings.gps, "steps").replace("{count}", &count.to_string())
    }

    pub fn gps_step_line(&self, n: usize, entrance: &str, game: &str, scene: &str) -> String {
        Self::get(&self.strings.gps, "step_line")
            .replace("{n}", &n.to_string())
            .replace("{entrance}", entrance)
            .replace("{game}", game)
            .replace("{scene}", scene)
    }

    pub fn gps_arrival_line(&self, ent: &str) -> String {
        Self::get(&self.strings.gps, "arrival_line").replace("{ent}", ent)
    }

    pub fn gps_no_route(&self) -> &str {
        Self::get(&self.strings.gps, "no_route")
    }

    pub fn gps_whole_scene(&self) -> &str {
        Self::get(&self.strings.gps, "whole_scene")
    }

    // ---------------------------------------------------------------------
    // Settings
    // ---------------------------------------------------------------------

    pub fn settings_rom_settings(&self) -> &str {
        Self::get(&self.strings.settings, "rom_settings")
    }

    pub fn settings_game(&self) -> &str {
        Self::get(&self.strings.settings, "game")
    }

    pub fn settings_build(&self) -> &str {
        Self::get(&self.strings.settings, "build")
    }

    pub fn settings_mode(&self) -> &str {
        Self::get(&self.strings.settings, "mode")
    }

    pub fn settings_goal(&self) -> &str {
        Self::get(&self.strings.settings, "goal")
    }

    pub fn settings_hidden_objs(&self) -> &str {
        Self::get(&self.strings.settings, "hidden_objs")
    }

    pub fn settings_general(&self) -> &str {
        Self::get(&self.strings.settings, "general")
    }

    pub fn settings_layouts(&self) -> &str {
        Self::get(&self.strings.settings, "layouts")
    }

    pub fn settings_layout_deku(&self) -> &str {
        Self::get(&self.strings.settings, "layout_deku")
    }

    pub fn settings_map_filters(&self) -> &str {
        Self::get(&self.strings.settings, "map_filters")
    }

    pub fn settings_item_settings(&self) -> &str {
        Self::get(&self.strings.settings, "item_settings")
    }
    
    // ---------------------------------------------------------------------
    // Shuffle
    // ---------------------------------------------------------------------

    pub fn shuffle_vanilla(&self) -> &str {
        Self::get(&self.strings.shuffle, "vanilla")
    }

    pub fn shuffle_removed(&self) -> &str {
        Self::get(&self.strings.shuffle, "removed")
    }

    pub fn shuffle_starting(&self) -> &str {
        Self::get(&self.strings.shuffle, "starting")
    }

    pub fn shuffle_all(&self) -> &str {
        Self::get(&self.strings.shuffle, "all")
    }

    pub fn shuffle_dungeons(&self) -> &str {
        Self::get(&self.strings.shuffle, "dungeons")
    }

    pub fn shuffle_overworld(&self) -> &str {
        Self::get(&self.strings.shuffle, "overworld")
    }

    // ---------------------------------------------------------------------
    // Spoiler
    // ---------------------------------------------------------------------

    pub fn spoiler_multiworld(&self, count: usize, worlds: usize) -> String {
        let template = Self::get(&self.strings.spoiler, "multiworld");
    
        template
            .replace("{count}", &count.to_string())
            .replace("{worlds}", &worlds.to_string())
    }

    pub fn spoiler_singleworld(&self, count: usize, scenes: usize) -> String {
        let template = Self::get(&self.strings.spoiler, "singleworld");
    
        template
            .replace("{count}", &count.to_string())
            .replace("{scenes}", &scenes.to_string())
    }

    // ---------------------------------------------------------------------
    // Logs
    // ---------------------------------------------------------------------

    pub fn reading_mem(&self) -> &str {
        Self::get(&self.strings.logs, "reading_mem")
    }

    pub fn log_tracker_stop(&self) -> &str {
        Self::get(&self.strings.logs, "tracking_stop")
    }

    pub fn file_saved(&self) -> &str {
        Self::get(&self.strings.logs, "file_saved")
    }

    pub fn file_loaded(&self) -> &str {
        Self::get(&self.strings.logs, "file_loaded")
    }

    pub fn spoiler_loaded(&self) -> &str {
        Self::get(&self.strings.logs, "spoiler_loaded")
    }

    pub fn log_reset_tracking(&self) -> &str {
        Self::get(&self.strings.logs, "reset_tracking")
    }

    pub fn no_img(&self) -> &str {
        Self::get(&self.strings.logs, "no_img")
    }

    // ---------------------------------------------------------------------
    // Map
    // ---------------------------------------------------------------------

    pub fn map_child(&self) -> &str {
        Self::get(&self.strings.map, "child")
    }

    pub fn map_adult(&self) -> &str {
        Self::get(&self.strings.map, "adult")
    }

    pub fn map_winter(&self) -> &str {
        Self::get(&self.strings.map, "winter")
    }

    pub fn map_spring(&self) -> &str {
        Self::get(&self.strings.map, "spring")
    }

    pub fn map_room(&self) -> &str {
        Self::get(&self.strings.map, "room")
    }

    pub fn map_select_scene(&self) -> &str {
        Self::get(&self.strings.map, "select_scene")
    }

    pub fn map_help_hint(&self) -> &str {
        Self::get(&self.strings.map, "help_hint")
    }

    // ---------------------------------------------------------------------
    // Name overlays (scenes / regions / objects). Keyed by the English name;
    // absent keys fall back to the English name, so translation is optional
    // and incremental. Fill the `[scene_names]` / `[region_names]` /
    // `[object_names]` sections of the locale files.
    // ---------------------------------------------------------------------

    pub fn tr_scene<'a>(&'a self, name: &'a str) -> &'a str {
        self.strings.scene_names.get(name).map(String::as_str).filter(|s| !s.is_empty()).unwrap_or(name)
    }

    pub fn tr_region<'a>(&'a self, name: &'a str) -> &'a str {
        self.strings.region_names.get(name).map(String::as_str).filter(|s| !s.is_empty()).unwrap_or(name)
    }

    pub fn tr_object<'a>(&'a self, name: &'a str) -> &'a str {
        self.strings.object_names.get(name).map(String::as_str).filter(|s| !s.is_empty()).unwrap_or(name)
    }

    /// The `[item_names]` translation for a canonical item id, if present and
    /// non-empty (the id resolves through the dense, id-ordered `ITEMS` table).
    fn item_fr_by_id(&self, id: u32) -> Option<&str> {
        let def = crate::data::ITEMS
            .get((id as usize).wrapping_sub(1))
            .filter(|d| d.id == id)
            .or_else(|| crate::data::ITEMS.iter().find(|d| d.id == id))?;
        self.strings.item_names.get(def.name).map(String::as_str).filter(|s| !s.is_empty())
    }

    /// Translate a spoiler item string. Resolves it to its canonical item id
    /// (same path as the progression tracker), then looks that item's
    /// `ItemDef.name` up in the `[item_names]` overlay. Falls back to the raw
    /// spoiler string when unknown or untranslated.
    pub fn tr_item<'a>(&'a self, raw: &'a str) -> &'a str {
        crate::progression::find_item_id(raw)
            .and_then(|id| self.item_fr_by_id(id))
            .unwrap_or(raw)
    }

    /// Display name for a progression entry: reuse the `[item_names]` overlay
    /// via the entry's item ids (so the progression page doesn't need its own
    /// translation table). Falls back to the English entry name for abstract
    /// entries whose items aren't translated.
    pub fn tr_prog_entry<'a>(&'a self, name: &'a str, lookup_keys: &[u32]) -> &'a str {
        lookup_keys
            .iter()
            .find_map(|&id| self.item_fr_by_id(id))
            .unwrap_or(name)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    /// Call every accessor so a key missing from a locale (which would panic via
    /// `Self::get`) is caught at test time instead of at runtime in the UI.
    fn touch_all(i: &I18n) {
        // Zero-argument string accessors.
        let s: &[&str] = &[
            i.options(), i.recenter_view(), i.rom_settings(),
            i.menu_tracking(), i.reveal_items(), i.auto_saving(), i.about(),
            i.opt_hide_collected(), i.opt_from_map(), i.opt_from_list(),
            i.opt_auto_snap(), i.opt_auto_zoom(), i.opt_backup(),
            i.opt_follow_item(), i.opt_follow_entrance(), i.opt_gps_start(),
            i.opt_auto_load(), i.opt_auto_load_tracking(), i.opt_auto_load_spoiler(),
            i.launch(), i.entrance(), i.progress(),
            i.tracking_active(), i.tracking_inactive(), i.tracker_state_tip(),
            i.start_tracking(), i.stop_tracking(), i.auto_save_tip(),
            i.items_revealed_tip(), i.total(),
            i.objects(), i.expand_all(), i.collapse_all(), i.item(),
            i.filter_tooltip(), i.filter_needs_game(),
            i.launch_options(), i.save_tracking(), i.load_tracking(), i.load_spoiler(),
            i.reset_tracking(), i.use_multiplayer(), i.journal(),
            i.drop_spoiler_hint(), i.simulate_event(),
            i.address_placeholder(), i.port_placeholder(),
            i.all(), i.none(), i.choose(), i.search(), i.apply(), i.lang(),
            i.trck_file(), i.txt_file(), i.choose_name(), i.choose_trck(), i.choose_spoiler(),
            i.scenes(), i.scenes_title(), i.player(), i.filters(),
            i.all_entrances(), i.gps(), i.entry(),
            i.entrance_col_scene(), i.entrance_col_spawn(), i.entrance_col_leads(),
            i.prog_world(), i.prog_reveal(), i.prog_select_item(),
            i.prog_found(), i.prog_not_found(), i.prog_starting_item(),
            i.prog_locations(), i.prog_no_location(), i.prog_not_found_yet(),
            i.departure(), i.arrival(), i.choose_route_scenes(),
            i.gps_route_title(), i.gps_already_there(), i.gps_no_route(), i.gps_whole_scene(),
            i.settings_rom_settings(), i.settings_game(), i.settings_build(), i.settings_mode(),
            i.settings_goal(), i.settings_hidden_objs(), i.settings_general(), i.settings_layouts(),
            i.settings_layout_deku(), i.settings_map_filters(), i.settings_item_settings(),
            i.shuffle_vanilla(), i.shuffle_removed(), i.shuffle_starting(),
            i.shuffle_all(), i.shuffle_dungeons(), i.shuffle_overworld(),
            i.reading_mem(), i.log_tracker_stop(), i.file_saved(), i.file_loaded(),
            i.spoiler_loaded(), i.log_reset_tracking(),
        ];
        assert!(s.iter().all(|t| !t.is_empty()));

        let map: &[&str] = &[
            i.no_img(),
            i.map_child(), i.map_adult(), i.map_winter(), i.map_spring(),
            i.map_room(), i.map_select_scene(), i.map_help_hint(),
        ];
        assert!(map.iter().all(|t| !t.is_empty()));

        // Boolean + templated accessors (also hit their underlying keys).
        i.auto_save(true);
        i.auto_save(false);
        i.items_revealed(true);
        i.items_revealed(false);
        i.entrance_detect("a", "b");
        i.entrance_all_game("Ocarina of Time");
        i.entrance_count(3);
        i.spoiler_multiworld(1, 2);
        i.spoiler_singleworld(1, 2);
        i.gps_from_entrance("e", "s");
        i.gps_steps(2);
        i.gps_step_line(1, "e", "OoT", "s");
        i.gps_arrival_line("e");
    }

    /// Both shipped locales must define every key the UI asks for.
    #[test]
    fn every_key_present_in_all_locales() {
        for lang in Language::ALL {
            touch_all(&I18n::new(lang));
        }
    }
}