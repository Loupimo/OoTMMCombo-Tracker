# Changelog

All notable changes to this project are documented in this file.

## [3.0.0] - 2026-08-28

This release is a full rewrite of the tracker from Qt/C++ to Rust + egui. Every 2.0.0 feature was ported over the same shared-memory contract (so the hooking DLL stays compatible), with the performance gains and new capabilities listed below.

### Added
- Full French translation, backed by an in-app i18n system and FR/EN locale files (every UI string localized)
- Reachability / accessibility logic engine: the OoTMM logic is compiled into the tracker and used to show only the checks reachable from the items collected so far; toggle between dimming and hiding unreachable checks on the map and object tree (Options → Accessibility)
- Entrance-randomizer awareness in reachability: the solver reroutes region edges from the spoiler's Entrances section, so accessibility stays correct under any entrance shuffle (interiors, grottos, dungeons, overworld, mixed, decoupled, spawns/warps)
- Multiworld reachability: the shown world's accessibility uses that player's routed inventory
- Support for OoTMM stable builds up to v32.3 and for dev builds, with raw in-game item IDs shifted to the tracker's numbering so both track correctly
- Dev-only item list and the latest dev ROM-setting options
- New items and ROM parameters through the adult mask (rusty keys, Powder Keg, GFS, stick & nut capacity, and more)
- Boulder and silver-boulder tracking for both games: placeholders, settings, filters, and map positions across the OoT and MM overworlds
- The new r4 multiplayer Go client integrated directly into the tracker (dev only), plus multi-source support
- Load a patch tied to the save, applied automatically when the launched game matches
- GPS scene routing over the entrance graph, now cross-game (OoT ↔ MM), with warp-song handling, a search field, and cross-game routing
- Full in-game timer tracking for every OoT and MM timer
- Setting to auto-load the map item / entrance / GPS view at the player's live in-game position
- Region entrance table (per region: scene, entrance, how to reach it, where it leads, and a status dot), with sortable and resizable columns and click-to-focus on the map
- Native Save / Load / Load Spoiler / Reset workflow with a Start/Stop Tracking control on the Launch page
- Per-seed autosaves: each seed gets its own file in a new `autosave/` folder (named from the spoiler's seed hash), so switching seeds keeps every playthrough's progress; runs with no spoiler share a single `empty` file
- Support for the PJ-EM 1.1.0 emulator build
- Granny's blue-potion buy spot as a tracked location

### Changed
- Entire tracker UI rewritten from Qt/C++ to Rust + egui: much faster startup and far lower resource use (idle CPU dropped from ~15% to ~0%, mouse-move interaction capped to display rate), with the shared-memory link moved onto a background poller so the UI is genuinely idle when nothing changes; the old C++ tracker moved to its own folder
- Injector-free DLL loading: Project64 now loads the tracking DLL itself instead of relying on the external injector, so antivirus no longer flags it, and the DLL loads and unloads cleanly inside the PJ64 process
- Hooking DLL revised to catch every entrance on both stable and dev builds, and updated to follow the latest dev build
- gLastScene address resolution made more robust for wider version compatibility
- Tracker saves are now a human-readable, hand-editable XML format instead of an opaque binary. Each check is keyed on its stable numeric identity (object id within its scene, split by type and layout) so a save survives location renames between versions, with the Location string as a fallback; placed items also store their stable item id, and entrance links record the fully-qualified "scene - side" name of the resolved exit. Pre-3.0 Qt `.trck` binaries and the earlier autosave are still imported
- All map images converted from PNG to JPG
- Reworked Death Mountain Trail, Goron City and Road to Ikana layouts to fit the boulder additions, and adjusted the OoT trade-quest icons

### Removed
- The standalone PJ64Injector.exe — the tracker no longer needs an external injector

### Fixed
- Many entrance issues: ER grotto data and the gLast entrance, the Bean grotto exit, a Kakariko entrance overlapping the map, Telescope entrances, Lone Peak Shrine and Spring Mountain context, and assorted entrance typos
- Grotto entrance tracking, including several entrance-randomizer-specific cases
- Missing item when loading a spoiler log from an entrance-randomizer seed
- Crashes when completing a Ganon Trial
- First-game spawn handling and MM grotto scenes
- Progression-tab bugs: quiver / bullet-bag locations from a spoiler log, non-shuffled items, and regressions from multiworld and the new item list
- Progression tab labelling the Deku Stick and Deku Nut capacity entries as "…Upgrade" instead of their real names
- Progression tab showing quantified pickup names (e.g. "5 Deku nuts", "Child Fish (2 pounds)") instead of the clean item family name
- Progressive items (ocarina, Deku stick capacity) lighting every stage on the first pickup instead of advancing one stage at a time
- Rusty keys collected but not reflected in the progression tab, now that spoiler names are mapped to the curated in-game item names
- Multiworld / coop tracking bug
- Race condition when emitting entrance updates
- Southern Swamp "cleared" state not being caught
- Clearer distinction between dev and stable builds in game
- Non-escaped path that could make injection fail
- Various object placements, icon fixes, and stable 31.1 / 32.0 version parsing

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
