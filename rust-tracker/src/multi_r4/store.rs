//! Persistent WAL and send queue for the r4 multiplayer (port of
//! `shared/wal/wal.go` and `client/internal/game/send_queue.go`).
//!
//! Both are simple append-mostly files under the session data directory so a
//! session survives a tracker restart:
//!   * the WAL is the ordered, server-confirmed log — length-prefixed
//!     `[len u32 LE][entry]` records, deduplicated by the entry's 16-byte key;
//!   * the send queue holds entries produced locally but not yet acked by the
//!     server — `[id 16][len u32 LE][data]` records keyed by dedup id, retransmit
//!     until acked, truncated to empty once everything is confirmed.

use std::collections::{HashMap, HashSet};
use std::fs::{File, OpenOptions};
use std::io::{self, Read, Seek, SeekFrom, Write};
use std::path::Path;

use super::proto::WalEntry;

/// The append-only, deduplicated log of confirmed WAL entries (`wal.WAL`).
pub struct Wal {
    file: File,
    set: HashSet<[u8; 16]>,
    entries: Vec<WalEntry>,
}

impl Wal {
    /// Open (creating if needed) and load an existing WAL file.
    pub fn open(path: &Path) -> io::Result<Wal> {
        let mut file = OpenOptions::new().read(true).write(true).create(true).open(path)?;
        let mut buf = Vec::new();
        file.read_to_end(&mut buf)?;

        let mut set = HashSet::new();
        let mut entries = Vec::new();
        let mut off = 0usize;
        while off + 4 <= buf.len() {
            let len = u32::from_le_bytes([buf[off], buf[off + 1], buf[off + 2], buf[off + 3]]) as usize;
            off += 4;
            if off + len > buf.len() {
                break; // truncated tail (e.g. crash mid-write): ignore it
            }
            if let Some(entry) = WalEntry::parse(&buf[off..off + len]) {
                set.insert(entry.dedup_key());
                entries.push(entry);
            }
            off += len;
        }
        // Leave the cursor at the end so appends extend the file.
        file.seek(SeekFrom::End(0))?;
        Ok(Wal { file, set, entries })
    }

    /// Number of stored entries (`WAL.Count`).
    pub fn count(&self) -> u32 {
        self.entries.len() as u32
    }

    /// Fetch the entry at `index` (`WAL.Get`).
    pub fn get(&self, index: u32) -> Option<&WalEntry> {
        self.entries.get(index as usize)
    }

    /// Append an entry, deduplicated by its key (`WAL.Append`). Returns whether
    /// it was newly stored (so the caller can act only on genuinely new entries).
    pub fn append(&mut self, entry: &WalEntry) -> io::Result<bool> {
        let key = entry.dedup_key();
        if self.set.contains(&key) {
            return Ok(false);
        }
        let data = entry.serialize();
        self.file.seek(SeekFrom::End(0))?;
        self.file.write_all(&(data.len() as u32).to_le_bytes())?;
        self.file.write_all(&data)?;
        self.file.sync_data()?;
        self.set.insert(key);
        self.entries.push(entry.clone());
        Ok(true)
    }
}

/// Local entries awaiting server acknowledgement (`SendQueue`).
pub struct SendQueue {
    file: File,
    entries: HashMap<[u8; 16], Vec<u8>>,
}

impl SendQueue {
    /// Open (creating if needed) and load the pending send queue.
    pub fn open(path: &Path) -> io::Result<SendQueue> {
        let mut file = OpenOptions::new().read(true).write(true).create(true).open(path)?;
        let mut buf = Vec::new();
        file.read_to_end(&mut buf)?;

        let mut entries = HashMap::new();
        let mut off = 0usize;
        while off + 20 <= buf.len() {
            let mut id = [0u8; 16];
            id.copy_from_slice(&buf[off..off + 16]);
            off += 16;
            let len = u32::from_le_bytes([buf[off], buf[off + 1], buf[off + 2], buf[off + 3]]) as usize;
            off += 4;
            if off + len > buf.len() {
                break; // truncated tail
            }
            entries.insert(id, buf[off..off + len].to_vec());
            off += len;
        }
        file.seek(SeekFrom::End(0))?;
        Ok(SendQueue { file, entries })
    }

    /// Append a pending entry keyed by its dedup id (`SendQueue.Add`).
    pub fn add(&mut self, id: [u8; 16], data: &[u8]) -> io::Result<()> {
        if self.entries.contains_key(&id) {
            return Ok(());
        }
        self.file.seek(SeekFrom::End(0))?;
        self.file.write_all(&id)?;
        self.file.write_all(&(data.len() as u32).to_le_bytes())?;
        self.file.write_all(data)?;
        self.file.sync_data()?;
        self.entries.insert(id, data.to_vec());
        Ok(())
    }

    /// Drop an acknowledged entry; truncate the file once empty (`SendQueue.Ack`).
    pub fn ack(&mut self, id: &[u8; 16]) -> io::Result<()> {
        if self.entries.remove(id).is_none() {
            return Ok(());
        }
        if self.entries.is_empty() {
            self.file.set_len(0)?;
            self.file.seek(SeekFrom::Start(0))?;
        }
        self.file.sync_data()?;
        Ok(())
    }

    /// The serialized entries still awaiting acknowledgement (`SendQueue.Pending`).
    pub fn pending(&self) -> Vec<Vec<u8>> {
        self.entries.values().cloned().collect()
    }
}

#[cfg(test)]
mod tests {
    use super::super::proto::{WalEntry, WAL_ITEM};
    use super::*;

    fn sample(from: u8, key: u32) -> WalEntry {
        let mut e = WalEntry::new();
        e.wal_type = WAL_ITEM;
        e.from = from;
        e.item_to = from;
        e.item_game = 1;
        e.item_gi = 0x0042;
        e.item_key = key;
        e
    }

    /// The WAL dedups, counts, indexes, and survives a reopen from disk.
    #[test]
    fn wal_dedup_and_reload() {
        let dir = std::env::temp_dir().join(format!("r4wal-{}", std::process::id()));
        let _ = std::fs::create_dir_all(&dir);
        let path = dir.join("wal.bin");
        let _ = std::fs::remove_file(&path);

        {
            let mut wal = Wal::open(&path).unwrap();
            assert!(wal.append(&sample(0, 0x0155_0201)).unwrap());
            assert!(!wal.append(&sample(0, 0x0155_0201)).unwrap()); // duplicate
            assert!(wal.append(&sample(1, 0x0155_0202)).unwrap());
            assert_eq!(wal.count(), 2);
            assert_eq!(wal.get(0).unwrap().item_key, 0x0155_0201);
            assert_eq!(wal.get(1).unwrap().from, 1);
        }
        // Reopen: the two entries load back, and the dedup set persists.
        let mut wal = Wal::open(&path).unwrap();
        assert_eq!(wal.count(), 2);
        assert!(!wal.append(&sample(0, 0x0155_0201)).unwrap());
        let _ = std::fs::remove_file(&path);
    }

    /// The send queue stores, reloads, and clears on ack.
    #[test]
    fn send_queue_add_ack() {
        let dir = std::env::temp_dir().join(format!("r4sq-{}", std::process::id()));
        let _ = std::fs::create_dir_all(&dir);
        let path = dir.join("sq.dat");
        let _ = std::fs::remove_file(&path);

        let e = sample(0, 0x0155_0201);
        let data = e.serialize();
        let id = e.dedup_key();
        {
            let mut sq = SendQueue::open(&path).unwrap();
            sq.add(id, &data).unwrap();
            sq.add(id, &data).unwrap(); // duplicate ignored
            assert_eq!(sq.pending().len(), 1);
        }
        // Reload keeps the pending entry, then ack clears it.
        let mut sq = SendQueue::open(&path).unwrap();
        assert_eq!(sq.pending().len(), 1);
        sq.ack(&id).unwrap();
        assert_eq!(sq.pending().len(), 0);
        let mut sq2 = SendQueue::open(&path).unwrap();
        assert_eq!(sq2.pending().len(), 0);
        let _ = std::fs::remove_file(&path);
    }
}
