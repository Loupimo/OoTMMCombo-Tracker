//! The Entrance tab: scene/region tree, the global entrance table, the
//! OoT/MM/GPS sub-tabs and the scene-focus navigation helpers.
use eframe::egui::{self, pos2, vec2, Align2, Color32, FontId, Rect, Sense, Stroke};

use crate::*;
use crate::scene::{self, Game, LiveScene};

impl TrackerApp {

    /// Right panel of the entrance tab: a collapsible tree of the scene's
    /// entrances (Qt entrance tree), built like the object tree — a "Find…"
    /// filter, an expand/collapse-all button, and one collapsible full-width
    /// header per entrance (icon + name) whose children are the green "how you
    /// arrive" rows and the red "where it leads" row. `None`-type entrances are
    /// hidden. Every child row focuses the matching scene / entrance on click.
    pub(crate) fn draw_entrance_tree(&mut self, ctx: &egui::Context) {
        // Deferred navigation: the click handlers run inside the panel closure,
        // which borrows `self`; apply the jump once it returns.
        let mut nav: Option<(Game, u16, u32)> = None;
        let mut set_all: Option<bool> = None; // "expand/collapse all" this frame
        egui::SidePanel::right("enttree")
            .resizable(true)
            .default_width(320.0)
            .show(ctx, |ui| {
                accent_heading(ui, self.i18n.entrance());
                let Some(scene) = self.scene.as_ref() else { return };
                let (game, sid) = (scene.game, scene.def.id);

                // Entrances of this scene under the active layout (skip the None
                // type — end-credits / no-entry placeholders — and nameless rows),
                // and how many have been walked (header count, like the objects).
                let entrances: Vec<&'static data::EntranceDef> = game
                    .entrances()
                    .iter()
                    .filter(|e| {
                        e.to_scene == sid
                            && e.type_ != data::EntranceType::None
                            && !e.from_name.is_empty()
                            && tracking::scene_layout_active(e.layout, game, e.to_scene, &self.mq_scenes)
                    })
                    .collect();
                let visited = entrances
                    .iter()
                    .filter(|e| self.visited_entrances.contains(&(game, e.to_id)))
                    .count();
                ui.label(format!("{visited} / {}", entrances.len()));
                progress_bar(ui, visited, entrances.len(), game_accent(game));
                ui.add_space(2.0);

                // Live filter (Qt "Find…") + expand / collapse all, like the object tree.
                ui.add(
                    egui::TextEdit::singleline(&mut self.ent_search)
                        .hint_text(self.i18n.search())
                        .desired_width(f32::INFINITY),
                );
                let label = if self.ent_all_expanded {
                    format!("⊟ {}", self.i18n.collapse_all())
                } else {
                    format!("⊞ {}", self.i18n.expand_all())
                };
                if ui.add(egui::Button::new(label).min_size(vec2(ui.available_width(), 0.0))).clicked() {
                    self.ent_all_expanded = !self.ent_all_expanded;
                    set_all = Some(self.ent_all_expanded);
                }
                ui.separator();

                let query = self.ent_search.trim().to_lowercase();
                let green = Color32::from_rgb(101, 224, 154);
                let red = Color32::from_rgb(255, 144, 144);
                let accent = game_accent(game);
                egui::ScrollArea::vertical().id_salt("enttree_s").show(ui, |ui| {
                    ui.spacing_mut().item_spacing.y = 2.0;
                    for e in entrances {
                        let data = entrance_box_data(game, e, &self.in_links, &self.out_links);

                        // Search matches the entrance name or any in/out row text.
                        if !query.is_empty() {
                            let hit = data.title.to_lowercase().contains(&query)
                                || data.in_rows.iter().any(|(t, _)| t.to_lowercase().contains(&query))
                                || data.out_row.as_ref().is_some_and(|(t, _)| t.to_lowercase().contains(&query));
                            if !hit {
                                continue;
                            }
                        }

                        let visited = self.visited_entrances.contains(&(game, e.to_id));
                        // Persisted open state: a search forces every node open;
                        // "expand/collapse all" overrides + persists; else remembered.
                        let id = ui.make_persistent_id(("enttree", game.idx(), e.to_id));
                        let mut open = if !query.is_empty() {
                            true
                        } else if let Some(v) = set_all {
                            ui.data_mut(|d| d.insert_persisted(id, v));
                            v
                        } else {
                            ui.data_mut(|d| d.get_persisted::<bool>(id)).unwrap_or(true)
                        };

                        // Collapsible header: full-width tinted row, entrance icon,
                        // name dimmed until the entrance has been walked.
                        let icon_tex = scene::entrance_icon_path(e.icon)
                            .and_then(|p| self.icon_cache.get(p))
                            .and_then(|t| t.as_ref())
                            .map(|t| t.id());
                        let col = if visited { Color32::from_rgb(230, 240, 255) } else { Color32::from_gray(190) };
                        let header = tinted_row(
                            ui, 26.0, 36.0, accent.linear_multiply(0.10),
                            data.title, col, None, Some(open), icon_tex,
                        );
                        if header.clicked() {
                            open = !open;
                            ui.data_mut(|d| d.insert_persisted(id, open));
                        }

                        if open {
                            // Green in rows + red out row: full-width indented leaves.
                            let mut leaf = |ui: &mut egui::Ui, c: Color32, text: &str, target: Option<(Game, u16, u32)>| {
                                let r = tinted_row(ui, 22.0, 46.0, Color32::TRANSPARENT, text, c, None, None, None);
                                if r.clicked() {
                                    if let Some(t) = target {
                                        nav = Some(t);
                                    }
                                }
                            };
                            for (t, target) in &data.in_rows {
                                leaf(ui, green, t, *target);
                            }
                            if let Some((t, target)) = &data.out_row {
                                leaf(ui, red, t, *target);
                            }
                        }
                    }
                });
            });

        if let Some((g, sc, eid)) = nav {
            self.focus_entrance_in_scene(g, sc, eid);
        }
    }

    /// The global entrance table for a region (Qt AllEntranceView): one row per
    /// entrance of the region's scenes — Scene / Entrance / How to spawn here? /
    /// Where does it lead? — with a red/yellow/green status and clickable rows
    /// that load the scene and centre the minimap on the entrance.
    pub(crate) fn draw_entrance_table(&mut self, ctx: &egui::Context, game: Game, region: u8) {
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

        let region_name: String = if all {
            let full = if game == Game::Oot { "Ocarina of Time" } else { "Majora's Mask" };
            self.i18n.entrance_all_game(full)
        } else {
            game.scenes()
                .iter()
                .find(|s| s.region_id == region)
                .map(|s| s.region_name)
                .unwrap_or("—")
                .to_string()
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
                    // Entrance name = the side facing away from this scene (Qt
                    // formatEntrance -> FromName), e.g. "Fire Temple" in Death
                    // Mountain Crater — same nomenclature as the map boxes.
                    entrance: e.from_name,
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
        let mut used_rest = 240.0_f32; // the divisible width, for the resize maths

        egui::CentralPanel::default().show(ctx, |ui| {
            ui.horizontal(|ui| {
                accent_heading(ui, &region_name);
                ui.label(egui::RichText::new(format!("· {}", self.i18n.entrance_count(n))).weak());
            });
            ui.separator();

            egui::ScrollArea::vertical().id_salt("enttable").show(ui, |ui| {
                // Fixed status column + Scene/Entrance/spawn/leads sharing the rest
                // by the (resizable) fractions. Header and rows use the SAME widths
                // with no gaps, so columns line up; the resize grips are overlays.
                let avail = ui.available_width();
                let rest = (avail - 26.0).max(240.0);
                used_rest = rest;
                let ws = [26.0_f32, rest * frac[0], rest * frac[1], rest * frac[2], rest * frac[3]];
                let headers = [
                    self.i18n.entrance_col_scene().to_string(),
                    self.i18n.entry().to_string(),
                    self.i18n.entrance_col_spawn().to_string(),
                    self.i18n.entrance_col_leads().to_string(),
                ];

                // Cumulative x-offset of column `k` (ws index: 0=status … 4=leads).
                let x_off = |k: usize| -> f32 { ws[..k].iter().sum::<f32>() };
                // Inset text rect for column `k` at `base`, leaving 4px padding.
                let cell_rect = |base: egui::Pos2, k: usize, h: f32| -> Rect {
                    Rect::from_min_size(base + vec2(x_off(k) + 4.0, 0.0), vec2((ws[k] - 8.0).max(1.0), h))
                };

                // Header row (clickable sort labels), then resize grips drawn on top.
                // Each header cell is pinned to its column's absolute rect so it
                // lines up exactly with the separators and the data below.
                let (head_rect, _) = ui.allocate_exact_size(vec2(avail, 24.0), Sense::hover());
                for c in 0..4 {
                    let arrow = if sort_col == c {
                        if asc { " ▲" } else { " ▼" }
                    } else {
                        ""
                    };
                    let cr = cell_rect(head_rect.min, c + 1, 24.0);
                    let mut child = ui.new_child(
                        egui::UiBuilder::new().max_rect(cr).layout(egui::Layout::left_to_right(egui::Align::Center)),
                    );
                    child.set_clip_rect(cr);
                    let r = child.add(
                        egui::Label::new(
                            egui::RichText::new(format!("{}{arrow}", headers[c])).strong().color(ACCENT),
                        )
                        .truncate()
                        .sense(Sense::click()),
                    );
                    if r.clicked() {
                        sort_click = Some(c);
                    }
                }

                // Resize grips: interactive overlays sitting on the 3 inner column
                // boundaries (they don't take layout space, so nothing shifts).
                let mut cum = head_rect.min.x + ws[0];
                for c in 0..4 {
                    cum += ws[c + 1];
                    if c < 3 {
                        let grip = Rect::from_min_max(
                            pos2(cum - 3.0, head_rect.min.y),
                            pos2(cum + 3.0, head_rect.max.y),
                        );
                        let gid = ui.make_persistent_id(("entgrip", c));
                        let gr = ui.interact(grip, gid, Sense::drag());
                        let col = if gr.hovered() || gr.dragged() { ACCENT } else { sep };
                        ui.painter().vline(cum, head_rect.y_range(), Stroke::new(1.0_f32, col));
                        if gr.hovered() || gr.dragged() {
                            ui.ctx().set_cursor_icon(egui::CursorIcon::ResizeHorizontal);
                        }
                        if gr.dragged() {
                            resize = Some((c, gr.drag_delta().x));
                        }
                    }
                }
                ui.separator();

                // Per-row vertical separators sit on the same column boundaries.
                let seps = [ws[0] + ws[1], ws[0] + ws[1] + ws[2], ws[0] + ws[1] + ws[2] + ws[3]];

                for (i, r) in data.iter().enumerate() {
                    let (row_rect, _) = ui.allocate_exact_size(vec2(avail, 22.0), Sense::hover());
                    let base = row_rect.min;
                    if i % 2 == 1 {
                        ui.painter().rect_filled(row_rect, 0.0, stripe);
                    }
                    for sx in seps {
                        ui.painter().vline(
                            base.x + sx,
                            row_rect.y_range(),
                            Stroke::new(1.0_f32, Color32::from_gray(58)),
                        );
                    }
                    // Status dot: painted centred in the 26px status column.
                    ui.painter().text(
                        Rect::from_min_size(base, vec2(ws[0], 22.0)).center(),
                        Align2::CENTER_CENTER,
                        "●",
                        FontId::proportional(12.0),
                        r.dot,
                    );
                    // Scene (plain, truncated).
                    table_cell_at(ui, cell_rect(base, 1, 22.0), |ui| {
                        ui.add(egui::Label::new(r.scene).truncate());
                    });
                    // Entrance (clickable link → focus that entrance).
                    if table_link_cell_at(ui, cell_rect(base, 2, 22.0), r.entrance).clicked() {
                        focus = Some(r.ent_target);
                    }
                    // "How to arrive" (spawn) and "Where it leads" columns.
                    for (cell, k) in [(&r.spawn, 3usize), (&r.leads, 4usize)] {
                        let cr = cell_rect(base, k, 22.0);
                        match cell {
                            EntCell::Na => table_cell_at(ui, cr, |ui| {
                                ui.colored_label(na, "N/A");
                            }),
                            EntCell::Unknown => table_cell_at(ui, cr, |ui| {
                                ui.weak("?");
                            }),
                            EntCell::Link(name, target) => {
                                if table_link_cell_at(ui, cr, name).clicked() {
                                    focus = Some(*target);
                                }
                            }
                        }
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
            // Move width between the two adjacent columns, keeping both >= 8%.
            let df = dx / used_rest;
            let a = self.ent_col_frac[c] + df;
            let b = self.ent_col_frac[c + 1] - df;
            if a >= 0.08 && b >= 0.08 {
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
    pub(crate) fn focus_entrance_in_scene(&mut self, game: Game, scene_id: u16, entrance_id: u32) {
        if self.scene.as_ref().map_or(true, |s| s.game != game || s.def.id != scene_id) {
            if let Some(def) = game.scenes().iter().find(|s| s.id == scene_id) {
                self.scene = Some(LiveScene::load(game, def, &self.mq_scenes));
                self.current_room = 0;
                self.map_texture = None;
                self.load_error = None;
            }
        }
        self.sel_scene[game.idx()] = scene_id;
        // Follow the entrance to its own game so a cross-game jump (an OoT door
        // that leads into MM, say) lands on the matching sub-tab and nav tree.
        self.entrance_sub = if game == Game::Oot { EntranceSub::Oot } else { EntranceSub::Mm };
        self.entrance_table = None; // switch the centre from the table to the minimap
        self.focus_entrance = Some(entrance_id);
        self.view_initialized = false; // let draw_map recentre on the entrance
    }

    /// Navigate from a location leaf to its object's scene (switch to the game's
    /// tab and load the render scene), mirroring NavigateToObject.
    pub(crate) fn navigate_to(&mut self, game: Game, scene_id: u16) {
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

    /// (visited, total) entrances of a game (active layout, named), for the sub-tab
    /// counters. Visited = discovered live (`visited_entrances`).
    pub(crate) fn entrance_counts(&self, game: Game) -> (usize, usize) {
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
    pub(crate) fn draw_entrance_subtabs(&mut self, ctx: &egui::Context) {
        let oot = self.entrance_counts(Game::Oot);
        let mm = self.entrance_counts(Game::Mm);
        egui::TopBottomPanel::top("entrance_subtabs").show(ctx, |ui| {
            ui.add_space(1.0);
            ui.horizontal(|ui| {
                ui.spacing_mut().item_spacing.x = 6.0;
                for (sub, text) in [
                    (EntranceSub::Oot, format!("OoT  {}/{}", oot.0, oot.1)),
                    (EntranceSub::Mm, format!("MM  {}/{}", mm.0, mm.1)),
                    (EntranceSub::Gps, self.i18n.gps().to_string()),
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
}
