//! Background shared-memory poller (mirror of the Qt `MemoryReader` thread).
//!
//! egui is immediate-mode: any wake-up re-renders the whole UI. To keep the UI
//! truly asleep when nothing happens, the DLL's shared memory is polled on this
//! dedicated thread instead of on every UI frame. The thread owns the mapping
//! locally (it never crosses a thread boundary, so `SharedMemory`'s raw pointer
//! stays sound), manages the connection (open / inject / liveness), and only
//! pushes a message + `request_repaint()` when there is a real event or a status
//! change. The UI then renders exactly when — and only when — something changed.

use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::mpsc::{self, Receiver, Sender};
use std::sync::Arc;
use std::thread;
use std::time::{Duration, Instant};

use eframe::egui;

use crate::inject;
use crate::shared_mem::{Event, SharedMemory};

/// A message from the poller thread to the UI thread.
pub enum PollMsg {
    /// New game events, with the current game-version words (for ROM detection).
    Events(Vec<Event>, [u32; 2]),
    /// A connection-status change: (connected?, status text).
    Status(bool, String),
}

/// The UI-side handle: the message receiver + a "force reconnect" switch that
/// the Launch button flips to drop the link and re-open / re-inject.
pub struct Poller {
    pub rx: Receiver<PollMsg>,
    force: Arc<AtomicBool>,
}

impl Poller {
    /// Request a fresh connection attempt (Launch "Connecter / Injecter" button).
    pub fn force_reconnect(&self) {
        self.force.store(true, Ordering::Relaxed);
    }
}

/// Spawn the poller thread bound to the given egui context (used to wake the UI).
pub fn spawn(ctx: egui::Context) -> Poller {
    let (tx, rx) = mpsc::channel();
    let force = Arc::new(AtomicBool::new(false));
    let force_thread = force.clone();
    thread::spawn(move || run(ctx, tx, force_thread));
    Poller { rx, force }
}

/// Emit a status only when the text actually changed, so an unchanging "waiting"
/// state never wakes the UI.
fn push_status(
    ctx: &egui::Context,
    tx: &Sender<PollMsg>,
    last: &mut String,
    connected: bool,
    text: &str,
) {
    if last != text {
        *last = text.to_string();
        let _ = tx.send(PollMsg::Status(connected, text.to_string()));
        ctx.request_repaint();
    }
}

fn run(ctx: egui::Context, tx: Sender<PollMsg>, force: Arc<AtomicBool>) {
    let mut shared: Option<SharedMemory> = None;
    let mut injected = false;
    let mut last_status = String::new();
    let mut last_liveness = Instant::now();

    loop {
        // A forced reconnect drops the current link so the disconnected branch
        // re-opens the mapping (or re-injects into a freshly relaunched PJ64).
        if force.swap(false, Ordering::Relaxed) {
            shared = None;
            injected = false;
        }

        if let Some(sm) = shared.as_mut() {
            let events = sm.poll();
            if !events.is_empty() {
                let gv = sm.game_version();
                let _ = tx.send(PollMsg::Events(events, gv));
                ctx.request_repaint(); // a real event: wake the UI now
            }
            // Liveness (~1.4x/s): drop the link when Project64 exits so we
            // re-inject on its next launch.
            if last_liveness.elapsed() >= Duration::from_millis(700) {
                last_liveness = Instant::now();
                if inject::find_pj64_pid().is_none() {
                    shared = None;
                    injected = false;
                    push_status(&ctx, &tx, &mut last_status, false, "Project64 fermé");
                }
            }
            thread::sleep(Duration::from_millis(40));
        } else {
            // Disconnected: open the mapping, else inject into a running PJ64.
            if let Some(sm) = SharedMemory::open() {
                shared = Some(sm);
                push_status(&ctx, &tx, &mut last_status, true, "Connecté à Project64");
            } else {
                match inject::find_pj64_pid() {
                    None => {
                        injected = false;
                        push_status(&ctx, &tx, &mut last_status, false, "En attente de Project64…");
                    }
                    Some(_) if injected => {
                        push_status(
                            &ctx, &tx, &mut last_status, false,
                            "DLL injectée — en attente du jeu…",
                        );
                    }
                    Some(_) => match inject::inject() {
                        Ok(()) => {
                            injected = true;
                            push_status(&ctx, &tx, &mut last_status, false, "DLL injectée");
                        }
                        Err(e) => push_status(&ctx, &tx, &mut last_status, false, &e),
                    },
                }
            }
            thread::sleep(Duration::from_millis(400));
        }
    }
}
