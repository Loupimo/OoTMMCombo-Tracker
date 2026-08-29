# OoTMMCombo-Tracker

An auto-tracker for the [OoTMM combo](https://ootmm.com/) randomizer.\
It automatically detects which location you collect and displays the item it contained on interactive scene maps — just like filling in the spoiler log yourself, but live, graphical and effortless.

As of **3.0.0** the shipped tracker is a **Rust + [egui](https://github.com/emilk/egui)** application (`rust-tracker/`); the original Qt/C++ tracker now lives in `C++-Tracker/`. **Releases are built from the Rust code.**

In **singleplayer**, tracking works through **hooking**: a tracking DLL runs inside a custom Project64 build and intercepts game events in real time (no lua adapter script needed). The DLL is now **loaded by Project64 itself** — the tracker copies it into PJ64's `Plugin/` folder and asks PJ64 to load it — so there is **no external injector anymore** and **no antivirus false-positive**.\
In **multiplayer**, it is a fork of the [multi-client](https://github.com/OoTMM/multi-client) written by Nax and talks to a remote server, exactly as before.

# Requirements

- **[Project64-EM](https://github.com/OoTMM/Project64-EM/releases)** — a `1.0.3-PJ` or `1.1.0-PJ` build is required (these are the only ones the hook supports).
- `PJ64OoTMMTracker.dll` must be in the **same folder** as the tracker. When you start tracking, the tracker copies it into Project64's `Plugin/` folder and has PJ64 load it automatically — **no separate injector is needed**.
- **Project64 must be in windowed mode (not fullscreen) when you start the tracker** — see [How to use](#how-to-use).
- **OoTMM build**: the tracker supports **stable releases up to v32.3** and **dev** builds, and should keep working with newer dev builds.

# Features

- Real-time item tracking for both **Ocarina of Time** and **Majora's Mask** on interactive scene maps
- Full **entrance randomizer** tracking — entrances, grottos, warp songs, spawns, deaths — with entrance maps, region/entrance trees and a GPS view
- A **progression tab** listing every item and the locations where each one can be found
- **Reachability / accessibility logic**: optionally dim or hide the checks you can't reach yet, computed from your current items and the seed settings (entrance-shuffle aware)
- **Spoiler-log support**: reveal uncollected item locations and pre-mark starting items
- **Master Quest** and **Majora's Mask JP** layouts are supported
- **Coop, multiplayer and multiworld** are supported (per-world scene maps + a world selector)
- ROM build parameters are parsed automatically from the spoiler log to enable/disable the right locations
- **English and French** UI (in-app language switch)
- Auto-save: progress is saved every time an item is collected, in a human-readable **XML** save; each seed gets its own file under `autosave/` (manual save is also available)

# How to use

## Playing in singleplayer

1. Launch **Project64-EM** (a `1.0.3-PJ` or `1.1.0-PJ` build) and start your OoTMM ROM.\
   **Keep Project64 in windowed mode — not fullscreen** (see the note below).
2. In the tracker's **"Launch"** tab, click **"Start Tracking"** — the tracker copies its DLL into PJ64's `Plugin/` folder and has PJ64 load it.
3. Create a new save and start playing. Collected items appear on the maps automatically.

> **⚠ Project64 must be windowed (not fullscreen) at the moment you click "Start Tracking".**\
> To load its plugin, the tracker sends **Ctrl+T** to the Project64 window (this makes PJ64 enumerate and load its plugins). A fullscreen Project64 swallows the keystroke, so the DLL never loads. Once tracking has started you can switch back to fullscreen. The journal shows this note when injection begins.

There is an auto-saving feature, enabled by default. When an item is collected the progress is saved.\
You can also save your progress manually.

## Playing in multiplayer

Your ROM must have been created with the **coop** or **multiworld** parameter.

In the **"Launch"** tab, check the **"Use multiplayer"** box and enter the server address and port you want to use.\
If you don't have your own server, just leave the default one.\
Then follow the **"Playing in singleplayer"** steps above.

## Loading a spoiler log (strongly recommended)

Importing the seed's spoiler log is **strongly recommended**. It is the most reliable way to detect the ROM build (stable vs dev), and it is the **only** way the tracker learns the world layouts — Master Quest and Majora's Mask JP — which the hook cannot detect on its own.

It also lets the tracker show the expected item on each location, reveal uncollected item locations, and account for starting items.

## Side notes

- **Project64 must be windowed when you start tracking** (fullscreen swallows the Ctrl+T that loads the plugin) — see the note in the singleplayer section.
- Tracking is **live**: items collected while the tracker is not running (or before the plugin loads) are not detected.
- Master Quest and Majora's Mask JP layouts are supported, but the hook **cannot** detect them — they are read from the spoiler log, so import it to get the correct layouts (see the spoiler-log section above).
- Multiworld is fully supported — use the world selector to browse any world's map and progression. Reachability/accessibility assumes a single shared settings/entrance/MQ layout across all worlds (the standard combo case); only the item routing is per-world.

## Troubleshooting

**"Start Tracking" does nothing / the plugin never loads.** Project64 was most likely in **fullscreen** when you started tracking. To load its plugin the tracker sends **Ctrl+T** to the Project64 window, and a fullscreen PJ64 swallows the keystroke. Put Project64 in **windowed mode**, then click **Start Tracking** again (you can go back to fullscreen once it's connected). The tracker's journal prints a reminder when injection begins.

**Antivirus false-positives** are no longer expected: the standalone `PJ64Injector.exe` is gone, and Project64 now loads the tracking DLL itself, so the injection pattern that used to trip antivirus software is no longer used. If your antivirus still quarantines a file, add an exclusion for the tracker's folder (Windows: *Settings → Privacy & security → Windows Security → Virus & threat protection → Manage settings → Add or remove exclusions*).

# For the dev

## Rust tracker (`rust-tracker/`) — the shipped app

**Releases are built from this project.** It is a Rust + egui application; the executable is named `OoTMMCombo-Tracker`.

```bash
cd rust-tracker
cargo run --release      # run it
cargo build --release    # build target/release/OoTMMCombo-Tracker.exe
cargo test               # unit tests
```

- Requires the Rust MSVC toolchain (`winget install Rustlang.Rustup`, then `rustup default stable-x86_64-pc-windows-msvc`).
- The tracker is **x86_64**; Project64 / the DLL are **x86 (32-bit)**. This is fine because the shared-memory struct is pointer-free, so its layout is identical across both bitnesses — keep it that way.
- Most of `src/data/*.rs` (object pools, scenes, entrances, progression, item table) and `src/data/logic.rs` are **generated**, not hand-written:
  - `python tools/gen_data.py` regenerates the game-data tables from the C++ sources / CSVs.
  - `python tools/gen_logic.py` compiles the reachability rules from the root `Logic/` folder.
  - Hand-written logic (`progression.rs`, `state.rs`, `i18n.rs`, `settings.rs`, `locales/*.toml`, …) is **not** regenerated.
- See [`rust-tracker/README.md`](rust-tracker/README.md) for the module map and the data-generation details.

## Legacy Qt/C++ tracker (`C++-Tracker/`)

The original tracker is kept in `C++-Tracker/` but is **no longer the basis for releases**. It is built with **Visual Studio 2022 (v143)** — no CMake, no Makefile — and contains two projects that build independently:

- **Main tracker app** (`C++-Tracker/`, root `.vcxproj`) — the Qt GUI application
- **PJ64OoTMMTracker** (`C++-Tracker/PJ64Tracking/PJ64OoTMMTracker/`) — the Windows DLL that hooks Project64 internals (**still C++ — the Rust tracker uses this same DLL over the same shared-memory contract**)

The standalone `PJ64Injector` project has been **removed**: Project64 now loads the DLL itself.

Building the Qt app requires **[Qt 6.9.1](https://www.qt.io/development/download-qt-installer-oss)** installed and linked (the Qt libraries are dynamically linked).
The installation settings used are the following

<img width="926" height="641" alt="image" src="qt-conf.png" />

Once you have built the project you can use the following command to run the .exe without the need of the IDE:
- **For the release version** : windeployqt --release <path_to_the_release_folder>/OoTMMCombo-Tracker.exe
- **For the debug version** : windeployqt --debug <path_to_the_debug_folder>/OoTMMCombo-Tracker.exe

# 📜 Licenses used in this project

## Main license
This project is licensed under the **GNU General Public License v3.0 (GPLv3)**. See the `LICENSE` file for more information.

## Qt and the LGPL
The **legacy C++ tracker** (`C++-Tracker/`) uses the **Qt** library, which is distributed under the **LGPL v3** license.
Under the terms of the LGPL, users are allowed to **replace or modify** the Qt libraries used.
The shipped **Rust tracker does not use Qt** — it is built with egui/eframe and other Rust crates under permissive licenses (mostly **MIT / Apache-2.0**); their licenses are reproduced in the crates and in `cargo`'s output.

The Qt libraries can be obtained from the official website:
[https://www.qt.io/download](https://www.qt.io/download)

### LGPL obligations:
- The Qt libraries used in this application are **dynamically linked**.
- The user may replace these libraries with another compatible version.
- The LGPL v3 license is included in the file `LGPL-3.0.txt`.

### MIT obligations:
Some of the code in this project comes from software under **MIT License**.
- The multiplayer code is a fork of the [OoTMM multi-client](https://github.com/OoTMM/multi-client) project — both in the legacy tracker (`C++-Tracker/Sources/Multi` and `C++-Tracker/Headers/Multi`) and in the Rust tracker (`rust-tracker/src/multi/` and `rust-tracker/src/multi_r4/`).
- The full MIT License is included in the file `MIT-LICENSE.txt`.

## License files
- `LICENSE` → Main license (GPLv3).
- `LGPL-3.0.txt` → Text of the LGPL v3 license (provided with Qt or downloadable [here](https://www.gnu.org/licenses/lgpl-3.0.txt)).
- `MIT-LICENSE.txt` → Multi-Client fork.