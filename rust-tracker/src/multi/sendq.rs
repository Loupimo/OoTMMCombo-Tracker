//! Persistent, disk-backed queue of outgoing ledger entries not yet acknowledged
//! by the server (port of `SendQueue`, Sources/Multi/Sendq.cpp).
//!
//! Each entry is retransmitted on a TTL cadence until the server echoes it back
//! (which acks and removes it). The queue is persisted under `data/<uuid>/` so
//! checks collected while the server was unreachable are re-sent on the next
//! session. The on-disk record is a fixed 137-byte layout owned by this tracker
//! (key LE + size + 128 data bytes); it never needs to interoperate with the C++
//! tracker's own file, so it drops the C struct's alignment padding.

use std::fs::{File, OpenOptions};
use std::io::{Read, Seek, SeekFrom, Write};
use std::path::PathBuf;

use super::buffer::NetBuffer;

/// Retransmission cooldown, in loop ticks (`SQ_TTL`). The App loop runs ~10 Hz,
/// so this is ~200 s between resends of an un-acked entry.
const SQ_TTL: i32 = 2000;

/// Bytes of one persisted record: key(8) + size(1) + data(128).
const RECORD_SIZE: usize = 8 + 1 + 128;

/// A complete ledger entry, keyed for deduplication (mirror of `LedgerFullEntry`).
#[derive(Clone)]
pub struct LedgerFullEntry {
    pub key: u64,
    pub size: u8,
    pub data: [u8; 128],
}

impl LedgerFullEntry {
    pub fn new() -> Self {
        LedgerFullEntry { key: 0, size: 0, data: [0; 128] }
    }

    /// Serialize into a fixed 137-byte record.
    fn to_record(&self) -> [u8; RECORD_SIZE] {
        let mut r = [0u8; RECORD_SIZE];
        r[0..8].copy_from_slice(&self.key.to_le_bytes());
        r[8] = self.size;
        r[9..9 + 128].copy_from_slice(&self.data);
        r
    }

    /// Parse a fixed 137-byte record.
    fn from_record(r: &[u8; RECORD_SIZE]) -> Self {
        let mut key = [0u8; 8];
        key.copy_from_slice(&r[0..8]);
        let mut data = [0u8; 128];
        data.copy_from_slice(&r[9..9 + 128]);
        LedgerFullEntry { key: u64::from_le_bytes(key), size: r[8], data }
    }
}

/// One queued entry with its remaining retransmission cooldown.
struct SendQueueEntry {
    entry: LedgerFullEntry,
    ttl: i32,
}

/// The persistent send queue: an in-memory list mirrored to a backing file.
#[derive(Default)]
pub struct SendQueue {
    file: Option<File>,
    data: Vec<SendQueueEntry>,
}

impl SendQueue {
    pub fn new() -> Self {
        SendQueue::default()
    }

    /// Open (or create) the per-uuid queue file and load its entries into memory
    /// (`sendqOpen`). Any previously opened file is closed first.
    pub fn open(&mut self, uuid: &[u8; 16]) {
        self.close();

        let mut dir = PathBuf::from("data");
        let mut hex = String::with_capacity(32);
        for b in uuid {
            hex.push_str(&format!("{b:02x}"));
        }
        dir.push(hex);
        let _ = std::fs::create_dir_all(&dir);
        let path = dir.join("sendq.bin");

        let Ok(mut file) = OpenOptions::new().read(true).write(true).create(true).open(&path) else {
            return;
        };

        // Read all persisted records.
        let mut bytes = Vec::new();
        if file.read_to_end(&mut bytes).is_ok() {
            for chunk in bytes.chunks_exact(RECORD_SIZE) {
                let mut rec = [0u8; RECORD_SIZE];
                rec.copy_from_slice(chunk);
                self.data.push(SendQueueEntry { entry: LedgerFullEntry::from_record(&rec), ttl: 0 });
            }
        }
        self.file = Some(file);
    }

    /// Close the backing file and drop the in-memory entries (`sendqClose`).
    pub fn close(&mut self) {
        self.file = None; // dropping the File closes it
        self.data.clear();
    }

    /// Index of the entry with the given key, if any (`sendqLocate`).
    fn locate(&self, key: u64) -> Option<usize> {
        self.data.iter().position(|e| e.entry.key == key)
    }

    /// Append an entry (in memory and on disk) unless its key is already queued
    /// (`sendqWrite` / `sendqAppend`). Fresh entries get TTL 0 so they transmit on
    /// the next tick.
    pub fn append(&mut self, entry: &LedgerFullEntry) {
        if self.locate(entry.key).is_some() {
            return;
        }
        let id = self.data.len();
        self.data.push(SendQueueEntry { entry: entry.clone(), ttl: 0 });
        if let Some(file) = self.file.as_mut() {
            let _ = file.seek(SeekFrom::Start((id * RECORD_SIZE) as u64));
            let _ = file.write_all(&entry.to_record());
            let _ = file.flush();
        }
    }

    /// Transmit every entry whose TTL has expired, arming its cooldown (`sendqTick`).
    pub fn tick(&mut self, nb: &mut NetBuffer) {
        for e in &mut self.data {
            if e.ttl > 0 {
                e.ttl -= 1;
                continue;
            }
            // Serialize as an OP_TRANSFER packet: 0x01 | key(8 LE) | size(1) | data.
            let mut pkt = Vec::with_capacity(1 + 8 + 1 + e.entry.size as usize);
            pkt.push(0x01);
            pkt.extend_from_slice(&e.entry.key.to_le_bytes());
            pkt.push(e.entry.size);
            pkt.extend_from_slice(&e.entry.data[..e.entry.size as usize]);
            nb.append(&pkt);
            e.ttl = SQ_TTL;
        }
    }

    /// Acknowledge a delivered entry by key: remove it (swap the last entry into
    /// its slot) and truncate the backing file (`sendqAck`).
    pub fn ack(&mut self, key: u64) {
        let Some(id) = self.locate(key) else { return };
        let last = self.data.len() - 1;
        if id != last {
            self.data.swap(id, last);
            if let Some(file) = self.file.as_mut() {
                let _ = file.seek(SeekFrom::Start((id * RECORD_SIZE) as u64));
                let _ = file.write_all(&self.data[id].entry.to_record());
                let _ = file.flush();
            }
        }
        self.data.pop();
        if let Some(file) = self.file.as_mut() {
            let _ = file.set_len((self.data.len() * RECORD_SIZE) as u64);
            let _ = file.flush();
        }
    }
}
