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
use std::sync::atomic::{AtomicBool, Ordering};

use crate::data::{GameLayout, ObjSystem, ObjectDef};
use crate::scene::Game;
use crate::shared_mem::Event;

const ENTRANCE_MAGIC: u32 = 0xF000_0000;

/// Whether the loaded ROM predates the compact-XflagID rework (stable <= v32.3). Mirror of the
/// C++ `UsesLegacyXflagsFlag` global: published once when a spoiler loads (`set_uses_legacy`),
/// read by `object_active` to gate version-specific objects (ObjSystem::Legacy / New) across
/// both display and resolution without threading the flag through every call site.
static USES_LEGACY: AtomicBool = AtomicBool::new(false);

/// Publish the ROM's xflag system (spoiler load / reset). Mirror of C++ SetUsesLegacyXflags.
pub fn set_uses_legacy(legacy: bool) {
    USES_LEGACY.store(legacy, Ordering::Relaxed);
}

/// Whether the loaded ROM uses the legacy xflag system. Mirror of C++ UsesLegacyXflags().
pub fn uses_legacy() -> bool {
    USES_LEGACY.load(Ordering::Relaxed)
}

// Overlay types (Headers/Combo/OvTypes.h). Values 0..=fish share ObjectType's
// numbering, which is what FindObject relies on to compare Type == OvType.
const OV_NONE: u8 = 0x00;
const OV_CHEST: u8 = 0x01;
const OV_COLLECTIBLE: u8 = 0x02;
const OV_NPC: u8 = 0x03;
const OV_GS: u8 = 0x04;
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

/// Whether an object's xflag system matches the loaded ROM's. Version-specific objects (a check
/// whose representation changed across OoTMM versions, e.g. the Kokiri Forest crawl grass) exist
/// under only one system: Legacy on <= v32.3 ROMs, New on > v32.3, Any on both. Pure so the gate
/// can be unit-tested without the `USES_LEGACY` global.
fn system_active(system: ObjSystem, legacy: bool) -> bool {
    match system {
        ObjSystem::Legacy => legacy,
        ObjSystem::New => !legacy,
        ObjSystem::Any => true,
    }
}

pub fn object_active(o: &ObjectDef, game: Game, mq: &HashSet<(Game, u16)>) -> bool {
    // Mirror of C++ HasCorrectLayout: layout gate, then the ObjSystem gate.
    scene_layout_active(o.layout, game, o.scene, mq) && system_active(o.system, uses_legacy())
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
    uses_legacy: bool,
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
        return match_object(game, scene, ov_type, id, room, rom, uses_legacy, mq);
    }

    // Collected item: Query[0..1] is the head of a ComboItemQuery.
    // MM hooks live above this PC; OoT below (see CheckEvent).
    let game = if ev.pc > 0x8070_0000 { Game::Mm } else { Game::Oot };
    let scene = (ev.query[0] & 0xFF) as u16;
    let ov_type = ((ev.query[0] >> 8) & 0xFF) as u8;
    let id = (ev.query[1] >> 16) & 0xFF;
    let room = (ev.query[1] >> 24) & 0xFF;
    match_object(game, scene, ov_type, id, room, rom, uses_legacy, mq)
}

/// CorrectComboItem + FindObject: resolve a decoded placement to a pool object.
/// Shared by the collected-item, "nothing" drop and network-ledger paths.
///
/// `uses_legacy` selects the xflag system: legacy ROMs (<= v32.3) put the full
/// scene / room / actor identity in the key; newer ROMs send only a compact
/// XflagID that we resolve by Location (mirror of C++ ResolveXflagItem).
pub(crate) fn match_object(
    game: Game,
    scene: u16,
    ov_type: u8,
    id: u32,
    room: u32,
    rom: RomVersion,
    uses_legacy: bool,
    mq: &HashSet<(Game, u16)>,
) -> Option<(Game, usize)> {
    if ov_type == OV_NONE {
        return None;
    }
    if ov_type >= OV_XFLAG0 {
        if !uses_legacy {
            // Compact system (> v32.3): the query only holds a 16-bit XflagID, split
            // across room (high byte) and id (low byte), with no scene. Resolve it by
            // Location (layout-aware). An unknown id (table built from a different ROM
            // version) falls through to the legacy scan, which fails harmlessly.
            let xflag_id = (((room & 0xFF) << 8) | (id & 0xFF)) as u16;
            if let Some(hit) = find_object_by_xflag_id(game, xflag_id, mq) {
                return Some(hit);
            }
        }
        // Legacy flag: fold the flag index (slice) + room into the id, match in-scene.
        let composite = (((ov_type - OV_XFLAG0) as u32) << 16) | (room << 8) | id;
        return find_object(game, Some(scene), composite, ov_type, mq);
    }
    match ov_type {
        // Direct overlays: matched within the reported scene.
        OV_CHEST | OV_COLLECTIBLE | OV_SF => find_object(game, Some(scene), id, ov_type, mq),
        // NPCs need the stable/dev id fix-up before the (global) lookup.
        OV_NPC => find_object(game, None, resolve_raw_npc(game, id, rom), OV_NPC, mq),
        // Gold skulltulas: the game reports the raw GS flag, but the pool keys every
        // GS on flag+8, so shift before the (global by type+id) lookup (see resolve_raw_gs).
        OV_GS => find_object(game, None, resolve_raw_gs(game, id), OV_GS, mq),
        // cow / shop / scrub / sr / fish: the pool object already carries the
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
        if type_matches(o.type_ as u8, ov_type) {
            return Some((game, i));
        }
    }
    // A few checks moved scene since the legacy (<= v32.3) ROMs: when the reported
    // scene came up empty, retry via the legacy-scene remap (mirror of C++ FindObject
    // falling back to FindObjectByLegacyScene).
    if let Some(s) = scene {
        return find_object_by_legacy_scene(game, s, object_id, ov_type, mq);
    }
    None
}

/// Whether an object of raw type `obj_type` satisfies an overlay of type `ov_type`:
/// exact match for the direct overlays, "any extended type" above fish (whose
/// ObjectType is render-only). Shared by every object scan.
fn type_matches(obj_type: u8, ov_type: u8) -> bool {
    if ov_type > OV_FISH {
        obj_type > OV_FISH
    } else {
        obj_type == ov_type
    }
}

/// Compact-XflagID lookup (new xflag ROMs > v32.3): the DLL no longer sends the
/// scene / room / actor, only a 16-bit XflagID stamped on each object by Location.
/// Return the active-layout object carrying it. A single XflagID can belong to
/// several objects when a check is placed differently across layouts (Deku Palace
/// rupees in MM_JP, Ice Cavern Sheik song in OoT_MQ): `object_active` keeps the one
/// whose scene runs the active layout, mirroring C++ FindObjectByXflagID.
fn find_object_by_xflag_id(
    game: Game,
    xflag_id: u16,
    mq: &HashSet<(Game, u16)>,
) -> Option<(Game, usize)> {
    if xflag_id == 0xFFFF {
        return None; // sentinel: not an xflag object
    }
    game.objects()
        .iter()
        .position(|o| o.xflag_id == xflag_id && object_active(o, game, mq))
        .map(|i| (game, i))
}

/// Legacy-scene remap fallback: a handful of checks (cows, Granny's potions, hatch
/// eggs, Oath to Order, Tingle maps) live under a different scene now than the one
/// pre-migration (<= v32.3) ROMs report. Given the reported (legacy) scene + the
/// object's id, resolve it in its true scene. Safe for every ROM: the key only ever
/// matches the old scene, which current ROMs no longer emit. Mirror of C++
/// FindObjectByLegacyScene.
///
/// The current tables are all cow / npc entries, which this runtime already resolves
/// through the global `(type, id)` match (scene = None), so the fallback is only
/// reached by scene-scoped overlays (chest / collectible / sf / extended) — kept for
/// parity with the Qt tracker and to cover any future scene-scoped remap.
fn find_object_by_legacy_scene(
    game: Game,
    reported_scene: u16,
    object_id: u32,
    ov_type: u8,
    mq: &HashSet<(Game, u16)>,
) -> Option<(Game, usize)> {
    let table = match game {
        Game::Oot => crate::data::OOT_LEGACY_SCENE_REMAP,
        Game::Mm => crate::data::MM_LEGACY_SCENE_REMAP,
    };
    for r in table {
        if r.legacy_scene != reported_scene || r.object_id != object_id {
            continue;
        }
        // Scan the true scene directly (no recursion back through find_object).
        for (i, o) in game.objects().iter().enumerate() {
            if o.scene == r.true_scene
                && object_active(o, game, mq)
                && o.object_id == object_id
                && type_matches(o.type_ as u8, ov_type)
            {
                return Some((game, i));
            }
        }
    }
    None
}

/// OoT gold-skulltula id fix-up. The game reports the raw GS flag `F` (the value the
/// OoTMM patch puts in the ComboItemQuery, mirrored by the OoTMM checks XML `flag`),
/// but the tracker keys every GS object — and the C++ `GetSceneGS` — on `F + 8`. Add
/// the +8 back before the global `(type, id)` lookup, otherwise flag `F` resolves to
/// whichever GS was stamped with object_id `F` (8 slots too early): e.g. Zora River
/// GS Tree (flag 0x89) wrongly marking Kakariko GS Bazaar (object_id 0x89, flag 0x81).
/// OoT only — MM skulltulas are not resolved through this flag-remapped path (C++
/// `CorrectComboItem` leaves MM `OV_GS` untouched). Applied on every ROM: the pool's
/// +8 offset is not version-specific, and no observed build sends `F + 8` directly.
fn resolve_raw_gs(game: Game, id: u32) -> u32 {
    if game == Game::Oot {
        id + 8
    } else {
        id
    }
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
        // Latest stable (v31 / v32.X): the dev build has the four items the stable
        // ROM lacks. dev-33 ALSO reordered the OoT stick/nut upgrades — the base
        // upgrade was pushed up to 0x80/0x81 to open room for the new UPGRADE3 tier,
        // and UPGRADE2 slid down — so raw 0x77..0x7a are an explicit permutation, not
        // a step. Recomposed from the (v31/32.X)->old-dev table and the dev-33
        // renumber (Items.h vs items_old.h, see tools scratchpad). Exact arms first.
        RomVersion::Stable => match raw {
            0x77 => 0x80, // OOT_STICK_UPGRADE  (base moved up)
            0x78 => 0x77, // OOT_STICK_UPGRADE2 (slid down)
            0x79 => 0x81, // OOT_NUT_UPGRADE    (base moved up)
            0x7a => 0x79, // OOT_NUT_UPGRADE2   (slid down)
            r if r >= 1024 => r + 9, // +5 (Clock/Shovel/…) + 4 (dev-33 UPGRADE3 x4)
            r if r >= 933 => r + 8,  // +4 + 4
            r if r >= 890 => r + 7,  // MM_NUT_UPGRADE region: +3 + 4
            r if r >= 888 => r + 6,  // MM_STICK_UPGRADE region: +3 + 3
            r if r >= 626 => r + 5,  // MM_MASK_ADULT: +4 + 1
            r if r >= 509 => r + 4,  // +2 (OoT Clock/Shovel) + 2 (OoT UPGRADE3 x2)
            r if r >= 128 => r + 2,  // OoT UPGRADE3 x2 inserted at 0x78 / 0x7a
            r => r,                  // < 0x77 and 0x7b..0x7f are unchanged
        },
        // v30.1: the older, larger shift, recomposed after the dev-33 renumber (the
        // same OoT stick/nut reorder applies at the low ids). Tested high->low.
        RomVersion::Stable301 => match raw {
            0x77 => 0x80, // OOT_STICK_UPGRADE
            0x78 => 0x77, // OOT_STICK_UPGRADE2
            0x79 => 0x81, // OOT_NUT_UPGRADE
            0x7a => 0x79, // OOT_NUT_UPGRADE2
            r if r >= 826 => r + 114, // MM_MASK_MAJORA (MM rusty keys)
            r if r >= 822 => r + 74,  // MM_STONE_OF_AGONY
            r if r >= 820 => r + 73,  // MM_NUT_UPGRADE
            r if r >= 818 => r + 72,  // MM_STICK_UPGRADE
            r if r >= 661 => r + 71,  // MM_SOUL_ENEMY_OCTOROK
            r if r >= 622 => r + 66,  // MM_REMAINS_ODOLWA
            r if r >= 610 => r + 58,  // MM_SONG_GORON_HALF
            r if r >= 576 => r + 50,  // MM_CHATEAU
            r if r >= 466 => r + 47,  // MM_RUPEE_GREEN (OoT rusty keys)
            r if r >= 458 => r + 18,  // OOT_TRAP_ICE
            r if r >= 452 => r + 17,  // OOT_WEIRD_MUSHROOM
            r if r >= 451 => r + 16,  // OOT_SCALE_BRONZE
            r if r >= 155 => r + 15,  // OOT_SWORD_MASTER
            r if r >= 142 => r + 9,   // OOT_SONG_NOTE_TP_FOREST
            r if r >= 128 => r + 2,   // OOT_RUPEE_RAINBOW (OoT UPGRADE3 inserts)
            r => r,                   // < 0x77 and 0x7b..0x7f are unchanged
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

/// Build a synthetic event that resolves to `o` (test-only, exercised by the
/// `encode_decode_round_trip` check). Returns None for objects this path cannot
/// represent (e.g. ids that don't fit the DLL's single id byte, or render-only types).
#[cfg(test)]
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
    } else if t == OV_GS && game == Game::Oot {
        // Gold skulltula: the DLL reports the raw flag (object_id - 8), which
        // resolve_raw_gs shifts back. Emit the raw flag for a faithful round-trip.
        (t, o.object_id - 8, 0)
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
    use crate::data::{ObjectContext, ObjectType, MM_OBJECTS, OOT_OBJECTS};

    /// Empty Master-Quest set: base layout everywhere.
    fn no_mq() -> HashSet<(Game, u16)> {
        HashSet::new()
    }

    /// The Kokiri Forest crawl grass changed representation across OoTMM versions: the per-era
    /// Child/Adult "Near Crawl 7 & 8" (legacy) were merged into the era-agnostic "Near Crawl 1 & 2"
    /// (new). Each set exists under only one xflag system, and legacy Child Near Crawl 7 shares its
    /// ObjectID with new Near Crawl 1 — the ObjSystem gate must keep the pair from ever both being
    /// active (mirror of the C++ HasCorrectLayout system branch).
    #[test]
    fn kokiri_crawl_system_gate() {
        let by_loc = |loc: &str| {
            OOT_OBJECTS
                .iter()
                .find(|o| o.location == loc)
                .unwrap_or_else(|| panic!("missing {loc}"))
        };

        // Any objects are active regardless of the ROM.
        assert!(system_active(ObjSystem::Any, true) && system_active(ObjSystem::Any, false));

        for loc in [
            "OOT Kokiri Forest Grass Child Near Crawl 7",
            "OOT Kokiri Forest Grass Child Near Crawl 8",
            "OOT Kokiri Forest Grass Adult Near Crawl 7",
            "OOT Kokiri Forest Grass Adult Near Crawl 8",
        ] {
            let o = by_loc(loc);
            assert!(o.system == ObjSystem::Legacy, "{loc} should be Legacy");
            // Legacy-only checks are absent from current OoTMM data -> no compact XflagID.
            assert_eq!(o.xflag_id, 0xFFFF, "{loc} should carry no XflagID");
            assert!(system_active(o.system, true), "{loc} shows on a legacy ROM");
            assert!(!system_active(o.system, false), "{loc} hidden on a new ROM");
        }

        for loc in [
            "OOT Kokiri Forest Grass Near Crawl 1",
            "OOT Kokiri Forest Grass Near Crawl 2",
        ] {
            let o = by_loc(loc);
            assert!(o.system == ObjSystem::New, "{loc} should be New");
            assert!(o.context == ObjectContext::All, "{loc} should be era-agnostic");
            assert!(system_active(o.system, false), "{loc} shows on a new ROM");
            assert!(!system_active(o.system, true), "{loc} hidden on a legacy ROM");
        }

        // The colliding pair (same scene + ObjectID) never both pass the gate, on either ROM.
        let child7 = by_loc("OOT Kokiri Forest Grass Child Near Crawl 7");
        let near1 = by_loc("OOT Kokiri Forest Grass Near Crawl 1");
        assert_eq!(child7.object_id, near1.object_id);
        assert_eq!(child7.scene, near1.scene);
        for legacy in [true, false] {
            assert!(
                system_active(child7.system, legacy) != system_active(near1.system, legacy),
                "exactly one of the legacy/new pair is active (legacy={legacy})"
            );
        }
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
                let (_, j) = resolve_collected(&ev, RomVersion::Dev, true, &no_mq())
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
        // Deliberately bogus scene byte (0xFF); ov = gs; the DLL sends the raw GS
        // flag (object_id - 8), which resolve_raw_gs shifts back.
        let ev = Event {
            pc: 0x8009_0000,
            mem: 0,
            query: [0xFF | ((o.type_ as u32) << 8), (o.object_id - 8) << 16, 0, 0, 0, 0],
        };
        let (_, j) = resolve_collected(&ev, RomVersion::Dev, true, &no_mq()).expect("gs resolves globally");
        assert_eq!(OOT_OBJECTS[j].object_id, o.object_id);
        assert_eq!(OOT_OBJECTS[j].type_ as u8, ObjectType::gs as u8);
    }

    /// Regression (reported seed): the DLL reports a GS by its raw flag `F`, but the
    /// pool keys each GS on `F + 8`. Without the resolve_raw_gs shift, flag `F` marks
    /// the GS whose object_id is `F` — 8 slots too early. Concretely, collecting
    /// Zora River GS Tree (flag 0x89) must mark *it*, not Kakariko GS Bazaar (object_id
    /// 0x89). Drives the collected-item path through `resolve_collected`.
    #[test]
    fn gs_flag_resolves_to_own_location_not_eight_earlier() {
        let tree = OOT_OBJECTS
            .iter()
            .position(|o| o.location == "OOT Zora River GS Tree")
            .expect("Zora River GS Tree in the pool");
        let flag = OOT_OBJECTS[tree].object_id - 8; // the raw flag the game/DLL reports (0x89)

        // Collected-item query: scene in q0 low byte, ov=gs in q0 byte 1, id in q1 byte 2.
        let ev = Event {
            pc: 0x8009_0000,
            mem: 0,
            query: [
                (OOT_OBJECTS[tree].scene as u32 & 0xFF) | ((OV_GS as u32) << 8),
                flag << 16,
                0, 0, 0, 0,
            ],
        };
        let (game, j) = resolve_collected(&ev, RomVersion::Dev, false, &no_mq()).expect("gs resolves");
        assert_eq!(game, Game::Oot);
        assert_eq!(OOT_OBJECTS[j].location, "OOT Zora River GS Tree", "raw flag must map to its own GS");

        // And it must NOT be the GS stamped with object_id == flag (the pre-fix bug).
        let bazaar = OOT_OBJECTS.iter().find(|o| o.location == "OOT Kakariko GS Bazaar").unwrap();
        assert_eq!(bazaar.object_id, flag, "precondition: Bazaar's object_id equals Zora Tree's flag");
        assert_ne!(OOT_OBJECTS[j].location, "OOT Kakariko GS Bazaar");
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
        let (_, j) = resolve_collected(&ev, RomVersion::Dev, true, &no_mq()).expect("nothing-drop grass resolves");
        assert_eq!(OOT_OBJECTS[j].object_id, o.object_id);
        assert_eq!(OOT_OBJECTS[j].type_ as u8, ObjectType::grass as u8);
    }

    /// Compact-XflagID resolution (new ROMs > v32.3): an event carrying only a
    /// 16-bit XflagID (no scene) resolves to the object stamped with it, by Location.
    #[test]
    fn xflag_by_location_resolves() {
        let o = OOT_OBJECTS
            .iter()
            .find(|o| o.xflag_id != 0xFFFF && object_active(o, Game::Oot, &no_mq()))
            .expect("a base-layout object carrying an XflagID");
        let xf = o.xflag_id as u32;
        // Compact query: ov = OV_XFLAG (0x10), scene byte = 0, id = xf low, room = xf high.
        let q0 = (OV_XFLAG0 as u32) << 8; // scene = 0
        let q1 = ((xf & 0xFF) << 16) | ((xf >> 8) << 24);
        let ev = Event { pc: 0x8009_0000, mem: 0, query: [q0, q1, 0, 0, 0, 0] };
        // New system (uses_legacy = false): resolves by XflagID.
        let (_, j) = resolve_collected(&ev, RomVersion::Dev, false, &no_mq())
            .expect("compact XflagID resolves");
        assert_eq!(OOT_OBJECTS[j].xflag_id, o.xflag_id);
        // Legacy system (uses_legacy = true) would instead scan scene 0 by the composite
        // id and miss (scene 0 carries no object with that id) — the paths are distinct.
        assert!(resolve_collected(&ev, RomVersion::Dev, true, &no_mq()).is_none());
    }

    /// The stable-ROM item-id shift is the identity on dev and adds the boundary
    /// shift on stable, and a resolved id maps to a real item name.
    #[test]
    fn net_item_name_resolves() {
        // Dev: identity. Item id 4 is the Fairy Bow (OoT).
        assert_eq!(resolve_raw_item_id(4, RomVersion::Dev), 4);
        assert_eq!(net_item_name(4, RomVersion::Dev), Some("Fairy Bow (OoT)"));
        // dev-33 reordered the OoT stick/nut upgrades: on a stable ROM the base
        // upgrade (raw 0x77 / 0x79) now resolves up to 0x80 / 0x81, UPGRADE2 down.
        assert_eq!(resolve_raw_item_id(0x77, RomVersion::Stable), 0x80);
        assert_eq!(resolve_raw_item_id(0x78, RomVersion::Stable), 0x77);
        assert_eq!(resolve_raw_item_id(0x77, RomVersion::Stable301), 0x80);
        // v30.1: an id past a boundary is shifted up by the (recomputed) amount.
        assert_eq!(resolve_raw_item_id(142, RomVersion::Stable301), 151);
        assert_eq!(resolve_raw_item_id(141, RomVersion::Stable301), 143);
        assert_eq!(resolve_raw_item_id(826, RomVersion::Stable301), 940);
        // Latest stable (v31 / v32.X): the two OoT UPGRADE3 inserts now shift even
        // the low ids by +2; each higher boundary climbs +4 / +5 (MM_MASK_ADULT) /
        // +6 / +7 / +8 / +9.
        assert_eq!(resolve_raw_item_id(0x76, RomVersion::Stable), 0x76); // below reorder
        assert_eq!(resolve_raw_item_id(142, RomVersion::Stable), 144);
        assert_eq!(resolve_raw_item_id(509, RomVersion::Stable), 513);
        assert_eq!(resolve_raw_item_id(933, RomVersion::Stable), 941);
        assert_eq!(resolve_raw_item_id(1024, RomVersion::Stable), 1033);
        // A zero / unknown id (e.g. a "nothing" drop) has no name.
        assert_eq!(net_item_name(0, RomVersion::Dev), None);
    }

    /// Entrance messages, and empty (ov=none) events, are ignored.
    #[test]
    fn non_item_events_ignored() {
        let entrance = Event { pc: 0x8009_0000, mem: 0xF000_1234, query: [0; 6] };
        assert!(resolve_collected(&entrance, RomVersion::Dev, true, &no_mq()).is_none());
        let empty = Event { pc: 0x8009_0000, mem: 0, query: [0, 0, 0xFFFF_0000, 0, 0, 0] };
        assert!(resolve_collected(&empty, RomVersion::Dev, true, &no_mq()).is_none());
    }
}
