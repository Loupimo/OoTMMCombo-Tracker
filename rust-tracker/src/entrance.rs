//! Live entrance-message decoding + tracking (EntranceHelper port).
//!
//! The DLL emits an IN / OUT message on each scene transition. `decode` is the
//! full port of `EntranceMessage::SetMessage` — every field the state machine
//! reads. The connection tracking (OUT paired with the following IN) lives in
//! main; the special-case resolvers (grottos by position, songs, spawns, deaths)
//! are ported on top of this decode.

use crate::data::{entr as e, scenes as s, EntranceDef, EntranceType};
use crate::scene::Game;
use crate::shared_mem::Event;

const IN_MAGIC: u32 = 0xFA00_0000;
const OUT_MAGIC: u32 = 0xFB00_0000;
const MM_GAME: u32 = 0x01;

/// Fully decoded entrance message (mirror of EntranceMessage::SetMessage).
#[derive(Clone)]
#[allow(dead_code)] // fields are consumed as the special-case handlers are ported
pub struct EntranceMsg {
    pub game: Game,
    /// true = arriving (IN), false = leaving (OUT).
    pub incoming: bool,
    /// Raw entrance id (matches an entry's `to_id`, the LookupEntrance key).
    pub entrance_id: u32,
    /// Final scene id (mirror of `EntranceMessage::SceneID`, a uint32_t so it can
    /// hold the WARP_SCENE sentinel the special-case handlers may write).
    pub scene: u32,
    /// gCurrScene / gLastScene bytes (Buffer[1]).
    pub curr_scene: u8,
    pub last_scene: u8,
    /// Current room, grotto data, song id (Buffer[0]).
    pub curr_room: u8,
    pub grotto_data: u8,
    pub song: u8,
    /// Link age / farore's wind / owl id (from the message flag word).
    pub age: u8,
    pub farore_wind: u8,
    pub owl_id: u8,
    /// Respawn coordinates (Buffer[3..5]) — used for grotto position matching.
    pub x: f32,
    pub y: f32,
    pub z: f32,
    /// Raw six data words, for the verbatim Buffer[N] comparisons (songs).
    pub raw: [u32; 6],
}

// Song ids as they appear in the message byte (game-specific numbering).
const OOT_SONG_SUN: u8 = 0x09;
const OOT_SONG_TIME: u8 = 0x0a;
const MM_SONG_SONATA: u8 = 0x00;
const MM_SONG_DOUBLE_TIME: u8 = 0x0d;

// LinkAge::Adult == 0 (Headers/Combo/Entrances.h). The message age byte holds the
// LinkAge value (values above 1 are the death-reload marker, see `is_death`).
const LINK_AGE_ADULT: u8 = 0;

/// Whether a shared-memory event is an entrance message (IN or OUT).
pub fn is_entrance(mem: u32) -> bool {
    matches!(mem & 0xFF00_0000, IN_MAGIC | OUT_MAGIC)
}

/// Full port of EntranceMessage::SetMessage. `mem` is the flag word, `query` the
/// six data words.
pub fn decode(ev: &Event) -> EntranceMsg {
    let flag = ev.mem;
    let b = &ev.query;
    let game = if b[0] & 0xFF == MM_GAME { Game::Mm } else { Game::Oot };
    EntranceMsg {
        game,
        incoming: (flag & 0xFF00_0000) == IN_MAGIC,
        entrance_id: b[2],
        scene: b[1] & 0xFFFF,
        curr_scene: ((b[1] >> 16) & 0xFF) as u8,
        last_scene: ((b[1] >> 24) & 0xFF) as u8,
        curr_room: ((b[0] >> 16) & 0xFF) as u8,
        grotto_data: ((b[0] >> 8) & 0xFF) as u8,
        song: ((b[0] >> 24) & 0xFF) as u8,
        age: ((flag >> 16) & 0xFF) as u8,
        farore_wind: ((flag >> 8) & 0xFF) as u8,
        owl_id: (flag & 0xFF) as u8,
        x: f32::from_bits(b[3]),
        y: f32::from_bits(b[4]),
        z: f32::from_bits(b[5]),
        raw: *b,
    }
}

// ── Special-case predicates (verbatim port) ──────────────────────────────────

/// IsNewCycle: MM re-entering a scene 0 with the same entrance (cycle reset).
pub fn is_new_cycle(prev: &EntranceMsg, curr: &EntranceMsg) -> bool {
    prev.game == Game::Mm && curr.game == Game::Mm && curr.scene == 0
        && curr.entrance_id == prev.entrance_id
}

/// IsMMExtra: MM extra / cutscene scenes and the moon-crash / SoT clock town.
pub fn is_mm_extra(m: &EntranceMsg) -> bool {
    m.game == Game::Mm
        && (matches!(m.curr_scene as u16, s::MM_EXTRA | s::MM_CUTSCENE_MAP)
            || matches!(
                m.entrance_id,
                e::MM_CLOCK_TOWER_MOON_CRASH_ENTR | e::MM_CLOCK_TOWN_FROM_SONG_OF_TIME_ENTR
            ))
}

/// IsDeath: the previous message's age byte above 1 marks a death reload.
pub fn is_death(prev: &EntranceMsg) -> bool {
    prev.age > 1
}

/// IsFaroreWind: farore's wind warp.
pub fn is_farore_wind(m: &EntranceMsg) -> bool {
    m.farore_wind != 0
}

/// IsSonataWoodfall: the Sonata self-loop at Woodfall's entrance.
pub fn is_sonata_woodfall(prev: &EntranceMsg, curr: &EntranceMsg) -> bool {
    prev.game == Game::Mm
        && curr.game == Game::Mm
        && prev.song == MM_SONG_SONATA
        && prev.scene == s::MM_WOODFALL as u32
        && curr.scene == s::MM_WOODFALL as u32
        && prev.entrance_id == curr.entrance_id
}

/// IsSpawn: the OoT child / adult spawn points.
pub fn is_spawn(m: &EntranceMsg) -> bool {
    matches!(m.entrance_id, e::OOT_SPAWN_CHILD | e::OOT_SPAWN_ADULT)
}

/// IsSunSong: the OoT Sun's Song day/night self-loop (with market special cases).
pub fn is_sun_song(prev: &EntranceMsg, curr: &EntranceMsg) -> bool {
    if curr.game != Game::Oot {
        return false;
    }
    let sun = prev.song == OOT_SONG_SUN;
    // The decoded scene always fits 16 bits here, so matching on `as u16` keeps
    // the human-readable scene constants as patterns without truncation risk.
    match curr.scene as u16 {
        s::OOT_MARKET_ADULT | s::OOT_MARKET_CHILD_DAY | s::OOT_MARKET_CHILD_NIGHT => {
            sun && prev.scene == s::OOT_MARKET as u32 && curr.entrance_id == prev.raw[2]
        }
        s::OOT_BACK_ALLEY_DAY | s::OOT_BACK_ALLEY_NIGHT => {
            sun && prev.scene == s::OOT_BACK_ALLEY as u32 && curr.entrance_id == prev.raw[2]
        }
        s::OOT_MARKET_ENTRANCE_ADULT
        | s::OOT_MARKET_ENTRANCE_CHILD_DAY
        | s::OOT_MARKET_ENTRANCE_CHILD_NIGHT => {
            sun && prev.scene == s::OOT_MARKET_ENTRANCE as u32 && curr.entrance_id == prev.raw[2]
        }
        s::OOT_TEMPLE_OF_TIME_EXTERIOR_ADULT
        | s::OOT_TEMPLE_OF_TIME_EXTERIOR_CHILD_DAY
        | s::OOT_TEMPLE_OF_TIME_EXTERIOR_CHILD_NIGHT => {
            sun && prev.scene == s::OOT_TEMPLE_OF_TIME_ENTRYWAY as u32
                && curr.entrance_id == prev.raw[2]
        }
        _ => sun && curr.scene == prev.raw[1] && curr.entrance_id == prev.raw[2],
    }
}

/// IsSongOfTime: the OoT Song of Time self-loop (identical Buffer[1..5]).
pub fn is_song_of_time(prev: &EntranceMsg, curr: &EntranceMsg) -> bool {
    prev.song == OOT_SONG_TIME
        && prev.raw[1] == curr.raw[1]
        && prev.raw[2] == curr.raw[2]
        && prev.raw[3] == curr.raw[3]
        && prev.raw[4] == curr.raw[4]
        && prev.raw[5] == curr.raw[5]
}

/// IsSongOfDoubleTime: the MM Song of Double Time self-loop.
pub fn is_song_of_double_time(prev: &EntranceMsg, curr: &EntranceMsg) -> bool {
    prev.song == MM_SONG_DOUBLE_TIME
        && curr.scene == prev.raw[1]
        && curr.entrance_id == prev.raw[2]
}

/// Port of LookupEntrance: the entrance whose `to_id` (map key) matches.
pub fn lookup(game: Game, entrance_id: u32) -> Option<&'static EntranceDef> {
    game.entrances().iter().find(|entr| entr.to_id == entrance_id)
}

// ── Grotto helpers (faithful port of Entrances.cpp) ──────────────────────────

/// IsGrottoEntrance: the six generic grotto-entry ids (game-agnostic).
pub fn is_grotto_entrance(entrance_id: u32) -> bool {
    matches!(entrance_id, 0x03f | 0x36d | 0x5bc | 0x5a4 | 0x1440 | 0x14a0)
}

/// IsGrottoExit: the internal "grotto exit touched" ids.
pub fn is_grotto_exit(entrance_id: u32) -> bool {
    entrance_id == e::OOT_INTERNAL_EXIT_GROTTO_ENTR
        || entrance_id == e::MM_INTERNAL_EXIT_GROTTO_ENTR
}

/// IsWarpEntrance: any warp song / save / zone load.
pub fn is_warp_entrance(entrance_id: u32) -> bool {
    entrance_id == e::WARP_LOADING
}

/// IsGrottoEntranceOutMessage: OoT grotto entrances seen on the OUT side.
pub fn is_grotto_entrance_out(game: Game, entrance_id: u32) -> bool {
    game == Game::Oot
        && matches!(
            entrance_id,
            e::OOT_GROTTO_CASTLE_ENTR
                | e::OOT_GROTTO_GENERIC_DMC_ENTR
                | e::OOT_GROTTO_SCRUBS3_DMC_ENTR
                | e::OOT_GROTTO_TRAIL_COW_ENTR
                | e::OOT_GROTTO_GENERIC_DMT_ENTR
                | e::OOT_GROTTO_SCRUBS2_COLOSSUS_ENTR
                | e::OOT_GROTTO_SCRUBS3_GORON_CITY_ENTR
                | e::OOT_GROTTO_FIELD_COW_ENTR
                | e::OOT_GROTTO_FIELD_TREE_ENTR
                | e::OOT_GROTTO_GENERIC_HF_MARKET_ENTR
                | e::OOT_GROTTO_GENERIC_HF_OPEN_ENTR
                | e::OOT_GROTTO_SCRUB_HEART_PIECE_ENTR
                | e::OOT_GROTTO_GENERIC_HF_SOUTHEAST_ENTR
                | e::OOT_GROTTO_TEKTITE_ENTR
                | e::OOT_GROTTO_GENERIC_KAKARIKO_ENTR
                | e::OOT_GROTTO_REDEAD_ENTR
                | e::OOT_GROTTO_GENERIC_KOKIRI_FOREST_ENTR
                | e::OOT_GROTTO_SCRUBS3_LAKE_ENTR
                | e::OOT_GROTTO_SCRUBS3_RANCH_ENTR
                | e::OOT_GROTTO_GENERIC_LOST_WOODS_ENTR
                | e::OOT_GROTTO_SCRUB_UPGRADE_ENTR
                | e::OOT_GROTTO_DEKU_THEATER_ENTR
                | e::OOT_GROTTO_SCRUBS2_SFM_ENTR
                | e::OOT_GROTTO_WOLFOS_ENTR
                | e::OOT_GROTTO_OCTOROK_ENTR
                | e::OOT_GROTTO_SCRUBS2_VALLEY_ENTR
                | e::OOT_GROTTO_GENERIC_RIVER_ENTR
                | e::OOT_GROTTO_SCRUBS2_RIVER_ENTR
        )
}

/// GetGrottoEntrance: resolve a generic grotto-entry id to the specific grotto.
pub fn get_grotto_entrance(game: Game, entrance_id: u32, grotto_data: u8, last_scene: u16) -> u32 {
    let gd = (grotto_data & 0x1f) as u32;
    if game == Game::Oot {
        match entrance_id {
            e::OOT_GROTTO_TYPE_GENERIC_ENTR => match gd {
                0x0c => e::OOT_GROTTO_GENERIC_KOKIRI_FOREST_ENTR,
                0x14 => e::OOT_GROTTO_GENERIC_LOST_WOODS_ENTR,
                0x08 => e::OOT_GROTTO_GENERIC_KAKARIKO_ENTR,
                0x17 => e::OOT_GROTTO_GENERIC_DMT_ENTR,
                0x1a => e::OOT_GROTTO_GENERIC_DMC_ENTR,
                0x09 => e::OOT_GROTTO_GENERIC_RIVER_ENTR,
                0x02 => e::OOT_GROTTO_GENERIC_HF_SOUTHEAST_ENTR,
                0x03 => e::OOT_GROTTO_GENERIC_HF_OPEN_ENTR,
                0x00 => e::OOT_GROTTO_GENERIC_HF_MARKET_ENTR,
                _ => entrance_id,
            },
            e::OOT_GROTTO_TYPE_FAIRY_ENTR => match last_scene {
                s::OOT_SACRED_FOREST_MEADOW => e::OOT_GROTTO_FAIRY_SFM_ENTR,
                s::OOT_HYRULE_FIELD => e::OOT_GROTTO_FAIRY_HF_ENTR,
                s::OOT_ZORA_RIVER => e::OOT_GROTTO_FAIRY_RIVER_ENTR,
                s::OOT_ZORA_DOMAIN => e::OOT_GROTTO_FAIRY_DOMAIN_ENTR,
                s::OOT_GERUDO_FORTRESS => e::OOT_GROTTO_FAIRY_FORTRESS_ENTR,
                _ => entrance_id,
            },
            e::OOT_GROTTO_TYPE_SCRUB2_ENTR | e::OOT_GROTTO_TYPE_SCRUB3_ENTR => match last_scene {
                s::OOT_SACRED_FOREST_MEADOW => e::OOT_GROTTO_SCRUBS2_SFM_ENTR,
                s::OOT_ZORA_RIVER => e::OOT_GROTTO_SCRUBS2_RIVER_ENTR,
                s::OOT_GERUDO_VALLEY => e::OOT_GROTTO_SCRUBS2_VALLEY_ENTR,
                s::OOT_DESERT_COLOSSUS => e::OOT_GROTTO_SCRUBS2_COLOSSUS_ENTR,
                s::OOT_LON_LON_RANCH => e::OOT_GROTTO_SCRUBS3_RANCH_ENTR,
                s::OOT_GORON_CITY => e::OOT_GROTTO_SCRUBS3_GORON_CITY_ENTR,
                s::OOT_DEATH_MOUNTAIN_CRATER => e::OOT_GROTTO_SCRUBS3_DMC_ENTR,
                s::OOT_LAKE_HYLIA => e::OOT_GROTTO_SCRUBS3_LAKE_ENTR,
                _ => entrance_id,
            },
            _ => entrance_id,
        }
    } else {
        // MM: normalise the entrance key first, then resolve.
        let key = match entrance_id >> 9 {
            0x06 => 0x42,
            0x57 => 0x4d,
            0x45 => 0x4a,
            0x5b => 0x5a,
            k => k,
        };
        let id = (key << 9) | (entrance_id & 0x1ff);
        match id {
            e::MM_GROTTO_TYPE_GENERIC_ENTR => match gd {
                0x13 => e::MM_GROTTO_GENERIC_PATH_SNOWHEAD_ENTR,
                0x14 => e::MM_GROTTO_GENERIC_VALLEY_ENTR,
                0x15 => e::MM_GROTTO_GENERIC_ZORA_CAPE_ENTR,
                0x16 => e::MM_GROTTO_GENERIC_PATH_IKANA_ENTR,
                0x17 => e::MM_GROTTO_GENERIC_GREAT_BAY_COAST_ENTR,
                0x18 => e::MM_GROTTO_GENERIC_GRAVEYARD_ENTR,
                0x19 => e::MM_GROTTO_GENERIC_TWIN_ISLANDS_ENTR,
                0x1a => e::MM_GROTTO_GENERIC_FIELD_PILLAR_ENTR,
                0x1b => e::MM_GROTTO_GENERIC_MOUNTAIN_VILLAGE_ENTR,
                0x1c => e::MM_GROTTO_GENERIC_WOODS_ENTR,
                0x1d => e::MM_GROTTO_GENERIC_SWAMP_ENTR,
                0x1e => e::MM_GROTTO_GENERIC_PATH_SWAMP_ENTR,
                0x1f => e::MM_GROTTO_GENERIC_GRASS_ENTR,
                _ => id,
            },
            e::MM_GROTTO_TYPE_COW_ENTR => match last_scene {
                s::MM_TERMINA_FIELD => e::MM_GROTTO_COW_FIELD_ENTR,
                s::MM_GREAT_BAY_COAST => e::MM_GROTTO_COW_COAST_ENTR,
                _ => id,
            },
            _ => id,
        }
    }
}

/// The grotto spawn-point table for a canonical entrance (GrottoEntrances.at).
fn grotto_positions(key: u32) -> Option<&'static [crate::data::GrottoPos]> {
    crate::data::GROTTO_ENTRANCES
        .iter()
        .find(|&&(k, _)| k == key)
        .map(|&(_, v)| v)
}

/// CheckGrottoSpawn: two stages. First canonicalise the entrance to its group
/// representative (generated table; an unmatched id is returned unchanged, the
/// C++ switch `default` early-out). Then, for a matched group, pick the grotto
/// entrance whose spawn point is closest to the respawn coordinates.
pub fn check_grotto_spawn(game: Game, entrance_id: u32, x: f32, y: f32, z: f32) -> u32 {
    let table = match game {
        Game::Oot => crate::data::CHECK_GROTTO_SPAWN_OOT,
        Game::Mm => crate::data::CHECK_GROTTO_SPAWN_MM,
    };
    // Stage 1: canonicalise (default -> unchanged, skipping the position stage).
    let canonical = match table.binary_search_by_key(&entrance_id, |&(from, _)| from) {
        Ok(i) => table[i].1,
        Err(_) => return entrance_id,
    };

    // Stage 2: closest spawn point (GrottoEntrances.at + GetDistanceGrottoEntrance).
    let Some(positions) = grotto_positions(canonical) else {
        return canonical;
    };
    if positions.is_empty() {
        return canonical;
    }
    let mut best_id = positions[0].id;
    let mut best_dist = grotto_distance(positions[0].pos, x, y, z);
    if best_dist == 0.0 {
        return best_id;
    }
    for p in &positions[1..] {
        let d = grotto_distance(p.pos, x, y, z);
        if d == 0.0 {
            return p.id;
        } else if d < best_dist {
            best_dist = d;
            best_id = p.id;
        }
    }
    best_id
}

/// CorrectGrottoScene: map a resolved grotto entrance/exit id to its detailed
/// grotto scene (used to render the object overlays on the right sub-scene).
pub fn correct_grotto_scene(game: Game, entrance_id: u32) -> u32 {
    if game == Game::Oot {
        match entrance_id {
            e::OOT_GROTTO_EXIT_GENERIC_KOKIRI_FOREST | e::OOT_GROTTO_GENERIC_KOKIRI_FOREST_ENTR => s::OOT_GROTTO_KOKIRI_FOREST_STORMS as u32,
            e::OOT_GROTTO_EXIT_GENERIC_LOST_WOODS | e::OOT_GROTTO_GENERIC_LOST_WOODS_ENTR => s::OOT_GROTTO_LOST_WOODS_GENERIC as u32,
            e::OOT_GROTTO_EXIT_GENERIC_KAKARIKO | e::OOT_GROTTO_GENERIC_KAKARIKO_ENTR => s::OOT_GROTTO_KAKARIKO_OPEN as u32,
            e::OOT_GROTTO_EXIT_GENERIC_DMT | e::OOT_GROTTO_GENERIC_DMT_ENTR => s::OOT_GROTTO_DEATH_TRIAL_STORMS as u32,
            e::OOT_GROTTO_EXIT_GENERIC_DMC | e::OOT_GROTTO_GENERIC_DMC_ENTR => s::OOT_GROTTO_DEATH_CRATER_GENERIC as u32,
            e::OOT_GROTTO_EXIT_GENERIC_RIVER | e::OOT_GROTTO_GENERIC_RIVER_ENTR => s::OOT_GROTTO_ZORA_RIVER_GENERIC as u32,
            e::OOT_GROTTO_EXIT_GENERIC_HF_SOUTHEAST | e::OOT_GROTTO_GENERIC_HF_SOUTHEAST_ENTR => s::OOT_GROTTO_HYRULE_SE as u32,
            e::OOT_GROTTO_EXIT_GENERIC_HF_OPEN | e::OOT_GROTTO_GENERIC_HF_OPEN_ENTR => s::OOT_GROTTO_HYRULE_OPEN as u32,
            e::OOT_GROTTO_EXIT_GENERIC_HF_MARKET | e::OOT_GROTTO_GENERIC_HF_MARKET_ENTR => s::OOT_GROTTO_HYRULE_MARKET as u32,
            e::OOT_GROTTO_EXIT_FAIRY_SFM | e::OOT_GROTTO_FAIRY_SFM_ENTR => s::OOT_FAIRY_SACRED_MEADOW as u32,
            e::OOT_GROTTO_EXIT_FAIRY_HF | e::OOT_GROTTO_FAIRY_HF_ENTR => s::OOT_FAIRY_HYRULE as u32,
            e::OOT_GROTTO_EXIT_FAIRY_RIVER | e::OOT_GROTTO_FAIRY_RIVER_ENTR => s::OOT_FAIRY_ZORA_RIVER as u32,
            e::OOT_GROTTO_EXIT_FAIRY_DOMAIN | e::OOT_GROTTO_FAIRY_DOMAIN_ENTR => s::OOT_FAIRY_ZORA_DOMAIN as u32,
            e::OOT_GROTTO_EXIT_FAIRY_FORTRESS | e::OOT_GROTTO_FAIRY_FORTRESS_ENTR => s::OOT_FAIRY_GERUDO_FORTRESS as u32,
            e::OOT_GROTTO_EXIT_SCRUBS2_SFM | e::OOT_GROTTO_SCRUBS2_SFM_ENTR => s::OOT_GROTTO_SACRED_MEADOW_STORMS as u32,
            e::OOT_GROTTO_EXIT_SCRUBS2_RIVER | e::OOT_GROTTO_SCRUBS2_RIVER_ENTR => s::OOT_GROTTO_ZORA_RIVER_STORMS as u32,
            e::OOT_GROTTO_EXIT_SCRUBS2_VALLEY | e::OOT_GROTTO_SCRUBS2_VALLEY_ENTR => s::OOT_GROTTO_VALLEY_STORMS as u32,
            e::OOT_GROTTO_EXIT_SCRUBS2_COLOSSUS | e::OOT_GROTTO_SCRUBS2_COLOSSUS_ENTR => s::OOT_GROTTO_DESERT_SCRUBS as u32,
            e::OOT_GROTTO_EXIT_SCRUBS3_RANCH | e::OOT_GROTTO_SCRUBS3_RANCH_ENTR => s::OOT_GROTTO_LON_LON_SCRUBS as u32,
            e::OOT_GROTTO_EXIT_SCRUBS3_GORON_CITY | e::OOT_GROTTO_SCRUBS3_GORON_CITY_ENTR => s::OOT_GROTTO_GORON_CITY_SCRUBS as u32,
            e::OOT_GROTTO_EXIT_SCRUBS3_DMC | e::OOT_GROTTO_SCRUBS3_DMC_ENTR => s::OOT_GROTTO_DEATH_CRATER_SCRUBS as u32,
            e::OOT_GROTTO_EXIT_SCRUBS3_LAKE | e::OOT_GROTTO_SCRUBS3_LAKE_ENTR => s::OOT_GROTTO_LAKE_HYLIA_SCRUBS as u32,
            e::OOT_GROTTO_EXIT_SCRUB_UPGRADE | e::OOT_GROTTO_SCRUB_UPGRADE_ENTR => s::OOT_GROTTO_LOST_WOODS_SCRUB_UPGRADE as u32,
            e::OOT_GROTTO_EXIT_SCRUB_HEART_PIECE | e::OOT_GROTTO_SCRUB_HEART_PIECE_ENTR => s::OOT_GROTTO_HYRULE_SCRUBS as u32,
            e::OOT_GROTTO_EXIT_DEKU_THEATER | e::OOT_GROTTO_DEKU_THEATER_ENTR => s::OOT_GROTTO_LOST_WOODS_THEATER as u32,
            e::OOT_GROTTO_EXIT_WOLFOS | e::OOT_GROTTO_WOLFOS_ENTR => s::OOT_GROTTO_SACRED_MEADOW_WOLFOS as u32,
            e::OOT_GROTTO_EXIT_TEKTITE | e::OOT_GROTTO_TEKTITE_ENTR => s::OOT_GROTTO_HYRULE_TEKTITE as u32,
            e::OOT_GROTTO_EXIT_FIELD_COW | e::OOT_GROTTO_FIELD_COW_ENTR => s::OOT_GROTTO_HYRULE_GERUDO as u32,
            e::OOT_GROTTO_EXIT_FIELD_TREE | e::OOT_GROTTO_FIELD_TREE_ENTR => s::OOT_GROTTO_HYRULE_KAKARIKO as u32,
            e::OOT_GROTTO_EXIT_TRAIL_COW | e::OOT_GROTTO_TRAIL_COW_ENTR => s::OOT_GROTTO_DEATH_TRIAL_COW as u32,
            e::OOT_GROTTO_EXIT_CASTLE | e::OOT_GROTTO_CASTLE_ENTR => s::OOT_GROTTO_CASTLE_STORMS as u32,
            e::OOT_GROTTO_EXIT_REDEAD | e::OOT_GROTTO_REDEAD_ENTR => s::OOT_GROTTO_KAKARIKO_REDEAD as u32,
            e::OOT_GROTTO_EXIT_OCTOROK | e::OOT_GROTTO_OCTOROK_ENTR => s::OOT_GROTTO_VALLEY_OCTOROK as u32,
            _ => entrance_id,
        }
    } else {
        match entrance_id {
            e::MM_GROTTO_GENERIC_FIELD_PILLAR_ENTR | e::MM_GROTTO_EXIT_GENERIC_FIELD_PILLAR => s::MM_GROTTO_TERMINA_PILLAR as u32,
            e::MM_GROTTO_GENERIC_GRASS_ENTR | e::MM_GROTTO_EXIT_GENERIC_GRASS => s::MM_GROTTO_TERMINA_TALL_GRASS as u32,
            e::MM_GROTTO_GENERIC_PATH_SWAMP_ENTR | e::MM_GROTTO_EXIT_GENERIC_PATH_SWAMP => s::MM_GROTTO_SOUTHERN_SWAMP_ROAD_OPEN as u32,
            e::MM_GROTTO_GENERIC_WOODS_ENTR | e::MM_GROTTO_EXIT_GENERIC_WOODS => s::MM_GROTTO_WOODS_OF_MYSTERY_OPEN as u32,
            e::MM_GROTTO_GENERIC_SWAMP_ENTR | e::MM_GROTTO_EXIT_GENERIC_SWAMP => s::MM_GROTTO_SOUTHERN_SWAMP_OPEN as u32,
            e::MM_GROTTO_GENERIC_MOUNTAIN_VILLAGE_ENTR | e::MM_GROTTO_EXIT_GENERIC_MOUNTAIN_VILLAGE => s::MM_GROTTO_MOUNTAIN_VILLAGE_GENERIC as u32,
            e::MM_GROTTO_GENERIC_TWIN_ISLANDS_ENTR | e::MM_GROTTO_EXIT_GENERIC_TWIN_ISLANDS => s::MM_GROTTO_TWIN_ISLANDS_RAMP as u32,
            e::MM_GROTTO_GENERIC_PATH_SNOWHEAD_ENTR | e::MM_GROTTO_EXIT_GENERIC_PATH_SNOWHEAD => s::MM_GROTTO_PATH_TO_SNOWHEAD_GENERIC as u32,
            e::MM_GROTTO_GENERIC_GREAT_BAY_COAST_ENTR | e::MM_GROTTO_EXIT_GENERIC_GREAT_BAY_COAST => s::MM_GROTTO_GREAT_BAY_COAST_FISHERMAN as u32,
            e::MM_GROTTO_GENERIC_ZORA_CAPE_ENTR | e::MM_GROTTO_EXIT_GENERIC_ZORA_CAPE => s::MM_GROTTO_ZORA_CAPE_GENERIC as u32,
            e::MM_GROTTO_GENERIC_PATH_IKANA_ENTR | e::MM_GROTTO_EXIT_GENERIC_PATH_IKANA => s::MM_GROTTO_IKANA_ROAD_GENERIC as u32,
            e::MM_GROTTO_GENERIC_GRAVEYARD_ENTR | e::MM_GROTTO_EXIT_GENERIC_GRAVEYARD => s::MM_GROTTO_IKANA_GRAVEYARD_GENERIC as u32,
            e::MM_GROTTO_GENERIC_VALLEY_ENTR | e::MM_GROTTO_EXIT_GENERIC_VALLEY => s::MM_GROTTO_IKANA_VALLEY_OPEN as u32,
            e::MM_GROTTO_COW_FIELD_ENTR | e::MM_GROTTO_EXIT_COW_FIELD => s::MM_GROTTO_TERMINA_COW as u32,
            e::MM_GROTTO_COW_COAST_ENTR | e::MM_GROTTO_EXIT_COW_COAST => s::MM_GROTTO_GREAT_BAY_COAST_COW as u32,
            e::MM_GROTTO_GOSSIPS_OCEAN_ENTR | e::MM_GROTTO_EXIT_GOSSIPS_OCEAN => s::MM_GROTTO_TERMINA_OCEAN_GOSSIP as u32,
            e::MM_GROTTO_GOSSIPS_SWAMP_ENTR | e::MM_GROTTO_EXIT_GOSSIPS_SWAMP => s::MM_GROTTO_TERMINA_SWAMP_GOSSIP as u32,
            e::MM_GROTTO_GOSSIPS_CANYON_ENTR | e::MM_GROTTO_EXIT_GOSSIPS_CANYON => s::MM_GROTTO_TERMINA_CANYON_GOSSIP as u32,
            e::MM_GROTTO_GOSSIPS_MOUNTAIN_ENTR | e::MM_GROTTO_EXIT_GOSSIPS_MOUNTAIN => s::MM_GROTTO_TERMINA_MOUNTAIN_GOSSIP as u32,
            e::MM_GROTTO_HOT_WATER_ENTR | e::MM_GROTTO_EXIT_HOT_WATER => s::MM_GROTTO_TWIN_ISLANDS_FROZEN as u32,
            e::MM_GROTTO_JP_LINE_END_ENTR | e::MM_GROTTO_EXIT_JP_LINE_END => s::MM_GROTTO_DEKU_PALACE_GENERIC as u32,
            e::MM_GROTTO_DODONGO_ENTR | e::MM_GROTTO_EXIT_DODONGO => s::MM_GROTTO_TERMINA_DODONGO as u32,
            e::MM_GROTTO_JP_CLIMB_RIGHT_ENTR | e::MM_GROTTO_EXIT_JP_CLIMB_RIGHT => s::MM_GROTTO_DEKU_PALACE_CLIMB as u32,
            e::MM_GROTTO_SCRUB_ENTR | e::MM_GROTTO_EXIT_SCRUB => s::MM_GROTTO_TERMINA_SCRUB as u32,
            e::MM_GROTTO_BIO_BABA_ENTR | e::MM_GROTTO_EXIT_BIO_BABA => s::MM_GROTTO_TERMINA_BIO_BABA as u32,
            e::MM_GROTTO_BEAN_ENTR | e::MM_GROTTO_EXIT_BEAN => s::MM_GROTTO_DEKU_PALACE_BEANS as u32,
            e::MM_GROTTO_PEAHAT_ENTR | e::MM_GROTTO_EXIT_PEAHAT => s::MM_GROTTO_TERMINA_PEAHAT as u32,
            e::MM_GROTTO_JP_LINE_START_ENTR | e::MM_GROTTO_EXIT_JP_LINE_START => s::MM_GROTTO_DEKU_PALACE_GENERIC as u32,
            e::MM_GROTTO_JP_CLIMB_LEFT_ENTR | e::MM_GROTTO_EXIT_JP_CLIMB_LEFT => s::MM_GROTTO_DEKU_PALACE_CLIMB as u32,
            _ => entrance_id,
        }
    }
}

/// GetWarpSong: resolve a warp-song / owl-warp to its target entrance.
/// Returns (target game, entrance id, is_warp_song) — mirrors the C++ out-params
/// (the C++ mutates Message.GameID; here it is the returned game).
pub fn get_warp_song(game: Game, scene: u16, song: u8, owl_id: u8, entrance_id: u32) -> (Game, u32, bool) {
    use crate::data::{owl as o, song_mm as sm, song_oot as so};

    // It is not possible to pull out the ocarina in an OoT boss lair scene.
    if game == Game::Oot && (s::OOT_LAIR_GOHMA..=s::OOT_LAIR_GANONDORF).contains(&scene) {
        return (Game::Oot, entrance_id, false);
    }

    // The MM owl-warp table, shared by both games' Song of Soaring branch.
    let owl_warp = |owl_id: u8| -> Option<u32> {
        Some(match owl_id {
            o::Great_Bay_Coast => e::MM_WARP_OWL_GREAT_BAY_ENTR,
            o::Zora_Cape => e::MM_WARP_OWL_ZORA_CAPE_ENTR,
            o::Snowhead => e::MM_WARP_OWL_SNOWHEAD_ENTR,
            o::Mountain_Village => e::MM_WARP_OWL_MOUNTAIN_VILLAGE_ENTR,
            o::Clock_Town => e::MM_WARP_OWL_CLOCK_TOWN_ENTR,
            o::Milk_Road => e::MM_WARP_OWL_MILK_ROAD_ENTR,
            o::Woodfall => e::MM_WARP_OWL_WOODFALL_ENTR,
            o::Southern_Swamp => e::MM_WARP_OWL_SOUTHERN_SWAMP_ENTR,
            o::Ikana_Canyon => e::MM_WARP_OWL_IKANA_CANYON_ENTR,
            o::Stone_Tower => e::MM_WARP_OWL_STONE_TOWER_ENTR,
            _ => return None,
        })
    };

    if game == Game::Oot {
        match song {
            so::Minuet_of_Forest => (game, e::OOT_MINUET_OF_FOREST_SONG, true),
            so::Bolero_of_Fire => (game, e::OOT_BOLERO_OF_FIRE_SONG, true),
            so::Serenade_of_Water => (game, e::OOT_SERENADE_OF_WATER_SONG, true),
            so::Requiem_of_Spirit => (game, e::OOT_REQUIEM_OF_SPIRIT_SONG, true),
            so::Nocturne_of_Shadow => (game, e::OOT_NOCTURNE_OF_SHADOW_SONG, true),
            so::Prelude_of_Light => (game, e::OOT_PRELUDE_OF_LIGHT_SONG, true),
            so::Song_of_Soaring_30_1 | so::Song_of_Soaring => match owl_warp(owl_id) {
                Some(id) => (Game::Mm, id, true),
                None => (Game::Oot, entrance_id, false),
            },
            _ => (game, entrance_id, false),
        }
    } else {
        // MM: default target is OoT for the warp songs.
        match song {
            sm::Minuet_of_Forest_30_1 | sm::Minuet_of_Forest => (Game::Oot, e::OOT_MINUET_OF_FOREST_SONG, true),
            sm::Bolero_of_Fire_30_1 | sm::Bolero_of_Fire => (Game::Oot, e::OOT_BOLERO_OF_FIRE_SONG, true),
            sm::Serenade_of_Water_30_1 | sm::Serenade_of_Water => (Game::Oot, e::OOT_SERENADE_OF_WATER_SONG, true),
            sm::Requiem_of_Spirit_30_1 | sm::Requiem_of_Spirit => (Game::Oot, e::OOT_REQUIEM_OF_SPIRIT_SONG, true),
            sm::Nocturne_of_Shadow_30_1 | sm::Nocturne_of_Shadow => (Game::Oot, e::OOT_NOCTURNE_OF_SHADOW_SONG, true),
            sm::Prelude_of_Light_30_1 | sm::Prelude_of_Light => (Game::Oot, e::OOT_PRELUDE_OF_LIGHT_SONG, true),
            sm::Song_of_Soaring => match owl_warp(owl_id) {
                Some(id) => (Game::Mm, id, true),
                None => (Game::Mm, entrance_id, false),
            },
            _ => (Game::Mm, entrance_id, false),
        }
    }
}

/// CheckWrapScene: boss lairs / caught rooms warp out; everything else falls
/// back to the WARP_SCENE marker. Returns (new scene as u32, entrance id).
pub fn check_wrap_scene(game: Game, scene: u16, entrance_id: u32) -> (u32, u32) {
    if game == Game::Oot {
        match scene {
            s::OOT_LAIR_GOHMA => return (scene as u32, e::OOT_BOSS_DEKU_TREE_WARP_OUT),
            s::OOT_LAIR_KING_DODONGO => return (scene as u32, e::OOT_BOSS_DODONGO_CAVERN_WARP_OUT),
            s::OOT_LAIR_BARINADE => return (scene as u32, e::OOT_BOSS_JABU_JABU_WARP_OUT),
            s::OOT_LAIR_PHANTOM_GANON => return (scene as u32, e::OOT_BOSS_FOREST_TEMPLE_WARP_OUT),
            s::OOT_LAIR_VOLVAGIA => return (scene as u32, e::OOT_BOSS_FIRE_TEMPLE_WARP_OUT),
            s::OOT_LAIR_MORPHA => return (scene as u32, e::OOT_BOSS_WATER_TEMPLE_WARP_OUT),
            s::OOT_LAIR_BONGO_BONGO => return (scene as u32, e::OOT_BOSS_SHADOW_TEMPLE_WARP_OUT),
            s::OOT_LAIR_TWINROVA => return (scene as u32, e::OOT_BOSS_SPIRIT_TEMPLE_WARP_OUT),
            _ => {}
        }
    } else {
        match scene {
            s::MM_DEKU_PALACE | s::MM_DEKU_KING_CHAMBER => {
                return (s::MM_DEKU_PALACE as u32, e::MM_DEKU_PALACE_CAUGHT)
            }
            s::MM_PIRATE_FORTRESS_ENTRANCE => return (scene as u32, e::MM_PIRATE_ENTRANCE_CAUGHT),
            s::MM_PIRATE_FORTRESS_INTERIOR | s::MM_PIRATE_FORTRESS_EXTERIOR => {
                return (scene as u32, e::MM_PIRATE_ENTRANCE_FROM_EXTERIOR_CAUGHT)
            }
            s::MM_LAIR_ODOLWA => return (scene as u32, e::MM_BOSS_TEMPLE_WOODFALL_WARP_OUT),
            s::MM_LAIR_GOHT => return (scene as u32, e::MM_BOSS_TEMPLE_SNOWHEAD_WARP_OUT),
            s::MM_LAIR_GYORG => return (scene as u32, e::MM_BOSS_TEMPLE_GREAT_BAY_WARP_OUT),
            s::MM_LAIR_TWINMOLD => {
                return (scene as u32, e::MM_BOSS_TEMPLE_STONE_TOWER_INVERTED_WARP_OUT)
            }
            s::MM_CLOCK_TOWN_SOUTH => return (e::WARP_SCENE, entrance_id),
            _ => {}
        }
    }
    (e::WARP_SCENE, entrance_id)
}

/// GetDistanceGrottoEntrance: Manhattan distance to a grotto spawn point.
pub fn grotto_distance(pos: [f32; 3], x: f32, y: f32, z: f32) -> f32 {
    (x - pos[0]).abs() + (y - pos[1]).abs() + (z - pos[2]).abs()
}

/// GetGrottoExit: resolve the grotto you exited from (room / grotto-data / scene).
pub fn get_grotto_exit(game: Game, curr_scene: u16, curr_room: u8, grotto_data: u8, scene: u16) -> u32 {
    let gd = (grotto_data & 0x1f) as u32;
    if game == Game::Oot {
        match curr_scene {
            s::OOT_GROTTOS => match curr_room {
                0x00 => match gd {
                    0x0c => return e::OOT_GROTTO_EXIT_GENERIC_KOKIRI_FOREST,
                    0x14 => return e::OOT_GROTTO_EXIT_GENERIC_LOST_WOODS,
                    0x08 => return e::OOT_GROTTO_EXIT_GENERIC_KAKARIKO,
                    0x17 => return e::OOT_GROTTO_EXIT_GENERIC_DMT,
                    0x1a => return e::OOT_GROTTO_EXIT_GENERIC_DMC,
                    0x09 => return e::OOT_GROTTO_EXIT_GENERIC_RIVER,
                    0x02 => return e::OOT_GROTTO_EXIT_GENERIC_HF_SOUTHEAST,
                    0x03 => return e::OOT_GROTTO_EXIT_GENERIC_HF_OPEN,
                    0x00 => return e::OOT_GROTTO_EXIT_GENERIC_HF_MARKET,
                    _ => {}
                },
                0x01 => return e::OOT_GROTTO_EXIT_SCRUB_HEART_PIECE,
                0x02 => return e::OOT_GROTTO_EXIT_REDEAD,
                0x03 => return e::OOT_GROTTO_EXIT_TRAIL_COW,
                0x04 => return e::OOT_GROTTO_EXIT_FIELD_COW,
                0x05 => return e::OOT_GROTTO_EXIT_OCTOROK,
                0x06 => return e::OOT_GROTTO_EXIT_SCRUB_UPGRADE,
                0x07 => return e::OOT_GROTTO_EXIT_WOLFOS,
                0x08 => return e::OOT_GROTTO_EXIT_CASTLE,
                0x09 | 0x0c => match scene {
                    s::OOT_SACRED_FOREST_MEADOW => return e::OOT_GROTTO_EXIT_SCRUBS2_SFM,
                    s::OOT_ZORA_RIVER => return e::OOT_GROTTO_EXIT_SCRUBS2_RIVER,
                    s::OOT_GERUDO_VALLEY => return e::OOT_GROTTO_EXIT_SCRUBS2_VALLEY,
                    s::OOT_DESERT_COLOSSUS => return e::OOT_GROTTO_EXIT_SCRUBS2_COLOSSUS,
                    s::OOT_LON_LON_RANCH => return e::OOT_GROTTO_EXIT_SCRUBS3_RANCH,
                    s::OOT_GORON_CITY => return e::OOT_GROTTO_EXIT_SCRUBS3_GORON_CITY,
                    s::OOT_DEATH_MOUNTAIN_CRATER => return e::OOT_GROTTO_EXIT_SCRUBS3_DMC,
                    s::OOT_LAKE_HYLIA => return e::OOT_GROTTO_EXIT_SCRUBS3_LAKE,
                    _ => {}
                },
                0x0a => return e::OOT_GROTTO_EXIT_TEKTITE,
                0x0b => return e::OOT_GROTTO_EXIT_DEKU_THEATER,
                0x0d => return e::OOT_GROTTO_EXIT_FIELD_TREE,
                _ => {}
            },
            s::OOT_FAIRY_FOUNTAIN => match scene {
                s::OOT_SACRED_FOREST_MEADOW => return e::OOT_GROTTO_EXIT_FAIRY_SFM,
                s::OOT_HYRULE_FIELD => return e::OOT_GROTTO_EXIT_FAIRY_HF,
                s::OOT_ZORA_RIVER => return e::OOT_GROTTO_EXIT_FAIRY_RIVER,
                s::OOT_ZORA_DOMAIN => return e::OOT_GROTTO_EXIT_FAIRY_DOMAIN,
                s::OOT_GERUDO_FORTRESS => return e::OOT_GROTTO_EXIT_FAIRY_FORTRESS,
                _ => {}
            },
            s::OOT_TOMB_FAIRY => return e::OOT_GRAVE_EXIT_SHIELD,
            s::OOT_TOMB_REDEAD => return e::OOT_GRAVE_EXIT_REDEAD,
            s::OOT_TOMB_ROYAL => return e::OOT_GRAVE_EXIT_ROYAL,
            s::OOT_TOMB_DAMPE_WINDMILL => return e::OOT_GRAVE_EXIT_DAMPE,
            _ => {}
        }
    } else {
        match curr_scene {
            s::MM_GROTTOS => match curr_room {
                0x00 => return e::MM_GROTTO_EXIT_GOSSIPS_OCEAN,
                0x01 => return e::MM_GROTTO_EXIT_GOSSIPS_SWAMP,
                0x02 => return e::MM_GROTTO_EXIT_GOSSIPS_CANYON,
                0x03 => return e::MM_GROTTO_EXIT_GOSSIPS_MOUNTAIN,
                0x04 => match gd {
                    0x13 => return e::MM_GROTTO_EXIT_GENERIC_PATH_SNOWHEAD,
                    0x14 => return e::MM_GROTTO_EXIT_GENERIC_VALLEY,
                    0x15 => return e::MM_GROTTO_EXIT_GENERIC_ZORA_CAPE,
                    0x16 => return e::MM_GROTTO_EXIT_GENERIC_PATH_IKANA,
                    0x17 => return e::MM_GROTTO_EXIT_GENERIC_GREAT_BAY_COAST,
                    0x18 => return e::MM_GROTTO_EXIT_GENERIC_GRAVEYARD,
                    0x19 => return e::MM_GROTTO_EXIT_GENERIC_TWIN_ISLANDS,
                    0x1a => return e::MM_GROTTO_EXIT_GENERIC_FIELD_PILLAR,
                    0x1b => return e::MM_GROTTO_EXIT_GENERIC_MOUNTAIN_VILLAGE,
                    0x1c => return e::MM_GROTTO_EXIT_GENERIC_WOODS,
                    0x1d => return e::MM_GROTTO_EXIT_GENERIC_SWAMP,
                    0x1e => return e::MM_GROTTO_EXIT_GENERIC_PATH_SWAMP,
                    0x1f => return e::MM_GROTTO_EXIT_GENERIC_GRASS,
                    _ => {}
                },
                0x07 => return e::MM_GROTTO_EXIT_DODONGO,
                0x09 => return e::MM_GROTTO_EXIT_SCRUB,
                0x0a => match scene {
                    s::MM_TERMINA_FIELD => return e::MM_GROTTO_EXIT_COW_FIELD,
                    s::MM_GREAT_BAY_COAST => return e::MM_GROTTO_EXIT_COW_COAST,
                    _ => {}
                },
                0x0b => return e::MM_GROTTO_EXIT_BIO_BABA,
                0x0d => return e::MM_GROTTO_EXIT_PEAHAT,
                0x0e => return e::MM_GROTTO_EXIT_HOT_WATER,
                _ => {}
            },
            _ => {}
        }
    }
    0
}

/// CheckSpecialCase: the big per-scene fix-up switch (Entrances.cpp).
///
/// OoT switches on `SceneID`, MM on `CurrSceneID`. It rewrites `m.scene`
/// (and `m.curr_scene` for MM) to the canonical scene the tracker renders and
/// returns the corrected entrance id — the original `m.entrance_id` when no case
/// applies. These are the hand-authored exceptions the engine can't express
/// through the generic entrance map: season variants, day/night market twins,
/// caught cutscenes, grotto rooms, magic-fountain aliasing, etc.
pub fn check_special_case(m: &mut EntranceMsg) -> u32 {
    // Direction helpers mirror `Message.Direction == OUT_MAGIC / IN_MAGIC`.
    let out = !m.incoming;
    let is_adult = m.age == LINK_AGE_ADULT;

    if m.game == Game::Oot {
        // OoT switches on the final scene id.
        match m.scene as u16 {
            s::OOT_GERUDO_VALLEY => {
                if out && m.entrance_id == e::OOT_GERUDO_FORTRESS_CAUGHT_NO_HOOK_ENTR {
                    return e::OOT_GERUDO_VALLEY_CAUGHT_ENTR;
                }
            }

            s::OOT_GERUDO_FORTRESS => {
                if out && m.entrance_id == e::OOT_GERUDO_FORTRESS_CAUGHT_ENTR {
                    return e::OOT_GERUDO_FORTRESS_JAIL_ENTR;
                }
            }

            s::OOT_BAZAAR => match m.entrance_id {
                e::OOT_KAKARIKO_FROM_BAZAAR_ENTR | e::OOT_KAKARIKO_BAZAAR_ENTR => {
                    m.scene = s::OOT_KAKARIKO_BAZAAR as u32;
                }
                e::OOT_MARKET_FROM_BAZAAR_ENTR | e::OOT_MARKET_BAZAAR_ENTR => {
                    m.scene = s::OOT_MARKET_BAZAAR as u32;
                }
                _ => {}
            },

            s::OOT_SHOOTING_GALLERY => match m.entrance_id {
                e::OOT_KAKARIKO_FROM_ARCHERY_ENTR | e::OOT_ADULT_ARCHERY_ENTR => {
                    m.scene = s::OOT_KAKARIKO_SHOOTING as u32;
                }
                e::OOT_MARKET_FROM_ARCHERY_ENTR | e::OOT_CHILD_ARCHERY_ENTR => {
                    m.scene = s::OOT_MARKET_SHOOTING as u32;
                }
                _ => {}
            },

            s::OOT_GREAT_FAIRY_FOUNTAIN_UPGRADES | s::OOT_GREAT_FAIRY_FOUNTAIN_SPELLS => {
                match m.entrance_id {
                    // First magic upgrade
                    e::OOT_DEATH_MOUNTAIN_FROM_FAIRY_ENTR | e::OOT_FAIRY_MAGIC_ENTR => {
                        m.scene = s::OOT_GREAT_FAIRY_MAGIC as u32;
                    }
                    // Second magic upgrade
                    e::OOT_DEATH_CRATER_FROM_FAIRY_ENTR | e::OOT_FAIRY_MAGIC2_ENTR => {
                        m.scene = s::OOT_GREAT_FAIRY_MAGIC2 as u32;
                    }
                    // Double defense
                    e::OOT_FAIRY_DEFENSE_ENTR => {
                        m.scene = s::OOT_GREAT_FAIRY_DEFENSE as u32;
                    }
                    // Din's fire
                    e::OOT_FAIRY_DIN_ENTR => {
                        if is_adult {
                            // Castle and Ganon exterior share the same entrance ids.
                            m.scene = s::OOT_GREAT_FAIRY_DEFENSE as u32;
                            return e::OOT_FAIRY_DEFENSE_ENTR;
                        }
                        m.scene = s::OOT_GREAT_FAIRY_CASTLE as u32;
                    }
                    // Farore's wind
                    e::OOT_ZORA_FOUNTAIN_FROM_FAIRY_ENTR | e::OOT_FAIRY_FARORE_ENTR => {
                        m.scene = s::OOT_GREAT_FAIRY_FARORE as u32;
                    }
                    // Nayru's love
                    e::OOT_DESERT_COLOSSUS_FROM_FAIRY_ENTR | e::OOT_FAIRY_NAYRU_ENTR => {
                        m.scene = s::OOT_GREAT_FAIRY_NAYRU as u32;
                    }
                    e::OOT_HYRULE_CASTLE_FROM_FAIRY_ENTR => {
                        if is_adult {
                            // Same castle / Ganon exterior ambiguity as Din's fire.
                            m.scene = s::OOT_GREAT_FAIRY_DEFENSE as u32;
                            return e::OOT_OUTSIDE_GANON_FROM_FAIRY_ENTR;
                        }
                        // Din's Fire to Castle
                        m.scene = s::OOT_GREAT_FAIRY_CASTLE as u32;
                    }
                    _ => {}
                }
            }

            s::OOT_CASTLE_COURTYARD => {
                if m.entrance_id == e::OOT_CASTLE_STEALTH_ENTR {
                    return e::OOT_CASTLE_STEALTH_FROM_COURTYARD_ENTR;
                }
            }

            s::OOT_HYRULE_CASTLE => match m.entrance_id {
                // Castle Courtyard -> Hyrule Castle
                0x23d => return e::OOT_CASTLE_STEALTH_FROM_COURTYARD_ENTR,
                // Hyrule Castle -> Castle Courtyard
                e::OOT_CASTLE_STEALTH_ENTR => return e::OOT_CASTLE_COURTYARD_ENTR,
                e::OOT_CASTLE_CAUGHT_ENTR => {
                    if out {
                        return e::OOT_CASTLE_GATE_ENTR;
                    }
                }
                _ => {}
            },

            s::OOT_GANON_CASTLE_EXTERIOR => match m.entrance_id {
                // Ganon castle exterior shares the same entrance ids as Hyrule castle.
                e::OOT_MARKET_FROM_CASTLE_ENTR => {
                    return e::OOT_MARKET_ADULT_FROM_GANON_CASTLE_EXTERIOR_ENTR
                }
                e::OOT_HYRULE_CASTLE_ENTR => return e::OOT_GANON_CASTLE_EXTERIOR_ENTR,
                e::OOT_HYRULE_CASTLE_FROM_FAIRY_ENTR => {
                    return e::OOT_OUTSIDE_GANON_FROM_FAIRY_ENTR
                }
                _ => {}
            },

            s::OOT_MARKET_ADULT => {
                m.scene = s::OOT_MARKET as u32;
                match m.entrance_id {
                    e::OOT_MARKET_FROM_CASTLE_ENTR => {
                        return e::OOT_MARKET_ADULT_FROM_GANON_CASTLE_EXTERIOR_ENTR
                    }
                    e::OOT_HYRULE_CASTLE_ENTR => return e::OOT_GANON_CASTLE_EXTERIOR_ENTR,
                    _ => {}
                }
            }

            s::OOT_MARKET_CHILD_DAY | s::OOT_MARKET_CHILD_NIGHT => {
                m.scene = s::OOT_MARKET as u32;
            }

            s::OOT_BACK_ALLEY_DAY | s::OOT_BACK_ALLEY_NIGHT => {
                m.scene = s::OOT_BACK_ALLEY as u32;
            }

            s::OOT_TEMPLE_OF_TIME_EXTERIOR_CHILD_DAY
            | s::OOT_TEMPLE_OF_TIME_EXTERIOR_CHILD_NIGHT
            | s::OOT_TEMPLE_OF_TIME_EXTERIOR_ADULT => {
                m.scene = s::OOT_TEMPLE_OF_TIME_ENTRYWAY as u32;
            }

            s::OOT_MARKET_ENTRANCE_CHILD_DAY
            | s::OOT_MARKET_ENTRANCE_CHILD_NIGHT
            | s::OOT_MARKET_ENTRANCE_ADULT => {
                m.scene = s::OOT_MARKET_ENTRANCE as u32;
            }

            s::OOT_TOMB_DAMPE_WINDMILL => match m.entrance_id {
                e::OOT_KAKARIKO_FROM_WINDMILL_ENTR | e::OOT_WINDMILL_ENTR => {
                    m.scene = s::OOT_WINDMILL as u32;
                }
                _ => {}
            },

            s::OOT_RANCH_HOUSE_SILO => match m.entrance_id {
                e::OOT_LON_LON_RANCH_FROM_SILO_ENTR | e::OOT_SILO_ENTR => {
                    m.scene = s::OOT_SILO as u32;
                }
                _ => {}
            },

            s::OOT_GROTTOS => match m.entrance_id {
                // Lost Woods scrub nuts upgrade grotto entry
                e::OOT_GROTTO_SCRUB_UPGRADE_ENTR => {
                    m.scene = s::OOT_GROTTO_LOST_WOODS_SCRUB_UPGRADE as u32;
                }
                // Deku's Theater grotto entry
                e::OOT_GROTTO_DEKU_THEATER_ENTR => {
                    m.scene = s::OOT_GROTTO_LOST_WOODS_THEATER as u32;
                }
                // Sacred Forest Meadow wolfos grotto entry
                e::OOT_GROTTO_WOLFOS_ENTR => {
                    m.scene = s::OOT_GROTTO_SACRED_MEADOW_WOLFOS as u32;
                }
                // Hyrule Field scrub grotto entry
                e::OOT_GROTTO_SCRUB_HEART_PIECE_ENTR => {
                    m.scene = s::OOT_GROTTO_HYRULE_SCRUBS as u32;
                }
                // Hyrule Field cow grotto entry
                e::OOT_GROTTO_FIELD_COW_ENTR => {
                    m.scene = s::OOT_GROTTO_HYRULE_GERUDO as u32;
                }
                // Hyrule Field tree near kakariko grotto entry
                e::OOT_GROTTO_FIELD_TREE_ENTR => {
                    m.scene = s::OOT_GROTTO_HYRULE_KAKARIKO as u32;
                }
                // Hyrule Castle grotto entry
                e::OOT_GROTTO_CASTLE_ENTR => {
                    m.scene = s::OOT_GROTTO_CASTLE_STORMS as u32;
                }
                // Hyrule Field tektite grotto entry
                e::OOT_GROTTO_TEKTITE_ENTR => {
                    m.scene = s::OOT_GROTTO_HYRULE_TEKTITE as u32;
                }
                // Death Mountain Trail cow grotto entry
                e::OOT_GROTTO_TRAIL_COW_ENTR => {
                    m.scene = s::OOT_GROTTO_DEATH_TRIAL_COW as u32;
                }
                // Kakariko redead grotto entry
                e::OOT_GROTTO_REDEAD_ENTR => {
                    m.scene = s::OOT_GROTTO_KAKARIKO_REDEAD as u32;
                }
                // Gerudo Valley octorok grotto entry
                e::OOT_GROTTO_OCTOROK_ENTR => {
                    m.scene = s::OOT_GROTTO_VALLEY_OCTOROK as u32;
                }
                _ => {}
            },

            _ => {}
        }
    } else if m.game == Game::Mm {
        // MM switches on the current scene byte.
        match m.curr_scene as u16 {
            s::MM_SOUTHERN_SWAMP_CLEAR => {
                m.scene = s::MM_SOUTHERN_SWAMP as u32;
                m.curr_scene = s::MM_SOUTHERN_SWAMP as u8;
                // Southern Swamp clear == scene 0, also used for new-cycle detection,
                // so we must key off the entrance id to tell the two apart.
                match m.entrance_id {
                    e::MM_SWAMP_CLEARED_FROM_SPIDER_HOUSE_ENTR => {
                        return e::MM_SWAMP_FROM_SPIDER_HOUSE_ENTR
                    }
                    e::MM_SWAMP_CLEARED_FROM_ROAD_ENTR => return e::MM_SWAMP_FROM_ROAD_ENTR,
                    e::MM_SWAMP_CLEARED_FROM_PALACE_MAIN_ENTRANCE_ENTR => {
                        return e::MM_SWAMP_FROM_PALACE_MAIN_ENTRANCE_ENTR
                    }
                    e::MM_SWAMP_CLEARED_FROM_PALACE_LEDGE_ENTR => {
                        return e::MM_SWAMP_FROM_PALACE_LEDGE_ENTR
                    }
                    e::MM_SWAMP_CLEARED_FROM_WOODFALL_ENTR => {
                        return e::MM_SWAMP_FROM_WOODFALL_ENTR
                    }
                    e::MM_SWAMP_CLEARED_FROM_MYSTERY_WOODS_ENTR => {
                        return e::MM_SWAMP_FROM_MYSTERY_WOODS_ENTR
                    }
                    e::MM_SWAMP_CLEARED_FROM_IKANA_CANYON_ENTR => {
                        return e::MM_SWAMP_FROM_IKANA_CANYON_ENTR
                    }
                    e::MM_SWAMP_CLEARED_FROM_POTION_SHOP_ENTR => {
                        return e::MM_SWAMP_FROM_POTION_SHOP_ENTR
                    }
                    e::MM_SWAMP_CLEARED_FROM_TOURIST_INFORMATION_ENTR => {
                        return e::MM_SWAMP_FROM_TOURIST_INFORMATION_ENTR
                    }
                    e::MM_WARP_OWL_SOUTHERN_SWAMP_CLEARED_ENTR => {
                        return e::MM_WARP_OWL_SOUTHERN_SWAMP_ENTR
                    }
                    e::MM_KOUME_TARGET_FROM_TOURIST_ENTR => {
                        return e::MM_KOUME_RIDE_FROM_TOURIST_ENTR
                    }
                    e::MM_WOODFALL_ENTR => return e::MM_WOODFALL_ENTR,
                    e::MM_TOURIST_INFORMATION_ENTR => return e::MM_TOURIST_INFORMATION_ENTR,
                    e::MM_POTION_SHOP_ENTR => return e::MM_POTION_SHOP_ENTR,
                    e::MM_MYSTERY_WOODS_ENTR => return e::MM_MYSTERY_WOODS_ENTR,
                    e::MM_DEKU_PALACE_MAIN_ENTRANCE_ENTR => {
                        return e::MM_DEKU_PALACE_MAIN_ENTRANCE_ENTR
                    }
                    e::MM_DEKU_PALACE_LEDGE_ENTR => return e::MM_DEKU_PALACE_LEDGE_ENTR,
                    e::MM_SPIDER_HOUSE_SWAMP_ENTR => return e::MM_SPIDER_HOUSE_SWAMP_ENTR,
                    e::MM_SWAMP_ROAD_FROM_SWAMP_ENTR => return e::MM_SWAMP_ROAD_FROM_SWAMP_ENTR,
                    e::MM_GROTTO_TYPE_GENERIC_ENTR => return e::MM_GROTTO_TYPE_GENERIC_ENTR,
                    e::MM_KOUME_RIDE_ENTR => return e::MM_KOUME_RIDE_ENTR,
                    _ => {}
                }
                // The entrance doesn't match ! Probably a new cycle.
                m.scene = 0;
                m.curr_scene = 0;
            }

            s::MM_MOUNTAIN_VILLAGE_WINTER | s::MM_MOUNTAIN_VILLAGE_SPRING => {
                m.scene = s::MM_MOUNTAIN_VILLAGE as u32;
                m.curr_scene = s::MM_MOUNTAIN_VILLAGE as u8;
                match m.entrance_id {
                    e::MM_MOUNTAIN_VILLAGE_SPRING_FROM_BLACKSMITH_ENTR => {
                        return e::MM_MOUNTAIN_VILLAGE_FROM_BLACKSMITH_ENTR
                    }
                    e::MM_WARP_OWL_MOUNTAIN_VILLAGE_SPRING_ENTR => {
                        return e::MM_WARP_OWL_MOUNTAIN_VILLAGE_ENTR
                    }
                    e::MM_MOUNTAIN_VILLAGE_SPRING_FROM_SNOWHEAD_PATH_ENTR => {
                        return e::MM_MOUNTAIN_VILLAGE_FROM_SNOWHEAD_PATH_ENTR
                    }
                    e::MM_MOUNTAIN_VILLAGE_SPRING_FROM_GORON_GRAVEYARD_ENTR => {
                        return e::MM_MOUNTAIN_VILLAGE_FROM_GORON_GRAVEYARD_ENTR
                    }
                    e::MM_MOUNTAIN_VILLAGE_SPRING_FROM_TWIN_ISLANDS_ENTR => {
                        return e::MM_MOUNTAIN_VILLAGE_FROM_TWIN_ISLANDS_ENTR
                    }
                    e::MM_TWIN_ISLANDS_SPRING_FROM_MOUNTAIN_VILLAGE_ENTR => {
                        return e::MM_TWIN_ISLANDS_FROM_MOUNTAIN_VILLAGE_ENTR
                    }
                    e::MM_MOUNTAIN_VILLAGE_SPRING_FROM_TERMINA_PATH_ENTR => {
                        return e::MM_MOUNTAIN_VILLAGE_FROM_PATH_ENTR
                    }
                    _ => {}
                }
            }

            s::MM_TWIN_ISLANDS_SPRING | s::MM_TWIN_ISLANDS_WINTER => {
                m.scene = s::MM_TWIN_ISLANDS as u32;
                m.curr_scene = s::MM_TWIN_ISLANDS as u8;
                match m.entrance_id {
                    e::MM_TWIN_ISLANDS_SPRING_FROM_MOUNTAIN_VILLAGE_ENTR => {
                        return e::MM_TWIN_ISLANDS_FROM_MOUNTAIN_VILLAGE_ENTR
                    }
                    e::MM_TWIN_ISLANDS_SPRING_FROM_GORON_VILLAGE_ENTR => {
                        return e::MM_TWIN_ISLANDS_FROM_GORON_VILLAGE_ENTR
                    }
                    e::MM_TWIN_ISLANDS_SPRING_FROM_GORON_RACETRACK_ENTR => {
                        return e::MM_TWIN_ISLANDS_FROM_GORON_RACETRACK_ENTR
                    }
                    e::MM_MOUNTAIN_VILLAGE_SPRING_FROM_TWIN_ISLANDS_ENTR => {
                        return e::MM_MOUNTAIN_VILLAGE_FROM_TWIN_ISLANDS_ENTR
                    }
                    e::MM_GORON_VILLAGE_SPRING_FROM_TWIN_ISLANDS_ENTR => {
                        return e::MM_GORON_VILLAGE_FROM_TWIN_ISLANDS_ENTR
                    }
                    _ => {}
                }
            }

            s::MM_GORON_VILLAGE_SPRING | s::MM_GORON_VILLAGE_WINTER => {
                m.scene = s::MM_GORON_VILLAGE as u32;
                m.curr_scene = s::MM_GORON_VILLAGE as u8;
                match m.entrance_id {
                    e::MM_GORON_VILLAGE_SPRING_FROM_TWIN_ISLANDS_ENTR => {
                        return e::MM_GORON_VILLAGE_FROM_TWIN_ISLANDS_ENTR
                    }
                    e::MM_GORON_VILLAGE_SPRING_FROM_GORON_SHRINE_ENTR => {
                        return e::MM_GORON_VILLAGE_FROM_GORON_SHRINE_ENTR
                    }
                    e::MM_GORON_VILLAGE_FROM_LONE_PEAK_SHRINE_ENTR
                    | e::MM_GORON_VILLAGE_SPRING_FROM_LONE_PEAK_SHRINE_ENTR => {
                        if out {
                            m.scene = s::MM_LONE_PEAK as u32;
                        }
                        return e::MM_GORON_VILLAGE_FROM_LONE_PEAK_SHRINE_ENTR;
                    }
                    e::MM_TWIN_ISLANDS_SPRING_FROM_GORON_VILLAGE_ENTR => {
                        return e::MM_TWIN_ISLANDS_FROM_GORON_VILLAGE_ENTR
                    }
                    _ => {}
                }
            }

            s::MM_PATH_MOUNTAIN_VILLAGE | s::MM_PATH_MOUNTAIN_VILLAGE_SPRING => {
                m.scene = s::MM_PATH_MOUNTAIN_VILLAGE as u32;
                m.curr_scene = s::MM_PATH_MOUNTAIN_VILLAGE as u8;
                if m.entrance_id == e::MM_MOUNTAIN_VILLAGE_SPRING_FROM_TERMINA_PATH_ENTR {
                    return e::MM_MOUNTAIN_VILLAGE_FROM_PATH_ENTR;
                }
            }

            s::MM_PATH_SNOWHEAD | s::MM_PATH_SNOWHEAD_SPRING => {
                m.scene = s::MM_PATH_SNOWHEAD as u32;
                m.curr_scene = s::MM_PATH_SNOWHEAD as u8;
                if m.entrance_id == e::MM_MOUNTAIN_VILLAGE_SPRING_FROM_SNOWHEAD_PATH_ENTR {
                    return e::MM_MOUNTAIN_VILLAGE_FROM_SNOWHEAD_PATH_ENTR;
                }
            }

            s::MM_GROTTOS => match m.entrance_id {
                // Lone peak (engine flags it as a grotto).
                e::MM_GORON_VILLAGE_FROM_LONE_PEAK_SHRINE_ENTR | e::MM_LONE_PEAK_SHRINE_ENTR => {
                    m.scene = s::MM_LONE_PEAK as u32;
                }
                // Termina pillar grotto entry
                e::MM_GROTTO_GENERIC_FIELD_PILLAR_ENTR => {
                    m.scene = s::MM_GROTTO_TERMINA_PILLAR as u32;
                }
                // Termina tall grass grotto entry
                e::MM_GROTTO_GENERIC_GRASS_ENTR => {
                    m.scene = s::MM_GROTTO_TERMINA_TALL_GRASS as u32;
                }
                // Road to southern swamp open grotto entry
                e::MM_GROTTO_GENERIC_PATH_SWAMP_ENTR => {
                    m.scene = s::MM_GROTTO_SOUTHERN_SWAMP_ROAD_OPEN as u32;
                }
                // Woods of mystery open grotto entry
                e::MM_GROTTO_GENERIC_WOODS_ENTR => {
                    m.scene = s::MM_GROTTO_WOODS_OF_MYSTERY_OPEN as u32;
                }
                // Southern swamp open grotto entry
                e::MM_GROTTO_GENERIC_SWAMP_ENTR => {
                    m.scene = s::MM_GROTTO_SOUTHERN_SWAMP_OPEN as u32;
                }
                // Mountain village open grotto entry
                e::MM_GROTTO_GENERIC_MOUNTAIN_VILLAGE_ENTR => {
                    m.scene = s::MM_GROTTO_MOUNTAIN_VILLAGE_GENERIC as u32;
                }
                // Twin islands ramp grotto entry
                e::MM_GROTTO_GENERIC_TWIN_ISLANDS_ENTR => {
                    m.scene = s::MM_GROTTO_TWIN_ISLANDS_RAMP as u32;
                }
                // Path to snowhead grotto entry
                e::MM_GROTTO_GENERIC_PATH_SNOWHEAD_ENTR => {
                    m.scene = s::MM_GROTTO_PATH_TO_SNOWHEAD_GENERIC as u32;
                }
                // Great bay coast open grotto
                e::MM_GROTTO_GENERIC_GREAT_BAY_COAST_ENTR => {
                    m.scene = s::MM_GROTTO_GREAT_BAY_COAST_FISHERMAN as u32;
                }
                // Zora cape grotto entry
                e::MM_GROTTO_GENERIC_ZORA_CAPE_ENTR => {
                    m.scene = s::MM_GROTTO_ZORA_CAPE_GENERIC as u32;
                }
                // Road to ikana grotto entry
                e::MM_GROTTO_GENERIC_PATH_IKANA_ENTR => {
                    m.scene = s::MM_GROTTO_IKANA_ROAD_GENERIC as u32;
                }
                // Ikana graveyard grotto entry
                e::MM_GROTTO_GENERIC_GRAVEYARD_ENTR => {
                    m.scene = s::MM_GROTTO_IKANA_GRAVEYARD_GENERIC as u32;
                }
                // Ikana canyon open grotto entry
                e::MM_GROTTO_GENERIC_VALLEY_ENTR => {
                    m.scene = s::MM_GROTTO_IKANA_VALLEY_OPEN as u32;
                }
                // Termina field cow grotto entry
                e::MM_GROTTO_COW_FIELD_ENTR => {
                    m.scene = s::MM_GROTTO_TERMINA_COW as u32;
                }
                // Great bay coast cow grotto entry
                e::MM_GROTTO_COW_COAST_ENTR => {
                    m.scene = s::MM_GROTTO_GREAT_BAY_COAST_COW as u32;
                }
                // Termina ocean gossip grotto entry
                e::MM_GROTTO_GOSSIPS_OCEAN_ENTR => {
                    m.scene = s::MM_GROTTO_TERMINA_OCEAN_GOSSIP as u32;
                }
                // Termina swamp gossip grotto entry
                e::MM_GROTTO_GOSSIPS_SWAMP_ENTR => {
                    m.scene = s::MM_GROTTO_TERMINA_SWAMP_GOSSIP as u32;
                }
                // Termina canyon gossip grotto entry
                e::MM_GROTTO_GOSSIPS_CANYON_ENTR => {
                    m.scene = s::MM_GROTTO_TERMINA_CANYON_GOSSIP as u32;
                }
                // Termina mountain gossip grotto entry
                e::MM_GROTTO_GOSSIPS_MOUNTAIN_ENTR => {
                    m.scene = s::MM_GROTTO_TERMINA_MOUNTAIN_GOSSIP as u32;
                }
                // Twin islands frozen grotto entry
                e::MM_GROTTO_HOT_WATER_ENTR => {
                    m.scene = s::MM_GROTTO_TWIN_ISLANDS_FROZEN as u32;
                }
                // MM JP layout deku palace line grotto entry
                e::MM_GROTTO_EXIT_JP_LINE_START
                | e::MM_GROTTO_EXIT_JP_LINE_END
                | e::MM_GROTTO_JP_LINE_START_ENTR
                | e::MM_GROTTO_JP_LINE_END_ENTR => {
                    m.scene = s::MM_GROTTO_DEKU_PALACE_GENERIC as u32;
                }
                // Termina dodongo grotto entry
                e::MM_GROTTO_DODONGO_ENTR => {
                    m.scene = s::MM_GROTTO_TERMINA_DODONGO as u32;
                }
                // MM JP layout deku palace climb grotto entry
                e::MM_GROTTO_EXIT_JP_CLIMB_LEFT
                | e::MM_GROTTO_EXIT_JP_CLIMB_RIGHT
                | e::MM_GROTTO_JP_CLIMB_LEFT_ENTR
                | e::MM_GROTTO_JP_CLIMB_RIGHT_ENTR => {
                    m.scene = s::MM_GROTTO_DEKU_PALACE_CLIMB as u32;
                }
                // Termina field scrub grotto entry
                e::MM_GROTTO_SCRUB_ENTR => {
                    m.scene = s::MM_GROTTO_TERMINA_SCRUB as u32;
                }
                // Termina bio baba grotto entry
                e::MM_GROTTO_BIO_BABA_ENTR => {
                    m.scene = s::MM_GROTTO_TERMINA_BIO_BABA as u32;
                }
                // Deku palace beans grotto entry
                e::MM_GROTTO_EXIT_BEAN | e::MM_GROTTO_BEAN_ENTR => {
                    m.scene = s::MM_GROTTO_DEKU_PALACE_BEANS as u32;
                }
                // Termina peahat grotto entry
                e::MM_GROTTO_PEAHAT_ENTR => {
                    m.scene = s::MM_GROTTO_TERMINA_PEAHAT as u32;
                }
                // Clock town deku playground grotto entry
                e::MM_GROTTO_DEKU_PLAYGROUND_ENTR => {
                    m.scene = s::MM_DEKU_PLAYGROUND as u32;
                }
                _ => {}
            },

            s::MM_CUTSCENE_MAP => {
                m.scene = s::MM_EXTRA as u32;
            }

            s::MM_ZORA_HALL_ROOMS => match m.entrance_id {
                e::MM_ROOM_EVANS_ENTR => m.scene = s::MM_ZORA_EVANS_ROOM as u32,
                e::MM_ZORA_HALL_FROM_EVANS_ENTR => {
                    if out {
                        m.scene = s::MM_ZORA_EVANS_ROOM as u32;
                    }
                }
                e::MM_ROOM_JAPAS_ENTR => m.scene = s::MM_ZORA_JAPAS_ROOM as u32,
                e::MM_ZORA_HALL_FROM_JAPAS_ENTR => {
                    if out {
                        m.scene = s::MM_ZORA_JAPAS_ROOM as u32;
                    }
                }
                e::MM_ROOM_TIJO_ENTR => m.scene = s::MM_ZORA_TIJO_ROOM as u32,
                e::MM_ZORA_HALL_FROM_TIJO_ENTR => {
                    if out {
                        m.scene = s::MM_ZORA_TIJO_ROOM as u32;
                    }
                }
                e::MM_ROOM_LULU_ENTR => m.scene = s::MM_ZORA_LULU_ROOM as u32,
                e::MM_ZORA_HALL_FROM_LULU_ENTR => {
                    if out {
                        m.scene = s::MM_ZORA_LULU_ROOM as u32;
                    }
                }
                e::MM_ZORA_SHOP_ENTR => m.scene = s::MM_ZORA_SHOP as u32,
                e::MM_ZORA_HALL_FROM_SHOP_ENTR => {
                    if out {
                        m.scene = s::MM_ZORA_SHOP as u32;
                    }
                }
                _ => {}
            },

            s::MM_CASTLE_IKANA => {
                if m.curr_room != 0 {
                    m.scene = s::MM_INSIDE_CASTLE_IKANA as u32;
                }
            }

            s::MM_MOON_DEKU => {
                if m.entrance_id == e::MM_MOON_ENTR {
                    return e::MM_MOON_FROM_MOON_DEKU_ENTR;
                }
            }

            s::MM_MOON_GORON => {
                if m.entrance_id == e::MM_MOON_ENTR {
                    return e::MM_MOON_FROM_MOON_GORON_ENTR;
                }
            }

            s::MM_MOON_ZORA => {
                if m.entrance_id == e::MM_MOON_ENTR {
                    return e::MM_MOON_FROM_MOON_ZORA_ENTR;
                } else if m.entrance_id == e::MM_MOON_ZORA_FROM_WRONG_PIPE_ENTR && out {
                    return e::MM_MOON_ZORA_WRONG_PIPE_ENTR;
                }
            }

            s::MM_MOON_LINK => {
                if m.entrance_id == e::MM_MOON_ENTR {
                    return e::MM_MOON_FROM_MOON_LINK_ENTR;
                }
            }

            s::MM_LAIR_MAJORA => {
                // Beating Majora should not lead to a shuffled entrance.
                if out {
                    m.scene = e::WARP_SCENE;
                }
            }

            s::MM_DAMPE_HOUSE => {
                if m.entrance_id == e::MM_IKANA_GRAVEYARD_FROM_DAMPE_ENTR {
                    return e::MM_DAMPE_TO_GRAVEYARD_ENTR;
                }
            }

            s::MM_IKANA_CANYON => match m.entrance_id {
                e::MM_IKANA_CAVERN_ENTR => {
                    if !out {
                        m.scene = s::MM_SPRING_WATER_CAVE as u32;
                    }
                }
                e::MM_IKANA_CANYON_FROM_CAVERN_ENTR => {
                    if out {
                        m.scene = s::MM_SPRING_WATER_CAVE as u32;
                    }
                }
                _ => {}
            },

            s::MM_IKANA_GRAVEYARD => {
                if m.entrance_id == e::MM_GRAVE_NIGHT3_ENTR {
                    return e::MM_GRAVE_EXIT_NIGHT3;
                }
            }

            s::MM_FAIRY_FOUNTAIN => match m.entrance_id {
                e::MM_CLOCK_TOWN_NORTH_FROM_FAIRY_FOUNTAIN_ENTR | e::MM_FAIRY_FOUNTAIN_TOWN_ENTR => {
                    m.scene = s::MM_FAIRY_CLOCK_TOWN as u32;
                }
                e::MM_WOODFALL_FROM_FAIRY_FOUNTAIN_ENTR | e::MM_FAIRY_FOUNTAIN_WOODFALL_ENTR => {
                    m.scene = s::MM_FAIRY_WOODFALL as u32;
                }
                e::MM_SNOWHEAD_FROM_FAIRY_FOUNTAIN_ENTR | e::MM_FAIRY_FOUNTAIN_SNOWHEAD_ENTR => {
                    m.scene = s::MM_FAIRY_SNOWHEAD as u32;
                }
                e::MM_GREAT_BAY_FROM_FAIRY_FOUNTAIN_ENTR | e::MM_FAIRY_FOUNTAIN_GREAT_BAY_ENTR => {
                    m.scene = s::MM_FAIRY_GREAT_BAY_COAST as u32;
                }
                e::MM_IKANA_CANYON_FROM_FAIRY_FOUNTAIN_ENTR | e::MM_FAIRY_FOUNTAIN_IKANA_ENTR => {
                    m.scene = s::MM_FAIRY_IKANA as u32;
                }
                _ => {}
            },

            s::MM_RANCH_HOUSE_BARN => match m.entrance_id {
                e::MM_ROMANI_RANCH_FROM_RANCH_HOUSE_ENTR | e::MM_RANCH_HOUSE_ENTR => {
                    m.scene = s::MM_ROMANI_RANCH_BARN as u32;
                }
                _ => {}
            },

            s::MM_DEKU_KING_CHAMBER => {
                if m.entrance_id == e::MM_DEKU_PALACE_CAUGHT {
                    m.scene = s::MM_DEKU_PALACE as u32;
                }
            }

            s::MM_DEKU_PALACE => {
                if out {
                    match m.entrance_id {
                        e::MM_GROTTO_EXIT_JP_CLIMB_LEFT | e::MM_GROTTO_EXIT_JP_CLIMB_RIGHT => {
                            m.scene = s::MM_GROTTO_DEKU_PALACE_CLIMB as u32;
                        }
                        e::MM_GROTTO_EXIT_JP_LINE_START | e::MM_GROTTO_EXIT_JP_LINE_END => {
                            m.scene = s::MM_GROTTO_DEKU_PALACE_GENERIC as u32;
                        }
                        e::MM_GROTTO_EXIT_BEAN => {
                            m.scene = s::MM_GROTTO_DEKU_PALACE_BEANS as u32;
                        }
                        _ => {}
                    }
                } else if m.entrance_id == e::MM_DEKU_PALACE_CAUGHT {
                    // Message IN
                    return e::MM_DEKU_PALACE_BRIDGE;
                }
            }

            s::MM_TEMPLE_STONE_TOWER_INVERTED => match m.entrance_id {
                e::MM_BOSS_TEMPLE_STONE_TOWER_ENTR => {
                    if out {
                        return e::MM_STONE_TOWER_TEMPLE_TO_BOSS_ENTR;
                    }
                    m.scene = s::MM_LAIR_TWINMOLD as u32;
                }
                e::MM_BOSS_ARENA_TWINMOLD_ENTR => {
                    m.scene = s::MM_LAIR_TWINMOLD as u32;
                    if out {
                        return e::MM_BOSS_TEMPLE_STONE_TOWER_INVERTED_TO_BOSS_ARENA;
                    }
                }
                _ => {}
            },

            _ => {}
        }
    }

    m.entrance_id
}

// ── Scene-entrance meta (GetSceneEntranceMetaInf gate) ───────────────────────

/// The entrance ids that render on a scene (SceneEntranceMeta). `None` mirrors
/// the C++ `GetSceneEntranceMetaInf` returning nullptr, which drops the
/// transition (the scene is not a tracked entrance scene).
pub fn scene_entrances(game: Game, scene: u32) -> Option<&'static [u32]> {
    if scene > u16::MAX as u32 {
        return None;
    }
    let table = match game {
        Game::Oot => crate::data::OOT_SCENE_ENTRANCES,
        Game::Mm => crate::data::MM_SCENE_ENTRANCES,
    };
    table
        .binary_search_by_key(&(scene as u16), |&(s, _)| s)
        .ok()
        .map(|i| table[i].1)
}

/// GetSceneEntranceMetaInf + `EntranceIDs.find`: the scene exists *and* lists
/// this entrance id. The C++ dereferences the find iterator unchecked, so it
/// relies on the entrance always being present; here the membership test drops
/// the (otherwise data-inconsistent) transition instead of reading garbage.
fn scene_has_entrance(game: Game, scene: u32, entrance_id: u32) -> bool {
    scene_entrances(game, scene).is_some_and(|ids| ids.contains(&entrance_id))
}

// ── OUT / IN assembly (ParseOutgoingMessage / ParseIncomingMessage) ──────────

/// A validated entrance transition (the C++ `NotifyEntranceFound` payload):
/// leaving `out_entrance` in `out_scene` led to `in_entrance` in `in_scene`.
/// The scenes carry the resolved SceneEntranceMeta scene of each endpoint; the
/// current UI keys purely by the (game-unique) entrance id, but they are kept
/// for scene-accurate rendering / logging.
#[derive(Clone, Copy, Debug)]
#[allow(dead_code)]
pub struct EntranceEvent {
    pub out_game: Game,
    pub out_scene: u32,
    pub out_entrance: u32,
    pub in_game: Game,
    pub in_scene: u32,
    pub in_entrance: u32,
}

/// Stateful two-message assembler — port of `EntranceHelper`'s OUT/IN parsing.
/// An OUT message resolves and arms `is_entrance_touched`; the next IN message
/// pairs with it to yield a connection (or is discarded by the predicates).
#[derive(Default)]
pub struct EntranceHelper {
    /// A valid OUT is waiting to be paired with the next IN.
    is_entrance_touched: bool,
    /// The last decoded OUT message, mutated in place by `parse_outgoing`.
    out: Option<EntranceMsg>,
}

impl EntranceHelper {
    /// ParseEntranceMessage: route a raw shared-memory event to the OUT or IN
    /// handler. Returns the discovered connection when an IN completes a pair.
    pub fn parse(&mut self, ev: &Event) -> Option<EntranceEvent> {
        match ev.mem & 0xFF00_0000 {
            IN_MAGIC => self.parse_incoming(decode(ev)),
            OUT_MAGIC => {
                self.parse_outgoing(decode(ev));
                None
            }
            _ => None,
        }
    }

    /// ParseOutgoingMessage: resolve the leaving entrance and arm the pairing.
    fn parse_outgoing(&mut self, mut msg: EntranceMsg) {
        if is_mm_extra(&msg) {
            self.is_entrance_touched = false;
            self.out = Some(msg);
            return;
        }

        self.is_entrance_touched = true;
        let mut is_warp_song = false;

        // Check that the entrance is not a special case.
        msg.entrance_id = check_special_case(&mut msg);

        if msg.scene == e::WARP_SCENE {
            // We don't want to catch this.
            self.is_entrance_touched = false;
            self.out = Some(msg);
            return;
        }

        if is_grotto_exit(msg.entrance_id) {
            // The current entrance is a grotto exit.
            msg.entrance_id = get_grotto_exit(
                msg.game,
                msg.curr_scene as u16,
                msg.curr_room,
                msg.grotto_data,
                msg.scene as u16,
            );
            msg.scene = correct_grotto_scene(msg.game, msg.entrance_id);
        } else if is_grotto_entrance(msg.entrance_id) {
            // The current entrance is a grotto entrance.
            msg.entrance_id =
                get_grotto_entrance(msg.game, msg.entrance_id, msg.grotto_data, msg.curr_scene as u16);
            msg.scene = msg.curr_scene as u32;
        } else if is_grotto_entrance_out(msg.game, msg.entrance_id) {
            msg.scene = msg.curr_scene as u32;
        } else if is_warp_entrance(msg.entrance_id) {
            // The current entrance is a warp zone.
            let (g, ent, warp) =
                get_warp_song(msg.game, msg.scene as u16, msg.song, msg.owl_id, msg.entrance_id);
            msg.game = g;
            msg.entrance_id = ent;
            is_warp_song = warp;

            if !is_warp_song {
                let (new_scene, new_ent) = check_wrap_scene(msg.game, msg.scene as u16, msg.entrance_id);
                msg.scene = new_scene;
                msg.entrance_id = new_ent;

                if msg.scene == e::WARP_SCENE {
                    // We don't want to catch this.
                    self.is_entrance_touched = false;
                    self.out = Some(msg);
                    return;
                }
            }
        } else if is_farore_wind(&msg) {
            // We don't want to catch this.
            self.is_entrance_touched = false;
            self.out = Some(msg);
            return;
        }

        // Retrieve the entrance meta information (entrance ids are unique per game).
        match lookup(msg.game, msg.entrance_id) {
            None => {
                self.is_entrance_touched = false;
            }
            Some(meta) if !is_warp_song => {
                // Reject a spawning location or other scene inconsistency.
                let expected = if meta.type_ == EntranceType::One_Way_Out {
                    meta.to_scene as u32
                } else {
                    meta.from_scene as u32
                };
                if expected != msg.scene {
                    self.is_entrance_touched = false;
                }
            }
            Some(_) => {}
        }

        self.out = Some(msg);
    }

    /// ParseIncomingMessage: resolve the arriving entrance, apply the validity
    /// predicates, and pair it with the armed OUT into a connection.
    fn parse_incoming(&mut self, mut msg: EntranceMsg) -> Option<EntranceEvent> {
        if !self.is_entrance_touched {
            self.is_entrance_touched = false;
            return None;
        }

        msg.entrance_id = check_special_case(&mut msg);

        // Owned copy of the OUT so the predicates and later field reads don't
        // hold a borrow of `self` across the `is_entrance_touched` writes.
        let Some(out) = self.out.clone() else {
            self.is_entrance_touched = false;
            return None;
        };

        if is_death(&out)
            || is_new_cycle(&out, &msg)
            || is_song_of_double_time(&out, &msg)
            || is_song_of_time(&out, &msg)
            || is_sun_song(&out, &msg)
            || is_sonata_woodfall(&out, &msg)
            || is_spawn(&msg)
        {
            self.is_entrance_touched = false;
            return None;
        }

        if is_grotto_entrance(msg.entrance_id) {
            msg.entrance_id =
                get_grotto_entrance(msg.game, msg.entrance_id, msg.grotto_data, msg.last_scene as u16);
            msg.scene = correct_grotto_scene(msg.game, msg.entrance_id);
        } else if is_grotto_exit(msg.entrance_id) {
            msg.entrance_id = get_grotto_exit(
                msg.game,
                msg.curr_scene as u16,
                msg.curr_room,
                msg.grotto_data,
                msg.scene as u16,
            );
        } else {
            msg.entrance_id = check_grotto_spawn(msg.game, msg.entrance_id, msg.x, msg.y, msg.z);
        }

        // Unknown arriving entrance drops the transition.
        if lookup(msg.game, msg.entrance_id).is_none() {
            self.is_entrance_touched = false;
            return None;
        }

        // Resolve the OUT-side scene / entrance from its meta (One_Way_Out uses
        // the To side, everything else the From side).
        let Some(out_meta) = lookup(out.game, out.entrance_id) else {
            self.is_entrance_touched = false;
            return None;
        };
        let (out_scene, out_entrance) = if out_meta.type_ == EntranceType::One_Way_Out {
            (out_meta.to_scene as u32, out_meta.to_id)
        } else {
            (out_meta.from_scene as u32, out_meta.from_id)
        };

        // GetSceneEntranceMetaInf gate on both sides.
        if !scene_has_entrance(out.game, out_scene, out_entrance)
            || !scene_has_entrance(msg.game, msg.scene, msg.entrance_id)
        {
            self.is_entrance_touched = false;
            return None;
        }

        self.is_entrance_touched = false;
        Some(EntranceEvent {
            out_game: out.game,
            out_scene,
            out_entrance,
            in_game: msg.game,
            in_scene: msg.scene,
            in_entrance: msg.entrance_id,
        })
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    /// Minimal message with everything zeroed but the fields under test.
    fn msg(game: Game, scene: u32, curr_scene: u8, entrance_id: u32) -> EntranceMsg {
        EntranceMsg {
            game,
            incoming: false,
            entrance_id,
            scene,
            curr_scene,
            last_scene: 0,
            curr_room: 0,
            grotto_data: 0,
            song: 0,
            age: 0,
            farore_wind: 0,
            owl_id: 0,
            x: 0.0,
            y: 0.0,
            z: 0.0,
            raw: [0; 6],
        }
    }

    // An id that matches no real entrance / special case, to check pass-through.
    const UNKNOWN: u32 = 0x00FF_FFFE;

    #[test]
    fn special_case_market_adult_rewrites_scene() {
        // OoT Market - Adult collapses onto the single Market scene.
        let mut m = msg(Game::Oot, s::OOT_MARKET_ADULT as u32, 0, UNKNOWN);
        let ent = check_special_case(&mut m);
        assert_eq!(m.scene, s::OOT_MARKET as u32);
        assert_eq!(ent, UNKNOWN); // unmatched entrance id passes through
    }

    #[test]
    fn special_case_mm_season_rewrites_scene_and_curr() {
        // MM Mountain Village Winter maps to the base Mountain Village scene.
        let mut m = msg(Game::Mm, 0, s::MM_MOUNTAIN_VILLAGE_WINTER as u8, UNKNOWN);
        check_special_case(&mut m);
        assert_eq!(m.scene, s::MM_MOUNTAIN_VILLAGE as u32);
        assert_eq!(m.curr_scene, s::MM_MOUNTAIN_VILLAGE as u8);
    }

    #[test]
    fn special_case_majora_warps_out() {
        // Leaving Majora's lair (OUT) must not become a shuffled entrance.
        let mut m = msg(Game::Mm, 0, s::MM_LAIR_MAJORA as u8, UNKNOWN);
        m.incoming = false; // OUT
        check_special_case(&mut m);
        assert_eq!(m.scene, e::WARP_SCENE);
    }

    #[test]
    fn warp_song_blocked_in_oot_lair() {
        // No ocarina in an OoT boss lair -> not treated as a warp song.
        let (g, ent, warp) = get_warp_song(Game::Oot, s::OOT_LAIR_GOHMA, 0, 0, UNKNOWN);
        assert_eq!(g, Game::Oot);
        assert_eq!(ent, UNKNOWN);
        assert!(!warp);
    }

    #[test]
    fn scene_entrances_gate() {
        // Kakariko Bazaar lists its single entrance (OOT_KAKARIKO_BAZAAR_ENTR).
        let ids = scene_entrances(Game::Oot, s::OOT_KAKARIKO_BAZAAR as u32).unwrap();
        assert!(ids.contains(&e::OOT_KAKARIKO_BAZAAR_ENTR));
        assert!(scene_has_entrance(
            Game::Oot,
            s::OOT_KAKARIKO_BAZAAR as u32,
            e::OOT_KAKARIKO_BAZAAR_ENTR
        ));
        // The WARP_SCENE sentinel is not a real entrance scene.
        assert!(scene_entrances(Game::Mm, e::WARP_SCENE).is_none());
    }
}
