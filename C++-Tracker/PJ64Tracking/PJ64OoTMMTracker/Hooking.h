#pragma once

// PJ64 Hooks
#define HOOK_ROM_LOAD_OFFSET            0xD5A9B     // Instruction offset to hook ROM loading to physical RAM address (PJ-EM v1.0.3)
#define HOOK_ROM_LOAD_OFFSET_V_1_1_0    0xD317B     // Instruction offset to hook ROM loading to physical RAM address (PJ-EM v1.1.0)
#define HOOK_ROM_LOAD_SIZE                   11     // Original instruction size of the hooked ROM loading code
#define HOOK_PC_OFFSET                  0xE64C9		// Instruction offset to hook for the PC updating (PJ-EM v1.0.3)
#define HOOK_PC_OFFSET_V_1_1_0          0xE4009		// Instruction offset to hook for the PC updating (PJ-EM v1.1.0)
#define HOOK_PC_SIZE                          7     // Original instruction size of the hooked PC updating code

// Registers, Memory, ROM
#define MMU_PTR_OFFSET         0x1B00BC     // The offset to get the MMU pointer structure (PJ-EM 1.0.3)
#define REG_PTR_OFFSET         0x1B00C4     // The offset to get the register structure (PJ-EM 1.0.3)
#define ROM_PTR_OFFSET         0x1B00CC     // The offset to get the ROM pointer structure (PJ-EM 1.0.3)
#define OFFSET_PTR_V_1_1_0       0x101C     // The offset to add to get the desired pointer structure for PJ 1.1.0

// Registers offsets
#define ROM_OFFSET                 0x10     // The offset to add to the ROM pointer structure to get the physical address of the loaded ROM.
#define MMU_OFFSET               0x1074     // The offset to apply to the MMU pointer structure to get the physical RAM address of the game
#define PC_OFFSET                 0x220		// PC offset in the CPU structure
#define V0_OFFSET      PC_OFFSET + 0x18     // V0 offset in the CPU structure (relative to PC offset)
#define V1_OFFSET      PC_OFFSET + 0x20     // V1 offset in the CPU structure (relative to PC offset)
#define A1_OFFSET      PC_OFFSET + 0x30     // A1 offset in the CPU structure (relative to PC offset)
#define S0_OFFSET      PC_OFFSET + 0x88     // S0 offset in the CPU structure (relative to PC offset)
#define S1_OFFSET      PC_OFFSET + 0x90     // S1 offset in the CPU structure (relative to PC offset)
#define SP_OFFSET      PC_OFFSET + 0xF0     // SP offset in the CPU structure (relative to PC offset)

// Function specific offsets
#define DROP_CUSTOM               -0x68     // The offset to add to SP to gather "Nothing" item objects on drop custom function
#define SHOP_CUSTOM               -0x40     // The offset to add to SP to gather "Nothing" item objects on shop function
#define OOT_ACTOR_ID               0x20     // The offset to add to SP to reach the spawned actor ID for OoT
#define MM_ACTOR_ID                0x08     // The offset to add to SP to reach the spawned actor ID for MM
#define BUTTERFLY_CUSTOM          -0x40     // The offset to add to SP to gather "Nothing" item objects on butterfly function
#define OOT_FAIRY_COMBO_OFFSET    -0x4C     // The offset to add to SP to reach the beginning of the fairy combo item
#define MM_FAIRY_COMBO_OFFSET     -0x84     // The offset to add to SP to reach the beginning of the fairy combo item

// Running stuff
#define BUFFER_SIZE                1024     // The number of event that can be store at the same time
#define PC_RANGE_START       0x80000000     // First possible PC to capture
#define PC_RANGE_SIZE        0x00800000     // 8 MB
#define OOT_PLAY_MAIN        0x8009CAC8     // The OoT Play_Main function used to know if RAM is fully loaded.
#define MM_PLAY_MAIN         0x80168f64     // The MM Play_Main function used to know if RAM is fully loaded.
#define OOT_PLAY_INIT        0x8009A750     // The OoT Play_Init function used to trigger the force game detection in case of game switch.
#define MM_PLAY_INIT         0x8016A2C8     // The MM Play_Init function used to trigger the force game detection in case of game switch.
#define DETECT_THROTTLE            8192

// Actor Stuff
#define OOT_FAIRY_ID         0x00000018     // The OoT fairy actor ID.
#define MM_FAIRY_ID          0x00000010     // The MM fairy actor ID. The real ID is 0x00150000, but as we need to add an offset of + 0x8 to find the function address I added a 8 at the end
#define OOT_BIG_FAIRY_ID     0x0000001A     // The OoT big fairy actor ID.
#define MM_BIG_FAIRY_ID      0x0000FFFF     // There is no MM big fairy.
#define OOT_BUTTERFLY_ID     0x0000001E     // The OoT butterfly actor ID.
#define MM_BUTTERFLY_ID      0x00000015     // The MM butterfly actor ID.
#define STABLE_NOTHING       0x0000033C     // The Nothing item ID for stable release (30.1).
#define DEV_NOTHING          0x000003AE     // The Nothing item ID for last dev release.

// Entrance stuff
//#define OOT_SCENE_OFFSET     0x00001A68     // The offset to add to the real game address to reach the gLastScene for OoT
//#define MM_SCENE_OFFSET     -0x00000118     // The offset to add to the real game address to reach the gLastScene for MM
//#define OOT_SCENE_OFFSET     0x00441A58     // The offset to add to the real game address to reach the gLastScene for OoT
//#define MM_SCENE_OFFSET      0x00770EC0     // The offset to add to the real game address to reach the gLastScene for MM
#define OOT_CURR_SCENE_OFFSET 0x001C8544     // The offset to add to the real game address to reach the current scene ID for OoT.
#define MM_CURR_SCENE_OFFSET  0x003E6BC4     // The offset to add to the real game address to reach the current scene ID for MM.
#define OOT_SCENE_OFFSET      0x00440000     // The offset to add to the real game address to reach the gLastScene for OoT. An offset is also added to reach the variable.
#define MM_SCENE_OFFSET       0x00770000     // The offset to add to the real game address to reach the gLastScene for MM. An offset is also added to reach the variable.
#define OOT_NEXT_ENTRANCE     0x001DA2B8     // The offset to add to the real game address to reach the playState.nextEntrance for OoT
#define MM_NEXT_ENTRANCE      0x003FF398     // The offset to add to the real game address to reach the playState.nextEntrance for MM
#define OOT_OWL_CHOICE_ID     0x001D8E4A     // The offset to add to the real game address to reach the playState.pauseCtx.cursorSlot[1] for OoT
#define MM_OWL_CHOICE_ID      0x003FDAB8     // The offset to add to the real game address to reach the playState.pauseCtx.cursorSlot[1] for MM
#define OOT_LAST_SONG_ID      0x001D8966     // The offset to add to the real game address to reach the playState.msgCtx.lastPlayedSong for OoT
#define MM_LAST_SONG_ID       0x003FD452     // The offset to add to the real game address to reach the playState.msgCtx.songPlayed for 
#define OOT_CURR_ROOM         0x001DA15F     // The offset to add to the real game address to reach the current room index for OoT
#define MM_CURR_ROOM          0x003FF203     // The offset to add to the real game address to reach the current room index for MM
#define OOT_PLAYER_COORD      0x001C8714     // The offset to add to the real game address to reach the player coordinates for OoT
#define MM_PLAYER_COORD       0x003E6DD4     // The offset to add to the real game address to reach the player coordinates for MM
#define OOT_GROTTO_DATA       0x0011B964     // The offset to add to the real game address to reach the gGrottoData for OoT
#define MM_GROTTO_DATA        0x001F3394     // The offset to add to the real game address to reach the gGrottoData for MM
#define IN_MAGIC              0xFA000000     // The magic flag that indicates the message is an incoming entrance
#define OUT_MAGIC             0xFB000000     // The magic flag that indicates the message is an outgoing entrance
#define OOT_FARORE_STATE      0x001DB09C     // The offset to add to real game address to reach the state flag used to determined if farore's wind has been used for OoT
#define MM_FARORE_STATE       0x0040081C     // The offset to add to real game address to reach the state flag used to determined if farore's wind has been used for MM
#define OOT_DEATH_STATE       0x001D8EC0     // The offset to add to real game address to reach the death state for OoT
#define MM_DEATH_STATE        0x003FDB20     // The offset to add to real game address to reach the death state for MM
#define FARORE_USED           0x30000000     // The value of the state flag when the farore's wind is used
#define OOT_LINK_AGE          0x001DA288     // The offset to add to real game address to reach link's age for OoT

#ifdef _DEBUG

#define LOG(fmt, ...) \
    printf("[LOG] " fmt "\n", __VA_ARGS__)

#else

#define LOG(fmt, ...)

#endif

enum GameID : uint8_t
{
    GAME_OOT = 0,
    GAME_MM = 1,
    GAME_UNKNOWN = 2
};

enum class TrackerCommand : uint8_t
{
    None = 0,       // Nothing to do, continue what you were doing
    Shutdown = 1    // The dll should stop and restore what have been patched (except for the ROM hook)
};

/* The Project64-EM fork version the DLL is injected into, read from the main window title. */
enum PJVersion : uint8_t
{
    EM_1_0_3 = 0,   // Default version assumed when the title cannot be parsed.
    EM_1_1_0 = 1
};


typedef struct Event
{
    uint32_t PC;
    uint32_t Mem;
    uint32_t Query[6];
} Event;


/* Tracker -> DLL : version deduite du spoiler ("Version:"). Override le CRC/offset si != UNKNOWN. */
#define HOST_VER_UNKNOWN 0
#define HOST_VER_DEV     1
#define HOST_VER_STABLE  2

struct SharedData
{
    uint32_t GameVersion[2];
    LONG MaxSize;
    volatile LONG CurrIndex;
    Event Buffer[BUFFER_SIZE];
    volatile int32_t HostROMVersion;    // Tracker -> DLL : voir HOST_VER_* (0 = inconnu / pas de spoiler)
    TrackerCommand Command;
};

extern HMODULE gSelfModule;                 // A reference to this module in order to keep it active after the dll has been loaded and to unload it properly.
extern SharedData* gData;                   // The shared data with the tracker
extern uintptr_t moduleBase;                // The module base address of Project 64
extern uintptr_t regBase;                   // The RAM address where the Project 64 registers are stored.
extern uintptr_t gameRAMBase;               // The RAM address where the game data are stored.
extern GameID gGame;                        // The current running game.
extern uint32_t * gActivePCs;               // The current set of program counters that are tracked.
extern uint32_t gActiveSceneOffset;         // The current offset to add to reach the last scene offset.
extern int16_t gOOTActiveGlobalOffset;      // An offset to add to the active scene offset to reach the gLastScene variable for OoT.
extern int16_t gMMActiveGlobalOffset;       // An offset to add to the active scene offset to reach the gLastScene variable for MM.
extern uint32_t gOOTLastSceneAddr;          // The actual address to get the gLastScene ID for OoT.
extern uint32_t gMMLastSceneAddr;           // The actual address to get the gLastScene ID for MM.
extern bool isStable;                       // Tell if the current game is stable release (true) or dev (false).
extern uint32_t gNothingID;                 // The current "Nothing" combo item ID (depends on stable/dev).
extern PJVersion gPJVersion;                // The Project64-EM fork version the DLL is injected into.

/*
*   Extract the version token located between the given prefix and suffix in a source string.
*
*   @param  Source      The string to search into.
*   @param  Prefix      The signature preceding the version token.
*   @param  Suffix      The delimiter following the version token.
*   @param  OutVersion  Buffer receiving the null-terminated version string.
*   @param  Size        Size of the output buffer in bytes.
*   @return true if a version token was successfully extracted, false otherwise.
*/
static bool ExtractVersion(const char* Source, const char* Prefix, const char* Suffix, char* OutVersion, size_t Size);

/*
*   Callback used by EnumWindows to locate the Project64 main window inside the current
*   process. It stops on the first top-level, visible window owned by this process whose
*   title carries the space-separated emulator signature.
*
*   @param  Handle  The window handle currently enumerated.
*   @param  LParam  Pointer to the HWND that receives the found main window.
*   @return FALSE to stop enumeration once found, TRUE to keep searching.
*/
static BOOL CALLBACK FindPJ64WindowProc(HWND Handle, LPARAM LParam);

/*
*   Read the Project64-EM version token. The main window title is the primary, location
*   independent source ("Project64-EM <version>-PJ-3.0.1"). The executable folder name
*   ("Project64-EM-<version>-PJ-3.0.1-win32") is used as a fallback and only works if the
*   user kept the distributed folder name.
*
*   @param  OutVersion  Buffer receiving the null-terminated version string.
*   @param  Size        Size of the output buffer in bytes.
*   @return true if a version token was successfully extracted, false otherwise.
*/
bool GetPJ64Version(char* OutVersion, size_t Size);

/*
*   Detect the Project64-EM fork version and store the result in gPJVersion. The main window
*   title ("Project64-EM <version>-PJ-3.0.1") is the primary, location independent source; the
*   executable folder name is used as a fallback. Defaults to EM_1_0_3 on failure.
*/
void DetectPJVersion();

/*
*   Attempt to resolve and cache the Project64 ROM base address.
*/
void TryResolveROMBase();

/*
*   Find the real game RAM address.
*
*	@return The real RAM address corresponding to the game start RAM address.
*/
uintptr_t FindGameRAM();

#ifdef _DEBUG

/*
*   Close the debug console previously opened for logging.
*/
void CloseDebugConsole();

#endif

/*
*   Thread routine that unloads this DLL once an unload has been requested.
*
*   @return The thread exit code.
*/
DWORD WINAPI DLLUnloadThread(LPVOID);
/*
*   Request this DLL to unload itself by spawning the unload thread.
*/
extern "C" void RequestDLLUnload();
/*
*   Check the command requested by the tracker in the shared data and act on it (e.g. shutdown).
*/
extern "C" void CheckTrackerCommand();
/*
*   Thread routine that watches for a shutdown request and triggers the DLL teardown.
*
*   @return The thread exit code.
*/
DWORD WINAPI ShutdownWatcherThread(LPVOID);
/*
*   Start the shutdown watcher thread.
*/
void StartShutdownWatcher();
/*
*   Stop the shutdown watcher thread.
*/
void StopShutdownWatcher();


/*
* Uses the version provided by the tracker (Share Data::HostROM Version, from the spoiler).
* If known, this takes precedence over CRC/offset detection for selecting the "Nothing" ID.
*/
void ApplyHostVersion();

/*
*   The program counter hook function used to track items and entrances.
*/
void PCHook();

/*
*   Tells if the ROM currently loaded at the given base is an OoTMM combo ROM.
*
*   @param ROMBase       The base address of the loaded ROM.
*
*   @return True if an OoTMM combo ROM is loaded, false otherwise.
*/
bool IsOoTMMROMLoaded(uintptr_t ROMBase);

/*
*   Thread routine that performs the ROM detection after a short delay.
*
*   @param lpParam       The thread parameter.
*
*   @return The thread exit code.
*/
DWORD WINAPI DelayedROMDetectionThread(LPVOID lpParam);

/*
*   Start the delayed ROM detection by spawning its worker thread.
*/
void StartDelayedROMDetection();

/*
*   Assembly-callable entry point that triggers the delayed ROM detection.
*/
extern "C" void StartDelayedROMDetectionASM();

/*
*   The ROM hook function used to catch game ROM changes.
*/
void ROMHook();

/*
*   Install the program counter hook.
*/
void InstallPCHook();

/*
*   Install the ROM hook.
*/
void InstallROMHook();

/*
*   Install the desired hook.
*
*   @param HookOffset		    The offset instruction to place the hook at.
*   @param HookSize			    The size of the hooked instructions.
*   @param Gateway			    The gateway used to execute the original hooked code.
*   @param OriginalBytes		The original code to execute.
*
*   @return True if the hook has been installed, false otherwise.
*/
bool InstallHook(size_t HookOffset, size_t HookSize, uintptr_t HookFunction, void** Gateway, BYTE OriginalBytes[]);

/*
*   Uninstall the ROM hook.
*/
void UninstallROMHook();

/*
*   Uninstall the program counter hook.
*/
void UninstallPCHook();

/*
*   Uninstall the desired hook.
*
*   @param HookOffset		    The offset instruction to place the hook at.
*   @param HookSize			    The size of the hooked instructions.
*   @param Gateway			    The gateway used to execute the original hooked code.
*   @param OriginalBytes		The original code to execute.
*
*   @return True if the hook has been uninstalled, false otherwise.
*/
void UninstallHook(size_t HookOffset, size_t HookSize, void* Gateway, BYTE OriginalBytes[]);

/*
*   Uninstall all the hooks installed by the dll.
*/
void ShutdownHooks();
