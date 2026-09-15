//! Loading the OoTMM game patch file for the dev multiplayer mechanism.
//!
//! In OoTMM builds newer than v32.0 the multiplayer session is described by a
//! `.ootmm` patch file, which is itself a zip archive holding a `meta.json`
//! (session id / secret, world id, game mode) and — since dev — optional
//! `manifests/items.json` (item id -> symbol) and `manifests/locations.json`
//! (location key -> name). This mirrors the Go client's `game.ExtractGameInfo`.
//! The user may point at the `.ootmm` directly, or at the ROM `.zip` that bundles
//! it (alongside a log `.txt` and the `.z64` ROM); both are handled, since both
//! are zip archives — the outer zip simply carries a `*.ootmm` entry that is
//! itself the patch archive.

use std::collections::HashMap;
use std::io::{Cursor, Read};
use std::path::Path;

use serde::Deserialize;

/// Session play mode declared in the patch (`meta.json` "mode").
#[derive(Clone, Copy, PartialEq, Eq, Debug)]
pub enum PatchMode {
    Single,
    Coop,
    Multi,
}

impl PatchMode {
    /// Human label for the Launch page.
    pub fn label(self) -> &'static str {
        match self {
            PatchMode::Single => "Single Player",
            PatchMode::Coop => "Co-op",
            PatchMode::Multi => "Multiworld",
        }
    }
}

/// The multiplayer session identity extracted from a patch file (mirror of the
/// Go client's `game.Info`). Consumed by the dev networking layer (`multi_dev`):
/// the session id / secret authenticate the uplink handshake and are matched
/// against the game's HELLO.
#[derive(Clone)]
pub struct PatchInfo {
    pub session_id: [u8; 16],
    pub session_secret: [u8; 8],
    pub world_id: u8,
    pub mode: PatchMode,
    /// Item id (gi) -> symbol, from `manifests/items.json` (dev). Empty when the
    /// patch predates the manifests or they fail to parse (a dev game's
    /// INFO_ITEM then logs the raw gi instead of a name).
    pub items: HashMap<u16, String>,
    /// Location key -> location name, from `manifests/locations.json` (dev).
    /// Empty under the same conditions as [`Self::items`].
    pub locations: HashMap<u32, String>,
    /// Entrance key -> symbol (e.g. `OOT_DEKU_TREE`), from `manifests/entrances.json`
    /// (dev). Resolves the INFO_ENTRANCE ids into the symbols the tracker's entrance
    /// data is keyed on. Empty under the same conditions as [`Self::items`].
    pub entrances: HashMap<u32, String>,
}

impl PatchInfo {
    /// Short one-line summary shown next to the loaded patch (mode + world).
    pub fn summary(&self) -> String {
        format!("{} — world {}", self.mode.label(), self.world_id)
    }
}

// `meta.json` shape (mirror of the Go `rawMetaFile`): a top-level "meta" object.
#[derive(Deserialize)]
struct RawMeta {
    meta: RawMetaInner,
}

#[derive(Deserialize)]
struct RawMetaInner {
    #[serde(rename = "sessionId")]
    session_id: String,
    #[serde(rename = "sessionSecret")]
    session_secret: String,
    #[serde(rename = "worldId")]
    world_id: u8,
    mode: String,
}

/// Load a patch file and extract its session info. `path` may be the `.ootmm`
/// itself or a `.zip` bundling it; the two cases are tried in that order.
pub fn load(path: &Path) -> Result<PatchInfo, String> {
    let bytes =
        std::fs::read(path).map_err(|e| format!("Cannot read {}: {e}", path.display()))?;

    // Case 1: the chosen file is the .ootmm (a zip with meta.json at its root).
    if let Ok(info) = read_meta_from_ootmm(&bytes) {
        return Ok(info);
    }
    // Case 2: an outer .zip that bundles a *.ootmm entry.
    let inner = extract_ootmm_from_zip(&bytes).map_err(|e| {
        format!("Not an OoTMM patch (no meta.json, and no inner .ootmm): {e}")
    })?;
    read_meta_from_ootmm(&inner)
}

/// Read `meta.json` (and, since dev, the optional item / location manifests) out
/// of a `.ootmm` archive's raw bytes.
fn read_meta_from_ootmm(bytes: &[u8]) -> Result<PatchInfo, String> {
    let mut zip = zip::ZipArchive::new(Cursor::new(bytes)).map_err(|e| e.to_string())?;
    let mut text = String::new();
    {
        let mut file = zip.by_name("meta.json").map_err(|e| e.to_string())?;
        file.read_to_string(&mut text).map_err(|e| e.to_string())?;
    }
    let mut info = parse_meta(&text)?;
    // dev: the manifests are optional. A missing / malformed manifest leaves the
    // map empty (matching the Go client, which silently skips them), so an older
    // patch or a partial archive still loads its session identity.
    info.items = read_items_manifest(&mut zip);
    info.locations = read_locations_manifest(&mut zip);
    info.entrances = read_entrances_manifest(&mut zip);
    Ok(info)
}

/// Read a JSON entry (`name`) from the archive into a string, or `None` when it
/// is absent / unreadable.
fn read_zip_text(zip: &mut zip::ZipArchive<Cursor<&[u8]>>, name: &str) -> Option<String> {
    let mut file = zip.by_name(name).ok()?;
    let mut text = String::new();
    file.read_to_string(&mut text).ok()?;
    Some(text)
}

/// Parse `manifests/items.json` (dev) into an id -> symbol map. Any error (absent
/// file, bad JSON, unexpected version) yields an empty map.
fn read_items_manifest(zip: &mut zip::ZipArchive<Cursor<&[u8]>>) -> HashMap<u16, String> {
    #[derive(Deserialize)]
    struct Entry {
        id: u16,
        sym: String,
    }
    #[derive(Deserialize)]
    struct Manifest {
        version: i64,
        #[serde(default)]
        items: Vec<Entry>,
    }
    let mut out = HashMap::new();
    let Some(text) = read_zip_text(zip, "manifests/items.json") else { return out };
    if let Ok(m) = serde_json::from_str::<Manifest>(&text) {
        if m.version == 1 {
            for e in m.items {
                out.insert(e.id, e.sym);
            }
        }
    }
    out
}

/// Parse `manifests/locations.json` (dev) into a key -> location-name map. Any
/// error yields an empty map (see [`read_items_manifest`]).
fn read_locations_manifest(zip: &mut zip::ZipArchive<Cursor<&[u8]>>) -> HashMap<u32, String> {
    #[derive(Deserialize)]
    struct Entry {
        key: u32,
        location: String,
    }
    #[derive(Deserialize)]
    struct Manifest {
        version: i64,
        #[serde(default)]
        locations: Vec<Entry>,
    }
    let mut out = HashMap::new();
    let Some(text) = read_zip_text(zip, "manifests/locations.json") else { return out };
    if let Ok(m) = serde_json::from_str::<Manifest>(&text) {
        if m.version == 1 {
            for e in m.locations {
                out.insert(e.key, e.location);
            }
        }
    }
    out
}

/// Parse `manifests/entrances.json` (dev) into a key -> symbol map. Any error
/// yields an empty map (see [`read_items_manifest`]).
fn read_entrances_manifest(zip: &mut zip::ZipArchive<Cursor<&[u8]>>) -> HashMap<u32, String> {
    #[derive(Deserialize)]
    struct Entry {
        key: u32,
        sym: String,
    }
    #[derive(Deserialize)]
    struct Manifest {
        version: i64,
        #[serde(default)]
        entrances: Vec<Entry>,
    }
    let mut out = HashMap::new();
    let Some(text) = read_zip_text(zip, "manifests/entrances.json") else { return out };
    if let Ok(m) = serde_json::from_str::<Manifest>(&text) {
        if m.version == 1 {
            for e in m.entrances {
                out.insert(e.key, e.sym);
            }
        }
    }
    out
}

/// Find and extract the first `*.ootmm` entry from an outer zip's raw bytes.
fn extract_ootmm_from_zip(bytes: &[u8]) -> Result<Vec<u8>, String> {
    let mut zip = zip::ZipArchive::new(Cursor::new(bytes)).map_err(|e| e.to_string())?;
    let n = zip.len();
    let idx = (0..n)
        .find(|&i| {
            zip.by_index(i)
                .map(|f| f.name().to_ascii_lowercase().ends_with(".ootmm"))
                .unwrap_or(false)
        })
        .ok_or_else(|| "archive contains no .ootmm file".to_string())?;
    let mut file = zip.by_index(idx).map_err(|e| e.to_string())?;
    let mut out = Vec::new();
    file.read_to_end(&mut out).map_err(|e| e.to_string())?;
    Ok(out)
}

/// Parse the `meta.json` text into a `PatchInfo`.
fn parse_meta(text: &str) -> Result<PatchInfo, String> {
    let raw: RawMeta =
        serde_json::from_str(text).map_err(|e| format!("meta.json parse error: {e}"))?;
    let session_id = hex_decode::<16>(&raw.meta.session_id)?;
    let session_secret = hex_decode::<8>(&raw.meta.session_secret)?;
    let mode = match raw.meta.mode.as_str() {
        "single" => PatchMode::Single,
        "coop" => PatchMode::Coop,
        "multi" => PatchMode::Multi,
        other => return Err(format!("unknown mode: {other}")),
    };
    Ok(PatchInfo {
        session_id,
        session_secret,
        world_id: raw.meta.world_id,
        mode,
        // Filled by `read_meta_from_ootmm` from the dev manifests, if present.
        items: HashMap::new(),
        locations: HashMap::new(),
        entrances: HashMap::new(),
    })
}

/// Decode an exact `N`-byte hex string into a fixed array.
fn hex_decode<const N: usize>(s: &str) -> Result<[u8; N], String> {
    if s.len() != N * 2 {
        return Err(format!("expected {} hex chars, got {}", N * 2, s.len()));
    }
    let mut out = [0u8; N];
    for (i, byte) in out.iter_mut().enumerate() {
        *byte = u8::from_str_radix(&s[i * 2..i * 2 + 2], 16)
            .map_err(|_| "invalid hex in session id/secret".to_string())?;
    }
    Ok(out)
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::io::Write;
    use zip::write::SimpleFileOptions;

    const META: &str = r#"{"meta":{"sessionId":"000102030405060708090a0b0c0d0e0f","sessionSecret":"1011121314151617","worldId":2,"mode":"multi"}}"#;

    /// Build an in-memory `.ootmm` (a zip with meta.json at its root).
    fn make_ootmm(meta: &str) -> Vec<u8> {
        let mut buf = Vec::new();
        let opts = SimpleFileOptions::default().compression_method(zip::CompressionMethod::Stored);
        let mut zw = zip::ZipWriter::new(Cursor::new(&mut buf));
        zw.start_file("meta.json", opts).unwrap();
        zw.write_all(meta.as_bytes()).unwrap();
        zw.finish().unwrap(); // drops the returned cursor, releasing &mut buf
        buf
    }

    /// Build an outer ROM zip bundling a `.ootmm` alongside a txt / z64.
    fn make_outer_zip(ootmm: &[u8]) -> Vec<u8> {
        let mut buf = Vec::new();
        let opts = SimpleFileOptions::default().compression_method(zip::CompressionMethod::Stored);
        let mut zw = zip::ZipWriter::new(Cursor::new(&mut buf));
        zw.start_file("game.z64", opts).unwrap();
        zw.write_all(b"rom").unwrap();
        zw.start_file("log.txt", opts).unwrap();
        zw.write_all(b"log").unwrap();
        zw.start_file("game.ootmm", opts).unwrap();
        zw.write_all(ootmm).unwrap();
        zw.finish().unwrap(); // drops the returned cursor, releasing &mut buf
        buf
    }

    #[test]
    fn reads_meta_from_ootmm() {
        let info = read_meta_from_ootmm(&make_ootmm(META)).unwrap();
        assert_eq!(info.session_id, [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15]);
        assert_eq!(info.session_secret, [0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17]);
        assert_eq!(info.world_id, 2);
        assert_eq!(info.mode, PatchMode::Multi);
    }

    #[test]
    fn extracts_ootmm_from_outer_zip() {
        let outer = make_outer_zip(&make_ootmm(META));
        // The outer zip has no meta.json at its root...
        assert!(read_meta_from_ootmm(&outer).is_err());
        // ...but the inner .ootmm is found and yields the session info.
        let inner = extract_ootmm_from_zip(&outer).unwrap();
        let info = read_meta_from_ootmm(&inner).unwrap();
        assert_eq!(info.world_id, 2);
        assert_eq!(info.mode, PatchMode::Multi);
    }

    #[test]
    fn reads_dev_manifests() {
        // A dev .ootmm carries meta.json plus the three manifests.
        let items = r#"{"version":1,"items":[{"id":322,"sym":"OOT_KOKIRI_SWORD"},{"id":1,"sym":"OOT_STICK"}]}"#;
        let locs = r#"{"version":1,"locations":[{"key":22282241,"location":"Kokiri Sword Chest"}]}"#;
        let ents = r#"{"version":1,"entrances":[{"key":16,"sym":"OOT_DEKU_TREE"},{"key":17,"sym":"OOT_DODONGO_CAVERN"}]}"#;
        let mut buf = Vec::new();
        let opts = SimpleFileOptions::default().compression_method(zip::CompressionMethod::Stored);
        {
            let mut zw = zip::ZipWriter::new(Cursor::new(&mut buf));
            zw.start_file("meta.json", opts).unwrap();
            zw.write_all(META.as_bytes()).unwrap();
            zw.start_file("manifests/items.json", opts).unwrap();
            zw.write_all(items.as_bytes()).unwrap();
            zw.start_file("manifests/locations.json", opts).unwrap();
            zw.write_all(locs.as_bytes()).unwrap();
            zw.start_file("manifests/entrances.json", opts).unwrap();
            zw.write_all(ents.as_bytes()).unwrap();
            zw.finish().unwrap();
        }
        let info = read_meta_from_ootmm(&buf).unwrap();
        assert_eq!(info.world_id, 2);
        assert_eq!(info.items.get(&322).map(String::as_str), Some("OOT_KOKIRI_SWORD"));
        assert_eq!(info.items.get(&1).map(String::as_str), Some("OOT_STICK"));
        assert_eq!(info.locations.get(&22282241).map(String::as_str), Some("Kokiri Sword Chest"));
        assert_eq!(info.entrances.get(&16).map(String::as_str), Some("OOT_DEKU_TREE"));
        assert_eq!(info.entrances.get(&17).map(String::as_str), Some("OOT_DODONGO_CAVERN"));
    }

    #[test]
    fn manifests_absent_yields_empty_maps() {
        // A pre-dev patch (meta only) still loads; the manifest maps are empty.
        let info = read_meta_from_ootmm(&make_ootmm(META)).unwrap();
        assert!(info.items.is_empty());
        assert!(info.locations.is_empty());
        assert!(info.entrances.is_empty());
    }

    #[test]
    fn rejects_unknown_mode() {
        let bad = r#"{"meta":{"sessionId":"000102030405060708090a0b0c0d0e0f","sessionSecret":"1011121314151617","worldId":0,"mode":"bogus"}}"#;
        assert!(parse_meta(bad).is_err());
    }
}
