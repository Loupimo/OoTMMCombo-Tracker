#pragma once

// PJ64 Hooks
#define HOOK_ROM_LOAD_OFFSET    0xD5A9B     // Instruction offset to hook ROM loading to physical RAM address
#define HOOK_ROM_LOAD_SIZE           11     // Original instruction size of the hooked ROM loading code
#define HOOK_PC_OFFSET          0xE64C9		// Instruction offset to hook for the PC updating
#define HOOK_PC_SIZE                  7     // Original instruction size of the hooked PC updating code

// Registers, Memory, ROM
#define MMU_PTR_OFFSET         0x1B00BC     // The offset to get the MMU pointer structure
#define REG_PTR_OFFSET         0x1B00C4     // The offset to get the register structure
#define ROM_PTR_OFFSET         0x1B00CC     // The offset to get the ROM pointer structure

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

// Entrance stuff
#define OOT_SCENE_OFFSET     0x00001A68     // The offset to add to the real game address to reach the gLastScene for OoT
#define MM_SCENE_OFFSET     -0x00000118     // The offset to add to the real game address to reach the gLastScene for MM
#define OOT_NEXT_ENTRANCE    0x001DA2B8     // The offset to add to the real game address to reach the playState.nextEntrance for OoT
#define MM_NEXT_ENTRANCE     0x003FF398     // The offset to add to the real game address to reach the playState.nextEntrance for MM
#define OOT_LAST_SONG_ID     0x001D8966     // The offset to add to the real game address to reach the playState.msgCtx.lastPlayedSong for OoT
#define MM_LAST_SONG_ID      0x003FD452     // The offset to add to the real game address to reach the playState.msgCtx.songPlayed for MM
#define OOT_CURR_ROOM        0x001DA15F     // The offset to add to the real game address to reach the current room index for OoT
#define MM_CURR_ROOM         0x003FF203     // The offset to add to the real game address to reach the current room index for MM
#define OOT_PLAYER_COORD     0x001C8714     // The offset to add to the real game address to reach the player coordinates for OoT
#define MM_PLAYER_COORD      0x003E6DD4     // The offset to add to the real game address to reach the player coordinates for MM
#define OOT_GROTTO_DATA      0x0011B964     // The offset to add to the real game address to reach the gGrottoData for OoT
#define MM_GROTTO_DATA       0x001F3394     // The offset to add to the real game address to reach the gGrottoData for MM
#define IN_MAGIC             0xFFFFAAAA     // The magic flag that indicates the message is an incoming entrance
#define OUT_MAGIC            0xFFFFBBBB     // The magic flag that indicates the message is an outgoing entrance


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

// ==============================
// Event buffer (lock-free)
// ==============================


enum PCType : uint8_t
{
    TYPE_NONE = 0,
    TYPE_BUTTERFLY = 1,
    TYPE_FAIRY = 2,
    TYPE_COMBO = 3,
    TYPE_XFLAG = 4,
    TYPE_SHOP = 5,
};


typedef struct Event
{
    uint32_t PC;
    uint32_t Mem;
    uint32_t Query[6];
} Event;


struct SharedData
{
    uint32_t GameVersion[2];
    LONG MaxSize;
    volatile LONG CurrIndex;
    Event Buffer[BUFFER_SIZE];
};

extern SharedData* gData;
extern uintptr_t moduleBase;
extern uintptr_t regBase;
extern uintptr_t gameRAMBase;
extern GameID gGame;
extern uint8_t typemask[2][PC_RANGE_SIZE];
extern uint32_t * gActivePCs;

void TryResolveROMBase();
uintptr_t FindGameRAM();
void GetGameVersion();
GameID DetectCurrentGame();
void PeriodicGameCheck();
void PCHook();
void ROMHook();
void InstallPCHook();
void InstallROMHook();
void InstallHook(size_t HookOffset, size_t HookSize, uintptr_t HookFunction, void** gateway, BYTE originalBytes[]);
void InstallHooks();