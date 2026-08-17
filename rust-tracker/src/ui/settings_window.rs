//! The ROM Settings window (shuffle parameters + item settings editors).
use eframe::egui::{self};

use crate::*;
use crate::scene::Game;

impl TrackerApp {

    pub(crate) fn draw_settings_window(&mut self, ctx: &egui::Context) {
        let mut open = self.show_settings;
        let mut apply = false;
        egui::Window::new(self.i18n.settings_rom_settings())
            .open(&mut open)
            .default_width(600.0)
            .resizable(true)
            .show(ctx, |ui| {
                ui.horizontal(|ui| {
                    ui.label(format!("{} : {:?}", self.i18n.settings_game(), self.rom));
                    ui.separator();
                    ui.label(format!("{} : {:?}", self.i18n.settings_build(), self.rom_settings.game));
                    ui.label(format!("{} : {:?}", self.i18n.settings_mode(), self.rom_settings.mode));
                    ui.label(format!("{} : {:?}", self.i18n.settings_goal(), self.rom_settings.goal));
                });
                ui.label(format!(
                    "{} : {} OoT / {} MM  ·  MQ/JP : {} {}",
                    self.i18n.settings_hidden_objs(),
                    self.excluded.oot.len(),
                    self.excluded.mm.len(),
                    self.mq_scenes.len(),
                    self.i18n.scenes()
                ));
                ui.separator();
                if ui.button(self.i18n.apply()).clicked() {
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
    pub(crate) fn draw_settings_params(&mut self, ui: &mut egui::Ui) {
        use data::ParamType as PT;

        // General page: game / mode / goal selectors (Settings::AddSetting fields).
        egui::CollapsingHeader::new(self.i18n.settings_general()).default_open(true).show(ui, |ui| {
            ui.horizontal(|ui| {
                ui.label(self.i18n.settings_game());
                egui::ComboBox::from_id_salt("set_game")
                    .selected_text(self.rom_settings.game.label())
                    .show_ui(ui, |ui| {
                        for g in settings::RomGame::ALL {
                            ui.selectable_value(&mut self.rom_settings.game, g, g.label());
                        }
                    });
            });
            ui.horizontal(|ui| {
                ui.label(self.i18n.settings_mode());
                egui::ComboBox::from_id_salt("set_mode")
                    .selected_text(self.rom_settings.mode.label())
                    .show_ui(ui, |ui| {
                        for m in settings::GameMode::ALL {
                            ui.selectable_value(&mut self.rom_settings.mode, m, m.label());
                        }
                    });
            });
            ui.horizontal(|ui| {
                ui.label(self.i18n.settings_goal());
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
        egui::CollapsingHeader::new(self.i18n.settings_layouts()).show(ui, |ui| {
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
            if ui.checkbox(&mut jp, self.i18n.settings_layout_deku()).changed() {
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
            (self.i18n.settings_map_filters(), data::FILTER_SETTINGS),
            (self.i18n.settings_item_settings(), data::ITEM_SETTINGS),
        ] {
            egui::CollapsingHeader::new(title).default_open(title.starts_with(self.i18n.filters())).show(ui, |ui| {
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
                            .selected_text(shuffle_label(&self.i18n, cur))
                            .show_ui(ui, |ui| {
                                for &opt in opts {
                                    if ui.selectable_label(cur == opt, shuffle_label(&self.i18n, opt)).clicked() {
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
}
