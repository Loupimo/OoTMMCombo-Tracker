//! Runtime types on top of the generated tables (data.rs).
//!
//! `data.rs` holds the immutable static tables (every scene/object of both
//! games, generated from the CSV pools). This module adds the mutable state
//! (collection) and the render helpers (per-type colour/glyph).

use std::collections::HashSet;
use std::path::{Path, PathBuf};
use std::sync::OnceLock;
use crate::i18n::{I18n};

use crate::data::{
    EntranceDef, ObjectContext, ObjectDef, ObjectType, RoomDef, SceneDef, MM_ENTRANCES, MM_OBJECTS,
    MM_ROOMS, MM_SCENES, OOT_ENTRANCES, OOT_OBJECTS, OOT_ROOMS, OOT_SCENES,
};

/// Repository root, resolved at compile time. Used as the dev fallback when the
/// build is run from the source tree (`cargo run`) rather than a deployed folder.
pub const REPO_ROOT: &str = concat!(env!("CARGO_MANIFEST_DIR"), "/..");

/// The executable's own directory, but only when it looks like a *deployed* build
/// — i.e. it ships a `Resources/` folder next to the `.exe`. `None` in dev, where
/// the exe sits under `target/…` with no adjacent `Resources/`.
fn deployed_dir() -> Option<&'static Path> {
    static DIR: OnceLock<Option<PathBuf>> = OnceLock::new();
    DIR.get_or_init(|| {
        std::env::current_exe()
            .ok()
            .and_then(|exe| exe.parent().map(Path::to_path_buf))
            .filter(|dir| dir.join("Resources").is_dir())
    })
    .as_ref()
    .map(|p| p.as_path())
}

/// Base directory for bundled assets (`Resources/…`): the executable's folder in a
/// deployed build, else the repository root during development.
pub fn base_dir() -> &'static Path {
    deployed_dir().unwrap_or_else(|| Path::new(REPO_ROOT))
}

/// Directory for the tracker's own data files (settings / save / spoiler): next to
/// the executable in a deployed build, or the `rust-tracker` crate dir in dev
/// (where the tracked defaults live).
pub fn data_dir() -> PathBuf {
    match deployed_dir() {
        Some(dir) => dir.to_path_buf(),
        None => Path::new(REPO_ROOT).join("rust-tracker"),
    }
}

#[derive(Clone, Copy, PartialEq, Eq, Hash, Debug)]
pub enum Game {
    Oot,
    Mm,
}

impl Game {
    pub fn label(self) -> &'static str {
        match self {
            Game::Oot => "Ocarina of Time",
            Game::Mm => "Majora's Mask",
        }
    }
    /// Index for per-game arrays (0 = OoT, 1 = MM).
    pub fn idx(self) -> usize {
        match self {
            Game::Oot => 0,
            Game::Mm => 1,
        }
    }
    pub fn scenes(self) -> &'static [SceneDef] {
        match self {
            Game::Oot => OOT_SCENES,
            Game::Mm => MM_SCENES,
        }
    }
    pub fn objects(self) -> &'static [ObjectDef] {
        match self {
            Game::Oot => OOT_OBJECTS,
            Game::Mm => MM_OBJECTS,
        }
    }
    pub fn entrances(self) -> &'static [EntranceDef] {
        match self {
            Game::Oot => OOT_ENTRANCES,
            Game::Mm => MM_ENTRANCES,
        }
    }
    /// The rooms of a scene, or an empty slice for single-image scenes.
    pub fn rooms(self, scene_id: u16) -> &'static [RoomDef] {
        let table = match self {
            Game::Oot => OOT_ROOMS,
            Game::Mm => MM_ROOMS,
        };
        table
            .iter()
            .find(|(id, _)| *id == scene_id)
            .map(|(_, rooms)| *rooms)
            .unwrap_or(&[])
    }
}

/// Absolute path of a `./Resources/...` asset, resolved against the base directory
/// (the executable's folder when deployed, the repo root in dev).
pub fn resource_path(rel: &str) -> String {
    base_dir().join(rel.trim_start_matches("./")).to_string_lossy().into_owned()
}

/// The first of two relative paths that isn't empty.
fn first_non_empty(a: &'static str, b: &'static str) -> &'static str {
    if a.is_empty() {
        b
    } else {
        a
    }
}

/// Resolve an object's map-icon image path: a specific EGameIcon by name, or —
/// for the "render_type" / "type" markers — the icon of its (render) type.
pub fn icon_path_for(map_icon: &str, t: ObjectType) -> Option<&'static str> {
    let by_type = |t: ObjectType| {
        crate::data::ICON_PATHS
            .get(t as usize)
            .copied()
            .filter(|p| !p.is_empty())
    };
    match map_icon {
        "render_type" | "type" | "" => by_type(t),
        name => crate::data::ICON_BY_NAME
            .binary_search_by_key(&name, |&(n, _)| n)
            .ok()
            .map(|i| crate::data::ICON_BY_NAME[i].1)
            .or_else(|| by_type(t)),
    }
}

/// The icon path of a region (Regions.h RegionsMetaInfo), or None for region 0.
pub fn region_icon(game: Game, region_id: u8) -> Option<&'static str> {
    let table = match game {
        Game::Oot => crate::data::OOT_REGION_ICONS,
        Game::Mm => crate::data::MM_REGION_ICONS,
    };
    table.get(region_id as usize).copied().filter(|p| !p.is_empty())
}

/// The display name of a region (Regions.h RegionsMetaInfo), or "" for region 0
/// / unknown. Unlike deriving it from a `SceneDef.region_name`, this table also
/// names the warp regions (OoT "Songs" / MM "Owls"), which own no ordinary scene.
pub fn region_name(game: Game, region_id: u8) -> &'static str {
    let table = match game {
        Game::Oot => crate::data::OOT_REGION_NAMES,
        Game::Mm => crate::data::MM_REGION_NAMES,
    };
    table.get(region_id as usize).copied().unwrap_or("")
}

/// Resolve an entrance's icon (an EntranceIcons variant name) to its image path.
pub fn entrance_icon_path(icon: &str) -> Option<&'static str> {
    crate::data::ENTRANCE_ICONS
        .binary_search_by_key(&icon, |&(n, _)| n)
        .ok()
        .map(|i| crate::data::ENTRANCE_ICONS[i].1)
        .filter(|p| !p.is_empty())
}

/// The base icon render size (px at zoom 1.0) for an object type (Icons.cpp),
/// defaulting to 26 when the type has no explicit size.
pub fn icon_render_size(t: ObjectType) -> f32 {
    crate::data::ICON_TYPE_SIZES
        .get(t as usize)
        .map(|[w, h]| (*w).max(*h) as f32)
        .filter(|&s| s > 0.0)
        .unwrap_or(26.0)
}

/// Marker RGB colour for an object type (render_type).
pub fn color_for(t: ObjectType) -> [u8; 3] {
    use ObjectType::*;
    match t {
        chest => [214, 170, 70],
        gs => [120, 200, 90],
        sf => [235, 225, 120],
        npc | merchant => [235, 170, 90],
        cow => [210, 150, 170],
        shop => [120, 190, 220],
        scrub => [150, 200, 120],
        sr => [200, 200, 210],
        fish => [90, 180, 210],
        heart | heart_piece | heart_container => [230, 90, 110],
        rupee => [90, 170, 230],
        wonder => [190, 130, 230],
        song => [230, 150, 220],
        mask => [220, 120, 90],
        small_key | boss_key => [200, 175, 90],
        map | compass => [140, 160, 210],
        sword | ocarina | egg | owl => [210, 200, 120],
        grass | bush | tree | soil => [110, 190, 120],
        pot | crate_ | barrel | hive | snowball | icicle => [180, 150, 110],
        rock | boulder | redboulder | silverboulder | redice => [150, 150, 155],
        fairy | fairy_spot => [150, 210, 235],
        butterfly => [200, 160, 220],
        _ => [170, 170, 170],
    }
}

/// Category label for the object tree (grouped by type).
pub fn type_label(t: ObjectType, i18n: &I18n) -> &str {
    use ObjectType::*;
    match t {
        chest => i18n.object_type_name("chest"),
        collectible => i18n.object_type_name("collectible"),
        npc => i18n.object_type_name("npc"),
        gs => i18n.object_type_name("gs"),
        sf => i18n.object_type_name("sf"),
        cow => i18n.object_type_name("cow"),
        shop | merchant => i18n.object_type_name("shop"),
        scrub => i18n.object_type_name("scrub"),
        sr => i18n.object_type_name("sr"),
        fish => i18n.object_type_name("fish"),
        wonder => i18n.object_type_name("wonder"),
        grass => i18n.object_type_name("grass"),
        crate_ => i18n.object_type_name("crate_"),
        pot => i18n.object_type_name("pot"),
        hive => i18n.object_type_name("hive"),
        butterfly => i18n.object_type_name("butterfly"),
        rupee => i18n.object_type_name("rupee"),
        snowball => i18n.object_type_name("snowball"),
        barrel => i18n.object_type_name("barrel"),
        heart | heart_piece | heart_container => i18n.object_type_name("heart"),
        fairy | fairy_spot => i18n.object_type_name("fairy"),
        redboulder | silverboulder | boulder | rock => i18n.object_type_name("rock"),
        icicle | redice => i18n.object_type_name("ice"),
        soil => i18n.object_type_name("soil"),
        tree | bush => i18n.object_type_name("bush"),
        song => i18n.object_type_name("song"),
        small_key | boss_key => i18n.object_type_name("key"),
        map | compass => i18n.object_type_name("map"),
        sword | ocarina | mask | egg | owl => i18n.object_type_name("equipement"),
        none => i18n.object_type_name("none"),
    }
}

/// Short glyph drawn at the centre of the marker.
pub fn glyph_for(t: ObjectType) -> &'static str {
    use ObjectType::*;
    match t {
        chest => "C",
        gs => "S",
        sf => "F",
        npc | merchant => "N",
        cow => "M",
        shop => "$",
        scrub => "D",
        heart | heart_piece | heart_container => "♥",
        rupee => "R",
        wonder => "?",
        song => "♪",
        mask => "☺",
        small_key | boss_key => "K",
        fairy | fairy_spot => "*",
        _ => "•",
    }
}

/// A live object placed on a scene: its def plus a mutable collection state.
pub struct LiveObject {
    /// Index into `game.objects()` — the key used by the collected-set.
    pub index: usize,
    pub name: &'static str,
    pub location: &'static str,
    pub type_: ObjectType,
    /// Per-object map icon name (EGameIcon); "render_type"/"type" defer to `type_`.
    pub map_icon: &'static str,
    /// Age / season the object belongs to (for the context display filter).
    pub context: ObjectContext,
    /// Room the object is in (for per-room dungeon rendering).
    pub room: u16,
    pub x: f32,
    pub y: f32,
    pub collected: bool,
}

/// A loaded scene: its metadata plus every object that renders on it.
pub struct LiveScene {
    pub game: Game,
    pub def: &'static SceneDef,
    pub objects: Vec<LiveObject>,
    /// Rooms of a multi-room dungeon (empty = single-image scene).
    pub rooms: &'static [RoomDef],
}

impl LiveScene {
    /// Load a scene: gather the objects that render on it and belong to the
    /// active layout (`mq` = scenes running Master Quest / JP). Collected states
    /// are synced afterwards from the app's collected-set.
    pub fn load(game: Game, def: &'static SceneDef, mq: &HashSet<(Game, u16)>) -> Self {
        let objects = game
            .objects()
            .iter()
            .enumerate()
            .filter(|(_, o)| {
                // Skip shadow placeholders (raw overlay Type == none): the Qt
                // renderer gates on `Type == ObjectType::none` (SceneRenderer.cpp:295),
                // so these never appear as markers or object-tree leaves.
                o.render_scene == def.id
                    && o.type_ != ObjectType::none
                    && crate::tracking::object_active(o, game, mq)
            })
            .map(|(index, o)| LiveObject {
                index,
                name: o.name,
                location: o.location,
                type_: o.render_type,
                map_icon: o.map_icon,
                context: o.context,
                room: o.room,
                x: o.x as f32,
                y: o.y as f32,
                collected: false,
            })
            .collect();
        LiveScene { game, def, objects, rooms: game.rooms(def.id) }
    }

    /// Absolute path of the background image to display.
    ///
    /// - `entrance_view`: entrances live on the scene's minimap.
    /// - otherwise (items): the room image if any, else the artistic map, else
    ///   the minimap as a fallback.
    pub fn image_path(&self, room: usize, entrance_view: bool) -> Option<String> {
        let rel = if entrance_view {
            first_non_empty(self.def.minimap_rel, self.def.image_rel)
        } else if let Some(r) = self.rooms.get(room) {
            r.image_rel
        } else {
            first_non_empty(self.def.image_rel, self.def.minimap_rel)
        };
        (!rel.is_empty()).then(|| resource_path(rel))
    }

    /// The RoomID currently displayed, or None for single-image scenes.
    pub fn active_room_id(&self, room: usize) -> Option<u32> {
        self.rooms.get(room).map(|r| r.id)
    }

    /// (collected, total) for the header.
    pub fn progress(&self) -> (usize, usize) {
        (self.objects.iter().filter(|o| o.collected).count(), self.objects.len())
    }
}
