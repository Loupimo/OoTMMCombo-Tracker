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

pub struct Spoiler {
    pub rom: RomVersion,
    /// Object `Location` -> item name it holds.
    pub items: HashMap<String, String>,
    /// Multiworld: `Location` -> destination player (1-based), when it differs.
    pub worlds: HashMap<String, u8>,
    /// Scenes running the Master Quest (OoT) / JP (MM) layout.
    pub mq_scenes: HashSet<(Game, u16)>,
}

/// Parse a spoiler log's text content.
pub fn parse(text: &str) -> Spoiler {
    let (items, worlds) = parse_locations(text);
    Spoiler {
        rom: parse_version(text),
        items,
        worlds,
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

/// Read the `Version:` line. Only `v30.1` (a.k.a. stable_30_1) needs the NPC id
/// fix-up, so everything else — including dev — is treated as Dev here.
fn parse_version(text: &str) -> RomVersion {
    for line in text.lines() {
        if let Some(rest) = line.trim().strip_prefix("Version:") {
            let v = rest.trim();
            if !v.starts_with("dev") && v.contains("30.1") {
                return RomVersion::Stable;
            }
            return RomVersion::Dev;
        }
    }
    RomVersion::Dev
}

/// Collect every `<Location>: <Item>` line whose left side is a real object
/// location (prefixed "OOT " / "MM "). Scene headers ("  Kokiri Forest:") have
/// no "`: `" and are skipped; settings lines never carry the game prefix.
///
/// Multiworld spoilers list each world under a "  World N" header. The Rust pool
/// models the single LOCAL world, so we scope to world 1's block — otherwise a
/// later world's identical location strings would overwrite the local ones. The
/// per-location "Player N" destination is still captured (in `worlds`) so the
/// progression can route items to the right player.
fn parse_locations(text: &str) -> (HashMap<String, String>, HashMap<String, u8>) {
    let mut items = HashMap::new();
    let mut worlds = HashMap::new();
    let multiworld = text.lines().any(|l| world_header(l).is_some());
    // Single-world: every location line counts. Multiworld: only inside "World 1".
    let mut in_local = !multiworld;
    for line in text.lines() {
        if let Some(n) = world_header(line) {
            in_local = n == 1;
            continue;
        }
        if !in_local {
            continue;
        }
        let Some((left, right)) = line.split_once(": ") else { continue };
        let loc = left.trim();
        if !(loc.starts_with("OOT ") || loc.starts_with("MM ")) {
            continue;
        }
        let (item, world) = strip_player(right.trim());
        items.insert(loc.to_string(), item.to_string());
        if let Some(w) = world {
            worlds.insert(loc.to_string(), w);
        }
    }
    (items, worlds)
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
        assert_eq!(sp.items.len(), 3);
        assert_eq!(
            sp.items.get("OOT Kokiri Forest Kokiri Sword Chest").map(String::as_str),
            Some("Kokiri Sword")
        );
        // The multiworld "Player N " prefix is stripped and its world captured.
        assert_eq!(
            sp.items.get("MM Great Bay Temple Boss").map(String::as_str),
            Some("Progressive Sword")
        );
        assert_eq!(sp.worlds.get("MM Great Bay Temple Boss"), Some(&2));
    }

    #[test]
    fn detects_stable_30_1() {
        assert_eq!(parse("Version: v30.1\n").rom, RomVersion::Stable);
        assert_eq!(parse("Version: dev-abc\n").rom, RomVersion::Dev);
    }

    #[test]
    fn multiworld_scopes_to_local_world() {
        // The same location exists in both worlds; the local (world 1) item must
        // win, and its destination player is captured.
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
        // World 1's item wins (not overwritten by world 2's identical location).
        assert_eq!(
            sp.items.get("OOT Kokiri Forest Kokiri Sword Chest").map(String::as_str),
            Some("Kokiri Sword")
        );
        // It is destined to player 1; world 2's line is out of scope entirely.
        assert_eq!(sp.worlds.get("OOT Kokiri Forest Kokiri Sword Chest"), Some(&1));
        assert_eq!(sp.items.len(), 1, "only world 1's locations are parsed");
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
}
