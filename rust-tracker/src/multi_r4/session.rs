//! The r4 session: one connected game + its server uplink (port of
//! `client/internal/game/session.go` and `uplink.go`).
//!
//! Threading (the Go build uses goroutines + channels; we use OS threads + mpsc):
//!   * a **pipe reader** thread turns pipe messages into `IpcMessage`s;
//!   * an **uplink manager** thread owns the TCP connection: it (re)connects,
//!     handshakes, sends the NOP heartbeat, forwards outgoing packets, and spawns
//!     a short-lived **socket reader** sub-thread per connection;
//!   * the **session main loop** (this thread) owns the WAL and send queue with no
//!     locking, draining both channels and driving the 10 s retransmit timer.
//!
//! The WAL is the server-confirmed order of record. Local pickups go to the send
//! queue + uplink and are applied to the map immediately; the server echoes them
//! back into the WAL (idempotent on the tracker's collected set).

use std::io::{Read, Write};
use std::net::{TcpStream, ToSocketAddrs};
use std::path::PathBuf;
use std::sync::atomic::{AtomicBool, AtomicU32, Ordering};
use std::sync::mpsc::{self, Receiver, Sender, TryRecvError};
use std::sync::Arc;
use std::thread;
use std::time::{Duration, Instant};

use crate::multi::NetItem;
use crate::patch::{PatchInfo, PatchMode};

use super::pipe::PipeConn;
use super::proto::*;
use super::store::{SendQueue, Wal};
use super::{rand_u32, R4Config, Reporter};

/// Build the UI-side `NetItem` from an r4 WAL item. The 32-bit key packs the
/// ParseKey (`ov<<24 | scene<<16 | room<<8 | id`); worlds are 0-based here but
/// 1-based for `apply_net_item` (world N -> index N-1), so both are shifted by 1.
fn net_item_from(item: &WalItem, from: u8, to: u8) -> NetItem {
    let key = item.key;
    NetItem {
        game_id: item.game,
        ov_type: ((key >> 24) & 0xFF) as u8,
        scene: ((key >> 16) & 0xFF) as u16,
        room: ((key >> 8) & 0xFF) as u32,
        object: (key & 0xFF) as u32,
        gi: item.gi,
        from_world: from as i32 + 1,
        to_world: to as i32 + 1,
    }
}

/// Per-session state owned by the main loop (single-threaded, no locks).
struct Session {
    conn: Arc<PipeConn>,
    reporter: Reporter,
    world_id: u8,
    single: bool,
    player_id: [u8; 16],
    player_name: [u8; 8],
    seq_game: u32,
    seq_net: u32,
    wal: Wal,
    wal_count: Arc<AtomicU32>,
    sendq: Option<SendQueue>,
    uplink_out: Option<Sender<(u8, Vec<u8>)>>,
}

impl Session {
    /// Write an outbound IPC message, stamping the next net sequence number
    /// (`handleMsgOut`). HELLO_OUT is sent separately with seq 0.
    fn send(&mut self, op: u8, payload: &[u8]) {
        let data = IpcMessage::serialize(self.seq_net, op, payload);
        self.seq_net = self.seq_net.wrapping_add(1);
        let _ = self.conn.write(&data);
    }

    /// Report a WAL item to the tracker map.
    fn report_item(&self, item: &WalItem, from: u8, to: u8) {
        self.reporter.item(net_item_from(item, from, to));
    }

    /// Dispatch one message from the game (`handleMsg`). Returns Err on a protocol
    /// desync, which ends the session (discovery then re-connects).
    fn handle_msg(&mut self, msg: IpcMessage) -> Result<(), String> {
        let expected = if msg.op == ipc_op::HELLO {
            0
        } else {
            let e = self.seq_game;
            self.seq_game = self.seq_game.wrapping_add(1);
            e
        };
        if msg.seq != expected {
            return Err(format!("unexpected sequence: got {}, expected {expected}", msg.seq));
        }
        match msg.op {
            ipc_op::HELLO => Err("unexpected HELLO from game".to_string()),
            ipc_op::WAL => {
                let w = WalIn::parse(&msg.payload).ok_or("WAL_IN too short")?;
                if !self.single {
                    self.handle_wal_in(&w)?;
                }
                Ok(())
            }
            ipc_op::WAL_QUERY => {
                if msg.payload.len() < 4 {
                    return Err("WAL_QUERY too short".to_string());
                }
                let index = u32::from_be_bytes([msg.payload[0], msg.payload[1], msg.payload[2], msg.payload[3]]);
                self.send_game_wal(index);
                Ok(())
            }
            ipc_op::POSITION => Ok(()), // position system deferred
            other => Err(format!("unhandled IPC opcode: {other}")),
        }
    }

    /// A local pickup from the game (`handleWalIn`): apply it to the map, enqueue
    /// it for the uplink, and ACK the game.
    fn handle_wal_in(&mut self, w: &WalIn) -> Result<(), String> {
        let mut entry = WalEntry::new();
        entry.player_id = self.player_id;
        entry.player_name = self.player_name;
        entry.from = self.world_id;

        match w.wal_type {
            WAL_ITEM => {
                let item = WalItem::parse(&w.data).ok_or("WAL item too short")?;
                entry.wal_type = WAL_ITEM;
                entry.item_to = item.to;
                entry.item_game = item.game;
                entry.item_gi = item.gi;
                entry.item_flags = item.flags;
                entry.item_key = item.key;
                // 0x0001 == OVF_RENEW: a fresh nonce so a re-obtainable item is a
                // distinct WAL entry each time.
                entry.item_nonce = if item.flags & 0x0001 != 0 { rand_u32() } else { 0 };
                self.reporter.log(format!(
                    "r4: local item — game {} key {:08x} gi {:04x} -> world {}",
                    item.game, item.key, item.gi, item.to
                ));
                self.report_item(&item, self.world_id, item.to);
            }
            WAL_EVENT => {
                if w.data.len() < 4 {
                    return Err("WAL event too short".to_string());
                }
                entry.wal_type = WAL_EVENT;
                entry.event_id = u32::from_be_bytes([w.data[0], w.data[1], w.data[2], w.data[3]]);
            }
            other => return Err(format!("unhandled WAL type: {other}")),
        }

        self.new_wal_entry(&entry);

        // ACK the game with the token it sent (big-endian), so it can retire it.
        self.send(ipc_op::WAL_ACK, &w.token.to_be_bytes());
        Ok(())
    }

    /// Persist a locally produced entry to the send queue and push it to the
    /// uplink without waiting for the retransmit tick (`newWalEntry`).
    fn new_wal_entry(&mut self, entry: &WalEntry) {
        let data = entry.serialize();
        let dedup = entry.dedup_key();
        if let Some(sq) = &mut self.sendq {
            let _ = sq.add(dedup, &data);
        }
        if let Some(tx) = &self.uplink_out {
            let _ = tx.send((up_op::WAL, data));
        }
    }

    /// Serve a WAL_QUERY: stream up to 16 entries from `index` to the game
    /// (`sendGameWal`). The player name is sent only for other players' entries.
    fn send_game_wal(&mut self, index: u32) {
        for i in 0..16u32 {
            let Some(entry) = self.wal.get(index + i).cloned() else {
                return;
            };
            let mut player_name = [0u8; 8];
            if entry.player_id != self.player_id {
                player_name = entry.player_name;
            }
            let data = match entry.wal_type {
                WAL_ITEM => WalItem {
                    to: entry.item_to,
                    game: entry.item_game,
                    gi: entry.item_gi,
                    flags: entry.item_flags,
                    key: entry.item_key,
                }
                .serialize(),
                WAL_EVENT => entry.event_id.to_be_bytes().to_vec(),
                _ => continue,
            };
            let payload = wal_out(index + i, entry.wal_type, entry.from, &player_name, &data);
            self.send(ipc_op::WAL, &payload);
        }
    }

    /// Handle one packet from the server uplink (`handleUplinkPacket`).
    fn handle_uplink_packet(&mut self, op: u8, data: Vec<u8>) {
        match op {
            up_op::NOP => {}
            up_op::WAL => {
                let Some((_index, entry)) = parse_server_wal(&data) else {
                    return;
                };
                let newly = self.wal.append(&entry).unwrap_or(false);
                self.wal_count.store(self.wal.count(), Ordering::Relaxed);
                let dedup = entry.dedup_key();
                if let Some(sq) = &mut self.sendq {
                    let _ = sq.ack(&dedup);
                }
                // Apply genuinely new item entries to the map (events have no
                // placement to mark). Duplicates were already applied.
                if newly && entry.wal_type == WAL_ITEM {
                    let item = WalItem {
                        to: entry.item_to,
                        game: entry.item_game,
                        gi: entry.item_gi,
                        flags: entry.item_flags,
                        key: entry.item_key,
                    };
                    self.reporter.log(format!(
                        "r4: server item — world {} -> {} game {} key {:08x}",
                        entry.from, entry.item_to, entry.item_game, entry.item_key
                    ));
                    self.report_item(&item, entry.from, entry.item_to);
                }
            }
            up_op::WAL_ACK => {
                if data.len() == 16 {
                    let mut key = [0u8; 16];
                    key.copy_from_slice(&data);
                    if let Some(sq) = &mut self.sendq {
                        let _ = sq.ack(&key);
                    }
                }
            }
            up_op::POSITION => {} // position system deferred
            _ => {}
        }
    }
}

/// Hex of a byte slice (lower-case), for the per-session data directory.
fn hex(bytes: &[u8]) -> String {
    let mut s = String::with_capacity(bytes.len() * 2);
    for b in bytes {
        s.push_str(&format!("{b:02x}"));
    }
    s
}

/// Why a session ended. The discovery loop uses this to deduplicate transient
/// log lines: a game that doesn't speak the IPC (older build) or whose session
/// differs from the loaded patch would otherwise spam the journal every retry.
/// While such a session is not validated, `connected` stays false, so the DLL
/// hook keeps ownership of items (the requested fallback).
pub(super) enum SessionEnd {
    /// Opened a pipe but no valid HELLO arrived — an older emulator build without
    /// the r4 IPC, or a pipe that isn't an OoTMM game.
    NoHello,
    /// HELLO arrived but the game's session / world differs from the patch.
    Mismatch(&'static str),
    /// A validated session ran and then ended (game disconnected or tracker stop).
    Ended,
    /// A local setup error (filesystem / WAL) prevented the session.
    Error(String),
}

/// Run a session for a connected pipe: read HELLO, validate, then loop until the
/// game disconnects or the tracker stops (`game.Run`). `connected` is set true
/// only for a validated, matching game and cleared when the session ends.
pub(super) fn run_session(
    cfg: &R4Config,
    info: &PatchInfo,
    running: &Arc<AtomicBool>,
    reporter: &Reporter,
    connected: &Arc<AtomicBool>,
    conn: Arc<PipeConn>,
) -> SessionEnd {
    // --- Pipe reader thread: pipe messages -> msg_in --------------------------
    let (msg_tx, msg_rx) = mpsc::channel::<IpcMessage>();
    let reader_conn = conn.clone();
    let reader_join = thread::spawn(move || {
        while let Ok(bytes) = reader_conn.read() {
            if let Some(msg) = IpcMessage::parse(&bytes) {
                if msg_tx.send(msg).is_err() {
                    break; // main loop gone
                }
            }
        }
    });

    // Tear down the reader thread (used by every early exit before the main loop).
    // Written inline rather than as a closure since the success path also needs
    // `reader_join` for its own teardown; each early return diverges, so moving it
    // here is fine.
    macro_rules! bail {
        ($end:expr) => {{
            conn.cancel();
            let _ = reader_join.join();
            return $end;
        }};
    }

    // --- HELLO handshake ------------------------------------------------------
    let Ok(hello) = msg_rx.recv_timeout(Duration::from_secs(5)) else {
        bail!(SessionEnd::NoHello); // no game spoke: older build / not ours
    };
    if hello.op != ipc_op::HELLO || hello.seq != 0 {
        bail!(SessionEnd::NoHello);
    }
    let Some(h) = HelloIn::parse(&hello.payload) else {
        bail!(SessionEnd::NoHello);
    };
    if h.magic != MAGIC {
        bail!(SessionEnd::NoHello);
    }
    if h.session_id != info.session_id || h.session_secret != info.session_secret {
        bail!(SessionEnd::Mismatch("session"));
    }
    if h.world_id != info.world_id {
        bail!(SessionEnd::Mismatch("world"));
    }

    // --- Per-session data directory + persistent stores -----------------------
    let session_dir: PathBuf = cfg
        .data_dir
        .join("sessions")
        .join(hex(&info.session_id[0..1]))
        .join(hex(&info.session_id[1..16]));
    if let Err(e) = std::fs::create_dir_all(&session_dir) {
        bail!(SessionEnd::Error(format!("cannot create session dir: {e}")));
    }
    let single = info.mode == PatchMode::Single;
    let wal = match Wal::open(&session_dir.join("wal.bin")) {
        Ok(w) => w,
        Err(e) => bail!(SessionEnd::Error(format!("cannot open WAL: {e}"))),
    };
    let sendq = if single {
        None
    } else {
        match SendQueue::open(&session_dir.join("send_queue.dat")) {
            Ok(sq) => Some(sq),
            Err(e) => bail!(SessionEnd::Error(format!("cannot open send queue: {e}"))),
        }
    };

    reporter.log(format!("r4: game connected ({})", info.summary()));
    // From here the game is validated and matches the patch: r4 owns real items,
    // so the DLL hook yields to it (until this session ends and clears the flag).
    connected.store(true, Ordering::Relaxed);

    // Replay the confirmed WAL onto the map so a mid-session restart still shows
    // every item (idempotent on the tracker's collected set).
    for i in 0..wal.count() {
        if let Some(e) = wal.get(i) {
            if e.wal_type == WAL_ITEM {
                let item = WalItem {
                    to: e.item_to,
                    game: e.item_game,
                    gi: e.item_gi,
                    flags: e.item_flags,
                    key: e.item_key,
                };
                reporter.item(net_item_from(&item, e.from, e.item_to));
            }
        }
    }

    // --- Random sequence bases + HELLO_OUT ------------------------------------
    let seq_game = rand_u32();
    let seq_net = rand_u32();
    let _ = conn.write(&IpcMessage::serialize(0, ipc_op::HELLO, &hello_out(seq_game, seq_net)));

    let wal_count = Arc::new(AtomicU32::new(wal.count()));

    // --- Uplink threads (non-single modes) ------------------------------------
    let alive = Arc::new(AtomicBool::new(true));
    let (uplink_out_tx, uplink_out_rx) = mpsc::channel::<(u8, Vec<u8>)>();
    let (uplink_in_tx, uplink_in_rx) = mpsc::channel::<(u8, Vec<u8>)>();
    let uplink_join = if single {
        None
    } else {
        let ctx = UplinkCtx {
            host: cfg.server_host.clone(),
            port: cfg.server_port,
            session_id: info.session_id,
            session_secret: info.session_secret,
            world_id: info.world_id,
            player_id: h.player_id,
            player_name: h.player_name,
            wal_count: wal_count.clone(),
            alive: alive.clone(),
            running: running.clone(),
            in_tx: uplink_in_tx,
            out_rx: uplink_out_rx,
            reporter: reporter.clone(),
        };
        Some(thread::spawn(move || uplink_manager(ctx)))
    };

    let mut session = Session {
        conn: conn.clone(),
        reporter: reporter.clone(),
        world_id: info.world_id,
        single,
        player_id: h.player_id,
        player_name: h.player_name,
        seq_game,
        seq_net,
        wal,
        wal_count,
        sendq,
        uplink_out: if single { None } else { Some(uplink_out_tx) },
    };

    // --- Main loop ------------------------------------------------------------
    let mut last_retransmit = Instant::now();
    'main: loop {
        if !running.load(Ordering::Relaxed) {
            break;
        }

        // Messages from the game.
        loop {
            match msg_rx.try_recv() {
                Ok(msg) => {
                    if let Err(e) = session.handle_msg(msg) {
                        session.reporter.log(format!("r4: {e}"));
                        break 'main;
                    }
                }
                Err(TryRecvError::Empty) => break,
                Err(TryRecvError::Disconnected) => break 'main, // pipe reader died
            }
        }

        // Packets from the server.
        loop {
            match uplink_in_rx.try_recv() {
                Ok((op, data)) => session.handle_uplink_packet(op, data),
                Err(TryRecvError::Empty) => break,
                Err(TryRecvError::Disconnected) => break, // manager gone; keep serving the game
            }
        }

        // Retransmit unacked entries every 10 s (`handleSendQueue`).
        if session.sendq.is_some() && last_retransmit.elapsed() >= Duration::from_secs(10) {
            last_retransmit = Instant::now();
            if let Some(sq) = &session.sendq {
                if let Some(tx) = &session.uplink_out {
                    for data in sq.pending() {
                        let _ = tx.send((up_op::WAL, data));
                    }
                }
            }
        }

        thread::sleep(Duration::from_millis(5));
    }

    // --- Teardown -------------------------------------------------------------
    connected.store(false, Ordering::Relaxed); // hand item ownership back to the DLL hook
    alive.store(false, Ordering::Relaxed);
    session.uplink_out = None; // drop the sender so the manager's recv unblocks
    conn.cancel(); // unblock the pipe reader
    let _ = reader_join.join();
    if let Some(j) = uplink_join {
        let _ = j.join();
    }
    reporter.log("r4: game disconnected".to_string());
    SessionEnd::Ended
}

/// Everything the uplink manager thread needs.
struct UplinkCtx {
    host: String,
    port: u16,
    session_id: [u8; 16],
    session_secret: [u8; 8],
    world_id: u8,
    player_id: [u8; 16],
    player_name: [u8; 8],
    wal_count: Arc<AtomicU32>,
    alive: Arc<AtomicBool>,
    running: Arc<AtomicBool>,
    in_tx: Sender<(u8, Vec<u8>)>,
    out_rx: Receiver<(u8, Vec<u8>)>,
    reporter: Reporter,
}

/// Connect / reconnect loop to the server (`handleUplink`). Ends when the session
/// stops (`alive` cleared) or the tracker stops (`running` cleared).
fn uplink_manager(ctx: UplinkCtx) {
    while ctx.alive.load(Ordering::Relaxed) && ctx.running.load(Ordering::Relaxed) {
        let connected = uplink_connection(&ctx);
        if !ctx.alive.load(Ordering::Relaxed) || !ctx.running.load(Ordering::Relaxed) {
            break;
        }
        if connected {
            ctx.reporter.log("r4: uplink lost, retrying in 5 s".to_string());
        }
        // Wait 5 s before retrying, but stay responsive to stop.
        let start = Instant::now();
        while start.elapsed() < Duration::from_secs(5) {
            if !ctx.alive.load(Ordering::Relaxed) || !ctx.running.load(Ordering::Relaxed) {
                return;
            }
            // Discard anything queued while disconnected; the send queue retransmits.
            while ctx.out_rx.try_recv().is_ok() {}
            thread::sleep(Duration::from_millis(200));
        }
    }
}

/// One uplink connection: connect, handshake, then pump packets until it drops.
/// Returns whether a connection was actually established (for the log message).
fn uplink_connection(ctx: &UplinkCtx) -> bool {
    // Connect, trying each resolved address. The Go client dials with a 10 s
    // timeout; we use 4 s so a Stop while the server is unreachable doesn't freeze
    // the UI (which joins this thread) for that long.
    let mut stream = None;
    if let Ok(addrs) = (ctx.host.as_str(), ctx.port).to_socket_addrs() {
        for addr in addrs {
            if !ctx.alive.load(Ordering::Relaxed) || !ctx.running.load(Ordering::Relaxed) {
                return false;
            }
            if let Ok(s) = TcpStream::connect_timeout(&addr, Duration::from_secs(4)) {
                stream = Some(s);
                break;
            }
        }
    }
    let Some(stream) = stream else {
        return false;
    };

    // Drain stale outgoing packets (`drain_done`).
    while ctx.out_rx.try_recv().is_ok() {}

    // Handshake: ClientHello -> ServerHello.
    let hello = client_hello(
        &ctx.session_id,
        &ctx.session_secret,
        &ctx.player_id,
        &ctx.player_name,
        ctx.world_id,
        ctx.wal_count.load(Ordering::Relaxed),
    );
    if send_packet(&stream, up_op::HELLO, &hello).is_err() {
        return true;
    }
    let _ = stream.set_read_timeout(Some(Duration::from_secs(10)));
    match recv_packet(&stream) {
        Ok((op, data)) if op == up_op::HELLO && data.len() >= 12 && data[0..8] == MAGIC => {}
        _ => {
            ctx.reporter.log("r4: uplink handshake failed".to_string());
            return true;
        }
    }
    ctx.reporter.log(format!("r4: connected to server {}:{}", ctx.host, ctx.port));

    // Socket reader sub-thread: server packets -> in_tx.
    let conn_lost = Arc::new(AtomicBool::new(false));
    let reader_stream = match stream.try_clone() {
        Ok(s) => s,
        Err(_) => return true,
    };
    let reader_lost = conn_lost.clone();
    let reader_in = ctx.in_tx.clone();
    let reader_alive = ctx.alive.clone();
    let reader = thread::spawn(move || {
        let _ = reader_stream.set_read_timeout(Some(Duration::from_secs(10)));
        while reader_alive.load(Ordering::Relaxed) {
            match recv_packet(&reader_stream) {
                Ok((op, data)) => {
                    if reader_in.send((op, data)).is_err() {
                        break;
                    }
                }
                Err(_) => break, // timeout / EOF / reset
            }
        }
        reader_lost.store(true, Ordering::Relaxed);
    });

    // Writer loop: forward outgoing packets and send a NOP every 3 s.
    let mut last_nop = Instant::now();
    loop {
        if !ctx.alive.load(Ordering::Relaxed)
            || !ctx.running.load(Ordering::Relaxed)
            || conn_lost.load(Ordering::Relaxed)
        {
            break;
        }
        match ctx.out_rx.recv_timeout(Duration::from_millis(500)) {
            Ok((op, data)) => {
                if send_packet(&stream, op, &data).is_err() {
                    break;
                }
            }
            Err(mpsc::RecvTimeoutError::Timeout) => {}
            Err(mpsc::RecvTimeoutError::Disconnected) => break, // session ending
        }
        if last_nop.elapsed() >= Duration::from_secs(3) {
            last_nop = Instant::now();
            if send_packet(&stream, up_op::NOP, &[]).is_err() {
                break;
            }
        }
    }

    // Tear down this connection: shut the socket so the reader unblocks, join it.
    let _ = stream.shutdown(std::net::Shutdown::Both);
    let _ = reader.join();
    true
}

/// Send one uplink packet: `[size u16 LE][op u8][data]` (`protocol.SendRaw`).
fn send_packet(mut stream: &TcpStream, op: u8, data: &[u8]) -> std::io::Result<()> {
    let mut header = [0u8; 3];
    header[0..2].copy_from_slice(&(data.len() as u16).to_le_bytes());
    header[2] = op;
    stream.write_all(&header)?;
    stream.write_all(data)
}

/// Receive one uplink packet (`protocol.RecvRaw`).
fn recv_packet(mut stream: &TcpStream) -> std::io::Result<(u8, Vec<u8>)> {
    let mut header = [0u8; 3];
    stream.read_exact(&mut header)?;
    let size = u16::from_le_bytes([header[0], header[1]]) as usize;
    let mut data = vec![0u8; size];
    stream.read_exact(&mut data)?;
    Ok((header[2], data))
}
