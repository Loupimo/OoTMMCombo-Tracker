//! Minimal GPS: shortest scene path over the entrance graph (phase 3).
//!
//! A BFS over vanilla `from_scene -> to_scene` entrance edges, with any
//! discovered live connection overriding the vanilla destination of that
//! entrance. Unweighted (hop count); the measured `entrance_costs` times are a
//! later refinement of `GPSPathfinder`.

use std::collections::{HashMap, HashSet, VecDeque};

use crate::scene::Game;

/// One step of a route: the entrance taken and the scene it leads to.
pub struct Step {
    pub scene: u16,
    pub entrance: &'static str,
}

/// Shortest scene path from `start` to `target`. `out_links` are the discovered
/// live EntranceLinks (entrance -> (game, destination entrance)) that override
/// the vanilla destination. Returns the ordered steps, or None if unreachable.
pub fn route(
    game: Game,
    start: u16,
    target: u16,
    out_links: &HashMap<(Game, u32), (Game, u32)>,
) -> Option<Vec<Step>> {
    if start == target {
        return Some(Vec::new());
    }

    // to_id -> the scene it actually lands in (discovered override).
    let dest_scene: HashMap<u32, u16> = game
        .entrances()
        .iter()
        .map(|e| (e.to_id, e.to_scene))
        .collect();

    let mut prev: HashMap<u16, (u16, &'static str)> = HashMap::new();
    let mut seen: HashSet<u16> = HashSet::from([start]);
    let mut queue = VecDeque::from([start]);

    while let Some(s) = queue.pop_front() {
        for e in game.entrances() {
            if e.from_scene != s {
                continue;
            }
            // A discovered connection reroutes this exit to another scene. Only
            // same-game links can be walked within this single-game BFS.
            let dest = out_links
                .get(&(game, e.to_id))
                .filter(|(g, _)| *g == game)
                .and_then(|(_, d)| dest_scene.get(d).copied())
                .unwrap_or(e.to_scene);

            if seen.insert(dest) {
                prev.insert(dest, (s, e.to_name));
                if dest == target {
                    return Some(reconstruct(&prev, start, target));
                }
                queue.push_back(dest);
            }
        }
    }
    None
}

fn reconstruct(prev: &HashMap<u16, (u16, &'static str)>, start: u16, target: u16) -> Vec<Step> {
    let mut steps = Vec::new();
    let mut cur = target;
    while cur != start {
        let (from, entrance) = prev[&cur];
        steps.push(Step { scene: cur, entrance });
        cur = from;
    }
    steps.reverse();
    steps
}
