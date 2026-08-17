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
        let scene = Game::Oot
            .scenes()
            .iter()
            .find(|s| s.id == DEFAULT_SCENE)
            .map(|def| LiveScene::load(Game::Oot, def, &HashSet::new()));
        // Restore the persisted UI language (system language on first launch).
        let app_settings_path =
            PathBuf::from(format!("{}/rust-tracker/tracker_settings.toml", scene::REPO_ROOT));
        let app_settings = AppSettings::load(&app_settings_path);
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
            use_multiplayer: false,
            mp_host: "multi.ootmm.com".to_string(),
            mp_port: "13248".to_string(),
            log_lines: VecDeque::new(),
            collected: HashSet::new(),
            forced: HashSet::new(),
            visited_entrances: HashSet::new(),
            out_links: HashMap::new(),
            in_links: HashMap::new(),
            ent_helper: entrance::EntranceHelper::default(),
            gps_from: None,
            gps_to: None,
            gps_from_ent: None,
            gps_to_ent: None,
            entrance_sub: EntranceSub::Oot,
            entrance_table: None,
            focus_entrance: None,
            nav_all_expanded: true,
            obj_all_expanded: true,
            ent_search: String::new(),
            ent_all_expanded: true,
            ent_sort_col: 0,
            ent_sort_asc: true,
            ent_col_frac: [0.20, 0.26, 0.27, 0.27],
            spoiler_items: HashMap::new(),
            spoiler_worlds: HashMap::new(),
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
            sel_scene: [DEFAULT_SCENE, 0x2d], // Kokiri Forest / Termina Field
            last_item: None,
            last_entrance: None,
            player_scene: None,
            followed_scene: None,
            pending_snap: None,
            snap_pos: None,
            show_settings: false,
            show_about: false,
            rom_settings: settings::Settings::default(),
            excluded: settings::Excluded::default(),
            dashboard: progression::Dashboard::new(),
            prog_dirty: true,
            cached_totals: [(0, 0); 2],
            cached_scene_counts: [HashMap::new(), HashMap::new()],
            counts_dirty: true,
            save_path: PathBuf::from(format!("{}/rust-tracker/tracker_save.txt", scene::REPO_ROOT)),
            spoiler_path_file: PathBuf::from(format!(
                "{}/rust-tracker/tracker_spoiler.txt",
                scene::REPO_ROOT
            )),
            dirty: false,
            status: String::new(),
            rom: RomVersion::Dev,
            icon_cache: HashMap::new(),
            map_texture: None,
            map_size: Vec2::ZERO,
            load_error: None,
            zoom: 1.0,
            pan: Vec2::ZERO,
            view_initialized: false,
            last_frame: Instant::now(),
        };
        // Startup auto-loads, each gated by its "Auto Load Most Recent" option.
        if app.app_settings.auto_load_tracking {
            app.load_state(); // restore previous progress
        }
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
    pub(crate) fn toggle_tracking(&mut self) {
        self.tracking = !self.tracking;
        self.poller.set_tracking(self.tracking);
        if self.tracking {
            if self.use_multiplayer {
                self.log_msg(format!(
                    "Multiplayer requested ({}:{}) — networking not ported in this build.",
                    self.mp_host.trim(),
                    self.mp_port.trim()
                ));
            }
            self.log_msg(self.i18n.reading_mem().to_string());
        } else {
            self.log_msg(self.i18n.log_tracker_stop().to_string());
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
        self.collected.clear();
        self.forced.clear();
        self.visited_entrances.clear();
        self.out_links.clear();
        self.in_links.clear();
        self.ent_helper = entrance::EntranceHelper::default();
        self.last_item = None;
        self.last_entrance = None;
        self.dirty = true;
        self.prog_dirty = true;
        self.counts_dirty = true;
        self.rebuild_scene();
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
            self.collected.clear();
            self.forced.clear();
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
    pub(crate) fn save_to(&self, path: &std::path::Path) {
        let mut out = String::new();
        for (game, idx) in &self.collected {
            // Manually-forced objects get an "F " tag so the violet tint survives
            // a reload; auto-collected ones stay as a plain Location line.
            if self.forced.contains(&(*game, *idx)) {
                out.push_str("F ");
            }
            out.push_str(game.objects()[*idx].location);
            out.push('\n');
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
        let Ok(text) = std::fs::read_to_string(path) else { return };
        // Location is globally unique (base and MQ carry distinct strings), so a
        // flat index has no collisions and restores either layout.
        let mut by_location: HashMap<&'static str, (Game, usize)> = HashMap::new();
        for game in [Game::Oot, Game::Mm] {
            for (i, o) in game.objects().iter().enumerate() {
                by_location.insert(o.location, (game, i));
            }
        }
        for line in text.lines() {
            let line = line.trim();
            if let Some(rest) = line.strip_prefix("E ") {
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
            } else if let Some(loc) = line.strip_prefix("F ") {
                // Manually-forced object: collected AND forced (violet on the map).
                if let Some(&key) = by_location.get(loc) {
                    self.collected.insert(key);
                    self.forced.insert(key);
                }
            } else if let Some(&key) = by_location.get(line) {
                self.collected.insert(key);
            }
        }
        self.rebuild_in_links();
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
                    if game_version[0] != 0 {
                        self.rom = tracking::detect_rom(game_version);
                    }
                    for ev in events {
                        self.process_event(ev);
                    }
                }
            }
        }
    }

    /// Handle one live event: an entrance message or a collected item.
    pub(crate) fn process_event(&mut self, ev: Event) {
        if entrance::is_entrance(ev.mem) {
            self.handle_entrance(&ev);
        } else if let Some(hit) = tracking::resolve_collected(&ev, self.rom, &self.mq_scenes) {
            let obj = &hit.0.objects()[hit.1];
            let item = self
                .spoiler_items
                .get(obj.location)
                .cloned()
                .unwrap_or_else(|| obj.name.to_string());
            self.last_item = Some(item.clone());
            let (rs, ox, oy) = (obj.render_scene, obj.x as f32, obj.y as f32);
            if self.collected.insert(hit) {
                // Mirror the Qt MemoryReader log line.
                let game = if hit.0 == Game::Oot { "OoT" } else { "MM" };
                self.log_msg(format!("{game} World Object: {} - Item : {item}", obj.location));
                self.dirty = true;
                self.prog_dirty = true;
                self.counts_dirty = true;
                // Auto Snap View: remember where to recentre the map next frame.
                if self.app_settings.auto_snap {
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
                self.mq_scenes = sp.mq_scenes;
                let (n, mq, mw) = (sp.items.len(), self.mq_scenes.len(), sp.worlds.len());
                self.spoiler_items = sp.items;
                self.spoiler_worlds = sp.worlds;
                // ROM settings: parse the shuffle parameters then rebuild the
                // excluded-object set that hides vanilla / removed categories.
                self.rom_settings.parse_spoiler(&text, &self.mq_scenes);
                self.excluded = self.rom_settings.apply(&self.mq_scenes);
                self.rebuild_scene(); // MQ set may have changed which objects show
                self.prog_dirty = true; // spoiler / settings changed the dashboard
                self.counts_dirty = true;
                // Remember this spoiler so the next launch auto-applies its settings.
                let _ = std::fs::write(&self.spoiler_path_file, path.to_string_lossy().as_ref());
                self.status = if mw > 0 {
                    self.i18n.spoiler_multiworld(n, mw)
                } else {
                    self.i18n.spoiler_singleworld(n, mq)
                };
            }
            Err(e) => self.status = format!("Lecture spoiler échouée : {e}"),
        }
    }

    /// Sélectionne une scène : réinitialise la texture et la vue.
    pub(crate) fn select_scene(&mut self, game: Game, def: &'static data::SceneDef) {
        self.sel_scene[game.idx()] = def.id;
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
        let Some(evt) = self.ent_helper.parse(ev) else { return };

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
            self.last_entrance = Some(d.to_name.to_string());
            // The player is now in this scene (drives auto-follow / auto-GPS).
            self.player_scene = Some((evt.in_game, d.to_scene));
            let from = entrance::lookup(evt.out_game, evt.out_entrance).map(|o| o.to_name).unwrap_or("?");
            self.log_msg(self.i18n.entrance_detect(from, d.to_name));
        }
        self.dirty = true;
    }

    /// Rebuild the InLinks index from OutLinks (each out -> in edge implies the
    /// reverse in <- out source). Used after loading a save.
    pub(crate) fn rebuild_in_links(&mut self) {
        self.in_links.clear();
        for (&out, &inc) in &self.out_links {
            self.in_links.entry(inc).or_default().push(out);
        }
    }

    /// Refresh the current scene's markers from the collected-set.
    pub(crate) fn sync_collected(&mut self) {
        if let Some(scene) = self.scene.as_mut() {
            let g = scene.game;
            for o in &mut scene.objects {
                o.collected = self.collected.contains(&(g, o.index));
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
                let got = self.collected.contains(&(game, i));
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

    /// Toggle an object by hand (map or tree click). A fresh click marks it Forced
    /// (gold); clicking a Forced object clears it. Auto-collected objects (found in
    /// game) can NOT be unchecked by the user — only manual marks are removable.
    pub(crate) fn toggle_object(&mut self, game: Game, index: usize) {
        let key = (game, index);
        if self.collected.contains(&key) {
            // Only clear if the mark was user-forced; keep auto-collected objects.
            if !self.forced.remove(&key) {
                return;
            }
            self.collected.remove(&key);
        } else {
            self.collected.insert(key);
            self.forced.insert(key);
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

    /// Fabricate a real collected-event for an uncollected object of the current
    /// scene, so the auto-check can be seen without the game running.
    pub(crate) fn simulate_event(&mut self) {
        let (game, sid) = match self.scene.as_ref() {
            Some(s) => (s.game, s.def.id),
            None => return,
        };
        for (i, o) in game.objects().iter().enumerate() {
            if o.render_scene != sid || self.collected.contains(&(game, i)) {
                continue;
            }
            if let Some(ev) = tracking::demo_event(game, o) {
                self.process_event(ev); // same path as a live event (logs + last item)
                return;
            }
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
        if self.scene.as_ref().map_or(true, |s| s.game != game) {
            let id = self.sel_scene[game.idx()];
            if let Some(def) = game.scenes().iter().find(|s| s.id == id) {
                self.scene = Some(LiveScene::load(game, def, &self.mq_scenes));
                self.current_room = 0;
                self.map_texture = None;
                self.load_error = None;
                self.view_initialized = false;
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
        if self.counts_dirty {
            self.recompute_counts(); // only when collected / exclusions / layout changed
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
        self.poller.shutdown_and_wait();
    }
}
