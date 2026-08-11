//! Spike Rust + egui du tracker OoTMM.
//!
//! Brique prouvées :
//!   1. mémoire partagée "PJ64_SHARED_MEM" + ring buffer d'Event en direct
//!      (shared_mem.rs) ;
//!   2. navigation dans TOUTES les scènes des deux jeux (données générées depuis
//!      le C++, data.rs) via un arbre jeux -> régions -> scènes ;
//!   3. rendu de la map sélectionnée (pan/zoom) avec overlays cliquables.
#![windows_subsystem = "windows"]

mod data;
mod entrance;
mod gps;
mod inject;
mod poller;
mod progression;
mod scene;
mod settings;
mod shared_mem;
mod spoiler;
mod tracking;

use std::collections::{HashMap, HashSet, VecDeque};
use std::path::PathBuf;
use std::time::{Duration, Instant};

use eframe::egui::{self, pos2, vec2, Align2, Color32, FontId, Rect, Sense, Stroke, Vec2};

use scene::{Game, LiveScene};
use shared_mem::Event;
use tracking::RomVersion;

/// Nombre max d'événements gardés dans le panneau de log.
const LOG_CAP: usize = 500;
/// Scène chargée par défaut au démarrage (OoT Kokiri Forest = 0x55).
const DEFAULT_SCENE: u16 = 0x55;

/// Whether an object of context `ctx` shows under the active scene context
/// `eff` (None = the scene has no age/season context, so show everything).
fn context_allows(eff: Option<data::ObjectContext>, ctx: data::ObjectContext) -> bool {
    match eff {
        None => true,
        Some(c) => ctx == data::ObjectContext::All || ctx == c,
    }
}

/// Top-level tabs, mirroring the original Qt layout.
#[derive(Clone, Copy, PartialEq)]
enum Tab {
    Launch,
    Oot,
    Mm,
    Entrance,
    Progression,
}

impl Tab {
    const ALL: [Tab; 5] = [Tab::Launch, Tab::Oot, Tab::Mm, Tab::Entrance, Tab::Progression];

    fn label(self) -> &'static str {
        match self {
            Tab::Launch => "Launch",
            Tab::Oot => "OoT",
            Tab::Mm => "MM",
            Tab::Entrance => "Entrance",
            Tab::Progression => "Progression",
        }
    }
    /// The game this tab tracks, if any (Entrance/Launch/Progression: none).
    fn game(self) -> Option<Game> {
        match self {
            Tab::Oot => Some(Game::Oot),
            Tab::Mm => Some(Game::Mm),
            _ => None,
        }
    }
    fn is_entrance(self) -> bool {
        matches!(self, Tab::Entrance)
    }
}

/// Sub-tabs of the Entrance tab (Qt EntranceTab), mirroring OoT / MM / GPS.
#[derive(Clone, Copy, PartialEq)]
enum EntranceSub {
    Oot,
    Mm,
    Gps,
}

impl EntranceSub {
    /// The game of a game sub-tab (None for GPS).
    fn game(self) -> Option<Game> {
        match self {
            EntranceSub::Oot => Some(Game::Oot),
            EntranceSub::Mm => Some(Game::Mm),
            EntranceSub::Gps => None,
        }
    }
}

fn main() -> eframe::Result<()> {
    let options = eframe::NativeOptions {
        viewport: egui::ViewportBuilder::default()
            .with_inner_size([1360.0, 860.0])
            .with_min_inner_size([980.0, 640.0])
            .with_title("OoTMM Tracker — Rust/egui spike"),
        ..Default::default()
    };
    eframe::run_native(
        "OoTMM Tracker (Rust spike)",
        options,
        Box::new(|cc| {
            apply_qt_style(&cc.egui_ctx);
            Ok(Box::new(TrackerApp::new(&cc.egui_ctx)))
        }),
    )
}

/// Shuffle options offered for a boolean-ish parameter, and for a full shuffle.
const BOOL_OPTIONS: [data::ShuffleSetting; 3] = [
    data::ShuffleSetting::all,
    data::ShuffleSetting::vanilla,
    data::ShuffleSetting::removed,
];
const SHUFFLE_OPTIONS: [data::ShuffleSetting; 6] = [
    data::ShuffleSetting::all,
    data::ShuffleSetting::dungeons,
    data::ShuffleSetting::overworld,
    data::ShuffleSetting::starting,
    data::ShuffleSetting::vanilla,
    data::ShuffleSetting::removed,
];

/// Human label for a shuffle setting (shown in the ROM Settings editors).
fn shuffle_label(s: data::ShuffleSetting) -> &'static str {
    use data::ShuffleSetting as S;
    match s {
        S::vanilla => "Vanilla",
        S::removed => "Retiré",
        S::starting => "Départ",
        S::all => "Mélangé",
        S::dungeons => "Donjons",
        S::overworld => "Overworld",
    }
}

/// The Qt tracker's accent blue (#4a9edb), reused for headers / selection.
const ACCENT: Color32 = Color32::from_rgb(74, 158, 219);

/// Sentinel `entrance_table` region meaning "every entrance of the game" (the Qt
/// "All" node that has no child scenes, just the full global table).
const ALL_REGION: u8 = 0xFF;

/// A Qt "Fusion dark"-like theme: gray panels, blue accent + selection, and
/// slightly roomier spacing so the trees / grids read like the Qt build.
fn apply_qt_style(ctx: &egui::Context) {
    let mut style = (*ctx.style()).clone();

    let mut v = egui::Visuals::dark();
    v.panel_fill = Color32::from_rgb(48, 48, 48);
    v.window_fill = Color32::from_rgb(53, 53, 53);
    v.extreme_bg_color = Color32::from_rgb(33, 33, 33);
    v.faint_bg_color = Color32::from_rgb(60, 60, 60);
    v.override_text_color = Some(Color32::from_rgb(221, 221, 221));
    // Softer translucent selection with a crisp accent outline (Qt highlight).
    v.selection.bg_fill = Color32::from_rgba_unmultiplied(74, 158, 219, 90);
    v.selection.stroke = Stroke::new(1.0_f32, ACCENT);
    v.hyperlink_color = ACCENT;
    // Subtle hover / press feedback on rows and buttons.
    v.widgets.hovered.weak_bg_fill = Color32::from_rgb(66, 66, 66);
    v.widgets.active.weak_bg_fill = Color32::from_rgb(72, 72, 72);
    v.widgets.hovered.bg_stroke = Stroke::new(1.0_f32, Color32::from_rgb(90, 90, 90));
    style.visuals = v;

    style.spacing.item_spacing = vec2(8.0, 5.0);
    style.spacing.button_padding = vec2(8.0, 3.0);
    style.spacing.indent = 16.0;

    ctx.set_style(style);
}

/// A section heading tinted with the Qt accent (used across the side panels).
fn accent_heading(ui: &mut egui::Ui, text: &str) {
    ui.add_space(2.0);
    ui.label(egui::RichText::new(text).heading().size(17.0).color(ACCENT));
}

/// A precomputed entrance-table row: display strings + click navigation targets.
struct EntRow {
    scene: &'static str,
    entrance: &'static str,
    ent_target: (Game, u16, u32),
    spawn: EntCell,
    leads: EntCell,
    dot: Color32,
}

/// One in/out-link cell: N/A (one-way), undiscovered ("?"), or a clickable link.
enum EntCell {
    Na,
    Unknown,
    Link(&'static str, (Game, u16, u32)),
}

impl EntCell {
    /// The sort / search text of the cell.
    fn text(&self) -> &str {
        match self {
            EntCell::Na => "N/A",
            EntCell::Unknown => "?",
            EntCell::Link(n, _) => n,
        }
    }
}

/// A fixed-width table cell (entrance table): allocates `w` px and runs `add`.
fn table_cell(ui: &mut egui::Ui, w: f32, add: impl FnOnce(&mut egui::Ui)) {
    ui.allocate_ui_with_layout(vec2(w, 22.0), egui::Layout::left_to_right(egui::Align::Center), add);
}

/// A fixed-width clickable table cell showing a link-tinted, truncated label.
fn table_link_cell(ui: &mut egui::Ui, w: f32, text: &str) -> egui::Response {
    ui.allocate_ui_with_layout(vec2(w, 22.0), egui::Layout::left_to_right(egui::Align::Center), |ui| {
        ui.add(
            egui::Label::new(egui::RichText::new(text).color(Color32::from_rgb(150, 190, 230)))
                .truncate()
                .sense(Sense::click()),
        )
    })
    .inner
}

/// Per-game accent colour (Qt GameTab::GetAccentColorFor: OoT #4a9edb / MM #9b5de5).
fn game_accent(game: Game) -> Color32 {
    match game {
        Game::Oot => Color32::from_rgb(74, 158, 219),
        Game::Mm => Color32::from_rgb(155, 93, 229),
    }
}

/// Per-game selected-row background (Qt MapTab: OoT #1a4a7a / MM #5a2580).
fn game_selection(game: Game) -> Color32 {
    match game {
        Game::Oot => Color32::from_rgb(26, 74, 122),
        Game::Mm => Color32::from_rgb(90, 37, 128),
    }
}

/// A full-width tinted tree row (Qt TintedTreeWidget): `bg` fills the whole row,
/// `text` sits at `indent`, an optional collapse `arrow` at the far left, and an
/// optional (collected, total) count right-aligned (green once complete). Returns
/// the click response.
#[allow(clippy::too_many_arguments)]
fn tinted_row(
    ui: &mut egui::Ui,
    height: f32,
    indent: f32,
    bg: Color32,
    text: &str,
    text_col: Color32,
    count: Option<(usize, usize)>,
    arrow: Option<&str>,
    icon: Option<egui::TextureId>,
) -> egui::Response {
    let w = ui.available_width();
    let (rect, resp) = ui.allocate_exact_size(vec2(w, height), Sense::click());
    if ui.is_rect_visible(rect) {
        let painter = ui.painter();
        painter.rect_filled(rect, 2.0, bg);
        if resp.hovered() {
            painter.rect_filled(rect, 2.0, Color32::from_white_alpha(14));
        }
        let mid = rect.left_center();
        if let Some(a) = arrow {
            painter.text(mid + vec2(7.0, 0.0), Align2::LEFT_CENTER, a, FontId::proportional(11.0), text_col);
        }
        // Region icon just left of the label (18px slot before `indent`).
        if let Some(tex) = icon {
            let ir = Rect::from_center_size(mid + vec2(indent - 10.0, 0.0), Vec2::splat(16.0));
            painter.image(tex, ir, Rect::from_min_max(pos2(0.0, 0.0), pos2(1.0, 1.0)), Color32::WHITE);
        }
        painter.text(mid + vec2(indent, 0.0), Align2::LEFT_CENTER, text, FontId::proportional(13.0), text_col);
        if let Some((done, total)) = count {
            let cc = if total > 0 && done >= total {
                Color32::from_rgb(120, 210, 120)
            } else {
                Color32::from_gray(165)
            };
            painter.text(
                rect.right_center() - vec2(8.0, 0.0),
                Align2::RIGHT_CENTER,
                format!("{done}/{total}"),
                FontId::proportional(11.5),
                cc,
            );
        }
    }
    resp
}

struct TrackerApp {
    // --- Navigation / scène courante ---
    scene: Option<LiveScene>,

    // --- Mémoire partagée / flux d'événements ---
    /// Background poller (owns the shared-memory link off the UI thread) and the
    /// live connection state it reports.
    poller: poller::Poller,
    connected: bool,
    log: VecDeque<Event>,
    /// Objets collectés, source de vérité globale (clé = jeu + index objet).
    collected: HashSet<(Game, usize)>,
    /// Subset of `collected` that was toggled by hand (ObjectState::Forced) — drawn
    /// in the accent/violet tint on the map, distinct from auto-collected objects.
    forced: HashSet<(Game, usize)>,
    /// Entrances visited live (keyed by game + entrance id).
    visited_entrances: HashSet<(Game, u32)>,
    /// EntranceLink OutLink: where leaving an entrance leads (entrance -> entrance).
    out_links: HashMap<(Game, u32), (Game, u32)>,
    /// EntranceLink InLinks: the sources known to lead to each entrance.
    in_links: HashMap<(Game, u32), Vec<(Game, u32)>>,
    /// The OUT/IN message assembler (faithful EntranceHelper state machine).
    ent_helper: entrance::EntranceHelper,
    /// GPS destination scene (route computed from the current scene).
    gps_target: Option<u16>,
    /// GPS sub-tab: game + start scene of the standalone route page.
    gps_game: Game,
    gps_from: Option<u16>,
    /// Entrance tab: the active sub-tab (OoT / MM / GPS), like the Qt EntranceTab.
    entrance_sub: EntranceSub,
    /// Entrance tab: when a region is selected, its center shows the global
    /// entrance table for that (game, region) instead of a scene minimap.
    entrance_table: Option<(Game, u8)>,
    /// Pending "focus this entrance" request: after loading a scene from the
    /// entrance table, centre the map on this entrance id (EntranceTab focus).
    focus_entrance: Option<u32>,
    /// "Expand/collapse all" toggle state for the scene nav / object trees.
    nav_all_expanded: bool,
    obj_all_expanded: bool,
    /// Entrance table sort column (0=Scene 1=Entrance 2=spawn 3=leads) + order.
    ent_sort_col: usize,
    ent_sort_asc: bool,
    /// Entrance-table column width fractions (Scene / Entrance / spawn / leads),
    /// user-resizable by dragging a header separator.
    ent_col_frac: [f32; 4],
    /// Item held at each object Location, from a loaded spoiler log.
    spoiler_items: HashMap<String, String>,
    /// Multiworld: destination player for a location's item (when it differs).
    spoiler_worlds: HashMap<String, u8>,
    /// Scenes running the Master Quest / JP layout (from the spoiler settings).
    mq_scenes: HashSet<(Game, u16)>,
    /// Age/season toggle for scenes that have a context (false = Child/Winter,
    /// true = Adult/Spring), mirroring the Qt ContextSwitchButton.
    context_toggle: bool,
    /// Live scene-list filter text ("Find…" box above the nav tree).
    scene_search: String,
    /// Live object-list filter text ("Find…" box above the object tree).
    obj_search: String,
    /// Category filter (FilterManager::ActiveFilter): visible render-types per
    /// game (index by Game::idx). An object shows on the map only if its
    /// render-type is in this set.
    active_types: [HashSet<data::ObjectType>; 2],
    /// Index of the displayed room (for multi-room dungeons).
    current_room: usize,
    /// Active top-level tab.
    active_tab: Tab,
    /// Last selected scene per game (index by Game::idx), restored on tab switch.
    sel_scene: [u16; 2],
    /// Status-bar memory: last collected item and last entrance crossed.
    last_item: Option<String>,
    last_entrance: Option<String>,
    /// Whether the ROM Settings window is open.
    show_settings: bool,
    /// Parsed / edited ROM build settings (shuffle parameters).
    rom_settings: settings::Settings,
    /// Objects hidden by the ROM settings (FilterManager::ExcludedObj).
    excluded: settings::Excluded,
    /// The progression dashboard (ProgressionTab) state.
    dashboard: progression::Dashboard,
    /// Whether the dashboard needs a rebuild (collected / spoiler / settings changed).
    prog_dirty: bool,
    /// Per-game (collected, total) and per-render-scene counts, recomputed once
    /// per frame into this cache so the tab bar + scene tree read the same values
    /// (one object scan per game instead of several across the panels).
    cached_totals: [(usize, usize); 2],
    cached_scene_counts: [HashMap<u16, (usize, usize)>; 2],
    /// Recompute the counts only when the collected set / exclusions / layout
    /// change — not on every mouse-move frame.
    counts_dirty: bool,
    /// Autosave file for the collected-set, and whether it needs flushing.
    save_path: PathBuf,
    /// Sidecar remembering the last loaded spoiler path (auto-loaded at startup,
    /// mirroring the Qt AutoLoadMostRecentSpoilerLog so the ROM settings — which
    /// filter the map — are applied from launch, not only after a manual drop).
    spoiler_path_file: PathBuf,
    dirty: bool,

    // --- Connexion / injection ---
    status: String,
    /// ROM build reported by the DLL (drives the NPC id fix-up).
    rom: RomVersion,

    // --- Rendu de la map ---
    /// Object-type icon textures (None = load failed → colored fallback).
    icon_cache: HashMap<&'static str, Option<egui::TextureHandle>>,
    map_texture: Option<egui::TextureHandle>,
    map_size: Vec2,
    load_error: Option<String>,

    // --- Transform de vue (pan/zoom) ---
    zoom: f32,
    pan: Vec2,
    view_initialized: bool,

    /// Wall-clock of the last rendered frame, for the interaction frame-rate cap.
    last_frame: Instant,
}

impl TrackerApp {
    fn new(ctx: &egui::Context) -> Self {
        let scene = Game::Oot
            .scenes()
            .iter()
            .find(|s| s.id == DEFAULT_SCENE)
            .map(|def| LiveScene::load(Game::Oot, def, &HashSet::new()));
        let mut app = TrackerApp {
            scene,
            // The poller thread wakes the UI only on a real event / status change.
            poller: poller::spawn(ctx.clone()),
            connected: false,
            log: VecDeque::new(),
            collected: HashSet::new(),
            forced: HashSet::new(),
            visited_entrances: HashSet::new(),
            out_links: HashMap::new(),
            in_links: HashMap::new(),
            ent_helper: entrance::EntranceHelper::default(),
            gps_target: None,
            gps_game: Game::Oot,
            gps_from: None,
            entrance_sub: EntranceSub::Oot,
            entrance_table: None,
            focus_entrance: None,
            nav_all_expanded: true,
            obj_all_expanded: true,
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
            show_settings: false,
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
        app.load_state(); // restore previous progress
        app.auto_load_spoiler(); // re-apply the last spoiler's settings / MQ layout
        app // the poller thread handles connection / injection from here
    }

    /// Re-load the most recently used spoiler (path remembered in the sidecar), so
    /// its ROM settings and MQ layout filter the map from launch — mirroring the Qt
    /// AutoLoadMostRecentSpoilerLog. A missing / stale path is silently ignored.
    fn auto_load_spoiler(&mut self) {
        let Ok(path) = std::fs::read_to_string(&self.spoiler_path_file) else { return };
        let path = PathBuf::from(path.trim());
        if path.is_file() {
            self.load_spoiler(&path);
        }
    }

    /// Persist the collected-set, keyed by each object's globally-unique
    /// Location so the file survives data regeneration / layout changes.
    fn save_state(&self) {
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
        let _ = std::fs::write(&self.save_path, out);
    }

    /// Restore the collected-set from the autosave file (by Location).
    fn load_state(&mut self) {
        let Ok(text) = std::fs::read_to_string(&self.save_path) else { return };
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
    fn process_poll(&mut self) {
        while let Ok(msg) = self.poller.rx.try_recv() {
            match msg {
                poller::PollMsg::Status(connected, text) => {
                    self.connected = connected;
                    self.status = text;
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
    fn process_event(&mut self, ev: Event) {
        if entrance::is_entrance(ev.mem) {
            self.handle_entrance(&ev);
        } else if let Some(hit) = tracking::resolve_collected(&ev, self.rom, &self.mq_scenes) {
            let obj = &hit.0.objects()[hit.1];
            self.last_item = Some(
                self.spoiler_items
                    .get(obj.location)
                    .cloned()
                    .unwrap_or_else(|| obj.name.to_string()),
            );
            if self.collected.insert(hit) {
                self.dirty = true;
                self.prog_dirty = true;
                self.counts_dirty = true;
            }
        }
        self.push_event(ev);
    }

    /// Load a spoiler log file: item placement + ROM build.
    fn load_spoiler(&mut self, path: &std::path::Path) {
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
                    format!("Spoiler multiworld : {n} emplacements, {mw} vers un autre joueur")
                } else {
                    format!("Spoiler chargé : {n} emplacements, {mq} scènes MQ/JP")
                };
            }
            Err(e) => self.status = format!("Lecture spoiler échouée : {e}"),
        }
    }

    /// Sélectionne une scène : réinitialise la texture et la vue.
    fn select_scene(&mut self, game: Game, def: &'static data::SceneDef) {
        self.sel_scene[game.idx()] = def.id;
        self.scene = Some(LiveScene::load(game, def, &self.mq_scenes));
        self.current_room = 0;
        self.map_texture = None;
        self.load_error = None;
        self.view_initialized = false;
    }

    /// Rebuild the current scene's objects (e.g. after the MQ set changed),
    /// keeping the loaded texture and view.
    fn rebuild_scene(&mut self) {
        if let Some(s) = &self.scene {
            self.scene = Some(LiveScene::load(s.game, s.def, &self.mq_scenes));
        }
    }

    fn push_event(&mut self, ev: Event) {
        if self.log.len() >= LOG_CAP {
            self.log.pop_back();
        }
        self.log.push_front(ev);
    }

    /// Feed a live entrance message to the state machine. When an OUT / IN pair
    /// validates, record the discovered EntranceLink (out -> in and its reverse
    /// in-link), mark both endpoints visited and update the status bar.
    fn handle_entrance(&mut self, ev: &Event) {
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

        // Status bar: the entrance we just arrived at.
        if let Some(d) = entrance::lookup(evt.in_game, evt.in_entrance) {
            self.last_entrance = Some(d.to_name.to_string());
        }
        self.dirty = true;
    }

    /// Rebuild the InLinks index from OutLinks (each out -> in edge implies the
    /// reverse in <- out source). Used after loading a save.
    fn rebuild_in_links(&mut self) {
        self.in_links.clear();
        for (&out, &inc) in &self.out_links {
            self.in_links.entry(inc).or_default().push(out);
        }
    }

    /// Refresh the current scene's markers from the collected-set.
    fn sync_collected(&mut self) {
        if let Some(scene) = self.scene.as_mut() {
            let g = scene.game;
            for o in &mut scene.objects {
                o.collected = self.collected.contains(&(g, o.index));
            }
        }
    }

    /// The active object context of the current scene (None = no context = show
    /// all), driven by the age/season toggle (RefreshSceneContext).
    fn effective_context(&self) -> Option<data::ObjectContext> {
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
    fn recompute_counts(&mut self) {
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

    /// Toggle an object collected/uncollected by hand (map or tree click): a fresh
    /// toggle marks it Forced (violet); clearing removes both states.
    fn toggle_object(&mut self, game: Game, index: usize) {
        let key = (game, index);
        if self.collected.remove(&key) {
            self.forced.remove(&key);
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
    fn ensure_icons(&mut self, ctx: &egui::Context) {
        let paths = data::ICON_PATHS
            .iter()
            .copied()
            .chain(data::ICON_BY_NAME.iter().map(|&(_, p)| p))
            .chain(data::ENTRANCE_ICONS.iter().map(|&(_, p)| p))
            .chain(data::OOT_REGION_ICONS.iter().copied())
            .chain(data::MM_REGION_ICONS.iter().copied());
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
    fn ensure_texture(&mut self, ctx: &egui::Context) {
        if self.map_texture.is_some() || self.load_error.is_some() {
            return;
        }
        let Some(scene) = &self.scene else { return };
        let entrance_view = self.active_tab.is_entrance();
        let Some(path) = scene.image_path(self.current_room, entrance_view) else {
            self.load_error = Some("(pas d'image pour cette scène)".to_owned());
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
    fn simulate_event(&mut self) {
        let (game, sid) = match self.scene.as_ref() {
            Some(s) => (s.game, s.def.id),
            None => return,
        };
        for (i, o) in game.objects().iter().enumerate() {
            if o.render_scene != sid || self.collected.contains(&(game, i)) {
                continue;
            }
            if let Some(ev) = tracking::demo_event(game, o) {
                if let Some(hit) = tracking::resolve_collected(&ev, self.rom, &self.mq_scenes) {
                    if self.collected.insert(hit) {
                        self.dirty = true;
                        self.prog_dirty = true;
                        self.counts_dirty = true;
                    }
                }
                self.push_event(ev);
                return;
            }
        }
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

        // Flush the autosave right after a change (frames are sparse now).
        if self.dirty {
            self.save_state();
            self.dirty = false;
        }
        // No idle timer: the poller thread wakes us on real events, egui wakes on
        // input, so the UI otherwise sleeps (idle ~0, like the Qt build).
    }
}

impl TrackerApp {
    /// The game currently in context: the OoT / MM tab, or the Entrance tab's
    /// active game sub-tab (None on Launch / Progression / Entrance-GPS).
    fn current_game(&self) -> Option<Game> {
        match self.active_tab {
            Tab::Oot => Some(Game::Oot),
            Tab::Mm => Some(Game::Mm),
            Tab::Entrance => self.entrance_sub.game(),
            _ => None,
        }
    }

    /// Move `self.scene` to the context game if needed (restoring its last
    /// selected scene). Launch / Progression / Entrance-GPS keep the current one.
    fn ensure_tab_scene(&mut self) {
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

    fn draw_menu_bar(&mut self, ctx: &egui::Context) {
        egui::TopBottomPanel::top("menu").show(ctx, |ui| {
            egui::menu::bar(ui, |ui| {
                ui.menu_button("Options", |ui| {
                    if ui.button("Recentrer la vue").clicked() {
                        self.view_initialized = false;
                        ui.close_menu();
                    }
                });
                // The age/season context toggle now lives above the map, and the
                // category filter moved next to the object list (funnel button).
                if ui.button("ROM Settings").clicked() {
                    self.show_settings = true;
                }
            });
        });
    }

    /// The category-filter popup: one checkbox per filterable object type of the
    /// active game, mirroring the Qt FilterManager button. Only meaningful on the
    /// OoT / MM item tabs.
    fn draw_filter_menu(&mut self, ui: &mut egui::Ui) {
        let Some(game) = self.active_tab.game() else {
            ui.weak("Onglet OoT ou MM requis.");
            return;
        };
        let types = match game {
            Game::Oot => data::OOT_FILTER_TYPES,
            Game::Mm => data::MM_FILTER_TYPES,
        };
        ui.horizontal(|ui| {
            if ui.button("Tout").clicked() {
                self.active_types[game.idx()] = types.iter().copied().collect();
            }
            if ui.button("Rien").clicked() {
                self.active_types[game.idx()].clear();
            }
        });
        ui.separator();
        // Sorted by display name, like the Qt filter menu.
        let mut sorted = types.to_vec();
        sorted.sort_by_key(|t| data::OBJ_TYPE_NAMES[*t as usize]);
        egui::ScrollArea::vertical().max_height(420.0).show(ui, |ui| {
            for t in sorted {
                let set = &mut self.active_types[game.idx()];
                let mut on = set.contains(&t);
                if ui.checkbox(&mut on, data::OBJ_TYPE_NAMES[t as usize]).changed() {
                    if on {
                        set.insert(t);
                    } else {
                        set.remove(&t);
                    }
                }
            }
        });
    }

    fn draw_tab_bar(&mut self, ctx: &egui::Context) {
        // Per-game object totals (drive the OoT/MM tab counters + the grand total).
        let oot = self.cached_totals[Game::Oot.idx()];
        let mm = self.cached_totals[Game::Mm.idx()];
        // Entrances visited / total (active-layout entrances of both games).
        let ent_total = Game::Oot
            .entrances()
            .iter()
            .filter(|e| tracking::scene_layout_active(e.layout, Game::Oot, e.to_scene, &self.mq_scenes))
            .count()
            + Game::Mm
                .entrances()
                .iter()
                .filter(|e| tracking::scene_layout_active(e.layout, Game::Mm, e.to_scene, &self.mq_scenes))
                .count();
        let ent_visited = self.visited_entrances.len();

        egui::TopBottomPanel::top("tabs").show(ctx, |ui| {
            ui.add_space(2.0);
            ui.horizontal(|ui| {
                ui.spacing_mut().item_spacing.x = 6.0;
                ui.spacing_mut().button_padding = vec2(10.0, 5.0);
                for tab in Tab::ALL {
                    let selected = self.active_tab == tab;
                    // Per-tab count suffix (OoT / MM object totals, entrances).
                    let count = match tab {
                        Tab::Oot => Some(format!("{}/{}", oot.0, oot.1)),
                        Tab::Mm => Some(format!("{}/{}", mm.0, mm.1)),
                        Tab::Entrance => Some(format!("{ent_visited}/{ent_total}")),
                        _ => None,
                    };
                    let selected_now = self.tab_button(ui, tab, selected, count.as_deref());
                    if selected_now && !selected {
                        self.active_tab = tab;
                        // The layer may change (item map vs minimap): reload.
                        self.map_texture = None;
                        self.view_initialized = false;
                        self.current_room = 0;
                    }
                }
                // Grand total (all objects, both games) on the far right.
                ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                    ui.label(
                        egui::RichText::new(format!("Total  {}/{}", oot.0 + mm.0, oot.1 + mm.1))
                            .strong()
                            .color(Color32::from_rgb(221, 238, 255)),
                    );
                });
            });
            ui.add_space(2.0);
        });
    }

    /// One tab button: bold name + a small dimmed count. Returns whether clicked.
    fn tab_button(&self, ui: &mut egui::Ui, tab: Tab, selected: bool, count: Option<&str>) -> bool {
        let name_col = if selected {
            Color32::from_rgb(221, 238, 255)
        } else {
            Color32::from_gray(190)
        };
        let mut job = egui::text::LayoutJob::default();
        job.append(
            tab.label(),
            0.0,
            egui::TextFormat {
                color: name_col,
                font_id: egui::FontId::proportional(14.5),
                ..Default::default()
            },
        );
        if let Some(c) = count {
            job.append(
                &format!("  {c}"),
                0.0,
                egui::TextFormat {
                    color: Color32::from_gray(150),
                    font_id: egui::FontId::proportional(11.5),
                    ..Default::default()
                },
            );
        }
        ui.selectable_label(selected, job).clicked()
    }

    fn draw_status_bar(&mut self, ctx: &egui::Context) {
        egui::TopBottomPanel::bottom("status").show(ctx, |ui| {
            ui.horizontal(|ui| {
                let col = if self.connected {
                    Color32::from_rgb(120, 200, 90)
                } else {
                    Color32::from_rgb(220, 120, 90)
                };
                ui.colored_label(col, "●");
                ui.label(&self.status);
                ui.separator();
                ui.label(format!("Item : {}", self.last_item.as_deref().unwrap_or("—")));
                ui.separator();
                ui.label(format!("Entrée : {}", self.last_entrance.as_deref().unwrap_or("—")));

                let green = Color32::from_rgb(120, 200, 90);
                let gray = Color32::from_gray(120);
                // Auto-save is always on in the Rust build; the reveal indicator
                // mirrors (and toggles) the progression "reveal uncollected" option.
                ui.separator();
                ui.colored_label(green, "●");
                ui.label("Auto-save on");
                ui.separator();
                let reveal = self.dashboard.reveal;
                ui.colored_label(if reveal { green } else { gray }, "●");
                if ui.selectable_label(false, "Items revealed").clicked() {
                    self.dashboard.set_reveal(!reveal);
                }
            });
        });
    }

    fn draw_settings_window(&mut self, ctx: &egui::Context) {
        let mut open = self.show_settings;
        let mut apply = false;
        egui::Window::new("ROM Settings")
            .open(&mut open)
            .default_width(600.0)
            .resizable(true)
            .show(ctx, |ui| {
                ui.horizontal(|ui| {
                    ui.label(format!("Build : {:?}", self.rom));
                    ui.separator();
                    ui.label(format!("Jeu : {:?}", self.rom_settings.game));
                    ui.label(format!("Mode : {:?}", self.rom_settings.mode));
                    ui.label(format!("Objectif : {:?}", self.rom_settings.goal));
                });
                ui.label(format!(
                    "Objets masqués : {} OoT / {} MM  ·  MQ/JP : {} scènes",
                    self.excluded.oot.len(),
                    self.excluded.mm.len(),
                    self.mq_scenes.len()
                ));
                ui.separator();
                if ui.button("Appliquer").clicked() {
                    apply = true;
                }
                ui.separator();
                egui::ScrollArea::vertical().id_salt("settings_s").show(ui, |ui| {
                    self.draw_settings_params(ui);
                });
            });
        if apply {
            self.excluded = self.rom_settings.apply(&self.mq_scenes);
            self.rebuild_scene();
            self.prog_dirty = true; // settings drive shared / disabled / starting items
            self.counts_dirty = true;
        }
        self.show_settings = open;
    }

    /// The editable list of shuffle parameters, grouped by kind. Each row picks a
    /// ShuffleSetting appropriate to the parameter's ParamType.
    fn draw_settings_params(&mut self, ui: &mut egui::Ui) {
        use data::ParamType as PT;

        // General page: game / mode / goal selectors (Settings::AddSetting fields).
        egui::CollapsingHeader::new("Général").default_open(true).show(ui, |ui| {
            ui.horizontal(|ui| {
                ui.label("Jeu");
                egui::ComboBox::from_id_salt("set_game")
                    .selected_text(self.rom_settings.game.label())
                    .show_ui(ui, |ui| {
                        for g in settings::RomGame::ALL {
                            ui.selectable_value(&mut self.rom_settings.game, g, g.label());
                        }
                    });
            });
            ui.horizontal(|ui| {
                ui.label("Mode");
                egui::ComboBox::from_id_salt("set_mode")
                    .selected_text(self.rom_settings.mode.label())
                    .show_ui(ui, |ui| {
                        for m in settings::GameMode::ALL {
                            ui.selectable_value(&mut self.rom_settings.mode, m, m.label());
                        }
                    });
            });
            ui.horizontal(|ui| {
                ui.label("Objectif");
                egui::ComboBox::from_id_salt("set_goal")
                    .selected_text(self.rom_settings.goal.label())
                    .show_ui(ui, |ui| {
                        for g in settings::GoalMode::ALL {
                            ui.selectable_value(&mut self.rom_settings.goal, g, g.label());
                        }
                    });
            });
        });

        // Layouts page: per-dungeon Master Quest + the MM JP Deku Palace toggle.
        egui::CollapsingHeader::new("Layouts MQ / JP").show(ui, |ui| {
            for &(name, scene) in settings::OOT_MQ_DUNGEONS {
                let key = (Game::Oot, scene);
                let mut on = self.mq_scenes.contains(&key);
                if ui.checkbox(&mut on, format!("MQ · {name}")).changed() {
                    if on {
                        self.mq_scenes.insert(key);
                    } else {
                        self.mq_scenes.remove(&key);
                    }
                }
            }
            let jp_key = (Game::Mm, settings::MM_JP_SCENES[0]);
            let mut jp = self.mq_scenes.contains(&jp_key);
            if ui.checkbox(&mut jp, "JP · Deku Palace").changed() {
                for &sc in settings::MM_JP_SCENES {
                    if jp {
                        self.mq_scenes.insert((Game::Mm, sc));
                    } else {
                        self.mq_scenes.remove(&(Game::Mm, sc));
                    }
                }
            }
        });

        // Two distinct effects: FilterSettings gate which object CATEGORIES render
        // on the map (via the excluded set); ItemSettings gate which ITEMS the
        // progression dashboard tracks. Making that split explicit avoids the
        // impression that editing a progression-only setting should move the map.
        for (title, table) in [
            ("Filtres carte (catégories d'objets)", data::FILTER_SETTINGS),
            ("Réglages items (progression uniquement)", data::ITEM_SETTINGS),
        ] {
            egui::CollapsingHeader::new(title).default_open(title.starts_with("Filtres")).show(ui, |ui| {
                for m in table {
                    // Skip the non-shuffle meta parameters (game / mode selectors).
                    if matches!(m.type_, PT::game | PT::mode) {
                        continue;
                    }
                    let cur = self.rom_settings.value(m.key);
                    let opts: &[data::ShuffleSetting] = match m.type_ {
                        PT::shuffle => &SHUFFLE_OPTIONS,
                        _ => &BOOL_OPTIONS,
                    };
                    ui.horizontal(|ui| {
                        egui::ComboBox::from_id_salt(m.key)
                            .width(120.0)
                            .selected_text(shuffle_label(cur))
                            .show_ui(ui, |ui| {
                                for &opt in opts {
                                    if ui.selectable_label(cur == opt, shuffle_label(opt)).clicked() {
                                        self.rom_settings.set_value(m.key, opt);
                                    }
                                }
                            });
                        ui.label(m.name);
                    });
                }
            });
        }
    }

    /// Launch tab: connection controls + program log.
    fn draw_launch(&mut self, ctx: &egui::Context) {
        egui::CentralPanel::default().show(ctx, |ui| {
            ui.heading("Launch");
            ui.horizontal(|ui| {
                if ui.button("Connecter / Injecter").clicked() {
                    self.poller.force_reconnect();
                }
                if ui.button("Simuler un évènement").clicked() {
                    self.simulate_event();
                }
                ui.weak("· Déposez un spoiler log sur la fenêtre");
            });
            ui.separator();
            ui.label("Journal :");
            egui::ScrollArea::vertical().id_salt("launchlog").show(ui, |ui| {
                if self.log.is_empty() {
                    ui.weak("Aucun évènement.");
                }
                for ev in self.log.iter() {
                    ui.monospace(format!(
                        "PC {:#010x}  Mem {:#010x}  q={:#x},{:#x},{:#x}",
                        ev.pc, ev.mem, ev.query[0], ev.query[1], ev.query[2]
                    ));
                }
            });
        });
    }

    /// Right panel of the item tabs: objects grouped by category.
    fn draw_object_tree(&mut self, ctx: &egui::Context) {
        let mut toggle: Option<(Game, usize)> = None;
        let mut obj_set_all: Option<bool> = None;
        egui::SidePanel::right("objtree")
            .resizable(true)
            .default_width(320.0)
            .show(ctx, |ui| {
                let Some((game, sname, done, total)) = self.scene.as_ref().map(|s| {
                    let (d, t) = s.progress();
                    (s.game, s.def.name, d, t)
                }) else {
                    accent_heading(ui, "Objets");
                    return;
                };

                // Header: scene name, collected/total, and the category filter
                // button (moved here from the menu bar).
                ui.horizontal(|ui| {
                    ui.label(egui::RichText::new(sname).heading().size(16.0).color(ACCENT));
                    ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                        ui.menu_button("⚙", |ui| self.draw_filter_menu(ui))
                            .response
                            .on_hover_text("Filtres de catégories");
                        ui.label(format!("{done} / {total}"));
                    });
                });
                ui.add(
                    egui::TextEdit::singleline(&mut self.obj_search)
                        .hint_text("Find…")
                        .desired_width(f32::INFINITY),
                );
                let label = if self.obj_all_expanded { "⊟ Réduire tout" } else { "⊞ Développer tout" };
                if ui.add(egui::Button::new(label).min_size(vec2(ui.available_width(), 0.0))).clicked() {
                    self.obj_all_expanded = !self.obj_all_expanded;
                    obj_set_all = Some(self.obj_all_expanded);
                }
                ui.separator();

                let query = self.obj_search.trim().to_lowercase();
                let active = self.active_types[game.idx()].clone();
                let scene = self.scene.as_ref().unwrap();
                let accent = game_accent(game);

                egui::ScrollArea::vertical().id_salt("objtree_s").show(ui, |ui| {
                    ui.spacing_mut().item_spacing.y = 3.0;
                    // Category order, respecting the ROM exclusion + the active filter.
                    let mut order: Vec<data::ObjectType> = Vec::new();
                    for o in &scene.objects {
                        if self.excluded.contains(game, o.index) || !active.contains(&o.type_) {
                            continue;
                        }
                        if !order.contains(&o.type_) {
                            order.push(o.type_);
                        }
                    }
                    for ty in order {
                        let in_cat =
                            |o: &&scene::LiveObject| o.type_ == ty && !self.excluded.contains(game, o.index);
                        // Objects of this category matching the search box.
                        let objs: Vec<&scene::LiveObject> = scene
                            .objects
                            .iter()
                            .filter(in_cat)
                            .filter(|o| {
                                query.is_empty()
                                    || o.name.to_lowercase().contains(&query)
                                    || self
                                        .spoiler_items
                                        .get(o.location)
                                        .is_some_and(|it| it.to_lowercase().contains(&query))
                            })
                            .collect();
                        if objs.is_empty() {
                            continue;
                        }
                        let total = scene.objects.iter().filter(in_cat).count();
                        let done = scene.objects.iter().filter(in_cat).filter(|o| o.collected).count();

                        // Collapsible category header with the ObjectType icon.
                        let id = ui.make_persistent_id(("objcat", game.idx(), ty as usize));
                        let mut open = if !query.is_empty() {
                            true
                        } else if let Some(v) = obj_set_all {
                            ui.data_mut(|d| d.insert_persisted(id, v));
                            v
                        } else {
                            ui.data_mut(|d| d.get_persisted::<bool>(id)).unwrap_or(true)
                        };
                        let cat_tex = data::ICON_PATHS
                            .get(ty as usize)
                            .copied()
                            .filter(|p| !p.is_empty())
                            .and_then(|p| self.icon_cache.get(p))
                            .and_then(|t| t.as_ref())
                            .map(|t| t.id());
                        let arrow = if open { "▾" } else { "▸" };
                        let header = tinted_row(
                            ui, 22.0, 40.0, accent.linear_multiply(0.22),
                            scene::type_label(ty), Color32::from_rgb(230, 240, 255),
                            Some((done, total)), Some(arrow), cat_tex,
                        );
                        if header.clicked() {
                            open = !open;
                            ui.data_mut(|d| d.insert_persisted(id, open));
                        }

                        if open {
                            for o in objs {
                                // Manually-forced = violet, auto-collected = green,
                                // outstanding = light (mirrors the map tint).
                                let forced = self.forced.contains(&(game, o.index));
                                let name_col = if forced {
                                    Color32::from_rgb(196, 146, 240)
                                } else if o.collected {
                                    Color32::from_rgb(150, 205, 150)
                                } else {
                                    Color32::from_rgb(222, 230, 242)
                                };
                                // Item held at this location (spoiler), or ??? when unknown.
                                let item =
                                    self.spoiler_items.get(o.location).map(String::as_str).unwrap_or("???");
                                let resp = ui
                                    .horizontal(|ui| {
                                        if let Some(tex) = scene::icon_path_for(o.map_icon, o.type_)
                                            .and_then(|p| self.icon_cache.get(p))
                                            .and_then(|t| t.as_ref())
                                        {
                                            let tint = if o.collected {
                                                Color32::from_gray(120)
                                            } else {
                                                Color32::WHITE
                                            };
                                            ui.add(egui::Image::new((tex.id(), vec2(22.0, 22.0))).tint(tint));
                                        }
                                        ui.vertical(|ui| {
                                            ui.spacing_mut().item_spacing.y = 0.0;
                                            let mark = if o.collected { "☑ " } else { "☐ " };
                                            let r = ui.add(
                                                egui::Label::new(
                                                    egui::RichText::new(format!("{mark}{}", o.name)).color(name_col),
                                                )
                                                .sense(Sense::click()),
                                            );
                                            ui.label(
                                                egui::RichText::new(item).small().color(Color32::from_gray(148)),
                                            );
                                            r
                                        })
                                        .inner
                                    })
                                    .inner;
                                if resp.clicked() {
                                    toggle = Some((game, o.index));
                                }
                            }
                        }
                    }
                });
            });
        if let Some((game, index)) = toggle {
            self.toggle_object(game, index);
        }
    }

    /// Right panel of the entrance tab: the scene's entrances (with per-type
    /// icons) and where each one leads once discovered.
    fn draw_entrance_tree(&mut self, ctx: &egui::Context) {
        egui::SidePanel::right("enttree")
            .resizable(true)
            .default_width(320.0)
            .show(ctx, |ui| {
                accent_heading(ui, "Entrées");
                let Some(scene) = self.scene.as_ref() else { return };
                let (game, sid) = (scene.game, scene.def.id);

                // Entrances of this scene under the active layout, and how many
                // have been walked (header count, like the object tree).
                let entrances: Vec<&'static data::EntranceDef> = game
                    .entrances()
                    .iter()
                    .filter(|e| {
                        e.to_scene == sid
                            && tracking::scene_layout_active(e.layout, game, e.to_scene, &self.mq_scenes)
                    })
                    .collect();
                let visited = entrances
                    .iter()
                    .filter(|e| self.visited_entrances.contains(&(game, e.to_id)))
                    .count();
                ui.label(format!("{visited} / {}", entrances.len()));
                ui.separator();

                egui::ScrollArea::vertical().id_salt("enttree_s").show(ui, |ui| {
                    for e in entrances {
                        let visited = self.visited_entrances.contains(&(game, e.to_id));
                        let dest = self
                            .out_links
                            .get(&(game, e.to_id))
                            .and_then(|&(g, id)| entrance::lookup(g, id));
                        let col = if visited {
                            Color32::from_rgb(120, 210, 120)
                        } else {
                            Color32::from_gray(205)
                        };
                        ui.horizontal(|ui| {
                            // Per-entrance icon (door / grotto / warp / owl…), dimmed
                            // until the entrance has been walked.
                            if let Some(tex) = scene::entrance_icon_path(e.icon)
                                .and_then(|p| self.icon_cache.get(p))
                                .and_then(|t| t.as_ref())
                            {
                                let tint = if visited {
                                    Color32::WHITE
                                } else {
                                    Color32::from_gray(130)
                                };
                                ui.add(egui::Image::new((tex.id(), vec2(18.0, 18.0))).tint(tint));
                            }
                            let mark = if visited { "✓ " } else { "" };
                            match dest {
                                Some(d) => {
                                    ui.colored_label(col, format!("{mark}{}", e.to_name));
                                    ui.weak("→");
                                    ui.colored_label(Color32::from_rgb(150, 190, 230), d.to_name);
                                }
                                None => {
                                    ui.colored_label(col, format!("{mark}{}", e.to_name));
                                }
                            }
                        });
                    }
                });
            });
    }

    /// The progression dashboard (ProgressionTab): four sub-tabs of item-icon
    /// grids on the left, a detail panel with the selected item's location tree
    /// on the right.
    fn draw_progression_tab(&mut self, ctx: &egui::Context) {
        // Recompute the per-entry state when the collected set / spoiler /
        // settings changed (RebuildFromSceneObjects). Disjoint field borrows.
        if self.prog_dirty {
            self.dashboard.rebuild(
                &self.collected,
                &self.spoiler_items,
                &self.spoiler_worlds,
                &self.rom_settings,
                &self.mq_scenes,
            );
            self.prog_dirty = false;
        }

        // Number of worlds (multiworld): the highest destination player, or 1.
        let num_worlds = self.spoiler_worlds.values().copied().max().unwrap_or(1).max(1);

        // Sub-tab selector + reveal toggle (top).
        let mut new_tab = self.dashboard.sub_tab;
        let mut new_world = self.dashboard.active_world;
        egui::TopBottomPanel::top("prog_subtabs").show(ctx, |ui| {
            ui.horizontal(|ui| {
                for (p, page) in data::PROG_PAGES.iter().enumerate() {
                    if !self.dashboard.page_visible(p, &self.rom_settings) {
                        continue; // whole page disabled by the ROM settings
                    }
                    if ui
                        .selectable_label(self.dashboard.sub_tab == p, page.title)
                        .clicked()
                    {
                        new_tab = p;
                    }
                }
                // Multiworld: pick which world's progression to view.
                if num_worlds > 1 {
                    ui.separator();
                    ui.label("Monde :");
                    egui::ComboBox::from_id_salt("prog_world")
                        .selected_text(format!("Joueur {}", self.dashboard.active_world))
                        .show_ui(ui, |ui| {
                            for w in 1..=num_worlds {
                                ui.selectable_value(&mut new_world, w, format!("Joueur {w}"));
                            }
                        });
                }
                ui.separator();
                let mut reveal = self.dashboard.reveal;
                if ui
                    .checkbox(&mut reveal, "Révéler les emplacements non collectés")
                    .changed()
                {
                    self.dashboard.set_reveal(reveal);
                }
            });
        });

        // A world switch recomputes the whole dashboard (different target world).
        if new_world != self.dashboard.active_world {
            self.dashboard.set_active_world(new_world);
            self.dashboard.rebuild(
                &self.collected,
                &self.spoiler_items,
                &self.spoiler_worlds,
                &self.rom_settings,
                &self.mq_scenes,
            );
        }

        // Refresh the cached detail tree (mutably) before the read-only panels.
        self.dashboard.ensure_tree(
            &self.collected,
            &self.spoiler_items,
            &self.spoiler_worlds,
            &self.mq_scenes,
        );

        // Detail panel (right), then the icon grid (center).
        let mut nav: Option<(Game, u16)> = None;
        egui::SidePanel::right("prog_detail")
            .resizable(false)
            .exact_width(300.0)
            .show(ctx, |ui| self.draw_prog_detail(ui, &mut nav));

        let mut new_selected = self.dashboard.selected;
        egui::CentralPanel::default()
            .show(ctx, |ui| self.draw_prog_grid(ui, &mut new_selected));

        // Apply the deferred UI mutations (kept out of the &self render borrows).
        if new_tab != self.dashboard.sub_tab {
            self.dashboard.sub_tab = new_tab;
        }
        self.dashboard.selected = new_selected;
        if let Some((game, scene_id)) = nav {
            self.navigate_to(game, scene_id);
        }
    }

    /// Resolve an EGameIcon variant name to its loaded texture (via ICON_BY_NAME).
    fn prog_icon_tex(&self, icon: &str) -> Option<&egui::TextureHandle> {
        let path = data::ICON_BY_NAME
            .binary_search_by_key(&icon, |&(n, _)| n)
            .ok()
            .map(|i| data::ICON_BY_NAME[i].1)?;
        self.icon_cache.get(path)?.as_ref()
    }

    /// The left grid: each visible section of the active page as a header + a
    /// wrapped row of icon cells (BuildPage). Sets `new_selected` on a click.
    fn draw_prog_grid(&self, ui: &mut egui::Ui, new_selected: &mut Option<usize>) {
        let page = self.dashboard.sub_tab;
        let accent = Color32::from_rgb(74, 158, 219);

        // Group the page's visible entries by section (flat order is page-major,
        // so a section's entries are contiguous). Fully-disabled sections vanish.
        let mut sections: Vec<(&'static str, Vec<usize>)> = Vec::new();
        let mut last_sec: Option<usize> = None;
        for (i, fe) in self.dashboard.flat().iter().enumerate() {
            if fe.page != page || self.dashboard.entry_hidden(i, &self.rom_settings) {
                continue;
            }
            if last_sec != Some(fe.section) {
                sections.push((fe.section_title, Vec::new()));
                last_sec = Some(fe.section);
            }
            sections.last_mut().unwrap().1.push(i);
        }

        egui::ScrollArea::vertical().id_salt("prog_grid").show(ui, |ui| {
            for (title, idxs) in &sections {
                ui.add_space(8.0);
                ui.label(egui::RichText::new(*title).strong().color(accent));
                ui.separator();
                ui.horizontal_wrapped(|ui| {
                    for &i in idxs {
                        self.draw_prog_cell(ui, i, new_selected);
                    }
                });
            }
        });
    }

    /// One icon cell: 44px icon (greyed until obtained), a count badge, the name
    /// below, and a selection outline (RefreshVisual, immediate-mode).
    fn draw_prog_cell(&self, ui: &mut egui::Ui, i: usize, new_selected: &mut Option<usize>) {
        let e = self.dashboard.flat()[i].entry;
        let st = self.dashboard.state(i);
        let complete = self.dashboard.complete(i);
        let accent = Color32::from_rgb(74, 158, 219);
        let cell = vec2(78.0, 92.0);

        let inner = ui.allocate_ui_with_layout(
            cell,
            egui::Layout::top_down(egui::Align::Center),
            |ui| {
                let tint = if complete { Color32::WHITE } else { Color32::from_gray(85) };
                if let Some(tex) = self.prog_icon_tex(e.icon) {
                    ui.add(egui::Image::new((tex.id(), vec2(44.0, 44.0))).tint(tint));
                } else {
                    ui.add_space(44.0);
                }
                if let Some(badge) = self.dashboard.badge_text(i) {
                    ui.label(
                        egui::RichText::new(badge)
                            .small()
                            .strong()
                            .color(Color32::from_rgb(221, 238, 255))
                            .background_color(accent),
                    );
                }
                let name_col = if complete {
                    Color32::from_rgb(221, 238, 255)
                } else {
                    Color32::from_gray(120)
                };
                ui.label(egui::RichText::new(e.name).small().color(name_col));
            },
        );

        let resp = inner.response.interact(Sense::click());
        if resp.clicked() {
            *new_selected = Some(i);
        }
        let mut tip = e.name.to_string();
        if st.is_starting {
            tip.push_str("\n★ Objet de départ");
        }
        let resp = resp.on_hover_text(tip);

        // Selection outline + starting-item corner mark.
        if self.dashboard.selected == Some(i) {
            ui.painter()
                .rect_stroke(resp.rect, 4.0, Stroke::new(1.5_f32, accent));
        }
        if st.is_starting {
            ui.painter().text(
                resp.rect.left_top() + vec2(4.0, 2.0),
                Align2::LEFT_TOP,
                "★",
                FontId::proportional(11.0),
                Color32::from_rgb(248, 200, 120),
            );
        }
    }

    /// The right detail panel (BuildDetailPanel / ShowDetailFor): big icon, name,
    /// found status, counter, and the per-scene location tree.
    fn draw_prog_detail(&self, ui: &mut egui::Ui, nav: &mut Option<(Game, u16)>) {
        ui.add_space(6.0);
        let Some(i) = self.dashboard.selected else {
            ui.weak("Sélectionne un item pour voir ses détails.");
            return;
        };
        let e = self.dashboard.flat()[i].entry;
        let st = self.dashboard.state(i);

        ui.vertical_centered(|ui| {
            let tint = if self.dashboard.complete(i) {
                Color32::WHITE
            } else {
                Color32::from_gray(85)
            };
            if let Some(tex) = self.prog_icon_tex(e.icon) {
                ui.add(egui::Image::new((tex.id(), vec2(64.0, 64.0))).tint(tint));
            }
            ui.label(egui::RichText::new(e.name).heading().size(16.0));
            if st.found {
                ui.colored_label(Color32::from_rgb(101, 224, 154), "✔ Trouvé");
            } else {
                ui.colored_label(Color32::from_rgb(255, 105, 105), "✘ Pas trouvé");
            }
            if st.is_starting {
                ui.colored_label(Color32::from_rgb(248, 200, 120), "★ Objet de départ");
            }
            if let Some(badge) = self.dashboard.badge_text(i) {
                ui.label(
                    egui::RichText::new(badge)
                        .size(20.0)
                        .strong()
                        .color(Color32::from_rgb(74, 158, 219)),
                );
            }
        });

        ui.add_space(6.0);
        ui.label(egui::RichText::new("Emplacements").strong().color(Color32::from_rgb(74, 158, 219)));
        ui.separator();

        let tree = self.dashboard.tree();
        egui::ScrollArea::vertical().id_salt("prog_locs").show(ui, |ui| {
            if tree.is_empty() {
                ui.weak(if self.dashboard.reveal {
                    "Aucun emplacement connu."
                } else {
                    "Pas encore trouvé."
                });
                return;
            }
            let mut cur_game: Option<Game> = None;
            for scene in tree {
                if cur_game != Some(scene.game) {
                    cur_game = Some(scene.game);
                    let col = if scene.game == Game::Oot {
                        Color32::from_rgb(74, 158, 219)
                    } else {
                        Color32::from_rgb(155, 93, 229)
                    };
                    ui.add_space(4.0);
                    ui.label(egui::RichText::new(scene.game.label()).strong().color(col));
                }
                egui::CollapsingHeader::new(format!("{} ({})", scene.title, scene.leaves.len()))
                    .id_salt((scene.game.idx(), scene.title.as_str()))
                    .default_open(true)
                    .show(ui, |ui| {
                        for leaf in &scene.leaves {
                            let mut txt = egui::RichText::new(leaf.name).small();
                            txt = if leaf.collected {
                                txt.strikethrough().color(Color32::from_gray(140))
                            } else {
                                txt.color(Color32::from_rgb(230, 240, 255))
                            };
                            if ui.selectable_label(false, txt).clicked() {
                                *nav = Some((leaf.game, leaf.render_scene));
                            }
                        }
                    });
            }
        });
    }

    /// The global entrance table for a region (Qt AllEntranceView): one row per
    /// entrance of the region's scenes — Scene / Entrance / How to spawn here? /
    /// Where does it lead? — with a red/yellow/green status and clickable rows
    /// that load the scene and centre the minimap on the entrance.
    fn draw_entrance_table(&mut self, ctx: &egui::Context, game: Game, region: u8) {
        use data::EntranceType as ET;
        let red = Color32::from_rgb(200, 90, 90);
        let yellow = Color32::from_rgb(210, 190, 90);
        let green = Color32::from_rgb(120, 200, 120);
        let na = Color32::from_gray(110);

        // Rows: every active-layout entrance whose destination scene is in the
        // region (or all of them for the "All" node). Type None entrances are
        // NOT skipped — end-game / special areas (Ganon's Castle, Sages Chamber…)
        // are all type None yet perfectly valid, so filtering them emptied whole
        // regions. We only drop the truly nameless placeholder rows.
        let all = region == ALL_REGION;
        let rows: Vec<&'static data::EntranceDef> = game
            .entrances()
            .iter()
            .filter(|e| {
                !e.to_name.is_empty()
                    && (all
                        || game
                            .scenes()
                            .iter()
                            .find(|s| s.id == e.to_scene)
                            .is_some_and(|s| s.region_id == region))
                    && tracking::scene_layout_active(e.layout, game, e.to_scene, &self.mq_scenes)
            })
            .collect();

        let region_name = if all {
            match game {
                Game::Oot => "Toutes les entrées — Ocarina of Time",
                Game::Mm => "Toutes les entrées — Majora's Mask",
            }
        } else {
            game.scenes()
                .iter()
                .find(|s| s.region_id == region)
                .map(|s| s.region_name)
                .unwrap_or("—")
        };

        // Precompute the display + navigation data for each row (so sorting and
        // rendering don't re-touch self / the link maps).
        let link_cell = |na_side: bool, target: Option<(Game, u32)>| -> EntCell {
            if na_side {
                EntCell::Na
            } else if let Some((g, id)) = target {
                match entrance::lookup(g, id) {
                    Some(d) => EntCell::Link(d.to_name, (g, d.to_scene, id)),
                    None => EntCell::Unknown,
                }
            } else {
                EntCell::Unknown
            }
        };
        let mut data: Vec<EntRow> = rows
            .iter()
            .map(|e| {
                let key = (game, e.to_id);
                let in_na = e.type_ == ET::One_Way_Out;
                let out_na = e.type_ == ET::One_Way_In;
                let src = self.in_links.get(&key).and_then(|v| v.last().copied());
                let dst = self.out_links.get(&key).copied();
                let applicable = (!in_na) as u8 + (!out_na) as u8;
                let known = (!in_na && src.is_some()) as u8 + (!out_na && dst.is_some()) as u8;
                let dot = if applicable > 0 && known == applicable {
                    green
                } else if known > 0 {
                    yellow
                } else {
                    red
                };
                let scene = game.scenes().iter().find(|s| s.id == e.to_scene).map(|s| s.name).unwrap_or("?");
                EntRow {
                    scene,
                    entrance: e.to_name,
                    ent_target: (game, e.to_scene, e.to_id),
                    spawn: link_cell(in_na, src),
                    leads: link_cell(out_na, dst),
                    dot,
                }
            })
            .collect();

        // Sort by the active column (Scene / Entrance / spawn / leads).
        let sort_col = self.ent_sort_col;
        let asc = self.ent_sort_asc;
        let sort_text = |r: &EntRow, c: usize| -> String {
            match c {
                0 => r.scene,
                1 => r.entrance,
                2 => r.spawn.text(),
                _ => r.leads.text(),
            }
            .to_lowercase()
        };
        data.sort_by(|a, b| {
            let o = sort_text(a, sort_col).cmp(&sort_text(b, sort_col));
            if asc { o } else { o.reverse() }
        });

        let frac = self.ent_col_frac;
        let stripe = Color32::from_white_alpha(8);
        let sep = Color32::from_gray(78);
        let n = data.len();
        let mut focus: Option<(Game, u16, u32)> = None;
        let mut sort_click: Option<usize> = None;
        let mut resize: Option<(usize, f32)> = None;

        egui::CentralPanel::default().show(ctx, |ui| {
            ui.horizontal(|ui| {
                accent_heading(ui, region_name);
                ui.label(egui::RichText::new(format!("· {n} entrées")).weak());
            });
            ui.separator();

            egui::ScrollArea::vertical().id_salt("enttable").show(ui, |ui| {
                // Fixed status column + Scene/Entrance/spawn/leads sharing the rest
                // by the (resizable) fractions; 3 four-px drag handles between them.
                let avail = ui.available_width();
                let rest = (avail - 26.0 - 12.0).max(240.0);
                let ws = [26.0_f32, rest * frac[0], rest * frac[1], rest * frac[2], rest * frac[3]];
                let headers = ["Scène", "Entrée", "Comment y arriver ?", "Où mène-t-elle ?"];

                // Header row: clickable sort labels + drag-to-resize handles.
                ui.allocate_ui_with_layout(vec2(avail, 24.0), egui::Layout::left_to_right(egui::Align::Center), |ui| {
                    ui.spacing_mut().item_spacing.x = 0.0;
                    table_cell(ui, ws[0], |_| {});
                    for c in 0..4 {
                        let arrow = if sort_col == c {
                            if asc { " ▲" } else { " ▼" }
                        } else {
                            ""
                        };
                        let label = format!("{}{arrow}", headers[c]);
                        let r = ui
                            .allocate_ui_with_layout(vec2(ws[c + 1], 24.0), egui::Layout::left_to_right(egui::Align::Center), |ui| {
                                ui.add(
                                    egui::Label::new(egui::RichText::new(label).strong().color(ACCENT))
                                        .truncate()
                                        .sense(Sense::click()),
                                )
                            })
                            .inner;
                        if r.clicked() {
                            sort_click = Some(c);
                        }
                        if c < 3 {
                            let (hr, hresp) = ui.allocate_exact_size(vec2(12.0, 24.0), Sense::drag());
                            ui.painter().vline(hr.center().x, hr.y_range(), Stroke::new(1.0_f32, sep));
                            if hresp.hovered() || hresp.dragged() {
                                ui.ctx().set_cursor_icon(egui::CursorIcon::ResizeHorizontal);
                            }
                            if hresp.dragged() {
                                resize = Some((c, hresp.drag_delta().x));
                            }
                        }
                    }
                });
                ui.separator();

                // Column separator x-offsets (for the faint per-row vlines).
                let seps = [ws[0] + 6.0, ws[0] + ws[1] + 12.0 + 6.0, ws[0] + ws[1] + ws[2] + 24.0 + 6.0];

                for (i, r) in data.iter().enumerate() {
                    let row_rect = Rect::from_min_size(ui.cursor().min, vec2(avail, 22.0));
                    if i % 2 == 1 {
                        ui.painter().rect_filled(row_rect, 2.0, stripe);
                    }
                    for sx in seps {
                        ui.painter().vline(
                            row_rect.min.x + sx,
                            row_rect.y_range(),
                            Stroke::new(1.0_f32, Color32::from_gray(58)),
                        );
                    }
                    let clicked = ui
                        .allocate_ui_with_layout(vec2(avail, 22.0), egui::Layout::left_to_right(egui::Align::Center), |ui| {
                            ui.spacing_mut().item_spacing.x = 0.0;
                            let mut f = None;
                            table_cell(ui, ws[0], |ui| {
                                ui.colored_label(r.dot, "●");
                            });
                            table_cell(ui, ws[1], |ui| {
                                ui.add(egui::Label::new(r.scene).truncate());
                            });
                            if table_link_cell(ui, ws[2], r.entrance).clicked() {
                                f = Some(r.ent_target);
                            }
                            for (cell, w) in [(&r.spawn, ws[3]), (&r.leads, ws[4])] {
                                match cell {
                                    EntCell::Na => table_cell(ui, w, |ui| {
                                        ui.colored_label(na, "N/A");
                                    }),
                                    EntCell::Unknown => table_cell(ui, w, |ui| {
                                        ui.weak("?");
                                    }),
                                    EntCell::Link(name, target) => {
                                        if table_link_cell(ui, w, name).clicked() {
                                            f = Some(*target);
                                        }
                                    }
                                }
                            }
                            f
                        })
                        .inner;
                    if let Some(fc) = clicked {
                        focus = Some(fc);
                    }
                }
            });
        });

        // Apply the deferred header actions.
        if let Some(c) = sort_click {
            if self.ent_sort_col == c {
                self.ent_sort_asc = !self.ent_sort_asc;
            } else {
                self.ent_sort_col = c;
                self.ent_sort_asc = true;
            }
        }
        if let Some((c, dx)) = resize {
            let rest = (ctx.available_rect().width() - 300.0).max(240.0);
            let df = dx / rest;
            let a = (self.ent_col_frac[c] + df).clamp(0.08, 0.72);
            let b = (self.ent_col_frac[c + 1] - df).clamp(0.08, 0.72);
            if (a - self.ent_col_frac[c]).abs() < 0.5 && (b - self.ent_col_frac[c + 1]).abs() < 0.5 {
                self.ent_col_frac[c] = a;
                self.ent_col_frac[c + 1] = b;
            }
        }
        if let Some((g, scene_id, eid)) = focus {
            self.focus_entrance_in_scene(g, scene_id, eid);
        }
    }

    /// Load a scene's minimap on the Entrance tab and queue a recentre on the
    /// given entrance (Qt FocusEntranceInGame).
    fn focus_entrance_in_scene(&mut self, game: Game, scene_id: u16, entrance_id: u32) {
        if self.scene.as_ref().map_or(true, |s| s.game != game || s.def.id != scene_id) {
            if let Some(def) = game.scenes().iter().find(|s| s.id == scene_id) {
                self.scene = Some(LiveScene::load(game, def, &self.mq_scenes));
                self.current_room = 0;
                self.map_texture = None;
                self.load_error = None;
            }
        }
        self.sel_scene[game.idx()] = scene_id;
        self.entrance_table = None; // switch the centre from the table to the minimap
        self.focus_entrance = Some(entrance_id);
        self.view_initialized = false; // let draw_map recentre on the entrance
    }

    /// Navigate from a location leaf to its object's scene (switch to the game's
    /// tab and load the render scene), mirroring NavigateToObject.
    fn navigate_to(&mut self, game: Game, scene_id: u16) {
        self.active_tab = if game == Game::Oot { Tab::Oot } else { Tab::Mm };
        self.sel_scene[game.idx()] = scene_id;
        if let Some(def) = game.scenes().iter().find(|s| s.id == scene_id) {
            self.scene = Some(LiveScene::load(game, def, &self.mq_scenes));
            self.current_room = 0;
            self.map_texture = None;
            self.load_error = None;
            self.view_initialized = false;
        }
    }


    /// Panneau gauche : arbre jeux -> régions -> scènes, façon Qt (lignes teintées
    /// par jeu, compteurs collectés/total, régions repliables).
    fn draw_nav(&mut self, ctx: &egui::Context) {
        let mut clicked: Option<(Game, &'static data::SceneDef)> = None;
        let mut region_selected: Option<(Game, u8)> = None;
        let mut set_all: Option<bool> = None; // "expand/collapse all" this frame
        let entrance_tab = self.active_tab.is_entrance();

        egui::SidePanel::left("nav")
            .resizable(true)
            .default_width(260.0)
            .show(ctx, |ui| {
                accent_heading(ui, "Scènes");
                // Live scene filter (Qt "Find…").
                ui.add(
                    egui::TextEdit::singleline(&mut self.scene_search)
                        .hint_text("Find…")
                        .desired_width(f32::INFINITY),
                );
                // Expand / collapse every region at once (Qt "Expand All").
                let label = if self.nav_all_expanded { "⊟ Réduire tout" } else { "⊞ Développer tout" };
                if ui.add(egui::Button::new(label).min_size(vec2(ui.available_width(), 0.0))).clicked() {
                    self.nav_all_expanded = !self.nav_all_expanded;
                    set_all = Some(self.nav_all_expanded);
                }
                ui.separator();
                let query = self.scene_search.trim().to_lowercase();
                // OoT/MM tabs show only their game; the Entrance tab shows the
                // active sub-tab's game (OoT / MM).
                let games: &[Game] = match self.active_tab {
                    Tab::Oot => &[Game::Oot],
                    Tab::Mm => &[Game::Mm],
                    Tab::Entrance => match self.entrance_sub {
                        EntranceSub::Mm => &[Game::Mm],
                        _ => &[Game::Oot],
                    },
                    _ => &[Game::Oot, Game::Mm],
                };
                egui::ScrollArea::vertical().show(ui, |ui| {
                    ui.spacing_mut().item_spacing.y = 2.0;
                    for &game in games {
                        let accent = game_accent(game);
                        let sel_bg = game_selection(game);
                        let region_bg = accent.linear_multiply(0.25); // Qt alpha 64
                        let scene_bg = accent.linear_multiply(0.06); //  Qt alpha 16
                        let cur_id = self
                            .scene
                            .as_ref()
                            .filter(|s| s.game == game)
                            .map(|s| s.def.id);
                        let scenes = game.scenes();
                        let counts = &self.cached_scene_counts[game.idx()];

                        // Game header (only when both games share the panel).
                        if games.len() > 1 {
                            ui.add_space(4.0);
                            ui.label(egui::RichText::new(game.label()).strong().size(15.0).color(accent));
                        }

                        // Entrance tab: an "All" node (no children) that opens the
                        // full global entrance table for the whole game.
                        if entrance_tab {
                            let sel = matches!(self.entrance_table, Some((g, ALL_REGION)) if g == game);
                            let bg = if sel { sel_bg } else { region_bg };
                            let row = tinted_row(
                                ui, 22.0, 22.0, bg, "Toutes les entrées",
                                Color32::from_rgb(230, 240, 255), None, None, None,
                            );
                            if row.clicked() {
                                region_selected = Some((game, ALL_REGION));
                            }
                        }

                        // Regions, sorted alphabetically by name (not by region id).
                        // Region 0 ("None") holds scene-less technical maps that the
                        // Qt tracker also hides (MapTab: `ParentRegion != 0`).
                        let mut regions: Vec<u8> = Vec::new();
                        for s in scenes {
                            if s.region_id != 0 && !regions.contains(&s.region_id) {
                                regions.push(s.region_id);
                            }
                        }
                        let region_name_of = |rid: u8| {
                            scenes.iter().find(|s| s.region_id == rid).map(|s| s.region_name).unwrap_or("")
                        };
                        regions.sort_by(|&a, &b| {
                            region_name_of(a).to_lowercase().cmp(&region_name_of(b).to_lowercase())
                        });
                        for rid in regions {
                            // Filter scenes by the search text; skip empty regions.
                            let region_scenes: Vec<&'static data::SceneDef> = scenes
                                .iter()
                                .filter(|s| {
                                    s.region_id == rid
                                        && (query.is_empty() || s.name.to_lowercase().contains(&query))
                                })
                                .collect();
                            if region_scenes.is_empty() {
                                continue;
                            }
                            let rname = region_scenes.first().map(|s| s.region_name).unwrap_or("—");
                            // Region count = sum of its scenes' counts.
                            let rcount = region_scenes.iter().fold((0, 0), |(d, t), s| {
                                let (sd, st) = counts.get(&s.id).copied().unwrap_or((0, 0));
                                (d + sd, t + st)
                            });

                            let id = ui.make_persistent_id(("navreg", game.idx(), rid));
                            // A search forces every region open; "expand/collapse
                            // all" overrides + persists; else the remembered state.
                            let mut open = if !query.is_empty() {
                                true
                            } else if let Some(v) = set_all {
                                ui.data_mut(|d| d.insert_persisted(id, v));
                                v
                            } else {
                                ui.data_mut(|d| d.get_persisted::<bool>(id)).unwrap_or(true)
                            };

                            // Region icon (Regions.h), loaded into the shared cache.
                            let region_tex = scene::region_icon(game, rid)
                                .and_then(|p| self.icon_cache.get(p))
                                .and_then(|t| t.as_ref())
                                .map(|t| t.id());
                            let arrow = if open { "▾" } else { "▸" };
                            let header = tinted_row(
                                ui, 22.0, 40.0, region_bg, rname,
                                Color32::from_rgb(230, 240, 255), Some(rcount), Some(arrow), region_tex,
                            );
                            // Region separator (Qt: faint white line under a region row).
                            ui.painter().line_segment(
                                [header.rect.left_bottom(), header.rect.right_bottom()],
                                Stroke::new(1.0_f32, Color32::from_white_alpha(24)),
                            );
                            if header.clicked() {
                                open = !open;
                                ui.data_mut(|d| d.insert_persisted(id, open));
                                // On the Entrance tab, selecting a region shows its
                                // global entrance table in the centre.
                                if entrance_tab {
                                    region_selected = Some((game, rid));
                                }
                            }

                            if open {
                                for s in region_scenes {
                                    let selected = cur_id == Some(s.id);
                                    let bg = if selected { sel_bg } else { scene_bg };
                                    let scount = counts.get(&s.id).copied();
                                    let row = tinted_row(
                                        ui, 20.0, 30.0, bg, s.name,
                                        Color32::from_rgb(214, 224, 238), scount, None, None,
                                    );
                                    if row.clicked() {
                                        clicked = Some((game, s));
                                    }
                                }
                            }
                        }
                        ui.add_space(2.0);
                    }
                });
            });

        if let Some((game, def)) = clicked {
            self.select_scene(game, def);
            self.entrance_table = None; // a scene switches the centre back to its map
        }
        if let Some(gr) = region_selected {
            self.entrance_table = Some(gr);
        }
    }

    /// (visited, total) entrances of a game (active layout, named), for the sub-tab
    /// counters. Visited = discovered live (`visited_entrances`).
    fn entrance_counts(&self, game: Game) -> (usize, usize) {
        let total = game
            .entrances()
            .iter()
            .filter(|e| {
                !e.to_name.is_empty()
                    && tracking::scene_layout_active(e.layout, game, e.to_scene, &self.mq_scenes)
            })
            .count();
        let visited = self.visited_entrances.iter().filter(|(g, _)| *g == game).count();
        (visited, total)
    }

    /// The Entrance tab's OoT / MM / GPS sub-tab bar (Qt EntranceTab).
    fn draw_entrance_subtabs(&mut self, ctx: &egui::Context) {
        let oot = self.entrance_counts(Game::Oot);
        let mm = self.entrance_counts(Game::Mm);
        egui::TopBottomPanel::top("entrance_subtabs").show(ctx, |ui| {
            ui.add_space(1.0);
            ui.horizontal(|ui| {
                ui.spacing_mut().item_spacing.x = 6.0;
                for (sub, text) in [
                    (EntranceSub::Oot, format!("OoT  {}/{}", oot.0, oot.1)),
                    (EntranceSub::Mm, format!("MM  {}/{}", mm.0, mm.1)),
                    (EntranceSub::Gps, "GPS".to_string()),
                ] {
                    let selected = self.entrance_sub == sub;
                    let col = if selected {
                        Color32::from_rgb(221, 238, 255)
                    } else {
                        Color32::from_gray(185)
                    };
                    if ui.selectable_label(selected, egui::RichText::new(text).color(col)).clicked() {
                        self.entrance_sub = sub;
                    }
                }
            });
            ui.add_space(1.0);
        });
    }

    /// GPS sub-tab: pick a game, a start and a destination scene, and show the
    /// shortest discovered route between them.
    fn draw_gps_page(&mut self, ctx: &egui::Context) {
        egui::CentralPanel::default().show(ctx, |ui| {
            accent_heading(ui, "GPS — itinéraire");
            ui.separator();
            ui.horizontal(|ui| {
                ui.label("Jeu :");
                if ui.selectable_label(self.gps_game == Game::Oot, "OoT").clicked() {
                    self.gps_game = Game::Oot;
                }
                if ui.selectable_label(self.gps_game == Game::Mm, "MM").clicked() {
                    self.gps_game = Game::Mm;
                }
            });
            let game = self.gps_game;
            let scene_name = |t: Option<u16>| {
                t.and_then(|id| game.scenes().iter().find(|s| s.id == id))
                    .map(|s| s.name)
                    .unwrap_or("(choisir)")
            };
            ui.horizontal(|ui| {
                ui.label("Départ  :");
                egui::ComboBox::from_id_salt("gps_from").width(240.0)
                    .selected_text(scene_name(self.gps_from))
                    .show_ui(ui, |ui| {
                        for s in game.scenes() {
                            if ui.selectable_label(self.gps_from == Some(s.id), s.name).clicked() {
                                self.gps_from = Some(s.id);
                            }
                        }
                    });
            });
            ui.horizontal(|ui| {
                ui.label("Arrivée :");
                egui::ComboBox::from_id_salt("gps_to").width(240.0)
                    .selected_text(scene_name(self.gps_target))
                    .show_ui(ui, |ui| {
                        for s in game.scenes() {
                            if ui.selectable_label(self.gps_target == Some(s.id), s.name).clicked() {
                                self.gps_target = Some(s.id);
                            }
                        }
                    });
            });
            ui.separator();

            let (Some(from), Some(to)) = (self.gps_from, self.gps_target) else {
                ui.weak("Choisis un départ et une arrivée.");
                return;
            };
            match gps::route(game, from, to, &self.out_links) {
                Some(steps) if steps.is_empty() => {
                    ui.label("Déjà sur place.");
                }
                Some(steps) => {
                    ui.label(egui::RichText::new(format!("{} étapes :", steps.len())).strong());
                    egui::ScrollArea::vertical().id_salt("gps_route").show(ui, |ui| {
                        for (i, st) in steps.iter().enumerate() {
                            let n = game
                                .scenes()
                                .iter()
                                .find(|s| s.id == st.scene)
                                .map(|s| s.name)
                                .unwrap_or("?");
                            ui.label(format!("{}. → {n}  (entrée {})", i + 1, st.entrance));
                        }
                    });
                }
                None => {
                    ui.colored_label(
                        Color32::from_rgb(220, 120, 90),
                        "Aucun chemin connu (explore plus d'entrées).",
                    );
                }
            }
        });
    }

    fn draw_map(&mut self, ctx: &egui::Context) {
        egui::CentralPanel::default().show(ctx, |ui| {
            let entrance_view = self.active_tab.is_entrance();

            // Age/season toggle (Qt ContextSwitchButton) + room selector, item view.
            // Copy the flags / room names out of the scene borrow first so the
            // header closure only touches plain fields.
            let mut change_room = None;
            let header = (!entrance_view).then(|| self.scene.as_ref()).flatten().map(|s| {
                (s.def.has_context, s.game, s.rooms.iter().map(|r| r.name).collect::<Vec<_>>())
            });
            if let Some((has_context, game, room_names)) = header {
                if has_context || !room_names.is_empty() {
                    ui.horizontal(|ui| {
                        if has_context {
                            let (off, on) = match game {
                                Game::Oot => ("Enfant", "Adulte"),
                                Game::Mm => ("Hiver", "Printemps"),
                            };
                            if ui.selectable_label(!self.context_toggle, off).clicked() {
                                self.context_toggle = false;
                            }
                            if ui.selectable_label(self.context_toggle, on).clicked() {
                                self.context_toggle = true;
                            }
                        }
                        if has_context && !room_names.is_empty() {
                            ui.separator();
                        }
                        if !room_names.is_empty() {
                            ui.label("Salle :");
                            for (k, name) in room_names.iter().enumerate() {
                                if ui.selectable_label(self.current_room == k, *name).clicked() {
                                    change_room = Some(k);
                                }
                            }
                        }
                    });
                    ui.separator();
                }
            }
            if let Some(k) = change_room {
                self.current_room = k;
                self.map_texture = None; // different room image
                self.view_initialized = false;
            }

            if let Some(err) = &self.load_error {
                ui.centered_and_justified(|ui| {
                    ui.colored_label(Color32::from_rgb(200, 160, 90), err);
                });
                return;
            }
            // Snapshot the active category set + effective context before borrowing
            // the scene mutably.
            let active_types: HashSet<data::ObjectType> = self
                .scene
                .as_ref()
                .map(|s| self.active_types[s.game.idx()].clone())
                .unwrap_or_default();
            let eff_ctx = self.effective_context();
            let (Some(tex), Some(scene)) = (self.map_texture.clone(), self.scene.as_mut()) else {
                ui.centered_and_justified(|ui| ui.weak("Sélectionne une scène à gauche…"));
                return;
            };
            // RoomID currently displayed (item view only; None otherwise).
            let room_id = if entrance_view {
                None
            } else {
                scene.active_room_id(self.current_room)
            };

            let (resp, painter) =
                ui.allocate_painter(ui.available_size(), Sense::click_and_drag());
            let rect = resp.rect;
            let painter = painter.with_clip_rect(rect);
            painter.rect_filled(rect, 0.0, Color32::from_gray(20));

            // Ajustement initial : image centrée et "fit", ou recentrage zoomé sur
            // une entrée demandée depuis la table (Qt FocusEntranceInGame).
            if !self.view_initialized && self.map_size.x > 0.0 {
                let fit = (rect.width() / self.map_size.x).min(rect.height() / self.map_size.y);
                let focus = if entrance_view {
                    self.focus_entrance.take().and_then(|fid| {
                        scene
                            .game
                            .entrances()
                            .iter()
                            .find(|e| e.to_id == fid && e.to_scene == scene.def.id)
                    })
                } else {
                    None
                };
                if let Some(e) = focus {
                    self.zoom = (fit * 2.5).clamp(0.3, 4.0);
                    let anchor = vec2(e.anchor[0] as f32, e.anchor[1] as f32);
                    self.pan = rect.size() * 0.5 - anchor * self.zoom;
                } else {
                    self.zoom = fit;
                    self.pan = (rect.size() - self.map_size * self.zoom) * 0.5;
                }
                self.view_initialized = true;
            }

            if resp.dragged() {
                self.pan += resp.drag_delta();
            }
            if resp.hovered() {
                let scroll = ui.input(|i| i.raw_scroll_delta.y);
                if scroll != 0.0 {
                    let old = self.zoom;
                    self.zoom = (old * (scroll * 0.0015).exp()).clamp(0.03, 10.0);
                    if let Some(p) = resp.hover_pos() {
                        let anchor = (p - rect.min - self.pan) / old;
                        self.pan = (p - rect.min) - anchor * self.zoom;
                    }
                }
            }

            let img_min = rect.min + self.pan;
            let img_rect = Rect::from_min_size(img_min, self.map_size * self.zoom);
            painter.image(
                tex.id(),
                img_rect,
                Rect::from_min_max(pos2(0.0, 0.0), pos2(1.0, 1.0)),
                Color32::WHITE,
            );

            let radius = 9.0_f32;

            // Clic : toggle le marqueur le plus proche (vue Objets uniquement).
            if resp.clicked() && !entrance_view {
                if let Some(cp) = resp.interact_pointer_pos() {
                    let mut best = f32::MAX;
                    let mut best_i = None;
                    for (i, obj) in scene.objects.iter().enumerate() {
                        if !context_allows(eff_ctx, obj.context)
                            || !active_types.contains(&obj.type_)
                            || self.excluded.contains(scene.game, obj.index)
                            || room_id.is_some_and(|rid| obj.room as u32 != rid)
                        {
                            continue;
                        }
                        let c = img_min + vec2(obj.x, obj.y) * self.zoom;
                        let d = (cp - c).length();
                        if d <= radius + 2.0 && d < best {
                            best = d;
                            best_i = Some(i);
                        }
                    }
                    if let Some(i) = best_i {
                        // Toggle collected + forced (violet); mirror onto the marker.
                        let key = (scene.game, scene.objects[i].index);
                        let now = !self.collected.contains(&key);
                        if now {
                            self.collected.insert(key);
                            self.forced.insert(key);
                        } else {
                            self.collected.remove(&key);
                            self.forced.remove(&key);
                        }
                        scene.objects[i].collected = now;
                        self.dirty = true;
                        self.prog_dirty = true;
                        self.counts_dirty = true;
                    }
                }
            }

            let hover = resp.hover_pos();
            let mut hovered: Option<&scene::LiveObject> = None;
            let uv_full = Rect::from_min_max(pos2(0.0, 0.0), pos2(1.0, 1.0));

            // Objects are drawn in the item view only.
            let objects: &[scene::LiveObject] = if entrance_view { &[] } else { &scene.objects };
            for obj in objects {
                if !context_allows(eff_ctx, obj.context)
                    || !active_types.contains(&obj.type_)
                    || self.excluded.contains(scene.game, obj.index)
                    || room_id.is_some_and(|rid| obj.room as u32 != rid)
                {
                    continue;
                }
                // Manually-forced objects render in violet (ObjectState::Forced),
                // distinct from auto-collected (dimmed) ones.
                let forced = self.forced.contains(&(scene.game, obj.index));
                let c = img_min + vec2(obj.x, obj.y) * self.zoom;
                let [r, g, b] = scene::color_for(obj.type_);
                if let Some(hp) = hover {
                    if (hp - c).length() <= radius + 2.0 {
                        hovered = Some(obj);
                    }
                }
                // Real per-object icon (MapIcon) if available, else the fallback.
                let icon = scene::icon_path_for(obj.map_icon, obj.type_)
                    .and_then(|p| self.icon_cache.get(p))
                    .and_then(|o| o.as_ref());
                if let Some(tex) = icon {
                    // Per-type size (Icons.cpp), scaled with the view like Qt.
                    let isz = (scene::icon_render_size(obj.type_) * self.zoom).clamp(12.0, 60.0);
                    let ir = Rect::from_center_size(c, Vec2::splat(isz));
                    let tint = if forced {
                        Color32::from_rgb(178, 120, 232) // violet (forced)
                    } else if obj.collected {
                        Color32::from_rgba_unmultiplied(255, 255, 255, 70)
                    } else {
                        Color32::WHITE
                    };
                    painter.image(tex.id(), ir, uv_full, tint);
                    if obj.collected {
                        painter.text(
                            c,
                            Align2::CENTER_CENTER,
                            "✓",
                            FontId::proportional(14.0),
                            if forced {
                                Color32::from_rgb(210, 170, 245)
                            } else {
                                Color32::from_rgb(120, 230, 120)
                            },
                        );
                    }
                } else if obj.collected {
                    let ring = if forced {
                        Color32::from_rgb(178, 120, 232)
                    } else {
                        Color32::from_rgba_unmultiplied(r, g, b, 110)
                    };
                    painter.circle_stroke(c, radius, Stroke::new(2.0_f32, ring));
                    painter.text(
                        c,
                        Align2::CENTER_CENTER,
                        "✓",
                        FontId::proportional(12.0),
                        Color32::from_rgba_unmultiplied(230, 230, 230, 150),
                    );
                } else {
                    painter.circle_filled(c, radius, Color32::from_rgb(r, g, b));
                    painter.circle_stroke(c, radius, Stroke::new(1.5_f32, Color32::BLACK));
                    painter.text(
                        c,
                        Align2::CENTER_CENTER,
                        scene::glyph_for(obj.type_),
                        FontId::proportional(11.0),
                        Color32::BLACK,
                    );
                }
            }

            // Entrances are drawn on the minimap in the entrance view.
            let mut hovered_entrance: Option<&data::EntranceDef> = None;
            if entrance_view {
                for e in scene.game.entrances() {
                    if e.to_scene != scene.def.id
                        || !tracking::scene_layout_active(e.layout, scene.game, e.to_scene, &self.mq_scenes)
                    {
                        continue;
                    }
                    let c = img_min + vec2(e.anchor[0] as f32, e.anchor[1] as f32) * self.zoom;
                    if hover.is_some_and(|hp| (hp - c).length() <= radius + 2.0) {
                        hovered_entrance = Some(e);
                    }
                    let d = radius;
                    let pts = vec![
                        c + vec2(0.0, -d),
                        c + vec2(d, 0.0),
                        c + vec2(0.0, d),
                        c + vec2(-d, 0.0),
                    ];
                    // Green once the entrance has been visited live, cyan otherwise.
                    let fill = if self.visited_entrances.contains(&(scene.game, e.to_id)) {
                        Color32::from_rgb(120, 210, 120)
                    } else {
                        Color32::from_rgb(90, 200, 220)
                    };
                    painter.add(egui::Shape::convex_polygon(
                        pts,
                        fill,
                        Stroke::new(1.5_f32, Color32::BLACK),
                    ));
                }
            }

            // Overlay d'aide / nom survolé (objet, puis entrée, sinon aide).
            let corner = rect.min + vec2(8.0, 8.0);
            if let Some(o) = hovered {
                let text = match self.spoiler_items.get(o.location) {
                    Some(item) => match self.spoiler_worlds.get(o.location) {
                        Some(w) => format!("{}  ·  {}  →  {item} (Player {w})", o.name, o.location),
                        None => format!("{}  ·  {}  →  {item}", o.name, o.location),
                    },
                    None => format!("{}  ·  {}", o.name, o.location),
                };
                draw_label(&painter, corner, &text, Color32::from_rgb(240, 220, 150));
            } else if let Some(e) = hovered_entrance {
                let dest = self
                    .out_links
                    .get(&(scene.game, e.to_id))
                    .and_then(|&(g, id)| entrance::lookup(g, id));
                let text = match dest {
                    Some(d) => format!("Entrée : {}  ↔  {}", e.to_name, d.to_name),
                    None => format!("Entrée : {}  ←  {}", e.to_name, e.from_name),
                };
                draw_label(&painter, corner, &text, Color32::from_rgb(140, 220, 235));
            } else {
                draw_label(
                    &painter,
                    corner,
                    "Glisser · Molette · Clic = collecter · Déposez un spoiler log",
                    Color32::from_gray(200),
                );
            }
        });
    }
}

/// Dessine un petit label avec fond semi-opaque.
fn draw_label(painter: &egui::Painter, pos: egui::Pos2, text: &str, color: Color32) {
    let galley = painter.layout_no_wrap(text.to_owned(), FontId::proportional(13.0), color);
    let pad = vec2(6.0, 3.0);
    let bg = Rect::from_min_size(pos, galley.size() + pad * 2.0);
    painter.rect_filled(bg, 4.0, Color32::from_rgba_unmultiplied(0, 0, 0, 170));
    painter.galley(pos + pad, galley, color);
}

/// Décode une image disque (JPG/PNG) en ColorImage egui.
fn load_color_image(path: &str) -> Result<egui::ColorImage, String> {
    let img = image::open(path).map_err(|e| format!("Image introuvable :\n{path}\n{e}"))?;
    let rgba = img.to_rgba8();
    let (w, h) = rgba.dimensions();
    Ok(egui::ColorImage::from_rgba_unmultiplied(
        [w as usize, h as usize],
        rgba.as_raw(),
    ))
}
