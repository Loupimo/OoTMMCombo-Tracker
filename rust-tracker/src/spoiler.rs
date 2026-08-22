//! Parse an OoTMM spoiler-log text file.
//!
//! Mirrors OoTMMComboTracker::LoadGameSpoiler / ParseWorldLocations, but relies
//! on the fact that an object's `Location` is globally unique and carries its
//! game prefix ("OOT " / "MM "). So instead of the hardcoded scene-name map, we
//! scan every `Location: Item` line and key by the full Location — the same
//! global fallback the C++ uses (AssignSpoilerObjectAnyScene).
//!
//! We only take what the functional tracker needs right now:
//!   - the ROM build from the `Version:` line,
//!   - the item held at each location (Location -> item name).
//! Settings (Master Quest, shuffles, …) are a later step.

use std::collections::{HashMap, HashSet};

use crate::data::scenes as sc;
use crate::scene::Game;
use crate::tracking::RomVersion;

/// One world's physical placements (multiworld). Index 0 is world 1 (the local
/// world); single / coop seeds have exactly one entry.
#[derive(Default, Clone)]
pub struct WorldPlacements {
    /// Object `Location` -> item name physically placed there in this world.
    pub items: HashMap<String, String>,
    /// `Location` -> destination player (1-based), when the spoiler carries an
    /// explicit "Player N" prefix. Absent means the item belongs to this world.
    pub dest: HashMap<String, u8>,
}

pub struct Spoiler {
    pub rom: RomVersion,
    /// Per-world placements. Mirrors the Qt per-world scene-object arrays: worlds
    /// beyond the first are the other players' seeds. Always at least one entry.
    pub worlds: Vec<WorldPlacements>,
    /// Scenes running the Master Quest (OoT) / JP (MM) layout.
    pub mq_scenes: HashSet<(Game, u16)>,
}

/// Parse a spoiler log's text content.
pub fn parse(text: &str) -> Spoiler {
    Spoiler {
        rom: parse_version(text),
        worlds: parse_locations(text, build_predates_v32_1(text)),
        mq_scenes: parse_mq(text),
    }
}

/// The twelve OoT dungeons that have a Master Quest variant.
const OOT_MQ_ALL: [u16; 12] = [
    sc::OOT_DEKU_TREE, sc::OOT_DODONGO_CAVERN, sc::OOT_INSIDE_JABU_JABU,
    sc::OOT_TEMPLE_FOREST, sc::OOT_TEMPLE_FIRE, sc::OOT_TEMPLE_WATER,
    sc::OOT_TEMPLE_SHADOW, sc::OOT_TEMPLE_SPIRIT, sc::OOT_BOTTOM_OF_THE_WELL,
    sc::OOT_ICE_CAVERN, sc::OOT_GERUDO_TRAINING_GROUND, sc::OOT_INSIDE_GANON_CASTLE,
];

/// The MM scenes affected by the Deku Palace JP layout.
const MM_JP_ALL: [u16; 4] = [
    sc::MM_GROTTOS, sc::MM_DEKU_PALACE,
    sc::MM_GROTTO_DEKU_PALACE_GENERIC, sc::MM_GROTTO_DEKU_PALACE_CLIMB,
];

/// Map an OoT "Master Quest Dungeons" list entry to its scene.
fn oot_mq_scene(name: &str) -> Option<u16> {
    Some(match name {
        "Deku Tree" => sc::OOT_DEKU_TREE,
        "Dodongo cavern" => sc::OOT_DODONGO_CAVERN,
        "Jabu-Jabu" => sc::OOT_INSIDE_JABU_JABU,
        "Forest Temple" => sc::OOT_TEMPLE_FOREST,
        "Fire Temple" => sc::OOT_TEMPLE_FIRE,
        "Water Temple" => sc::OOT_TEMPLE_WATER,
        "Shadow Temple" => sc::OOT_TEMPLE_SHADOW,
        "Spirit Temple" => sc::OOT_TEMPLE_SPIRIT,
        "Bottom of the Well" => sc::OOT_BOTTOM_OF_THE_WELL,
        "Ice Cavern" => sc::OOT_ICE_CAVERN,
        "Gerudo Training Grounds" => sc::OOT_GERUDO_TRAINING_GROUND,
        "Ganon's Castle" => sc::OOT_INSIDE_GANON_CASTLE,
        _ => return None,
    })
}

/// Parse the "Master Quest Dungeons" / "Majora's Mask JP Layouts" settings into
/// the set of scenes running the alternate layout (mirrors Settings::ParseLayout).
fn parse_mq(text: &str) -> HashSet<(Game, u16)> {
    let mut mq = HashSet::new();
    let lines: Vec<&str> = text.lines().collect();
    for (i, line) in lines.iter().enumerate() {
        let t = line.trim_start();
        if let Some(rest) = t.strip_prefix("Master Quest Dungeons:") {
            add_layout(&mut mq, Game::Oot, rest.trim(), &lines[i + 1..], &OOT_MQ_ALL, |n| {
                oot_mq_scene(n).map(|s| vec![s])
            });
        } else if let Some(rest) = t.strip_prefix("Majora's Mask JP Layouts:") {
            // The only JP list entry is "Deku Palace", which affects four scenes.
            add_layout(&mut mq, Game::Mm, rest.trim(), &lines[i + 1..], &MM_JP_ALL, |n| {
                (n == "Deku Palace").then(|| MM_JP_ALL.to_vec())
            });
        }
    }
    mq
}

/// Apply one layout setting: `all` selects every scene, `none`/absent selects
/// none, and an empty inline value means an indented `- Name` list follows.
fn add_layout(
    mq: &mut HashSet<(Game, u16)>,
    game: Game,
    value: &str,
    following: &[&str],
    all: &[u16],
    name_to_scenes: impl Fn(&str) -> Option<Vec<u16>>,
) {
    if value == "all" {
        mq.extend(all.iter().map(|&s| (game, s)));
    } else if value.is_empty() {
        for l in following {
            let Some(name) = l.trim_start().strip_prefix("- ") else { break };
            if let Some(scenes) = name_to_scenes(name.trim()) {
                mq.extend(scenes.into_iter().map(|s| (game, s)));
            }
        }
    }
}

/// Read the `Version:` line and map it to a build tier (mirror of the C++
/// `OoTMMComboTracker` version parsing): a `dev` build is the internal numbering;
/// `v30.1` is the old `Stable301` shift; any other stable release (v31 / v32.X) is
/// the smaller `Stable` shift. No / unknown version line defaults to `Dev`.
fn parse_version(text: &str) -> RomVersion {
    for line in text.lines() {
        if let Some(rest) = line.trim().strip_prefix("Version:") {
            let v = rest.trim();
            if v.starts_with("dev") {
                return RomVersion::Dev;
            }
            if v.starts_with("v30.1") {
                return RomVersion::Stable301;
            }
            return RomVersion::Stable;
        }
    }
    RomVersion::Dev
}

/// Whether the spoiler's build predates OoTMM v32.1, the release that renamed and reshuffled
/// the Great Bay Coast pot / rock location labels. Such builds list those locations under their
/// former names, so `parse_loc_line` translates them (see `legacy_location`). Dev builds and
/// unrecognised version strings are treated as current (>= v32.1): no translation.
fn build_predates_v32_1(text: &str) -> bool {
    for line in text.lines() {
        let Some(rest) = line.trim().strip_prefix("Version:") else { continue };
        let v = rest.trim();
        if v.starts_with("dev") {
            return false;
        }
        // Parse the leading "MAJOR.MINOR" (ignoring a leading 'v' and any trailing text).
        let digits = v.trim_start_matches(|c: char| !c.is_ascii_digit());
        let mut parts = digits.split('.');
        let major = parse_leading_u32(parts.next().unwrap_or(""));
        let minor = parse_leading_u32(parts.next().unwrap_or(""));
        return major < 32 || (major == 32 && minor < 1);
    }
    false
}

/// Parse the leading run of ASCII digits of `s` as a u32 (0 if none).
fn parse_leading_u32(s: &str) -> u32 {
    s.chars()
        .take_while(|c| c.is_ascii_digit())
        .collect::<String>()
        .parse()
        .unwrap_or(0)
}

/// Map a pre-v32.1 MM location label to its current pool name. Unlisted labels pass through
/// unchanged. Applied only for legacy builds (see `build_predates_v32_1`) because the rename
/// was a reshuffle: the same string denotes different objects across it, so a blind alias would
/// mis-assign. Must stay in sync with the C++ `LegacyMMLocationAliases` table.
fn legacy_location(loc: &str) -> &str {
    match loc {
        "MM Great Bay Coast Pot Ledge 1" => "MM Great Bay Coast Pot Upper Cliffs 1",
        "MM Great Bay Coast Pot Ledge 2" => "MM Great Bay Coast Pot Upper Cliffs 2",
        "MM Great Bay Coast Pot Ledge 3" => "MM Great Bay Coast Pot Upper Cliffs 3",
        "MM Great Bay Coast Pot 01" => "MM Great Bay Coast Pot Ledge 1",
        "MM Great Bay Coast Pot 02" => "MM Great Bay Coast Pot 1",
        "MM Great Bay Coast Pot 03" => "MM Great Bay Coast Pot Lower Cliffs 3",
        "MM Great Bay Coast Pot 04" => "MM Great Bay Coast Pot Lower Cliffs 2",
        "MM Great Bay Coast Pot 05" => "MM Great Bay Coast Pot Platform 1",
        "MM Great Bay Coast Pot 06" => "MM Great Bay Coast Pot Platform 3",
        "MM Great Bay Coast Pot 07" => "MM Great Bay Coast Pot Platform 2",
        "MM Great Bay Coast Pot 08" => "MM Great Bay Coast Pot Ledge 2",
        "MM Great Bay Coast Pot 09" => "MM Great Bay Coast Pot 2",
        "MM Great Bay Coast Pot 10" => "MM Great Bay Coast Pot Lower Cliffs 4",
        "MM Great Bay Coast Pot 11" => "MM Great Bay Coast Pot Lower Cliffs 1",
        "MM Great Bay Coast Pot 12" => "MM Great Bay Coast Pot Platform 4",
        "MM Great Bay Coast Rock Ledge 1" => "MM Great Bay Coast Rock Cliffs 1",
        "MM Great Bay Coast Rock Ledge 2" => "MM Great Bay Coast Rock Cliffs 2",
        "MM Great Bay Coast Rock Ledge 3" => "MM Great Bay Coast Rock Cliffs 3",
        other => other,
    }
}

/// Collect every `<Location>: <Item>` line whose left side is a real object
/// location (prefixed "OOT " / "MM "). Scene headers ("  Kokiri Forest:") have
/// no "`: `" and are skipped; settings lines never carry the game prefix.
///
/// Multiworld spoilers list each world under a "  World N" header (mirroring the
/// Qt LoadGameSpoiler split). We parse EVERY world into its own placement set so
/// the world selector can show any world's map, and each world keeps its own
/// per-location "Player N" destination for the progression routing.
fn parse_locations(text: &str, legacy: bool) -> Vec<WorldPlacements> {
    let multiworld = text.lines().any(|l| world_header(l).is_some());
    if !multiworld {
        // Single / coop: one world, every location line counts.
        let mut w = WorldPlacements::default();
        for line in text.lines() {
            if let Some((loc, item, dest)) = parse_loc_line(line, legacy) {
                w.items.insert(loc.clone(), item);
                if let Some(d) = dest {
                    w.dest.insert(loc, d);
                }
            }
        }
        return vec![w];
    }

    // Multiworld: partition the location lines into per-world blocks by their
    // "World N" headers (worlds are 1-based in the spoiler, index 0 = world 1).
    let mut worlds: Vec<WorldPlacements> = Vec::new();
    let mut cur: Option<usize> = None;
    for line in text.lines() {
        if let Some(n) = world_header(line) {
            let idx = (n as usize).saturating_sub(1);
            if worlds.len() <= idx {
                worlds.resize_with(idx + 1, WorldPlacements::default);
            }
            cur = Some(idx);
            continue;
        }
        let Some(idx) = cur else { continue };
        if let Some((loc, item, dest)) = parse_loc_line(line, legacy) {
            worlds[idx].items.insert(loc.clone(), item);
            if let Some(d) = dest {
                worlds[idx].dest.insert(loc, d);
            }
        }
    }
    if worlds.is_empty() {
        worlds.push(WorldPlacements::default());
    }
    worlds
}

/// Parse one `<Location>: <Item>` line into (location, item, destination player).
/// Returns None for anything that is not a real object-location line.
fn parse_loc_line(line: &str, legacy: bool) -> Option<(String, String, Option<u8>)> {
    let (left, right) = line.split_once(": ")?;
    let loc = left.trim();
    if !(loc.starts_with("OOT ") || loc.starts_with("MM ")) {
        return None;
    }
    // Pre-v32.1 spoilers label some MM locations by their former names; map them to the
    // current pool names before keying (see legacy_location / build_predates_v32_1).
    let loc = if legacy { legacy_location(loc) } else { loc };
    let (item, dest) = strip_player(right.trim());
    Some((loc.to_string(), item.to_string(), dest))
}

/// A multiworld "World N" section header (any indent) -> its 1-based number.
/// Location lines start with the game prefix, so they never match this.
fn world_header(line: &str) -> Option<u8> {
    let rest = line.trim_start().strip_prefix("World ")?;
    let digits: String = rest.chars().take_while(|c| c.is_ascii_digit()).collect();
    digits.parse().ok()
}

/// Multiworld items are prefixed with their destination player
/// ("Player 2 Compass (Water Temple)"). Returns (item name, dest world).
fn strip_player(item: &str) -> (&str, Option<u8>) {
    if let Some(rest) = item.strip_prefix("Player ") {
        if let Some((num, name)) = rest.split_once(' ') {
            if let Ok(w) = num.parse::<u8>() {
                return (name, Some(w));
            }
        }
    }
    (item, None)
}

#[cfg(test)]
mod tests {
    use super::*;

    const SAMPLE: &str = "\
Version: dev-20240101
Setting: something
===========================================================================
Playthrough
===========================================================================
  Kokiri Forest:
    OOT Kokiri Forest Kokiri Sword Chest: Kokiri Sword
    OOT Kokiri Forest GS Soil: Gold Token
  Water Temple:
    MM Great Bay Temple Boss: Player 2 Progressive Sword
";

    #[test]
    fn parses_version_and_items() {
        let sp = parse(SAMPLE);
        assert_eq!(sp.rom, RomVersion::Dev);
        // Single-world seed: one world holding every location.
        assert_eq!(sp.worlds.len(), 1);
        let w = &sp.worlds[0];
        assert_eq!(w.items.len(), 3);
        assert_eq!(
            w.items.get("OOT Kokiri Forest Kokiri Sword Chest").map(String::as_str),
            Some("Kokiri Sword")
        );
        // The multiworld "Player N " prefix is stripped and its world captured.
        assert_eq!(
            w.items.get("MM Great Bay Temple Boss").map(String::as_str),
            Some("Progressive Sword")
        );
        assert_eq!(w.dest.get("MM Great Bay Temple Boss"), Some(&2));
    }

    #[test]
    fn detects_stable_30_1() {
        assert_eq!(parse("Version: v30.1\n").rom, RomVersion::Stable301);
        assert_eq!(parse("Version: dev-abc\n").rom, RomVersion::Dev);
        // Any other stable release (v31 / v32.X) is the latest-stable tier.
        assert_eq!(parse("Version: v32.1\n").rom, RomVersion::Stable);
        assert_eq!(parse("Version: v31.0\n").rom, RomVersion::Stable);
    }

    #[test]
    fn multiworld_parses_every_world() {
        // The same location exists in both worlds with different placements; each
        // world keeps its own item and destination player.
        let sp = parse(
            "Version: dev-mw\n\
             ===========================================================================\n\
             Location List\n\
             ===========================================================================\n\
             \x20\x20World 1 (abcd)\n\
             \x20\x20\x20\x20Kokiri Forest:\n\
             \x20\x20\x20\x20\x20\x20OOT Kokiri Forest Kokiri Sword Chest: Player 1 Kokiri Sword\n\
             \x20\x20World 2 (ef01)\n\
             \x20\x20\x20\x20Kokiri Forest:\n\
             \x20\x20\x20\x20\x20\x20OOT Kokiri Forest Kokiri Sword Chest: Player 2 Fairy Bow\n",
        );
        assert_eq!(sp.worlds.len(), 2, "both worlds are parsed");
        // World 1 physically holds Player 1's Kokiri Sword.
        assert_eq!(
            sp.worlds[0].items.get("OOT Kokiri Forest Kokiri Sword Chest").map(String::as_str),
            Some("Kokiri Sword")
        );
        assert_eq!(sp.worlds[0].dest.get("OOT Kokiri Forest Kokiri Sword Chest"), Some(&1));
        // World 2 physically holds Player 2's Fairy Bow at the same coordinate.
        assert_eq!(
            sp.worlds[1].items.get("OOT Kokiri Forest Kokiri Sword Chest").map(String::as_str),
            Some("Fairy Bow")
        );
        assert_eq!(sp.worlds[1].dest.get("OOT Kokiri Forest Kokiri Sword Chest"), Some(&2));
    }

    #[test]
    fn parses_mq_all_none_and_list() {
        assert_eq!(parse("Master Quest Dungeons: all\n").mq_scenes.len(), 12);
        assert!(parse("Master Quest Dungeons: none\n").mq_scenes.is_empty());

        let list = "  Master Quest Dungeons:\n    - Water Temple\n    - Ganon's Castle\n  goal: boss\n";
        let mq = parse(list).mq_scenes;
        assert_eq!(mq.len(), 2);
        assert!(mq.contains(&(Game::Oot, sc::OOT_TEMPLE_WATER)));
        assert!(mq.contains(&(Game::Oot, sc::OOT_INSIDE_GANON_CASTLE)));
    }

    #[test]
    fn build_version_boundary() {
        // v32.1 is the first build using the new Great Bay Coast names.
        assert!(build_predates_v32_1("Version: v30.1\n"));
        assert!(build_predates_v32_1("Version: v32.0\n"));
        assert!(!build_predates_v32_1("Version: v32.1\n"));
        assert!(!build_predates_v32_1("Version: v33.0\n"));
        assert!(!build_predates_v32_1("Version: dev-20260101\n"));
        assert!(!build_predates_v32_1("Seed: no-version-line\n"));
    }

    #[test]
    fn legacy_great_bay_names_translated_pre_v32_1() {
        // A pre-v32.1 spoiler lists the reshuffled Great Bay Coast pots/rocks under their former
        // names; they must be keyed by the current pool names.
        let sp = parse(
            "Version: v32.0\n\
             \x20\x20Great Bay Coast:\n\
             \x20\x20\x20\x20MM Great Bay Coast Pot 01: Item A\n\
             \x20\x20\x20\x20MM Great Bay Coast Pot Ledge 1: Item B\n\
             \x20\x20\x20\x20MM Great Bay Coast Rock Ledge 3: Item C\n",
        );
        let w = &sp.worlds[0];
        // "Pot 01" (former) -> "Pot Ledge 1" (current).
        assert_eq!(w.items.get("MM Great Bay Coast Pot Ledge 1").map(String::as_str), Some("Item A"));
        // The collision case: "Pot Ledge 1" (former) -> "Pot Upper Cliffs 1" (current).
        assert_eq!(w.items.get("MM Great Bay Coast Pot Upper Cliffs 1").map(String::as_str), Some("Item B"));
        assert_eq!(w.items.get("MM Great Bay Coast Rock Cliffs 3").map(String::as_str), Some("Item C"));
        // The former names must not survive as keys.
        assert!(w.items.get("MM Great Bay Coast Pot 01").is_none());
        assert!(w.items.get("MM Great Bay Coast Rock Ledge 3").is_none());
    }

    #[test]
    fn current_build_keeps_new_great_bay_names() {
        // A current (dev / >= v32.1) spoiler already uses the new names: no translation. The
        // collision string "Pot Ledge 1" must map to its current object, not the former one.
        let sp = parse(
            "Version: dev-20260101\n\
             \x20\x20Great Bay Coast:\n\
             \x20\x20\x20\x20MM Great Bay Coast Pot Ledge 1: Item X\n\
             \x20\x20\x20\x20MM Great Bay Coast Pot Upper Cliffs 1: Item Y\n",
        );
        let w = &sp.worlds[0];
        assert_eq!(w.items.get("MM Great Bay Coast Pot Ledge 1").map(String::as_str), Some("Item X"));
        assert_eq!(w.items.get("MM Great Bay Coast Pot Upper Cliffs 1").map(String::as_str), Some("Item Y"));
    }
}
