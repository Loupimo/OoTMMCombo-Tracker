//! Loading the OoTMM game patch file for the r4 multiplayer mechanism.
//!
//! In OoTMM builds newer than v32.0 the multiplayer session is described by a
//! `.ootmm` patch file, which is itself a zip archive holding a `meta.json`
//! (session id / secret, world id, game mode). This mirrors the Go client's
//! `game.ExtractGameInfo`. The user may point at the `.ootmm` directly, or at the
//! ROM `.zip` that bundles it (alongside a log `.txt` and the `.z64` ROM); both
//! are handled, since both are zip archives — the outer zip simply carries a
//! `*.ootmm` entry that is itself the patch archive.

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
/// Go client's `game.Info`). Consumed by the r4 networking layer (`multi_r4`):
/// the session id / secret authenticate the uplink handshake and are matched
/// against the game's HELLO.
#[derive(Clone)]
pub struct PatchInfo {
    pub session_id: [u8; 16],
    pub session_secret: [u8; 8],
    pub world_id: u8,
    pub mode: PatchMode,
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

/// Read `meta.json` out of a `.ootmm` archive's raw bytes.
fn read_meta_from_ootmm(bytes: &[u8]) -> Result<PatchInfo, String> {
    let mut zip = zip::ZipArchive::new(Cursor::new(bytes)).map_err(|e| e.to_string())?;
    let mut file = zip.by_name("meta.json").map_err(|e| e.to_string())?;
    let mut text = String::new();
    file.read_to_string(&mut text).map_err(|e| e.to_string())?;
    parse_meta(&text)
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
    Ok(PatchInfo { session_id, session_secret, world_id: raw.meta.world_id, mode })
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
    fn rejects_unknown_mode() {
        let bad = r#"{"meta":{"sessionId":"000102030405060708090a0b0c0d0e0f","sessionSecret":"1011121314151617","worldId":0,"mode":"bogus"}}"#;
        assert!(parse_meta(bad).is_err());
    }
}
