//! The Progression dashboard: item grid, per-item detail panel and icons.
use eframe::egui::{self, vec2, Align2, Color32, FontId, Sense, Stroke};

use crate::*;
use crate::scene::Game;
use crate::ui::kbdnav;
use std::collections::HashMap;

impl TrackerApp {

    /// The progression dashboard (ProgressionTab): four sub-tabs of item-icon
    /// grids on the left, a detail panel with the selected item's location tree
    /// on the right.
    pub(crate) fn draw_progression_tab(&mut self, ctx: &egui::Context) {
        // Recompute the per-entry state when the collected set / spoiler /
        // settings / active world changed (RebuildFromSceneObjects). The world to
        // display is driven by the global selector in the tab bar (SetActiveWorld
        // keeps the dashboard's target world in sync), matching the Qt corner
        // selector that switches maps and progression together.
        if self.prog_dirty {
            self.dashboard.rebuild(&self.worlds, &self.rom_settings, &self.mq_scenes);
            self.prog_dirty = false;
        }

        // Build the greyscale "uncollected" icon variants (budgeted, cached).
        self.ensure_prog_grey_icons(ctx);

        // Sub-tab selector + reveal toggle (top).
        let mut new_tab = self.dashboard.sub_tab;
        egui::TopBottomPanel::top("prog_subtabs").show(ctx, |ui| {
            ui.horizontal(|ui| {
                for (p, page) in data::PROG_PAGES.iter().enumerate() {
                    if !self.dashboard.page_visible(p, &self.rom_settings) {
                        continue; // whole page disabled by the ROM settings
                    }
                    if ui
                        .selectable_label(self.dashboard.sub_tab == p, self.i18n.prog_title(page.title))
                        .clicked()
                    {
                        new_tab = p;
                    }
                }
                ui.separator();
                // Reveal pill: identical to the status-bar toggle (same dot, text
                // and tooltip) so both controls for the shared `reveal` flag read
                // the same, instead of a differently-worded checkbox.
                let reveal = self.dashboard.reveal;
                let green = Color32::from_rgb(120, 200, 90);
                let gray = Color32::from_gray(120);
                ui.colored_label(if reveal { green } else { gray }, "●");
                if ui
                    .selectable_label(false, self.i18n.items_revealed(reveal))
                    .on_hover_text(self.i18n.items_revealed_tip())
                    .clicked()
                {
                    self.dashboard.set_reveal(!reveal);
                }
            });
        });

        // Refresh the cached detail tree (mutably) before the read-only panels.
        self.dashboard.ensure_tree(&self.worlds, &self.mq_scenes);

        // Keyboard navigation for the location tree: arrows highlight a location,
        // Enter / Space jumps to its scene (the same action as a click).
        let kid = egui::Id::new("kbd_prog_tree");
        let mut klist = self.kbd.begin(kid);
        let mut kout = kbdnav::KbdOut::default();
        let mut loc_targets: HashMap<u64, (Game, u16)> = HashMap::new();

        // Detail panel (right), then the icon grid (center).
        let mut nav: Option<(Game, u16)> = None;
        egui::SidePanel::right("prog_detail")
            .resizable(false)
            .exact_width(300.0)
            .show(ctx, |ui| {
                self.draw_prog_detail(ui, &mut nav, &mut klist, &mut loc_targets, &mut kout)
            });

        let mut new_selected = self.dashboard.selected;
        egui::CentralPanel::default()
            .show(ctx, |ui| self.draw_prog_grid(ui, &mut new_selected));

        // Apply the deferred UI mutations (kept out of the &self render borrows).
        if new_tab != self.dashboard.sub_tab {
            self.dashboard.sub_tab = new_tab;
        }
        self.dashboard.selected = new_selected;
        // Keyboard move: sync the tree focus / active target, then honour an
        // Enter / Space activation as a navigation to that location's scene.
        self.kbd.apply(kid, &kout);
        if let Some(t) = kout.activate.and_then(|k| loc_targets.get(&k).copied()) {
            nav = Some(t);
        }
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

    /// The greyscale ("uncollected") variant of an EGameIcon texture, if built.
    pub(crate) fn prog_icon_tex_grey(&self, icon: &str) -> Option<&egui::TextureHandle> {
        let path = data::ICON_BY_NAME
            .binary_search_by_key(&icon, |&(n, _)| n)
            .ok()
            .map(|i| data::ICON_BY_NAME[i].1)?;
        self.grey_icon_cache.get(path)?.as_ref()
    }

    /// The solid-blue silhouette variant of an EGameIcon texture (for the "obtained"
    /// glow), if built.
    pub(crate) fn prog_icon_tex_glow(&self, icon: &str) -> Option<&egui::TextureHandle> {
        let path = data::ICON_BY_NAME
            .binary_search_by_key(&icon, |&(n, _)| n)
            .ok()
            .map(|i| data::ICON_BY_NAME[i].1)?;
        self.glow_icon_cache.get(path)?.as_ref()
    }

    /// Pick the texture id + tint for a progression icon. `complete` → the full
    /// colour icon; otherwise the desaturated placeholder (Qt look). While that
    /// grey variant is still being built, fall back to the colour icon dimmed by a
    /// grey tint so it never flashes at full colour.
    pub(crate) fn prog_icon_display(&self, icon: &str, complete: bool) -> Option<(egui::TextureId, Color32)> {
        if complete {
            return self.prog_icon_tex(icon).map(|t| (t.id(), Color32::WHITE));
        }
        if let Some(t) = self.prog_icon_tex_grey(icon) {
            return Some((t.id(), Color32::WHITE));
        }
        self.prog_icon_tex(icon).map(|t| (t.id(), Color32::from_gray(85)))
    }

    /// The left grid: each visible section of the active page as a header + a
    /// wrapped row of icon cells (BuildPage). Sets `new_selected` on a click.
    pub(crate) fn draw_prog_grid(&self, ui: &mut egui::Ui, new_selected: &mut Option<usize>) {
        let page = self.dashboard.sub_tab;
        let accent = Color32::from_rgb(74, 158, 219);

        // Group the page's visible entries by section (flat order is page-major,
        // so a section's entries are contiguous). Fully-disabled sections vanish.
        let mut sections: Vec<(&str, Vec<usize>)> = Vec::new();
        let mut last_sec: Option<usize> = None;
        for (i, fe) in self.dashboard.flat().iter().enumerate() {
            if fe.page != page || self.dashboard.entry_hidden(i, &self.rom_settings) {
                continue;
            }
            if last_sec != Some(fe.section) {
                sections.push((self.i18n.prog_title(fe.section_title), Vec::new()));
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
        const ICON: f32 = 56.0;
        let cell = vec2(96.0, 112.0);

        let inner = ui.allocate_ui_with_layout(
            cell,
            egui::Layout::top_down(egui::Align::Center),
            |ui| {
                // Icon slot: a discreet blue "obtained" glow that follows the icon's
                // CONTOURS (mirrors the Qt drop-shadow, which blurs the alpha). We
                // stamp a solid-blue silhouette of the icon a few times, offset in a
                // ring, behind the real icon — so the halo hugs the shape (a skull, a
                // sword…) instead of the square image bounds. Kept small (~3px) and
                // low-alpha so it stays discreet and inside the cell.
                let (icon_rect, _) = ui.allocate_exact_size(vec2(ICON, ICON), Sense::hover());
                if complete {
                    if let Some(sil) = self.prog_icon_tex_glow(e.icon) {
                        // Clip to the icon slot as insurance against any bleed.
                        let p = ui.painter().with_clip_rect(icon_rect.expand(6.0));
                        let uv = egui::Rect::from_min_max(egui::pos2(0.0, 0.0), egui::pos2(1.0, 1.0));
                        // Two fading rings of 8 offsets: the union is a soft, even
                        // halo around the silhouette that fades outward.
                        for (radius, alpha) in [(1.6_f32, 22u8), (3.2, 11)] {
                            for k in 0..8 {
                                let a = std::f32::consts::TAU * k as f32 / 8.0;
                                let off = vec2(a.cos(), a.sin()) * radius;
                                p.image(sil.id(), icon_rect.translate(off), uv, Color32::from_white_alpha(alpha));
                            }
                        }
                    }
                }
                if let Some((id, tint)) = self.prog_icon_display(e.icon, complete) {
                    egui::Image::new((id, vec2(ICON, ICON))).tint(tint).paint_at(ui, icon_rect);
                }
                // Name, then the counter BELOW it. The old blue-filled badge was
                // illegible, so the count is now plain light-blue text (no fill).
                let name_col = if complete {
                    Color32::from_rgb(221, 238, 255)
                } else {
                    Color32::from_gray(120)
                };
                ui.label(egui::RichText::new(self.i18n.tr_prog_entry(e.name, e.lookup_keys)).small().color(name_col));
                if let Some(badge) = self.dashboard.badge_text(i) {
                    ui.label(
                        egui::RichText::new(badge)
                            .small()
                            .strong()
                            .color(Color32::from_rgb(130, 200, 255)),
                    );
                }
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
    pub(crate) fn draw_prog_detail(
        &self,
        ui: &mut egui::Ui,
        nav: &mut Option<(Game, u16)>,
        klist: &mut kbdnav::KbdList,
        loc_targets: &mut HashMap<u64, (Game, u16)>,
        kout: &mut kbdnav::KbdOut,
    ) {
        ui.add_space(6.0);
        let Some(i) = self.dashboard.selected else {
            ui.weak(self.i18n.prog_select_item());
            return;
        };
        let e = self.dashboard.flat()[i].entry;
        let st = self.dashboard.state(i);

        ui.vertical_centered(|ui| {
            if let Some((id, tint)) = self.prog_icon_display(e.icon, self.dashboard.complete(i)) {
                ui.add(egui::Image::new((id, vec2(72.0, 72.0))).tint(tint));
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
            // Order scenes by their DISPLAYED (translated) name so the tree is alphabetical
            // in the active language, keeping OoT before MM. The model sorts by the raw
            // English name (no i18n there), which would not follow the language.
            let mut ordered: Vec<_> = tree.iter().collect();
            ordered.sort_by(|a, b| {
                (a.game.idx(), self.i18n.tr_scene(&a.title).to_lowercase())
                    .cmp(&(b.game.idx(), self.i18n.tr_scene(&b.title).to_lowercase()))
            });
            let mut cur_game: Option<Game> = None;
            for scene in ordered {
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
                egui::CollapsingHeader::new(format!(
                    "{} ({})",
                    self.i18n.tr_scene(&scene.title),
                    scene.leaves.len()
                ))
                    .id_salt((scene.game.idx(), scene.title.as_str()))
                    .default_open(true)
                    .show(ui, |ui| {
                        // Leaves: uncollected ("to-find") first, then alphabetical by the
                        // translated name so the order follows the active language.
                        let mut leaves: Vec<_> = scene.leaves.iter().collect();
                        leaves.sort_by(|a, b| {
                            (a.collected as u8, self.i18n.tr_object(a.name).to_lowercase())
                                .cmp(&(b.collected as u8, self.i18n.tr_object(b.name).to_lowercase()))
                        });
                        for leaf in leaves {
                            let mut txt = egui::RichText::new(self.i18n.tr_object(leaf.name)).small();
                            txt = if leaf.collected {
                                txt.strikethrough().color(Color32::from_gray(140))
                            } else {
                                txt.color(Color32::from_rgb(230, 240, 255))
                            };
                            // Small per-type icon (chest / pot / GS / …) ahead of the name;
                            // greyed to match a collected row. Reserve the width when the
                            // type has no icon so the names stay aligned.
                            let tex = leaf
                                .icon
                                .and_then(|p| self.icon_cache.get(p))
                                .and_then(|t| t.as_ref())
                                .map(|t| t.id());
                            let resp = ui
                                .horizontal(|ui| {
                                    if let Some(id) = tex {
                                        let tint = if leaf.collected {
                                            Color32::from_gray(140)
                                        } else {
                                            Color32::WHITE
                                        };
                                        ui.add(egui::Image::new((id, vec2(16.0, 16.0))).tint(tint));
                                    } else {
                                        ui.add_space(16.0);
                                    }
                                    ui.selectable_label(false, txt)
                                })
                                .inner;
                            let lk = kbdnav::key((leaf.game.idx() as u8, leaf.render_scene, leaf.name));
                            klist.leaf(ui, lk, &resp);
                            loc_targets.insert(lk, (leaf.game, leaf.render_scene));
                            if resp.clicked() {
                                *nav = Some((leaf.game, leaf.render_scene));
                            }
                        }
                    });
            }
        });
        // Read the arrow keys once every location row has registered.
        *kout = klist.finish(ui);
    }
}
