//! The GPS page: cross-game route finder over the weighted entrance graph.
//! Shows up to three fastest scene routes, each as a card with a total time,
//! transition count and a per-hop breakdown (mirrors the Qt GPS panel).
use eframe::egui::{self, vec2, Color32, RichText};

use crate::scene::Game;
use crate::*;

/// Format seconds as `m:ss` (matches the Qt route headers).
fn fmt_mmss(secs: u32) -> String {
    format!("{}:{:02}", secs / 60, secs % 60)
}

/// Translate a step's exit label: real entrance names go through `tr_entrance`,
/// the fixed pseudo-exits use their own locale strings.
fn via_label(i18n: &I18n, via: &str) -> String {
    match via {
        "Walk" => i18n.gps_via_walk().to_string(),
        "Warp Song" => i18n.gps_via_warp_song().to_string(),
        "Warp Owl" => i18n.gps_via_warp_owl().to_string(),
        other => i18n.tr_entrance(other).to_string(),
    }
}

impl TrackerApp {
    /// GPS sub-tab: pick a start and a destination scene + entrance (in either
    /// game) and show the fastest discovered routes, crossing OoT<->MM if needed.
    pub(crate) fn draw_gps_page(&mut self, ctx: &egui::Context) {
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

            // Recompute only when an input changed; the graph build + Yen search
            // is far too heavy for every frame.
            let cross_oot = self.rom_settings.value("crossWarpOot") == data::ShuffleSetting::all;
            let cross_mm = self.rom_settings.value("crossWarpMm") == data::ShuffleSetting::all;
            let key = gps::GpsKey {
                from,
                to,
                from_ent: self.gps_from_ent,
                to_ent: self.gps_to_ent,
                cross_oot,
                cross_mm,
                links: gps::links_hash(&self.out_links),
            };
            if self.gps_cache.as_ref().is_none_or(|(k, _)| *k != key) {
                let result = gps::find_routes(
                    from,
                    to,
                    self.gps_from_ent,
                    self.gps_to_ent,
                    3,
                    cross_oot,
                    cross_mm,
                    &self.out_links,
                );
                self.gps_cache = Some((key, result));
            }
            let result = &self.gps_cache.as_ref().unwrap().1;

            match result.status {
                gps::GpsStatus::SameScene => {
                    ui.label(self.i18n.gps_already_there());
                }
                gps::GpsStatus::NoRoute => {
                    ui.colored_label(Color32::from_rgb(220, 120, 90), self.i18n.gps_no_route());
                }
                gps::GpsStatus::Ok => {
                    egui::ScrollArea::vertical().id_salt("gps_routes").show(ui, |ui| {
                        for (idx, route) in result.routes.iter().enumerate() {
                            self.route_card(ui, idx, route);
                            ui.add_space(6.0);
                        }
                    });
                }
            }
        });
    }

    /// Render one route as a bordered card: header (total time / rank / transition
    /// count) followed by the ordered scene stations and their exit + hop time.
    fn route_card(&self, ui: &mut egui::Ui, idx: usize, route: &gps::GpsRoute) {
        egui::Frame::group(ui.style())
            .fill(crate::BG_PANEL)
            .rounding(6.0)
            .inner_margin(egui::Margin::same(8.0))
            .show(ui, |ui| {
                ui.horizontal(|ui| {
                    ui.label(RichText::new(fmt_mmss(route.total)).strong().size(18.0).color(ACCENT));
                    let tag = if idx == 0 {
                        self.i18n.gps_fastest().to_string()
                    } else {
                        self.i18n.gps_alternative(idx)
                    };
                    ui.label(RichText::new(tag).weak());
                    ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                        ui.weak(self.i18n.gps_transitions(route.transitions()));
                    });
                });
                ui.add_space(4.0);

                let n = route.steps.len();
                // Only tag stations with their game when the route actually crosses
                // OoT <-> MM; a single-game route stays uncluttered like the Qt panel.
                let cross_game = route.steps.iter().any(|s| s.game != route.steps[0].game);
                for (i, st) in route.steps.iter().enumerate() {
                    let name = scene_display_name(&self.i18n, st.game, st.scene);
                    let gtag = if st.game == Game::Oot { "OoT" } else { "MM" };
                    ui.horizontal(|ui| {
                        ui.label(RichText::new(format!("{}.", i + 1)).weak().monospace());
                        if cross_game {
                            ui.label(RichText::new(gtag).small().color(game_accent(st.game)));
                        }
                        if i + 1 == n {
                            ui.label(RichText::new(name.as_str()).strong().color(ACCENT));
                        } else {
                            ui.label(name.as_str());
                            ui.label(RichText::new("→").weak());
                            ui.label(
                                RichText::new(via_label(&self.i18n, &st.via)).color(game_accent(st.game)),
                            );
                            ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                                ui.weak(format!("~{}s", st.cost));
                            });
                        }
                    });
                }
            });
    }
}
