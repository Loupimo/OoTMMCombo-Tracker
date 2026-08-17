//! Background shared-memory poller (mirror of the Qt `MemoryReader` thread).
//!
//! egui is immediate-mode: any wake-up re-renders the whole UI. To keep the UI
//! truly asleep when nothing happens, the DLL's shared memory is polled on this
//! dedicated thread instead of on every UI frame. The thread owns the mapping
//! locally (it never crosses a thread boundary, so `SharedMemory`'s raw pointer
//! stays sound), manages the connection (open / load plugin / liveness), and
//! only pushes a message + `request_repaint()` when there is a real event or a
//! status change. The UI then renders exactly when — and only when — something
//! changed.
//!
//! Connexion : au lieu d'un injecteur externe, la DLL est chargée par PJ64
//! lui-même (voir `inject::load_plugin` : copie dans `Plugin/` + Ctrl+T). Ce
//! thread orchestre tout le cycle de vie décrit dans `MemoryReader.cpp` :
//! chargement, attente de la mémoire partagée, fermeture des réglages, puis
//! déchargement propre (Command = Shutdown) + suppression de la DLL à l'arrêt.

use std::path::PathBuf;
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
    /// A connection-status change: (connected?, short status-bar text).
    Status(bool, String),
    /// A journal line (the step-by-step messages, reused from the Qt build).
    Log(String),
}

/// The UI-side handle: the message receiver + a "tracking active" switch that
/// the Start/Stop button flips (mirror of `LogTab::PressLaunchButton`), and a
/// shutdown latch used on app exit to unload the DLL cleanly.
pub struct Poller {
    pub rx: Receiver<PollMsg>,
    running: Arc<AtomicBool>,
    shutdown: Arc<AtomicBool>,
    shutdown_done: Arc<AtomicBool>,
}

impl Poller {
    /// Start / stop the auto-tracker. When switched off, the poller unloads the
    /// DLL (Command = Shutdown → wait → remove) and goes idle; when switched on,
    /// it connects / loads the plugin again.
    pub fn set_tracking(&self, on: bool) {
        self.running.store(on, Ordering::Relaxed);
    }

    /// Ask the poller to unload the DLL cleanly (Command = Shutdown → wait for
    /// unload → remove the plugin file) and block until it finishes or times
    /// out. Called from `eframe::App::on_exit` so Project64 keeps running with
    /// its patches restored and no leftover DLL in `Plugin/`.
    pub fn shutdown_and_wait(&self) {
        self.shutdown.store(true, Ordering::Relaxed);
        let start = Instant::now();
        while !self.shutdown_done.load(Ordering::Relaxed) && start.elapsed() < Duration::from_secs(7)
        {
            thread::sleep(Duration::from_millis(20));
        }
    }
}

/// Spawn the poller thread bound to the given egui context (used to wake the UI).
/// It starts idle: tracking begins only when the UI calls `set_tracking(true)`
/// (the "Start Tracking" button), mirroring the Qt build.
pub fn spawn(ctx: egui::Context) -> Poller {
    let (tx, rx) = mpsc::channel();
    let running = Arc::new(AtomicBool::new(false));
    let shutdown = Arc::new(AtomicBool::new(false));
    let shutdown_done = Arc::new(AtomicBool::new(false));
    let (r, s, d) = (running.clone(), shutdown.clone(), shutdown_done.clone());
    thread::spawn(move || run(ctx, tx, r, s, d));
    Poller { rx, running, shutdown, shutdown_done }
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

/// Append a journal line (the step-by-step C++ messages) and wake the UI.
fn send_log(ctx: &egui::Context, tx: &Sender<PollMsg>, msg: &str) {
    let _ = tx.send(PollMsg::Log(msg.to_string()));
    ctx.request_repaint();
}

/// The whole tracking connection state that a reconnect / shutdown must reset.
#[derive(Default)]
struct Link {
    shared: Option<SharedMemory>,
    /// `load_plugin` done, waiting for the DLL to create the shared memory.
    loading: bool,
    /// The DLL copied into `Plugin/` (to remove once unloaded).
    plugin_path: Option<PathBuf>,
    /// The settings window opened by Ctrl+T (to close once connected).
    settings_window: Option<isize>,
    /// The tracked PJ64 process id (for liveness + unload waiting).
    pid: Option<u32>,
}

/// Ask the DLL to unload, wait (bounded) for it, then drop the mapping and
/// remove the plugin file. Mirror of `StartMemoryReader` steps 10–12.
fn unload(link: &mut Link, wait: Duration, log: &dyn Fn(&str)) {
    if let Some(sm) = link.shared.as_ref() {
        log("Requesting tracker DLL shutdown...");
        sm.request_shutdown();
    }
    let pid = link.pid.or_else(inject::find_pj64_pid);
    let unloaded = match pid {
        Some(pid) => inject::wait_for_unload(pid, wait),
        None => true, // PJ64 gone: the DLL went with it
    };
    link.shared = None; // unmap + close our handle (Drop)
    if let Some(path) = link.plugin_path.take() {
        if unloaded {
            inject::remove_tracker_dll(&path);
            log("Tracker DLL successfully unloaded.");
            log("Tracker DLL removed from Project64 Plugin folder.");
        } else {
            log("Tracker DLL is still loaded. Cannot safely remove it.");
        }
    }
    link.loading = false;
    link.settings_window = None;
}

fn run(
    ctx: egui::Context,
    tx: Sender<PollMsg>,
    running: Arc<AtomicBool>,
    shutdown: Arc<AtomicBool>,
    shutdown_done: Arc<AtomicBool>,
) {
    let mut link = Link::default();
    let mut last_status = String::new();
    let mut last_liveness = Instant::now();
    // Whether "Searching for Project64…" was already logged for this attempt, so
    // the retry message appears once instead of every 400 ms.
    let mut announced_search = false;

    loop {
        // App exit: unload the DLL cleanly and end the thread.
        if shutdown.load(Ordering::Relaxed) {
            unload(&mut link, Duration::from_secs(5), &|s| send_log(&ctx, &tx, s));
            shutdown_done.store(true, Ordering::Relaxed);
            return;
        }

        // Stopped (Start/Stop button off): tear down any active link once, then
        // idle. Mirror of `PressLaunchButton` stopping the MemoryReader thread.
        if !running.load(Ordering::Relaxed) {
            if link.shared.is_some() || link.loading || link.plugin_path.is_some() {
                unload(&mut link, Duration::from_secs(2), &|s| send_log(&ctx, &tx, s));
                link.pid = None;
            }
            announced_search = false;
            push_status(&ctx, &tx, &mut last_status, false, "Tracking inactif");
            thread::sleep(Duration::from_millis(200));
            continue;
        }

        if let Some(sm) = link.shared.as_mut() {
            // Connected: drain events + liveness.
            let events = sm.poll();
            if !events.is_empty() {
                let gv = sm.game_version();
                let _ = tx.send(PollMsg::Events(events, gv));
                ctx.request_repaint(); // a real event: wake the UI now
            }
            if last_liveness.elapsed() >= Duration::from_millis(700) {
                last_liveness = Instant::now();
                match inject::find_pj64_pid() {
                    Some(pid) => link.pid = Some(pid),
                    None => {
                        // PJ64 exited: its DLL went with it, remove the leftover file.
                        send_log(&ctx, &tx, "Project 64 has been closed. Stop tracking...");
                        link.shared = None;
                        link.loading = false;
                        link.settings_window = None;
                        link.pid = None;
                        if let Some(path) = link.plugin_path.take() {
                            inject::remove_tracker_dll(&path);
                        }
                        announced_search = false;
                        push_status(&ctx, &tx, &mut last_status, false, "Project64 fermé");
                    }
                }
            }
            thread::sleep(Duration::from_millis(40));
        } else if link.loading {
            // Injected (Ctrl+T sent): now wait for the DLL to (re)create the
            // shared memory. Opening it here — AFTER re-triggering the load — is
            // what fixes re-Start: a stale named mapping survives for PJ64's
            // whole lifetime, so we must never treat "open succeeds" as connected
            // without re-injecting first.
            if let Some(sm) = SharedMemory::open() {
                send_log(&ctx, &tx, "Shared memory found !");
                send_log(&ctx, &tx, "Tracker DLL successfully loaded.");
                link.shared = Some(sm);
                link.loading = false;
                if let Some(hwnd) = link.settings_window.take() {
                    inject::close_window(hwnd);
                    send_log(&ctx, &tx, "Project64 Settings closed.");
                } else {
                    send_log(
                        &ctx, &tx,
                        "Warning: Project64 Settings window could not be identified. The dll may have not been loaded correctly.",
                    );
                }
                send_log(&ctx, &tx, "Reading game memory...");
                push_status(&ctx, &tx, &mut last_status, true, "Connecté à Project64");
            } else if inject::find_pj64_pid().is_none() {
                send_log(&ctx, &tx, "Tracker DLL failed to initialize.");
                link.loading = false;
                link.settings_window = None;
                link.pid = None;
                if let Some(path) = link.plugin_path.take() {
                    inject::remove_tracker_dll(&path);
                }
                announced_search = false;
                push_status(&ctx, &tx, &mut last_status, false, "Project64 fermé");
            } else {
                push_status(&ctx, &tx, &mut last_status, false, "En attente du jeu…");
                thread::sleep(Duration::from_millis(50));
            }
        } else {
            // Fresh start of this session: ALWAYS (re)run the load sequence
            // before opening the mapping (mirror of the C++ StartMemoryReader,
            // which re-injects on every Start).
            match inject::find_pj64_pid() {
                None => {
                    if !announced_search {
                        send_log(
                            &ctx, &tx,
                            "No Project64 process found. Retrying in 1 second...",
                        );
                        announced_search = true;
                    }
                    push_status(&ctx, &tx, &mut last_status, false, "En attente de Project64…");
                    thread::sleep(Duration::from_millis(400));
                }
                Some(pid) => {
                    announced_search = false;
                    link.pid = Some(pid);
                    send_log(&ctx, &tx, &format!("Process Found : {pid}"));
                    let result = inject::load_plugin(pid, &|s| send_log(&ctx, &tx, s));
                    match result {
                        Ok(loaded) => {
                            link.plugin_path = Some(loaded.dll_path);
                            link.settings_window = loaded.settings_window;
                            link.loading = true;
                            push_status(&ctx, &tx, &mut last_status, false, "Chargement du plugin…");
                        }
                        Err(e) => {
                            send_log(&ctx, &tx, &e);
                            push_status(&ctx, &tx, &mut last_status, false, "Injection échouée");
                            thread::sleep(Duration::from_millis(400));
                        }
                    }
                }
            }
        }
    }
}
