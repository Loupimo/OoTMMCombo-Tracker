//! Minimal GPS: shortest scene path over the entrance graph (phase 3).
//!
//! A BFS over vanilla `from_scene -> to_scene` entrance edges, with any
//! discovered live connection overriding the vanilla destination of that
//! entrance. The graph spans BOTH games: a discovered OoT<->MM link crosses the
//! boundary, so a route can go from an OoT scene to an MM one and back.
//! Unweighted (hop count); the measured `entrance_costs` times are a later
//! refinement of `GPSPathfinder`.

use std::collections::{HashMap, HashSet, VecDeque};

use crate::scene::Game;

/// One step of a route: the entrance taken and the (game, scene) it leads to.
pub struct Step {
    pub game: Game,
    pub scene: u16,
    pub entrance: &'static str,
}

/// Shortest path from `start` to `target`, both `(game, scene)`. `out_links` are
/// the discovered live EntranceLinks (entrance -> (game, destination entrance))
/// that override the vanilla destination and can cross OoT<->MM. Returns the
/// ordered steps, or None if unreachable with what has been discovered so far.
pub fn route_cross(
    start: (Game, u16),
    target: (Game, u16),
    out_links: &HashMap<(Game, u32), (Game, u32)>,
) -> Option<Vec<Step>> {
    if start == target {
        return Some(Vec::new());
    }

    let mut prev: HashMap<(Game, u16), ((Game, u16), &'static str)> = HashMap::new();
    let mut seen: HashSet<(Game, u16)> = HashSet::from([start]);
    let mut queue = VecDeque::from([start]);

    while let Some((g, s)) = queue.pop_front() {
        for e in g.entrances() {
            if e.from_scene != s {
                continue;
            }
            // A discovered connection reroutes this exit (possibly to the other
            // game); otherwise it lands at its vanilla destination scene.
            let dest = match out_links.get(&(g, e.to_id)) {
                Some(&(lg, lid)) => (
                    lg,
                    crate::entrance::lookup(lg, lid).map(|d| d.to_scene).unwrap_or(e.to_scene),
                ),
                None => (g, e.to_scene),
            };

            if seen.insert(dest) {
                prev.insert(dest, ((g, s), e.to_name));
                if dest == target {
                    return Some(reconstruct(&prev, start, target));
                }
                queue.push_back(dest);
            }
        }
    }
    None
}

fn reconstruct(
    prev: &HashMap<(Game, u16), ((Game, u16), &'static str)>,
    start: (Game, u16),
    target: (Game, u16),
) -> Vec<Step> {
    let mut steps = Vec::new();
    let mut cur = target;
    while cur != start {
        let (from, entrance) = prev[&cur];
        steps.push(Step { game: cur.0, scene: cur.1, entrance });
        cur = from;
    }
    steps.reverse();
    steps
}
