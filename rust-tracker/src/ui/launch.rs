//! The Launch page (start/stop, save/load, spoiler, multiplayer, journal).
use eframe::egui::{self, Color32};

use crate::*;

impl TrackerApp {

    /// Launch tab: file / multiplayer controls + program log (Qt `LogTab`).
    pub(crate) fn draw_launch(&mut self, ctx: &egui::Context) {
        // Deferred actions so button closures don't borrow `self` mutably while
        // the surrounding UI closure also holds it.
        let mut action = LaunchAction::None;

        egui::CentralPanel::default().show(ctx, |ui| {
            // "Launch Options" group box.
            ui.group(|ui| {
                ui.set_width(ui.available_width());
                accent_heading(ui, self.i18n.launch_options());
                ui.add_space(4.0);

                // Row 1: Save / Load / Load Spoiler / Reset (four equal columns).
                let files = [
                    (format!("💾  {}", self.i18n.save_tracking()), LaunchAction::Save),
                    (format!("📂  {}", self.i18n.load_tracking()), LaunchAction::Load),
                    (format!("📜  {}", self.i18n.load_spoiler()), LaunchAction::Spoiler),
                    (format!("↺  {}", self.i18n.reset_tracking()), LaunchAction::Reset),
                ];
                ui.columns(files.len(), |cols| {
                    for (col, (label, act)) in cols.iter_mut().zip(files) {
                        if col
                            .add_sized([col.available_width(), 30.0], egui::Button::new(label))
                            .clicked()
                        {
                            action = act;
                        }
                    }
                });
                ui.add_space(4.0);

                // Row 2: [Use Multiplayer] [host] [port].
                ui.horizontal(|ui| {
                    ui.checkbox(&mut self.use_multiplayer, self.i18n.use_multiplayer());
                    ui.add_enabled_ui(self.use_multiplayer, |ui| {
                        let port_w = 90.0;
                        let host_w = (ui.available_width() - port_w - 8.0).max(120.0);
                        ui.add_sized(
                            [host_w, 24.0],
                            egui::TextEdit::singleline(&mut self.mp_host)
                                .hint_text(self.i18n.address_placeholder()),
                        );
                        ui.add_sized(
                            [port_w, 24.0],
                            egui::TextEdit::singleline(&mut self.mp_port).hint_text(self.i18n.port_placeholder()),
                        );
                    });
                });
                ui.add_space(4.0);

                // Row 3: Start / Stop tracking (full width).
                let (label, fill) = if self.tracking {
                    (format!("⏹  {}", self.i18n.stop_tracking()), Color32::from_rgb(150, 60, 60))
                } else {
                    (format!("▶  {}", self.i18n.start_tracking()), Color32::from_rgb(52, 110, 66))
                };
                if ui
                    .add_sized([ui.available_width(), 32.0], egui::Button::new(label).fill(fill))
                    .clicked()
                {
                    action = LaunchAction::Toggle;
                }
            });

            ui.add_space(6.0);
            ui.horizontal(|ui| {
                ui.label(format!("{} :", self.i18n.journal()));
                ui.weak(self.i18n.drop_spoiler_hint());
                if ui.small_button(self.i18n.simulate_event()).clicked() {
                    self.simulate_event();
                }
            });
            egui::ScrollArea::vertical().stick_to_bottom(true).id_salt("launchlog").show(ui, |ui| {
                if self.log_lines.is_empty() {
                    ui.weak(self.i18n.no_event());
                }
                for line in self.log_lines.iter() {
                    ui.monospace(line);
                }
            });
        });

        match action {
            LaunchAction::None => {}
            LaunchAction::Save => self.save_tracking_dialog(),
            LaunchAction::Load => self.load_tracking_dialog(),
            LaunchAction::Spoiler => self.load_spoiler_dialog(),
            LaunchAction::Reset => self.reset_tracking(),
            LaunchAction::Toggle => self.toggle_tracking(),
        }
    }
}
