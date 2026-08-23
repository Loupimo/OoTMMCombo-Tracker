//! Keyboard navigation shared by the tree panels (scenes, objects, entrances,
//! progression locations). A tree becomes the keyboard target when the user
//! clicks one of its rows; the arrow keys then move a focus highlight through
//! that tree's rows, Left / Right fold / unfold the focused branch, and Enter /
//! Space activates the focused leaf (or toggles a branch). Only one tree owns
//! the keyboard at a time (the last one clicked), so several visible trees never
//! fight over the arrows.
//!
//! Immediate-mode contract: the caller builds a `KbdList` each frame (copying
//! this tree's state out of the persistent `KbdNav`), registers every row as it
//! is drawn — `branch` for a collapsible header, `leaf` for an item — then calls
//! `finish` once, still inside the panel, to read the keys. Branch fold / unfold
//! is written straight to the persisted open-state (the trees read it back via
//! `get_persisted`); the moves come back as plain data (`KbdOut`) for the caller
//! to apply after the panel closure, keeping `self` borrows simple.

use eframe::egui::{self, Align, Color32, Key, Response, Stroke};
use std::collections::HashMap;
use std::hash::{Hash, Hasher};

/// The focus-ring colour: a warm accent that reads over both the OoT blue and
/// the MM violet row tints and stays distinct from the blue "selected" fill.
const FOCUS_RING: Color32 = Color32::from_rgb(255, 214, 120);

/// A stable row key from any hashable identity (scene id, object index, an
/// entrance tuple, a location name…).
pub(crate) fn key(v: impl Hash) -> u64 {
    let mut h = std::collections::hash_map::DefaultHasher::new();
    v.hash(&mut h);
    h.finish()
}

/// Persistent keyboard-navigation state (one instance on `TrackerApp`), keyed by
/// each tree's stable `egui::Id`.
#[derive(Default)]
pub(crate) struct KbdNav {
    /// The tree that currently owns the arrow keys (set when a row is clicked).
    pub active: Option<egui::Id>,
    /// The focused row key per tree (a caller-defined stable hash).
    pub focus: HashMap<egui::Id, u64>,
    /// Per-tree "scroll the focused row into view next frame" flag, raised when
    /// the focus moved via the keyboard.
    pub scroll: HashMap<egui::Id, bool>,
}

impl KbdNav {
    /// Snapshot a tree's state for this frame, consuming any pending scroll
    /// request. Call before the panel closure so no `&mut self.kbd` leaks inside.
    pub fn begin(&mut self, id: egui::Id) -> KbdList {
        KbdList {
            active: self.active == Some(id),
            focus: self.focus.get(&id).copied(),
            scroll_pending: self.scroll.remove(&id).unwrap_or(false),
            rows: Vec::new(),
            claim: None,
        }
    }

    /// Apply the deferred moves after the panel closure: retarget the keyboard
    /// (on click), remember the new focus and raise a scroll request. The row's
    /// own side effect (select / toggle / navigate) is the caller's job.
    pub fn apply(&mut self, id: egui::Id, out: &KbdOut) {
        if let Some(k) = out.claim {
            self.active = Some(id);
            self.focus.insert(id, k);
        }
        if let Some(k) = out.moved {
            self.focus.insert(id, k);
            self.scroll.insert(id, true);
        }
    }
}

/// What the arrow keys asked a tree to do this frame (applied by the caller).
#[derive(Default)]
pub(crate) struct KbdOut {
    /// A row was clicked: this tree takes the keyboard and focuses that key.
    pub claim: Option<u64>,
    /// Focus moved to this key (highlight only; the scene tree also selects it).
    pub moved: Option<u64>,
    /// Enter / Space asked to activate this focused leaf.
    pub activate: Option<u64>,
}

/// One registered row: a leaf, or a collapsible branch header carrying the
/// persisted open-state id and its current open flag (for Left / Right folding).
struct Row {
    key: u64,
    branch: Option<(egui::Id, bool)>,
}

/// Per-frame builder (see the module note). Pure data apart from the open-state
/// writes in `finish`, so it can live across a panel closure without borrow
/// trouble.
pub(crate) struct KbdList {
    active: bool,
    focus: Option<u64>,
    scroll_pending: bool,
    rows: Vec<Row>,
    claim: Option<u64>,
}

impl KbdList {
    /// Register a leaf row (an item: scene, object, entrance side, location).
    pub fn leaf(&mut self, ui: &egui::Ui, k: u64, resp: &Response) {
        self.push(ui, k, None, resp);
    }

    /// Register a collapsible branch header: `open_id` is the persisted
    /// open-state id (the same one the tree reads with `get_persisted`), `open`
    /// its current value. Left folds it, Right unfolds it, Enter toggles it.
    pub fn branch(&mut self, ui: &egui::Ui, k: u64, open_id: egui::Id, open: bool, resp: &Response) {
        self.push(ui, k, Some((open_id, open)), resp);
    }

    fn push(&mut self, ui: &egui::Ui, k: u64, branch: Option<(egui::Id, bool)>, resp: &Response) {
        self.rows.push(Row { key: k, branch });
        if resp.clicked() {
            self.claim = Some(k);
        }
        // The ring shows only while this tree owns the keyboard, so a remembered
        // focus stays invisible until the user clicks back into the tree.
        if self.active && self.focus == Some(k) {
            // Rectangular ring (rounding 0) — the Qt selection frame is square, not
            // the rounded egui default.
            ui.painter()
                .rect_stroke(resp.rect.shrink(1.0), 0.0, Stroke::new(1.5_f32, FOCUS_RING));
            if self.scroll_pending {
                resp.scroll_to_me(Some(Align::Center));
            }
        }
    }

    /// Finish the frame (inside the panel): when this tree owns the keyboard and
    /// no text field is capturing input, read the arrows / Enter and act. Branch
    /// fold / unfold is written straight to the persisted open-state; the focus
    /// moves and leaf activation come back in `KbdOut` for the caller to apply.
    pub fn finish(&mut self, ui: &egui::Ui) -> KbdOut {
        let mut out = KbdOut { claim: self.claim, ..Default::default() };
        // A click wins outright: it both retargets the keyboard and focuses the
        // row, so ignore any arrow in the same frame.
        if out.claim.is_some() || !self.active || self.rows.is_empty() || ui.ctx().wants_keyboard_input()
        {
            return out;
        }
        let (down, up, left, right, activate) = ui.input(|i| {
            (
                i.key_pressed(Key::ArrowDown),
                i.key_pressed(Key::ArrowUp),
                i.key_pressed(Key::ArrowLeft),
                i.key_pressed(Key::ArrowRight),
                i.key_pressed(Key::Enter) || i.key_pressed(Key::Space),
            )
        });
        let cur = self.rows.iter().position(|r| Some(r.key) == self.focus);
        let last = self.rows.len() - 1;
        let mut new_focus: Option<usize> = None;
        let mut changed = false;

        if down {
            new_focus = Some(cur.map_or(0, |i| (i + 1).min(last)));
        } else if up {
            new_focus = Some(cur.map_or(last, |i| i.saturating_sub(1)));
        } else if right {
            match cur.map(|i| (i, &self.rows[i].branch)) {
                // Collapsed branch: unfold it (focus stays on the header).
                Some((_, Some((id, false)))) => changed |= self.set_open(ui, *id, true),
                // Open branch: step into its first child.
                Some((i, Some((_, true)))) => new_focus = Some((i + 1).min(last)),
                // Leaf: nothing to unfold.
                Some((_, None)) => {}
                None => new_focus = Some(0),
            }
        } else if left {
            match cur.map(|i| (i, &self.rows[i].branch)) {
                // Open branch: fold it (focus stays on the header).
                Some((_, Some((id, true)))) => changed |= self.set_open(ui, *id, false),
                // Collapsed branch or leaf: jump to the parent branch header.
                Some((i, _)) => {
                    if let Some(p) = self.rows[..i].iter().rposition(|r| r.branch.is_some()) {
                        new_focus = Some(p);
                    }
                }
                None => new_focus = Some(last),
            }
        }

        if let Some(ni) = new_focus {
            let k = self.rows[ni].key;
            if Some(k) != self.focus {
                out.moved = Some(k);
                changed = true;
            }
        }
        if activate {
            match cur.map(|i| &self.rows[i].branch) {
                // Enter on a branch folds / unfolds it, like Left / Right.
                Some(Some((id, open))) => changed |= self.set_open(ui, *id, !*open),
                Some(None) => out.activate = self.focus,
                None => {}
            }
        }
        // A change means the next frame must repaint (ring, scroll, fold, select)
        // even without the poller waking the UI.
        if changed {
            ui.ctx().request_repaint();
        }
        out
    }

    /// Write a branch's persisted open-state; returns whether it actually flipped.
    fn set_open(&self, ui: &egui::Ui, id: egui::Id, open: bool) -> bool {
        let cur = ui.data_mut(|d| d.get_persisted::<bool>(id));
        if cur == Some(open) {
            return false;
        }
        ui.data_mut(|d| d.insert_persisted(id, open));
        true
    }
}
