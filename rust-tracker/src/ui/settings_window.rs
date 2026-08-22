//! The ROM Settings window: a left category nav + a grouped, paged editor,
//! mirroring the Qt SettingsTab (left QListWidget + QStackedWidget of pages).
//! Each page is a stack of titled group "bubbles"; every row is
//! `badge | name | editor`, exactly like the Qt `AddParamRow` grid.
use eframe::egui::{self, Color32, RichText};

use crate::*;
use crate::scene::Game;

/// Which game a parameter row belongs to (draws a small coloured chip, like the
/// Qt `MakeGameBadge`). `None` leaves the badge column blank for cross-game rows.
#[derive(Clone, Copy)]
enum Badge {
    None,
    Oot,
    Mm,
}

/// A titled group ("bubble") of parameter rows on a page (Qt `MakeParamGroup`).
struct Group {
    title: &'static str,
    rows: &'static [(&'static str, Badge)],
}

// The Qt-style accent colours for the OoT / MM chips.
const OOT_BLUE: Color32 = Color32::from_rgb(74, 158, 219);
const MM_PURPLE: Color32 = Color32::from_rgb(155, 93, 229);

/// The left-nav categories (index == `self.settings_nav`), in Qt page order.
const CATEGORIES: &[&str] = &[
    "General",
    "Keys & Dungeons",
    "NPC & Shops",
    "Breakables",
    "Special",
    "Progressive Items",
    "Shared Items",
    "Songs",
    "World Items",
    "MQ / JP Layouts",
];

const KEYS_GROUPS: &[Group] = &[
    Group { title: "Small Keys", rows: &[
        ("smallKeyShuffleOot", Badge::Oot),
        ("smallKeyShuffleMm", Badge::Mm),
        ("smallKeyShuffleHideout", Badge::Oot),
        ("smallKeyShuffleChestGame", Badge::Oot),
    ] },
    Group { title: "Boss Keys", rows: &[
        ("bossKeyShuffleOot", Badge::Oot),
        ("bossKeyShuffleMm", Badge::Mm),
        ("ganonBossKey", Badge::Oot),
    ] },
    Group { title: "Dungeon Aids", rows: &[
        ("silverRupeeShuffle", Badge::Oot),
        ("mapCompassShuffle", Badge::None),
        ("tingleShuffle", Badge::Mm),
    ] },
    Group { title: "Rusty Keys", rows: &[
        ("rustyKeysOot", Badge::Oot),
        ("rustyKeysMm", Badge::Mm),
    ] },
];

const NPC_GROUPS: &[Group] = &[
    Group { title: "Scrubs & Cows", rows: &[
        ("scrubShuffleOot", Badge::Oot),
        ("scrubShuffleMm", Badge::Mm),
        ("cowShuffleOot", Badge::Oot),
        ("cowShuffleMm", Badge::Mm),
    ] },
    Group { title: "Shops & Trading", rows: &[
        ("shopShuffleOot", Badge::Oot),
        ("shopShuffleMm", Badge::Mm),
        ("shuffleMerchantsOot", Badge::Oot),
        ("shuffleMerchantsMm", Badge::Mm),
        ("shuffleMaskTrades", Badge::None),
    ] },
    Group { title: "Mini-games & Rewards", rows: &[
        ("owlShuffle", Badge::Mm),
        ("pondFishShuffle", Badge::Oot),
        ("eggShuffle", Badge::None),
        ("shuffleLotteryMm", Badge::Mm),
        ("shuffleFrogsRupeesOot", Badge::Oot),
        ("shuffleSkulltulaFinalReward", Badge::Oot),
        ("divingGameRupeeShuffle", Badge::Oot),
    ] },
];

const BREAK_GROUPS: &[Group] = &[
    Group { title: "Containers", rows: &[
        ("shufflePotsOot", Badge::Oot),
        ("shufflePotsMm", Badge::Mm),
        ("shuffleCratesOot", Badge::Oot),
        ("shuffleCratesMm", Badge::Mm),
        ("shuffleBarrelsMm", Badge::Mm),
    ] },
    Group { title: "Vegetation", rows: &[
        ("shuffleGrassOot", Badge::Oot),
        ("shuffleGrassMm", Badge::Mm),
        ("shuffleTFGrassMm", Badge::Mm),
        ("shuffleTreesOot", Badge::Oot),
        ("shuffleTreesMm", Badge::Mm),
        ("shuffleBushOot", Badge::Oot),
        ("shuffleBushMm", Badge::Mm),
        ("shuffleSoilOot", Badge::Oot),
        ("shuffleSoilMm", Badge::Mm),
        ("shuffleHivesOot", Badge::Oot),
        ("shuffleHivesMm", Badge::Mm),
    ] },
    Group { title: "Rocks & Ice", rows: &[
        ("shuffleRocksOot", Badge::Oot),
        ("shuffleRocksMm", Badge::Mm),
        ("shuffleBouldersOot", Badge::Oot),
        ("shuffleBouldersMm", Badge::Mm),
        ("shuffleSilverBouldersOot", Badge::Oot),
        ("shuffleRedBouldersOot", Badge::Oot),
        ("shuffleRedBouldersMm", Badge::Mm),
        ("shuffleIciclesOot", Badge::Oot),
        ("shuffleIciclesMm", Badge::Mm),
        ("shuffleRedIceOot", Badge::Oot),
        ("shuffleSnowballsMm", Badge::Mm),
    ] },
    Group { title: "Misc", rows: &[
        ("shuffleWonderItemsOot", Badge::Oot),
        ("shuffleWonderItemsMm", Badge::Mm),
        ("shuffleButterfliesOot", Badge::Oot),
        ("shuffleButterfliesMm", Badge::Mm),
    ] },
];

const SPECIAL_GROUPS: &[Group] = &[
    Group { title: "Tokens & Souls", rows: &[
        ("goldSkulltulaTokens", Badge::Oot),
        ("housesSkulltulaTokens", Badge::Mm),
    ] },
    Group { title: "Fairies", rows: &[
        ("fairyFountainFairyShuffleOot", Badge::Oot),
        ("fairyFountainFairyShuffleMm", Badge::Mm),
        ("fairySpotShuffleOot", Badge::Oot),
        ("townFairyShuffle", Badge::Mm),
        ("strayFairyChestShuffle", Badge::Mm),
        ("strayFairyOtherShuffle", Badge::Mm),
    ] },
    Group { title: "Freestanding", rows: &[
        ("shuffleFreeRupeesOot", Badge::Oot),
        ("shuffleFreeRupeesMm", Badge::Mm),
        ("shuffleFreeHeartsOot", Badge::Oot),
        ("shuffleFreeHeartsMm", Badge::Mm),
    ] },
    Group { title: "Unique Items", rows: &[
        ("songs", Badge::None),
        ("shuffleOcarinasOot", Badge::Oot),
        ("shuffleMasterSword", Badge::Oot),
        ("shuffleGerudoCard", Badge::Oot),
    ] },
    Group { title: "Misc", rows: &[
        ("skipZelda", Badge::Oot),
        ("agelessStrength", Badge::Oot),
        ("restoreBrokenActors", Badge::None),
    ] },
    Group { title: "Cross-Games Warps", rows: &[
        ("crossWarpOot", Badge::Oot),
        ("crossWarpMm", Badge::Mm),
    ] },
];

const WORLD_PARAM_GROUPS: &[Group] = &[
    Group { title: "Unique World Items", rows: &[
        ("kamaroMaskOot", Badge::Oot),
        ("powderKegOot", Badge::Oot),
        ("gfsOot", Badge::Oot),
        ("boomerangMm", Badge::Mm),
        ("slingshotMm", Badge::Mm),
    ] },
    Group { title: "Masks", rows: &[
        ("gerudoMaskMm", Badge::Mm),
        ("skullMaskMm", Badge::Mm),
        ("spookyMaskMm", Badge::Mm),
    ] },
];

/// The item-setting keys that live on their own pages (Songs / World Items /
/// Keys) and must be skipped by the Progressive Items page (Qt's continue list).
fn on_other_page(key: &str) -> bool {
    matches!(
        key,
        "kamaroMaskOot" | "boomerangMm" | "powderKegOot" | "gfsOot" | "slingshotMm"
            | "gerudoMaskMm" | "skullMaskMm" | "spookyMaskMm"
            | "rustyKeysOot" | "rustyKeysMm"
    )
}

/// The game badge inferred from a key's `Oot` / `Mm` suffix (Qt does the same).
fn badge_from_suffix(key: &str) -> Badge {
    if key.ends_with("Oot") {
        Badge::Oot
    } else if key.ends_with("Mm") {
        Badge::Mm
    } else {
        Badge::None
    }
}

/// The parameter metadata for a key (filter or item settings), Qt `FindParameter`.
fn find_meta(key: &str) -> Option<&'static data::SettingMeta> {
    data::FILTER_SETTINGS
        .iter()
        .chain(data::ITEM_SETTINGS.iter())
        .find(|m| m.key == key)
}

/// A small coloured "OoT" / "MM" chip in the badge column (Qt `MakeGameBadge`).
fn game_badge(ui: &mut egui::Ui, badge: Badge) {
    match badge {
        Badge::Oot => {
            ui.label(
                RichText::new(" OoT ")
                    .color(OOT_BLUE)
                    .size(11.0)
                    .strong()
                    .background_color(Color32::from_rgba_unmultiplied(74, 158, 219, 28)),
            );
        }
        Badge::Mm => {
            ui.label(
                RichText::new(" MM ")
                    .color(MM_PURPLE)
                    .size(11.0)
                    .strong()
                    .background_color(Color32::from_rgba_unmultiplied(155, 93, 229, 28)),
            );
        }
        Badge::None => {
            ui.label(""); // keep the badge column occupied so the grid stays aligned
        }
    }
}

/// A titled group frame hosting a 3-column (`badge | name | editor`) grid, shared
/// by every page so paddings / column widths stay identical (Qt `MakeParamGroup`).
fn settings_group(ui: &mut egui::Ui, id: usize, title: &str, add: impl FnOnce(&mut egui::Ui)) {
    ui.group(|ui| {
        ui.set_width(ui.available_width());
        ui.label(RichText::new(title).strong().color(ACCENT));
        ui.add_space(4.0);
        egui::Grid::new(("settings_grid", id))
            .num_columns(3)
            .spacing([12.0, 6.0])
            .show(ui, |ui| add(ui));
    });
    ui.add_space(8.0);
}

impl TrackerApp {
    pub(crate) fn draw_settings_window(&mut self, ctx: &egui::Context) {
        let mut open = self.show_settings;
        let mut apply = false;
        let mut load_spoiler = false;
        egui::Window::new(self.i18n.settings_rom_settings())
            .open(&mut open)
            .default_width(880.0)
            .default_height(560.0)
            .resizable(true)
            .show(ctx, |ui| {
                self.settings_header(ui);
                ui.separator();
                ui.horizontal_top(|ui| {
                    // Navigation column (fixed width) + Apply / Load Spoiler buttons.
                    ui.vertical(|ui| {
                        ui.set_width(190.0);
                        for (i, cat) in CATEGORIES.iter().enumerate() {
                            let label = self.i18n.tr_settings(cat).to_owned();
                            if ui.selectable_label(self.settings_nav == i, label).clicked() {
                                self.settings_nav = i;
                            }
                        }
                        ui.add_space(10.0);
                        ui.separator();
                        if ui.button(self.i18n.load_spoiler()).clicked() {
                            load_spoiler = true;
                        }
                        if ui.button(self.i18n.apply()).clicked() {
                            apply = true;
                        }
                    });
                    ui.separator();
                    // Page column (scrollable), the selected category's groups.
                    let w = ui.available_width();
                    ui.vertical(|ui| {
                        ui.set_width(w);
                        egui::ScrollArea::vertical()
                            .id_salt("settings_page")
                            .show(ui, |ui| self.draw_settings_page(ui));
                    });
                });
            });
        // Deferred side-effects (kept out of the `&mut self`-borrowing window closure).
        if load_spoiler {
            self.load_spoiler_dialog();
        }
        if apply {
            self.excluded = self.rom_settings.apply(&self.mq_scenes);
            // Allocate the world set from Mode / team count so the world selector
            // appears from the settings alone (Qt SettingsTab), even with no spoiler.
            self.sync_worlds_from_settings();
            self.rebuild_scene();
            self.prog_dirty = true; // settings drive shared / disabled / starting items
            self.counts_dirty = true;
        }
        self.show_settings = open;
    }

    /// The summary header: detected ROM + current build / mode / goal + how many
    /// objects the current settings hide (mirror of the old header line).
    fn settings_header(&mut self, ui: &mut egui::Ui) {
        ui.horizontal_wrapped(|ui| {
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
            self.i18n.scenes(),
        ));
    }

    /// Render the selected category's page.
    fn draw_settings_page(&mut self, ui: &mut egui::Ui) {
        match self.settings_nav {
            0 => self.page_general(ui),
            1 => self.groups_page(ui, KEYS_GROUPS),
            2 => self.groups_page(ui, NPC_GROUPS),
            3 => self.groups_page(ui, BREAK_GROUPS),
            4 => self.groups_page(ui, SPECIAL_GROUPS),
            5 => self.page_progressive(ui),
            6 => self.page_shared(ui),
            7 => self.page_songs(ui),
            8 => self.page_world(ui),
            9 => self.page_layouts(ui),
            _ => {}
        }
    }

    /// A page built entirely from static parameter groups.
    fn groups_page(&mut self, ui: &mut egui::Ui, groups: &'static [Group]) {
        for (gi, g) in groups.iter().enumerate() {
            let title = self.i18n.tr_settings(g.title).to_owned();
            settings_group(ui, gi, &title, |ui| {
                for &(key, badge) in g.rows {
                    self.param_row(ui, key, badge);
                }
            });
        }
    }

    /// One parameter row: badge, translated name, and a shuffle / boolean combo
    /// (the editor mirrors the current Rust settings — a combo for every value).
    fn param_row(&mut self, ui: &mut egui::Ui, key: &'static str, badge: Badge) {
        use data::ParamType as PT;
        let Some(m) = find_meta(key) else { return };
        game_badge(ui, badge);
        ui.label(self.i18n.tr_settings(m.name));
        let cur = self.rom_settings.value(key);
        let opts: &[data::ShuffleSetting] = match m.type_ {
            PT::shuffle => &SHUFFLE_OPTIONS,
            _ => &BOOL_OPTIONS,
        };
        egui::ComboBox::from_id_salt(key)
            .width(120.0)
            .selected_text(shuffle_label(&self.i18n, cur))
            .show_ui(ui, |ui| {
                for &opt in opts {
                    if ui.selectable_label(cur == opt, shuffle_label(&self.i18n, opt)).clicked() {
                        self.rom_settings.set_value(key, opt);
                    }
                }
            });
        ui.end_row();
    }

    /// General: game / mode / goal selectors (radio rows) + the team count.
    fn page_general(&mut self, ui: &mut egui::Ui) {
        let game_t = self.i18n.tr_settings("Game").to_owned();
        settings_group(ui, 0, &game_t, |ui| {
            for g in settings::RomGame::ALL {
                ui.radio_value(&mut self.rom_settings.game, g, g.label());
            }
            ui.end_row();
        });
        let mode_t = self.i18n.tr_settings("Mode").to_owned();
        settings_group(ui, 1, &mode_t, |ui| {
            for m in settings::GameMode::ALL {
                ui.radio_value(&mut self.rom_settings.mode, m, m.label());
            }
            ui.end_row();
        });
        let goal_t = self.i18n.tr_settings("Goal").to_owned();
        settings_group(ui, 2, &goal_t, |ui| {
            for g in settings::GoalMode::ALL {
                ui.radio_value(&mut self.rom_settings.goal, g, g.label());
            }
            ui.end_row();
        });
        let teams_t = self.i18n.tr_settings("Teams").to_owned();
        let num_label = self.i18n.tr_settings("Number of teams").to_owned();
        settings_group(ui, 3, &teams_t, |ui| {
            ui.label(num_label);
            ui.add(egui::DragValue::new(&mut self.rom_settings.num_teams).range(1..=64));
            ui.end_row();
        });
    }

    /// Progressive Items: every non-shared, non-song item setting that isn't shown
    /// on the Songs / World Items / Keys pages (Qt `BuildProgressiveItemsPage`).
    fn page_progressive(&mut self, ui: &mut egui::Ui) {
        let title = self.i18n.tr_settings("Progressive Items").to_owned();
        settings_group(ui, 0, &title, |ui| {
            for m in data::ITEM_SETTINGS {
                let key = m.key;
                if key.starts_with("shared") || key.starts_with("song") || on_other_page(key) {
                    continue;
                }
                self.param_row(ui, key, badge_from_suffix(key));
            }
        });
    }

    /// Shared Items: every `shared*` item setting except the shared songs.
    fn page_shared(&mut self, ui: &mut egui::Ui) {
        let title = self.i18n.tr_settings("Shared Items").to_owned();
        settings_group(ui, 0, &title, |ui| {
            for m in data::ITEM_SETTINGS {
                let key = m.key;
                if !key.starts_with("shared") || key.starts_with("sharedSong") {
                    continue;
                }
                self.param_row(ui, key, Badge::None);
            }
        });
    }

    /// Songs: per-game songs + shared songs, in two groups (Qt `BuildSongsPage`).
    fn page_songs(&mut self, ui: &mut egui::Ui) {
        let songs_t = self.i18n.tr_settings("Songs").to_owned();
        settings_group(ui, 0, &songs_t, |ui| {
            for m in data::ITEM_SETTINGS {
                let key = m.key;
                if !key.starts_with("song") {
                    continue;
                }
                self.param_row(ui, key, badge_from_suffix(key));
            }
        });
        let shared_t = self.i18n.tr_settings("Shared Songs").to_owned();
        settings_group(ui, 1, &shared_t, |ui| {
            for m in data::ITEM_SETTINGS {
                let key = m.key;
                if !key.starts_with("sharedSong") {
                    continue;
                }
                self.param_row(ui, key, Badge::None);
            }
        });
    }

    /// World Items: unique items + masks (params), open dungeons + the per-dungeon
    /// key ring / silver pouch / owl editors (Qt `BuildWorldItemsPage`).
    fn page_world(&mut self, ui: &mut egui::Ui) {
        for (gi, g) in WORLD_PARAM_GROUPS.iter().enumerate() {
            let title = self.i18n.tr_settings(g.title).to_owned();
            settings_group(ui, gi, &title, |ui| {
                for &(key, badge) in g.rows {
                    self.param_row(ui, key, badge);
                }
            });
        }

        // Open Dungeons: a plain Settings member, not a shuffle parameter.
        let open_t = self.i18n.tr_settings("Open Dungeons").to_owned();
        let fire_label = self.i18n.tr_settings("Fire Temple Open As Child").to_owned();
        settings_group(ui, 10, &open_t, |ui| {
            game_badge(ui, Badge::Oot);
            ui.label(fire_label);
            ui.checkbox(&mut self.rom_settings.fire_temple_open_as_child, "");
            ui.end_row();
        });

        // Small Key Rings: checked => deliver a ring, unchecked => small keys.
        let ring_t = self.i18n.tr_settings("Small Key Rings").to_owned();
        settings_group(ui, 11, &ring_t, |ui| {
            for r in settings::KEY_RINGS_OOT {
                self.key_ring_row(ui, r, Badge::Oot);
            }
            for r in settings::KEY_RINGS_MM {
                self.key_ring_row(ui, r, Badge::Mm);
            }
        });

        // Silver Rupee Pouches: clusters absent from the active layout are greyed.
        let pouch_t = self.i18n.tr_settings("Silver Rupee Pouches").to_owned();
        settings_group(ui, 12, &pouch_t, |ui| {
            for a in settings::SILVER_AREAS {
                self.silver_row(ui, a);
            }
        });

        // Pre-Activated Owl Statues.
        let owl_t = self.i18n.tr_settings("Pre-Activated Owl Statues").to_owned();
        settings_group(ui, 13, &owl_t, |ui| {
            for o in settings::OWL_STATUES {
                self.owl_row(ui, o);
            }
        });
    }

    fn key_ring_row(&mut self, ui: &mut egui::Ui, r: &settings::KeyRing, badge: Badge) {
        game_badge(ui, badge);
        ui.label(self.i18n.tr_settings(r.label));
        let mut on = self.rom_settings.key_ring_on(r.ring);
        if ui.checkbox(&mut on, "").changed() {
            self.rom_settings.set_key_ring(r.small, r.ring, on);
        }
        ui.end_row();
    }

    fn silver_row(&mut self, ui: &mut egui::Ui, a: &settings::SilverArea) {
        let exists = self.rom_settings.silver_area_exists(a, &self.mq_scenes);
        game_badge(ui, Badge::Oot);
        let label = self.i18n.tr_settings(a.label).to_owned();
        if exists {
            ui.label(label);
        } else {
            ui.weak(label); // cluster not present in this seed's layout
        }
        let mut on = exists && self.rom_settings.silver_pouch_on(a.pouch);
        if ui.add_enabled(exists, egui::Checkbox::new(&mut on, "")).changed() {
            self.rom_settings.set_silver_pouch(a.rupee, a.pouch, on);
        }
        ui.end_row();
    }

    fn owl_row(&mut self, ui: &mut egui::Ui, o: &settings::OwlStatue) {
        game_badge(ui, Badge::Mm);
        ui.label(self.i18n.tr_settings(o.label));
        let mut on = self.rom_settings.owl_on(o.id);
        if ui.checkbox(&mut on, "").changed() {
            self.rom_settings.set_owl(o.id, on);
        }
        ui.end_row();
    }

    /// MQ / JP Layouts: per-dungeon Master Quest toggles + the MM JP Deku Palace
    /// toggle (which swaps four scenes at once).
    fn page_layouts(&mut self, ui: &mut egui::Ui) {
        let mq_t = self.i18n.tr_settings("Master Quest Dungeons").to_owned();
        settings_group(ui, 0, &mq_t, |ui| {
            for &(name, scene) in settings::OOT_MQ_DUNGEONS {
                game_badge(ui, Badge::Oot);
                ui.label(name);
                let key = (Game::Oot, scene);
                let mut on = self.mq_scenes.contains(&key);
                if ui.checkbox(&mut on, "").changed() {
                    if on {
                        self.mq_scenes.insert(key);
                    } else {
                        self.mq_scenes.remove(&key);
                    }
                }
                ui.end_row();
            }
        });
        let jp_t = self.i18n.tr_settings("Majora's Mask JP Layouts").to_owned();
        let deku = self.i18n.tr_settings("Deku Palace").to_owned();
        settings_group(ui, 1, &jp_t, |ui| {
            game_badge(ui, Badge::Mm);
            ui.label(deku);
            let jp_key = (Game::Mm, settings::MM_JP_SCENES[0]);
            let mut jp = self.mq_scenes.contains(&jp_key);
            if ui.checkbox(&mut jp, "").changed() {
                for &sc in settings::MM_JP_SCENES {
                    if jp {
                        self.mq_scenes.insert((Game::Mm, sc));
                    } else {
                        self.mq_scenes.remove(&(Game::Mm, sc));
                    }
                }
            }
            ui.end_row();
        });
    }
}
