//! Milestone B (phase 1): map incoming DLL events onto tracked objects.
//!
//! Ports the "collected item" path of MemoryReader::CheckEvent + FindObject:
//! Query[0..1] is reinterpreted as the head of a ComboItemQuery, CorrectComboItem
//! adjusts the id per overlay type, then the object is matched by (scene, id, type).
//!
//! Phase 1 covers the DIRECT overlays (chest / collectible / stray-fairy) and the
//! EXTENDED-FLAG overlays (grass / pot / butterfly / ... encoded in the id). The
//! scene-remapped overlays (GS / shop / npc / cow / scrub / silver-rupee / fish)
//! and the "nothing" drop path need the GetSceneX / ParseKey tables — phase 2.

use std::collections::HashSet;

use crate::data::{GameLayout, ObjectDef};
use crate::scene::Game;
use crate::shared_mem::Event;

const ENTRANCE_MAGIC: u32 = 0xF000_0000;

// Overlay types (Headers/Combo/OvTypes.h). Values 0..=fish share ObjectType's
// numbering, which is what FindObject relies on to compare Type == OvType.
const OV_NONE: u8 = 0x00;
const OV_CHEST: u8 = 0x01;
const OV_COLLECTIBLE: u8 = 0x02;
const OV_NPC: u8 = 0x03;
const OV_SF: u8 = 0x05;
const OV_FISH: u8 = 0x0a;
const OV_XFLAG0: u8 = 0x10;

/// Whether an object belongs to the active layout of ITS scene.
///
/// `mq` is the set of scenes running the Master Quest (OoT) / JP (MM) layout,
/// from the spoiler settings (empty = base game everywhere). An object shows
/// when its layout is `all`, or matches its scene's active layout. Mirrors the
/// C++ `HasCorrectLayout` gate.
pub fn scene_layout_active(
    layout: GameLayout,
    game: Game,
    scene: u16,
    mq: &HashSet<(Game, u16)>,
) -> bool {
    match layout {
        GameLayout::all => true,
        GameLayout::oot | GameLayout::mm => !mq.contains(&(game, scene)),
        GameLayout::oot_mq | GameLayout::mm_jp => mq.contains(&(game, scene)),
    }
}

pub fn object_active(o: &ObjectDef, game: Game, mq: &HashSet<(Game, u16)>) -> bool {
    scene_layout_active(o.layout, game, o.scene, mq)
}

/// Which OoTMM ROM build is running, for the id numbering differences (mirror of
/// the C++ `ROMVersion`). The internal item numbering follows `Dev`; the two
/// stable tiers report lower ids that `resolve_raw_item_id` shifts up. `Stable`
/// (the latest release, v31 / v32.X) only lacks the newest dev items (Shovel /
/// Clock); `Stable301` (v30.1) is older and lacks much more.
#[derive(Clone, Copy, PartialEq, Eq, Debug)]
pub enum RomVersion {
    Dev,
    Stable,
    Stable301,
}

/// Fingerprint of the stable build in SharedData.GameVersion (MemoryReader.cpp).
const STABLE_GAME_VERSION: [u32; 2] = [0x69F7_A146, 0x224A_FE45];

/// Derive the ROM build from the DLL-reported game version. A known fingerprint
/// means a stable release (`Stable`); anything else is a dev build. v30.1 can't be
/// told apart from another stable by fingerprint alone — a loaded spoiler's
/// `Version:` line refines it to `Stable301` (mirror of MemoryReader.cpp).
pub fn detect_rom(game_version: [u32; 2]) -> RomVersion {
    if game_version == STABLE_GAME_VERSION {
        RomVersion::Stable
    } else {
        RomVersion::Dev
    }
}

/// Decode an event into the collected object `(game, index)` it refers to,
/// where `index` indexes into `game.objects()`. Returns None when the event is
/// not a phase-1 collected item, or when no object matches.
pub fn resolve_collected(
    ev: &Event,
    rom: RomVersion,
    mq: &HashSet<(Game, u16)>,
) -> Option<(Game, usize)> {
    // Entrance messages are handled elsewhere.
    if ev.mem & 0xF000_0000 == ENTRANCE_MAGIC {
        return None;
    }

    if ev.query[2] & 0xFFFF_0000 == 0xFFFF_0000 {
        // "Nothing" drop: an empty grass / pot / ... marks its location checked.
        // The placement is a combo key in Query[0] (ParseKey after byteswap32),
        // the game in the low byte of Query[2].
        let game = if ev.query[2] & 0xFF == 0 { Game::Oot } else { Game::Mm };
        let ov_type = ((ev.query[0] >> 24) & 0xFF) as u8;
        let scene = ((ev.query[0] >> 16) & 0xFF) as u16;
        let room = (ev.query[0] >> 8) & 0xFF;
        let id = ev.query[0] & 0xFF;
        return match_object(game, scene, ov_type, id, room, rom, mq);
    }

    // Collected item: Query[0..1] is the head of a ComboItemQuery.
    // MM hooks live above this PC; OoT below (see CheckEvent).
    let game = if ev.pc > 0x8070_0000 { Game::Mm } else { Game::Oot };
    let scene = (ev.query[0] & 0xFF) as u16;
    let ov_type = ((ev.query[0] >> 8) & 0xFF) as u8;
    let id = (ev.query[1] >> 16) & 0xFF;
    let room = (ev.query[1] >> 24) & 0xFF;
    match_object(game, scene, ov_type, id, room, rom, mq)
}

/// CorrectComboItem + FindObject: resolve a decoded placement to a pool object.
/// Shared by the collected-item, "nothing" drop and network-ledger paths.
pub(crate) fn match_object(
    game: Game,
    scene: u16,
    ov_type: u8,
    id: u32,
    room: u32,
    rom: RomVersion,
    mq: &HashSet<(Game, u16)>,
) -> Option<(Game, usize)> {
    if ov_type == OV_NONE {
        return None;
    }
    if ov_type >= OV_XFLAG0 {
        // Extended flag: fold the flag index + room into the id, match in-scene.
        let composite = (((ov_type - OV_XFLAG0) as u32) << 16) | (room << 8) | id;
        return find_object(game, Some(scene), composite, ov_type, mq);
    }
    match ov_type {
        // Direct overlays: matched within the reported scene.
        OV_CHEST | OV_COLLECTIBLE | OV_SF => find_object(game, Some(scene), id, ov_type, mq),
        // NPCs need the stable/dev id fix-up before the (global) lookup.
        OV_NPC => find_object(game, None, resolve_raw_npc(game, id, rom), OV_NPC, mq),
        // gs / cow / shop / scrub / sr / fish: the pool object already carries the
        // right scene, so match globally by (type, id) — no GetSceneX port needed.
        _ => find_object(game, None, id, ov_type, mq),
    }
}

/// Port of FindObject. `scene` = Some(id) scopes the search to one scene (direct
/// / extended overlays); None searches the whole game (scene-remapped overlays,
/// whose scene the pool already encodes). Objects outside the active layout are
/// skipped, mirroring HasCorrectLayout.
fn find_object(
    game: Game,
    scene: Option<u16>,
    object_id: u32,
    ov_type: u8,
    mq: &HashSet<(Game, u16)>,
) -> Option<(Game, usize)> {
    for (i, o) in game.objects().iter().enumerate() {
        if !object_active(o, game, mq) || o.object_id != object_id {
            continue;
        }
        if let Some(s) = scene {
            if o.scene != s {
                continue;
            }
        }
        let type_num = o.type_ as u8;
        let ok = if ov_type > OV_FISH {
            type_num > OV_FISH // any extended object type
        } else {
            type_num == ov_type
        };
        if ok {
            return Some((game, i));
        }
    }
    None
}

/// Port of ResolveRawOoTNpcID: on stable ROMs the GS-reward NPCs (0x13..=0x17)
/// are reported one higher than the tracker's dev numbering. OoT only.
fn resolve_raw_npc(game: Game, id: u32, rom: RomVersion) -> u32 {
    // Only v30.1 still carries the unused SCRUB_HP slot; every later build (stable
    // or dev) already uses the tracker's NPC numbering.
    if game == Game::Oot && rom == RomVersion::Stable301 && (0x13..=0x17).contains(&id) {
        id - 1
    } else {
        id
    }
}

/// Port of `ResolveRawItemID` (Items.cpp): on a stable ROM the ledger carries the
/// lower stable item ids, which the dev build shifts up at each boundary below.
/// Add the cumulative shift back so the id matches the tracker's internal (dev)
/// numbering. Dev builds are the identity. Tested high -> low.
pub fn resolve_raw_item_id(raw: u32, rom: RomVersion) -> u32 {
    match rom {
        // Dev builds already use the tracker's internal numbering.
        RomVersion::Dev => raw,
        // Latest stable (v31 / v32.X): only the newest dev items are missing, so a
        // handful of insertion points. Generated from the old (v31/32.X) vs new
        // (dev) iid numbering — see tools scratchpad compute_shift.py. Tested high->low.
        RomVersion::Stable => match raw {
            r if r >= 1024 => r + 4, // +1 at SHARED_SHOVEL (before SHARED_SKELETON_KEY)
            r if r >= 933 => r + 3,  // +1 at MM_SHOVEL (before MM_MASK_MAJORA)
            r if r >= 509 => r + 2,  // +2 at OOT_CLOCK/OOT_SHOVEL (before MM_RUPEE_GREEN)
            r => r,
        },
        // v30.1: the older, larger shift, recomputed after the Shovel/Clock inserts
        // (compose the v30.1->dev table with the dev renumber). Tested high->low.
        RomVersion::Stable301 => match raw {
            r if r >= 826 => r + 110, // MM_MASK_MAJORA (MM rusty keys)
            r if r >= 818 => r + 70,  // MM_STICK_UPGRADE
            r if r >= 661 => r + 69,  // MM_SOUL_ENEMY_OCTOROK
            r if r >= 622 => r + 64,  // MM_REMAINS_ODOLWA
            r if r >= 610 => r + 56,  // MM_SONG_GORON_HALF
            r if r >= 576 => r + 48,  // MM_CHATEAU
            r if r >= 466 => r + 45,  // MM_RUPEE_GREEN (OoT rusty keys)
            r if r >= 458 => r + 16,  // OOT_TRAP_ICE
            r if r >= 452 => r + 15,  // OOT_WEIRD_MUSHROOM
            r if r >= 451 => r + 14,  // OOT_SCALE_BRONZE
            r if r >= 155 => r + 13,  // OOT_SWORD_MASTER
            r if r >= 142 => r + 7,   // OOT_SONG_NOTE_TP_FOREST
            r => r,                   // low ids are identical in both builds
        },
    }
}

/// Port of `FindItem(ResolveRawItemID(gi))`: resolve a raw ledger item id to its
/// English name. `ITEMS` is dense and id-ordered (item ids start at 1), so the
/// direct index hits; a fallback linear search covers any gap. Returns None for
/// an unknown / zero id (e.g. a "nothing" drop).
pub fn net_item_name(gi: u16, rom: RomVersion) -> Option<&'static str> {
    let id = resolve_raw_item_id(gi as u32, rom);
    let items = crate::data::ITEMS;
    (id as usize)
        .checked_sub(1)
        .and_then(|i| items.get(i))
        .filter(|d| d.id == id)
        .or_else(|| items.iter().find(|d| d.id == id))
        .map(|d| d.name)
}

/// Build a synthetic event that resolves to `o` (for the "Simulate" button).
/// Returns None for objects this path cannot represent (e.g. ids that don't fit
/// the DLL's single id byte, or render-only types).
pub fn demo_event(game: Game, o: &ObjectDef) -> Option<Event> {
    let t = o.type_ as u8;
    let (ov, id, room) = if t > OV_FISH {
        // Extended-flag object: split the composite id back into ov / room / id.
        if o.object_id > 0xFF {
            (OV_XFLAG0 + (o.object_id >> 16) as u8, o.object_id & 0xFF, (o.object_id >> 8) & 0xFF)
        } else {
            (OV_XFLAG0, o.object_id, 0)
        }
    } else if t == OV_NONE || o.object_id > 0xFF {
        return None; // no overlay, or id wider than the DLL's id byte
    } else {
        // Direct or scene-remapped overlay: the ObjectType value IS its OV value.
        (t, o.object_id, 0)
    };

    let q0 = (o.scene as u32 & 0xFF) | ((ov as u32) << 8);
    let q1 = (id << 16) | (room << 24);
    let pc = if game == Game::Mm { 0x8071_0000 } else { 0x8009_0000 };
    Some(Event { pc, mem: 0, query: [q0, q1, 0, 0, 0, 0] })
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::data::{ObjectType, MM_OBJECTS, OOT_OBJECTS};

    /// Empty Master-Quest set: base layout everywhere.
    fn no_mq() -> HashSet<(Game, u16)> {
        HashSet::new()
    }

    /// Every encodable, active-layout object must decode back to its placement.
    #[test]
    fn encode_decode_round_trip() {
        for (game, objs) in [(Game::Oot, OOT_OBJECTS), (Game::Mm, MM_OBJECTS)] {
            let mut encodable = 0usize;
            for o in objs {
                if !object_active(o, game, &no_mq()) {
                    continue; // base layout only in this test
                }
                let Some(ev) = demo_event(game, o) else { continue };
                encodable += 1;
                let (_, j) = resolve_collected(&ev, RomVersion::Dev, &no_mq())
                    .unwrap_or_else(|| panic!("no match: {} (id {:#x})", o.location, o.object_id));
                // The matched object shares the placement key (scene + id).
                assert_eq!(objs[j].scene, o.scene, "scene mismatch: {}", o.location);
                assert_eq!(objs[j].object_id, o.object_id, "id mismatch: {}", o.location);
            }
            assert!(encodable > 500, "too few encodable objects: {encodable}");
        }
    }

    /// A scene-remapped overlay (gold skulltula) resolves via the GLOBAL search
    /// even when the event's scene byte is wrong — the pool recovers the scene.
    #[test]
    fn remapped_type_matches_globally() {
        let o = OOT_OBJECTS
            .iter()
            .find(|o| matches!(o.type_, ObjectType::gs) && object_active(o, Game::Oot, &no_mq()) && o.object_id <= 0xFF)
            .expect("a base-layout GS with a byte-sized id");
        // Deliberately bogus scene byte (0xFF); ov = gs; id in the id byte.
        let ev = Event {
            pc: 0x8009_0000,
            mem: 0,
            query: [0xFF | ((o.type_ as u32) << 8), o.object_id << 16, 0, 0, 0, 0],
        };
        let (_, j) = resolve_collected(&ev, RomVersion::Dev, &no_mq()).expect("gs resolves globally");
        assert_eq!(OOT_OBJECTS[j].object_id, o.object_id);
        assert_eq!(OOT_OBJECTS[j].type_ as u8, ObjectType::gs as u8);
    }

    /// The scene-remapped overlays can be matched globally by (type, id) only
    /// if that pair is unique per game — verify it, since phase 2 relies on it.
    #[test]
    fn remap_ids_unique_per_type() {
        use std::collections::HashMap;
        let remap = [
            ObjectType::npc, ObjectType::gs, ObjectType::cow,
            ObjectType::shop, ObjectType::scrub, ObjectType::sr, ObjectType::fish,
        ];
        for (game, objs) in [(Game::Oot, OOT_OBJECTS), (Game::Mm, MM_OBJECTS)] {
            let mut seen: HashMap<(u8, u32), &str> = HashMap::new();
            for o in objs {
                if remap.contains(&o.type_) && object_active(o, game, &no_mq()) {
                    let key = (o.type_ as u8, o.object_id);
                    if let Some(prev) = seen.insert(key, o.location) {
                        panic!(
                            "duplicate (type {}, id {:#x}): {} vs {}",
                            o.type_ as u8, o.object_id, prev, o.location
                        );
                    }
                }
            }
        }
    }

    /// A "nothing" drop (empty grass) marks its placement, decoded from the
    /// Query[0] combo key.
    #[test]
    fn nothing_drop_marks_placement() {
        let o = OOT_OBJECTS
            .iter()
            .find(|o| matches!(o.type_, ObjectType::grass) && object_active(o, Game::Oot, &no_mq()) && o.object_id > 0xFF)
            .expect("a base-layout grass with a composite id");
        let ov = OV_XFLAG0 as u32 + (o.object_id >> 16);
        let room = (o.object_id >> 8) & 0xFF;
        let id = o.object_id & 0xFF;
        let q0 = id | (room << 8) | ((o.scene as u32 & 0xFF) << 16) | (ov << 24);
        // Query[2] high half = 0xFFFF marks the "nothing" path; low byte = game.
        let ev = Event { pc: 0x8009_0000, mem: 0, query: [q0, 0, 0xFFFF_0000, 0, 0, 0] };
        let (_, j) = resolve_collected(&ev, RomVersion::Dev, &no_mq()).expect("nothing-drop grass resolves");
        assert_eq!(OOT_OBJECTS[j].object_id, o.object_id);
        assert_eq!(OOT_OBJECTS[j].type_ as u8, ObjectType::grass as u8);
    }

    /// The stable-ROM item-id shift is the identity on dev and adds the boundary
    /// shift on stable, and a resolved id maps to a real item name.
    #[test]
    fn net_item_name_resolves() {
        // Dev: identity. Item id 4 is the Fairy Bow (OoT).
        assert_eq!(resolve_raw_item_id(4, RomVersion::Dev), 4);
        assert_eq!(net_item_name(4, RomVersion::Dev), Some("Fairy Bow (OoT)"));
        // v30.1: an id past a boundary is shifted up by the (recomputed) amount.
        assert_eq!(resolve_raw_item_id(142, RomVersion::Stable301), 142 + 7);
        assert_eq!(resolve_raw_item_id(141, RomVersion::Stable301), 141);
        assert_eq!(resolve_raw_item_id(826, RomVersion::Stable301), 826 + 110);
        // Latest stable (v31 / v32.X): only the newest dev items shift it. Below the
        // first insertion (MM_RUPEE_GREEN = 509) it is the identity; at / past each
        // insertion it climbs +2 / +3 / +4.
        assert_eq!(resolve_raw_item_id(142, RomVersion::Stable), 142);
        assert_eq!(resolve_raw_item_id(509, RomVersion::Stable), 511);
        assert_eq!(resolve_raw_item_id(933, RomVersion::Stable), 936);
        assert_eq!(resolve_raw_item_id(1024, RomVersion::Stable), 1028);
        // A zero / unknown id (e.g. a "nothing" drop) has no name.
        assert_eq!(net_item_name(0, RomVersion::Dev), None);
    }

    /// Entrance messages, and empty (ov=none) events, are ignored.
    #[test]
    fn non_item_events_ignored() {
        let entrance = Event { pc: 0x8009_0000, mem: 0xF000_1234, query: [0; 6] };
        assert!(resolve_collected(&entrance, RomVersion::Dev, &no_mq()).is_none());
        let empty = Event { pc: 0x8009_0000, mem: 0, query: [0, 0, 0xFFFF_0000, 0, 0, 0] };
        assert!(resolve_collected(&empty, RomVersion::Dev, &no_mq()).is_none());
    }
}
