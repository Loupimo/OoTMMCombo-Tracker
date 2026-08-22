//! The GPS page: cross-game route finder over discovered entrance links.
use eframe::egui::{self, vec2, Color32};

use crate::*;
use crate::scene::Game;

impl TrackerApp {

    /// GPS sub-tab: pick a start and a destination scene + entrance (in either
    /// game) and show the shortest discovered route, crossing OoT<->MM if needed.
    pub(crate) fn draw_gps_page(&mut self, ctx: &egui::Context) {
        let scene_label = |sel: Option<(Game, u16)>| {
            sel.and_then(|(g, s)| g.scenes().iter().find(|d| d.id == s).map(|d| d.name))
                .unwrap_or("?")
        };
        egui::CentralPanel::default().show(ctx, |ui| {
            accent_heading(ui, self.i18n.gps_route_title());
            ui.separator();

            let (dep, arr) = (format!("{} :", self.i18n.departure()), format!("{} :", self.i18n.arrival()));
            egui::Grid::new("gps_grid").spacing(vec2(8.0, 8.0)).show(ui, |ui| {
                ui.label(&dep);
                gps_scene_combo(ui, &self.i18n, "gps_from_sc", &mut self.gps_from, &mut self.gps_from_ent);
                gps_entrance_combo(ui, &self.i18n, "gps_from_en", self.gps_from, &mut self.gps_from_ent);
                ui.end_row();
                ui.label(&arr);
                gps_scene_combo(ui, &self.i18n, "gps_to_sc", &mut self.gps_to, &mut self.gps_to_ent);
                gps_entrance_combo(ui, &self.i18n, "gps_to_en", self.gps_to, &mut self.gps_to_ent);
                ui.end_row();
            });
            ui.separator();

            let (Some(from), Some(to)) = (self.gps_from, self.gps_to) else {
                ui.weak(self.i18n.choose_route_scenes());
                return;
            };
            // Optional start / arrival entrance names, for context.
            let from_ent = self.gps_from_ent.and_then(|e| entrance::lookup(from.0, e)).map(|d| self.i18n.tr_entrance(d.to_name));
            let to_ent = self.gps_to_ent.and_then(|e| entrance::lookup(to.0, e)).map(|d| self.i18n.tr_entrance(d.to_name));
            if let Some(fe) = from_ent {
                ui.label(self.i18n.gps_from_entrance(fe, scene_label(Some(from))));
            }

            match gps::route_cross(from, to, &self.out_links) {
                Some(steps) if steps.is_empty() => {
                    ui.label(self.i18n.gps_already_there());
                }
                Some(steps) => {
                    ui.label(egui::RichText::new(self.i18n.gps_steps(steps.len())).strong());
                    egui::ScrollArea::vertical().id_salt("gps_route").show(ui, |ui| {
                        for (i, st) in steps.iter().enumerate() {
                            let n = st
                                .game
                                .scenes()
                                .iter()
                                .find(|s| s.id == st.scene)
                                .map(|s| s.name)
                                .unwrap_or("?");
                            let tag = if st.game == Game::Oot { "OoT" } else { "MM" };
                            ui.label(self.i18n.gps_step_line(i + 1, self.i18n.tr_entrance(st.entrance), tag, n));
                        }
                        if let Some(te) = to_ent {
                            ui.label(
                                egui::RichText::new(self.i18n.gps_arrival_line(te)).color(ACCENT),
                            );
                        }
                    });
                }
                None => {
                    ui.colored_label(Color32::from_rgb(220, 120, 90), self.i18n.gps_no_route());
                }
            }
        });
    }
}
