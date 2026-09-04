//! `TrackerApp` state: construction, persistence, live-event handling,
//! per-frame update loop (`eframe::App`) and the small tab/context helpers.
use eframe::egui::{self, vec2, Vec2};
use std::collections::{HashMap, HashSet, VecDeque};
use std::path::PathBuf;
use std::time::{Duration, Instant};

use crate::*;
use crate::scene::{self, Game, LiveScene};
use crate::shared_mem::Event;
use crate::tracking::RomVersion;
use crate::i18n::{AppSettings, I18n, Language};

impl TrackerApp {
    pub(crate) fn new(ctx: &egui::Context) -> Self {
        // Start with no scene loaded: the map + object panels stay empty until the
        // user picks a scene (or the tracker auto-follows the player into one).
        let scene: Option<LiveScene> = None;
        // Restore the persisted UI language (system language on first launch).
        // Data files live next to the exe when deployed, or in the crate dir in dev.
        let data_dir = scene::data_dir();
        let app_settings_path = data_dir.join("tracker_settings.toml");
        let app_settings = AppSettings::load(&app_settings_path);
        // Restore the persisted multiplayer launch options (checkbox + server).
        let use_multiplayer = app_settings.use_multiplayer;
        let mp_host = app_settings.mp_host.clone();
        let mp_port = app_settings.mp_port.clone();
        let i18n = I18n::new(app_settings.language);
        // Shared, swappable snapshot of the poller / inject journal + status strings
        // for the background thread; rebuilt on a language change (see set_language).
        let log_strings: poller::SharedLog =
            std::sync::Arc::new(std::sync::Mutex::new(std::sync::Arc::new(i18n.log_strings())));
        let mut app = TrackerApp {
            i18n,
            app_settings,
            app_settings_path,
            scene,
            // The poller thread wakes the UI only on a real event / status change.
            poller: poller::spawn(ctx.clone(), log_strings.clone()),
            log_strings,
            connected: false,
            tracking: false,
            auto_save: true,
            use_multiplayer,
            mp_host,
            mp_port,
            multi: None,
            r4: None,
            patch_path: None,
            patch_info: None,
            patch_startup_check: false,
            log_lines: VecDeque::new(),
            worlds: vec![crate::WorldData::default()],
            active_world: 0,
            visited_entrances: HashSet::new(),
            out_links: HashMap::new(),
            in_links: HashMap::new(),
            ent_helper: entrance::EntranceHelper::default(),
            gps_from: None,
            gps_to: None,
            gps_from_ent: None,
            gps_to_ent: None,
            gps_cache: None,
            entrance_sub: EntranceSub::Oot,
            entrance_table: None,
            focus_entrance: None,
            nav_all_expanded: false, // scene trees start collapsed (regions folded)
            obj_all_expanded: true,
            ent_search: String::new(),
            ent_table_search: String::new(),
            ent_all_expanded: true,
            ent_sort_col: 0,
            ent_sort_asc: true,
            ent_col_frac: [0.20, 0.26, 0.27, 0.27],
            mq_scenes: HashSet::new(),
            context_toggle: false,
            scene_search: String::new(),
            obj_search: String::new(),
            active_types: [
                data::OOT_FILTER_TYPES.iter().copied().collect(),
                data::MM_FILTER_TYPES.iter().copied().collect(),
            ],
            current_room: 0,
            active_tab: Tab::Oot,
            sel_scene: [None, None], // nothing shown until the user opens a scene
            last_item: None,
            last_entrance: None,
            player_scene: None,
            player_obj_scene: None,
            followed_scene: None,
            pending_snap: None,
            snap_pos: None,
            show_settings: false,
            settings_nav: 0,
            show_about: false,
            rom_settings: settings::Settings::default(),
            excluded: settings::Excluded::default(),
            dashboard: progression::Dashboard::new(),
            prog_dirty: true,
            cached_totals: [(0, 0); 2],
            cached_scene_counts: [HashMap::new(), HashMap::new()],
            counts_dirty: true,
            autosave_dir: data_dir.join("autosave"),
            seed_tag: "empty".to_string(),
            legacy_save_path: data_dir.join("tracker_save.txt"),
            spoiler_path_file: data_dir.join("tracker_spoiler.txt"),
            dirty: false,
            status: String::new(),
            rom: RomVersion::Dev,
            rom_from_spoiler: false,
            uses_legacy_xflags: false,
            icon_cache: HashMap::new(),
            grey_icon_cache: HashMap::new(),
            glow_icon_cache: HashMap::new(),
            map_texture: None,
            map_size: Vec2::ZERO,
            load_error: None,
            zoom: 1.0,
            pan: Vec2::ZERO,
            view_initialized: false,
            last_frame: Instant::now(),
            kbd: ui::kbdnav::KbdNav::default(),
            reach: None,
            logic_dirty: true,
            logic_locs: crate::logic::logic_location_set(),
        };
        // Restore the persisted "reveal uncollected placements" toggle into the
        // dashboard (its runtime home, which drives the map + progression trees).
        app.dashboard.reveal = app.app_settings.reveal;
        // Startup auto-loads, each gated by its "Auto Load Most Recent" option.
        if app.app_settings.auto_load_tracking {
            app.load_state(); // restore previous progress (may set patch_path)
        }
        if app.app_settings.auto_load_spoiler {
            // Applies the last spoiler's settings / MQ layout AND, since it switches
            // to that seed's autosave, may restore a different patch path than the
            // no-seed file did — so the patch check below runs after it.
            app.auto_load_spoiler();
        }
        // If a restored save carried a patch path, resolve it on the first frame
        // (loads it, or warns + prompts if the file moved / disappeared).
        app.patch_startup_check = app.patch_path.is_some();
        app // the poller thread handles connection / injection from here
    }

    /// Re-load the most recently used spoiler (path remembered in the sidecar), so
    /// its ROM settings and MQ layout filter the map from launch — mirroring the Qt
    /// AutoLoadMostRecentSpoilerLog. A missing / stale path is silently ignored.
    pub(crate) fn auto_load_spoiler(&mut self) {
        let Ok(path) = std::fs::read_to_string(&self.spoiler_path_file) else { return };
        let path = PathBuf::from(path.trim());
        if path.is_file() {
            self.load_spoiler(&path);
        }
    }

    /// The displayed world's data (every map / tree / progression view reads it).
    pub(crate) fn cw(&self) -> &crate::WorldData {
        &self.worlds[self.active_world]
    }

    /// Number of allocated worlds (at least 1). Mirrors Qt `GetNumWorlds`.
    pub(crate) fn num_worlds(&self) -> usize {
        self.worlds.len().max(1)
    }

    /// Allocate the world set from the ROM Mode / team count (Qt SettingsTab
    /// `SaveSettingsFromUI`), so the world selector appears even WITHOUT a spoiler:
    /// Multiworld with N teams -> N worlds, everything else collapses to one.
    /// Existing worlds keep their placements / collected; new ones start empty.
    pub(crate) fn sync_worlds_from_settings(&mut self) {
        let desired = if self.rom_settings.mode == settings::GameMode::Multi {
            self.rom_settings.num_teams.max(1)
        } else {
            1
        };
        if desired == self.worlds.len() {
            return;
        }
        if self.worlds.len() < desired {
            self.worlds.resize_with(desired, crate::WorldData::default);
        } else {
            self.worlds.truncate(desired);
        }
        if self.active_world >= desired {
            self.active_world = 0;
        }
        self.dashboard.set_active_world((self.active_world + 1) as u8);
        self.rebuild_scene();
        self.sync_collected();
        self.prog_dirty = true;
        self.counts_dirty = true;
    }

    /// Switch the displayed world (Qt `OnWorldSelected` / `SetActiveWorld`): every
    /// view re-points at that world's placements + collected marks, and the
    /// progression dashboard re-filters to the items destined to it. Re-derives the
    /// scene, dashboard and counters so the maps and progression tab update at once.
    pub(crate) fn set_active_world(&mut self, world: usize) {
        let world = world.min(self.num_worlds().saturating_sub(1));
        if world == self.active_world {
            return;
        }
        self.active_world = world;
        // The dashboard filters placements by their 1-based destination player.
        self.dashboard.set_active_world((world + 1) as u8);
        self.rebuild_scene(); // the active world's placements/collected changed
        self.sync_collected(); // repaint the markers this frame, no uncollected flash
        self.prog_dirty = true;
        self.counts_dirty = true;
    }

    /// Persist the collected-set, keyed by each object's globally-unique
    /// Location so the file survives data regeneration / layout changes.
    /// Append a line to the Launch journal (Qt `LogTab::LogMessage`).
    pub(crate) fn log_msg(&mut self, msg: impl Into<String>) {
        if self.log_lines.len() >= LOG_CAP {
            self.log_lines.pop_front();
        }
        self.log_lines.push_back(msg.into());
    }

    /// Start / stop the auto-tracker (Qt `PressLaunchButton`): flip the state,
    /// drive the poller and update the journal. The status pill reflects the
    /// same flag.
    pub(crate) fn toggle_tracking(&mut self, ctx: &egui::Context) {
        // Starting the tracker: the r4 method needs a patch file. If none is loaded
        // yet, prompt for one now (the DLL hook still works as a backup if the user
        // cancels the dialog).
        if !self.tracking && self.patch_info.is_none() {
            self.prompt_patch_dialog();
        }
        self.tracking = !self.tracking;
        self.poller.set_tracking(self.tracking);
        if self.tracking {
            // A loaded patch selects the r4 mechanism (dev OoTMM builds only, the
            // ones exposing the emulator IPC pipe); otherwise the old net-context
            // client runs when multiplayer is on. On a non-dev build the pipe is
            // absent, r4 never validates HELLO, and the DLL hook stays authoritative.
            if self.patch_info.is_some() {
                self.start_r4(ctx);
            } else if self.use_multiplayer {
                self.start_multiplayer(ctx);
            }
            self.log_msg(self.i18n.reading_mem().to_string());
        } else {
            self.stop_multiplayer();
            self.stop_r4();
            self.log_msg(self.i18n.log_tracker_stop().to_string());
        }
    }

    /// Spawn the multiplayer client (Qt `App::appRun` on `TrackerThread`): listen
    /// for the custom Project64 build, relay ledger entries to / from the server.
    /// The DLL hook path (poller) still runs in parallel.
    pub(crate) fn start_multiplayer(&mut self, ctx: &egui::Context) {
        if self.multi.is_some() {
            return;
        }
        let port = self.mp_port.trim().parse::<u16>().unwrap_or(13248);
        let cfg = multi::MultiConfig {
            server_host: self.mp_host.trim().to_string(),
            server_port: port,
            net_enabled: true,
        };
        let server = format!("{}:{}", cfg.server_host, cfg.server_port);
        self.log_msg(self.i18n.log_mp_enabled(&server));
        self.multi = Some(multi::spawn(ctx.clone(), cfg));
    }

    /// Stop the multiplayer client if it is running (joins its thread).
    pub(crate) fn stop_multiplayer(&mut self) {
        if let Some(mut handle) = self.multi.take() {
            handle.stop();
        }
    }

    /// Spawn the r4 multiplayer client (dev OoTMM builds only): it connects to the
    /// emulator's named pipe and relays WAL entries to / from the OoTMM server,
    /// deriving the session identity from the loaded patch. The DLL hook path
    /// (poller) keeps running in parallel as a backup — and becomes the sole source
    /// of truth on non-dev builds, where the pipe never appears.
    pub(crate) fn start_r4(&mut self, ctx: &egui::Context) {
        if self.r4.is_some() {
            return;
        }
        let Some(info) = self.patch_info.clone() else { return };
        // The r4 server lives at multi.ootmm.com:14236 (distinct from the old
        // mechanism's port); the host field is reused so a custom server works.
        let host = self.mp_host.trim();
        let host = if host.is_empty() { "multi.ootmm.com" } else { host };
        let cfg = multi_r4::R4Config {
            server_host: host.to_string(),
            server_port: 14236,
            data_dir: r4_data_dir(),
        };
        let server = format!("{}:{}", cfg.server_host, cfg.server_port);
        self.log_msg(self.i18n.log_r4_enabled(&info.summary(), &server));
        self.r4 = Some(multi_r4::spawn(ctx.clone(), cfg, info));
    }

    /// Stop the r4 client if it is running (joins its thread).
    pub(crate) fn stop_r4(&mut self) {
        if let Some(mut handle) = self.r4.take() {
            handle.stop();
        }
    }

    /// Resolve the patch path restored from the save at startup: load the patch
    /// if it still exists, otherwise warn the user and open a dialog to pick a new
    /// one (mirror of the requested "load the referenced file, or prompt if gone").
    pub(crate) fn resolve_startup_patch(&mut self) {
        let Some(path) = self.patch_path.clone() else { return };
        if path.is_file() {
            match patch::load(&path) {
                Ok(info) => {
                    self.log_msg(self.i18n.log_patch_loaded(&path.display().to_string(), &info.summary()));
                    self.patch_info = Some(info);
                }
                Err(e) => {
                    self.log_msg(self.i18n.log_patch_invalid(&e.to_string()));
                    self.prompt_patch_dialog();
                }
            }
        } else {
            self.log_msg(self.i18n.log_patch_missing(&path.display().to_string()));
            self.prompt_patch_dialog();
        }
    }

    /// Open a file dialog to choose a patch file (`.ootmm` or the `.zip` bundling
    /// it) and load it. Used at startup (file moved), from the "Load Patch" button,
    /// and from Start Tracking when no valid patch is available.
    pub(crate) fn prompt_patch_dialog(&mut self) {
        if let Some(path) = dialog::open_file(
            self.i18n.choose_patch(),
            &[(self.i18n.patch_file(), "*.ootmm;*.zip"), (self.i18n.all_files(), "*.*")],
        ) {
            self.set_patch(path);
        }
    }

    /// Load `path` as a patch file: on success store the path + parsed session
    /// info and persist it in the save; on failure keep the previous patch and log.
    pub(crate) fn set_patch(&mut self, path: std::path::PathBuf) {
        match patch::load(&path) {
            Ok(info) => {
                self.log_msg(self.i18n.log_patch_loaded(&path.display().to_string(), &info.summary()));
                self.patch_path = Some(path);
                self.patch_info = Some(info);
                self.save_state(); // persist the new patch path in the save file
            }
            Err(e) => self.log_msg(self.i18n.log_patch_load_failed(&e.to_string())),
        }
    }

    /// Unload the selected patch (the Launch ✕ button): drop the path + parsed
    /// session info, stop the r4 client if it was running (it derives its identity
    /// from the patch), and persist the removal. The DLL hook keeps tracking.
    pub(crate) fn clear_patch(&mut self) {
        if self.patch_path.is_none() && self.patch_info.is_none() {
            return;
        }
        self.stop_r4();
        self.patch_path = None;
        self.patch_info = None;
        self.log_msg(self.i18n.patch_unloaded().to_string());
        self.save_state(); // drop the PATCH line from the save file
    }

    /// Persist the multiplayer launch options (checkbox + host + port) when they
    /// change, so they survive across sessions (Qt `AppConfig` UseMultiplayer /
    /// Host / Port). Called from the Launch page after the widgets are edited.
    pub(crate) fn persist_launch_settings(&mut self) {
        if self.app_settings.use_multiplayer != self.use_multiplayer
            || self.app_settings.mp_host != self.mp_host
            || self.app_settings.mp_port != self.mp_port
        {
            self.app_settings.use_multiplayer = self.use_multiplayer;
            self.app_settings.mp_host = self.mp_host.clone();
            self.app_settings.mp_port = self.mp_port.clone();
            self.app_settings.save(&self.app_settings_path);
        }
    }

    /// Toggle the "auto save".
    pub(crate) fn set_auto_save(&mut self, v: bool) {
        if self.auto_save != v {
            self.auto_save = v;
        }
    }

    /// Toggle "reveal uncollected placements" and persist it. The flag lives in the
    /// dashboard (it drives the map + progression trees), and is mirrored into the
    /// saved `AppSettings` so the choice survives a restart.
    pub(crate) fn set_reveal(&mut self, v: bool) {
        self.dashboard.set_reveal(v);
        if self.app_settings.reveal != v {
            self.app_settings.reveal = v;
            self.app_settings.save(&self.app_settings_path);
        }
    }

    /// Reset all tracked progress (Qt `LogTab::ResetTracking`): autosave first,
    /// then clear collected items, forced marks and discovered entrances.
    pub(crate) fn reset_tracking(&mut self) {
        self.save_state(); // Qt autosaves the current state before wiping it
        // Back to a no-spoiler run: future autosaves target `empty.xml`, so the seed
        // file we just flushed keeps its progress instead of being overwritten empty.
        self.seed_tag = "empty".to_string();
        // Full clean slate: the Qt `ResetObject` clears both `Status` AND `Item`, so
        // a reset drops the loaded spoiler too — not only the collected marks. Left
        // in place, the placements linger as item traces on every actor and the
        // spoiler's starting items stay lit on the progression tab. Reset every
        // spoiler-derived field back to its fresh-launch value so the tracker looks
        // exactly like a launch with no spoiler loaded.
        self.worlds = vec![crate::WorldData::default()];
        self.active_world = 0;
        self.dashboard.set_active_world(1);
        self.rom_settings = settings::Settings::default();
        self.excluded = settings::Excluded::default();
        self.mq_scenes.clear();
        self.rom_from_spoiler = false;
        self.uses_legacy_xflags = false;
        tracking::set_uses_legacy(false);
        // Live exploration (entrances / links / last picks).
        self.visited_entrances.clear();
        self.out_links.clear();
        self.in_links.clear();
        self.ent_helper = entrance::EntranceHelper::default();
        self.last_item = None;
        self.last_entrance = None;
        self.dirty = true;
        self.prog_dirty = true;
        self.counts_dirty = true;
        self.rebuild_scene(); // reload the current scene under the now-empty MQ set
        self.log_msg(self.i18n.log_reset_tracking().to_string());
    }

    /// "Save Tracking": pick an `.xml` file and write the current state to it.
    /// Saves are always the XML format now (the legacy Qt `.trck` binary is
    /// read-only — importable via "Load Tracking", never written).
    pub(crate) fn save_tracking_dialog(&mut self) {
        if let Some(path) =
            dialog::save_file(self.i18n.choose_name(), &[(self.i18n.xml_file(), "*.xml")], "xml")
        {
            // Create Backup When Saving: copy any existing file to `.bak` first.
            if self.app_settings.backup_on_save && path.exists() {
                let _ = std::fs::copy(&path, path.with_extension("bak"));
            }
            self.save_to(&path);
            self.log_msg(format!("{} {}", self.i18n.file_saved(), path.display()));
        }
    }

    /// "Load Tracking": pick an `.xml` save or a legacy `.trck` binary and restore
    /// the state from it. `load_from` auto-detects the format from the file
    /// contents, so both extensions are offered here.
    pub(crate) fn load_tracking_dialog(&mut self) {
        if let Some(path) = dialog::open_file(
            self.i18n.choose_trck(),
            &[
                (self.i18n.tracking_files(), "*.xml;*.trck"),
                (self.i18n.xml_file(), "*.xml"),
                (self.i18n.trck_file(), "*.trck"),
            ],
        ) {
            for w in &mut self.worlds {
                w.collected.clear();
                w.forced.clear();
            }
            self.visited_entrances.clear();
            self.out_links.clear();
            self.in_links.clear();
            self.load_from(&path);
            self.dirty = true;
            self.prog_dirty = true;
            self.counts_dirty = true;
            self.rebuild_scene();
            self.log_msg(format!("{} {}", self.i18n.file_loaded(), path.display()));
        }
    }

    /// "Load Spoiler Log": pick a `.txt` spoiler and apply it.
    pub(crate) fn load_spoiler_dialog(&mut self) {
        if let Some(path) =
            dialog::open_file(self.i18n.choose_spoiler(), &[(self.i18n.txt_file(), "*.txt")])
        {
            self.load_spoiler(&path);
            self.log_msg(format!("{} {}", self.i18n.spoiler_loaded(), path.display()));
        }
    }

    /// The active per-seed autosave file: `<autosave_dir>/<seed_tag>.xml`. The stem
    /// is the loaded spoiler's seed hash, or `empty` when no spoiler is loaded, so a
    /// new seed lands in a new file while the no-seed run always overwrites one.
    fn autosave_path(&self) -> PathBuf {
        self.autosave_dir.join(format!("{}.xml", self.seed_tag))
    }

    pub(crate) fn save_state(&self) {
        let _ = std::fs::create_dir_all(&self.autosave_dir);
        self.save_to(&self.autosave_path());
    }

    /// Serialize the tracked state to a human-readable / hand-editable XML save
    /// (`<tracker version="6">`). Shared by the autosave and the "Save Tracking"
    /// dialog. See [`Self::load_from_xml`] for the reader.
    ///
    /// Layout: `<paths>` (remembered spoiler + r4 patch), then `<worlds>` — one
    /// `<world index="N">` (1-based) per world, its collected/placed locations
    /// grouped by scene — then `<entrances>` (visited flag + discovered out/in
    /// links, grouped by scene). A `<location>` loads on its numeric identity
    /// (`oid`/`type`/`layout` within the parent `<scene>`), with `loc` as a
    /// fallback; a placed `<item>` loads on its stable `id`, with `name` as a
    /// fallback. The remaining `name` annotations are readable and ignored on load.
    pub(crate) fn save_to(&self, path: &std::path::Path) {
        // The remembered spoiler (sidecar) + the r4 patch path go in <paths>.
        let spoiler = std::fs::read_to_string(&self.spoiler_path_file)
            .ok()
            .map(|s| s.trim().to_string())
            .filter(|s| !s.is_empty());
        let patch = self.patch_path.as_ref().map(|p| p.display().to_string());
        let xml = render_save_xml(
            &self.worlds,
            patch.as_deref(),
            spoiler.as_deref(),
            &self.visited_entrances,
            &self.out_links,
            &self.in_links,
        );
        let _ = std::fs::write(path, xml);
    }

    /// Restore the collected-set from the active seed's autosave file (by Location).
    /// Falls back once to the pre-3.0 single autosave (`tracker_save.txt`) when no
    /// per-seed file exists yet, so upgrading users keep their last no-seed progress.
    pub(crate) fn load_state(&mut self) {
        let path = self.autosave_path();
        if path.is_file() {
            self.load_from(&path);
        } else if self.seed_tag == "empty" && self.legacy_save_path.is_file() {
            self.load_from(&self.legacy_save_path.clone());
        }
    }

    /// Restore the tracked state from a file (by Location). Shared by the startup
    /// autoload and the "Load Tracking" dialog.
    pub(crate) fn load_from(&mut self, path: &std::path::Path) {
        let Ok(bytes) = std::fs::read(path) else { return };
        // A Qt binary `.trck` leads with a small `u32` format version (0..=3); a
        // Rust text save starts with ASCII ("TRACKER_SAVE", "OOT", "MM", "F ", …)
        // whose first four bytes as a `u32` are far larger. So a tiny leading
        // value means the file is a Qt binary save — import it instead.
        if bytes.len() >= 4 && u32::from_le_bytes([bytes[0], bytes[1], bytes[2], bytes[3]]) <= 3 {
            match qtsave::parse(&bytes) {
                Some(save) => self.apply_qt_import(save),
                None => self.log_msg(self.i18n.load_qt_unsupported().to_string()),
            }
            return;
        }
        let Ok(text) = String::from_utf8(bytes) else { return };
        // Version 5+: an XML document (`<?xml …` / `<tracker …`). Older Rust saves
        // are the legacy line-based text format parsed below.
        let head = text.trim_start();
        if head.starts_with("<?xml") || head.starts_with("<tracker") {
            self.load_from_xml(&text);
            return;
        }
        // Location is globally unique (base and MQ carry distinct strings), so a
        // flat index has no collisions and restores either layout.
        let mut by_location: HashMap<&'static str, (Game, usize)> = HashMap::new();
        for game in [Game::Oot, Game::Mm] {
            for (i, o) in game.objects().iter().enumerate() {
                by_location.insert(o.location, (game, i));
            }
        }
        // Collected / forced lines apply to the world named by the last `WORLD`
        // marker (0-based); untagged files load into world 0 (old single-world
        // saves). The world Vec is grown on demand so a save with more worlds than
        // are currently allocated still restores fully.
        let mut cur_world = 0usize;
        for line in text.lines() {
            let line = line.trim();
            if line.starts_with(SAVE_VERSION_TAG) {
                // Save-format version marker (latest-version saves only); no action.
            } else if let Some(rest) = line.strip_prefix("PATCH ") {
                // r4 patch file path, restored so startup can auto-load it.
                self.patch_path = Some(std::path::PathBuf::from(rest.trim()));
            } else if let Some(rest) = line.strip_prefix("WORLD ") {
                if let Ok(n) = rest.trim().parse::<usize>() {
                    cur_world = n;
                    if self.worlds.len() <= n {
                        self.worlds.resize_with(n + 1, crate::WorldData::default);
                    }
                }
            } else if let Some(rest) = line.strip_prefix("E ") {
                let mut it = rest.split_whitespace();
                if let (Some(Ok(g)), Some(Ok(id))) =
                    (it.next().map(str::parse::<u8>), it.next().map(str::parse::<u32>))
                {
                    let game = if g == 1 { Game::Mm } else { Game::Oot };
                    self.visited_entrances.insert((game, id));
                }
            } else if let Some(rest) = line.strip_prefix("O ") {
                let mut it = rest.split_whitespace();
                if let (Some(Ok(og)), Some(Ok(oid)), Some(Ok(ig)), Some(Ok(iid))) = (
                    it.next().map(str::parse::<u8>),
                    it.next().map(str::parse::<u32>),
                    it.next().map(str::parse::<u8>),
                    it.next().map(str::parse::<u32>),
                ) {
                    let og = if og == 1 { Game::Mm } else { Game::Oot };
                    let ig = if ig == 1 { Game::Mm } else { Game::Oot };
                    self.out_links.insert((og, oid), (ig, iid));
                }
            } else if let Some(rest) = line.strip_prefix("ITEM ") {
                // Version 4: a persisted placement (location <TAB> item name).
                if let Some((loc, item)) = rest.split_once('\t') {
                    self.worlds[cur_world].items.insert(loc.to_string(), item.to_string());
                }
            } else if let Some(rest) = line.strip_prefix("DEST ") {
                // Version 4: a placement's destination player (location <TAB> N).
                if let Some((loc, player)) = rest.split_once('\t') {
                    if let Ok(p) = player.trim().parse::<u8>() {
                        self.worlds[cur_world].dest.insert(loc.to_string(), p);
                    }
                }
            } else if let Some(loc) = line.strip_prefix("F ") {
                // Manually-forced object: collected AND forced (gold on the map).
                if let Some(&key) = by_location.get(loc) {
                    let w = &mut self.worlds[cur_world];
                    w.collected.insert(key);
                    w.forced.insert(key);
                }
            } else if let Some(&key) = by_location.get(line) {
                self.worlds[cur_world].collected.insert(key);
            }
        }
        self.rebuild_in_links();
    }

    /// Apply a parsed Qt binary `.trck` (imported by [`qtsave`]): restore each
    /// world's collected / forced marks, the item placements (so icons show with
    /// no spoiler loaded, like the Qt app) and the discovered out-links. Callers
    /// clear the tracked state first, exactly as for a text load.
    pub(crate) fn apply_qt_import(&mut self, save: qtsave::QtSave) {
        // Per-game (scene id, object id) -> (object index, Location). Qt matches
        // objects by id (layout-resilient), so we do too rather than by position.
        let mut lut: [HashMap<(u32, u32), (usize, &'static str)>; 2] =
            [HashMap::new(), HashMap::new()];
        for game in [Game::Oot, Game::Mm] {
            for (i, o) in game.objects().iter().enumerate() {
                lut[game.idx()].insert((o.scene as u32, o.object_id), (i, o.location));
            }
        }

        for (w, objs) in save.worlds.into_iter().enumerate() {
            if self.worlds.len() <= w {
                self.worlds.resize_with(w + 1, crate::WorldData::default);
            }
            for ob in objs {
                let Some(&(idx, location)) = lut[ob.game.idx()].get(&(ob.scene, ob.obj_id)) else {
                    continue; // an object the current data no longer has
                };
                if ob.status == qtsave::ST_COLLECTED || ob.status == qtsave::ST_FORCED {
                    self.worlds[w].collected.insert((ob.game, idx));
                    if ob.status == qtsave::ST_FORCED {
                        self.worlds[w].forced.insert((ob.game, idx));
                    }
                }
                if let Some(name) = qtsave::item_name(ob.item_id) {
                    self.worlds[w].items.insert(location.to_string(), name.to_string());
                }
            }
        }

        for (out, inc) in save.out_links {
            self.out_links.insert(out, inc);
        }
        // Restore the full in-link lists (multiple discovered sources per target),
        // dropping any that no longer exist in the current entrance data.
        for (target, sources) in save.in_links {
            let list = self.in_links.entry(target).or_default();
            for src in sources {
                if !list.contains(&src) {
                    list.push(src);
                }
            }
        }
        // Mark every touched entrance visited so the found/total counters reflect
        // the loaded state (the Qt save has no separate "visited" flag).
        for v in save.visited {
            self.visited_entrances.insert(v);
        }

        self.log_msg(self.i18n.load_qt_imported(save.version));
        if save.partial {
            self.log_msg(self.i18n.load_qt_partial().to_string());
        }
    }

    /// Restore the tracked state from a version-6 XML save (see [`Self::save_to`]).
    /// Locations are matched by their unique `loc` attribute (layout-resilient);
    /// the entrance graph (visited flag + out/in links) is restored directly, so —
    /// unlike the legacy text load — no in-link rebuild is needed. Malformed XML
    /// stops the parse and keeps whatever was read so far.
    fn load_from_xml(&mut self, text: &str) {
        let parsed = parse_save_xml(text);
        // Merge into the live state. Dialog loads clear collected / forced /
        // entrances first; the startup autoload merges into a fresh state. Growing
        // the world Vec on demand restores a save with more worlds than allocated.
        for (wi, w) in parsed.worlds.into_iter().enumerate() {
            if self.worlds.len() <= wi {
                self.worlds.resize_with(wi + 1, crate::WorldData::default);
            }
            let tw = &mut self.worlds[wi];
            tw.collected.extend(w.collected);
            tw.forced.extend(w.forced);
            tw.items.extend(w.items);
            tw.dest.extend(w.dest);
        }
        self.visited_entrances.extend(parsed.visited);
        for (k, v) in parsed.out_links {
            self.out_links.insert(k, v);
        }
        for (k, srcs) in parsed.in_links {
            let list = self.in_links.entry(k).or_default();
            for s in srcs {
                if !list.contains(&s) {
                    list.push(s);
                }
            }
        }
        if let Some(p) = parsed.patch_path {
            self.patch_path = Some(p);
        }
        // Remember the referenced spoiler so the startup auto-load re-applies its
        // ROM settings / MQ layout (mirrors how load_spoiler persists the path).
        if let Some(sp) = parsed.spoiler_path {
            let _ = std::fs::write(&self.spoiler_path_file, sp);
        }
    }

    /// Drain the poller thread's messages: update the connection status and
    /// process any live game events. Runs once per UI frame (a frame only happens
    /// on input or when the poller woke us).
    pub(crate) fn process_poll(&mut self) {
        while let Ok(msg) = self.poller.rx.try_recv() {
            match msg {
                poller::PollMsg::Status(connected, text) => {
                    // Short status-bar summary (the journal gets the detailed
                    // step-by-step Log messages instead).
                    self.connected = connected;
                    self.status = text;
                }
                poller::PollMsg::Log(line) => {
                    self.log_msg(line);
                }
                poller::PollMsg::Events(events, game_version) => {
                    // The DLL fingerprint is a fallback; a loaded spoiler's version
                    // is authoritative and must not be overridden (C++ guard).
                    if game_version[0] != 0 && !self.rom_from_spoiler {
                        self.rom = tracking::detect_rom(game_version);
                    }
                    for ev in events {
                        self.process_event(ev);
                    }
                }
            }
        }

        // Drain the multiplayer client thread: its journal lines and the ledger
        // transfers it decoded (buffer first so applying them can borrow `self`).
        let mut net_msgs = Vec::new();
        if let Some(handle) = self.multi.as_ref() {
            while let Ok(msg) = handle.rx.try_recv() {
                net_msgs.push(msg);
            }
        }
        for msg in net_msgs {
            match msg {
                multi::MultiMsg::Log(line) => self.log_msg(line),
                multi::MultiMsg::Item(item) => self.apply_net_item(item),
            }
        }

        // Drain the r4 client thread the same way (shares NetItem / apply_net_item).
        let mut r4_msgs = Vec::new();
        if let Some(handle) = self.r4.as_ref() {
            while let Ok(msg) = handle.rx.try_recv() {
                r4_msgs.push(msg);
            }
        }
        for msg in r4_msgs {
            match msg {
                multi_r4::R4Msg::Log(line) => self.log_msg(line),
                multi_r4::R4Msg::Item(item) => self.apply_net_item(item),
            }
        }
    }

    /// Apply a decoded network ledger transfer to the tracked worlds (port of
    /// `OoTMMComboTracker::UpdateTrackedObject`). The check physically lives in the
    /// "from" world, so it is marked collected there; the destination world's
    /// progression is re-derived from that placement's spoiler destination. A
    /// single-world seed (single / coop) collapses everything onto world 0.
    pub(crate) fn apply_net_item(&mut self, item: multi::NetItem) {
        let game = if item.game_id == 0 { Game::Oot } else { Game::Mm };
        let Some((g, idx)) =
            tracking::match_object(game, item.scene, item.ov_type, item.object, item.room, self.rom, self.uses_legacy_xflags, &self.mq_scenes)
        else {
            return; // no matching placement (e.g. filtered / unknown overlay)
        };

        // The map world is where the check physically lives (the "from" world).
        let single_world = self.num_worlds() <= 1;
        let map_world = if single_world || item.from_world <= 0 { 1 } else { item.from_world as usize };
        let map_idx = map_world - 1;
        if map_idx >= self.worlds.len() {
            return;
        }

        // Prefer the ledger's own item name (Qt `FindItem`); fall back to the
        // object's default name when the id is unknown (e.g. a "nothing" drop).
        let name = tracking::net_item_name(item.gi, self.rom)
            .map(str::to_string)
            .unwrap_or_else(|| g.objects()[idx].name.to_string());
        self.last_item = Some(name.clone());

        if self.worlds[map_idx].collected.insert((g, idx)) {
            let obj = &g.objects()[idx];
            let tag = if g == Game::Oot { "OoT" } else { "MM" };
            self.log_msg(self.i18n.log_world_object_net(
                tag, obj.location, &name, item.from_world, item.to_world,
            ));
            self.dirty = true;
            self.prog_dirty = true;
            self.counts_dirty = true;
            // Auto Snap View only when the affected world is the one on screen.
            if self.app_settings.auto_snap && self.active_world == map_idx {
                self.pending_snap = Some((g, obj.render_scene, obj.room, obj.x as f32, obj.y as f32));
            }
        }
    }

    /// Queue a hook-captured "nothing" drop for the multiplayer client to push to
    /// the ledger (mirror of `MemoryReader`'s `QueueTrackerNothing`). Only in coop,
    /// only while the multiplayer client is running.
    fn forward_nothing_drop(&self, ev: &Event) {
        let Some(handle) = self.multi.as_ref() else { return };
        if self.rom_settings.mode != settings::GameMode::Coop {
            return;
        }
        // A "nothing" drop: high half of Query[2] is 0xFFFF; low byte selects the
        // game. The `>> 2` gate skips events already flagged treated (as the DLL
        // hook does before writing back the treated flag).
        if ev.query[2] & 0xFFFF_0000 != 0xFFFF_0000 || (ev.query[2] & 0x0000_FF00) >> 2 != 0 {
            return;
        }
        handle.queue_nothing(multi::TrackerNothing {
            game_id: if ev.query[2] & 0xFF == 0 { 0 } else { 1 },
            // Raw Query[0]: already in the big-endian order the ledger round-trip
            // expects (a local byteswap would double-reverse it).
            key: ev.query[0],
            gi: (ev.query[1] & 0xFFFF) as u16,
        });
    }

    /// Handle one live event: an entrance message or a collected item.
    pub(crate) fn process_event(&mut self, ev: Event) {
        if entrance::is_entrance(ev.mem) {
            self.handle_entrance(&ev);
            return;
        }
        // Coop: forward hook-captured "nothing" drops (which never travel over the
        // wire) to the ledger so the whole team's shared map stays in sync.
        self.forward_nothing_drop(&ev);
        // "Nothing" drops are always hook-owned; real items are owned by the
        // network ledger in a coop / multi seed (mirror of UpdateTrackedObject's
        // HookItem gate). Skipping the hook for real items avoids marking them in
        // the local world when the ledger will place them in the correct world.
        let is_nothing = ev.query[2] & 0xFFFF_0000 == 0xFFFF_0000;
        // The old net-context client owns real items in a coop / multi seed; the r4
        // client (WAL) owns them only when it has a LIVE session matching the patch
        // (non-single). If the loaded game predates the IPC or its session differs
        // from the patch, r4 isn't connected → the hook keeps the item (fallback).
        let old_owns = self.multi.is_some() && self.rom_settings.mode != settings::GameMode::Single;
        let r4_owns = self.r4.as_ref().map(|h| h.is_connected()).unwrap_or(false)
            && self.patch_info.as_ref().map(|p| p.mode != patch::PatchMode::Single).unwrap_or(false);
        let net_owns_real = old_owns || r4_owns;
        if let Some(hit) = tracking::resolve_collected(&ev, self.rom, self.uses_legacy_xflags, &self.mq_scenes) {
            if !is_nothing && net_owns_real {
                return; // let the network ledger own this real item
            }
            let obj = &hit.0.objects()[hit.1];
            // A live pickup is the local player's, so it always lands in the local
            // world (index 0) regardless of which world the user is viewing. Its
            // shown name comes from the local world's placement.
            const LOCAL: usize = 0;
            let item = self.worlds[LOCAL]
                .items
                .get(obj.location)
                .cloned()
                .unwrap_or_else(|| obj.name.to_string());
            self.last_item = Some(item.clone());
            let (rs, room, ox, oy) = (obj.render_scene, obj.room, obj.x as f32, obj.y as f32);
            if self.worlds[LOCAL].collected.insert(hit) {
                // Mirror the Qt MemoryReader log line.
                let game = if hit.0 == Game::Oot { "OoT" } else { "MM" };
                self.log_msg(self.i18n.log_world_object(game, obj.location, &item));
                self.dirty = true;
                self.prog_dirty = true;
                self.counts_dirty = true;
                // Auto Snap View: remember where to recentre the map next frame,
                // but only while the local world is the one on screen (else the
                // mark isn't visible on the world the user is looking at).
                if self.app_settings.auto_snap && self.active_world == LOCAL {
                    self.pending_snap = Some((hit.0, rs, room, ox, oy));
                }
            }
        }
    }

    /// Load a spoiler log file: item placement + ROM build.
    pub(crate) fn load_spoiler(&mut self, path: &std::path::Path) {
        match std::fs::read_to_string(path) {
            Ok(text) => {
                // A different seed is a distinct playthrough: flush the outgoing
                // seed's progress to its own autosave, then start from a clean
                // collected set (restored from the new seed's file further down).
                // Re-loading the SAME seed keeps the in-memory progress (below).
                let new_seed = seed_tag_from_spoiler(&text);
                let seed_changed = new_seed != self.seed_tag;
                if seed_changed {
                    let has_progress = self.worlds.iter().any(|w| !w.collected.is_empty())
                        || !self.visited_entrances.is_empty();
                    if self.auto_save && has_progress {
                        self.save_state(); // preserve the previous seed's file
                    }
                    for w in &mut self.worlds {
                        w.collected.clear();
                        w.forced.clear();
                    }
                    self.visited_entrances.clear();
                    self.out_links.clear();
                    self.in_links.clear();
                    self.seed_tag = new_seed;
                }
                let sp = spoiler::parse(&text);
                self.rom = sp.rom;
                self.rom_from_spoiler = true;
                self.uses_legacy_xflags = sp.uses_legacy_xflags;
                tracking::set_uses_legacy(sp.uses_legacy_xflags);
                self.mq_scenes = sp.mq_scenes;
                let num_worlds = sp.worlds.len().max(1);
                // Re-fill each world's placements, PRESERVING the collected / forced
                // sets across the reload (Qt `preservedStatus`) so re-loading the same
                // spoiler never wipes progress — critical with auto-save on. Grow
                // first, set the placements, then drop any world the seed no longer has.
                if self.worlds.len() < num_worlds {
                    self.worlds.resize_with(num_worlds, crate::WorldData::default);
                }
                for (i, wp) in sp.worlds.into_iter().enumerate() {
                    let w = &mut self.worlds[i];
                    w.items = wp.items;
                    w.dest = wp.dest;
                }
                self.worlds.truncate(num_worlds);
                // Keep the displayed world in range and sync the dashboard filter.
                if self.active_world >= num_worlds {
                    self.active_world = 0;
                }
                self.dashboard.set_active_world((self.active_world + 1) as u8);
                // ROM settings: parse the shuffle parameters then rebuild the
                // excluded-object set that hides vanilla / removed categories.
                self.rom_settings.parse_spoiler(&text, &self.mq_scenes);
                self.excluded = self.rom_settings.apply(&self.mq_scenes);
                self.rebuild_scene(); // MQ set may have changed which objects show
                self.prog_dirty = true; // spoiler / settings changed the dashboard
                self.counts_dirty = true;
                // Remember this spoiler so the next launch auto-applies its settings.
                let _ = std::fs::write(&self.spoiler_path_file, path.to_string_lossy().as_ref());
                self.status = if num_worlds > 1 {
                    let total: usize = self.worlds.iter().map(|w| w.items.len()).sum();
                    self.i18n.spoiler_multiworld(total, num_worlds)
                } else {
                    self.i18n.spoiler_singleworld(self.worlds[0].items.len(), self.mq_scenes.len())
                };
                // New seed: restore this seed's own progress from its autosave (the
                // collected set was cleared above), if it has been played before.
                if seed_changed {
                    let auto = self.autosave_path();
                    if auto.is_file() {
                        self.load_from(&auto);
                        self.prog_dirty = true;
                        self.counts_dirty = true;
                    }
                }
            }
            Err(e) => self.status = format!("Lecture spoiler échouée : {e}"),
        }
    }

    /// Sélectionne une scène : réinitialise la texture et la vue.
    pub(crate) fn select_scene(&mut self, game: Game, def: &'static data::SceneDef) {
        self.sel_scene[game.idx()] = Some(def.id);
        self.scene = Some(LiveScene::load(game, def, &self.mq_scenes));
        self.current_room = 0;
        self.map_texture = None;
        self.load_error = None;
        self.view_initialized = false;
    }

    /// Rebuild the current scene's objects (e.g. after the MQ set changed),
    /// keeping the loaded texture and view.
    pub(crate) fn rebuild_scene(&mut self) {
        if let Some(s) = &self.scene {
            self.scene = Some(LiveScene::load(s.game, s.def, &self.mq_scenes));
        }
    }

    /// Feed a live entrance message to the state machine. When an OUT / IN pair
    /// validates, record the discovered EntranceLink (out -> in and its reverse
    /// in-link), mark both endpoints visited and update the status bar.
    pub(crate) fn handle_entrance(&mut self, ev: &Event) {
        let parsed = self.ent_helper.parse(ev);
        // TEMP DIAG: surface the parser's per-message trace to the journal so we can
        // see exactly where an OoT transition is being dropped.
        let trace = std::mem::take(&mut self.ent_helper.trace);
        for line in trace {
            self.log_msg(line);
        }
        let Some(evt) = parsed else { return };

        let out = (evt.out_game, evt.out_entrance);
        let inc = (evt.in_game, evt.in_entrance);

        // OutLink: leaving `out` now leads to `inc`.
        let newly_discovered = self.out_links.insert(out, inc) != Some(inc);
        // Progressive-discovery reachability crosses only walked entrances, so a
        // freshly discovered one must invalidate the logic (`counts_dirty` in the
        // update loop then cascades to a recompute when the filter is on).
        if newly_discovered && self.app_settings.logic_progressive_entrances {
            self.logic_dirty = true;
        }
        // InLinks: `out` is a (distinct) source that leads to `inc`.
        let sources = self.in_links.entry(inc).or_default();
        if !sources.contains(&out) {
            sources.push(out);
        }

        self.visited_entrances.insert(out);
        self.visited_entrances.insert(inc);

        // Status bar + journal: the entrance we just arrived at.
        if let Some(d) = entrance::lookup(evt.in_game, evt.in_entrance) {
            self.last_entrance = Some(self.i18n.tr_entrance(d.to_name).to_string());
            // The player is now in this scene (drives auto-GPS + entrance-map
            // auto-follow). `d.to_scene` is the entrance meta's map node — for the
            // Market that is the generic combined scene, which carries no objects.
            self.player_scene = Some((evt.in_game, d.to_scene));
            // The object-map follow needs the scene that actually renders objects:
            // the generic Market resolves to its Day / Night variant (told apart by
            // the raw arriving scene); object-less zones resolve to `None`.
            self.player_obj_scene =
                resolve_obj_scene(evt.in_game, d.to_scene, evt.in_msg.scene, &self.mq_scenes);
            let from = entrance::lookup(evt.out_game, evt.out_entrance).map(|o| o.to_name).unwrap_or("?");
            let msg = self.i18n.entrance_detect(self.i18n.tr_entrance(from), self.i18n.tr_entrance(d.to_name));
            self.log_msg(msg);
        }
        // The full FROM/TO field table, mirroring the Qt `ParseIncomingMessage`.
        self.log_entrance_details(&evt);
        self.dirty = true;
    }

    /// Log the full FROM/TO field table for a validated transition, mirroring
    /// the Qt `ParseIncomingMessage` console output (on top of the one-line
    /// summary above). Rendered in the monospace journal so the columns align.
    fn log_entrance_details(&mut self, evt: &entrance::EntranceEvent) {
        let out = evt.out_msg;
        let inc = evt.in_msg;
        let name = |g: Game, id: u32| -> String {
            match entrance::lookup(g, id) {
                Some(d) => format!("{} - {}", d.to_name, d.from_name),
                None => "?".to_string(),
            }
        };
        let out_str = name(out.game, out.entrance_id);
        let in_str = name(inc.game, inc.entrance_id);
        let game_lbl = |g: Game| if g == Game::Oot { "OoT" } else { "MM" };

        // X/Y/Z are universal axis names; the coordinates line stays as-is.
        self.log_msg(format!("X = {:.6}, Y = {:.6}, Z = {:.6}", inc.x, inc.y, inc.z));
        self.log_msg(self.i18n.log_new_scene(&out_str, out.entrance_id, &in_str, inc.entrance_id));
        if entrance::lookup(out.game, out.entrance_id)
            .is_some_and(|m| m.type_ == data::EntranceType::One_Way_In)
        {
            self.log_msg(self.i18n.log_one_way_in(&out_str, out.entrance_id));
        }
        if entrance::lookup(inc.game, inc.entrance_id)
            .is_some_and(|m| m.type_ == data::EntranceType::One_Way_Out)
        {
            self.log_msg(self.i18n.log_one_way_out(&in_str, inc.entrance_id));
        }

        // Gather the localized table labels up front (owned), so the log loop below
        // never holds an `i18n` borrow across a `&mut self` `log_msg` call.
        let h_from = self.i18n.tbl_from().to_string();
        let h_to = self.i18n.tbl_to().to_string();
        let l_game = self.i18n.tbl_game().to_string();
        let l_scene = self.i18n.tbl_scene().to_string();
        let l_entrance = self.i18n.tbl_entrance().to_string();
        let l_room = self.i18n.tbl_room().to_string();
        let l_grotto = self.i18n.tbl_grotto().to_string();
        let l_age = self.i18n.tbl_age().to_string();
        let l_farore = self.i18n.tbl_farore().to_string();
        let l_owl = self.i18n.tbl_owl().to_string();
        let l_song = self.i18n.tbl_song().to_string();

        let sep = || "-".repeat(49);
        let hex08 = |v: u32| format!("0x{v:08X}");
        let hex02 = |v: u8| format!("0x{v:02X}");
        // Label column widened to 14 so the longer French labels still align.
        let row = |label: &str, from: String, to: String| {
            format!("{label:<14} | {from:>14} | {to:>14} |")
        };
        let header = format!("{:<14} | {h_from:^14} | {h_to:^14} |", "");
        let lines = [
            self.i_row_f("X", out.x, inc.x),
            self.i_row_f("Y", out.y, inc.y),
            self.i_row_f("Z", out.z, inc.z),
            row(&l_game, game_lbl(out.game).to_string(), game_lbl(inc.game).to_string()),
            row(&l_scene, hex08(evt.out_scene), hex08(evt.in_scene)),
            row(&l_entrance, hex08(evt.out_entrance), hex08(evt.in_entrance)),
            row(&l_room, hex02(out.curr_room), hex02(inc.curr_room)),
            row(&l_grotto, hex02(out.grotto_data), hex02(inc.grotto_data)),
            row(&l_age, hex02(out.age), hex02(inc.age)),
            row(&l_farore, hex02(out.farore_wind), hex02(inc.farore_wind)),
            row(&l_owl, hex02(out.owl_id), hex02(inc.owl_id)),
            row(&l_song, hex02(out.song), hex02(inc.song)),
        ];
        self.log_msg(sep());
        self.log_msg(header);
        for line in lines {
            self.log_msg(sep());
            self.log_msg(line);
        }
        self.log_msg(sep());
    }

    /// One float table row for `log_entrance_details`.
    fn i_row_f(&self, label: &str, from: f32, to: f32) -> String {
        format!("{label:<14} | {:>14.6} | {:>14.6} |", from, to)
    }

    /// Rebuild the InLinks index from OutLinks (each out -> in edge implies the
    /// reverse in <- out source). Used after loading a save.
    pub(crate) fn rebuild_in_links(&mut self) {
        self.in_links.clear();
        for (&out, &inc) in &self.out_links {
            self.in_links.entry(inc).or_default().push(out);
        }
    }

    /// (found, total) entrance count for one scene, faithful to the Qt
    /// `SceneEntranceItemTree::CountValidEntrances`. Entrances come from the
    /// curated per-scene list (not `to_scene`), and each is counted per direction:
    /// the in-side scales with its discovered sources (at least one slot to find),
    /// and Normal / One_Way_Out add one out-side slot found once its OutLink is set.
    pub(crate) fn entrance_scene_counts(&self, game: Game, scene: u16) -> (usize, usize) {
        let Some(ids) = entrance::scene_entrances(game, scene as u32) else { return (0, 0) };
        let (mut found, mut total) = (0usize, 0usize);
        for &eid in ids {
            let Some(e) = entrance::lookup(game, eid) else { continue };
            if e.type_ == data::EntranceType::None
                || !tracking::scene_layout_active(e.layout, game, scene, &self.mq_scenes)
            {
                continue;
            }
            let in_found = self.in_links.get(&(game, eid)).map_or(0, |v| v.len());
            let in_total = in_found.max(1);
            let out_set = self.out_links.contains_key(&(game, eid));
            match e.type_ {
                data::EntranceType::Normal => {
                    total += in_total + 1;
                    found += in_found + out_set as usize;
                }
                data::EntranceType::One_Way_In => {
                    total += in_total;
                    found += in_found;
                }
                data::EntranceType::One_Way_Out => {
                    total += 1;
                    found += out_set as usize;
                }
                data::EntranceType::None => {}
            }
        }
        (found, total)
    }

    /// (found, total) entrances of a game, summed over its curated scene list —
    /// the denominator the Qt entrance tab shows.
    pub(crate) fn entrance_counts(&self, game: Game) -> (usize, usize) {
        let scenes = match game {
            Game::Oot => data::OOT_SCENE_ENTRANCES,
            Game::Mm => data::MM_SCENE_ENTRANCES,
        };
        let (mut found, mut total) = (0usize, 0usize);
        for &(scene, _region, _ids) in scenes {
            let (f, t) = self.entrance_scene_counts(game, scene);
            found += f;
            total += t;
        }
        (found, total)
    }

    /// Refresh the current scene's markers from the active world's collected-set.
    pub(crate) fn sync_collected(&mut self) {
        let coll = &self.worlds[self.active_world].collected;
        if let Some(scene) = self.scene.as_mut() {
            let g = scene.game;
            for o in &mut scene.objects {
                o.collected = coll.contains(&(g, o.index));
            }
        }
    }

    /// The active object context of the current scene (None = no context = show
    /// all), driven by the age/season toggle (RefreshSceneContext).
    pub(crate) fn effective_context(&self) -> Option<data::ObjectContext> {
        let scene = self.scene.as_ref()?;
        if !scene.def.has_context {
            return None;
        }
        use data::ObjectContext as C;
        Some(match (scene.game, self.context_toggle) {
            (Game::Oot, false) => C::Child,
            (Game::Oot, true) => C::Adult,
            (Game::Mm, false) => C::Winter,
            (Game::Mm, true) => C::Spring,
        })
    }

    /// Recompute the cached per-game totals + per-render-scene counts in one pass
    /// per game (gated like the map: raw Type != none, active layout, not excluded).
    /// Feeds the tab-bar counters and the scene-tree counts.
    pub(crate) fn recompute_counts(&mut self) {
        let aw = self.active_world;
        // "Hide unreachable" mode: an uncollected unreachable check is not shown, so
        // it must not be counted either — the scene/category totals then reflect only
        // the reachable-or-collected checks, and empty scenes drop from the trees.
        let hide = self.app_settings.logic_filter_enabled
            && self.app_settings.logic_hide_unreachable;
        for game in [Game::Oot, Game::Mm] {
            let mut totals = (0usize, 0usize);
            let mut sc: HashMap<u16, (usize, usize)> = HashMap::new();
            for (i, o) in game.objects().iter().enumerate() {
                if o.type_ == data::ObjectType::none
                    || !tracking::object_active(o, game, &self.mq_scenes)
                    || self.excluded.contains(game, i)
                {
                    continue;
                }
                let got = self.worlds[aw].collected.contains(&(game, i));
                if hide && !got && self.obj_unreachable(o.location) {
                    continue; // hidden on the map → excluded from the counts
                }
                totals.1 += 1;
                let e = sc.entry(o.render_scene).or_default();
                e.1 += 1;
                if got {
                    totals.0 += 1;
                    e.0 += 1;
                }
            }
            self.cached_totals[game.idx()] = totals;
            self.cached_scene_counts[game.idx()] = sc;
        }
    }

    /// Recompute the shown player's reachability from their multiworld inventory +
    /// the seed settings. Called only when the accessibility filter is on and the
    /// state is dirty (see the update loop).
    pub(crate) fn recompute_reachability(&mut self) {
        let player = (self.active_world + 1) as u8; // 1-based player whose map is shown
        let progressive = self.app_settings.logic_progressive_entrances;
        // Progressive discovery only needs the set of walked entrances; skip the
        // allocation entirely when the mode is off (full spoiler knowledge).
        let discovered: std::collections::HashSet<(u8, u32)> = if progressive {
            self.out_links.keys().map(|&(g, id)| (g as u8, id)).collect()
        } else {
            std::collections::HashSet::new()
        };
        self.reach = Some(crate::logic::solve_world(
            &self.rom_settings,
            &self.worlds,
            player,
            &discovered,
            progressive,
        ));
    }

    /// Whether a check must be treated as *unreachable* by the accessibility
    /// filter, i.e. hidden/dimmed. Requires the filter on and a reachability
    /// result. Then:
    /// - a location that carries a logic rule is unreachable when the solver did
    ///   not reach it;
    /// - a location with no rule is normally kept fully shown (the logic has
    ///   nothing to say), EXCEPT once a spoiler is loaded: a location with no item
    ///   placed at it is not part of the randomizer pool at all (decorative
    ///   "… Unreachable" / "Out of Bounds" checks, entries the logic comments out),
    ///   so it is treated as unreachable rather than shown as always-reachable.
    pub(crate) fn obj_unreachable(&self, location: &str) -> bool {
        if !self.app_settings.logic_filter_enabled {
            return false;
        }
        let Some(r) = &self.reach else { return false };
        if self.logic_locs.contains(location) {
            return !r.reachable(location);
        }
        // No logic rule: hide only a confirmed non-pool location — a spoiler is
        // loaded (placements known) and it holds no item for this check.
        let cw = &self.worlds[self.active_world];
        !cw.items.is_empty() && !cw.items.contains_key(location)
    }

    /// Toggle an object by hand (map or tree click). A fresh click marks it Forced
    /// (gold); clicking a Forced object clears it. Auto-collected objects (found in
    /// game) can NOT be unchecked by the user — only manual marks are removable.
    pub(crate) fn toggle_object(&mut self, game: Game, index: usize) {
        let key = (game, index);
        let w = &mut self.worlds[self.active_world];
        if w.collected.contains(&key) {
            // Only clear if the mark was user-forced; keep auto-collected objects.
            if !w.forced.remove(&key) {
                return;
            }
            w.collected.remove(&key);
        } else {
            w.collected.insert(key);
            w.forced.insert(key);
        }
        self.dirty = true;
        self.prog_dirty = true;
        self.counts_dirty = true;
    }

    /// Preload the icon textures, a small budget per frame to avoid a startup
    /// hitch (type icons + every named per-object MapIcon).
    pub(crate) fn ensure_icons(&mut self, ctx: &egui::Context) {
        let paths = data::ICON_PATHS
            .iter()
            .copied()
            .chain(data::ICON_BY_NAME.iter().map(|&(_, p)| p))
            .chain(data::ENTRANCE_ICONS.iter().map(|&(_, p)| p))
            .chain(data::OOT_REGION_ICONS.iter().copied())
            .chain(data::MM_REGION_ICONS.iter().copied())
            .chain(CONTEXT_ICON_PATHS.iter().copied());
        let mut budget = 24;
        for path in paths {
            if path.is_empty() || self.icon_cache.contains_key(path) {
                continue;
            }
            let tex = load_color_image(&scene::resource_path(path))
                .ok()
                .map(|img| ctx.load_texture(path, img, egui::TextureOptions::LINEAR));
            self.icon_cache.insert(path, tex);
            budget -= 1;
            if budget == 0 {
                ctx.request_repaint(); // keep loading the rest next frame
                break;
            }
        }
    }

    /// Preload the greyscale ("uncollected") and blue-silhouette ("obtained glow")
    /// variants of the progression item icons, a small budget per frame like
    /// `ensure_icons`. Only the icons the progression grid can show (ICON_BY_NAME)
    /// get these copies — the map / region icons never need one.
    pub(crate) fn ensure_prog_grey_icons(&mut self, ctx: &egui::Context) {
        // The accent blue reused for the "obtained" glow silhouette.
        let glow = egui::Color32::from_rgb(74, 158, 219);
        let mut budget = 24;
        for &(_, path) in data::ICON_BY_NAME.iter() {
            if path.is_empty() || self.grey_icon_cache.contains_key(path) {
                continue;
            }
            let img = load_color_image(&scene::resource_path(path)).ok();
            let grey = img.as_ref().map(|img| {
                ctx.load_texture(format!("{path}#grey"), greyscale_image(img), egui::TextureOptions::LINEAR)
            });
            let sil = img.as_ref().map(|img| {
                ctx.load_texture(format!("{path}#glow"), silhouette_image(img, glow), egui::TextureOptions::LINEAR)
            });
            self.grey_icon_cache.insert(path, grey);
            self.glow_icon_cache.insert(path, sil);
            budget -= 1;
            if budget == 0 {
                ctx.request_repaint(); // keep loading the rest next frame
                break;
            }
        }
    }

    /// Charge l'image de la scène courante en texture (une seule fois).
    pub(crate) fn ensure_texture(&mut self, ctx: &egui::Context) {
        if self.map_texture.is_some() || self.load_error.is_some() {
            return;
        }
        let Some(scene) = &self.scene else { return };
        let entrance_view = self.active_tab.is_entrance();
        let Some(path) = scene.image_path(self.current_room, entrance_view, self.context_toggle) else {
            self.load_error = Some(self.i18n.no_img().to_owned());
            return;
        };
        match load_color_image(&path) {
            Ok(img) => {
                self.map_size = vec2(img.size[0] as f32, img.size[1] as f32);
                self.map_texture =
                    Some(ctx.load_texture("scene_map", img, egui::TextureOptions::LINEAR));
            }
            Err(e) => self.load_error = Some(e),
        }
    }

    /// The game currently in context: the OoT / MM tab, or the Entrance tab's
    /// active game sub-tab (None on Launch / Progression / Entrance-GPS).
    pub(crate) fn current_game(&self) -> Option<Game> {
        match self.active_tab {
            Tab::Oot => Some(Game::Oot),
            Tab::Mm => Some(Game::Mm),
            Tab::Entrance => self.entrance_sub.game(),
            _ => None,
        }
    }

    /// Apply the auto-follow / auto-GPS-start / auto-snap options after polling
    /// live events. Auto-follow moves the displayed scene to the player's location
    /// on the matching tab (only when the player just changed scene, so it never
    /// fights manual navigation); auto-snap jumps to the last collected object.
    fn apply_auto_options(&mut self) {
        // Player just moved: follow on the matching tab + seed the GPS start.
        if let Some(ps) = self.player_scene {
            if self.followed_scene != Some(ps) {
                let (g, sid) = ps;
                match self.active_tab {
                    // Item map: follow only to a scene that actually holds tracked
                    // objects. `player_obj_scene` already resolved that (the Market's
                    // real Day / Night map), and is `None` for object-less zones
                    // (Market Entrance, Back Alley…), which then keep the current map.
                    Tab::Oot | Tab::Mm if self.app_settings.auto_follow_item => {
                        if let Some((fg, fsid)) = self.player_obj_scene {
                            if self.active_tab.game() == Some(fg) {
                                if let Some(def) = fg.scenes().iter().find(|s| s.id == fsid) {
                                    self.select_scene(fg, def);
                                }
                            }
                        }
                    }
                    // Entrance map: entrances render on the generic scene's minimap,
                    // so follow it verbatim (object-less zones still carry entrances).
                    Tab::Entrance if self.app_settings.auto_follow_entrance => {
                        if self.entrance_sub.game() == Some(g) {
                            if let Some(def) = g.scenes().iter().find(|s| s.id == sid) {
                                self.select_scene(g, def);
                            }
                        }
                    }
                    _ => {}
                }
                if self.app_settings.auto_gps_start {
                    self.gps_from = Some(ps);
                    self.gps_from_ent = None;
                }
            }
            self.followed_scene = Some(ps);
        }
        // Auto Snap View: jump to the last collected object's scene and centre on it.
        if let Some((g, sid, room, x, y)) = self.pending_snap.take() {
            if let Some(def) = g.scenes().iter().find(|s| s.id == sid) {
                self.active_tab = if g == Game::Oot { Tab::Oot } else { Tab::Mm };
                if self.scene.as_ref().map_or(true, |s| s.game != g || s.def.id != sid) {
                    self.select_scene(g, def);
                }
                // Multi-room scene: load the room that actually holds the object
                // before zooming, so the view (and its object coordinates, which are
                // relative to the room image) snap onto the right map.
                if let Some(k) = g.rooms(sid).iter().position(|r| r.id == room as u32) {
                    if self.current_room != k {
                        self.current_room = k;
                        self.map_texture = None; // different room image
                    }
                }
                self.snap_pos = Some((x, y));
                self.view_initialized = false;
            }
        }
    }

    /// Move `self.scene` to the context game if needed (restoring its last
    /// selected scene). Launch / Progression / Entrance-GPS keep the current one.
    pub(crate) fn ensure_tab_scene(&mut self) {
        let Some(game) = self.current_game() else { return };
        // Keep the scene if it already belongs to this tab's game.
        if self.scene.as_ref().is_some_and(|s| s.game == game) {
            return;
        }
        // Otherwise restore this game's remembered scene — or clear the view when
        // the user hasn't opened one yet (startup / a freshly switched-to game).
        match self.sel_scene[game.idx()].and_then(|id| game.scenes().iter().find(|s| s.id == id)) {
            Some(def) => {
                self.scene = Some(LiveScene::load(game, def, &self.mq_scenes));
                self.current_room = 0;
                self.map_texture = None;
                self.load_error = None;
                self.view_initialized = false;
            }
            None => {
                if self.scene.is_some() {
                    self.scene = None;
                    self.map_texture = None;
                    self.load_error = None;
                    self.view_initialized = false;
                }
            }
        }
    }

    pub(crate) fn set_language(&mut self, language: Language) {
        if self.i18n.language() == language {
            return;
        }

        self.i18n.set_language(language);

        // Refresh the poller thread's log / status snapshot so its journal follows
        // the new language too (it reads this on its next iteration).
        if let Ok(mut guard) = self.log_strings.lock() {
            *guard = std::sync::Arc::new(self.i18n.log_strings());
        }

        // Persist the choice so it is restored on the next launch.
        self.app_settings.language = language;
        self.app_settings.save(&self.app_settings_path);
    }
}

impl eframe::App for TrackerApp {
    fn update(&mut self, ctx: &egui::Context, _frame: &mut eframe::Frame) {
        // Frame-rate cap. egui is immediate-mode: every mouse-move event would
        // otherwise re-tessellate the whole UI as fast as the OS delivers moves,
        // pinning a core (~100%). Sleeping out the rest of a ~60 fps budget bounds
        // that to display-rate frames; the thread sleeps (no spin), so idle stays
        // ~0 and interaction stays smooth. Retained-mode Qt gets this for free.
        let budget = Duration::from_millis(16);
        let elapsed = self.last_frame.elapsed();
        if elapsed < budget {
            std::thread::sleep(budget - elapsed);
        }
        self.last_frame = Instant::now();

        // First frame: resolve a patch path restored from the save (load it, or
        // warn + prompt if the file moved). Deferred here so the main window
        // exists before any dialog appears.
        if self.patch_startup_check {
            self.patch_startup_check = false;
            self.resolve_startup_patch();
        }

        // Drain the poller thread (connection status + live events).
        self.process_poll();
        self.apply_auto_options();

        // Load a spoiler log dropped onto the window.
        let dropped = ctx.input(|i| i.raw.dropped_files.iter().find_map(|f| f.path.clone()));
        if let Some(path) = dropped {
            self.load_spoiler(&path);
        }

        self.ensure_icons(ctx);
        self.ensure_tab_scene();
        self.sync_collected();
        self.ensure_texture(ctx);
        // A tracked-state change (collected / exclusions / layout / world) also
        // invalidates reachability. Reachability must refresh BEFORE the counts,
        // because in "hide" mode the counts (and the trees) drop unreachable checks.
        if self.counts_dirty {
            self.logic_dirty = true;
        }
        // Reachability: only computed when the filter is on and something changed
        // (never per mouse-move frame — the fixed point over ~1300 regions is far
        // heavier than the count scan).
        if self.app_settings.logic_filter_enabled && self.logic_dirty {
            self.recompute_reachability();
            self.logic_dirty = false;
            self.counts_dirty = true; // hide-mode counts depend on the new reachability
        }
        if self.counts_dirty {
            self.recompute_counts();
            self.counts_dirty = false;
        }

        self.draw_menu_bar(ctx);
        self.draw_tab_bar(ctx);
        self.draw_status_bar(ctx);
        self.draw_settings_window(ctx);

        match self.active_tab {
            Tab::Launch => self.draw_launch(ctx),
            Tab::Oot | Tab::Mm => {
                self.draw_nav(ctx);
                self.draw_object_tree(ctx);
                self.draw_map(ctx);
            }
            Tab::Entrance => {
                // Qt EntranceTab: OoT / MM / GPS sub-tabs.
                self.draw_entrance_subtabs(ctx);
                match self.entrance_sub {
                    EntranceSub::Gps => self.draw_gps_page(ctx),
                    sub => {
                        let game = sub.game().unwrap_or(Game::Oot);
                        self.draw_nav(ctx);
                        // Region selected -> full-width entrance table (no right
                        // tree); scene selected -> entrance tree + minimap.
                        match self.entrance_table {
                            Some((g, region)) if g == game => {
                                self.draw_entrance_table(ctx, g, region)
                            }
                            _ => {
                                self.draw_entrance_tree(ctx);
                                self.draw_map(ctx);
                            }
                        }
                    }
                }
            }
            Tab::Progression => self.draw_progression_tab(ctx),
        }

        // Flush the autosave right after a change (frames are sparse now), but only
        // when auto-save is enabled (the menu / status-bar toggle).
        if self.dirty {
            if self.auto_save {
                self.save_state();
            }
            self.dirty = false;
        }
        // No idle timer: the poller thread wakes us on real events, egui wakes on
        // input, so the UI otherwise sleeps (idle ~0, like the Qt build).
    }

    /// On close, unload the tracking DLL cleanly (mirror `StartMemoryReader`
    /// steps 10–12): the poller writes Command = Shutdown so the DLL restores
    /// its patches and unloads, then removes the plugin file. We block briefly
    /// so Project64 is left clean even though it keeps running.
    fn on_exit(&mut self, _gl: Option<&eframe::glow::Context>) {
        self.stop_multiplayer();
        self.stop_r4();
        self.poller.shutdown_and_wait();
    }
}

/// The OoT Market's generic map node (`OOT_MARKET`) carries no objects: entrances
/// render on its combined minimap, while the collectibles live on the Day / Night
/// variants, told apart only by the raw runtime scene in the arriving message. Map
/// the generic node + raw scene to the real object scene, or `None` for anything
/// else (including the adult Market, which has no tracked objects).
fn market_object_scene(game: Game, generic: u16, raw: u16) -> Option<u16> {
    use crate::data::scenes as sc;
    if game == Game::Oot && generic == sc::OOT_MARKET {
        match raw {
            sc::OOT_MARKET_CHILD_DAY | sc::OOT_MARKET_CHILD_NIGHT => Some(raw),
            _ => None,
        }
    } else {
        None
    }
}

/// The object-rendering scene to auto-follow for the player's current location.
/// `generic` is the entrance meta's scene; `raw` is the arriving message's own
/// scene. Resolves the Market's Day / Night special case first, then follows the
/// generic scene only when it actually holds tracked objects — so object-less
/// zones (Market Entrance, Back Alley, plain interiors) yield `None` (no follow).
fn resolve_obj_scene(
    game: Game,
    generic: u16,
    raw: u32,
    mq: &HashSet<(Game, u16)>,
) -> Option<(Game, u16)> {
    if let Some(obj) = market_object_scene(game, generic, raw as u16) {
        return Some((game, obj));
    }
    game.scene_has_objects(generic, mq).then_some((game, generic))
}

/// Root directory for the r4 client's per-session data (WAL + send queue). Mirrors
/// the Go client's `%APPDATA%/OoTMM/client`, but under a tracker-specific folder
/// so it never clashes with a standalone client's data.
fn r4_data_dir() -> std::path::PathBuf {
    if let Ok(appdata) = std::env::var("APPDATA") {
        std::path::PathBuf::from(appdata).join("OoTMM").join("tracker-client")
    } else {
        std::path::PathBuf::from("data").join("tracker-client")
    }
}

/// The autosave file stem for a spoiler: its seed hash (the `Seed:` first line),
/// reduced to filename-safe characters and capped so the name stays tidy. Returns
/// `empty` when the text carries no seed, giving the no-spoiler run a stable file.
fn seed_tag_from_spoiler(text: &str) -> String {
    // The seed is the first non-empty line ("Seed: <hash>"); nothing else counts.
    for line in text.lines() {
        let line = line.trim();
        if line.is_empty() {
            continue;
        }
        if let Some(rest) = line.strip_prefix("Seed:") {
            let tag: String =
                rest.chars().filter(|c| c.is_ascii_alphanumeric()).take(32).collect();
            if !tag.is_empty() {
                return tag;
            }
        }
        break;
    }
    "empty".to_string()
}

// ── XML save format (version 6) ──────────────────────────────────────────────
// Pure (self-free) serializer/parser so a round-trip can be unit-tested without
// constructing a full `TrackerApp`. `TrackerApp::save_to` / `load_from_xml` are
// thin adapters over these.

/// The tracked data recovered from a version-6 XML save. `worlds` reuses
/// [`crate::WorldData`] (its `collected` / `forced` / `items` / `dest` are exactly
/// the per-world fields we persist); the entrance graph and remembered paths are
/// returned separately for the caller to merge into the live state.
struct ParsedSave {
    worlds: Vec<crate::WorldData>,
    visited: HashSet<(Game, u32)>,
    out_links: HashMap<(Game, u32), (Game, u32)>,
    in_links: HashMap<(Game, u32), Vec<(Game, u32)>>,
    patch_path: Option<PathBuf>,
    spoiler_path: Option<String>,
}

/// Serialize the tracked state to the human-readable / hand-editable XML save.
/// Output is deterministic (scenes, locations and entrances are sorted) so saves
/// diff cleanly. The `loc` attribute — the globally-unique object `Location` — is
/// the authoritative key for a `<location>`; a placed `<item>` additionally carries
/// its stable `id` (preferred over the name on load). Entrance `name`s are the
/// fully-qualified "scene - side" form and, like every other `name`, are readable
/// annotations free to hand-edit (the entrance graph reloads from the `id`s).
fn render_save_xml(
    worlds: &[crate::WorldData],
    patch: Option<&str>,
    spoiler: Option<&str>,
    visited: &HashSet<(Game, u32)>,
    out_links: &HashMap<(Game, u32), (Game, u32)>,
    in_links: &HashMap<(Game, u32), Vec<(Game, u32)>>,
) -> String {
    use std::fmt::Write as _;
    // Minimal XML escaping for a double-quoted attribute: only `&`, `<` and `"`
    // must be escaped there. Leaving `'` and `>` literal keeps the save readable —
    // "Mido's House", and the spawn arrow "scene -> side" instead of "-&gt;". `&`
    // is replaced first so we don't double-escape the entities we introduce.
    let esc = |s: &str| {
        s.replace('&', "&amp;").replace('<', "&lt;").replace('"', "&quot;")
    };
    let tag = |g: Game| if g == Game::Mm { "MM" } else { "OoT" };
    let scene_name = |g: Game, id: u16| -> String {
        g.scenes()
            .iter()
            .find(|s| s.id == id)
            .map(|s| s.name.to_string())
            .unwrap_or_else(|| format!("Scene {id:#x}"))
    };
    // The entrance's identity / where it leads: the dash "scene - side" form, used
    // for the <entrance> itself and for its <out> link ("Kokiri Forest - Deku Tree",
    // not just "Kokiri Forest").
    let ent_name = |g: Game, id: u32| -> String {
        entrance::display_name(g, id).unwrap_or_default()
    };
    // Where an entrance is entered FROM: the arrow "scene -> side" form (tracker
    // GetEntranceSpawnsString), so an <in> link reads differently from an <out>.
    let spawn_name = |g: Game, id: u32| -> String {
        entrance::spawns_name(g, id).unwrap_or_default()
    };

    // location string -> (game, object index): lets an item/dest-only row (a
    // placement whose object was never collected) still carry a scene + name.
    let mut by_loc: HashMap<&str, (Game, usize)> = HashMap::new();
    for game in [Game::Oot, Game::Mm] {
        for (i, o) in game.objects().iter().enumerate() {
            by_loc.insert(o.location, (game, i));
        }
    }

    let mut out = String::new();
    out.push_str("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    writeln!(out, "<tracker version=\"{}\">", crate::SAVE_VERSION).ok();

    // <paths>: the remembered spoiler (sidecar) + the r4 patch file.
    out.push_str("  <paths>\n");
    if let Some(sp) = spoiler {
        writeln!(out, "    <spoiler>{}</spoiler>", esc(sp)).ok();
    }
    if let Some(p) = patch {
        writeln!(out, "    <patch>{}</patch>", esc(p)).ok();
    }
    out.push_str("  </paths>\n");

    // <worlds>: one <world index="N"> (1-based) per world, its touched locations
    // grouped by scene. A location is written when it is collected, or carries an
    // item placement / destination player.
    struct Row<'a> {
        game: Game,
        scene: u16,
        name: &'a str,
        loc: &'a str,
        state: &'static str,
        item: Option<&'a str>,
        dest: Option<u8>,
        // Stable numeric identity, disambiguated within the parent <scene>: the
        // object id, then its type + layout to split the (few) checks that share a
        // scene + object id (MQ variants, co-located shops, overlapping id spaces).
        // `None` only when the location matches no object (an item on an unknown loc).
        oid: Option<u32>,
        type_id: Option<u32>,
        layout_id: Option<u32>,
    }
    // The numeric-key attributes for a <location>, in a stable order.
    let key_attrs = |r: &Row| -> String {
        let mut s = String::new();
        if let Some(o) = r.oid { let _ = write!(s, " oid=\"{o:#x}\""); }
        if let Some(t) = r.type_id { let _ = write!(s, " type=\"{t}\""); }
        if let Some(l) = r.layout_id { let _ = write!(s, " layout=\"{l}\""); }
        s
    };
    out.push_str("  <worlds>\n");
    for (wi, w) in worlds.iter().enumerate() {
        writeln!(out, "    <world index=\"{}\">", wi + 1).ok();

        let mut rows: HashMap<&str, Row> = HashMap::new();
        for &(g, idx) in &w.collected {
            let o = &g.objects()[idx];
            let state = if w.forced.contains(&(g, idx)) { "forced" } else { "collected" };
            rows.insert(
                o.location,
                Row {
                    game: g, scene: o.scene, name: o.name, loc: o.location, state,
                    item: None, dest: None,
                    oid: Some(o.object_id), type_id: Some(o.type_ as u32), layout_id: Some(o.layout as u32),
                },
            );
        }
        // A placement / destination on a location that was never collected still
        // gets a row; its scene + name + numeric keys come from the object index.
        for (loc, item) in &w.items {
            rows.entry(loc.as_str())
                .or_insert_with(|| {
                    by_loc
                        .get(loc.as_str())
                        .map(|&(g, i)| {
                            let o = &g.objects()[i];
                            Row {
                                game: g, scene: o.scene, name: o.name, loc: loc.as_str(), state: "none",
                                item: None, dest: None,
                                oid: Some(o.object_id), type_id: Some(o.type_ as u32), layout_id: Some(o.layout as u32),
                            }
                        })
                        .unwrap_or(Row {
                            game: Game::Oot, scene: 0, name: "", loc: loc.as_str(), state: "none",
                            item: None, dest: None, oid: None, type_id: None, layout_id: None,
                        })
                })
                .item = Some(item.as_str());
        }
        for (loc, d) in &w.dest {
            rows.entry(loc.as_str())
                .or_insert_with(|| {
                    by_loc
                        .get(loc.as_str())
                        .map(|&(g, i)| {
                            let o = &g.objects()[i];
                            Row {
                                game: g, scene: o.scene, name: o.name, loc: loc.as_str(), state: "none",
                                item: None, dest: None,
                                oid: Some(o.object_id), type_id: Some(o.type_ as u32), layout_id: Some(o.layout as u32),
                            }
                        })
                        .unwrap_or(Row {
                            game: Game::Oot, scene: 0, name: "", loc: loc.as_str(), state: "none",
                            item: None, dest: None, oid: None, type_id: None, layout_id: None,
                        })
                })
                .dest = Some(*d);
        }

        let mut list: Vec<Row> = rows.into_values().collect();
        list.sort_by(|a, b| (a.game.idx(), a.scene, a.loc).cmp(&(b.game.idx(), b.scene, b.loc)));

        out.push_str("      <scenes>\n");
        let mut cur: Option<(usize, u16)> = None;
        for r in &list {
            let key = (r.game.idx(), r.scene);
            if cur != Some(key) {
                if cur.is_some() {
                    out.push_str("        </scene>\n");
                }
                writeln!(
                    out,
                    "        <scene game=\"{}\" id=\"{:#x}\" name=\"{}\">",
                    tag(r.game), r.scene, esc(&scene_name(r.game, r.scene))
                ).ok();
                cur = Some(key);
            }
            let keys = key_attrs(r);
            if r.item.is_none() && r.dest.is_none() {
                writeln!(
                    out,
                    "          <location loc=\"{}\" name=\"{}\"{keys} state=\"{}\"/>",
                    esc(r.loc), esc(r.name), r.state
                ).ok();
            } else {
                writeln!(
                    out,
                    "          <location loc=\"{}\" name=\"{}\"{keys} state=\"{}\">",
                    esc(r.loc), esc(r.name), r.state
                ).ok();
                let mut attrs = String::new();
                if let Some(it) = r.item {
                    // The item id is stable and unique; write it as the authoritative
                    // key (immune to item renames). Names that don't resolve to an id
                    // (multiworld "Player N's …", unknowns) keep only their name.
                    if let Some(id) = crate::progression::find_item_id(it) {
                        write!(attrs, " id=\"{id:#x}\"").ok();
                    }
                    write!(attrs, " name=\"{}\"", esc(it)).ok();
                }
                if let Some(d) = r.dest {
                    write!(attrs, " dest=\"{d}\"").ok();
                }
                writeln!(out, "            <item{attrs}/>").ok();
                out.push_str("          </location>\n");
            }
        }
        if cur.is_some() {
            out.push_str("        </scene>\n");
        }
        out.push_str("      </scenes>\n");
        out.push_str("    </world>\n");
    }
    out.push_str("  </worlds>\n");

    // <entrances>: every entrance that is visited or carries a discovered link,
    // grouped by its (departure) scene. `visited` is a flag; `out` is the single
    // discovered destination, `in` the discovered sources.
    struct Ent {
        game: Game,
        id: u32,
        scene: Option<u16>,
        name: String,
        visited: bool,
        out: Option<(Game, u32)>,
        ins: Vec<(Game, u32)>,
    }
    let mut keys: HashSet<(Game, u32)> = HashSet::new();
    keys.extend(visited.iter().copied());
    keys.extend(out_links.keys().copied());
    keys.extend(in_links.keys().copied());
    let mut ents: Vec<Ent> = keys
        .into_iter()
        .map(|(g, id)| Ent {
            game: g,
            id,
            scene: entrance::lookup(g, id).map(|e| e.from_scene),
            name: ent_name(g, id),
            visited: visited.contains(&(g, id)),
            out: out_links.get(&(g, id)).copied(),
            ins: in_links.get(&(g, id)).cloned().unwrap_or_default(),
        })
        .collect();
    ents.sort_by_key(|e| (e.game.idx(), e.scene.unwrap_or(u16::MAX), e.id));

    out.push_str("  <entrances>\n");
    let mut cur: Option<(usize, u16)> = None;
    for e in &ents {
        let s = e.scene.unwrap_or(u16::MAX);
        let key = (e.game.idx(), s);
        if cur != Some(key) {
            if cur.is_some() {
                out.push_str("    </scene>\n");
            }
            let nm = e.scene.map(|id| scene_name(e.game, id)).unwrap_or_else(|| "Other".to_string());
            writeln!(out, "    <scene game=\"{}\" id=\"{:#x}\" name=\"{}\">", tag(e.game), s, esc(&nm)).ok();
            cur = Some(key);
        }
        if e.out.is_none() && e.ins.is_empty() {
            writeln!(
                out,
                "      <entrance game=\"{}\" id=\"{:#x}\" name=\"{}\" visited=\"{}\"/>",
                tag(e.game), e.id, esc(&e.name), e.visited
            ).ok();
        } else {
            writeln!(
                out,
                "      <entrance game=\"{}\" id=\"{:#x}\" name=\"{}\" visited=\"{}\">",
                tag(e.game), e.id, esc(&e.name), e.visited
            ).ok();
            if let Some((og, oid)) = e.out {
                writeln!(
                    out,
                    "        <out game=\"{}\" id=\"{:#x}\" name=\"{}\"/>",
                    tag(og), oid, esc(&ent_name(og, oid))
                ).ok();
            }
            for &(ig, iid) in &e.ins {
                writeln!(
                    out,
                    "        <in game=\"{}\" id=\"{:#x}\" name=\"{}\"/>",
                    tag(ig), iid, esc(&spawn_name(ig, iid))
                ).ok();
            }
            out.push_str("      </entrance>\n");
        }
    }
    if cur.is_some() {
        out.push_str("    </scene>\n");
    }
    out.push_str("  </entrances>\n");

    out.push_str("</tracker>\n");
    out
}

/// Parse a version-6 XML save (see [`render_save_xml`]). A `<location>` is matched
/// by its stable numeric identity — the object id within the parent `<scene>`, then
/// its type + layout to split the checks that share a scene + object id — and only
/// falls back to the `loc` string when the numeric keys are absent (older save),
/// unresolved (the object id changed), or still ambiguous. The entrance graph is
/// restored directly from the entrance `id` attributes. Malformed XML stops the
/// parse and keeps whatever was read so far.
fn parse_save_xml(text: &str) -> ParsedSave {
    use quick_xml::events::{BytesStart, Event};

    let mut out = ParsedSave {
        worlds: vec![crate::WorldData::default()],
        visited: HashSet::new(),
        out_links: HashMap::new(),
        in_links: HashMap::new(),
        patch_path: None,
        spoiler_path: None,
    };
    // Location -> (game, object index): the last-resort match when the numeric keys
    // can't resolve (an unknown id, or a genuine full-footprint tie).
    let mut by_loc: HashMap<&'static str, (Game, usize)> = HashMap::new();
    // (game, scene, object_id) -> candidate indices. Not unique on its own (MQ
    // variants, co-located scenes, overlapping id spaces), hence the type/layout
    // narrowing + loc tiebreak below.
    let mut by_scene_oid: HashMap<(u8, u16, u32), Vec<usize>> = HashMap::new();
    for game in [Game::Oot, Game::Mm] {
        for (i, o) in game.objects().iter().enumerate() {
            by_loc.insert(o.location, (game, i));
            by_scene_oid.entry((game.idx() as u8, o.scene, o.object_id)).or_default().push(i);
        }
    }

    fn attr_map(e: &BytesStart) -> HashMap<String, String> {
        let mut m = HashMap::new();
        for a in e.attributes().flatten() {
            if let Ok(v) = a.unescape_value() {
                m.insert(String::from_utf8_lossy(a.key.as_ref()).into_owned(), v.into_owned());
            }
        }
        m
    }
    fn parse_game(s: Option<&String>) -> Game {
        if s.is_some_and(|s| s.eq_ignore_ascii_case("MM")) { Game::Mm } else { Game::Oot }
    }
    fn parse_id(s: Option<&String>) -> Option<u32> {
        let t = s?.trim();
        match t.strip_prefix("0x").or_else(|| t.strip_prefix("0X")) {
            Some(h) => u32::from_str_radix(h, 16).ok(),
            None => t.parse::<u32>().ok(),
        }
    }

    let mut reader = quick_xml::Reader::from_str(text);
    let mut buf = Vec::new();
    let mut cur_world = 0usize;
    let mut cur_scene: Option<(Game, u16)> = None; // active <scene game id> (for oid keys)
    let mut cur_loc: Option<String> = None; // active <location loc="…">
    let mut cur_ent: Option<(Game, u32)> = None; // active <entrance>
    let mut path_tag = 0u8; // 1 = inside <spoiler>, 2 = inside <patch>

    loop {
        match reader.read_event_into(&mut buf) {
            Ok(Event::Eof) | Err(_) => break,
            Ok(Event::Start(e)) | Ok(Event::Empty(e)) => match e.name().as_ref() {
                b"world" => {
                    let n = attr_map(&e).get("index").and_then(|s| s.parse::<usize>().ok()).unwrap_or(1);
                    cur_world = n.saturating_sub(1);
                    if out.worlds.len() <= cur_world {
                        out.worlds.resize_with(cur_world + 1, crate::WorldData::default);
                    }
                }
                b"scene" => {
                    let a = attr_map(&e);
                    cur_scene = parse_id(a.get("id")).map(|id| (parse_game(a.get("game")), id as u16));
                }
                b"location" => {
                    let a = attr_map(&e);
                    // Candidate objects for (parent scene, object id); empty when the
                    // numeric keys are absent (older save) or the id is unknown.
                    let g = cur_scene.map(|(g, _)| g).unwrap_or(Game::Oot);
                    let mut cands: Vec<usize> =
                        match (cur_scene, a.get("oid").and_then(|s| parse_id(Some(s)))) {
                            (Some((_, sc)), Some(oid)) => {
                                by_scene_oid.get(&(g.idx() as u8, sc, oid)).cloned().unwrap_or_default()
                            }
                            _ => Vec::new(),
                        };
                    // Narrow the (rare) multi-candidate case by type, then layout.
                    if cands.len() > 1 {
                        if let Some(t) = a.get("type").and_then(|s| parse_id(Some(s))) {
                            let f: Vec<usize> =
                                cands.iter().copied().filter(|&i| g.objects()[i].type_ as u32 == t).collect();
                            if !f.is_empty() {
                                cands = f;
                            }
                        }
                    }
                    if cands.len() > 1 {
                        if let Some(l) = a.get("layout").and_then(|s| parse_id(Some(s))) {
                            let f: Vec<usize> =
                                cands.iter().copied().filter(|&i| g.objects()[i].layout as u32 == l).collect();
                            if !f.is_empty() {
                                cands = f;
                            }
                        }
                    }
                    let loc = a.get("loc");
                    // Unique numeric match wins; a genuine full-footprint tie is split
                    // by the loc string; an unresolved id falls back to loc entirely.
                    let resolved: Option<(Game, usize)> = match cands.len() {
                        1 => Some((g, cands[0])),
                        0 => loc.and_then(|l| by_loc.get(l.as_str()).copied()),
                        _ => loc
                            .and_then(|l| {
                                cands.iter().copied().find(|&i| g.objects()[i].location == l.as_str()).map(|i| (g, i))
                            })
                            .or(Some((g, cands[0]))),
                    };
                    if let Some((g, i)) = resolved {
                        let state = a.get("state").map(String::as_str).unwrap_or("none");
                        if state == "collected" || state == "forced" {
                            out.worlds[cur_world].collected.insert((g, i));
                            if state == "forced" {
                                out.worlds[cur_world].forced.insert((g, i));
                            }
                        }
                        // Key any child <item>/dest on the object's CURRENT location, so
                        // a renamed location still lands on the right object.
                        cur_loc = Some(g.objects()[i].location.to_string());
                    } else {
                        // Unresolved: keep the raw loc so a child <item>/dest round-trips.
                        cur_loc = loc.cloned();
                    }
                }
                b"item" => {
                    if let Some(loc) = cur_loc.clone() {
                        let a = attr_map(&e);
                        // Prefer the stable item id (resolved to its current canonical
                        // name); fall back to the saved name for ids that no longer
                        // exist or items that never had one.
                        let name = a
                            .get("id")
                            .and_then(|s| parse_id(Some(s)))
                            .and_then(|id| qtsave::item_name(id).map(str::to_string))
                            .or_else(|| a.get("name").filter(|s| !s.is_empty()).cloned());
                        if let Some(name) = name {
                            out.worlds[cur_world].items.insert(loc.clone(), name);
                        }
                        if let Some(d) = a.get("dest").and_then(|s| s.parse::<u8>().ok()) {
                            out.worlds[cur_world].dest.insert(loc, d);
                        }
                    }
                }
                b"entrance" => {
                    let a = attr_map(&e);
                    let g = parse_game(a.get("game"));
                    if let Some(id) = parse_id(a.get("id")) {
                        if a.get("visited").map(String::as_str) == Some("true") {
                            out.visited.insert((g, id));
                        }
                        cur_ent = Some((g, id));
                    }
                }
                b"out" => {
                    if let Some(key) = cur_ent {
                        let a = attr_map(&e);
                        if let Some(id) = parse_id(a.get("id")) {
                            out.out_links.insert(key, (parse_game(a.get("game")), id));
                        }
                    }
                }
                b"in" => {
                    if let Some(key) = cur_ent {
                        let a = attr_map(&e);
                        if let Some(id) = parse_id(a.get("id")) {
                            let src = (parse_game(a.get("game")), id);
                            let list = out.in_links.entry(key).or_default();
                            if !list.contains(&src) {
                                list.push(src);
                            }
                        }
                    }
                }
                b"spoiler" => path_tag = 1,
                b"patch" => path_tag = 2,
                _ => {}
            },
            Ok(Event::Text(t)) => {
                if path_tag != 0 {
                    if let Ok(s) = t.unescape() {
                        let s = s.trim();
                        if !s.is_empty() {
                            if path_tag == 1 {
                                out.spoiler_path = Some(s.to_string());
                            } else {
                                out.patch_path = Some(PathBuf::from(s));
                            }
                        }
                    }
                }
            }
            Ok(Event::End(e)) => match e.name().as_ref() {
                b"location" => cur_loc = None,
                b"entrance" => cur_ent = None,
                b"spoiler" | b"patch" => path_tag = 0,
                _ => {}
            },
            _ => {}
        }
        buf.clear();
    }
    out
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn seed_tag_extraction() {
        // The seed hash drives the per-seed autosave filename.
        let spoiler = "Seed: 18316056476e293e4cc59cfd\nVersion: dev-6d50571\n";
        assert_eq!(seed_tag_from_spoiler(spoiler), "18316056476e293e4cc59cfd");
        // Capped at 32 chars, non-alphanumerics dropped.
        let long = "Seed: abcd-efgh_0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ\n";
        let tag = seed_tag_from_spoiler(long);
        assert_eq!(tag.len(), 32);
        assert!(tag.chars().all(|c| c.is_ascii_alphanumeric()));
        // No seed line (or empty text) -> the shared no-spoiler file.
        assert_eq!(seed_tag_from_spoiler("Version: dev\nSettings\n"), "empty");
        assert_eq!(seed_tag_from_spoiler(""), "empty");
    }

    #[test]
    fn xml_save_round_trips() {
        // World 0: two collected objects (one forced), a placement + destination on
        // a collected location, and an item-only placement (never collected) whose
        // name exercises XML escaping. World 1: a lone MM collect.
        let loc0 = Game::Oot.objects()[0].location.to_string();
        let loc5 = Game::Oot.objects()[5].location.to_string();
        let mut w0 = crate::WorldData::default();
        w0.collected.insert((Game::Oot, 0));
        w0.collected.insert((Game::Oot, 1));
        w0.forced.insert((Game::Oot, 1));
        w0.items.insert(loc0.clone(), "Kokiri Sword".to_string());
        w0.dest.insert(loc0.clone(), 2);
        w0.items.insert(loc5.clone(), "Rupee & <Stick>".to_string());
        let mut w1 = crate::WorldData::default();
        w1.collected.insert((Game::Mm, 3));
        let worlds = vec![w0, w1];

        // Entrances: a visited flag, a discovered out-link and its mirror in-link.
        let a = (Game::Oot, Game::Oot.entrances()[0].to_id);
        let b = (Game::Mm, Game::Mm.entrances()[0].to_id);
        let mut visited = HashSet::new();
        visited.insert(a);
        visited.insert(b);
        let mut out_links = HashMap::new();
        out_links.insert(a, b);
        let mut in_links = HashMap::new();
        in_links.insert(b, vec![a]);

        let xml = render_save_xml(
            &worlds,
            Some("C:/patch.ootmm"),
            Some("C:/spoiler.txt"),
            &visited,
            &out_links,
            &in_links,
        );
        // Sanity: it is the version-6 XML the loader routes on.
        assert!(xml.starts_with("<?xml"));
        assert!(xml.contains("<tracker version=\"6\">"));

        let p = parse_save_xml(&xml);
        assert_eq!(p.worlds.len(), 2);
        // `Location` is the authoritative key, and a handful of pool locations are
        // shared by two objects, so `by_loc` may resolve a saved location back to a
        // different index. Compare the restored *locations*, which both the writer
        // and reader collapse identically, not the raw indices.
        let locs = |set: &HashSet<(Game, usize)>| -> HashSet<&'static str> {
            set.iter().map(|&(g, i)| g.objects()[i].location).collect()
        };
        assert_eq!(locs(&p.worlds[0].collected), locs(&worlds[0].collected));
        assert_eq!(locs(&p.worlds[0].forced), locs(&worlds[0].forced));
        assert_eq!(locs(&p.worlds[1].collected), locs(&worlds[1].collected));
        assert_eq!(p.worlds[0].items.get(&loc0).map(String::as_str), Some("Kokiri Sword"));
        assert_eq!(p.worlds[0].dest.get(&loc0).copied(), Some(2));
        // The escaped name survives the round trip verbatim.
        assert_eq!(p.worlds[0].items.get(&loc5).map(String::as_str), Some("Rupee & <Stick>"));

        assert_eq!(p.visited, visited);
        assert_eq!(p.out_links, out_links);
        assert_eq!(p.in_links, in_links);
        assert_eq!(p.spoiler_path.as_deref(), Some("C:/spoiler.txt"));
        assert_eq!(p.patch_path, Some(PathBuf::from("C:/patch.ootmm")));
    }

    #[test]
    fn xml_location_survives_rename() {
        // The whole point of the numeric key: a collected location still loads after
        // its `loc` string is renamed in a future version. Pick an object whose full
        // numeric footprint (scene, oid, type, layout) is unique so the match is
        // unambiguous, render it collected, then corrupt only the loc string.
        let objs = Game::Oot.objects();
        let idx = (0..objs.len())
            .find(|&i| {
                let o = &objs[i];
                objs.iter().filter(|x| {
                    x.scene == o.scene
                        && x.object_id == o.object_id
                        && x.type_ as u32 == o.type_ as u32
                        && x.layout as u32 == o.layout as u32
                }).count() == 1
                    && !o.location.contains(['&', '<', '>', '"'])
            })
            .expect("a uniquely-keyed plain-ASCII object");
        let real_loc = objs[idx].location;

        let mut w = crate::WorldData::default();
        w.collected.insert((Game::Oot, idx));
        let xml = render_save_xml(&[w], None, None, &HashSet::new(), &HashMap::new(), &HashMap::new());
        assert!(xml.contains(&format!("oid=\"{:#x}\"", objs[idx].object_id)));

        // Simulate an upstream rename: the saved loc no longer matches any object.
        let renamed = xml.replacen(
            &format!("loc=\"{real_loc}\""),
            "loc=\"OOT Totally Renamed Location That Matches Nothing\"",
            1,
        );
        assert_ne!(renamed, xml, "the loc attribute must have been rewritten");

        let p = parse_save_xml(&renamed);
        assert!(
            p.worlds[0].collected.contains(&(Game::Oot, idx)),
            "the object must still load via its (scene, oid, type, layout) key"
        );
    }

    #[test]
    fn xml_item_survives_rename() {
        // A placed item loads from its stable `id` even if the saved `name` is stale.
        // (Spoilers store the game-suffixed name, which is what the item table uses.)
        let loc = Game::Oot.objects()[0].location.to_string();
        let stored = "Kokiri Sword (OoT)";
        let id = crate::progression::find_item_id(stored).expect("known item");
        let canonical = qtsave::item_name(id).unwrap().to_string();

        let mut w = crate::WorldData::default();
        w.items.insert(loc.clone(), stored.to_string());
        let xml = render_save_xml(&[w], None, None, &HashSet::new(), &HashMap::new(), &HashMap::new());
        assert!(xml.contains(&format!("id=\"{id:#x}\"")));

        // Corrupt the item name; the id must still resolve it to the canonical name.
        let renamed = xml.replacen(&format!("name=\"{stored}\""), "name=\"Old Renamed Sword\"", 1);
        assert_ne!(renamed, xml);
        let p = parse_save_xml(&renamed);
        assert_eq!(p.worlds[0].items.get(&loc).map(String::as_str), Some(canonical.as_str()));
    }

    #[test]
    fn xml_entrance_names_are_fully_qualified() {
        // <in>/<out> (and the entrance itself) name the resolved exit as
        // "scene - side", not a bare, ambiguous destination.
        let ent = Game::Oot.entrances()[0].to_id;
        let full = entrance::display_name(Game::Oot, ent).unwrap();
        assert!(full.contains(" - "), "expected a 'scene - side' name, got {full:?}");
        let mut visited = HashSet::new();
        visited.insert((Game::Oot, ent));
        let xml = render_save_xml(
            &[crate::WorldData::default()],
            None,
            None,
            &visited,
            &HashMap::new(),
            &HashMap::new(),
        );
        assert!(xml.contains(&format!("name=\"{full}\"")));
    }

    #[test]
    fn xml_in_link_uses_spawn_arrow_form() {
        // An <in> link records where an entrance is entered FROM, and must read with
        // the arrow "scene -> side" form (tracker GetEntranceSpawnsString), distinct
        // from an <out> which keeps the dash "scene - side" form. Pick a Normal
        // entrance so the arrow keeps the same token order as the dash form.
        let src = Game::Oot
            .entrances()
            .iter()
            .find(|e| e.type_ == crate::data::EntranceType::Normal && !e.from_name.is_empty())
            .map(|e| e.to_id)
            .expect("a Normal entrance");
        let host = (Game::Oot, Game::Oot.entrances()[0].to_id);
        let leads = entrance::display_name(Game::Oot, src).unwrap(); // "scene - side"
        let spawn = entrance::spawns_name(Game::Oot, src).unwrap(); // "scene -> side"
        assert!(spawn.contains(" -> "), "spawn form should use an arrow, got {spawn:?}");
        assert_eq!(spawn, leads.replacen(" - ", " -> ", 1), "same tokens, arrow separator");

        let mut out_links = HashMap::new();
        out_links.insert(host, (Game::Oot, src));
        let mut in_links = HashMap::new();
        in_links.insert(host, vec![(Game::Oot, src)]);
        let mut visited = HashSet::new();
        visited.insert(host);
        let xml = render_save_xml(
            &[crate::WorldData::default()],
            None,
            None,
            &visited,
            &out_links,
            &in_links,
        );
        // The <out> carries the dash form, the <in> the arrow form.
        assert!(xml.contains(&format!("<out game=\"OoT\" id=\"{src:#x}\" name=\"{leads}\"/>")));
        assert!(xml.contains(&format!("<in game=\"OoT\" id=\"{src:#x}\" name=\"{spawn}\"/>")));
    }
}
