//! The ROM Settings window: a left category nav + a paged editor.
//! The filter pages (Keys & Dungeons … Unique & Rules) render as a 2-column
//! dashboard of group "cards" — each card carries a shuffled-count meter, rows of
//! `[state dot] name | badge | value`, and bulk Shuffle-all / Reset actions, so a
//! group's state reads at a glance. The item pages (General, Progressive, Shared,
//! Songs, World, Layouts) keep the plainer Qt-style titled `settings_group` bubbles.
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

// Card-dashboard state palette: a check type that is placed somewhere (so the tracker
// shows it) reads soft green; left at Vanilla it is muted grey; Removed is soft red.
const STATE_ON: Color32 = Color32::from_rgb(0x6b, 0xd0, 0x8a);
const STATE_OFF: Color32 = Color32::from_rgb(0xd9, 0x6a, 0x80);

/// A shuffle value that actually places the check somewhere (i.e. the tracker shows it).
fn is_shuffled(v: data::ShuffleSetting) -> bool {
    use data::ShuffleSetting as S;
    matches!(v, S::all | S::dungeons | S::overworld | S::starting)
}

/// The state colour for a value: green when shuffled, red when removed, muted otherwise.
fn state_color(v: data::ShuffleSetting) -> Color32 {
    use data::ShuffleSetting as S;
    match v {
        S::removed => STATE_OFF,
        S::vanilla => TEXT_MUTED,
        _ => STATE_ON,
    }
}

/// A thin "N shuffled / total" progress bar drawn under a group card's header.
fn meter(ui: &mut egui::Ui, frac: f32) {
    let w = ui.available_width();
    let (rect, _) = ui.allocate_exact_size(egui::vec2(w, 4.0), egui::Sense::hover());
    let p = ui.painter();
    p.rect_filled(rect, 2.0, BG_INPUT);
    let frac = frac.clamp(0.0, 1.0);
    if frac > 0.0 {
        let mut fill = rect;
        fill.set_width(w * frac);
        p.rect_filled(fill, 2.0, STATE_ON);
    }
}

/// The left-nav categories (index == `self.settings_nav`). The four "world-check"
/// filter pages (Breakables / Fairies & Gossips / Collectibles / Unique & Rules)
/// replace the old catch-all "Special" page so every check type has an obvious home.
const CATEGORIES: &[&str] = &[
    "General",
    "Keys & Dungeons",
    "NPC & Shops",
    "Breakables",
    "Fairies & Gossips",
    "Collectibles",
    "Unique & Rules",
    "Progressive Items",
    "Shared Items",
    "Songs",
    "World Items",
    "Starting & Tricks",
    "Logic / Access",
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
];

const FAIRIES_GROUPS: &[Group] = &[
    Group { title: "Fountain Fairies", rows: &[
        ("fairyFountainFairyShuffleOot", Badge::Oot),
        ("fairyFountainFairyShuffleMm", Badge::Mm),
    ] },
    Group { title: "Fairy Spots", rows: &[
        ("fairySpotShuffleOot", Badge::Oot),
    ] },
    Group { title: "Gossip Stones", rows: &[
        ("shuffleGossipFairiesOot", Badge::Oot),
        ("shuffleGossipBigFairiesOot", Badge::Oot),
        ("shuffleGossipFairiesMm", Badge::Mm),
        ("shuffleGossipBigFairiesMm", Badge::Mm),
    ] },
    Group { title: "Stray Fairies", rows: &[
        ("townFairyShuffle", Badge::Mm),
        ("strayFairyChestShuffle", Badge::Mm),
        ("strayFairyOtherShuffle", Badge::Mm),
    ] },
];

const COLLECTIBLES_GROUPS: &[Group] = &[
    Group { title: "Freestanding", rows: &[
        ("shuffleFreeRupeesOot", Badge::Oot),
        ("shuffleFreeRupeesMm", Badge::Mm),
        ("shuffleFreeHeartsOot", Badge::Oot),
        ("shuffleFreeHeartsMm", Badge::Mm),
    ] },
    Group { title: "Wonder Items & Bugs", rows: &[
        ("shuffleWonderItemsOot", Badge::Oot),
        ("shuffleWonderItemsMm", Badge::Mm),
        ("shuffleButterfliesOot", Badge::Oot),
        ("shuffleButterfliesMm", Badge::Mm),
    ] },
    Group { title: "Tokens & Souls", rows: &[
        ("goldSkulltulaTokens", Badge::Oot),
        ("housesSkulltulaTokens", Badge::Mm),
    ] },
];

const RULES_GROUPS: &[Group] = &[
    Group { title: "Unique Items", rows: &[
        ("songs", Badge::None),
        ("shuffleOcarinasOot", Badge::Oot),
        ("shuffleMasterSword", Badge::Oot),
        ("shuffleGerudoCard", Badge::Oot),
    ] },
    Group { title: "Cross-Games Warps", rows: &[
        ("crossWarpOot", Badge::Oot),
        ("crossWarpMm", Badge::Mm),
    ] },
    Group { title: "Game Rules", rows: &[
        ("skipZelda", Badge::Oot),
        ("agelessStrength", Badge::Oot),
        ("restoreBrokenActors", Badge::None),
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

/// The English name of an item id (mirror of `net_item_name`'s dense-then-linear
/// lookup over `data::ITEMS`). Used to label spoiler-derived starting items.
fn item_name_by_id(id: u32) -> Option<&'static str> {
    let items = crate::data::ITEMS;
    (id as usize)
        .checked_sub(1)
        .and_then(|i| items.get(i))
        .filter(|d| d.id == id)
        .or_else(|| items.iter().find(|d| d.id == id))
        .map(|d| d.name)
}

/// The OoTMM display name of a trick id (reverse of `data::TRICK_NAME_TO_ID`).
fn trick_display_name(id: &str) -> Option<&'static str> {
    crate::data::TRICK_NAME_TO_ID
        .iter()
        .find(|(_, tid)| *tid == id)
        .map(|(name, _)| *name)
}

/// Whether a setting key also drives the reachability logic (it is one of the compiled
/// `SETTING_KEYS`). A filter / item setting that is also a logic key must mirror its edit
/// into `raw_settings`, so the single control feeds both the map filter and the solver.
fn is_logic_key(key: &str) -> bool {
    crate::data::SETTING_KEYS.contains(&key)
}

/// The selected members of a set setting's raw value (comma / whitespace separated),
/// dropping the empty-set sentinel `none`.
fn set_members(s: &str) -> impl Iterator<Item = &str> {
    s.split(|c: char| c == ',' || c.is_whitespace())
        .filter(|t| !t.is_empty() && *t != "none")
}

/// The `Special Conditions` block name for a win-condition setting (its `custom`
/// threshold lives there): `rainbowBridge` -> `BRIDGE`, etc. `None` if it has none.
fn special_name_for(key: &str) -> Option<&'static str> {
    Some(match key {
        "rainbowBridge" => "BRIDGE",
        "moon" => "MOON",
        "lacs" => "LACS",
        "ganonBossKey" => "GANON_BK",
        _ => return None,
    })
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
            4 => self.groups_page(ui, FAIRIES_GROUPS),
            5 => self.groups_page(ui, COLLECTIBLES_GROUPS),
            6 => self.groups_page(ui, RULES_GROUPS),
            7 => self.page_progressive(ui),
            8 => self.page_shared(ui),
            9 => self.page_songs(ui),
            10 => self.page_world(ui),
            11 => self.page_starting_tricks(ui),
            12 => self.page_access(ui),
            13 => self.page_layouts(ui),
            _ => {}
        }
    }

    /// A filter page as a 2-column dashboard of group "cards" (the Option C layout):
    /// each card shows a shuffled-count meter and bulk Shuffle-all / Reset actions, so
    /// the state of a whole group reads at a glance and many rows flip in one click.
    fn groups_page(&mut self, ui: &mut egui::Ui, groups: &'static [Group]) {
        ui.columns(2, |cols| {
            for (gi, g) in groups.iter().enumerate() {
                self.group_card(&mut cols[gi % 2], g);
            }
        });
    }

    /// One group card: header (title + `shuffled / total`), a fill meter, the rows,
    /// then the bulk actions.
    fn group_card(&mut self, ui: &mut egui::Ui, g: &'static Group) {
        let mut total = 0usize;
        let mut shuffled = 0usize;
        for &(key, _) in g.rows {
            if find_meta(key).is_some() {
                total += 1;
                if is_shuffled(self.rom_settings.value(key)) {
                    shuffled += 1;
                }
            }
        }
        egui::Frame::group(ui.style())
            .fill(BG_PANEL)
            .stroke(egui::Stroke::new(1.0_f32, BORDER))
            .rounding(8.0)
            .inner_margin(egui::Margin::symmetric(12.0, 10.0))
            .show(ui, |ui| {
                ui.set_width(ui.available_width());
                ui.horizontal(|ui| {
                    ui.label(RichText::new(self.i18n.tr_settings(g.title)).strong().color(ACCENT));
                    ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                        let c = if shuffled > 0 { STATE_ON } else { TEXT_MUTED };
                        ui.label(RichText::new(format!("{shuffled} / {total}")).color(c).size(11.0));
                    });
                });
                ui.add_space(6.0);
                meter(ui, shuffled as f32 / total.max(1) as f32);
                ui.add_space(8.0);
                for &(key, badge) in g.rows {
                    self.card_row(ui, key, badge);
                }
                ui.add_space(8.0);
                ui.horizontal(|ui| {
                    if ui.button(self.i18n.tr_settings("Shuffle all")).clicked() {
                        for &(key, _) in g.rows {
                            if find_meta(key).is_some() {
                                self.rom_settings.set_value(key, data::ShuffleSetting::all);
                            }
                        }
                    }
                    if ui.button(self.i18n.tr_settings("Reset")).clicked() {
                        for &(key, _) in g.rows {
                            if let Some(m) = find_meta(key) {
                                self.rom_settings.set_value(key, m.default);
                            }
                        }
                    }
                });
            });
        ui.add_space(10.0);
    }

    /// One card row: `[state dot] name … [badge] [value combo]`, the dot and the combo
    /// text both coloured by the current state so the row's meaning is scannable. The
    /// combo offers this parameter's REAL OoTMM options (`m.options`) when known, so a
    /// key shows "Own Dungeon / Anywhere / …" and a breakable "None / All / Overworld /
    /// Dungeons"; only settings OoTMM has no option list for fall back to the generic set.
    fn card_row(&mut self, ui: &mut egui::Ui, key: &'static str, badge: Badge) {
        use data::ParamType as PT;
        let Some(m) = find_meta(key) else { return };
        let cur = self.rom_settings.value(key);
        ui.horizontal(|ui| {
            ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                let combo = egui::ComboBox::from_id_salt(key).width(118.0);
                if m.options.is_empty() {
                    // No OoTMM option list (a handful of tracker-only keys): generic set.
                    let opts: &[data::ShuffleSetting] = match m.type_ {
                        PT::shuffle => &SHUFFLE_OPTIONS,
                        _ => &BOOL_OPTIONS,
                    };
                    combo
                        .selected_text(RichText::new(shuffle_label(&self.i18n, cur)).color(state_color(cur)))
                        .show_ui(ui, |ui| {
                            for &opt in opts {
                                if ui.selectable_label(cur == opt, shuffle_label(&self.i18n, opt)).clicked() {
                                    self.rom_settings.set_value(key, opt);
                                    // A boolean setting that also drives the logic (e.g.
                                    // skipZelda) mirrors on/off into raw_settings.
                                    if m.type_ == PT::boolean && is_logic_key(key) {
                                        let raw = if opt == data::ShuffleSetting::all { "true" } else { "false" };
                                        self.rom_settings.raw_settings.insert(key.to_string(), raw.to_string());
                                    }
                                }
                            }
                        });
                } else {
                    // For a logic setting the exact raw value (from raw_settings) is the
                    // source of truth — it disambiguates buckets that collide (ganonBossKey
                    // `ganon` vs `anywhere` both map to `all`). Others show the bucket.
                    let raw_cur = is_logic_key(key)
                        .then(|| self.rom_settings.raw_settings.get(key).cloned())
                        .flatten();
                    // When only the coarse bucket is known (no exact raw value), highlight a
                    // SINGLE representative — the first option that maps to the current bucket
                    // — so an enum whose values share a bucket (e.g. `songs`: songLocations and
                    // anywhere both collapse to `vanilla`) never lights up two options at once.
                    let repr = m
                        .options
                        .iter()
                        .find(|o| crate::settings::setting_bucket(key, o.value) == cur)
                        .map(|o| o.value);
                    let cur_label = raw_cur
                        .as_deref()
                        .and_then(|rc| m.options.iter().find(|o| o.value == rc))
                        .or_else(|| m.options.iter().find(|o| Some(o.value) == repr))
                        .map(|o| self.i18n.tr_settings(o.label))
                        .unwrap_or_else(|| shuffle_label(&self.i18n, cur));
                    combo
                        .selected_text(RichText::new(cur_label).color(state_color(cur)))
                        .show_ui(ui, |ui| {
                            for o in m.options {
                                let bucket = crate::settings::setting_bucket(key, o.value);
                                let selected = match raw_cur.as_deref() {
                                    Some(rc) => o.value == rc,
                                    None => Some(o.value) == repr,
                                };
                                let lbl = RichText::new(self.i18n.tr_settings(o.label)).color(state_color(bucket));
                                if ui.selectable_label(selected, lbl).clicked() {
                                    self.rom_settings.set_value(key, bucket);
                                    // Mirror the exact choice into the logic feed too.
                                    if is_logic_key(key) {
                                        self.rom_settings.raw_settings.insert(key.to_string(), o.value.to_string());
                                    }
                                }
                            }
                        });
                }
                game_badge(ui, badge);
                // Remaining space, left-aligned: the state dot then the (truncating) name.
                ui.with_layout(egui::Layout::left_to_right(egui::Align::Center), |ui| {
                    let (dot, _) = ui.allocate_exact_size(egui::vec2(11.0, 11.0), egui::Sense::hover());
                    ui.painter().circle_filled(dot.center(), 3.5, state_color(cur));
                    ui.add(egui::Label::new(RichText::new(self.i18n.tr_settings(m.name)).size(13.0)).truncate());
                });
            });
        });
    }

    /// One settings card: the same chrome as `group_card` (header + `on / total` count,
    /// fill meter, bulk Shuffle-all / Reset). Each row is a checkbox (boolean setting) or
    /// a combo (everything else), so an item page reads as checkboxes while a numeric
    /// coin count keeps its combo. Built from a dynamic `rows` list so an item card can't
    /// drift from `ITEM_SETTINGS`. An empty `rows` renders nothing.
    fn settings_card(&mut self, ui: &mut egui::Ui, title: &'static str, rows: &[(&'static str, Badge)]) {
        if rows.is_empty() {
            return;
        }
        let mut total = 0usize;
        let mut on = 0usize;
        for &(key, _) in rows {
            if find_meta(key).is_some() {
                total += 1;
                if is_shuffled(self.rom_settings.value(key)) {
                    on += 1;
                }
            }
        }
        egui::Frame::group(ui.style())
            .fill(BG_PANEL)
            .stroke(egui::Stroke::new(1.0_f32, BORDER))
            .rounding(8.0)
            .inner_margin(egui::Margin::symmetric(12.0, 10.0))
            .show(ui, |ui| {
                ui.set_width(ui.available_width());
                ui.horizontal(|ui| {
                    ui.label(RichText::new(self.i18n.tr_settings(title)).strong().color(ACCENT));
                    ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                        let c = if on > 0 { STATE_ON } else { TEXT_MUTED };
                        ui.label(RichText::new(format!("{on} / {total}")).color(c).size(11.0));
                    });
                });
                ui.add_space(6.0);
                meter(ui, on as f32 / total.max(1) as f32);
                ui.add_space(8.0);
                for &(key, badge) in rows {
                    match find_meta(key).map(|m| m.type_) {
                        Some(data::ParamType::boolean) => self.bool_card_row(ui, key, badge),
                        _ => self.card_row(ui, key, badge),
                    }
                }
                ui.add_space(8.0);
                ui.horizontal(|ui| {
                    if ui.button(self.i18n.tr_settings("Shuffle all")).clicked() {
                        for &(key, _) in rows {
                            if find_meta(key).is_some() {
                                self.rom_settings.set_value(key, data::ShuffleSetting::all);
                            }
                        }
                    }
                    if ui.button(self.i18n.tr_settings("Reset")).clicked() {
                        for &(key, _) in rows {
                            if let Some(m) = find_meta(key) {
                                self.rom_settings.set_value(key, m.default);
                            }
                        }
                    }
                });
            });
        ui.add_space(10.0);
    }

    /// A read-only info card (same chrome, no meter or bulk actions): a header with a
    /// count, then one muted line per entry. Used for spoiler-derived data the user
    /// can't toggle here (starting inventory, enabled tricks / glitches).
    fn info_card(&mut self, ui: &mut egui::Ui, title: &'static str, lines: &[String]) {
        egui::Frame::group(ui.style())
            .fill(BG_PANEL)
            .stroke(egui::Stroke::new(1.0_f32, BORDER))
            .rounding(8.0)
            .inner_margin(egui::Margin::symmetric(12.0, 10.0))
            .show(ui, |ui| {
                ui.set_width(ui.available_width());
                ui.horizontal(|ui| {
                    ui.label(RichText::new(self.i18n.tr_settings(title)).strong().color(ACCENT));
                    ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                        let c = if lines.is_empty() { TEXT_MUTED } else { STATE_ON };
                        ui.label(RichText::new(format!("{}", lines.len())).color(c).size(11.0));
                    });
                });
                ui.add_space(6.0);
                if lines.is_empty() {
                    ui.label(RichText::new(self.i18n.tr_settings("(load a spoiler)")).color(TEXT_MUTED).italics());
                } else {
                    for line in lines {
                        ui.label(RichText::new(line).size(13.0).color(TEXT));
                    }
                }
            });
        ui.add_space(10.0);
    }

    /// One boolean card row: `[state dot] name … [badge] [checkbox]`, mirroring
    /// `card_row` but with a checkbox (checked → `all`, unchecked → `vanilla`).
    fn bool_card_row(&mut self, ui: &mut egui::Ui, key: &'static str, badge: Badge) {
        let Some(m) = find_meta(key) else { return };
        let cur = self.rom_settings.value(key);
        ui.horizontal(|ui| {
            ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                let mut on = is_shuffled(cur);
                if ui.checkbox(&mut on, "").changed() {
                    let v = if on { data::ShuffleSetting::all } else { data::ShuffleSetting::vanilla };
                    self.rom_settings.set_value(key, v);
                    // An item toggle that also drives the logic (shared / progressive
                    // items are `SETTING_KEYS`) mirrors on/off into raw_settings.
                    if is_logic_key(key) {
                        let raw = if on { "true" } else { "false" };
                        self.rom_settings.raw_settings.insert(key.to_string(), raw.to_string());
                    }
                }
                game_badge(ui, badge);
                ui.with_layout(egui::Layout::left_to_right(egui::Align::Center), |ui| {
                    let (dot, _) = ui.allocate_exact_size(egui::vec2(11.0, 11.0), egui::Sense::hover());
                    ui.painter().circle_filled(dot.center(), 3.5, state_color(cur));
                    ui.add(egui::Label::new(RichText::new(self.i18n.tr_settings(m.name)).size(13.0)).truncate());
                });
            });
        });
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

    /// Progressive Items: every non-shared, non-song item setting that isn't shown on the
    /// Songs / World Items / Keys pages (Qt `BuildProgressiveItemsPage`), as a card
    /// dashboard split by kind (progressive families, souls, traps, coins, the rest).
    fn page_progressive(&mut self, ui: &mut egui::Ui) {
        use data::ParamCategory as PC;
        // The item settings that belong on this page (not shared, song, or another page).
        let pick = |f: &dyn Fn(&data::SettingMeta) -> bool| -> Vec<(&'static str, Badge)> {
            data::ITEM_SETTINGS
                .iter()
                .filter(|m| {
                    !m.key.starts_with("shared")
                        && !m.key.starts_with("song")
                        && !on_other_page(m.key)
                        && f(m)
                })
                .map(|m| (m.key, badge_from_suffix(m.key)))
                .collect()
        };
        let cards: Vec<(&'static str, Vec<(&'static str, Badge)>)> = vec![
            ("Progressive Items", pick(&|m| m.cat == PC::progressive)),
            ("Souls", pick(&|m| m.cat == PC::souls)),
            ("Traps", pick(&|m| m.cat == PC::standard && m.key.starts_with("trap"))),
            ("Coins", pick(&|m| m.cat == PC::standard && m.key.starts_with("coin"))),
            ("Other Items", pick(&|m| {
                m.cat == PC::standard && !m.key.starts_with("trap") && !m.key.starts_with("coin")
            })),
        ];
        ui.columns(2, |cols| {
            for (i, (title, rows)) in cards.iter().enumerate() {
                self.settings_card(&mut cols[i % 2], title, rows);
            }
        });
    }

    /// Shared Items: every `shared*` item setting except the shared songs, as a card
    /// dashboard (equipment, masks, souls). OoTMM models each as a boolean → checkboxes.
    fn page_shared(&mut self, ui: &mut egui::Ui) {
        let pick = |f: &dyn Fn(&data::SettingMeta) -> bool| -> Vec<(&'static str, Badge)> {
            data::ITEM_SETTINGS
                .iter()
                .filter(|m| m.key.starts_with("shared") && !m.key.starts_with("sharedSong") && f(m))
                .map(|m| (m.key, Badge::None))
                .collect()
        };
        let cards: Vec<(&'static str, Vec<(&'static str, Badge)>)> = vec![
            ("Shared Items", pick(&|m| !m.key.starts_with("sharedSouls") && !m.key.starts_with("sharedMask"))),
            ("Shared Masks", pick(&|m| m.key.starts_with("sharedMask"))),
            ("Shared Souls", pick(&|m| m.key.starts_with("sharedSouls"))),
        ];
        ui.columns(2, |cols| {
            for (i, (title, rows)) in cards.iter().enumerate() {
                self.settings_card(&mut cols[i % 2], title, rows);
            }
        });
    }

    /// Songs: per-game songs + shared songs, as two cards (checkbox rows). Rows are built
    /// from `ITEM_SETTINGS` so the cards track the data. OoTMM models each as a boolean.
    fn page_songs(&mut self, ui: &mut egui::Ui) {
        let per_game: Vec<(&'static str, Badge)> = data::ITEM_SETTINGS
            .iter()
            .filter(|m| m.key.starts_with("song"))
            .map(|m| (m.key, badge_from_suffix(m.key)))
            .collect();
        let shared: Vec<(&'static str, Badge)> = data::ITEM_SETTINGS
            .iter()
            .filter(|m| m.key.starts_with("sharedSong"))
            .map(|m| (m.key, Badge::None))
            .collect();
        ui.columns(2, |cols| {
            self.settings_card(&mut cols[0], "Songs", &per_game);
            self.settings_card(&mut cols[1], "Shared Songs", &shared);
        });
    }

    /// Starting & Tricks: read-only info cards for the spoiler-derived starting inventory
    /// and the enabled tricks / glitches (a trick id whose `GLITCH_` prefix marks it a
    /// glitch). These come from the loaded spoiler, not user toggles.
    fn page_starting_tricks(&mut self, ui: &mut egui::Ui) {
        let mut starting: Vec<String> = self
            .rom_settings
            .starting_item_ids
            .iter()
            .filter_map(|(&id, &n)| {
                item_name_by_id(id).map(|nm| if n > 1 { format!("{nm} ×{n}") } else { nm.to_string() })
            })
            .collect();
        starting.sort();

        let mut tricks: Vec<String> = Vec::new();
        let mut glitches: Vec<String> = Vec::new();
        for &id in &self.rom_settings.enabled_trick_ids {
            let name = trick_display_name(id).unwrap_or(id).to_string();
            if id.starts_with("GLITCH_") {
                glitches.push(name);
            } else {
                tricks.push(name);
            }
        }
        tricks.sort();
        glitches.sort();

        let cards: [(&'static str, &[String]); 3] = [
            ("Starting Items", &starting),
            ("Tricks", &tricks),
            ("Glitches", &glitches),
        ];
        ui.columns(2, |cols| {
            for (i, (title, lines)) in cards.iter().enumerate() {
                self.info_card(&mut cols[i % 2], title, lines);
            }
        });
    }

    /// Logique / Accès: the access + win-condition settings the reachability solver
    /// reads (open dungeons, door of time, rainbow bridge / moon / LACS …) but the
    /// filter / item pages never exposed. Edited straight into `raw_settings` (the raw
    /// OoTMM value string), so the solver honours a manual config with no spoiler; a
    /// loaded spoiler overwrites them (parse_spoiler resets `raw_settings`).
    fn page_access(&mut self, ui: &mut egui::Ui) {
        const WIN: [&str; 4] = ["rainbowBridge", "moon", "lacs", "ganonTrials"];
        let access: Vec<&'static data::AccessSetting> =
            data::ACCESS_SETTINGS.iter().filter(|a| !WIN.contains(&a.key)).collect();
        let conds: Vec<&'static data::AccessSetting> =
            data::ACCESS_SETTINGS.iter().filter(|a| WIN.contains(&a.key)).collect();
        ui.columns(2, |cols| {
            self.access_card(&mut cols[0], "Open Dungeons & Access", &access);
            self.access_card(&mut cols[1], "Win Conditions", &conds);
        });
    }

    /// One access card: header + `active / total` count + meter, then a row per setting
    /// (enum combo, boolean checkbox, or a set's member checkboxes). "Active" = a value
    /// moved away from its OoTMM default (bool true / enum ≠ default / non-empty set).
    fn access_card(&mut self, ui: &mut egui::Ui, title: &'static str, items: &[&'static data::AccessSetting]) {
        let active = items.iter().filter(|a| self.access_active(a)).count();
        let total = items.len();
        egui::Frame::group(ui.style())
            .fill(BG_PANEL)
            .stroke(egui::Stroke::new(1.0_f32, BORDER))
            .rounding(8.0)
            .inner_margin(egui::Margin::symmetric(12.0, 10.0))
            .show(ui, |ui| {
                ui.set_width(ui.available_width());
                ui.horizontal(|ui| {
                    ui.label(RichText::new(self.i18n.tr_settings(title)).strong().color(ACCENT));
                    ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                        let c = if active > 0 { STATE_ON } else { TEXT_MUTED };
                        ui.label(RichText::new(format!("{active} / {total}")).color(c).size(11.0));
                    });
                });
                ui.add_space(6.0);
                meter(ui, active as f32 / total.max(1) as f32);
                ui.add_space(8.0);
                for &a in items {
                    match a.kind {
                        data::AccessKind::Bool => self.access_bool_row(ui, a),
                        data::AccessKind::Enum => self.access_enum_row(ui, a),
                        data::AccessKind::Set => self.access_set_block(ui, a),
                    }
                }
            });
        ui.add_space(10.0);
    }

    /// The current raw value of an access setting (its OoTMM default when unset).
    fn access_value(&self, a: &data::AccessSetting) -> String {
        self.rom_settings
            .raw_settings
            .get(a.key)
            .cloned()
            .unwrap_or_else(|| a.default.to_string())
    }

    /// Whether a setting sits away from its default (drives the dot / active count).
    fn access_active(&self, a: &data::AccessSetting) -> bool {
        let cur = self.access_value(a);
        match a.kind {
            data::AccessKind::Bool => cur == "true",
            data::AccessKind::Enum => cur != a.default,
            data::AccessKind::Set => set_members(&cur).next().is_some(),
        }
    }

    fn access_bool_row(&mut self, ui: &mut egui::Ui, a: &'static data::AccessSetting) {
        let on0 = self.access_value(a) == "true";
        let label = self.i18n.tr_settings(a.name).to_owned();
        let badge = badge_from_suffix(a.key);
        ui.horizontal(|ui| {
            ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                let mut on = on0;
                if ui.checkbox(&mut on, "").changed() {
                    let v = if on { "true" } else { "false" };
                    self.rom_settings.raw_settings.insert(a.key.to_string(), v.to_string());
                }
                game_badge(ui, badge);
                ui.with_layout(egui::Layout::left_to_right(egui::Align::Center), |ui| {
                    let (dot, _) = ui.allocate_exact_size(egui::vec2(11.0, 11.0), egui::Sense::hover());
                    ui.painter().circle_filled(dot.center(), 3.5, if on0 { STATE_ON } else { TEXT_MUTED });
                    ui.add(egui::Label::new(RichText::new(label).size(13.0)).truncate());
                });
            });
        });
    }

    fn access_enum_row(&mut self, ui: &mut egui::Ui, a: &'static data::AccessSetting) {
        let cur = self.access_value(a);
        let active = cur != a.default;
        // A win condition set to `custom` carries a spoiler-defined token threshold;
        // surface it so the row explains what "custom" resolves to.
        let mut label = self.i18n.tr_settings(a.name).to_owned();
        if cur == "custom" {
            if let Some(c) = special_name_for(a.key).and_then(|n| self.rom_settings.special_conds.get(n)) {
                label = format!("{label}  (≥{})", c.count);
            }
        }
        let badge = badge_from_suffix(a.key);
        let cur_label = a
            .options
            .iter()
            .find(|o| o.value == cur)
            .map(|o| self.i18n.tr_settings(o.label).to_owned())
            .unwrap_or_else(|| cur.clone());
        ui.horizontal(|ui| {
            ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                egui::ComboBox::from_id_salt(a.key)
                    .width(130.0)
                    .selected_text(RichText::new(cur_label).color(if active { STATE_ON } else { TEXT_MUTED }))
                    .show_ui(ui, |ui| {
                        for o in a.options {
                            if ui.selectable_label(o.value == cur, self.i18n.tr_settings(o.label)).clicked() {
                                self.rom_settings.raw_settings.insert(a.key.to_string(), o.value.to_string());
                            }
                        }
                    });
                game_badge(ui, badge);
                ui.with_layout(egui::Layout::left_to_right(egui::Align::Center), |ui| {
                    let (dot, _) = ui.allocate_exact_size(egui::vec2(11.0, 11.0), egui::Sense::hover());
                    ui.painter().circle_filled(dot.center(), 3.5, if active { STATE_ON } else { TEXT_MUTED });
                    ui.add(egui::Label::new(RichText::new(label).size(13.0)).truncate());
                });
            });
        });
    }

    /// A set setting (open dungeons / trials): a header line then wrapped member
    /// checkboxes. Selected members are written comma-joined (empty → `none`), which the
    /// solver tokenises for `setting(k, member)` (see `inputs::WorldInputs`).
    fn access_set_block(&mut self, ui: &mut egui::Ui, a: &'static data::AccessSetting) {
        let cur = self.access_value(a);
        let mut members: std::collections::HashSet<String> = set_members(&cur).map(str::to_string).collect();
        let active = !members.is_empty();
        let label = self.i18n.tr_settings(a.name).to_owned();
        let badge = badge_from_suffix(a.key);
        ui.horizontal(|ui| {
            let (dot, _) = ui.allocate_exact_size(egui::vec2(11.0, 11.0), egui::Sense::hover());
            ui.painter().circle_filled(dot.center(), 3.5, if active { STATE_ON } else { TEXT_MUTED });
            ui.label(RichText::new(label).size(13.0).strong());
            game_badge(ui, badge);
            ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                ui.label(RichText::new(format!("{}/{}", members.len(), a.options.len())).size(11.0).color(TEXT_MUTED));
            });
        });
        let mut changed = false;
        ui.horizontal_wrapped(|ui| {
            for o in a.options {
                let mut on = members.contains(o.value);
                if ui.checkbox(&mut on, self.i18n.tr_settings(o.label)).changed() {
                    if on {
                        members.insert(o.value.to_string());
                    } else {
                        members.remove(o.value);
                    }
                    changed = true;
                }
            }
        });
        if changed {
            // Keep the option order, and write `none` for the empty set (OoTMM's default).
            let sel: Vec<&str> = a.options.iter().map(|o| o.value).filter(|v| members.contains(*v)).collect();
            let value = if sel.is_empty() { "none".to_string() } else { sel.join(",") };
            self.rom_settings.raw_settings.insert(a.key.to_string(), value);
        }
        ui.add_space(6.0);
    }

    /// World Items: unique items + masks (shuffle-model cards) plus the open-dungeon,
    /// key-ring, silver-pouch and owl editors (base-set / member / layout toggles) as
    /// matching editor cards (Qt `BuildWorldItemsPage`).
    fn page_world(&mut self, ui: &mut egui::Ui) {
        // Shuffle-model item cards (unique items + masks): checkbox / combo rows.
        ui.columns(2, |cols| {
            for (i, g) in WORLD_PARAM_GROUPS.iter().enumerate() {
                self.settings_card(&mut cols[i % 2], g.title, g.rows);
            }
        });

        // Editor cards: their `on / total` counts need `&self`, so compute them before
        // the `&mut`-borrowing columns closure. (The OoT Fire Temple "open as child" toggle
        // now lives with the other dungeons, on the Logic / Access page's openDungeonsOot
        // set — it drives both the map filter and the logic from there.)
        let ring_total = settings::KEY_RINGS_OOT.len() + settings::KEY_RINGS_MM.len();
        let ring_on = settings::KEY_RINGS_OOT
            .iter()
            .chain(settings::KEY_RINGS_MM.iter())
            .filter(|r| self.rom_settings.key_ring_on(r.ring))
            .count();
        let silver_total = settings::SILVER_AREAS.len();
        let silver_on = settings::SILVER_AREAS
            .iter()
            .filter(|a| {
                self.rom_settings.silver_area_exists(a, &self.mq_scenes)
                    && self.rom_settings.silver_pouch_on(a.pouch)
            })
            .count();
        let owl_total = settings::OWL_STATUES.len();
        let owl_on = settings::OWL_STATUES
            .iter()
            .filter(|o| self.rom_settings.owl_on(o.id))
            .count();
        ui.columns(2, |cols| {
            self.editor_card(&mut cols[0], "Small Key Rings", ring_on, ring_total, |s, ui| {
                for r in settings::KEY_RINGS_OOT {
                    s.key_ring_card_row(ui, r, Badge::Oot);
                }
                for r in settings::KEY_RINGS_MM {
                    s.key_ring_card_row(ui, r, Badge::Mm);
                }
            });
            self.editor_card(&mut cols[1], "Silver Rupee Pouches", silver_on, silver_total, |s, ui| {
                for a in settings::SILVER_AREAS {
                    s.silver_card_row(ui, a);
                }
            });
            self.editor_card(&mut cols[0], "Pre-Activated Owl Statues", owl_on, owl_total, |s, ui| {
                for o in settings::OWL_STATUES {
                    s.owl_card_row(ui, o);
                }
            });
        });
    }

    /// Card chrome for the World-Items editors: framed panel, header with an `on / total`
    /// count and a fill meter, then a custom `body`. (The item-shuffle groups use
    /// `settings_card`; these edit base sets / members / layout, so they bring their own
    /// rows and have no Shuffle-all / Reset.)
    fn editor_card(
        &mut self,
        ui: &mut egui::Ui,
        title: &'static str,
        on: usize,
        total: usize,
        body: impl FnOnce(&mut Self, &mut egui::Ui),
    ) {
        egui::Frame::group(ui.style())
            .fill(BG_PANEL)
            .stroke(egui::Stroke::new(1.0_f32, BORDER))
            .rounding(8.0)
            .inner_margin(egui::Margin::symmetric(12.0, 10.0))
            .show(ui, move |ui| {
                ui.set_width(ui.available_width());
                ui.horizontal(|ui| {
                    ui.label(RichText::new(self.i18n.tr_settings(title)).strong().color(ACCENT));
                    ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                        let c = if on > 0 { STATE_ON } else { TEXT_MUTED };
                        ui.label(RichText::new(format!("{on} / {total}")).color(c).size(11.0));
                    });
                });
                ui.add_space(6.0);
                meter(ui, on as f32 / total.max(1) as f32);
                ui.add_space(8.0);
                body(self, ui);
            });
        ui.add_space(10.0);
    }

    /// A card row: `[state dot] name … [badge] [checkbox]` — the shared layout of the
    /// World-Items editor rows (mirror of `bool_card_row`). `on` colours the dot / drives
    /// the box; `enabled` greys a row whose cluster is absent from the active layout.
    fn editor_row(
        &mut self,
        ui: &mut egui::Ui,
        label: &str,
        badge: Badge,
        on: bool,
        enabled: bool,
        toggle: impl FnOnce(&mut Self, bool),
    ) {
        ui.horizontal(|ui| {
            ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                let mut v = on;
                if ui.add_enabled(enabled, egui::Checkbox::new(&mut v, "")).changed() {
                    toggle(self, v);
                }
                game_badge(ui, badge);
                ui.with_layout(egui::Layout::left_to_right(egui::Align::Center), |ui| {
                    let (dot, _) = ui.allocate_exact_size(egui::vec2(11.0, 11.0), egui::Sense::hover());
                    ui.painter().circle_filled(dot.center(), 3.5, if on { STATE_ON } else { TEXT_MUTED });
                    let mut txt = RichText::new(label).size(13.0);
                    if !enabled {
                        txt = txt.weak();
                    }
                    ui.add(egui::Label::new(txt).truncate());
                });
            });
        });
    }

    fn key_ring_card_row(&mut self, ui: &mut egui::Ui, r: &'static settings::KeyRing, badge: Badge) {
        let on = self.rom_settings.key_ring_on(r.ring);
        let label = self.i18n.tr_settings(r.label).to_owned();
        self.editor_row(ui, &label, badge, on, true, |s, v| {
            s.rom_settings.set_key_ring(r.small, r.ring, v);
        });
    }

    fn silver_card_row(&mut self, ui: &mut egui::Ui, a: &'static settings::SilverArea) {
        let exists = self.rom_settings.silver_area_exists(a, &self.mq_scenes);
        let on = exists && self.rom_settings.silver_pouch_on(a.pouch);
        let label = self.i18n.tr_settings(a.label).to_owned();
        self.editor_row(ui, &label, Badge::Oot, on, exists, |s, v| {
            s.rom_settings.set_silver_pouch(a.rupee, a.pouch, v);
        });
    }

    fn owl_card_row(&mut self, ui: &mut egui::Ui, o: &'static settings::OwlStatue) {
        let on = self.rom_settings.owl_on(o.id);
        let label = self.i18n.tr_settings(o.label).to_owned();
        self.editor_row(ui, &label, Badge::Mm, on, true, |s, v| {
            s.rom_settings.set_owl(o.id, v);
        });
    }

    /// MQ / JP Layouts: per-dungeon Master Quest toggles + the MM JP Deku Palace
    /// toggle (which swaps four scenes at once).
    fn page_layouts(&mut self, ui: &mut egui::Ui) {
        let mq_t = self.i18n.tr_settings("Master Quest Dungeons").to_owned();
        settings_group(ui, 0, &mq_t, |ui| {
            for &(name, scene) in settings::OOT_MQ_DUNGEONS {
                game_badge(ui, Badge::Oot);
                ui.label(self.i18n.tr_settings(name).to_owned());
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
