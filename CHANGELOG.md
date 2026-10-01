# Changelog

All notable changes to this project are documented in this file.

## [3.0.0] - 2026-10-01

This release is a full rewrite of the tracker from Qt/C++ to Rust + egui. Every 2.0.0 feature was ported over the same shared-memory contract, and the tracker now ships a reachability logic engine, OoTMM's new check identification system, gossip stones and a much more robust hooking DLL.

### Added
- Reachability logic engine: the OoTMM logic is compiled into the tracker and shows only the checks reachable with the items collected so far (dim or hide unreachable checks on the map and object tree, Options → Accessibility)
  - Honours every seed setting: open dungeons and conditions, key rings, silver rupee pouches, MM clocks and time of day, shared items, renewable items, pond fish shuffle, checks removed by settings, and more
  - Entrance-randomizer aware (any shuffle: interiors, grottos, dungeons, overworld, mixed, decoupled, spawns/warps), including progressive entrance discovery
  - Multiworld: each world's accessibility uses that player's own inventory
- New check format matching OoTMM's own system (compact xflag IDs on recent builds, legacy system kept for v32.3 and older)
- Gossip stones (fairies and big fairies) for both games, the remaining MM grottos, boulders / silver boulders, and Granny's blue-potion buy spot
- Full French translation backed by an in-app i18n system (FR/EN)
- Support for OoTMM stable builds up to v32.3 and for the latest dev builds, including their new items and settings (rusty keys, Powder Keg, GFS, stick & nut capacity, Razor / Gilded swords, …)
- Starting items and items removed by the seed's settings handled automatically (e.g. Gerudo Member Card with an open fortress, Ruto's / Zelda's letters, hideout keys)
- Multiplayer client for OoTMM 1.10 integrated directly into the tracker (items and entrances), with a fallback so a pickup is never lost when the multiplayer link drops
- GPS route finder over the entrance graph, cross-game (OoT ↔ MM), with warp songs, readable entrance names and auto-start from your current position and arrival entrance
- Region entrance table (scene, entrance, how to reach it, where it leads, status), sortable, with click-to-focus on the map
- Auto-follow: the map, item / entrance / GPS views follow the player's in-game position, and the scene and object trees unfold to it; the map switches automatically with the Adult/Child (OoT) or Spring/Winter (MM) context
- Full in-game timer tracking for every OoT and MM timer
- Heaviest caught fish shown under the fishing entries
- DLL health indicator in the status bar (heartbeat frozen, hook removed, automatic recoveries)
- Patch tied to the save, applied automatically when the launched game matches
- Native Save / Load / Load Spoiler / Reset workflow with a Start/Stop Tracking control, and per-seed timestamped autosaves
- Support for the PJ-EM 1.1.0 emulator build

### Changed
- Entire UI rewritten in Rust + egui: much faster startup and far lower resource use (idle CPU from ~15% to ~0%); the old C++ tracker moved to its own folder
- Injector-free DLL loading: Project64 loads the tracking DLL itself, so antivirus no longer flags it
- Hooking DLL optimized and updated for the latest OoTMM builds, catching every entrance on stable and dev builds
- Saves are now a human-readable XML format keyed on stable check identities, so they survive location renames; pre-3.0 `.trck` saves are still imported
- Map images converted from PNG to JPG; reworked Death Mountain Trail, Goron City and Road to Ikana layouts

### Removed
- The standalone PJ64Injector.exe

### Fixed
- Item tracking going silent after loading savestates (the DLL now recovers on its own)
- Many entrance issues: ER grottos, Bean grotto exit, Telescope entrances, Lone Peak Shrine, Spring Mountain context, first-game spawn, and more
- Missing item when loading a spoiler log from an entrance-randomizer seed
- Crashes when completing a Ganon Trial
- Progression tab: progressive items lighting every stage at once, wrong names for quantified pickups and capacity upgrades, rusty keys, quiver / bullet-bag locations
- Multiworld / coop tracking, Southern Swamp "cleared" state, path escaping during injection
- Various object placements, icons, and stable 31.1 / 32.0 version parsing

## [2.0.0] - 2026-06-02

### Added
- DLL-based memory hooking system injected into Project64 to track items, entrances, and game state in real time. This means that solo game can also be tracked !
- Pattern detection to auto-locate game memory addresses across all ROM versions, with a fallback triggered on actor spawn for unstable functions
- Per-game tabs with interactive scene maps for both OoT and Majora's Mask
- Scene maps and item layouts for every OoT dungeon: Deku Tree, Dodongo's Cavern, Jabu-Jabu, Forest, Fire, Water, Shadow, Spirit, Ice Cavern, Bottom of the Well, Gerudo Training Ground, Ganon's Castle (with MQ / MM JP variants)
- Hundreds of item icons for OoT and MM, including the full set of MM masks
- Item filtering: show/hide collected items, category filter, search bar, dedicated bush filter for MM, MM Lottery tracking
- Settings panel that parses ROM build parameters and toggles MQ / MM JP layouts, with save/load of user filters
- `Reset Tracking` button and a routine to reset counters to match available tracked objects
- Full entrance tracking: regular entrances, grottos (entry and outside exit with player position), warp songs (Sun's Song, Song of Time, Song of Double Time, Song of Soaring, Farore's Wind for both games), Moon Crash, OoT end credits
- Entrance visualization on scene maps with arrows, highlight, auto-positioned text, and per-scene minimaps (DMC, Gerudo Valley, …)
- Entrance tree, region tree, search bar, counters, and an `AllEntranceView` table with per-region filtering
- Click-to-zoom interaction between entrance map and entrance tree (double-click, unknown-entrance highlight, cell-click center action)
- Entrance save/load, region IDs on entrances, color status, manual override of text position on the map
- Support for Nothing shop items, butterflies (with `EnButte_TransformIntoFairy` hook), fairies, and big fairies
- Link age tracked through the hook
- Progression tab listing every item, with forced item discovery wired to save loading
- Detail panel under the progression tab showing the locations where each item can be found
- Shared-item dispatch across the progression tab using a per-item `CanBeShared` flag
- Parsing of starting items from the ROM settings, with non-shuffled items hidden in the progression tab
- Setting to reveal uncollected item locations when a spoiler log is loaded (drives both the per-object UI and the progression detail panel)
- Per-object UI in the scene tree displaying the object's own icon and the items it contains
- Progress bar on the global object counter and an interactive status line
- All OoT overworld scene maps, plus a fallback minimap file covering scenes that don't have a dedicated map
- MM scene minimaps (path-only outlines for scenes still missing a full map)
- Hover highlighting of group boxes, scene anchors and matching rows in the entrance tree
- Clicking a scene anchor now focuses the associated entrance group text box
- Multiworld support: per-world scene objects and a world selector to browse each world's map and progression
- Stable and dev ROM build support: raw in-game item IDs are translated to the tracker's internal numbering so both builds track items correctly
- Dev-build items and the latest dev ROM settings options
- Coop propagation of "nothing" item drops over the network so the whole team's shared map stays in sync
- In-game run timer tracking

### Changed
- Network tracking automatically disabled when multiplayer is unchecked
- Console font switched to Consolas for readability
- C++ source files reorganized so function order matches their headers, and code clean-up passes across hook, injector, and entrance modules
- Active layout (MQ / MM JP) now also filters the entrance set
- Entrance graphical style rewritten with refreshed icons and hovered-background tinting
- More reliable death detection
- Memory hooking updated to follow the latest dev OoTMM build
- Progression lookup keyed by item ID instead of regex-matched names
- `ObjectScene` split into one `.cpp` per scene and progression data extracted into its own file
- New `GameIcons` class centralises pixmap creation for lower memory use and faster startup

### Removed
- Unused image assets

### Fixed
- Crash when Project64 is closed before the tracker is stopped
- Crash on butterfly-to-fairy transformation after a savestate load
- Item overwrite when a valid item already existed on an object
- Caught-by-guard cutscene incorrectly interpreted as a death warp
- Many MM entrances: Market, Hyrule Castle, Temple of Time, Back Alley, Mask Shop ↔ Clock Town, Ikana Castle, Deku Palace grottos, Great Bay, Zora Cape, Pirate Fortress, and assorted others
- Many OoT entrances: Silo, all OoT grotto spawns, bazaar / fairy fountain split, defense upgrade → Ganon's Castle exterior, boss temple one-way in/out, Koume's ride, and the full grotto exit pass
- `gLastScene` hook for both OoT and MM
- Farore's Wind handling for both games
- Sun's Song and death checker
- Item tree search bar showing empty / wrong layout when the search was cleared
- Filter loading and reset-of-excluded-objects bugs
- Visual bug in the search bar
- Multiple ASM hook bugs introduced during the optimization passes
- Multi-client buffer overflow and miscellaneous Linux/Mac build warnings
- Renewable item handling (multi)
- DMT scene typo and various other text typos
- Lake Hylia entrance handling
- Cross-references between objects and items
- Restored entrance table indicators
- Spoiler log now correctly reloads every item
- Missing entries in the ROM settings options
- Several UI color bugs and visual glitches around the search bar and Hyrule Field anchors
- Multiworld / coop tracking bug
