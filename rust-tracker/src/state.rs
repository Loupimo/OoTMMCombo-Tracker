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
        let mut app = TrackerApp {
            i18n: I18n::new(app_settings.language),
            app_settings,
            app_settings_path,
            scene,
            // The poller thread wakes the UI only on a real event / status change.
            poller: poller::spawn(ctx.clone()),
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
            save_path: data_dir.join("tracker_save.txt"),
            spoiler_path_file: data_dir.join("tracker_spoiler.txt"),
            dirty: false,
            status: String::new(),
            rom: RomVersion::Dev,
            rom_from_spoiler: false,
            icon_cache: HashMap::new(),
            grey_icon_cache: HashMap::new(),
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
        // Startup auto-loads, each gated by its "Auto Load Most Recent" option.
        if app.app_settings.auto_load_tracking {
            app.load_state(); // restore previous progress (may set patch_path)
        }
        // If a latest-version save restored a patch path, resolve it on the first
        // frame (loads it, or warns + prompts if the file moved / disappeared).
        app.patch_startup_check = app.patch_path.is_some();
        if app.app_settings.auto_load_spoiler {
            app.auto_load_spoiler(); // re-apply the last spoiler's settings / MQ layout
        }
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
        self.log_msg(format!("Multiplayer enabled — server {}:{}", cfg.server_host, cfg.server_port));
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
        self.log_msg(format!("r4 multiplayer — {} (server {}:{})", info.summary(), cfg.server_host, cfg.server_port));
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
                    self.log_msg(format!("Patch loaded: {} ({})", path.display(), info.summary()));
                    self.patch_info = Some(info);
                }
                Err(e) => {
                    self.log_msg(format!("Saved patch file is invalid ({e}). Please pick a patch file."));
                    self.prompt_patch_dialog();
                }
            }
        } else {
            self.log_msg(format!(
                "Saved patch file no longer exists: {}. Please pick the new location.",
                path.display()
            ));
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
                self.log_msg(format!("Patch loaded: {} ({})", path.display(), info.summary()));
                self.patch_path = Some(path);
                self.patch_info = Some(info);
                self.save_state(); // persist the new patch path in the save file
            }
            Err(e) => self.log_msg(format!("Failed to load patch file: {e}")),
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

    /// Reset all tracked progress (Qt `LogTab::ResetTracking`): autosave first,
    /// then clear collected items, forced marks and discovered entrances.
    pub(crate) fn reset_tracking(&mut self) {
        self.save_state(); // Qt autosaves the current state before wiping it
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

    /// "Save Tracking": pick a `.trck` file and write the current state to it.
    pub(crate) fn save_tracking_dialog(&mut self) {
        if let Some(path) =
            dialog::save_file(self.i18n.choose_name(), &[(self.i18n.trck_file(), "*.trck")], "trck")
        {
            // Create Backup When Saving: copy any existing file to `.bak` first.
            if self.app_settings.backup_on_save && path.exists() {
                let _ = std::fs::copy(&path, path.with_extension("bak"));
            }
            self.save_to(&path);
            self.log_msg(format!("{} {}", self.i18n.file_saved(), path.display()));
        }
    }

    /// "Load Tracking": pick a `.trck` file and restore the state from it.
    pub(crate) fn load_tracking_dialog(&mut self) {
        if let Some(path) =
            dialog::open_file(self.i18n.choose_trck(), &[(self.i18n.trck_file(), "*.trck")])
        {
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

    pub(crate) fn save_state(&self) {
        self.save_to(&self.save_path);
    }

    /// Serialize the tracked state (collected / forced / entrances / links) to a
    /// file. Shared by the autosave and the "Save Tracking" dialog.
    ///
    /// Multiworld: each world's collected / forced lines are grouped under a
    /// `WORLD <n>` marker (0-based). A file with no marker (or a single world)
    /// reads back into world 0, so old single-world saves still load. Entrances
    /// and links are the local player's exploration and stay world-independent.
    pub(crate) fn save_to(&self, path: &std::path::Path) {
        let mut out = String::new();
        // Version 3 header + the r4 patch file path. Older saves have no header
        // (and no PATCH line); load treats a PATCH line as "latest version" and
        // auto-loads the referenced patch on the next launch.
        out.push_str(&format!("{} {}\n", SAVE_VERSION_TAG, SAVE_VERSION));
        if let Some(p) = &self.patch_path {
            out.push_str(&format!("PATCH {}\n", p.display()));
        }
        for (wi, world) in self.worlds.iter().enumerate() {
            // The marker is only needed past world 0, so single-world saves keep
            // their original (untagged) shape.
            if wi > 0 {
                out.push_str(&format!("WORLD {wi}\n"));
            }
            for (game, idx) in &world.collected {
                // Manually-forced objects get an "F " tag so the gold tint survives
                // a reload; auto-collected ones stay as a plain Location line.
                if world.forced.contains(&(*game, *idx)) {
                    out.push_str("F ");
                }
                out.push_str(game.objects()[*idx].location);
                out.push('\n');
            }
            // Version 4: persist the spoiler placements (location -> item, and the
            // destination player when multiworld) so a reload restores the item
            // icons on its own — no spoiler needed, matching the Qt binary save.
            // Tab-separated because locations and item names both contain spaces.
            for (loc, item) in &world.items {
                out.push_str(&format!("ITEM {loc}\t{item}\n"));
            }
            for (loc, player) in &world.dest {
                out.push_str(&format!("DEST {loc}\t{player}\n"));
            }
        }
        // Visited entrances, distinct from object-location lines by the "E " tag.
        for (game, id) in &self.visited_entrances {
            let g = if *game == Game::Mm { 1 } else { 0 };
            out.push_str(&format!("E {g} {id}\n"));
        }
        // Discovered OutLinks: "O <outGame> <outId> <inGame> <inId>". The
        // InLinks index is rebuilt from these on load.
        for ((og, oid), (ig, iid)) in &self.out_links {
            let og = if *og == Game::Mm { 1 } else { 0 };
            let ig = if *ig == Game::Mm { 1 } else { 0 };
            out.push_str(&format!("O {og} {oid} {ig} {iid}\n"));
        }
        let _ = std::fs::write(path, out);
    }

    /// Restore the collected-set from the autosave file (by Location).
    pub(crate) fn load_state(&mut self) {
        let path = self.save_path.clone();
        self.load_from(&path);
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
            tracking::match_object(game, item.scene, item.ov_type, item.object, item.room, self.rom, &self.mq_scenes)
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
            self.log_msg(format!(
                "{tag} World Object: {} - Item : {name} (from world {} to world {})",
                obj.location, item.from_world, item.to_world
            ));
            self.dirty = true;
            self.prog_dirty = true;
            self.counts_dirty = true;
            // Auto Snap View only when the affected world is the one on screen.
            if self.app_settings.auto_snap && self.active_world == map_idx {
                self.pending_snap = Some((g, obj.render_scene, obj.x as f32, obj.y as f32));
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
        if let Some(hit) = tracking::resolve_collected(&ev, self.rom, &self.mq_scenes) {
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
            let (rs, ox, oy) = (obj.render_scene, obj.x as f32, obj.y as f32);
            if self.worlds[LOCAL].collected.insert(hit) {
                // Mirror the Qt MemoryReader log line.
                let game = if hit.0 == Game::Oot { "OoT" } else { "MM" };
                self.log_msg(format!("{game} World Object: {} - Item : {item}", obj.location));
                self.dirty = true;
                self.prog_dirty = true;
                self.counts_dirty = true;
                // Auto Snap View: remember where to recentre the map next frame,
                // but only while the local world is the one on screen (else the
                // mark isn't visible on the world the user is looking at).
                if self.app_settings.auto_snap && self.active_world == LOCAL {
                    self.pending_snap = Some((hit.0, rs, ox, oy));
                }
            }
        }
    }

    /// Load a spoiler log file: item placement + ROM build.
    pub(crate) fn load_spoiler(&mut self, path: &std::path::Path) {
        match std::fs::read_to_string(path) {
            Ok(text) => {
                let sp = spoiler::parse(&text);
                self.rom = sp.rom;
                self.rom_from_spoiler = true;
                self.mq_scenes = sp.mq_scenes;
                let num_worlds = sp.worlds.len().max(1);
                // Re-fill each world's placements, PRESERVING the collected / forced
                // sets across the reload (Qt `preservedStatus`) so loading a spoiler
                // never wipes progress — critical with auto-save on. Grow first, set
                // the placements, then drop any world the new seed no longer has.
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
        self.out_links.insert(out, inc);
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
            // The player is now in this scene (drives auto-follow / auto-GPS).
            self.player_scene = Some((evt.in_game, d.to_scene));
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

        self.log_msg(format!("X = {:.6}, Y = {:.6}, Z = {:.6}", inc.x, inc.y, inc.z));
        self.log_msg(format!(
            "New scene Loaded ! From : {out_str} (0x{:X}), To : {in_str} (0x{:X})",
            out.entrance_id, inc.entrance_id
        ));
        if entrance::lookup(out.game, out.entrance_id)
            .is_some_and(|m| m.type_ == data::EntranceType::One_Way_In)
        {
            self.log_msg(format!("Warning ! Entrance {out_str} (0x{:X}) is one way in only !", out.entrance_id));
        }
        if entrance::lookup(inc.game, inc.entrance_id)
            .is_some_and(|m| m.type_ == data::EntranceType::One_Way_Out)
        {
            self.log_msg(format!("Warning ! Entrance {in_str} (0x{:X}) is one way out only !", inc.entrance_id));
        }

        let sep = || "-".repeat(49);
        let hex08 = |v: u32| format!("0x{v:08X}");
        let hex02 = |v: u8| format!("0x{v:02X}");
        let row = |label: &str, from: String, to: String| {
            format!("{label:<13} | {from:>14} | {to:>14} |")
        };
        self.log_msg("              -----------------------------------".to_string());
        self.log_msg("              |      FROM      |       TO       |".to_string());
        let lines = [
            self.i_row_f("X", out.x, inc.x),
            self.i_row_f("Y", out.y, inc.y),
            self.i_row_f("Z", out.z, inc.z),
            row("Game", game_lbl(out.game).to_string(), game_lbl(inc.game).to_string()),
            row("Scene", hex08(evt.out_scene), hex08(evt.in_scene)),
            row("Entrance", hex08(evt.out_entrance), hex08(evt.in_entrance)),
            row("Room ID", hex02(out.curr_room), hex02(inc.curr_room)),
            row("Grotto Data", hex02(out.grotto_data), hex02(inc.grotto_data)),
            row("Age", hex02(out.age), hex02(inc.age)),
            row("Farore's Wind", hex02(out.farore_wind), hex02(inc.farore_wind)),
            row("Owl ID", hex02(out.owl_id), hex02(inc.owl_id)),
            row("Song ID", hex02(out.song), hex02(inc.song)),
        ];
        for line in lines {
            self.log_msg(sep());
            self.log_msg(line);
        }
        self.log_msg(sep());
    }

    /// One float table row for `log_entrance_details`.
    fn i_row_f(&self, label: &str, from: f32, to: f32) -> String {
        format!("{label:<13} | {:>14.6} | {:>14.6} |", from, to)
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
        self.reach = Some(crate::logic::solve_world(&self.rom_settings, &self.worlds, player));
    }

    /// Whether a check must be treated as *unreachable* by the accessibility
    /// filter: the filter is on, a reachability result exists, the location
    /// carries a logic rule, and the solver did not reach it. A check with no
    /// rule (not in `logic_locs`) is never considered unreachable — the logic has
    /// nothing to say about it, so it stays fully shown.
    pub(crate) fn obj_unreachable(&self, location: &str) -> bool {
        if !self.app_settings.logic_filter_enabled {
            return false;
        }
        match &self.reach {
            Some(r) => self.logic_locs.contains(location) && !r.reachable(location),
            None => false,
        }
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

    /// Preload the greyscale ("uncollected") variants of the progression item
    /// icons, a small budget per frame like `ensure_icons`. Only the icons the
    /// progression grid can show (ICON_BY_NAME) get a grey copy — the map / region
    /// icons never need one.
    pub(crate) fn ensure_prog_grey_icons(&mut self, ctx: &egui::Context) {
        let mut budget = 24;
        for &(_, path) in data::ICON_BY_NAME.iter() {
            if path.is_empty() || self.grey_icon_cache.contains_key(path) {
                continue;
            }
            let tex = load_color_image(&scene::resource_path(path)).ok().map(|img| {
                ctx.load_texture(
                    format!("{path}#grey"),
                    greyscale_image(&img),
                    egui::TextureOptions::LINEAR,
                )
            });
            self.grey_icon_cache.insert(path, tex);
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
        let Some(path) = scene.image_path(self.current_room, entrance_view) else {
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
                let follow = match self.active_tab {
                    Tab::Oot | Tab::Mm => {
                        self.app_settings.auto_follow_item && self.active_tab.game() == Some(g)
                    }
                    Tab::Entrance => {
                        self.app_settings.auto_follow_entrance && self.entrance_sub.game() == Some(g)
                    }
                    _ => false,
                };
                if follow {
                    if let Some(def) = g.scenes().iter().find(|s| s.id == sid) {
                        self.select_scene(g, def);
                    }
                }
                if self.app_settings.auto_gps_start {
                    self.gps_from = Some(ps);
                    self.gps_from_ent = None;
                }
            }
            self.followed_scene = Some(ps);
        }
        // Auto Snap View: jump to the last collected object's scene and centre on it.
        if let Some((g, sid, x, y)) = self.pending_snap.take() {
            if let Some(def) = g.scenes().iter().find(|s| s.id == sid) {
                self.active_tab = if g == Game::Oot { Tab::Oot } else { Tab::Mm };
                if self.scene.as_ref().map_or(true, |s| s.game != g || s.def.id != sid) {
                    self.select_scene(g, def);
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
