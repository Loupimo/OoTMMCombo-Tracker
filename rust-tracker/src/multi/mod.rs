//! Multiplayer networking (port of Sources/Multi, the OoTMM multi client).
//!
//! Runs the whole App loop of the Qt build on a dedicated thread, spawned when
//! the auto-tracker starts with multiplayer enabled. The loop accepts the custom
//! Project64 build's API connection (and, optionally, the Ares GDB stub), then
//! ticks each connected game: it reads the OoTMM net context out of emulator
//! memory, relays ledger entries to / from the remote server, and reports every
//! transfer to the UI thread through `MultiMsg`. The DLL shared-memory hook path
//! (poller.rs) is unchanged and runs in parallel, exactly like the Qt MemoryReader.

mod buffer;
mod game;
mod protocol;
mod sendq;

use std::net::{TcpListener, TcpStream, ToSocketAddrs};
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::mpsc::{self, Receiver, Sender};
use std::sync::{Arc, Mutex};
use std::thread::{self, JoinHandle};
use std::time::Duration;

use eframe::egui;

use game::Game;
use protocol::{ApiConn, Protocol};

/// Maximum number of concurrent emulator connections (`MAX_GAMES`).
const MAX_GAMES: usize = 8;
/// The custom Project64 build connects here for the memory API (`appStartPj64`).
/// Resolved like the C++ `getaddrinfo("localhost", ...)` so the tracker binds the
/// same address family the emulator dials.
const PJ64_API_ADDR: &str = "localhost:13249";
/// The Ares emulator's GDB remote stub (`appCheckAres`).
const ARES_API_ADDR: &str = "localhost:9123";

/// A decoded network ledger transfer, handed to the UI thread to apply. Carries
/// the raw ParseKey fields (the UI runs CorrectComboItem + FindObject with the
/// live ROM build / MQ set) plus the item id and the sender / receiver worlds.
pub struct NetItem {
    /// OoTMM game id: 0 = OoT, 1 = MM.
    pub game_id: u8,
    pub ov_type: u8,
    pub scene: u16,
    pub room: u32,
    pub object: u32,
    /// The granted item id (raw ledger `gi`), for the item-name lookup.
    pub gi: u16,
    /// 1-based sender / receiver worlds, or <= 0 when unknown. The check lives in
    /// `from_world`; its progression is credited to `to_world` (via the spoiler
    /// destination the tracker already stores per placement).
    pub from_world: i32,
    pub to_world: i32,
}

/// A hook-captured "nothing" drop queued for the network thread to push to the
/// ledger (port of `TrackerNothing`). Crosses from the UI/poller side to the
/// multi thread through `MultiHandle::queue_nothing`.
pub struct TrackerNothing {
    pub game_id: u8,
    pub key: u32,
    pub gi: u16,
}

/// A message from the multi thread to the UI thread.
pub enum MultiMsg {
    /// A journal line (the MultiLogger step-by-step messages).
    Log(String),
    /// A decoded ledger transfer to apply to the tracked worlds.
    Item(NetItem),
}

/// Sends journal lines / items to the UI and wakes it (egui is immediate-mode, so
/// a repaint request is what actually shows the update). Cloned into every game.
#[derive(Clone)]
pub struct Reporter {
    tx: Sender<MultiMsg>,
    ctx: egui::Context,
}

impl Reporter {
    pub fn log(&self, msg: impl Into<String>) {
        let _ = self.tx.send(MultiMsg::Log(msg.into()));
        self.ctx.request_repaint();
    }
    pub fn item(&self, item: NetItem) {
        let _ = self.tx.send(MultiMsg::Item(item));
        self.ctx.request_repaint();
    }
}

/// Launch parameters for the multi thread (server address + networking flag).
pub struct MultiConfig {
    pub server_host: String,
    pub server_port: u16,
    pub net_enabled: bool,
}

/// UI-side handle: the message receiver + the stop switch + the shared
/// "pending nothings" queue drained by the network thread.
pub struct MultiHandle {
    pub rx: Receiver<MultiMsg>,
    running: Arc<AtomicBool>,
    pending: Arc<Mutex<Vec<TrackerNothing>>>,
    join: Option<JoinHandle<()>>,
}

impl MultiHandle {
    /// Queue a hook-captured "nothing" drop for the network thread (thread-safe).
    pub fn queue_nothing(&self, nothing: TrackerNothing) {
        if let Ok(mut q) = self.pending.lock() {
            q.push(nothing);
        }
    }

    /// Signal the thread to stop and block until it has torn down its sockets.
    pub fn stop(&mut self) {
        self.running.store(false, Ordering::Relaxed);
        if let Some(j) = self.join.take() {
            let _ = j.join();
        }
    }
}

impl Drop for MultiHandle {
    fn drop(&mut self) {
        self.stop();
    }
}

/// Spawn the multi thread and return its UI-side handle.
pub fn spawn(ctx: egui::Context, cfg: MultiConfig) -> MultiHandle {
    let (tx, rx) = mpsc::channel();
    let running = Arc::new(AtomicBool::new(true));
    let pending = Arc::new(Mutex::new(Vec::new()));
    let reporter = Reporter { tx, ctx };
    let (r, p) = (running.clone(), pending.clone());
    let join = thread::spawn(move || run(cfg, r, p, reporter));
    MultiHandle { rx, running, pending, join: Some(join) }
}

/// The App main loop (`App::appRun`): accept emulator connections and tick every
/// active game until stopped.
fn run(
    cfg: MultiConfig,
    running: Arc<AtomicBool>,
    pending: Arc<Mutex<Vec<TrackerNothing>>>,
    reporter: Reporter,
) {
    reporter.log("Tracker Loop Started.");

    // Listening socket the custom Project64 build connects to (`appStartPj64`).
    let listener = match TcpListener::bind(PJ64_API_ADDR) {
        Ok(l) => {
            let _ = l.set_nonblocking(true);
            Some(l)
        }
        Err(e) => {
            reporter.log(format!("Unable to listen on {PJ64_API_ADDR}: {e}"));
            None
        }
    };

    let mut games: Vec<Game> = Vec::new();

    while running.load(Ordering::Relaxed) {
        // Accept any pending Project64 connections.
        if let Some(listener) = listener.as_ref() {
            loop {
                match listener.accept() {
                    Ok((sock, _)) => {
                        let _ = sock.set_nonblocking(false); // blocking API transactions
                        if games.len() < MAX_GAMES {
                            reporter.log("Project64 API connected.");
                            games.push(Game::new(ApiConn::new(sock, Protocol::Pj64), reporter.clone()));
                        }
                    }
                    Err(ref e) if e.kind() == std::io::ErrorKind::WouldBlock => break,
                    Err(_) => break,
                }
            }
        }

        // Best-effort single Ares connection (`appCheckAres`): connect only when
        // no Ares game is already active, so we never stack duplicates.
        let ares_active = games.iter().any(|g| g.is_ares);
        if !ares_active && games.len() < MAX_GAMES {
            if let Some(sock) = try_connect_ares() {
                let _ = sock.set_nonblocking(false);
                reporter.log("Ares API connected.");
                games.push(Game::new(ApiConn::new(sock, Protocol::Ares), reporter.clone()));
            }
        }

        // Tick every active game, then drop the ones that disconnected.
        for g in games.iter_mut() {
            g.tick(cfg.net_enabled, &cfg.server_host, cfg.server_port, &pending);
        }
        games.retain(|g| g.valid);

        thread::sleep(Duration::from_millis(100));
    }

    // Dropping the games closes their sockets (`appQuit`).
    drop(games);
    reporter.log("Tracker Loop Stopped.");
}

/// Try to reach the Ares GDB stub, trying every resolved address (IPv4 / IPv6)
/// with a short timeout so a down / absent Ares never stalls the loop.
fn try_connect_ares() -> Option<TcpStream> {
    for addr in ARES_API_ADDR.to_socket_addrs().ok()? {
        if let Ok(sock) = TcpStream::connect_timeout(&addr, Duration::from_millis(50)) {
            return Some(sock);
        }
    }
    None
}
