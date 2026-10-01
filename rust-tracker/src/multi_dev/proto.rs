//! Wire types for the dev multiplayer (port of Sources multiplayer-dev
//! `client/internal/ipc/msg.go`, `shared/protocol`, `shared/wal`).
//!
//! Two protocols meet here:
//!   * the IPC protocol spoken to the emulator over a named pipe — big-endian
//!     framing, `[Seq u32][Op u8][payload]`;
//!   * the uplink protocol spoken to the OoTMM server over TCP — little-endian,
//!     `[size u16][op u8][data]`.
//! The persistent WAL entry sits between them (its own little-endian layout). The
//! endianness differs field by field, so each accessor is explicit.

/// The magic every HELLO carries (`"OoTMM\x7f\x01\x00"`).
pub const MAGIC: [u8; 8] = [b'O', b'o', b'T', b'M', b'M', 0x7f, 0x01, 0x00];
/// Uplink protocol version advertised in the client hello.
pub const UPLINK_VERSION: u32 = 0x0001_0000;

// ── IPC opcodes (pipe) ───────────────────────────────────────────────────────
pub mod ipc_op {
    pub const HELLO: u8 = 0x01;
    pub const WAL: u8 = 0x02;
    pub const WAL_QUERY: u8 = 0x03;
    pub const WAL_ACK: u8 = 0x04;
    pub const POSITION: u8 = 0x05;
    /// dev: informational item preview (game -> client), resolved against the
    /// patch's item / location manifests for a human-readable journal line.
    pub const INFO_ITEM: u8 = 0x06;
    /// dev: informational entrance transition (game -> client): the entrance the
    /// player arrived at plus the original (pre-shuffle) doorway and the age,
    /// resolved against the patch's entrance manifest.
    pub const INFO_ENTRANCE: u8 = 0x07;
}

// ── Uplink opcodes (TCP) ─────────────────────────────────────────────────────
pub mod up_op {
    pub const NOP: u8 = 0x00;
    pub const HELLO: u8 = 0x01;
    pub const WAL: u8 = 0x02;
    pub const WAL_ACK: u8 = 0x03;
    pub const POSITION: u8 = 0x04;
}

// ── WAL entry types ──────────────────────────────────────────────────────────
pub const WAL_ITEM: u8 = 0x01;
pub const WAL_EVENT: u8 = 0x02;

/// One decoded IPC frame from the pipe.
pub struct IpcMessage {
    pub seq: u32,
    pub op: u8,
    pub payload: Vec<u8>,
}

impl IpcMessage {
    /// Parse `[Seq u32 BE][Op u8][payload]`.
    pub fn parse(data: &[u8]) -> Option<IpcMessage> {
        if data.len() < 5 {
            return None;
        }
        Some(IpcMessage {
            seq: u32::from_be_bytes([data[0], data[1], data[2], data[3]]),
            op: data[4],
            payload: data[5..].to_vec(),
        })
    }

    /// Serialize with the given sequence number.
    pub fn serialize(seq: u32, op: u8, payload: &[u8]) -> Vec<u8> {
        let mut out = Vec::with_capacity(5 + payload.len());
        out.extend_from_slice(&seq.to_be_bytes());
        out.push(op);
        out.extend_from_slice(payload);
        out
    }
}

/// The game's HELLO_IN body (57 bytes).
pub struct HelloIn {
    pub magic: [u8; 8],
    pub session_id: [u8; 16],
    pub session_secret: [u8; 8],
    pub player_id: [u8; 16],
    pub player_name: [u8; 8],
    pub world_id: u8,
}

impl HelloIn {
    pub fn parse(data: &[u8]) -> Option<HelloIn> {
        if data.len() < 57 {
            return None;
        }
        let mut h = HelloIn {
            magic: [0; 8],
            session_id: [0; 16],
            session_secret: [0; 8],
            player_id: [0; 16],
            player_name: [0; 8],
            world_id: data[56],
        };
        h.magic.copy_from_slice(&data[0..8]);
        h.session_id.copy_from_slice(&data[8..24]);
        h.session_secret.copy_from_slice(&data[24..32]);
        h.player_id.copy_from_slice(&data[32..48]);
        h.player_name.copy_from_slice(&data[48..56]);
        Some(h)
    }
}

/// Serialize the HELLO_OUT body sent back to the game (16 bytes).
pub fn hello_out(seq_game: u32, seq_net: u32) -> Vec<u8> {
    let mut out = Vec::with_capacity(16);
    out.extend_from_slice(&MAGIC);
    out.extend_from_slice(&seq_game.to_be_bytes());
    out.extend_from_slice(&seq_net.to_be_bytes());
    out
}

/// A game→client WAL_IN body: a token to ack plus the typed entry data.
pub struct WalIn {
    pub token: u32,
    pub wal_type: u8,
    pub data: Vec<u8>,
}

impl WalIn {
    pub fn parse(payload: &[u8]) -> Option<WalIn> {
        if payload.len() < 5 {
            return None;
        }
        Some(WalIn {
            token: u32::from_be_bytes([payload[0], payload[1], payload[2], payload[3]]),
            wal_type: payload[4],
            data: payload[5..].to_vec(),
        })
    }
}

/// The game's INFO_ITEM body (`MessageBodyInfoItem`, big-endian): a location key,
/// the item id (gi) placed there, and (dev) a flags byte whose bit 0 (`0x01`) is
/// "self" — the item belongs to the local player. Purely informational — the game
/// announces what an item is / where it lives, not a network pickup — so it is
/// resolved against the patch manifests and logged, never written to the WAL.
pub struct InfoItem {
    pub key: u32,
    pub gi: u16,
    pub flags: u8,
}

impl InfoItem {
    /// "Self" bit: the collected item belongs to the local player.
    pub const FLAG_SELF: u8 = 0x01;

    /// Parse `[Key u32 BE][GI u16 BE][Flags u8?]`. The flags byte is optional (a
    /// pre-dev game omits it); when absent it defaults to `FLAG_SELF`, matching the
    /// Go client's fallback.
    pub fn parse(data: &[u8]) -> Option<InfoItem> {
        if data.len() < 6 {
            return None;
        }
        Some(InfoItem {
            key: u32::from_be_bytes([data[0], data[1], data[2], data[3]]),
            gi: u16::from_be_bytes([data[4], data[5]]),
            flags: if data.len() >= 7 { data[6] } else { Self::FLAG_SELF },
        })
    }

    /// Whether the collected item is the local player's own.
    pub fn is_self(&self) -> bool {
        self.flags & Self::FLAG_SELF != 0
    }
}

/// The game's INFO_ENTRANCE body (`MessageBodyInfoEntrance`, 9 bytes, big-endian):
/// the `entrance` the player arrived at, the `original` (pre-shuffle) doorway, and
/// the `age` (0 = adult, 1 = child). `original` is `0xFFFFFFFF` on respawns / age
/// swaps (no source doorway); `entrance == original` when the entrance is not
/// shuffled. Both are keys into the patch's entrance manifest.
pub struct InfoEntrance {
    pub original: u32,
    pub entrance: u32,
    pub age: u8,
}

impl InfoEntrance {
    /// The sentinel `original` meaning "no source doorway" (respawn / age swap).
    pub const NO_ORIGINAL: u32 = 0xFFFF_FFFF;

    /// Parse `[Original u32 BE][Entrance u32 BE][Age u8]`.
    pub fn parse(data: &[u8]) -> Option<InfoEntrance> {
        if data.len() < 9 {
            return None;
        }
        Some(InfoEntrance {
            original: u32::from_be_bytes([data[0], data[1], data[2], data[3]]),
            entrance: u32::from_be_bytes([data[4], data[5], data[6], data[7]]),
            age: data[8],
        })
    }
}

/// An item as it crosses the IPC boundary (big-endian gi / flags / key).
#[derive(Clone, Copy)]
pub struct WalItem {
    pub to: u8,
    /// The wire `unused` byte (`MultiPacketWalItemOut.unused`), always 0 — NOT the
    /// game. The game (OoT / MM) lives in bit 31 of `key`; do not resolve it from
    /// this field. Kept only to round-trip the byte back to the game verbatim.
    pub game: u8,
    pub gi: u16,
    pub flags: u16,
    pub key: u32,
}

impl WalItem {
    pub fn parse(data: &[u8]) -> Option<WalItem> {
        if data.len() < 10 {
            return None;
        }
        Some(WalItem {
            to: data[0],
            game: data[1],
            gi: u16::from_be_bytes([data[2], data[3]]),
            flags: u16::from_be_bytes([data[4], data[5]]),
            key: u32::from_be_bytes([data[6], data[7], data[8], data[9]]),
        })
    }

    /// Serialize back for a WAL_OUT to the game (big-endian).
    pub fn serialize(&self) -> Vec<u8> {
        let mut out = Vec::with_capacity(10);
        out.push(self.to);
        out.push(self.game);
        out.extend_from_slice(&self.gi.to_be_bytes());
        out.extend_from_slice(&self.flags.to_be_bytes());
        out.extend_from_slice(&self.key.to_be_bytes());
        out
    }
}

/// Serialize a WAL_OUT frame sent to the game (`MessageBodyWalOut`).
pub fn wal_out(index: u32, wal_type: u8, from: u8, player_name: &[u8; 8], data: &[u8]) -> Vec<u8> {
    let mut out = Vec::with_capacity(14 + data.len());
    out.extend_from_slice(&index.to_be_bytes());
    out.push(wal_type);
    out.push(from);
    out.extend_from_slice(player_name);
    out.extend_from_slice(data);
    out
}

/// A persistent WAL entry (`shared/wal` — little-endian item fields).
#[derive(Clone)]
pub struct WalEntry {
    pub wal_type: u8,
    pub player_id: [u8; 16],
    pub player_name: [u8; 8],
    pub from: u8,
    // Item fields (valid when wal_type == WAL_ITEM).
    pub item_to: u8,
    pub item_game: u8,
    pub item_gi: u16,
    pub item_flags: u16,
    pub item_key: u32,
    pub item_nonce: u32,
    // Event field (valid when wal_type == WAL_EVENT).
    pub event_id: u32,
}

impl WalEntry {
    pub fn new() -> WalEntry {
        WalEntry {
            wal_type: 0,
            player_id: [0; 16],
            player_name: [0; 8],
            from: 0,
            item_to: 0,
            item_game: 0,
            item_gi: 0,
            item_flags: 0,
            item_key: 0,
            item_nonce: 0,
            event_id: 0,
        }
    }

    /// Parse a serialized WAL entry (little-endian item fields).
    pub fn parse(data: &[u8]) -> Option<WalEntry> {
        if data.len() < 26 {
            return None;
        }
        let mut e = WalEntry::new();
        e.wal_type = data[0];
        e.player_id.copy_from_slice(&data[1..17]);
        e.player_name.copy_from_slice(&data[17..25]);
        e.from = data[25];
        match e.wal_type {
            WAL_ITEM => {
                if data.len() < 40 {
                    return None;
                }
                e.item_to = data[26];
                e.item_game = data[27];
                e.item_gi = u16::from_le_bytes([data[28], data[29]]);
                e.item_flags = u16::from_le_bytes([data[30], data[31]]);
                e.item_key = u32::from_le_bytes([data[32], data[33], data[34], data[35]]);
                e.item_nonce = u32::from_le_bytes([data[36], data[37], data[38], data[39]]);
            }
            WAL_EVENT => {
                if data.len() < 30 {
                    return None;
                }
                e.event_id = u32::from_le_bytes([data[26], data[27], data[28], data[29]]);
            }
            _ => return None,
        }
        Some(e)
    }

    /// Serialize the WAL entry (little-endian item fields).
    pub fn serialize(&self) -> Vec<u8> {
        let mut out = Vec::with_capacity(40);
        out.push(self.wal_type);
        out.extend_from_slice(&self.player_id);
        out.extend_from_slice(&self.player_name);
        out.push(self.from);
        match self.wal_type {
            WAL_ITEM => {
                out.push(self.item_to);
                out.push(self.item_game);
                out.extend_from_slice(&self.item_gi.to_le_bytes());
                out.extend_from_slice(&self.item_flags.to_le_bytes());
                out.extend_from_slice(&self.item_key.to_le_bytes());
                out.extend_from_slice(&self.item_nonce.to_le_bytes());
            }
            WAL_EVENT => {
                out.extend_from_slice(&self.event_id.to_le_bytes());
            }
            _ => {}
        }
        out
    }

    /// The 16-byte deduplication key (`WalEntry.DedupKey`).
    pub fn dedup_key(&self) -> [u8; 16] {
        let mut key = [0u8; 16];
        key[0] = self.wal_type;
        key[1] = self.from;
        match self.wal_type {
            WAL_ITEM => {
                key[2] = self.item_game;
                key[3..7].copy_from_slice(&self.item_key.to_le_bytes());
                key[7..11].copy_from_slice(&self.item_nonce.to_le_bytes());
            }
            WAL_EVENT => {
                key[2..6].copy_from_slice(&self.event_id.to_le_bytes());
            }
            _ => {}
        }
        key
    }
}

/// Serialize the uplink client hello (`protocol.ClientHello`, 65 bytes, LE).
#[allow(clippy::too_many_arguments)]
pub fn client_hello(
    session_id: &[u8; 16],
    session_secret: &[u8; 8],
    player_id: &[u8; 16],
    player_name: &[u8; 8],
    world_id: u8,
    wal_index: u32,
) -> Vec<u8> {
    let mut out = Vec::with_capacity(65);
    out.extend_from_slice(&MAGIC);
    out.extend_from_slice(&UPLINK_VERSION.to_le_bytes());
    out.extend_from_slice(session_id);
    out.extend_from_slice(session_secret);
    out.extend_from_slice(player_id);
    out.extend_from_slice(player_name);
    out.push(world_id);
    out.extend_from_slice(&wal_index.to_le_bytes());
    out
}

/// Parse a server WAL packet: `[index u32 LE][wal entry]` (`protocol.ServerWal`).
pub fn parse_server_wal(data: &[u8]) -> Option<(u32, WalEntry)> {
    if data.len() < 4 {
        return None;
    }
    let index = u32::from_le_bytes([data[0], data[1], data[2], data[3]]);
    let entry = WalEntry::parse(&data[4..])?;
    Some((index, entry))
}

#[cfg(test)]
mod tests {
    use super::*;

    /// A WAL item survives serialize -> parse unchanged, and its dedup key is
    /// stable (the send queue / WAL rely on it to drop duplicates).
    #[test]
    fn wal_entry_round_trip() {
        let mut e = WalEntry::new();
        e.wal_type = WAL_ITEM;
        e.player_id = [7; 16];
        e.player_name = *b"ALICE\0\0\0";
        e.from = 3;
        e.item_to = 5;
        e.item_game = 1;
        e.item_gi = 0x0142;
        e.item_flags = 0x0001;
        e.item_key = 0x0155_0201;
        e.item_nonce = 0xdead_beef;
        let bytes = e.serialize();
        assert_eq!(bytes.len(), 40);
        let back = WalEntry::parse(&bytes).unwrap();
        assert_eq!(back.item_key, e.item_key);
        assert_eq!(back.item_gi, e.item_gi);
        assert_eq!(back.item_nonce, e.item_nonce);
        assert_eq!(back.from, 3);
        assert_eq!(e.dedup_key(), back.dedup_key());
    }

    /// The IPC item key is big-endian; the WAL entry stores it little-endian. A
    /// key round-trips through both without swapping.
    #[test]
    fn ipc_item_key_is_big_endian() {
        let raw = [5u8, 1, 0x01, 0x42, 0x00, 0x01, 0x01, 0x55, 0x02, 0x01];
        let item = WalItem::parse(&raw).unwrap();
        assert_eq!(item.to, 5);
        assert_eq!(item.game, 1);
        assert_eq!(item.gi, 0x0142);
        assert_eq!(item.flags, 0x0001);
        assert_eq!(item.key, 0x0155_0201);
        assert_eq!(item.serialize(), raw);
    }

    /// The server WAL packet is index (LE) + entry.
    #[test]
    fn server_wal_frames() {
        let mut e = WalEntry::new();
        e.wal_type = WAL_EVENT;
        e.event_id = 0x1234_5678;
        let mut data = 42u32.to_le_bytes().to_vec();
        data.extend_from_slice(&e.serialize());
        let (index, back) = parse_server_wal(&data).unwrap();
        assert_eq!(index, 42);
        assert_eq!(back.event_id, 0x1234_5678);
    }

    /// An INFO_ITEM body is big-endian key then gi, with an optional flags byte;
    /// a short body is rejected and a missing flags byte defaults to "self".
    #[test]
    fn info_item_parse() {
        let raw = [0x01u8, 0x55, 0x02, 0x01, 0x01, 0x42];
        let info = InfoItem::parse(&raw).unwrap();
        assert_eq!(info.key, 0x0155_0201);
        assert_eq!(info.gi, 0x0142);
        assert!(info.is_self()); // 6-byte body: flags default to FLAG_SELF
        assert!(InfoItem::parse(&raw[..5]).is_none());

        // 7-byte body: flags byte is honoured (0x00 -> not self).
        let raw7 = [0x01u8, 0x55, 0x02, 0x01, 0x01, 0x42, 0x00];
        let info7 = InfoItem::parse(&raw7).unwrap();
        assert!(!info7.is_self());
    }

    /// An INFO_ENTRANCE body is original, entrance (both BE u32) then the age byte;
    /// a short body is rejected.
    #[test]
    fn info_entrance_parse() {
        let raw = [0xFFu8, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x01, 0x23, 0x01];
        let e = InfoEntrance::parse(&raw).unwrap();
        assert_eq!(e.original, InfoEntrance::NO_ORIGINAL);
        assert_eq!(e.entrance, 0x0000_0123);
        assert_eq!(e.age, 1);
        assert!(InfoEntrance::parse(&raw[..8]).is_none());
    }
}
