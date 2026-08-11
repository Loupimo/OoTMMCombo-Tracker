//! Runtime types on top of the generated tables (data.rs).
//!
//! `data.rs` holds the immutable static tables (every scene/object of both
//! games, generated from the CSV pools). This module adds the mutable state
//! (collection) and the render helpers (per-type colour/glyph).

use std::collections::HashSet;

use crate::data::{
    EntranceDef, ObjectContext, ObjectDef, ObjectType, RoomDef, SceneDef, MM_ENTRANCES, MM_OBJECTS,
    MM_ROOMS, MM_SCENES, OOT_ENTRANCES, OOT_OBJECTS, OOT_ROOMS, OOT_SCENES,
};

/// Repository root, resolved at compile time: lets us load the images
/// (`./Resources/...`) whatever the cwd is during `cargo run`.
pub const REPO_ROOT: &str = concat!(env!("CARGO_MANIFEST_DIR"), "/..");

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

/// Absolute path of a `./Resources/...` asset relative to the repository.
pub fn resource_path(rel: &str) -> String {
    format!("{REPO_ROOT}/{}", rel.trim_start_matches("./"))
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
pub fn type_label(t: ObjectType) -> &'static str {
    use ObjectType::*;
    match t {
        chest => "Coffres",
        collectible => "Collectibles",
        npc => "PNJ / Évènements",
        gs => "Skulltulas d'or",
        sf => "Fées égarées",
        cow => "Vaches",
        shop | merchant => "Boutiques",
        scrub => "Scrubs",
        sr => "Rupees d'argent",
        fish => "Poissons",
        wonder => "Objets cachés",
        grass => "Herbes",
        crate_ => "Caisses",
        pot => "Pots",
        hive => "Ruches",
        butterfly => "Papillons",
        rupee => "Rupees",
        snowball => "Boules de neige",
        barrel => "Tonneaux",
        heart | heart_piece | heart_container => "Cœurs",
        fairy | fairy_spot => "Fées",
        redboulder | silverboulder | boulder | rock => "Rochers",
        icicle | redice => "Glace",
        soil => "Terre",
        tree | bush => "Végétation",
        song => "Chansons",
        small_key | boss_key => "Clés",
        map | compass => "Cartes / Boussoles",
        sword | ocarina | mask | egg | owl => "Équipement",
        none => "Autres",
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
