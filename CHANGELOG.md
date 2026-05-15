# Changelog

All notable changes to this project are documented in this file.

## [2.0.0] - 2026-04-20

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

### Changed
- Network tracking automatically disabled when multiplayer is unchecked
- Console font switched to Consolas for readability
- C++ source files reorganized so function order matches their headers, and code clean-up passes across hook, injector, and entrance modules
- Active layout (MQ / MM JP) now also filters the entrance set
- Entrance graphical style rewritten with refreshed icons and hovered-background tinting
- More reliable death detection
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
