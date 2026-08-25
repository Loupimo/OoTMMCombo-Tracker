//! Bytecode evaluator: run one compiled access rule (`&[data::Op]`) against a
//! world state and return whether the rule is satisfied.
//!
//! The rules are a post-order (RPN) stream: leaves push a boolean, and
//! `Not`/`And`/`Or`/`Cond` fold the top of a small stack. There are no side
//! effects, so evaluation is eager (both branches of `cond` are computed) — the
//! stream is short (a handful of ops after dedup) and this runs inside the
//! solver's fixed point, so a branchless walk is the right shape.
//!
//! The state is abstracted behind [`WorldState`] so the evaluator stays
//! decoupled from how the solver derives the inventory / settings / reachable
//! events (those are the other M2 pieces). Tests here drive it with a mock.

use crate::data::Op;

/// Everything an access rule can query. The solver implements this over the
/// player's inventory + the seed settings + the events/flags reached so far;
/// the age is fixed per evaluation (the solver evaluates each region in both
/// ages when it may change).
pub trait WorldState {
    /// Count of an item id currently owned (0 if none).
    fn item_count(&self, id: u32) -> u32;
    /// Number of MM masks owned (`masks(n)`).
    fn mask_count(&self) -> u16;
    /// The value a setting is fixed to (`Some(value_id)`), for `setting(k, v)`.
    fn setting_value(&self, key: u32) -> Option<u32>;
    /// Whether a boolean-form setting is enabled (`setting(k)`).
    fn setting_enabled(&self, key: u32) -> bool;
    /// Whether a trick is enabled for this seed.
    fn trick(&self, id: u32) -> bool;
    /// Whether an event has been reached (fixed-point state).
    fn event(&self, id: u32) -> bool;
    /// Whether an MM region-state flag is set (`flag_on`) / clear (`flag_off`).
    fn flag_on(&self, id: u32) -> bool;
    /// Whether `flag_on(id)` / `flag_off(id)` (`want` distinguishes them) holds.
    /// Overridable so the solver can treat MM region state optimistically (both
    /// cleared and cursed reachable over a run) without the evaluator knowing.
    fn flag(&self, id: u32, want: bool) -> bool {
        self.flag_on(id) == want
    }
    /// Whether a win-condition gate is met (`special(x)`).
    fn special(&self, id: u32) -> bool;
    /// Whether the seed placed song index `song` at event slot `slot` of `game`
    /// (0 = OoT, 1 = MM). Default `true` (optimistic) so a state without the
    /// spoiler's song-event map never hides a check on this basis.
    fn song_event(&self, _game: u8, _slot: u8, _song: u8) -> bool {
        true
    }
    /// The current age: 0 = child, 1 = adult.
    fn age(&self) -> u8;
    /// Whether the required time of day is reachable. The tracker treats time as
    /// manipulable (Sun's Song / Song of Time), so this is `true` by default.
    fn time_reachable(&self) -> bool {
        true
    }
    /// A renewable source of the item exists. Approximated as "owned" until the
    /// solver models drop/shop sources (M3); good enough for most ammo checks.
    fn renewable(&self, id: u32) -> bool {
        self.item_count(id) >= 1
    }
    /// A license to use the item is held (bombchu bag, ...). Approximated as
    /// "owned" for now.
    fn license(&self, id: u32) -> bool {
        self.item_count(id) >= 1
    }
}

/// Evaluate one compiled rule against `w`. A malformed stream (bad emission)
/// would leave the stack empty; we default to `false` rather than panic so a
/// data bug degrades to "unreachable" instead of crashing the UI.
pub fn eval<W: WorldState + ?Sized>(expr: &[Op], w: &W) -> bool {
    let mut stack: Vec<bool> = Vec::with_capacity(8);
    for op in expr {
        match *op {
            Op::Const(b) => stack.push(b),
            Op::Has(id, n) => stack.push(w.item_count(id) >= n as u32),
            Op::Renewable(id) => stack.push(w.renewable(id)),
            Op::License(id) => stack.push(w.license(id)),
            Op::Event(id) => stack.push(w.event(id)),
            Op::Trick(id) => stack.push(w.trick(id)),
            Op::Setting(k) => stack.push(w.setting_enabled(k)),
            Op::SettingEq(k, v) => stack.push(w.setting_value(k) == Some(v)),
            Op::Age(a) => stack.push(w.age() == a),
            Op::OotTime(_) | Op::MmTime(_) => stack.push(w.time_reachable()),
            Op::Masks(n) => stack.push(w.mask_count() >= n),
            Op::Special(id) => stack.push(w.special(id)),
            Op::Flag(id, want) => stack.push(w.flag(id, want)),
            Op::SongEvent(g, slot, song) => stack.push(w.song_event(g, slot, song)),
            // Should never appear (BUILTIN_NAMES is empty); be conservative.
            Op::Builtin(_) => stack.push(false),
            Op::Not => {
                let a = stack.pop().unwrap_or(false);
                stack.push(!a);
            }
            Op::And => {
                let b = stack.pop().unwrap_or(false);
                let a = stack.pop().unwrap_or(false);
                stack.push(a && b);
            }
            Op::Or => {
                let b = stack.pop().unwrap_or(false);
                let a = stack.pop().unwrap_or(false);
                stack.push(a || b);
            }
            Op::Cond => {
                let c = stack.pop().unwrap_or(false);
                let b = stack.pop().unwrap_or(false);
                let a = stack.pop().unwrap_or(false);
                stack.push(if a { b } else { c });
            }
        }
    }
    stack.pop().unwrap_or(false)
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::collections::{HashMap, HashSet};

    /// A minimal in-memory world state for exercising the evaluator.
    #[derive(Default)]
    struct Mock {
        items: HashMap<u32, u32>,
        masks: u16,
        settings: HashMap<u32, Option<u32>>, // key -> Some(value) or None (just enabled)
        enabled: HashSet<u32>,
        tricks: HashSet<u32>,
        events: HashSet<u32>,
        flags: HashSet<u32>,
        specials: HashSet<u32>,
        age: u8,
        time: bool,
    }

    impl WorldState for Mock {
        fn item_count(&self, id: u32) -> u32 {
            self.items.get(&id).copied().unwrap_or(0)
        }
        fn mask_count(&self) -> u16 {
            self.masks
        }
        fn setting_value(&self, key: u32) -> Option<u32> {
            self.settings.get(&key).copied().flatten()
        }
        fn setting_enabled(&self, key: u32) -> bool {
            self.enabled.contains(&key)
        }
        fn trick(&self, id: u32) -> bool {
            self.tricks.contains(&id)
        }
        fn event(&self, id: u32) -> bool {
            self.events.contains(&id)
        }
        fn flag_on(&self, id: u32) -> bool {
            self.flags.contains(&id)
        }
        fn special(&self, id: u32) -> bool {
            self.specials.contains(&id)
        }
        fn age(&self) -> u8 {
            self.age
        }
        fn time_reachable(&self) -> bool {
            self.time
        }
    }

    fn base() -> Mock {
        Mock { time: true, ..Default::default() }
    }

    #[test]
    fn constants_and_boolean_folding() {
        let w = base();
        assert!(eval(&[Op::Const(true)], &w));
        assert!(!eval(&[Op::Const(false)], &w));
        // true && false
        assert!(!eval(&[Op::Const(true), Op::Const(false), Op::And], &w));
        // false || true
        assert!(eval(&[Op::Const(false), Op::Const(true), Op::Or], &w));
        // !false
        assert!(eval(&[Op::Const(false), Op::Not], &w));
    }

    #[test]
    fn has_respects_count() {
        let mut w = base();
        w.items.insert(0x8, 1); // one hookshot
        assert!(eval(&[Op::Has(0x8, 1)], &w));
        assert!(!eval(&[Op::Has(0x8, 2)], &w)); // needs longshot (count 2)
        w.items.insert(0x8, 2);
        assert!(eval(&[Op::Has(0x8, 2)], &w));
    }

    #[test]
    fn cond_selects_the_right_branch() {
        // cond(a, b, c): a ? b : c. Encoded a, b, c, Cond.
        let w = base();
        let expr_true = [Op::Const(true), Op::Const(true), Op::Const(false), Op::Cond];
        let expr_false = [Op::Const(false), Op::Const(true), Op::Const(false), Op::Cond];
        assert!(eval(&expr_true, &w)); // picks b = true
        assert!(!eval(&expr_false, &w)); // picks c = false
    }

    #[test]
    fn settings_age_flags_specials() {
        let mut w = base();
        w.age = 1; // adult
        assert!(eval(&[Op::Age(1)], &w));
        assert!(!eval(&[Op::Age(0)], &w));

        w.enabled.insert(3); // setting 3 on
        assert!(eval(&[Op::Setting(3)], &w));
        w.settings.insert(5, Some(2)); // setting 5 == value 2
        assert!(eval(&[Op::SettingEq(5, 2)], &w));
        assert!(!eval(&[Op::SettingEq(5, 1)], &w));

        w.flags.insert(4);
        assert!(eval(&[Op::Flag(4, true)], &w)); // flag_on(4)
        assert!(!eval(&[Op::Flag(4, false)], &w)); // flag_off(4) is false when set
        assert!(eval(&[Op::Flag(9, false)], &w)); // an unset flag is "off"

        w.specials.insert(0);
        assert!(eval(&[Op::Special(0)], &w));
    }

    #[test]
    fn time_is_optimistic_but_configurable() {
        let mut w = base();
        assert!(eval(&[Op::OotTime(0)], &w));
        w.time = false;
        assert!(!eval(&[Op::MmTime(1)], &w));
    }

    /// Every emitted expression evaluates without panicking against a fixed
    /// state — guards the stack machine against an unbalanced emission.
    #[test]
    fn all_real_exprs_evaluate() {
        let w = base();
        for expr in crate::data::EXPRS {
            let _ = eval(expr, &w);
        }
    }
}
