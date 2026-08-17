//! Spike Rust + egui tracker OoTMM.
#![windows_subsystem = "windows"]

mod data;
mod dialog;
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
mod i18n;
mod state;
mod ui;

use i18n::{AppSettings, I18n};
use std::collections::{HashMap, HashSet, VecDeque};
use std::path::PathBuf;
use std::time::Instant;

use eframe::egui::{self, pos2, vec2, Align2, Color32, FontId, Rect, Sense, Stroke, Vec2};

use scene::{Game, LiveScene};
use tracking::RomVersion;

/// Maximum number of events kept in the log panel.
const LOG_CAP: usize = 500;
/// Scene loaded by default at startup (OoT Kokiri Forest = 0x55).
const DEFAULT_SCENE: u16 = data::scenes::OOT_KOKIRI_FOREST;

/// Icons flanking the age/season context switch (Qt ContextSwitchButton): OoT
/// child/adult heads and MM winter/spring, preloaded into the icon cache.
const CONTEXT_ICON_PATHS: [&str; 4] = [
    "./Resources/Common/ChildHead.png",
    "./Resources/Common/AdultHead.png",
    "./Resources/Common/Winter.png",
    "./Resources/Common/Spring.png",
];

/// Whether an object of context `ctx` shows under the active scene context
/// `eff` (None = the scene has no age/season context, so show everything).
fn context_allows(eff: Option<data::ObjectContext>, ctx: data::ObjectContext) -> bool {
    match eff {
        None => true,
        Some(c) => ctx == data::ObjectContext::All || ctx == c,
    }
}

/// A Launch-page button press, resolved after the UI closure so the handlers
/// can borrow `self` mutably without conflicting with the surrounding draw.
#[derive(Clone, Copy)]
enum LaunchAction {
    None,
    Save,
    Load,
    Spoiler,
    Reset,
    Toggle,
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

    fn label(self, i18n: &I18n) -> &str {
        match self {
            Tab::Launch => i18n.launch(),
            Tab::Oot => "OoT",
            Tab::Mm => "MM",
            Tab::Entrance => i18n.entrance(),
            Tab::Progression => i18n.progress(),
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
            .with_title("OoTMMCombo Auto Tracker"),
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
fn shuffle_label(i18n: &I18n, s: data::ShuffleSetting) -> &str {
    use data::ShuffleSetting as S;
    match s {
        S::vanilla => i18n.shuffle_vanilla(),
        S::removed => i18n.shuffle_removed(),
        S::starting => i18n.shuffle_starting(),
        S::all => i18n.shuffle_all(),
        S::dungeons => i18n.shuffle_dungeons(),
        S::overworld => i18n.shuffle_overworld(),
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

/// The rendered content of one entrance info box (Qt `EntranceGroupBoxItem`):
/// a title (icon + entrance name), the green "how you arrive" rows (one per
/// inbound source) and the single red "where it leads" row. Shared by the map
/// overlay boxes and the right-panel entrance cards so both stay identical.
pub(crate) struct EntranceBoxData {
    pub title: &'static str,
    pub icon: &'static str,
    pub has_in: bool,
    pub has_out: bool,
    /// One row per known inbound source ("?" placeholder when none is known);
    /// each carries the (game, scene, entrance) to focus when clicked.
    pub in_rows: Vec<(String, Option<(Game, u16, u32)>)>,
    /// The single "where it leads" row, present iff `has_out`.
    pub out_row: Option<(String, Option<(Game, u16, u32)>)>,
}

/// Format one inbound-source label the way Qt `GetEntranceSpawnsString` /
/// `GetOneWayInName` do: `e_type` is the described entrance's type, `s` the
/// linked source entrance whose from/to names build the text.
fn fmt_in_link(e_type: data::EntranceType, s: &data::EntranceDef) -> String {
    use data::EntranceType::*;
    match e_type {
        Normal => {
            if s.type_ == One_Way_Out {
                format!("{} → {}", s.from_name, s.to_name)
            } else {
                format!("{} → {}", s.to_name, s.from_name)
            }
        }
        One_Way_In => {
            if s.type_ == Normal {
                format!("{} → {}", s.from_name, s.to_name)
            } else {
                s.to_name.to_string()
            }
        }
        _ => s.to_name.to_string(),
    }
}

/// Format the outbound-destination label the way Qt `GetEntranceLeadsString` /
/// `GetOneWayOutName` do: `d` is the destination entrance.
fn fmt_out_link(e_type: data::EntranceType, d: &data::EntranceDef) -> String {
    use data::EntranceType::*;
    match e_type {
        One_Way_Out => {
            if d.type_ == Normal {
                format!("{} - {}", d.to_name, d.from_name)
            } else {
                d.to_name.to_string()
            }
        }
        _ => format!("{} - {}", d.to_name, d.from_name),
    }
}

/// Build the display + navigation content of one entrance's box from the live
/// link maps (port of `EntranceGroupBoxItem::RefreshText`). Takes the two maps
/// by reference (rather than `&self`) so it can run while the scene is borrowed
/// mutably in `draw_map`.
pub(crate) fn entrance_box_data(
    game: Game,
    e: &'static data::EntranceDef,
    in_links: &HashMap<(Game, u32), Vec<(Game, u32)>>,
    out_links: &HashMap<(Game, u32), (Game, u32)>,
) -> EntranceBoxData {
    use data::EntranceType as ET;
    let has_in = e.type_ != ET::One_Way_Out;
    let has_out = e.type_ != ET::One_Way_In;
    let key = (game, e.to_id);

    // Green rows: every inbound source, or a single "?" placeholder when none is
    // known yet (mirrors the legacy single-arm box).
    let mut in_rows: Vec<(String, Option<(Game, u16, u32)>)> = Vec::new();
    if has_in {
        match in_links.get(&key).filter(|v| !v.is_empty()) {
            Some(sources) => {
                for &(sg, sid) in sources {
                    match entrance::lookup(sg, sid) {
                        Some(s) => in_rows.push((fmt_in_link(e.type_, s), Some((sg, s.to_scene, sid)))),
                        None => in_rows.push(("?".to_string(), None)),
                    }
                }
            }
            None => in_rows.push(("?".to_string(), None)),
        }
    }

    // Red row: where this entrance leads once discovered.
    let out_row = has_out.then(|| match out_links.get(&key) {
        Some(&(dg, did)) => match entrance::lookup(dg, did) {
            Some(d) => (fmt_out_link(e.type_, d), Some((dg, d.to_scene, did))),
            None => ("?".to_string(), None),
        },
        None => ("?".to_string(), None),
    });

    // The box/card title is the entrance's own name (Qt `formatEntrance` ->
    // `GetEntranceFromName` -> `FromName`): the side facing away from this scene,
    // e.g. "Fire Temple" for the Fire Temple doorway inside Death Mountain Crater.
    EntranceBoxData { title: e.from_name, icon: e.icon, has_in, has_out, in_rows, out_row }
}

/// A fixed-width table cell (entrance table): allocates `w` px and runs `add`.
/// Render cell content pinned to an absolute sub-rect. Using an explicit
/// `max_rect` (instead of `allocate_ui_with_layout`, which only reserves the
/// content's `min_rect` and lets short cells collapse) guarantees every cell
/// occupies its whole column, so the columns line up with the header separators.
/// The clip rect keeps content from bleeding past the column edge.
fn table_cell_at(ui: &mut egui::Ui, rect: Rect, add: impl FnOnce(&mut egui::Ui)) {
    let mut child = ui.new_child(
        egui::UiBuilder::new().max_rect(rect).layout(egui::Layout::left_to_right(egui::Align::Center)),
    );
    child.set_clip_rect(rect);
    add(&mut child);
}

/// GPS scene picker listing both games' scenes (skipping technical maps);
/// resets the paired entrance when the scene changes.
fn gps_scene_combo(
    ui: &mut egui::Ui,
    i18n: &I18n,
    id: &str,
    sel: &mut Option<(Game, u16)>,
    ent: &mut Option<u32>,
) {
    let text = sel
        .and_then(|(g, s)| {
            g.scenes().iter().find(|d| d.id == s).map(|d| {
                let tag = if g == Game::Oot { "OoT" } else { "MM" };
                format!("{tag} — {}", d.name)
            })
        })
        .unwrap_or_else(|| format!("({})", i18n.choose()));
    egui::ComboBox::from_id_salt(id).width(230.0).selected_text(text).show_ui(ui, |ui| {
        for game in [Game::Oot, Game::Mm] {
            ui.label(egui::RichText::new(game.label()).strong().color(game_accent(game)));
            for s in game.scenes() {
                if s.region_id == 0 {
                    continue; // technical / scene-less maps
                }
                if ui.selectable_label(*sel == Some((game, s.id)), s.name).clicked() {
                    *sel = Some((game, s.id));
                    *ent = None;
                }
            }
        }
    });
}

/// GPS entrance picker for a chosen scene (or "(toute)" = the whole scene).
fn gps_entrance_combo(
    ui: &mut egui::Ui,
    i18n: &I18n,
    id: &str,
    scene: Option<(Game, u16)>,
    ent: &mut Option<u32>,
) {
    let Some((game, sid)) = scene else {
        ui.weak("—");
        return;
    };
    let text = ent
        .and_then(|e| entrance::lookup(game, e).map(|d| d.to_name))
        .unwrap_or(i18n.gps_whole_scene());
    egui::ComboBox::from_id_salt(id).width(220.0).selected_text(text).show_ui(ui, |ui| {
        if ui.selectable_label(ent.is_none(), i18n.gps_whole_scene()).clicked() {
            *ent = None;
        }
        for e in game.entrances() {
            if e.to_scene != sid || e.to_name.is_empty() {
                continue;
            }
            if ui.selectable_label(*ent == Some(e.to_id), e.to_name).clicked() {
                *ent = Some(e.to_id);
            }
        }
    });
}

/// A clickable table cell (absolute sub-rect) showing a link-tinted, truncated
/// label. Truncation clips to the column width so long names never overflow.
fn table_link_cell_at(ui: &mut egui::Ui, rect: Rect, text: &str) -> egui::Response {
    let mut child = ui.new_child(
        egui::UiBuilder::new().max_rect(rect).layout(egui::Layout::left_to_right(egui::Align::Center)),
    );
    child.set_clip_rect(rect);
    child.add(
        egui::Label::new(egui::RichText::new(text).color(Color32::from_rgb(150, 190, 230)))
            .truncate()
            .sense(Sense::click()),
    )
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
/// `text` sits at `indent`, an optional collapse pill (`expand`: Some(true)=open
/// "−", Some(false)=closed "+") at the far left, and an optional (collected, total)
/// count right-aligned (green once complete). Returns the click response.
#[allow(clippy::too_many_arguments)]
fn tinted_row(
    ui: &mut egui::Ui,
    height: f32,
    indent: f32,
    bg: Color32,
    text: &str,
    text_col: Color32,
    count: Option<(usize, usize)>,
    expand: Option<bool>,
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
        // Collapse pill, redrawn from Qt's Plus.svg / Minus.svg (muted blue lines,
        // rounded): "−" when open, "+" when closed. Painter-drawn so we need no SVG
        // decoder (the `image` crate only handles JPEG/PNG).
        if let Some(open) = expand {
            let pc = mid + vec2(9.0, 0.0);
            let r = 4.5_f32;
            let sw = Stroke::new(1.6_f32, Color32::from_rgb(0x7a, 0x9a, 0xbf));
            painter.line_segment([pc + vec2(-r, 0.0), pc + vec2(r, 0.0)], sw);
            if !open {
                painter.line_segment([pc + vec2(0.0, -r), pc + vec2(0.0, r)], sw);
            }
        }
        // Region / category icon just left of the label (slot before `indent`).
        if let Some(tex) = icon {
            let ir = Rect::from_center_size(mid + vec2(indent - 11.0, 0.0), Vec2::splat(18.0));
            painter.image(tex, ir, Rect::from_min_max(pos2(0.0, 0.0), pos2(1.0, 1.0)), Color32::WHITE);
        }
        // Label, elided with "…" so long (translated) region / scene names never
        // overrun the collected/total count or the right edge.
        let text_left = rect.left() + indent;
        let reserve = if count.is_some() { 46.0 } else { 8.0 };
        let mut job = egui::text::LayoutJob::single_section(
            text.to_owned(),
            egui::TextFormat { font_id: FontId::proportional(14.0), color: text_col, ..Default::default() },
        );
        job.wrap = egui::text::TextWrapping {
            max_width: (rect.right() - reserve - text_left).max(6.0),
            max_rows: 1,
            break_anywhere: true,
            overflow_character: Some('…'),
        };
        let galley = painter.layout_job(job);
        painter.galley(pos2(text_left, mid.y - galley.size().y * 0.5), galley, text_col);
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
                FontId::proportional(12.5),
                cc,
            );
        }
    }
    resp
}

/// Paint one single line of tree-leaf text, elided with "…" past `max_w`,
/// vertically centred on `center_y` at `x`, optionally struck through (Qt
/// `elidedText` + `setStrikeOut`).
#[allow(clippy::too_many_arguments)]
fn elided_line(
    painter: &egui::Painter,
    x: f32,
    center_y: f32,
    max_w: f32,
    text: &str,
    size: f32,
    color: Color32,
    strike: bool,
) {
    let mut job = egui::text::LayoutJob::single_section(
        text.to_owned(),
        egui::TextFormat {
            font_id: FontId::proportional(size),
            color,
            strikethrough: if strike { Stroke::new(1.1_f32, color) } else { Stroke::NONE },
            ..Default::default()
        },
    );
    job.wrap = egui::text::TextWrapping {
        max_width: max_w.max(6.0),
        max_rows: 1,
        break_anywhere: true,
        overflow_character: Some('…'),
    };
    let galley = painter.layout_job(job);
    painter.galley(pos2(x, center_y - galley.size().y * 0.5), galley, color);
}

/// A thin full-width progress bar (Qt scene/room progress) filled to `done/total`
/// in `color` over a dark track.
fn progress_bar(ui: &mut egui::Ui, done: usize, total: usize, color: Color32) {
    let w = ui.available_width();
    let (rect, _) = ui.allocate_exact_size(vec2(w, 6.0), Sense::hover());
    let p = ui.painter();
    p.rect_filled(rect, 3.0, Color32::from_gray(60));
    let frac = if total > 0 { done as f32 / total as f32 } else { 0.0 };
    if frac > 0.0 {
        let fill = Rect::from_min_size(rect.min, vec2(rect.width() * frac.clamp(0.0, 1.0), rect.height()));
        p.rect_filled(fill, 3.0, color);
    }
}

struct TrackerApp {
    /// --- Current UI language ---
    i18n: I18n,
    /// Persisted app settings (currently just the UI language) and the file they
    /// live in, so the chosen language survives across sessions.
    app_settings: AppSettings,
    app_settings_path: PathBuf,

    // --- Navigation / Current scene ---
    scene: Option<LiveScene>,

    // --- Shared memory / event stream ---
    /// Background poller (owns the shared-memory link off the UI thread) and the
    /// live connection state it reports.
    poller: poller::Poller,
    connected: bool,
    /// Whether the auto-tracker is started (Start/Stop button + status pill).
    /// Mirror of `LogTab::IsRunning`; drives the poller's connect/idle gate.
    tracking: bool,
    /// Whether the program should auto save or not when an entrance or item is received.
    auto_save: bool,
    /// Multiplayer launch options (Qt `NetCheckBox` / `Host` / `Port`). The
    /// networking itself is not ported yet, but the controls mirror the Qt page.
    use_multiplayer: bool,
    mp_host: String,
    mp_port: String,
    /// Text journal shown on the Launch page (Qt `LogViewer`): action / status
    /// messages, newest at the bottom.
    log_lines: VecDeque<String>,
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
    /// GPS sub-tab: start / arrival scene (per game) + a specific entrance in
    /// each (cross-game routing over discovered links).
    gps_from: Option<(Game, u16)>,
    gps_to: Option<(Game, u16)>,
    gps_from_ent: Option<u32>,
    gps_to_ent: Option<u32>,
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
    /// Right-panel entrance tree: "Find…" filter + "expand/collapse all" state.
    ent_search: String,
    ent_all_expanded: bool,
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
    /// The player's current scene, derived from the last IN entrance (drives the
    /// auto-follow and auto-GPS-start options).
    player_scene: Option<(Game, u16)>,
    /// Last scene we already auto-followed to (so a follow only fires on a move).
    followed_scene: Option<(Game, u16)>,
    /// Pending auto-snap request (game, scene, x, y) from the last collected object.
    pending_snap: Option<(Game, u16, f32, f32)>,
    /// Map position to centre on when the view (re)initialises (auto-snap target).
    snap_pos: Option<(f32, f32)>,
    /// Whether the ROM Settings window is open.
    show_settings: bool,
    /// Whether the "About" dialog window is open.
    show_about: bool,
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

    // --- Connection / injection ---
    status: String,
    /// ROM build reported by the DLL (drives the NPC id fix-up).
    rom: RomVersion,

    // --- Map rendering ---
    /// Object-type icon textures (None = load failed → colored fallback).
    icon_cache: HashMap<&'static str, Option<egui::TextureHandle>>,
    map_texture: Option<egui::TextureHandle>,
    map_size: Vec2,
    load_error: Option<String>,

    // --- View transform (pan/zoom) ---
    zoom: f32,
    pan: Vec2,
    view_initialized: bool,

    /// Wall-clock of the last rendered frame, for the interaction frame-rate cap.
    last_frame: Instant,
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
