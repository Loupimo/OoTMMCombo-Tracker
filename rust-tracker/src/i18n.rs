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
    /// Optional overlay for the ROM Settings structural labels (category names,
    /// group titles, parameter display names), keyed by the ENGLISH string used
    /// in `settings_window.rs` / `data.rs`. Absent / empty keys fall back to
    /// English, so translating these is optional and incremental.
    #[serde(default)]
    settings_names: HashMap<String, String>,
    /// Keyed by the canonical `ItemDef.name` (spoiler strings resolve to it via
    /// `progression::find_item_id`). Absent / empty keys fall back to the raw
    /// spoiler string.
    #[serde(default)]
    item_names: HashMap<String, String>,
    /// Keyed by an entrance's English `from_name` / `to_name` atom (the box
    /// titles and the in/out link rows are composed from these). Absent / empty
    /// keys fall back to the English name.
    #[serde(default)]
    entrance_names: HashMap<String, String>,
}

fn default_true() -> bool {
    true
}

/// Default multiplayer server host / port (Qt `LogTab` line edits), used for
/// first launch and for settings files written before these fields existed.
fn default_mp_host() -> String {
    "multi.ootmm.com".to_string()
}
fn default_mp_port() -> String {
    "13248".to_string()
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
    // Reachability logic (accessibility): only show checks the player can reach
    // given the items collected so far. Off by default (opt-in). When on,
    // `logic_hide_unreachable` chooses how unreachable checks are shown: dimmed
    // (false) or fully hidden (true).
    #[serde(default)]
    pub logic_filter_enabled: bool,
    #[serde(default)]
    pub logic_hide_unreachable: bool,
    // Multiplayer launch options (Qt `AppConfig` UseMultiplayer / Host / Port),
    // persisted so the checkbox and server address survive across sessions.
    #[serde(default)]
    pub use_multiplayer: bool,
    #[serde(default = "default_mp_host")]
    pub mp_host: String,
    #[serde(default = "default_mp_port")]
    pub mp_port: String,
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
            logic_filter_enabled: false,
            logic_hide_unreachable: false,
            use_multiplayer: false,
            mp_host: default_mp_host(),
            mp_port: default_mp_port(),
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

/// A plain, owned snapshot of every injection / connection log + status-bar string
/// for the current language. Built on the UI thread and shared with the background
/// poller thread (which has no `I18n`); swapped on a language change so the journal
/// follows the UI language. Templated fields keep their `{…}` placeholders for the
/// caller to fill.
#[derive(Clone)]
pub struct LogStrings {
    // Journal (poller.rs)
    pub requesting_shutdown: String,
    pub dll_unloaded: String,
    pub dll_removed: String,
    pub dll_still_loaded: String,
    pub pj64_closed_log: String,
    pub shared_mem_found: String,
    pub dll_loaded: String,
    pub settings_closed: String,
    pub settings_not_identified: String,
    pub reading_mem: String,
    pub dll_failed_init: String,
    pub no_pj64_retry: String,
    pub process_found: String, // {pid}
    // Journal (inject.rs)
    pub pj64_path: String,        // {path}
    pub plugin_dir: String,       // {path}
    pub tracker_dll: String,      // {path}
    pub dll_copied: String,       // {path}
    pub windowed_note: String,
    pub main_window_found: String, // {hwnd}
    pub ctrl_t_sent: String,
    // Injection error strings (inject.rs `Err(String)`)
    pub err_no_exe_path: String,
    pub err_no_plugin_dir: String,
    pub err_dll_missing: String, // {name}
    pub err_dll_copy: String,    // {err}
    pub err_no_main_window: String,
    pub err_sendinput: String,
    // Status bar (poller.rs)
    pub st_inactive: String,
    pub st_pj64_closed: String,
    pub st_connected: String,
    pub st_waiting_game: String,
    pub st_loading_plugin: String,
    pub st_injection_failed: String,
    pub st_waiting_pj64: String,
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

    pub fn opt_logic_menu(&self) -> &str {
        Self::get(&self.strings.menu, "logic_menu")
    }

    pub fn opt_logic_filter(&self) -> &str {
        Self::get(&self.strings.menu, "logic_filter")
    }

    pub fn opt_logic_mode(&self) -> &str {
        Self::get(&self.strings.menu, "logic_mode")
    }

    pub fn opt_logic_dim(&self) -> &str {
        Self::get(&self.strings.menu, "logic_dim")
    }

    pub fn opt_logic_hide(&self) -> &str {
        Self::get(&self.strings.menu, "logic_hide")
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

    pub fn object_type_name(&self, obj_type: &'static str) -> &str {
        Self::get(&self.strings.objects, obj_type)
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

    pub fn copy_log(&self) -> &str {
        Self::get(&self.strings.launch, "copy_log")
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

    pub fn load_patch(&self) -> &str {
        Self::get(&self.strings.launch, "load_patch")
    }

    pub fn patch_none(&self) -> &str {
        Self::get(&self.strings.launch, "patch_none")
    }

    pub fn patch_label(&self) -> &str {
        Self::get(&self.strings.launch, "patch_label")
    }

    pub fn unload_patch(&self) -> &str {
        Self::get(&self.strings.launch, "unload_patch")
    }

    pub fn patch_unloaded(&self) -> &str {
        Self::get(&self.strings.launch, "patch_unloaded")
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

    pub fn xml_file(&self) -> &str {
        Self::get(&self.strings.common, "xml_file")
    }

    pub fn tracking_files(&self) -> &str {
        Self::get(&self.strings.common, "tracking_files")
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

    pub fn choose_patch(&self) -> &str {
        Self::get(&self.strings.common, "choose_patch")
    }

    pub fn patch_file(&self) -> &str {
        Self::get(&self.strings.common, "patch_file")
    }

    pub fn all_files(&self) -> &str {
        Self::get(&self.strings.common, "all_files")
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

    pub fn prog_title<'a>(&'a self, name: &'a str) -> &'a str {
        self.strings.progression.get(name).map(String::as_str).filter(|s| !s.is_empty()).unwrap_or(name)
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

    pub fn gps_already_there(&self) -> &str {
        Self::get(&self.strings.gps, "already_there")
    }

    pub fn gps_no_route(&self) -> &str {
        Self::get(&self.strings.gps, "no_route")
    }

    pub fn gps_whole_scene(&self) -> &str {
        Self::get(&self.strings.gps, "whole_scene")
    }

    pub fn gps_fastest(&self) -> &str {
        Self::get(&self.strings.gps, "fastest")
    }

    pub fn gps_alternative(&self, n: usize) -> String {
        Self::get(&self.strings.gps, "alternative").replace("{n}", &n.to_string())
    }

    pub fn gps_transitions(&self, count: usize) -> String {
        Self::get(&self.strings.gps, "transitions").replace("{count}", &count.to_string())
    }

    /// The label for the exit taken between two stations: a real entrance name
    /// (translated elsewhere), or one of these fixed pseudo-exits.
    pub fn gps_via_walk(&self) -> &str {
        Self::get(&self.strings.gps, "via_walk")
    }

    pub fn gps_via_warp_song(&self) -> &str {
        Self::get(&self.strings.gps, "via_warp_song")
    }

    pub fn gps_via_warp_owl(&self) -> &str {
        Self::get(&self.strings.gps, "via_warp_owl")
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

    pub fn load_qt_imported(&self, version: u32) -> String {
        // The on-disk version 1/2 map to the Qt "V2_0" / "V2_1" format names.
        let name = match version {
            1 => "V2_0",
            2 => "V2_1",
            3 => "V2_1+",
            _ => "?",
        };
        Self::get(&self.strings.logs, "qt_imported").replace("{ver}", name)
    }

    pub fn load_qt_unsupported(&self) -> &str {
        Self::get(&self.strings.logs, "qt_unsupported")
    }

    pub fn load_qt_partial(&self) -> &str {
        Self::get(&self.strings.logs, "qt_partial")
    }

    pub fn no_img(&self) -> &str {
        Self::get(&self.strings.logs, "no_img")
    }

    // --- Multiplayer / patch log lines (state.rs) ---

    pub fn log_mp_enabled(&self, server: &str) -> String {
        Self::get(&self.strings.logs, "mp_enabled").replace("{server}", server)
    }

    pub fn log_r4_enabled(&self, info: &str, server: &str) -> String {
        Self::get(&self.strings.logs, "r4_enabled")
            .replace("{info}", info)
            .replace("{server}", server)
    }

    pub fn log_patch_loaded(&self, path: &str, info: &str) -> String {
        Self::get(&self.strings.logs, "patch_loaded")
            .replace("{path}", path)
            .replace("{info}", info)
    }

    pub fn log_patch_invalid(&self, err: &str) -> String {
        Self::get(&self.strings.logs, "patch_invalid").replace("{err}", err)
    }

    pub fn log_patch_missing(&self, path: &str) -> String {
        Self::get(&self.strings.logs, "patch_missing").replace("{path}", path)
    }

    pub fn log_patch_load_failed(&self, err: &str) -> String {
        Self::get(&self.strings.logs, "patch_load_failed").replace("{err}", err)
    }

    // --- Live-event log lines (state.rs) ---

    pub fn log_world_object(&self, game: &str, loc: &str, item: &str) -> String {
        Self::get(&self.strings.logs, "world_object")
            .replace("{game}", game)
            .replace("{loc}", loc)
            .replace("{item}", item)
    }

    pub fn log_world_object_net(
        &self,
        game: &str,
        loc: &str,
        item: &str,
        from: i32,
        to: i32,
    ) -> String {
        Self::get(&self.strings.logs, "world_object_net")
            .replace("{game}", game)
            .replace("{loc}", loc)
            .replace("{item}", item)
            .replace("{from}", &from.to_string())
            .replace("{to}", &to.to_string())
    }

    pub fn log_new_scene(&self, from: &str, fromid: u32, to: &str, toid: u32) -> String {
        Self::get(&self.strings.logs, "ent_new_scene")
            .replace("{from}", from)
            .replace("{fromid}", &format!("{fromid:X}"))
            .replace("{to}", to)
            .replace("{toid}", &format!("{toid:X}"))
    }

    pub fn log_one_way_in(&self, ent: &str, id: u32) -> String {
        Self::get(&self.strings.logs, "ent_one_way_in")
            .replace("{ent}", ent)
            .replace("{id}", &format!("{id:X}"))
    }

    pub fn log_one_way_out(&self, ent: &str, id: u32) -> String {
        Self::get(&self.strings.logs, "ent_one_way_out")
            .replace("{ent}", ent)
            .replace("{id}", &format!("{id:X}"))
    }

    // --- Entrance debug-table headers / row labels (state.rs) ---
    pub fn tbl_from(&self) -> &str { Self::get(&self.strings.logs, "ent_tbl_from") }
    pub fn tbl_to(&self) -> &str { Self::get(&self.strings.logs, "ent_tbl_to") }
    pub fn tbl_game(&self) -> &str { Self::get(&self.strings.logs, "ent_tbl_game") }
    pub fn tbl_scene(&self) -> &str { Self::get(&self.strings.logs, "ent_tbl_scene") }
    pub fn tbl_entrance(&self) -> &str { Self::get(&self.strings.logs, "ent_tbl_entrance") }
    pub fn tbl_room(&self) -> &str { Self::get(&self.strings.logs, "ent_tbl_room") }
    pub fn tbl_grotto(&self) -> &str { Self::get(&self.strings.logs, "ent_tbl_grotto") }
    pub fn tbl_age(&self) -> &str { Self::get(&self.strings.logs, "ent_tbl_age") }
    pub fn tbl_farore(&self) -> &str { Self::get(&self.strings.logs, "ent_tbl_farore") }
    pub fn tbl_owl(&self) -> &str { Self::get(&self.strings.logs, "ent_tbl_owl") }
    pub fn tbl_song(&self) -> &str { Self::get(&self.strings.logs, "ent_tbl_song") }

    /// Snapshot every poller / injection log + status string for the current
    /// language, for the background poller thread (see [`LogStrings`]).
    pub fn log_strings(&self) -> LogStrings {
        let g = |k: &str| Self::get(&self.strings.logs, k).to_string();
        LogStrings {
            requesting_shutdown: g("requesting_shutdown"),
            dll_unloaded: g("dll_unloaded"),
            dll_removed: g("dll_removed"),
            dll_still_loaded: g("dll_still_loaded"),
            pj64_closed_log: g("pj64_closed_log"),
            shared_mem_found: g("shared_mem_found"),
            dll_loaded: g("dll_loaded"),
            settings_closed: g("settings_closed"),
            settings_not_identified: g("settings_not_identified"),
            reading_mem: g("reading_mem"),
            dll_failed_init: g("dll_failed_init"),
            no_pj64_retry: g("no_pj64_retry"),
            process_found: g("process_found"),
            pj64_path: g("pj64_path"),
            plugin_dir: g("plugin_dir"),
            tracker_dll: g("tracker_dll"),
            dll_copied: g("dll_copied"),
            windowed_note: g("windowed_note"),
            main_window_found: g("main_window_found"),
            ctrl_t_sent: g("ctrl_t_sent"),
            err_no_exe_path: g("err_no_exe_path"),
            err_no_plugin_dir: g("err_no_plugin_dir"),
            err_dll_missing: g("err_dll_missing"),
            err_dll_copy: g("err_dll_copy"),
            err_no_main_window: g("err_no_main_window"),
            err_sendinput: g("err_sendinput"),
            st_inactive: g("st_inactive"),
            st_pj64_closed: g("st_pj64_closed"),
            st_connected: g("st_connected"),
            st_waiting_game: g("st_waiting_game"),
            st_loading_plugin: g("st_loading_plugin"),
            st_injection_failed: g("st_injection_failed"),
            st_waiting_pj64: g("st_waiting_pj64"),
        }
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

    /// Translate a ROM Settings structural label (category / group title /
    /// parameter name). Absent / empty keys fall back to the English string.
    pub fn tr_settings<'a>(&'a self, name: &'a str) -> &'a str {
        self.strings.settings_names.get(name).map(String::as_str).filter(|s| !s.is_empty()).unwrap_or(name)
    }

    /// Translate an entrance name atom (a `from_name` / `to_name` value). The
    /// composed link rows ("A → B", "A - B") stay language-neutral; only these
    /// atoms are translated. Absent / empty keys fall back to the English name.
    pub fn tr_entrance<'a>(&'a self, name: &'a str) -> &'a str {
        self.strings.entrance_names.get(name).map(String::as_str).filter(|s| !s.is_empty()).unwrap_or(name)
    }

    /// The `[item_names]` translation for a canonical item id, if present and
    /// non-empty (the id resolves through the dense, id-ordered `ITEMS` table).
    fn item_fr_by_id(&self, id: u32) -> Option<&str> {
        let def = item_def_by_id(id)?;
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

    /// Display name for a progression entry. This must be the entry's OWN curated
    /// name (translated), never a sibling item that merely shares its lookup keys
    /// — otherwise "Deku Stick Capacity" (keys `[…, SHARED_STICK_UPGRADE]`) shows
    /// as its shared sibling "Deku Stick Upgrade", and the collectible counters
    /// ("Deku Nuts", "Child Fish") show as a quantified pickup ("5 Deku Nuts",
    /// "Child Fish (2 pounds)"). Resolution order:
    ///   1. a direct `[item_names]` hit on the untagged entry name;
    ///   2. the lookup-key item whose own (game-tag-stripped) name equals the
    ///      entry name — so tagged item names still localize;
    ///   3. the curated English entry name.
    /// The redundant " (OoT)" / " (MM)" tag is stripped from the result (the
    /// overlay is keyed — and often valued — by the tagged `ItemDef.name`, while
    /// the OoT/MM sub-tab already says which game the widget belongs to).
    pub fn tr_prog_entry<'a>(&'a self, name: &'a str, lookup_keys: &[u32]) -> &'a str {
        if let Some(tr) = self.strings.item_names.get(name).map(String::as_str).filter(|s| !s.is_empty()) {
            return strip_game_tag(tr);
        }
        for &id in lookup_keys {
            if let Some(def) = item_def_by_id(id) {
                if strip_game_tag(def.name).eq_ignore_ascii_case(name) {
                    let s = self
                        .strings
                        .item_names
                        .get(def.name)
                        .map(String::as_str)
                        .filter(|s| !s.is_empty())
                        .unwrap_or(name);
                    return strip_game_tag(s);
                }
            }
        }
        name
    }
}

/// The dense, id-ordered `ITEMS` entry for a canonical item id, if any.
fn item_def_by_id(id: u32) -> Option<&'static crate::data::ItemDef> {
    crate::data::ITEMS
        .get((id as usize).wrapping_sub(1))
        .filter(|d| d.id == id)
        .or_else(|| crate::data::ITEMS.iter().find(|d| d.id == id))
}

/// Drop a trailing " (OoT)" / " (MM)" game tag, keeping any other parenthetical.
fn strip_game_tag(s: &str) -> &str {
    s.strip_suffix(" (OoT)")
        .or_else(|| s.strip_suffix(" (MM)"))
        .unwrap_or(s)
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
            i.opt_logic_menu(), i.opt_logic_filter(), i.opt_logic_mode(),
            i.opt_logic_dim(), i.opt_logic_hide(),
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
            i.drop_spoiler_hint(), i.copy_log(), i.unload_patch(), i.patch_unloaded(),
            i.address_placeholder(), i.port_placeholder(),
            i.all(), i.none(), i.choose(), i.search(), i.apply(), i.lang(),
            i.trck_file(), i.xml_file(), i.tracking_files(), i.txt_file(), i.choose_name(), i.choose_trck(), i.choose_spoiler(),
            i.scenes(), i.scenes_title(), i.player(),
            i.all_entrances(), i.gps(), i.entry(),
            i.entrance_col_scene(), i.entrance_col_spawn(), i.entrance_col_leads(),
            i.prog_world(), i.prog_select_item(),
            i.prog_found(), i.prog_not_found(), i.prog_starting_item(),
            i.prog_locations(), i.prog_no_location(), i.prog_not_found_yet(),
            i.departure(), i.arrival(), i.choose_route_scenes(),
            i.gps_route_title(), i.gps_already_there(), i.gps_no_route(), i.gps_whole_scene(),
            i.gps_fastest(), i.gps_via_walk(), i.gps_via_warp_song(), i.gps_via_warp_owl(),
            i.settings_rom_settings(), i.settings_game(), i.settings_build(), i.settings_mode(),
            i.settings_goal(), i.settings_hidden_objs(),
            i.shuffle_vanilla(), i.shuffle_removed(), i.shuffle_starting(),
            i.shuffle_all(), i.shuffle_dungeons(), i.shuffle_overworld(),
            i.reading_mem(), i.log_tracker_stop(), i.file_saved(), i.file_loaded(),
            i.spoiler_loaded(), i.log_reset_tracking(),
            i.load_qt_unsupported(), i.load_qt_partial(),
        ];
        i.load_qt_imported(2);
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
        i.gps_alternative(2);
        i.gps_transitions(3);

        // Multiplayer / patch / live-event log lines (state.rs) — templated.
        i.log_mp_enabled("host:1");
        i.log_r4_enabled("info", "host:1");
        i.log_patch_loaded("p", "info");
        i.log_patch_invalid("e");
        i.log_patch_missing("p");
        i.log_patch_load_failed("e");
        i.log_world_object("OoT", "loc", "item");
        i.log_world_object_net("OoT", "loc", "item", 1, 2);
        i.log_new_scene("a", 1, "b", 2);
        i.log_one_way_in("a", 1);
        i.log_one_way_out("a", 1);
        // Entrance debug-table headers / row labels.
        let tbl: &[&str] = &[
            i.tbl_from(), i.tbl_to(), i.tbl_game(), i.tbl_scene(), i.tbl_entrance(),
            i.tbl_room(), i.tbl_grotto(), i.tbl_age(), i.tbl_farore(), i.tbl_owl(), i.tbl_song(),
        ];
        assert!(tbl.iter().all(|t| !t.is_empty()));
        // Poller / inject snapshot: touches every injection / connection / status key.
        let _ = i.log_strings();
    }

    /// Both shipped locales must define every key the UI asks for.
    #[test]
    fn every_key_present_in_all_locales() {
        for lang in Language::ALL {
            touch_all(&I18n::new(lang));
        }
    }

    /// A progression entry displays its OWN curated name, never a sibling item in
    /// its lookup keys (regression for the "Deku Stick Capacity" widget showing
    /// "Deku Stick Upgrade", and the collectible counters showing quantified
    /// pickups like "5 Deku Nuts" / "Child Fish (2 pounds)").
    #[test]
    fn prog_entry_shows_curated_name_not_sibling() {
        use crate::data::iid;
        let en = I18n::new(Language::English);
        // Capacity widget: keys hold the shared "Deku Stick Upgrade" sibling.
        assert_eq!(
            en.tr_prog_entry("Deku Stick Capacity", &[iid::OOT_STICK_UPGRADE, iid::SHARED_STICK_UPGRADE]),
            "Deku Stick Capacity"
        );
        // Collectible counter: keys are the quantified pickups.
        assert_eq!(en.tr_prog_entry("Deku Nuts", &[iid::OOT_NUTS_5, iid::OOT_NUTS_10]), "Deku Nuts");
        assert_eq!(
            en.tr_prog_entry("Child Fish", &[iid::OOT_FISHING_POND_CHILD_FISH_2LBS]),
            "Child Fish"
        );
        // Localised (FR): still the entry's own item, and with the game tag
        // stripped — asserted structurally so it survives translation rewording.
        let fr = I18n::new(Language::French);
        let cap = fr.tr_prog_entry("Deku Stick Capacity", &[iid::OOT_STICK_UPGRADE, iid::SHARED_STICK_UPGRADE]);
        assert!(!cap.ends_with("(OoT)") && !cap.ends_with("(MM)"), "game tag stripped: {cap:?}");
        assert_ne!(cap, "Deku Stick Capacity", "the FR locale translates this entry");
        // And never the shared "Deku Stick Upgrade" sibling (its FR name).
        let sibling_fr =
            fr.tr_prog_entry("Deku Stick Upgrade", &[iid::OOT_STICK_UPGRADE2, iid::SHARED_STICK_UPGRADE]);
        assert_ne!(cap, sibling_fr, "capacity widget must not borrow the upgrade sibling's name");
    }
}