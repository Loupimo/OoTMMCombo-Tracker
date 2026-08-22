//! The r4 multiplayer mechanism (OoTMM builds newer than v32.0).
//!
//! Port of the Go client in `multiplayer-r4/client`. Unlike the old net-context
//! mechanism (see `multi/`), the session identity comes from the game's `.ootmm`
//! patch file (see `patch.rs`) and the transport is a Windows named pipe to the
//! emulator plus a TCP uplink to the OoTMM server. The client relays WAL entries
//! both ways and reports every item to the tracker map via `apply_net_item`.
//!
//! Spawned when the auto-tracker starts with a patch loaded. The DLL hook path
//! (poller) keeps running in parallel as a backup, exactly like the old client.

mod pipe;
mod proto;
mod session;
mod store;

use std::path::PathBuf;
use std::sync::atomic::{AtomicBool, AtomicU64, Ordering};
use std::sync::mpsc::{self, Receiver, Sender};
use std::sync::Arc;
use std::thread::{self, JoinHandle};
use std::time::{Duration, SystemTime, UNIX_EPOCH};

use eframe::egui;

use crate::multi::NetItem;
use crate::patch::PatchInfo;

/// Launch parameters for the r4 client thread.
pub struct R4Config {
    /// Uplink server host (`multi.ootmm.com` by default).
    pub server_host: String,
    /// Uplink server port (`14236` by default).
    pub server_port: u16,
    /// Root of the per-session data directory (WAL + send queue live under it).
    pub data_dir: PathBuf,
}

/// A message from the r4 client thread to the UI thread.
pub enum R4Msg {
    /// A journal line for the Launch page.
    Log(String),
    /// A decoded WAL item to apply to the tracked worlds (reuses the old client's
    /// `NetItem` so `apply_net_item` handles both mechanisms identically).
    Item(NetItem),
}

/// Sends journal lines / items to the UI and wakes egui (immediate mode).
#[derive(Clone)]
pub struct Reporter {
    tx: Sender<R4Msg>,
    ctx: egui::Context,
}

impl Reporter {
    pub fn log(&self, msg: impl Into<String>) {
        let _ = self.tx.send(R4Msg::Log(msg.into()));
        self.ctx.request_repaint();
    }
    pub fn item(&self, item: NetItem) {
        let _ = self.tx.send(R4Msg::Item(item));
        self.ctx.request_repaint();
    }
}

/// UI-side handle: the message receiver + the stop switch + the "live session"
/// flag (true only while connected to a validated game matching the patch).
pub struct R4Handle {
    pub rx: Receiver<R4Msg>,
    running: Arc<AtomicBool>,
    connected: Arc<AtomicBool>,
    join: Option<JoinHandle<()>>,
}

impl R4Handle {
    /// Whether r4 currently has a validated, connected game session. When false
    /// (older build without IPC, no game, or a game whose session differs from
    /// the patch), the DLL hook keeps ownership of items as a fallback.
    pub fn is_connected(&self) -> bool {
        self.connected.load(Ordering::Relaxed)
    }

    /// Signal the thread to stop and block until it has torn down its session.
    pub fn stop(&mut self) {
        self.running.store(false, Ordering::Relaxed);
        if let Some(j) = self.join.take() {
            let _ = j.join();
        }
    }
}

impl Drop for R4Handle {
    fn drop(&mut self) {
        self.stop();
    }
}

/// Spawn the r4 client thread and return its UI-side handle.
pub fn spawn(ctx: egui::Context, cfg: R4Config, info: PatchInfo) -> R4Handle {
    let (tx, rx) = mpsc::channel();
    let running = Arc::new(AtomicBool::new(true));
    let connected = Arc::new(AtomicBool::new(false));
    let reporter = Reporter { tx, ctx };
    let (r, c) = (running.clone(), connected.clone());
    let join = thread::spawn(move || run(cfg, info, r, c, reporter));
    R4Handle { rx, running, connected, join: Some(join) }
}

/// The client main loop (`app.loop`): poll for a game pipe, then run its session
/// until it disconnects, repeating until stopped. `connected` mirrors whether a
/// validated session is live, so the tracker can fall back to the DLL hook.
fn run(
    cfg: R4Config,
    info: PatchInfo,
    running: Arc<AtomicBool>,
    connected: Arc<AtomicBool>,
    reporter: Reporter,
) {
    reporter.log("r4: waiting for a game instance to connect…");
    // Remember the last transient note so an older / mismatched game (which fails
    // the same way every poll) doesn't spam the journal.
    let mut last_note = String::new();
    // Logs `msg` only when it differs from the previous transient note.
    let note = |last: &mut String, msg: String| {
        if *last != msg {
            reporter.log(msg.clone());
            *last = msg;
        }
    };

    while running.load(Ordering::Relaxed) {
        let pipes = pipe::list_pipes();
        if pipes.is_empty() {
            note(&mut last_note, "r4: no pj64em-ipc pipe found yet (emulator not running an r4 build?)".to_string());
        }
        for path in pipes {
            if !running.load(Ordering::Relaxed) {
                break;
            }
            let conn = match pipe::PipeConn::open(&path) {
                Ok(c) => c,
                Err(e) => {
                    note(&mut last_note, format!("r4: cannot open {path}: {e}"));
                    continue;
                }
            };
            match session::run_session(&cfg, &info, &running, &reporter, &connected, Arc::new(conn)) {
                // A validated session ran: log freely and reset the note tracker.
                session::SessionEnd::Ended => last_note.clear(),
                session::SessionEnd::Error(e) => note(&mut last_note, format!("r4: {e}")),
                // Transient: an older build / not-our-pipe, or a game whose session
                // doesn't match the patch. Logged once; the DLL hook stays in charge.
                session::SessionEnd::NoHello => note(
                    &mut last_note,
                    format!("r4: {path} is not an r4 game (no HELLO) — using the DLL hook instead"),
                ),
                session::SessionEnd::Mismatch(what) => note(
                    &mut last_note,
                    format!("r4: game {what} differs from the loaded patch — using the DLL hook instead"),
                ),
            }
        }
        if !running.load(Ordering::Relaxed) {
            break;
        }
        // Poll ~1 Hz. A validated session blocks in run_session for its whole life,
        // so this only paces the (re)discovery / retry of unmatched pipes.
        thread::sleep(Duration::from_secs(1));
    }
    connected.store(false, Ordering::Relaxed);
    reporter.log("r4: stopped");
}

/// A small non-crypto RNG for sequence bases and OVF_RENEW nonces (the Go build
/// uses `crypto/rand`, but only uniqueness matters here). Xorshift64* seeded from
/// the clock and a monotically bumped counter so parallel calls never collide.
pub(crate) fn rand_u32() -> u32 {
    static STATE: AtomicU64 = AtomicU64::new(0);
    let mut x = STATE.load(Ordering::Relaxed);
    if x == 0 {
        let nanos = SystemTime::now().duration_since(UNIX_EPOCH).map(|d| d.as_nanos() as u64).unwrap_or(0x9E37_79B9_7F4A_7C15);
        x = nanos | 1;
    }
    x = x.wrapping_add(0x9E37_79B9_7F4A_7C15);
    let mut z = x;
    z = (z ^ (z >> 30)).wrapping_mul(0xBF58_476D_1CE4_E5B9);
    z = (z ^ (z >> 27)).wrapping_mul(0x94D0_49BB_1331_11EB);
    z ^= z >> 31;
    STATE.store(x, Ordering::Relaxed);
    (z >> 32) as u32
}
