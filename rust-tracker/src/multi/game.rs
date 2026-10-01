//! Per-connection multiplayer state machine (port of Sources/Multi/Game.cpp).
//!
//! One `Game` owns two sockets: `api` talks to the emulator to read / write the
//! OoTMM net context in game memory, and `server` relays ledger entries to / from
//! the remote OoTMM server. Every ledger transfer (a check the local player sent,
//! or an item granted to a world) is decoded and reported to the UI thread, which
//! marks the matching object collected in the right world.

use std::net::{TcpStream, ToSocketAddrs};
use std::sync::{Arc, Mutex};
use std::time::Duration;

use super::protocol::ApiConn;
use super::sendq::{LedgerFullEntry, SendQueue};
use super::{NetItem, Reporter, TrackerNothing};
use crate::multi::buffer::NetBuffer;

const STATE_INIT: u8 = 0x00;
const STATE_CONNECT: u8 = 0x01;
const STATE_JOIN: u8 = 0x02;
const STATE_READY: u8 = 0x03;

const OP_NONE: u8 = 0x00;
const OP_TRANSFER: u8 = 0x01;
const OP_MSG: u8 = 0x02;

/// Protocol version advertised to the server (`VERSION`).
const VERSION: u32 = 0x0000_0200;

/// A short in-game chat / status message relayed between clients (`NetMsg`).
#[derive(Clone)]
struct NetMsg {
    size: u8,
    client_id: u16,
    data: [u8; 32],
}

/// One emulator + server connection and its ledger state.
pub struct Game {
    pub valid: bool,
    /// Whether the emulator link is the Ares GDB stub (drives the App's single
    /// outstanding Ares connection).
    pub is_ares: bool,
    api: ApiConn,
    server: Option<TcpStream>,
    state: u8,
    delay: i32,
    nop_acc: u32,
    timeout: u32,
    uuid: [u8; 16],
    tx: NetBuffer,
    /// Accumulator for the in-flight inbound frame (replaces the C++ rxBuffer +
    /// rxBufferSize pair). A frame parser reads exactly what it needs and clears
    /// this once the frame is complete.
    rx: Vec<u8>,
    client_id: u16,
    /// Ledger entries received from the server, indexed by the emulator's apply id.
    entries: Vec<LedgerFullEntry>,
    sendq: SendQueue,
    /// Inbound chat messages waiting for a free emulator slot.
    msgs: Vec<NetMsg>,
    net_enabled: bool,
    /// Last from / to worlds seen on a local ITEM OUT, reused to attribute
    /// hook-captured "nothing" drops (which carry no player info).
    local_from: u8,
    local_to: u8,
    reporter: Reporter,
}

impl Game {
    /// Build a game around a freshly connected emulator socket.
    pub fn new(api: ApiConn, reporter: Reporter) -> Self {
        Game {
            valid: true,
            is_ares: api.is_ares(),
            api,
            server: None,
            state: STATE_INIT,
            delay: 0,
            nop_acc: 0,
            timeout: 0,
            uuid: [0; 16],
            tx: NetBuffer::new(),
            rx: Vec::with_capacity(256),
            client_id: 0,
            entries: Vec::new(),
            sendq: SendQueue::new(),
            msgs: Vec::new(),
            net_enabled: false,
            local_from: 0,
            local_to: 0,
            reporter,
        }
    }

    fn log(&self, msg: impl Into<String>) {
        self.reporter.log(msg);
    }

    // ── Server socket lifecycle ──────────────────────────────────────────────

    fn server_close(&mut self) {
        self.server = None;
        self.timeout = 0;
        self.rx.clear();
        self.tx.clear();
    }

    fn server_reconnect(&mut self) {
        self.log("Disconnected from server, reconnecting...");
        self.server_close();
        self.state = STATE_CONNECT;
    }

    // ── Emulator API side ────────────────────────────────────────────────────

    /// Read the game UUID from memory, open the per-uuid send queue, then advance
    /// to the connect state (`gameLoadApiData`).
    fn game_load_api_data(&mut self) {
        let uuid_addr = self.api.read32(self.api.net_addr + 0x00);
        let mut uuid = [0u8; 16];
        self.api.read_buffer(uuid_addr, &mut uuid);
        if !self.api.error {
            self.uuid = uuid;
            self.sendq.open(&uuid);
            self.state = STATE_CONNECT;
        }
        if !self.net_enabled {
            self.state = STATE_READY;
        }
    }

    /// Handle a pending outgoing item: decode it, report it, and (when networking)
    /// push it to the ledger, then clear the emulator's "out" flag (`gameApiItemOut`).
    fn game_api_item_out(&mut self) {
        let item_base = self.api.net_addr + 0x0c;
        let mut buf = [0u8; 16];
        self.api.read_buffer(item_base, &mut buf);
        let player_from = buf[0];
        let player_to = buf[1];
        let game_id = buf[2];
        let key = u32::from_be_bytes([buf[4], buf[5], buf[6], buf[7]]);
        let gi = u16::from_be_bytes([buf[8], buf[9]]);
        let flags = u16::from_be_bytes([buf[10], buf[11]]);

        if self.api.error {
            return;
        }

        self.log(format!(
            "ITEM OUT - FROM: {player_from}, TO: {player_to}, GAME: {game_id}, KEY: {key:04X}, GI: {gi:04X}, FLAGS: {flags:04X}"
        ));

        self.local_from = player_from;
        self.local_to = player_to;

        self.parse_ledger_full_entry(&buf, true, player_from, player_to);
        if self.net_enabled {
            self.write_item_ledger(player_from, player_to, game_id, key, gi, flags);
            self.api.write8(self.api.net_addr + 0x08, 0x00);
        } else {
            self.api.write8(self.api.net_addr + 0x08, 0x00);
        }
    }

    /// Apply the next pending ledger entry to emulator memory and report it
    /// (`gameApiApplyLedger`).
    fn game_api_apply_ledger(&mut self) {
        let entry_id = self.api.read32(self.api.net_addr + 0x04);
        if entry_id == 0xffff_ffff || entry_id as usize >= self.entries.len() || self.api.error {
            return;
        }

        self.log(format!("LEDGER APPLY #{entry_id}"));
        let fe = self.entries[entry_id as usize].clone();
        self.api.write8(self.api.net_addr + 0x18, 0x01);
        let cmd_base = self.api.net_addr + 0x1c;

        // playerFrom, playerTo, gameId
        self.api.write_buffer(cmd_base + 0x00, &fe.data[0..3]);

        // key / gi / flags, re-reversed back to the emulator's byte order.
        let mut tmp = [0u8; 8];
        rev_copy(&mut tmp[0..4], &fe.data[4..8]);
        rev_copy(&mut tmp[4..6], &fe.data[8..10]);
        rev_copy(&mut tmp[6..8], &fe.data[10..12]);
        self.api.write_buffer(cmd_base + 0x04, &tmp);

        let player_from = fe.data[0];
        let player_to = fe.data[1];
        let game_id = fe.data[2];
        let key = u32::from_be_bytes([fe.data[4], fe.data[5], fe.data[6], fe.data[7]]);
        let gi = u16::from_be_bytes([fe.data[8], fe.data[9]]);
        let flags = u16::from_be_bytes([fe.data[10], fe.data[11]]);
        self.log(format!(
            "ITEM IN - FROM: {player_from}, TO: {player_to}, GAME: {game_id}, KEY: {key:04X}, GI: {gi:04X}, FLAGS: {flags:04X}"
        ));

        self.parse_ledger_full_entry(&fe.data, false, player_from, player_to);
    }

    /// Insert one inbound chat message into a free emulator slot (`insertMessage`).
    /// Returns false when no slot is free.
    fn insert_message(&mut self, msg: &NetMsg) -> bool {
        let mut sizes = [0u8; 32];
        self.api.read_buffer(self.api.net_addr + 0x28, &mut sizes);
        let Some(index) = sizes.iter().position(|&s| s == 0) else {
            return false;
        };
        let i = index as u32;
        self.api.write8(self.api.net_addr + 0x28 + i, msg.size);
        self.api.write16(self.api.net_addr + 0x68 + i * 2, msg.client_id);
        if msg.size > 0 {
            self.api.write_buffer(self.api.net_addr + 0xa8 + i * 32, &msg.data[..msg.size as usize]);
        }
        true
    }

    /// Push buffered inbound messages into the emulator (`gameApiProcessMessagesIn`).
    fn game_api_process_messages_in(&mut self) {
        while let Some(msg) = self.msgs.first().cloned() {
            if !self.insert_message(&msg) {
                return;
            }
            self.msgs.remove(0);
        }
    }

    /// Read outbound messages from the emulator and queue them (`gameApiProcessMessagesOut`).
    fn game_api_process_messages_out(&mut self) {
        let mut sizes = [0u8; 32];
        self.api.read_buffer(self.api.net_addr + 0x48, &mut sizes);
        for i in 0..32u32 {
            let size = sizes[i as usize];
            if size == 0 {
                continue;
            }
            let mut body = vec![0u8; size as usize];
            self.api.read_buffer(self.api.net_addr + 0xa8 + i * 32, &mut body);
            let mut data = Vec::with_capacity(size as usize + 2);
            data.push(OP_MSG);
            data.push(size);
            data.extend_from_slice(&body);
            self.tx.append(&data);
            self.api.write8(self.api.net_addr + 0x48 + i, 0x00);
        }
    }

    /// Run one emulator API tick (`gameApiTick`).
    fn game_api_tick(&mut self) {
        if self.state == STATE_INIT {
            self.game_load_api_data();
        }
        if self.state == STATE_READY {
            let op_out = self.api.read8(self.api.net_addr + 0x08);
            let op_in = self.api.read8(self.api.net_addr + 0x18);
            if self.api.error {
                return;
            }
            if op_out == 0x02 {
                self.game_api_item_out();
            }
            if self.net_enabled {
                if op_in == 0x00 {
                    self.game_api_apply_ledger();
                }
                self.game_api_process_messages_out();
                self.game_api_process_messages_in();
            }
        }
    }

    // ── Server (network) side ────────────────────────────────────────────────

    /// Accumulate at least `size` bytes of the current inbound frame (`gameProcessInputRx`).
    /// Never over-reads past `size`, so successive header / body reads stay framed.
    fn process_input_rx(&mut self, size: usize) -> bool {
        use std::io::Read;
        while self.rx.len() < size {
            let need = size - self.rx.len();
            let mut tmp = [0u8; 256];
            let n = need.min(tmp.len());
            let res = match self.server.as_mut() {
                Some(s) => s.read(&mut tmp[..n]),
                None => return false,
            };
            match res {
                Ok(0) => {
                    self.server_reconnect();
                    return false;
                }
                Ok(got) => {
                    self.rx.extend_from_slice(&tmp[..got]);
                    self.timeout = 0;
                }
                Err(e) if e.kind() == std::io::ErrorKind::WouldBlock => return false,
                Err(e) if e.kind() == std::io::ErrorKind::Interrupted => continue,
                Err(_) => return false,
            }
        }
        true
    }

    /// Parse one ledger-transfer frame, store + ack it (`gameProcessRxLedgerEntry`).
    fn process_rx_ledger_entry(&mut self) -> bool {
        if !self.process_input_rx(10) {
            return false;
        }
        let extra = self.rx[9] as usize;
        if !self.process_input_rx(10 + extra) {
            return false;
        }
        let mut fe = LedgerFullEntry::new();
        fe.key = u64::from_le_bytes(self.rx[1..9].try_into().unwrap());
        fe.size = extra as u8;
        fe.data[..extra].copy_from_slice(&self.rx[10..10 + extra]);

        if extra >= 2 {
            self.log(format!(
                "LEDGER ENTRY: {extra} bytes - from world {} to world {}",
                fe.data[0], fe.data[1]
            ));
        } else {
            self.log(format!("LEDGER ENTRY: {extra} bytes"));
        }
        self.rx.clear();

        let key = fe.key;
        self.entries.push(fe);
        self.sendq.ack(key);
        true
    }

    /// Parse one chat-message frame and buffer it (`gameProcessRxMessage`).
    fn process_rx_message(&mut self) -> bool {
        if !self.process_input_rx(4) {
            return false;
        }
        let size = self.rx[1] as usize;
        let client_id = u16::from_le_bytes([self.rx[2], self.rx[3]]);
        if !self.process_input_rx(4 + size) {
            return false;
        }
        let mut data = [0u8; 32];
        let n = size.min(32);
        data[..n].copy_from_slice(&self.rx[4..4 + n]);
        self.rx.clear();
        if self.msgs.len() < 128 {
            self.msgs.push(NetMsg { size: size as u8, client_id, data });
        }
        true
    }

    /// Dispatch inbound bytes to the matching frame parser (`gameProcessInput`).
    fn process_input(&mut self) {
        loop {
            if self.rx.is_empty() && !self.process_input_rx(1) {
                return;
            }
            match self.rx[0] {
                OP_NONE => self.rx.clear(),
                OP_TRANSFER => {
                    if !self.process_rx_ledger_entry() {
                        return;
                    }
                }
                OP_MSG => {
                    if !self.process_rx_message() {
                        return;
                    }
                }
                op => {
                    self.log(format!("Unknown opcode: {op:02x}"));
                    self.rx.clear();
                }
            }
        }
    }

    /// Send the join message (UUID + ledger base) and move to the ready state
    /// (`gameServerJoin`). Sent while the socket is still blocking so the 20-byte
    /// frame goes out atomically.
    fn server_join(&mut self) -> bool {
        use std::io::Write;
        let ledger_base = self.entries.len() as u32;
        let mut buf = [0u8; 20];
        buf[..16].copy_from_slice(&self.uuid);
        buf[16..20].copy_from_slice(&ledger_base.to_le_bytes());
        let sent = match self.server.as_mut() {
            Some(s) => s.write_all(&buf).is_ok(),
            None => false,
        };
        if sent {
            self.state = STATE_READY;
            true
        } else {
            self.log("Unable to send join message");
            self.server_reconnect();
            false
        }
    }

    /// Resolve, connect and handshake with the server, then join it (`gameServerConnect`).
    /// The connect + handshake are time-bounded (the C++ blocks indefinitely) so a
    /// down / slow server never stalls the loop or its shutdown.
    fn server_connect(&mut self, host: &str, port: u16) {
        use std::io::{Read, Write};

        let addr = (host, port).to_socket_addrs().ok().and_then(|mut it| it.next());
        let stream = addr.and_then(|a| TcpStream::connect_timeout(&a, Duration::from_secs(4)).ok());
        let mut s = match stream {
            Some(s) => s,
            None => {
                self.log(format!("Unable to connect to server at {host}:{port}"));
                self.delay = 100;
                return;
            }
        };
        let _ = s.set_nodelay(true);
        let _ = s.set_read_timeout(Some(Duration::from_secs(5)));
        let _ = s.set_write_timeout(Some(Duration::from_secs(5)));

        // Handshake (blocking, bounded): send "OOMM2" + version, expect the 11-byte reply.
        let mut hello = [0u8; 9];
        hello[..5].copy_from_slice(b"OOMM2");
        hello[5..9].copy_from_slice(&VERSION.to_le_bytes());
        let mut reply = [0u8; 11];
        if s.write_all(&hello).is_err() || s.read_exact(&mut reply).is_err() || &reply[..5] != b"OOMM2" {
            self.log(format!("Unable to connect to server at {host}:{port}"));
            self.delay = 100;
            return;
        }
        self.client_id = u16::from_le_bytes([reply[9], reply[10]]);

        self.log(format!("Connected to server at {host}:{port}"));
        self.server = Some(s);
        self.state = STATE_JOIN;
        if self.server_join() {
            // Everything above ran blocking; switch to non-blocking for the ready loop.
            if let Some(sock) = self.server.as_ref() {
                let _ = sock.set_nonblocking(true);
            }
        }
    }

    /// Run one server-side tick (`gameServerTick`).
    fn server_tick(&mut self, host: &str, port: u16) {
        if self.api.error {
            return;
        }
        if self.delay > 0 {
            self.delay -= 1;
            return;
        }
        self.timeout += 1;
        if self.timeout >= 1500 {
            self.server_reconnect();
            return;
        }

        match self.state {
            STATE_INIT | STATE_JOIN => {}
            STATE_CONNECT => self.server_connect(host, port),
            STATE_READY => {
                // NOP heartbeat when idle so the server keeps the link alive.
                if self.tx.is_empty() && self.nop_acc >= 100 {
                    self.nop_acc = 0;
                    self.tx.append(&[0x00]);
                } else {
                    self.nop_acc += 1;
                }
                self.sendq.tick(&mut self.tx);
                if let Some(sock) = self.server.as_mut() {
                    let _ = self.tx.transfer(sock, self.net_enabled);
                }
                self.process_input();
            }
            _ => {}
        }
    }

    /// Drain hook-captured "nothing" drops and push each to the ledger so the coop
    /// team receives it (`gameFlushTrackerNothings`).
    fn flush_tracker_nothings(&mut self, pending: &Arc<Mutex<Vec<TrackerNothing>>>) {
        if !self.net_enabled || self.state != STATE_READY {
            return;
        }
        let drained: Vec<TrackerNothing> = match pending.lock() {
            Ok(mut guard) if !guard.is_empty() => std::mem::take(&mut *guard),
            _ => return,
        };
        for n in drained {
            self.write_item_ledger(self.local_from, self.local_to, n.game_id, n.key, n.gi, 0);
            self.log(format!(
                "NOTHING OUT - FROM: {}, TO: {}, GAME: {}, KEY: {:04X}, GI: {:04X}",
                self.local_from, self.local_to, n.game_id, n.key, n.gi
            ));
        }
    }

    /// Run one full game tick (`gameTick`).
    pub fn tick(
        &mut self,
        net_enabled: bool,
        host: &str,
        port: u16,
        pending: &Arc<Mutex<Vec<TrackerNothing>>>,
    ) {
        self.net_enabled = net_enabled;
        if self.api.context_lock() {
            self.game_api_tick();
            self.api.context_unlock();
        }
        if net_enabled {
            self.flush_tracker_nothings(pending);
            self.server_tick(host, port);
            if self.api.error {
                self.log("Game disconnected");
                self.valid = false; // dropped next time games are pruned
            }
        }
    }

    // ── Ledger helpers ───────────────────────────────────────────────────────

    /// Build a ledger entry for an item and queue it (`writeItemLedger`).
    fn write_item_ledger(&mut self, from: u8, to: u8, game_id: u8, k: u32, gi: u16, flags: u16) {
        let mut e = LedgerFullEntry::new();
        // A counter item (flag bit 2) disambiguates duplicates by the entries count;
        // everything else uses the no-counter sentinel so it dedups naturally.
        let counter = if flags & (1 << 2) != 0 { self.entries.len() as u32 } else { 0xffff_ffff };
        e.key = item_key(k, game_id, from, counter);
        e.size = 0x10;
        e.data[0] = from;
        e.data[1] = to;
        e.data[2] = game_id;
        e.data[4..8].copy_from_slice(&k.to_le_bytes());
        e.data[8..10].copy_from_slice(&gi.to_le_bytes());
        e.data[10..12].copy_from_slice(&flags.to_le_bytes());
        self.sendq.append(&e);
    }

    /// Decode a ledger entry and report it to the UI (`ParseLedgerFullEntry`). The
    /// raw ParseKey fields are forwarded; the UI applies CorrectComboItem +
    /// FindObject and resolves the item name with the live ROM build / MQ set.
    fn parse_ledger_full_entry(&self, data: &[u8], going_out: bool, player_from: u8, player_to: u8) {
        let (key_arr, gi) = if going_out {
            (
                [data[2], data[4], data[5], data[6], data[7]],
                ((data[8] as u16) << 8) | data[9] as u16,
            )
        } else {
            (
                [data[2], data[7], data[6], data[5], data[4]],
                ((data[9] as u16) << 8) | data[8] as u16,
            )
        };

        // ParseKey: game / ov / scene / room / object. NetOut vs NetIn only differs
        // in the key byte order (handled above); the UI applies both the same way
        // (the check is marked in the "from" world either way).
        self.reporter.item(NetItem {
            game_id: key_arr[0],
            ov_type: key_arr[1],
            scene: key_arr[2] as u16,
            room: key_arr[3] as u32,
            object: key_arr[4] as u32,
            gi,
            from_world: player_from as i32,
            to_world: player_to as i32,
        });
    }
}

/// Copy `src` into `dst` in reverse byte order (`memcpy_rev`).
fn rev_copy(dst: &mut [u8], src: &[u8]) {
    let n = src.len();
    for i in 0..n {
        dst[n - 1 - i] = src[i];
    }
}

/// CRC-64 (poly 0x42f0e1eba9ea3693, MSB-first) over `data` (`Game::crc64`).
fn crc64(data: &[u8]) -> u64 {
    let mut crc: u64 = 0;
    for &b in data {
        crc ^= (b as u64) << 56;
        for _ in 0..8 {
            if crc & 0x8000_0000_0000_0000 != 0 {
                crc = (crc << 1) ^ 0x42f0_e1eb_a9ea_3693;
            } else {
                crc <<= 1;
            }
        }
    }
    crc
}

/// Build the 64-bit ledger key for an item (`itemKey`).
fn item_key(check_key: u32, game_id: u8, player_from: u8, entries_count: u32) -> u64 {
    let mut buf = [0u8; 16];
    buf[0..4].copy_from_slice(&check_key.to_le_bytes());
    buf[4] = game_id;
    buf[5] = player_from;
    buf[6..10].copy_from_slice(&entries_count.to_le_bytes());
    crc64(&buf)
}

#[cfg(test)]
mod tests {
    use super::*;

    /// CRC-64/ECMA-182 (poly 0x42f0e1eba9ea3693, init 0, no reflection): the
    /// canonical check value for "123456789". Locks the polynomial + bit order the
    /// server relies on to key ledger entries.
    #[test]
    fn crc64_check_value() {
        assert_eq!(crc64(b"123456789"), 0x6c40_df5f_0b49_7347);
    }

    /// The ledger key is deterministic in its inputs and a counter id only matters
    /// when the flag selects it (so a plain item dedups by the no-counter sentinel).
    #[test]
    fn item_key_is_deterministic() {
        let a = item_key(0x0155_0201, 0, 1, 0xffff_ffff);
        let b = item_key(0x0155_0201, 0, 1, 0xffff_ffff);
        assert_eq!(a, b);
        assert_ne!(a, item_key(0x0155_0201, 1, 1, 0xffff_ffff)); // game id matters
        assert_ne!(a, item_key(0x0155_0201, 0, 2, 0xffff_ffff)); // sender matters
    }

    /// The check-key bytes survive the write (little-endian into the entry) then
    /// the read-back reversal (NetIn), recovering the original ParseKey ordering.
    /// A "nothing" drop and an ITEM OUT that share the same raw key must resolve
    /// to the same [game, ov, scene, room, id].
    #[test]
    fn ledger_key_round_trip() {
        // Raw net-context bytes: key big-endian = [ov, scene, room, id].
        let (ov, scene, room, id) = (0x01u8, 0x55u8, 0x00u8, 0x02u8);
        let k = u32::from_be_bytes([ov, scene, room, id]);

        // writeItemLedger stores k little-endian at data[4..8].
        let mut data = [0u8; 16];
        data[2] = 0; // game id
        data[4..8].copy_from_slice(&k.to_le_bytes());

        // ParseLedgerFullEntry (NetIn) reverses [7,6,5,4] to recover ParseKey.
        let key_arr = [data[2], data[7], data[6], data[5], data[4]];
        assert_eq!(key_arr, [0, ov, scene, room, id]);
    }
}
