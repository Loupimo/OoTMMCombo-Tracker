//! The item tabs: scene navigation tree, per-scene object tree, and the
//! pan/zoom scene map with its clickable object overlays.
use eframe::egui::{self, pos2, vec2, Align2, Color32, FontId, Rect, Sense, Stroke, Vec2};
use std::collections::{HashMap, HashSet};

use crate::*;
use crate::scene::{self, Game};
use crate::ui::kbdnav;

// Entrance info-box metrics, in map units (they scale with the view zoom, like
// the Qt QGraphicsScene items). Mirror EntranceGroupBoxItem's k* constants.
const EB_TITLE_H: f32 = 22.0; // title row height
const EB_ROW_H: f32 = 18.0; // in/out row height
const EB_PAD_X: f32 = 9.0; // horizontal text padding
const EB_ACCENT_W: f32 = 3.0; // left colour accent strip
const EB_TITLE_ICON: f32 = 16.0; // square side of the title icon
const EB_ICON_GAP: f32 = 6.0; // gap after the title icon
const EB_ROW_MARK: f32 = 11.0; // width reserved for a row's in/out triangle
const EB_MIN_W: f32 = 80.0; // lower bound on the computed width
const EB_MAX_W: f32 = 400.0; // safety cap for degenerate strings
const EB_TITLE_FONT: f32 = 12.0; // title font size (map units)
const EB_ROW_FONT: f32 = 10.5; // in/out row font size (map units)

impl TrackerApp {

    /// One object row (Qt SceneItemTree leaf): a full-width tinted card whose
    /// background reflects the collection kind (uncollected / auto-collected /
    /// manually-forced), the item icon, the object name struck through once
    /// collected, and the spoiler item (or "???") beneath. Returns the click
    /// response (a click toggles the object).
    fn object_leaf(&self, ui: &mut egui::Ui, game: Game, o: &scene::LiveObject) -> egui::Response {
        let forced = self.cw().forced.contains(&(game, o.index));
        let collected = o.collected;
        // Card + text tints (port of ObjectRenderer.cpp): amber/gold for the
        // user-forced entries (so they read apart from OoT blue / MM violet),
        // dim-neutral for auto-collected, bright for undiscovered.
        let (card_bg, name_col, item_col) = if forced {
            (
                Some(Color32::from_rgba_unmultiplied(255, 190, 90, 32)),
                Color32::from_rgb(248, 200, 120),
                Color32::from_rgba_unmultiplied(208, 168, 110, 200),
            )
        } else if collected {
            (
                Some(Color32::from_rgba_unmultiplied(255, 255, 255, 28)),
                Color32::from_rgba_unmultiplied(204, 218, 240, 150),
                Color32::from_rgba_unmultiplied(141, 162, 192, 140),
            )
        } else {
            (None, Color32::from_rgb(221, 238, 255), Color32::from_rgb(122, 154, 191))
        };
        // Accessibility filter (dim mode): an uncollected unreachable check reads
        // faded. In hide mode the caller skips the row entirely, so this only ever
        // applies while the row is still shown.
        let unreachable = !collected && self.obj_unreachable(o.location);
        let (card_bg, name_col, item_col) = if unreachable {
            (
                None,
                Color32::from_rgb(110, 118, 130),
                Color32::from_rgb(88, 96, 108),
            )
        } else {
            (card_bg, name_col, item_col)
        };
        // Item held here (spoiler). "Reveal uncollected items" off → hide unknown
        // items on undiscovered objects behind "???" (Qt ObjectRenderer).
        let item = if collected || self.dashboard.reveal {
            match self.cw().items.get(o.location) {
                Some(it) => self.i18n.tr_item(it),
                None => "???",
            }
        } else {
            "???"
        };
        let name = self.i18n.tr_object(o.name);
        let icon = scene::icon_path_for(o.map_icon, o.type_)
            .and_then(|p| self.icon_cache.get(p))
            .and_then(|t| t.as_ref())
            .map(|t| t.id());

        let (rect, resp) = ui.allocate_exact_size(vec2(ui.available_width(), 42.0), Sense::click());
        if ui.is_rect_visible(rect) {
            let painter = ui.painter();
            // Faint per-game accent tint over each object row (Qt TintedTreeWidget
            // depth-1, alpha 16) — the "extra" background layer, but only as tall as
            // the object cards, not the whole panel.
            let a = game_accent(game);
            painter.rect_filled(rect, 0.0, Color32::from_rgba_unmultiplied(a.r(), a.g(), a.b(), 16));
            // Collected/forced "card" highlight (undiscovered rows get no card).
            let card = rect.shrink2(vec2(3.0, 1.5));
            if let Some(bg) = card_bg {
                painter.rect_filled(card, 6.0, bg);
            }
            if resp.hovered() {
                painter.rect_filled(card, 6.0, Color32::from_white_alpha(10));
            }
            // Icon over a faint rounded square (Qt icon backdrop).
            let icon_sz = 30.0;
            let icon_center = pos2(rect.left() + 9.0 + icon_sz * 0.5, rect.center().y);
            painter.rect_filled(
                Rect::from_center_size(icon_center, Vec2::splat(icon_sz)),
                5.0,
                Color32::from_white_alpha(18),
            );
            if let Some(id) = icon {
                let tint = if collected { Color32::from_gray(150) } else { Color32::WHITE };
                let ir = Rect::from_center_size(icon_center, Vec2::splat(icon_sz - 4.0));
                painter.image(id, ir, Rect::from_min_max(pos2(0.0, 0.0), pos2(1.0, 1.0)), tint);
            }
            // Name (top) + spoiler item (bottom), elided to the panel width and
            // struck through once collected.
            let text_x = icon_center.x + icon_sz * 0.5 + 9.0;
            let max_w = rect.right() - text_x - 8.0;
            elided_line(painter, text_x, rect.center().y - 8.0, max_w, name, 14.0, name_col, collected);
            elided_line(painter, text_x, rect.center().y + 9.0, max_w, item, 11.5, item_col, collected);
        }
        resp
    }

    /// The Qt age/season toggle (ContextSwitchButton): a rounded pill (grey when
    /// off / blue when on) with a sliding white knob, flanked by the left/right
    /// context icons (child/adult or winter/spring). Returns the toggled value.
    fn context_switch(&self, ui: &mut egui::Ui, on: bool, left: &str, right: &str) -> (bool, egui::Response) {
        let tex = |p: &str| self.icon_cache.get(p).and_then(|t| t.as_ref()).map(|t| t.id());
        let icon_sz = 22.0_f32;
        let (pill_w, pill_h, gap) = (46.0_f32, 24.0_f32, 6.0_f32);
        let total = vec2(icon_sz + gap + pill_w + gap + icon_sz, pill_h.max(icon_sz));
        let (rect, resp) = ui.allocate_exact_size(total, Sense::click());
        let val = if resp.clicked() { !on } else { on };
        // Smoothly slide / fade between states (Qt QPropertyAnimation).
        let t = ui.ctx().animate_bool(resp.id, val);
        let painter = ui.painter();
        let cy = rect.center().y;
        let uv = Rect::from_min_max(pos2(0.0, 0.0), pos2(1.0, 1.0));
        // Left icon (child / winter): bright when off, dimmed when on.
        if let Some(id) = tex(left) {
            let ir = Rect::from_center_size(pos2(rect.left() + icon_sz * 0.5, cy), Vec2::splat(icon_sz));
            painter.image(id, ir, uv, Color32::from_white_alpha((255.0 * (1.0 - 0.55 * t)) as u8));
        }
        // Pill track: grey (#ccc) -> blue (#0078D7).
        let pill = Rect::from_min_size(pos2(rect.left() + icon_sz + gap, cy - pill_h * 0.5), vec2(pill_w, pill_h));
        let mix = |a: u8, b: u8| (a as f32 + (b as f32 - a as f32) * t) as u8;
        painter.rect_filled(pill, pill_h * 0.5, Color32::from_rgb(mix(204, 0), mix(204, 120), mix(204, 215)));
        // Sliding white knob.
        let kr = pill_h * 0.5 - 3.0;
        let kx = pill.left() + kr + 3.0 + (pill_w - 2.0 * (kr + 3.0)) * t;
        painter.circle_filled(pos2(kx, cy), kr, Color32::WHITE);
        // Right icon (adult / spring): dimmed when off, bright when on.
        if let Some(id) = tex(right) {
            let ir = Rect::from_center_size(pos2(rect.right() - icon_sz * 0.5, cy), Vec2::splat(icon_sz));
            painter.image(id, ir, uv, Color32::from_white_alpha((255.0 * (0.45 + 0.55 * t)) as u8));
        }
        (val, resp)
    }

    /// Right panel of the item tabs: objects grouped by category.
    pub(crate) fn draw_object_tree(&mut self, ctx: &egui::Context) {
        // No scene open -> hide the whole panel (nothing to show until the user
        // picks a scene), instead of an empty "Objects" placeholder.
        if self.scene.is_none() {
            return;
        }
        let mut toggle: Option<(Game, usize)> = None;
        let mut obj_set_all: Option<bool> = None;
        // Keyboard navigation: arrows move through category headers + objects,
        // Left / Right fold / unfold a category, Enter / Space toggles an object.
        let kid = egui::Id::new("kbd_obj_tree");
        let mut klist = self.kbd.begin(kid);
        let mut kout = kbdnav::KbdOut::default();
        let mut obj_targets: HashMap<u64, (Game, usize)> = HashMap::new();
        egui::SidePanel::right("objtree")
            .resizable(true)
            .default_width(320.0)
            .show(ctx, |ui| {
                let Some((game, sname, done, total)) = self.scene.as_ref().map(|s| {
                    let (d, t) = s.progress();
                    (s.game, s.def.name, d, t)
                }) else {
                    accent_heading(ui, self.i18n.objects());
                    return;
                };

                // Slightly taller search bar / expand button (Qt QPushButton /
                // QLineEdit min-height 24 + padding), applied to every tree.
                ui.spacing_mut().interact_size.y = 26.0;

                // Header: active scene name, collected/total, the category filter
                // button (moved here from the menu bar), and a scene progress bar.
                let filter_tip = self.i18n.filter_tooltip().to_owned();
                ui.horizontal(|ui| {
                    ui.label(egui::RichText::new(self.i18n.tr_scene(sname)).heading().size(17.0).color(ACCENT));
                    ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                        ui.menu_button("⚙", |ui| self.draw_filter_menu(ui))
                            .response
                            .on_hover_text(&filter_tip);
                        ui.label(format!("{done} / {total}"));
                    });
                });
                progress_bar(ui, done, total, game_accent(game));
                ui.add_space(2.0);
                ui.add(
                    egui::TextEdit::singleline(&mut self.obj_search)
                        .hint_text(self.i18n.search())
                        .desired_width(f32::INFINITY),
                );
                let label = if self.obj_all_expanded {
                    format!("⊟ {}", self.i18n.collapse_all())
                } else {
                    format!("⊞ {}", self.i18n.expand_all())
                };
                if ui.add(egui::Button::new(label).min_size(vec2(ui.available_width(), 0.0))).clicked() {
                    self.obj_all_expanded = !self.obj_all_expanded;
                    obj_set_all = Some(self.obj_all_expanded);
                }
                ui.separator();

                let query = self.obj_search.trim().to_lowercase();
                let active = self.active_types[game.idx()].clone();
                let hide_list = self.app_settings.hide_collected_list;
                let scene = self.scene.as_ref().unwrap();
                let accent = game_accent(game);

                egui::ScrollArea::vertical().id_salt("objtree_s").show(ui, |ui| {
                    // Rows sit flush so the per-row accent tint is continuous (Qt tree).
                    ui.spacing_mut().item_spacing.y = 0.0;
                    // Categories present in this scene (ROM exclusion + active filter).
                    let mut order: Vec<data::ObjectType> = Vec::new();
                    for o in &scene.objects {
                        if self.excluded.contains(game, o.index) || !active.contains(&o.type_) {
                            continue;
                        }
                        if !order.contains(&o.type_) {
                            order.push(o.type_);
                        }
                    }
                    // Sort alphabetically by the DISPLAYED (translated) category name,
                    // like the nav regions/scenes, so the order follows the language.
                    order.sort_by(|&a, &b| {
                        scene::type_label(a, &self.i18n)
                            .to_lowercase()
                            .cmp(&scene::type_label(b, &self.i18n).to_lowercase())
                    });
                    // "Hide unreachable" mode: an uncollected unreachable object is
                    // not listed, so it drops from the category counts too — a
                    // category with nothing visible is left out entirely.
                    let hide_mode = self.app_settings.logic_filter_enabled
                        && self.app_settings.logic_hide_unreachable;
                    let visible =
                        |o: &scene::LiveObject| !(hide_mode && !o.collected && self.obj_unreachable(o.location));
                    for ty in order {
                        let in_cat = |o: &&scene::LiveObject| {
                            o.type_ == ty && !self.excluded.contains(game, o.index) && visible(o)
                        };
                        // Objects of this category matching the search box.
                        let objs: Vec<&scene::LiveObject> = scene
                            .objects
                            .iter()
                            .filter(in_cat)
                            .filter(|o| {
                                if query.is_empty() {
                                    return true;
                                }
                                // Match both the raw (English) name and the translated
                                // one shown in the tree, plus the placed item (raw +
                                // translated) so a French search hits French labels.
                                o.name.to_lowercase().contains(&query)
                                    || self.i18n.tr_object(o.name).to_lowercase().contains(&query)
                                    || self.cw().items.get(o.location).is_some_and(|it| {
                                        it.to_lowercase().contains(&query)
                                            || self.i18n.tr_item(it).to_lowercase().contains(&query)
                                    })
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
                        let header = tinted_row(
                            ui, 24.0, 40.0,
                            Color32::from_rgba_unmultiplied(accent.r(), accent.g(), accent.b(), 64),
                            game_hover(game),
                            scene::type_label(ty, &self.i18n), TEXT,
                            Some((done, total)), Some(open), cat_tex,
                        );
                        // Faint white underline marking the category separation
                        // (Qt TintedTreeWidget depth-0 rows).
                        ui.painter().line_segment(
                            [header.rect.left_bottom(), header.rect.right_bottom()],
                            Stroke::new(1.0_f32, Color32::from_white_alpha(32)),
                        );
                        if header.clicked() {
                            open = !open;
                            ui.data_mut(|d| d.insert_persisted(id, open));
                        }
                        // Keyboard branch: Left / Right fold / unfold this category.
                        let ckey = kbdnav::key(("objcat", game.idx() as u8, ty as usize));
                        klist.branch(ui, ckey, id, open, &header);

                        if open {
                            for o in objs {
                                if hide_list && o.collected {
                                    continue; // "Hide Collected Object → From Object List"
                                }
                                let resp = self.object_leaf(ui, game, o);
                                let ok = ((game.idx() as u64) << 32) | o.index as u64;
                                klist.leaf(ui, ok, &resp);
                                obj_targets.insert(ok, (game, o.index));
                                if resp.clicked() {
                                    toggle = Some((game, o.index));
                                }
                            }
                        }
                    }
                    // Read the arrow keys once every object row has registered.
                    kout = klist.finish(ui);
                });
            });
        // Keep the tree's focus / active target in sync; Enter / Space toggles the
        // highlighted object (arrow moves only move the highlight).
        self.kbd.apply(kid, &kout);
        if let Some((g, i)) = kout.activate.and_then(|k| obj_targets.get(&k).copied()) {
            toggle = Some((g, i));
        }
        if let Some((game, index)) = toggle {
            self.toggle_object(game, index);
        }
    }

    /// Panneau gauche : arbre jeux -> régions -> scènes, façon Qt (lignes teintées
    /// par jeu, compteurs collectés/total, régions repliables).
    pub(crate) fn draw_nav(&mut self, ctx: &egui::Context) {
        let mut clicked: Option<(Game, &'static data::SceneDef)> = None;
        let mut region_selected: Option<(Game, u8)> = None;
        let mut set_all: Option<bool> = None; // "expand/collapse all" this frame
        let entrance_tab = self.active_tab.is_entrance();
        // "Hide unreachable" mode: the scene counts already drop hidden checks, so a
        // scene with a zero visible total has nothing reachable and is left out of
        // the tree (item tabs only; the entrance tab keeps its own `t > 0` gate).
        let hide_mode = self.app_settings.logic_filter_enabled
            && self.app_settings.logic_hide_unreachable;

        // Keyboard navigation: arrows move through region headers + scenes,
        // Left / Right fold / unfold the focused region, and focusing a scene
        // loads its map (the same action as a click).
        let skey = |g: Game, id: u16| -> u64 { ((g.idx() as u64) << 32) | id as u64 };
        let kid = egui::Id::new("kbd_nav_tree");
        let mut klist = self.kbd.begin(kid);
        let mut kout = kbdnav::KbdOut::default();
        let mut key_targets: HashMap<u64, (Game, &'static data::SceneDef)> = HashMap::new();

        egui::SidePanel::left("nav")
            .resizable(true)
            .default_width(260.0)
            .show(ctx, |ui| {
                accent_heading(ui, self.i18n.scenes_title());
                // Slightly taller search bar / expand button (Qt min-height 24),
                // consistent across every tree.
                ui.spacing_mut().interact_size.y = 26.0;
                // Live scene filter (Qt "Find…").
                ui.add(
                    egui::TextEdit::singleline(&mut self.scene_search)
                        .hint_text(self.i18n.search())
                        .desired_width(f32::INFINITY),
                );
                // Expand / collapse every region at once (Qt "Expand All").
                let label = if self.nav_all_expanded {
                    format!("⊟ {}", self.i18n.collapse_all())
                } else {
                    format!("⊞ {}", self.i18n.expand_all())
                };
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
                    // Rows sit flush (Qt tree has no inter-row gap); the rounded
                    // corners + 3px spacing used to leave a visible seam between
                    // regions.
                    ui.spacing_mut().item_spacing.y = 0.0;
                    for &game in games {
                        let accent = game_accent(game);
                        let sel_bg = game_selection(game);
                        // Qt MapTab drawRow: regions get an accent tint at alpha 64
                        // *over the base*, scenes at alpha 16 — a muted navy, not the
                        // heavily saturated blue a solid multiply produced.
                        let region_bg =
                            Color32::from_rgba_unmultiplied(accent.r(), accent.g(), accent.b(), 64);
                        let scene_bg =
                            Color32::from_rgba_unmultiplied(accent.r(), accent.g(), accent.b(), 16);
                        let hover = game_hover(game); // Qt ::item:hover:!selected
                        let cur_id = self
                            .scene
                            .as_ref()
                            .filter(|s| s.game == game)
                            .map(|s| s.def.id);
                        let scenes = game.scenes();
                        // Per-scene (found, total) counts driving the nav rows. The item
                        // tabs show the object collection counts (cached). The entrance
                        // tab instead shows the per-direction entrance counts (Qt
                        // EntranceGameTabView::RefreshRegionCounters / RefreshName), so
                        // songs / lairs / houses that own no map object still get a real
                        // count and the region totals add up to the global total.
                        let ent_counts: HashMap<u16, (usize, usize)> = if entrance_tab {
                            let table = match game {
                                Game::Oot => data::OOT_SCENE_ENTRANCES,
                                Game::Mm => data::MM_SCENE_ENTRANCES,
                            };
                            table
                                .iter()
                                .map(|&(s, _, _)| (s, self.entrance_scene_counts(game, s)))
                                .collect()
                        } else {
                            HashMap::new()
                        };
                        let counts: &HashMap<u16, (usize, usize)> = if entrance_tab {
                            &ent_counts
                        } else {
                            &self.cached_scene_counts[game.idx()]
                        };

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
                                ui, 24.0, 22.0, bg, hover, self.i18n.all_entrances(),
                                TEXT, None, None, None,
                            );
                            if row.clicked() {
                                region_selected = Some((game, ALL_REGION));
                            }
                        }

                        // Effective region of a scene for the current tab. The item
                        // tabs group by the scene's own ParentRegion (Qt MapTab:
                        // `ParentRegion != 0`). The entrance tab groups by the
                        // entrance-side region and shows only scenes that actually
                        // have entrances — so the Market / ToT hub scenes (ParentRegion
                        // None, entrance-region Market) appear, while the object-only
                        // Market Day / Night variants (no entrances) drop out.
                        let eff_region = |s: &data::SceneDef| -> Option<u8> {
                            if entrance_tab {
                                // Group by the entrance-side region (region 0 included).
                                // Scenes with no valid entrance are dropped later by the
                                // `total > 0` gate below, so the region-0 grottos / MM
                                // Song-of-Time never actually surface a "None" region.
                                entrance::scene_entrance_region(game, s.id as u32)
                            } else if s.region_id != 0 {
                                Some(s.region_id)
                            } else {
                                None
                            }
                        };
                        // Regions, sorted alphabetically by name (not by region id).
                        let mut regions: Vec<u8> = Vec::new();
                        for s in scenes {
                            if let Some(rid) = eff_region(s) {
                                if !regions.contains(&rid) {
                                    regions.push(rid);
                                }
                            }
                        }
                        // Prefer a scene's own region name; fall back to the region
                        // name table for warp regions (Songs / Owls) that own no
                        // ordinary scene, so the entrance nav labels them instead of
                        // showing a bare "—".
                        let region_name_of = |rid: u8| {
                            scenes
                                .iter()
                                .find(|s| s.region_id == rid && s.region_name != "None")
                                .map(|s| s.region_name)
                                .unwrap_or_else(|| scene::region_name(game, rid))
                        };
                        // Alphabetical by the DISPLAYED (translated) region name, so the
                        // order follows the active language (e.g. Woodfall -> Cascade Mojo).
                        regions.sort_by(|&a, &b| {
                            self.i18n
                                .tr_region(region_name_of(a))
                                .to_lowercase()
                                .cmp(&self.i18n.tr_region(region_name_of(b)).to_lowercase())
                        });
                        for rid in regions {
                            // Filter scenes by the search text; skip empty regions.
                            let mut region_scenes: Vec<&'static data::SceneDef> = scenes
                                .iter()
                                .filter(|s| {
                                    eff_region(s) == Some(rid)
                                        // Entrance tab: drop scenes with no valid entrance
                                        // (all None-type / wrong layout -> total 0), exactly
                                        // like Qt's `hasValid` gate in EntranceGameTabView::
                                        // RefreshName. Hides the "None" region (grottos / MM
                                        // Song-of-Time), the OoT mask shop, end/boss cutscene
                                        // scenes, etc. In hide-unreachable mode the item tabs
                                        // apply the same `t > 0` gate to drop scenes with
                                        // nothing reachable.
                                        && ((!entrance_tab && !hide_mode)
                                            || counts.get(&s.id).is_some_and(|&(_, t)| t > 0))
                                        && (query.is_empty()
                                            // Match the raw (English) name and the
                                            // translated one shown in the tree.
                                            || s.name.to_lowercase().contains(&query)
                                            || self.i18n.tr_scene(s.name).to_lowercase().contains(&query))
                                })
                                .collect();
                            if region_scenes.is_empty() {
                                continue;
                            }
                            // Scenes alphabetical by their DISPLAYED (translated) name so the
                            // order follows the active language, like the regions above.
                            region_scenes.sort_by(|a, b| {
                                self.i18n
                                    .tr_scene(a.name)
                                    .to_lowercase()
                                    .cmp(&self.i18n.tr_scene(b.name).to_lowercase())
                            });
                            // Region name from the region id (the entrance hub scenes
                            // carry region_name "None" on their own SceneDef, so read
                            // the name from any scene owning that ParentRegion instead).
                            let rname = match region_name_of(rid) {
                                // Region 0 has no name in the table; Qt labels it "None"
                                // (Regions.h), which the locales render as "Aucune".
                                "" => "None",
                                n => n,
                            };
                            // Region count = sum of its scenes' counts.
                            let rcount = region_scenes.iter().fold((0, 0), |(d, t), s| {
                                let (sd, st) = counts.get(&s.id).copied().unwrap_or((0, 0));
                                (d + sd, t + st)
                            });

                            let id = ui.make_persistent_id(("navreg", game.idx(), rid));
                            // A search forces every region open; "expand/collapse
                            // all" overrides + persists; else the remembered state
                            // (scene trees start collapsed, so default to closed).
                            let mut open = if !query.is_empty() {
                                true
                            } else if let Some(v) = set_all {
                                ui.data_mut(|d| d.insert_persisted(id, v));
                                v
                            } else {
                                ui.data_mut(|d| d.get_persisted::<bool>(id)).unwrap_or(false)
                            };

                            // Region icon (Regions.h), loaded into the shared cache.
                            let region_tex = scene::region_icon(game, rid)
                                .and_then(|p| self.icon_cache.get(p))
                                .and_then(|t| t.as_ref())
                                .map(|t| t.id());
                            let header = tinted_row(
                                ui, 24.0, 40.0, region_bg, hover, self.i18n.tr_region(rname),
                                TEXT, Some(rcount), Some(open), region_tex,
                            );
                            // Region separator (Qt: faint white line under a region row).
                            ui.painter().line_segment(
                                [header.rect.left_bottom(), header.rect.right_bottom()],
                                Stroke::new(1.0_f32, Color32::from_white_alpha(32)),
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
                            // Keyboard branch: Left / Right fold / unfold this region.
                            let rkey = kbdnav::key(("navreg", game.idx() as u8, rid));
                            klist.branch(ui, rkey, id, open, &header);

                            if open {
                                for s in region_scenes {
                                    let selected = cur_id == Some(s.id);
                                    // Qt MapTab drawRow: scenes carry a faint accent tint
                                    // (alpha 16); the selected one takes the opaque
                                    // selection fill, everything else lights up on hover.
                                    let bg = if selected { sel_bg } else { scene_bg };
                                    let hov = if selected { sel_bg } else { hover };
                                    let scount = counts.get(&s.id).copied();
                                    let row = tinted_row(
                                        ui, 22.0, 30.0, bg, hov, self.i18n.tr_scene(s.name),
                                        TEXT, scount, None, None,
                                    );
                                    let sk = skey(game, s.id);
                                    klist.leaf(ui, sk, &row);
                                    key_targets.insert(sk, (game, s));
                                    if row.clicked() {
                                        clicked = Some((game, s));
                                    }
                                }
                            }
                        }
                        ui.add_space(2.0);
                    }
                    // Read the arrow keys once every scene row has registered.
                    kout = klist.finish(ui);
                });
            });

        // Keep the tree's focus / active target in sync, then (for the scene
        // tree) load the focused scene exactly like a click on it. Region headers
        // are not in `key_targets`, so focusing one only moves the highlight.
        self.kbd.apply(kid, &kout);
        if let Some((g, def)) = kout.moved.or(kout.activate).and_then(|k| key_targets.get(&k).copied()) {
            clicked = Some((g, def));
        }

        if let Some((game, def)) = clicked {
            self.select_scene(game, def);
            self.entrance_table = None; // a scene switches the centre back to its map
        }
        if let Some(gr) = region_selected {
            self.entrance_table = Some(gr);
        }
    }

    pub(crate) fn draw_map(&mut self, ctx: &egui::Context) {
        // Deferred entrance navigation: a click on a box row can't borrow `self`
        // mutably while the scene is borrowed, so we record the target and apply
        // it after the panel closure returns.
        let mut ent_nav: Option<(Game, u16, u32)> = None;
        // Deferred pan target: clicking an entrance diamond re-centres the view on
        // its info box (applied after the closure so it can write `self.pan`).
        let mut center_pan: Option<Vec2> = None;
        egui::CentralPanel::default().show(ctx, |ui| {
            let entrance_view = self.active_tab.is_entrance();

            // Age/season toggle (Qt ContextSwitchButton) + room selector, item view.
            // Copy the flags / room names out of the scene borrow first so the
            // header closure only touches plain fields.
            let mut change_room = None;
            // Per-room (collected, total) counts, computed while the scene is still
            // borrowed immutably (respecting the ROM exclusions like the nav tree).
            let header = (!entrance_view).then(|| self.scene.as_ref()).flatten().map(|s| {
                let g = s.game;
                let rooms: Vec<(&'static str, usize, usize)> = s
                    .rooms
                    .iter()
                    .map(|r| {
                        let (d, t) = s
                            .objects
                            .iter()
                            .filter(|o| o.room as u32 == r.id && !self.excluded.contains(g, o.index))
                            .fold((0usize, 0usize), |(d, t), o| (d + o.collected as usize, t + 1));
                        (r.name, d, t)
                    })
                    .collect();
                (s.def.has_context, g, rooms)
            });
            if let Some((has_context, game, rooms)) = header {
                if has_context || !rooms.is_empty() {
                    ui.horizontal(|ui| {
                        if has_context {
                            let (left, right, off, on_) = match game {
                                Game::Oot => (
                                    CONTEXT_ICON_PATHS[0], CONTEXT_ICON_PATHS[1],
                                    self.i18n.map_child().to_owned(), self.i18n.map_adult().to_owned(),
                                ),
                                Game::Mm => (
                                    CONTEXT_ICON_PATHS[2], CONTEXT_ICON_PATHS[3],
                                    self.i18n.map_winter().to_owned(), self.i18n.map_spring().to_owned(),
                                ),
                            };
                            let (new_ctx, resp) =
                                self.context_switch(ui, self.context_toggle, left, right);
                            resp.on_hover_text(format!("{off}  ↔  {on_}"));
                            if new_ctx != self.context_toggle {
                                self.context_toggle = new_ctx;
                                // A scene that ships a per-context map must reload its
                                // background when the winter/spring (child/adult) toggle flips.
                                if self
                                    .scene
                                    .as_ref()
                                    .is_some_and(|s| !s.def.context_image_rel.is_empty())
                                {
                                    self.map_texture = None;
                                }
                            }
                        }
                        if has_context && !rooms.is_empty() {
                            ui.separator();
                        }
                        if !rooms.is_empty() {
                            ui.label(format!("{} :", self.i18n.map_room()));
                            for (k, (name, d, t)) in rooms.iter().enumerate() {
                                let label = format!("{name}  {d}/{t}");
                                if ui.selectable_label(self.current_room == k, label).clicked() {
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
            // Snapshot the active category set + effective context + option flags
            // before borrowing the scene mutably.
            let active_types: HashSet<data::ObjectType> = self
                .scene
                .as_ref()
                .map(|s| self.active_types[s.game.idx()].clone())
                .unwrap_or_default();
            let eff_ctx = self.effective_context();
            let hide_map = self.app_settings.hide_collected_map;
            let auto_zoom = self.app_settings.auto_zoom;
            let snap = if entrance_view { None } else { self.snap_pos.take() };
            // The displayed world (multiworld): its placements + collected marks are
            // what this map shows. Captured before the scene borrow so the field
            // accesses below stay disjoint from `self.scene`.
            let aw = self.active_world;
            // Reachability filter: the positions in the current scene the logic
            // marks as unreachable, captured (with the hide/dim mode) before the
            // mutable `scene` borrow below — `obj_unreachable` borrows all of self.
            let logic_hide = self.app_settings.logic_hide_unreachable;
            let logic_unreachable: HashSet<usize> = if self.app_settings.logic_filter_enabled {
                match self.scene.as_ref() {
                    Some(sc) => sc
                        .objects
                        .iter()
                        .enumerate()
                        .filter(|(_, o)| self.obj_unreachable(o.location))
                        .map(|(i, _)| i)
                        .collect(),
                    None => HashSet::new(),
                }
            } else {
                HashSet::new()
            };
            let (Some(tex), Some(scene)) = (self.map_texture.clone(), self.scene.as_mut()) else {
                ui.centered_and_justified(|ui| ui.weak(self.i18n.map_select_scene()));
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
            // Pure-black viewport backdrop for the scene render (user preference:
            // the map viewport is black, not the near-black navy of the inputs).
            painter.rect_filled(rect, 0.0, Color32::BLACK);

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
                } else if let Some((x, y)) = snap {
                    // Auto Snap View: centre on the collected object (Auto Zoom in).
                    self.zoom = if auto_zoom { (fit * 2.5).clamp(0.3, 4.0) } else { fit };
                    self.pan = rect.size() * 0.5 - vec2(x, y) * self.zoom;
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
                    // Mirror Qt's top-most-Z hit-testing: among the markers under the
                    // cursor, the one drawn in front (highest Z) takes the click, so a
                    // rupee sitting on top of a crate is the one toggled; ties break to
                    // the nearest. Matches the ascending-Z draw order below.
                    let mut best_i = None;
                    let mut best_z = i32::MIN;
                    let mut best_d = f32::MAX;
                    for (i, obj) in scene.objects.iter().enumerate() {
                        if !context_allows(eff_ctx, obj.context)
                            || !active_types.contains(&obj.type_)
                            || self.excluded.contains(scene.game, obj.index)
                            || room_id.is_some_and(|rid| obj.room as u32 != rid)
                            || (hide_map && obj.collected)
                            // Hidden-unreachable markers are not clickable; dimmed
                            // ones (dim mode) still are.
                            || (logic_hide && !obj.collected && logic_unreachable.contains(&i))
                        {
                            continue;
                        }
                        let c = img_min + vec2(obj.x, obj.y) * self.zoom;
                        let d = (cp - c).length();
                        if d <= radius + 2.0 && (obj.z > best_z || (obj.z == best_z && d < best_d)) {
                            best_z = obj.z;
                            best_d = d;
                            best_i = Some(i);
                        }
                    }
                    if let Some(i) = best_i {
                        // Same rule as the object tree: a fresh click force-marks it
                        // (gold), a Forced click clears it, and auto-collected objects
                        // can't be unchecked by the user.
                        let key = (scene.game, scene.objects[i].index);
                        let world = &mut self.worlds[aw];
                        let changed = if world.collected.contains(&key) {
                            if world.forced.remove(&key) {
                                world.collected.remove(&key);
                                scene.objects[i].collected = false;
                                true
                            } else {
                                false
                            }
                        } else {
                            world.collected.insert(key);
                            world.forced.insert(key);
                            scene.objects[i].collected = true;
                            true
                        };
                        if changed {
                            self.dirty = true;
                            self.prog_dirty = true;
                            self.counts_dirty = true;
                        }
                    }
                }
            }

            let hover = resp.hover_pos();
            let mut hovered: Option<&scene::LiveObject> = None;
            let uv_full = Rect::from_min_max(pos2(0.0, 0.0), pos2(1.0, 1.0));

            // Objects are drawn in the item view only.
            let objects: &[scene::LiveObject] = if entrance_view { &[] } else { &scene.objects };
            // Paint in ascending Z so a higher check (e.g. a silver rupee above a crate)
            // draws on top instead of being hidden behind it — mirror of the Qt
            // ObjectRenderer::setZValue(Position[2]). Stable sort keeps the many z==0
            // markers in declaration order; `i` stays the Vec index (the key used by
            // logic_unreachable / scene.objects[i]).
            let mut draw_order: Vec<usize> = (0..objects.len()).collect();
            draw_order.sort_by_key(|&i| objects[i].z);
            for &i in &draw_order {
                let obj = &objects[i];
                if !context_allows(eff_ctx, obj.context)
                    || !active_types.contains(&obj.type_)
                    || self.excluded.contains(scene.game, obj.index)
                    || room_id.is_some_and(|rid| obj.room as u32 != rid)
                    || (hide_map && obj.collected)
                    // Hide mode: an uncollected unreachable check is not drawn.
                    || (logic_hide && !obj.collected && logic_unreachable.contains(&i))
                {
                    continue;
                }
                // Dim mode: an uncollected unreachable check is drawn faded.
                let logic_dim =
                    !logic_hide && !obj.collected && logic_unreachable.contains(&i);
                // Manually-forced objects render in violet (ObjectState::Forced),
                // distinct from auto-collected (dimmed) ones.
                let forced = self.worlds[aw].forced.contains(&(scene.game, obj.index));
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
                    // Collected: dim the icon (forced = violet tint). No overlay
                    // glyph — the default egui font lacks "✓" and rendered it as a
                    // small "tofu" square on top of the icon.
                    let tint = if forced {
                        Color32::from_rgb(253, 218, 0) // gold (user-forced)
                    } else if obj.collected {
                        Color32::from_rgba_unmultiplied(255, 255, 255, 70)
                    } else if logic_dim {
                        Color32::from_rgba_unmultiplied(130, 130, 130, 55) // unreachable
                    } else {
                        Color32::WHITE
                    };
                    painter.image(tex.id(), ir, uv_full, tint);
                } else if obj.collected {
                    // Collected without an icon: just the coloured ring (no glyph).
                    let ring = if forced {
                        Color32::from_rgb(253, 218, 0)
                    } else {
                        Color32::from_rgba_unmultiplied(r, g, b, 110)
                    };
                    painter.circle_stroke(c, radius, Stroke::new(2.0_f32, ring));
                } else if logic_dim {
                    // Uncollected + unreachable: faded gray disc, no glyph.
                    painter.circle_filled(c, radius, Color32::from_rgba_unmultiplied(90, 90, 90, 90));
                    painter.circle_stroke(
                        c,
                        radius,
                        Stroke::new(1.0_f32, Color32::from_rgba_unmultiplied(0, 0, 0, 90)),
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

            // ── Entrance overlay (Qt EntranceRenderer::RenderSceneOverlayGrouped) ──
            // One amber diamond at each entrance anchor + a grouped info box at its
            // authored TextPos, joined by a dashed line to the nearest box edge.
            // Hovering a diamond or a box focuses that pair (solid line, others
            // dimmed); clicking a row navigates to the matching scene / entrance.
            let mut hovered_ent_label: Option<String> = None;
            if entrance_view {
                let z = self.zoom;

                // One entry per active-layout entrance of this scene: box content
                // (shared with the right panel) + screen-space geometry.
                struct EntBox {
                    e: &'static data::EntranceDef,
                    data: EntranceBoxData,
                    anchor: egui::Pos2,
                    rect: Rect,
                    in_count: usize,
                }
                let mut boxes: Vec<EntBox> = Vec::new();
                for e in scene.game.entrances() {
                    if e.to_scene != scene.def.id
                        || e.type_ == data::EntranceType::None // no-entry / end-credits placeholders
                        || e.from_name.is_empty() // nameless placeholder (Qt skips these)
                        || !tracking::scene_layout_active(e.layout, scene.game, e.to_scene, &self.mq_scenes)
                    {
                        continue;
                    }
                    let data = entrance_box_data(&self.i18n, scene.game, e, &self.in_links, &self.out_links);
                    let in_count = if data.has_in { data.in_rows.len() } else { 0 };

                    // Box width = widest row (title incl. icon, in/out rows incl. their
                    // triangle marker). Measured at the *on-screen* font size so the box
                    // fits the text exactly at any zoom (font advances don't scale
                    // perfectly linearly, so measuring at map size then scaling clipped
                    // the longest name); the box then grows to its content like the Qt
                    // BoxWidth. The +4px slack keeps the boundary case from eliding.
                    let measure = |txt: &str, font_map: f32| -> f32 {
                        ui.ctx().fonts(|f| {
                            f.layout_no_wrap(txt.to_owned(), FontId::proportional(font_map * z), Color32::WHITE)
                                .size()
                                .x
                        })
                    };
                    let mut content_w = (EB_TITLE_ICON + EB_ICON_GAP) * z + measure(data.title.as_str(), EB_TITLE_FONT);
                    for (t, _) in &data.in_rows {
                        content_w = content_w.max(EB_ROW_MARK * z + measure(t, EB_ROW_FONT));
                    }
                    if let Some((t, _)) = &data.out_row {
                        content_w = content_w.max(EB_ROW_MARK * z + measure(t, EB_ROW_FONT));
                    }
                    let w_screen =
                        (content_w + EB_PAD_X * 2.0 * z + 4.0).clamp(EB_MIN_W * z, EB_MAX_W * z);
                    let h_screen = (EB_TITLE_H
                        + if data.has_in { in_count as f32 * EB_ROW_H } else { 0.0 }
                        + if data.has_out { EB_ROW_H } else { 0.0 }
                        + 4.0)
                        * z;

                    let tl = img_min + vec2(e.text[0] as f32, e.text[1] as f32) * z;
                    let rect = Rect::from_min_size(tl, vec2(w_screen, h_screen));
                    let anchor = img_min + vec2(e.anchor[0] as f32, e.anchor[1] as f32) * z;
                    boxes.push(EntBox { e, data, anchor, rect, in_count });
                }

                // Group under the pointer? Boxes win over diamonds; later entrances
                // sit on top, so the last containing box takes the hover.
                let anchor_r = 7.0_f32;
                let mut focus: Option<usize> = None;
                let mut focus_row: i32 = -1; // 0 title, 1..in, out row, -1 = diamond/padding
                let mut on_diamond = false; // pointer over the diamond, not the box
                if let Some(hp) = hover {
                    for (i, b) in boxes.iter().enumerate() {
                        if b.rect.contains(hp) {
                            focus = Some(i);
                            focus_row =
                                row_from_y((hp.y - b.rect.top()) / z, b.data.has_in, b.data.has_out, b.in_count);
                        }
                    }
                    if focus.is_none() {
                        let mut best = anchor_r + 1.0;
                        for (i, b) in boxes.iter().enumerate() {
                            let d = (hp - b.anchor).length();
                            if d <= anchor_r && d < best {
                                best = d;
                                focus = Some(i);
                                focus_row = -1;
                                on_diamond = true;
                            }
                        }
                    }
                }

                // Deferred click: navigate to the hovered row's target, falling back
                // to focusing this entrance (title / diamond / undiscovered rows).
                if resp.clicked() {
                    if let Some(i) = focus {
                        let b = &boxes[i];
                        let self_focus = (scene.game, b.e.to_scene, b.e.to_id);
                        if on_diamond {
                            // Clicking a diamond pans the view so its info box is
                            // centred (the box's TextPos can sit far from the anchor).
                            center_pan = Some(self.pan + (rect.center() - b.rect.center()));
                        } else {
                            ent_nav = Some(match focus_row {
                                r if r >= 1 && (r as usize) <= b.in_count => {
                                    b.data.in_rows[r as usize - 1].1.unwrap_or(self_focus)
                                }
                                r if b.data.has_out && r == b.in_count as i32 + 1 => {
                                    b.data.out_row.as_ref().and_then(|o| o.1).unwrap_or(self_focus)
                                }
                                _ => self_focus,
                            });
                        }
                    }
                }

                // Draw order (Qt z-levels): non-focused groups first (dimmed when a
                // group is focused), the focused group on top with a solid line.
                let others_op = if focus.is_some() { 0.12 } else { 1.0 };
                let others = || (0..boxes.len()).filter(|i| focus != Some(*i));
                for i in others() {
                    draw_ent_line(&painter, boxes[i].anchor, boxes[i].rect, others_op, false);
                }
                for i in others() {
                    let b = &boxes[i];
                    draw_ent_box(&painter, b.rect, &b.data, b.in_count, z, others_op, false, -1, &self.icon_cache);
                }
                for i in others() {
                    draw_ent_diamond(&painter, boxes[i].anchor, others_op, false);
                }
                if let Some(i) = focus {
                    let b = &boxes[i];
                    draw_ent_line(&painter, b.anchor, b.rect, 1.0, true);
                    draw_ent_box(&painter, b.rect, &b.data, b.in_count, z, 1.0, true, focus_row, &self.icon_cache);
                    draw_ent_diamond(&painter, b.anchor, 1.0, true);
                    hovered_ent_label = Some(b.data.title.to_string());
                }
            }

            // Overlay d'aide / nom survolé (objet, puis entrée, sinon aide).
            let corner = rect.min + vec2(8.0, 8.0);
            if let Some(o) = hovered {
                let name = self.i18n.tr_object(o.name);
                // Same reveal gate as the object tree: hide the contained item on
                // undiscovered objects unless "Reveal items" is on.
                let reveal = o.collected || self.dashboard.reveal;
                let world = &self.worlds[aw];
                let text = match world.items.get(o.location).filter(|_| reveal) {
                    Some(item) => {
                        let it = self.i18n.tr_item(item);
                        // Tag the destination player only when the item is routed to
                        // another world (multiworld); own-world items read plainly.
                        match world.dest.get(o.location).filter(|w| **w != (aw as u8 + 1)) {
                            Some(w) => format!(
                                "{name}  ·  {}  →  {it} ({} {w})",
                                o.location, self.i18n.player()
                            ),
                            _ => format!("{name}  ·  {}  →  {it}", o.location),
                        }
                    }
                    None => format!("{name}  ·  {}", o.location),
                };
                draw_label(&painter, corner, &text, Color32::from_rgb(240, 220, 150));
            } else if let Some(name) = &hovered_ent_label {
                draw_label(
                    &painter,
                    corner,
                    &format!("{} : {name}", self.i18n.entry()),
                    Color32::from_rgb(140, 220, 235),
                );
            } else {
                draw_label(&painter, corner, self.i18n.map_help_hint(), Color32::from_gray(200));
            }
        });

        if let Some((g, sc, eid)) = ent_nav {
            self.focus_entrance_in_scene(g, sc, eid);
        }
        // Re-centre on a clicked diamond's info box (skips view re-init, so the
        // pan sticks instead of being snapped back to the entrance anchor).
        if let Some(pan) = center_pan {
            self.pan = pan;
            self.view_initialized = true;
        }
    }
}

/// Scale a colour's alpha by `op` (group dimming for the entrance overlay).
fn fade(r: u8, g: u8, b: u8, a: u8, op: f32) -> Color32 {
    Color32::from_rgba_unmultiplied(r, g, b, (a as f32 * op).clamp(0.0, 255.0) as u8)
}

/// The row index under an item-local Y (map units) inside an entrance box:
/// 0 = title, 1..=in_count = the in rows, in_count+1 = the out row, -1 = none.
/// Mirrors EntranceGroupBoxItem::RowFromY.
fn row_from_y(y_map: f32, has_in: bool, has_out: bool, in_count: usize) -> i32 {
    if y_map < EB_TITLE_H {
        return 0;
    }
    let mut top = EB_TITLE_H;
    if has_in {
        for i in 0..in_count {
            if y_map < top + EB_ROW_H {
                return 1 + i as i32;
            }
            top += EB_ROW_H;
        }
    }
    if has_out && y_map < top + EB_ROW_H {
        return 1 + if has_in { in_count as i32 } else { 0 };
    }
    -1
}

/// Draw the dashed (or solid, when focused) connector from an anchor to the
/// nearest point on the box perimeter (Qt EntranceGroupBoxItem::RebuildCurve).
fn draw_ent_line(painter: &egui::Painter, anchor: egui::Pos2, rect: Rect, op: f32, solid: bool) {
    let conn = pos2(
        anchor.x.clamp(rect.left(), rect.right()),
        anchor.y.clamp(rect.top(), rect.bottom()),
    );
    if solid {
        painter.line_segment([anchor, conn], Stroke::new(2.2_f32, fade(255, 255, 255, 220, op)));
    } else {
        painter.extend(egui::Shape::dashed_line(
            &[anchor, conn],
            Stroke::new(1.6_f32, fade(255, 255, 255, 90, op)),
            5.0,
            4.0,
        ));
    }
}

/// Draw the amber diamond anchor (white when focused) at the entrance position.
fn draw_ent_diamond(painter: &egui::Painter, c: egui::Pos2, op: f32, hi: bool) {
    let r = if hi { 8.0 } else { 6.5 };
    let pts = vec![
        c + vec2(0.0, -r),
        c + vec2(r, 0.0),
        c + vec2(0.0, r),
        c + vec2(-r, 0.0),
    ];
    let fill = if hi {
        fade(255, 255, 255, 255, op)
    } else {
        fade(239, 191, 4, 235, op)
    };
    painter.add(egui::Shape::convex_polygon(
        pts,
        fill,
        Stroke::new(1.2_f32, fade(0, 0, 0, 180, op)),
    ));
}

/// Paint one grouped entrance box (Qt EntranceGroupBoxItem::paint): dark glass
/// background, title row (icon + name), green in rows and the red out row, with
/// a per-row hover tint. All metrics scale by `z` (the view zoom).
#[allow(clippy::too_many_arguments)]
fn draw_ent_box(
    painter: &egui::Painter,
    rect: Rect,
    data: &EntranceBoxData,
    in_count: usize,
    z: f32,
    op: f32,
    hi: bool,
    hovered_row: i32,
    icons: &HashMap<&'static str, Option<egui::TextureHandle>>,
) {
    // Background + border.
    let bg = if hi { fade(14, 14, 30, 250, op) } else { fade(6, 6, 18, 230, op) };
    painter.rect_filled(rect, 2.0, bg);
    let border = if hi { fade(255, 255, 255, 90, op) } else { fade(255, 255, 255, 28, op) };
    painter.rect_stroke(rect, 2.0, Stroke::new(if hi { 1.4_f32 } else { 1.0_f32 }, border));

    let left = rect.left();
    let pad = EB_PAD_X * z;

    // ── Title row: entrance icon + name ──
    let mut title_x = left + pad;
    let icon_sz = EB_TITLE_ICON * z;
    if let Some(tex) = scene::entrance_icon_path(data.icon)
        .and_then(|p| icons.get(p))
        .and_then(|t| t.as_ref())
    {
        let iy = rect.top() + (EB_TITLE_H * z - icon_sz) * 0.5;
        let ir = Rect::from_min_size(pos2(left + pad, iy), Vec2::splat(icon_sz));
        painter.image(
            tex.id(),
            ir,
            Rect::from_min_max(pos2(0.0, 0.0), pos2(1.0, 1.0)),
            fade(255, 255, 255, 255, op),
        );
        title_x += icon_sz + EB_ICON_GAP * z;
    }
    let title_col = if hi { fade(255, 255, 255, 255, op) } else { fade(255, 255, 255, 200, op) };
    elided_line(
        painter,
        title_x,
        rect.top() + EB_TITLE_H * z * 0.5,
        rect.right() - pad - title_x,
        data.title.as_str(),
        EB_TITLE_FONT * z,
        title_col,
        false,
    );

    // Title separator.
    let sep_y = rect.top() + EB_TITLE_H * z;
    painter.line_segment(
        [pos2(left, sep_y), pos2(rect.right(), sep_y)],
        Stroke::new(1.0_f32, fade(255, 255, 255, 22, op)),
    );

    // ── In / out rows ──
    let draw_row = |y: f32, accent: Color32, text_col: Color32, text: &str, up: bool, row_hovered: bool| {
        let rh = EB_ROW_H * z;
        if row_hovered {
            painter.rect_filled(
                Rect::from_min_size(pos2(left + EB_ACCENT_W * z, y), vec2(rect.width() - EB_ACCENT_W * z, rh)),
                0.0,
                fade(255, 255, 255, 22, op),
            );
        }
        // Left accent strip (green = in, red = out).
        painter.rect_filled(Rect::from_min_size(pos2(left, y), vec2(EB_ACCENT_W * z, rh)), 0.0, accent);
        // In/out triangle marker, drawn as a polygon (the default egui font lacks
        // the ▲ / ▼ glyphs and would render them as "tofu" squares).
        let cy = y + rh * 0.5;
        let tx = left + pad;
        let ts = 3.2 * z;
        let tri = if up {
            vec![pos2(tx, cy + ts), pos2(tx + ts * 2.0, cy + ts), pos2(tx + ts, cy - ts)]
        } else {
            vec![pos2(tx, cy - ts), pos2(tx + ts * 2.0, cy - ts), pos2(tx + ts, cy + ts)]
        };
        painter.add(egui::Shape::convex_polygon(tri, text_col, Stroke::NONE));
        // Row text after the triangle, elided to the box width.
        let text_x = tx + ts * 2.0 + 4.0 * z;
        elided_line(painter, text_x, cy, rect.right() - pad - text_x, text, EB_ROW_FONT * z, text_col, false);
    };

    let green_a = fade(61, 220, 132, 255, op);
    let green_t = fade(101, 224, 154, 255, op);
    let red_a = fade(255, 82, 82, 255, op);
    let red_t = fade(255, 144, 144, 255, op);

    let mut y = sep_y;
    if data.has_in {
        for (idx, (t, _)) in data.in_rows.iter().enumerate() {
            draw_row(y, green_a, green_t, t, true, hovered_row == 1 + idx as i32);
            y += EB_ROW_H * z;
        }
    }
    if let Some((t, _)) = &data.out_row {
        let out_idx = 1 + if data.has_in { in_count as i32 } else { 0 };
        draw_row(y, red_a, red_t, t, false, hovered_row == out_idx);
    }
}
