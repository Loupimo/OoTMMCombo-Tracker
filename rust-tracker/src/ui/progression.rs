//! The Progression dashboard: item grid, per-item detail panel and icons.
use eframe::egui::{self, vec2, Align2, Color32, FontId, Sense, Stroke};

use crate::*;
use crate::scene::Game;

impl TrackerApp {

    /// The progression dashboard (ProgressionTab): four sub-tabs of item-icon
    /// grids on the left, a detail panel with the selected item's location tree
    /// on the right.
    pub(crate) fn draw_progression_tab(&mut self, ctx: &egui::Context) {
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
                    ui.label(format!("{} :", self.i18n.prog_world()));
                    egui::ComboBox::from_id_salt("prog_world")
                        .selected_text(format!("{} {}", self.i18n.player(), self.dashboard.active_world))
                        .show_ui(ui, |ui| {
                            for w in 1..=num_worlds {
                                ui.selectable_value(&mut new_world, w, format!("{} {w}", self.i18n.player()));
                            }
                        });
                }
                ui.separator();
                let mut reveal = self.dashboard.reveal;
                if ui
                    .checkbox(&mut reveal, self.i18n.prog_reveal())
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
    pub(crate) fn prog_icon_tex(&self, icon: &str) -> Option<&egui::TextureHandle> {
        let path = data::ICON_BY_NAME
            .binary_search_by_key(&icon, |&(n, _)| n)
            .ok()
            .map(|i| data::ICON_BY_NAME[i].1)?;
        self.icon_cache.get(path)?.as_ref()
    }

    /// The left grid: each visible section of the active page as a header + a
    /// wrapped row of icon cells (BuildPage). Sets `new_selected` on a click.
    pub(crate) fn draw_prog_grid(&self, ui: &mut egui::Ui, new_selected: &mut Option<usize>) {
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
    pub(crate) fn draw_prog_cell(&self, ui: &mut egui::Ui, i: usize, new_selected: &mut Option<usize>) {
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
                ui.label(egui::RichText::new(self.i18n.tr_prog_entry(e.name, e.lookup_keys)).small().color(name_col));
            },
        );

        let resp = inner.response.interact(Sense::click());
        if resp.clicked() {
            *new_selected = Some(i);
        }
        let mut tip = self.i18n.tr_prog_entry(e.name, e.lookup_keys).to_string();
        if st.is_starting {
            tip.push_str(&format!("\n★ {}", self.i18n.prog_starting_item()));
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
    pub(crate) fn draw_prog_detail(&self, ui: &mut egui::Ui, nav: &mut Option<(Game, u16)>) {
        ui.add_space(6.0);
        let Some(i) = self.dashboard.selected else {
            ui.weak(self.i18n.prog_select_item());
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
            ui.label(egui::RichText::new(self.i18n.tr_prog_entry(e.name, e.lookup_keys)).heading().size(16.0));
            if st.found {
                ui.colored_label(Color32::from_rgb(101, 224, 154), format!("✔ {}", self.i18n.prog_found()));
            } else {
                ui.colored_label(Color32::from_rgb(255, 105, 105), format!("✘ {}", self.i18n.prog_not_found()));
            }
            if st.is_starting {
                ui.colored_label(Color32::from_rgb(248, 200, 120), format!("★ {}", self.i18n.prog_starting_item()));
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
        ui.label(egui::RichText::new(self.i18n.prog_locations()).strong().color(Color32::from_rgb(74, 158, 219)));
        ui.separator();

        let tree = self.dashboard.tree();
        egui::ScrollArea::vertical().id_salt("prog_locs").show(ui, |ui| {
            // Ellipsize long scene / location names instead of wrapping (like the
            // other trees).
            ui.style_mut().wrap_mode = Some(egui::TextWrapMode::Truncate);
            if tree.is_empty() {
                ui.weak(if self.dashboard.reveal {
                    self.i18n.prog_no_location()
                } else {
                    self.i18n.prog_not_found_yet()
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
}
