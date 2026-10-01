//! UI drawing code, split by feature. Each submodule adds an
//! `impl TrackerApp` block; the struct itself lives in `main.rs`.

pub(crate) mod kbdnav;
mod menu;
mod settings_window;
mod launch;
mod map;
mod entrance;
mod progression;
mod gps;
