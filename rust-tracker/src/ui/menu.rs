//! Top bars: the Options menu, the category-filter popup, the tab bar and
//! its per-tab counters, and the bottom status bar.
use eframe::egui::{self, pos2, vec2, Color32, Rect, Sense};
use std::collections::HashSet;

use crate::*;
use crate::scene::Game;
use crate::i18n::Language;

impl TrackerApp {

    pub(crate) fn draw_menu_bar(&mut self, ctx: &egui::Context) {
        // Deferred actions (so the menu closures only borrow `self` immutably).
        let (mut act_save, mut act_load, mut act_spoiler, mut act_reset, mut act_toggle) =
            (false, false, false, false, false);
        // Global keyboard shortcuts (Qt: F8 / F10 / F11 / F3), active anytime.
        ctx.input_mut(|i| {
            act_save = i.consume_key(egui::Modifiers::NONE, egui::Key::F8);
            act_load = i.consume_key(egui::Modifiers::NONE, egui::Key::F10);
            act_spoiler = i.consume_key(egui::Modifiers::NONE, egui::Key::F11);
            act_toggle = i.consume_key(egui::Modifiers::NONE, egui::Key::F3);
        });
        let mut new_language = None;
        let mut set_reveal = None;
        let mut set_autosave = None;
        let (mut recenter, mut open_settings, mut open_about) = (false, false, false);
        // Persisted option toggles are edited on a clone, diffed + saved afterwards.
        let mut opts = self.app_settings.clone();

        egui::TopBottomPanel::top("menu").show(ctx, |ui| {
            egui::menu::bar(ui, |ui| {
                // Tracking menu (save / load / spoiler / reset / start-stop).
                ui.menu_button(self.i18n.menu_tracking(), |ui| {
                    if ui.add(egui::Button::new(self.i18n.save_tracking()).shortcut_text("F8")).clicked() {
                        act_save = true;
                        ui.close_menu();
                    }
                    if ui.add(egui::Button::new(self.i18n.load_tracking()).shortcut_text("F10")).clicked() {
                        act_load = true;
                        ui.close_menu();
                    }
                    if ui.add(egui::Button::new(self.i18n.load_spoiler()).shortcut_text("F11")).clicked() {
                        act_spoiler = true;
                        ui.close_menu();
                    }
                    ui.separator();
                    if ui.button(self.i18n.reset_tracking()).clicked() {
                        act_reset = true;
                        ui.close_menu();
                    }
                    let start_stop =
                        if self.tracking { self.i18n.stop_tracking() } else { self.i18n.start_tracking() };
                    if ui.add(egui::Button::new(start_stop).shortcut_text("F3")).clicked() {
                        act_toggle = true;
                        ui.close_menu();
                    }
                });

                // Options menu.
                ui.menu_button(self.i18n.options(), |ui| {
                    if ui.button(self.i18n.recenter_view()).clicked() {
                        recenter = true;
                        ui.close_menu();
                    }
                    ui.separator();
                    // View / map options.
                    ui.checkbox(&mut opts.auto_snap, self.i18n.opt_auto_snap());
                    ui.checkbox(&mut opts.auto_zoom, self.i18n.opt_auto_zoom());
                    let mut reveal = self.dashboard.reveal;
                    if ui.checkbox(&mut reveal, self.i18n.reveal_items()).changed() {
                        set_reveal = Some(reveal);
                    }
                    ui.menu_button(self.i18n.opt_hide_collected(), |ui| {
                        ui.checkbox(&mut opts.hide_collected_map, self.i18n.opt_from_map());
                        ui.checkbox(&mut opts.hide_collected_list, self.i18n.opt_from_list());
                    });
                    ui.separator();
                    // Live-follow options (driven by the player's current scene).
                    ui.checkbox(&mut opts.auto_follow_item, self.i18n.opt_follow_item());
                    ui.checkbox(&mut opts.auto_follow_entrance, self.i18n.opt_follow_entrance());
                    ui.checkbox(&mut opts.auto_gps_start, self.i18n.opt_gps_start());
                    ui.separator();
                    // Save options.
                    let mut autosave = self.auto_save;
                    if ui.checkbox(&mut autosave, self.i18n.auto_saving()).changed() {
                        set_autosave = Some(autosave);
                    }
                    ui.checkbox(&mut opts.backup_on_save, self.i18n.opt_backup());
                    ui.menu_button(self.i18n.opt_auto_load(), |ui| {
                        ui.checkbox(&mut opts.auto_load_tracking, self.i18n.opt_auto_load_tracking());
                        ui.checkbox(&mut opts.auto_load_spoiler, self.i18n.opt_auto_load_spoiler());
                    });
                    ui.separator();
                    ui.menu_button(self.i18n.lang(), |ui| {
                        for language in Language::ALL {
                            let selected = self.i18n.language() == language;
                            if ui.selectable_label(selected, language.label()).clicked() {
                                new_language = Some(language);
                                ui.close_menu();
                            }
                        }
                    });
                });

                if ui.button(self.i18n.rom_settings()).clicked() {
                    open_settings = true;
                }

                // Help menu.
                ui.menu_button("?", |ui| {
                    if ui.button(self.i18n.about()).clicked() {
                        open_about = true;
                        ui.close_menu();
                    }
                });
            });
        });

        // Apply the deferred actions.
        if act_save {
            self.save_tracking_dialog();
        }
        if act_load {
            self.load_tracking_dialog();
        }
        if act_spoiler {
            self.load_spoiler_dialog();
        }
        if act_reset {
            self.reset_tracking();
        }
        if act_toggle {
            self.toggle_tracking();
        }
        if recenter {
            self.view_initialized = false;
        }
        if let Some(v) = set_reveal {
            self.dashboard.set_reveal(v);
        }
        if let Some(v) = set_autosave {
            self.set_auto_save(v);
        }
        if let Some(language) = new_language {
            self.set_language(language);
        }
        // Persist any option toggle change (language already handled above, so we
        // sync it into `opts` before the diff to avoid a spurious extra save).
        opts.language = self.app_settings.language;
        if opts != self.app_settings {
            self.app_settings = opts;
            self.app_settings.save(&self.app_settings_path);
        }
        if open_settings {
            self.show_settings = true;
        }
        if open_about {
            self.show_about = true;
        }
        self.draw_about_window(ctx);
    }

    /// The "About" dialog (Qt ShowAboutDialog): app name, version and repo link.
    fn draw_about_window(&mut self, ctx: &egui::Context) {
        if !self.show_about {
            return;
        }
        let mut open = true;
        egui::Window::new(self.i18n.about())
            .collapsible(false)
            .resizable(false)
            .open(&mut open)
            .anchor(egui::Align2::CENTER_CENTER, vec2(0.0, 0.0))
            .show(ctx, |ui| {
                ui.vertical_centered(|ui| {
                    ui.add_space(4.0);
                    ui.label(egui::RichText::new("OoTMMCombo Auto Tracker").heading().color(ACCENT));
                    ui.label("Rust / egui build");
                    ui.label("© 2025-2026 Loupimo");
                    ui.add_space(6.0);
                    ui.hyperlink("https://github.com/Loupimo/OoTMMCombo-Tracker");
                    ui.add_space(4.0);
                });
            });
        self.show_about = open;
    }

    /// The category-filter popup: one checkbox per filterable object type of the
    /// active game, mirroring the Qt FilterManager button. Only meaningful on the
    /// OoT / MM item tabs.
    pub(crate) fn draw_filter_menu(&mut self, ui: &mut egui::Ui) {
        let Some(game) = self.active_tab.game() else {
            ui.weak(self.i18n.filter_needs_game());
            return;
        };
        let types = match game {
            Game::Oot => data::OOT_FILTER_TYPES,
            Game::Mm => data::MM_FILTER_TYPES,
        };
        // Precompute each row's category icon (TextureId is Copy) so the render
        // loop doesn't borrow self.icon_cache while we mutate the active set.
        let mut rows: Vec<(data::ObjectType, Option<egui::TextureId>, &'static str)> = types
            .iter()
            .map(|&t| {
                let tex = data::ICON_PATHS
                    .get(t as usize)
                    .copied()
                    .filter(|p| !p.is_empty())
                    .and_then(|p| self.icon_cache.get(p))
                    .and_then(|o| o.as_ref())
                    .map(|h| h.id());
                (t, tex, data::OBJ_TYPE_NAMES[t as usize])
            })
            .collect();
        rows.sort_by_key(|r| r.2); // by display name, like the Qt filter menu

        // Work on a local copy of the active set, written back at the end.
        let mut set = self.active_types[game.idx()].clone();

        ui.set_min_width(360.0);
        ui.horizontal(|ui| {
            if ui.button(self.i18n.all()).clicked() {
                set = types.iter().copied().collect();
            }
            if ui.button(self.i18n.none()).clicked() {
                set.clear();
            }
        });
        ui.separator();

        // Two columns with icons so the whole (~40-entry) list fits at once.
        let mid = rows.len().div_ceil(2);
        let row = |ui: &mut egui::Ui, t: data::ObjectType, tex: Option<egui::TextureId>, name: &str, set: &mut HashSet<data::ObjectType>| {
            ui.horizontal(|ui| {
                if let Some(id) = tex {
                    ui.add(egui::Image::new((id, vec2(18.0, 18.0))));
                } else {
                    ui.add_space(18.0);
                }
                let mut on = set.contains(&t);
                if ui.checkbox(&mut on, name).changed() {
                    if on {
                        set.insert(t);
                    } else {
                        set.remove(&t);
                    }
                }
            });
        };
        // Use all the room from here down to the bottom of the window so the whole
        // list shows at once; only fall back to a scrollbar when it would overflow.
        let max_h = (ui.ctx().screen_rect().bottom() - ui.cursor().top() - 12.0).max(240.0);
        egui::ScrollArea::vertical().max_height(max_h).show(ui, |ui| {
            ui.columns(2, |cols| {
                for (i, r) in rows.iter().enumerate() {
                    let col = if i < mid { &mut cols[0] } else { &mut cols[1] };
                    row(col, r.0, r.1, r.2, &mut set);
                }
            });
        });

        self.active_types[game.idx()] = set;
    }

    pub(crate) fn draw_tab_bar(&mut self, ctx: &egui::Context) {
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
                let frac = |(d, t): (usize, usize)| if t > 0 { d as f32 / t as f32 } else { 0.0 };
                for tab in Tab::ALL {
                    let selected = self.active_tab == tab;
                    // Per-tab count suffix (OoT / MM object totals, entrances).
                    let count = match tab {
                        Tab::Oot => Some(format!("{}/{}", oot.0, oot.1)),
                        Tab::Mm => Some(format!("{}/{}", mm.0, mm.1)),
                        Tab::Entrance => Some(format!("{ent_visited}/{ent_total}")),
                        _ => None,
                    };
                    // OoT / MM tabs carry a thin per-game progress bar in their accent.
                    let bar = match tab {
                        Tab::Oot => Some((frac(oot), game_accent(Game::Oot))),
                        Tab::Mm => Some((frac(mm), game_accent(Game::Mm))),
                        _ => None,
                    };
                    let selected_now = self.tab_button(ui, tab, selected, count.as_deref(), bar);
                    if selected_now && !selected {
                        self.active_tab = tab;
                        // The layer may change (item map vs minimap): reload.
                        self.map_texture = None;
                        self.view_initialized = false;
                        self.current_room = 0;
                    }
                }
                // Grand total on the far right, with a two-colour bar (blue = OoT
                // progress, purple = MM progress) over the combined total.
                ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                    ui.label(
                        egui::RichText::new(format!("{} {}/{}", self.i18n.total(), oot.0 + mm.0, oot.1 + mm.1))
                            .strong()
                            .color(Color32::from_rgb(221, 238, 255)),
                    );
                    ui.add_space(8.0);
                    let grand = (oot.1 + mm.1).max(1) as f32;
                    let (rect, _) = ui.allocate_exact_size(vec2(160.0, 10.0), Sense::hover());
                    let p = ui.painter();
                    p.rect_filled(rect, 3.0, Color32::from_gray(60));
                    let oot_w = rect.width() * (oot.0 as f32 / grand);
                    let mm_w = rect.width() * (mm.0 as f32 / grand);
                    p.rect_filled(
                        Rect::from_min_size(rect.min, vec2(oot_w, rect.height())),
                        3.0,
                        game_accent(Game::Oot),
                    );
                    p.rect_filled(
                        Rect::from_min_size(pos2(rect.left() + oot_w, rect.top()), vec2(mm_w, rect.height())),
                        0.0,
                        game_accent(Game::Mm),
                    );
                });
            });
            ui.add_space(2.0);
        });
    }

    /// One tab button: bold name + a small dimmed count, and an optional thin
    /// progress bar (`bar` = fraction + colour) along its bottom edge. Returns
    /// whether it was clicked.
    pub(crate) fn tab_button(
        &self,
        ui: &mut egui::Ui,
        tab: Tab,
        selected: bool,
        count: Option<&str>,
        bar: Option<(f32, Color32)>,
    ) -> bool {
        let name_col = if selected {
            Color32::from_rgb(221, 238, 255)
        } else {
            Color32::from_gray(190)
        };
        let mut job = egui::text::LayoutJob::default();
        job.append(
            tab.label(&self.i18n),
            0.0,
            egui::TextFormat {
                color: name_col,
                font_id: egui::FontId::proportional(15.5),
                ..Default::default()
            },
        );
        if let Some(c) = count {
            job.append(
                &format!("  {c}"),
                0.0,
                egui::TextFormat {
                    color: Color32::from_gray(150),
                    font_id: egui::FontId::proportional(12.0),
                    ..Default::default()
                },
            );
        }
        let resp = ui.selectable_label(selected, job);
        if let Some((frac, col)) = bar {
            let r = resp.rect;
            let track = Rect::from_min_max(pos2(r.left() + 6.0, r.bottom() - 3.0), pos2(r.right() - 6.0, r.bottom() - 1.0));
            let p = ui.painter();
            p.rect_filled(track, 1.0, Color32::from_gray(70));
            let f = frac.clamp(0.0, 1.0);
            if f > 0.0 {
                let fill = Rect::from_min_max(track.min, pos2(track.left() + track.width() * f, track.bottom()));
                p.rect_filled(fill, 1.0, col);
            }
        }
        resp.clicked()
    }

    pub(crate) fn draw_status_bar(&mut self, ctx: &egui::Context) {
        let green = Color32::from_rgb(120, 200, 90);
        let gray = Color32::from_gray(120);
        let tracking = self.tracking;
        let connected = self.connected;
        let status = self.status.clone();
        let last_item = self.last_item.clone();
        let last_entrance = self.last_entrance.clone();
        let reveal = self.dashboard.reveal;
        let auto_save = self.auto_save;
        // Deferred pill clicks (avoid borrowing self mutably mid-closure).
        let mut toggle_tracking = false;
        let mut toggle_reveal = false;
        let mut toggle_auto_save = false;

        egui::TopBottomPanel::bottom("status").show(ctx, |ui| {
            ui.horizontal(|ui| {
                // Tracking pill (Qt TrackingPill): reflects + toggles Start/Stop.
                ui.colored_label(if tracking { green } else { gray }, "●");
                
                let tracking_text = if tracking {
                    self.i18n.tracking_active()
                } else {
                    self.i18n.tracking_inactive()
                };

                if ui
                    .selectable_label(false, tracking_text)
                    .on_hover_text(self.i18n.tracker_state_tip())
                    .clicked()
                {
                    toggle_tracking = true;
                }
                ui.separator();
                // Auto-save is always on in the Rust build.
                ui.colored_label(if auto_save { green } else { gray }, "●");
                
                if ui
                    .selectable_label(false, self.i18n.auto_save(auto_save))
                    .on_hover_text(self.i18n.auto_save_tip())
                    .clicked()
                {
                    toggle_auto_save = true;
                }

                ui.separator();
                // Reveal pill (mirrors the progression "reveal uncollected" option).
                ui.colored_label(if reveal { green } else { gray }, "●");
                if ui
                    .selectable_label(false, self.i18n.items_revealed(reveal))
                    .on_hover_text(self.i18n.items_revealed_tip())
                    .clicked() {
                    toggle_reveal = true;
                }

                // Right side: connection state + last activity.
                ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                    ui.label(format!("{} : {}", self.i18n.entry(), last_entrance.as_deref().unwrap_or("—")));
                    ui.separator();
                    ui.label(format!("{} : {}", self.i18n.item(), last_item.as_deref().unwrap_or("—")));
                    ui.separator();
                    let ccol = if connected { green } else { Color32::from_rgb(220, 120, 90) };
                    ui.weak(&status);
                    ui.colored_label(ccol, "●");
                });
            });
        });

        if toggle_tracking {
            self.toggle_tracking();
        }
        if toggle_auto_save {
            self.set_auto_save(!auto_save);
        }
        if toggle_reveal {
            self.dashboard.set_reveal(!reveal);
        }
    }
}
