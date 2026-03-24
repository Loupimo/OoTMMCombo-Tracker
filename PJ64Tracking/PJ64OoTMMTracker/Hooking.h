#pragma once

#define ROM_PTR_OFFSET 0x1B00CC             // The offset to get the ROM pointer structure
#define ROM_OFFSET 0x10                     // The offset to add to the ROM pointer structure to get the physical address of the loaded ROM.
#define MMU_PTR_OFFSET 0x1B00BC             // The offset to get the MMU pointer structure
#define MMU_OFFSET     0x1074               // The offset to apply to the MMU pointer structure to get the physical RAM address of the game
#define REG_PTR_OFFSET 0x1B00C4             // The offset to get the register structure
#define PC_OFFSET      0x220				// PC offset in the CPU structure
#define V0_OFFSET      PC_OFFSET + 0x18     // V0 offset in the CPU structure (relative to PC offset)
#define V1_OFFSET      PC_OFFSET + 0x20     // V1 offset in the CPU structure (relative to PC offset)
#define A1_OFFSET      PC_OFFSET + 0x30     // A1 offset in the CPU structure (relative to PC offset)
#define S0_OFFSET      PC_OFFSET + 0x88     // S0 offset in the CPU structure (relative to PC offset)
#define SP_OFFSET      PC_OFFSET + 0xF0     // SP offset in the CPU structure (relative to PC offset)
#define DROP_CUSTOM    -0x68                // The offset to add to SP to gather "Nothing" item objects on drop custom function
#define SHOP_CUSTOM    -0x40                // The offset to add to SP to gather "Nothing" item objects on shop function
#define BUTTERFLY_CUSTOM    -0x40           // The offset to add to SP to gather "Nothing" item objects on butterfly function
#define BUTTERFLY_FUNCTION   0x240          // The offset to add the S0 register to get the butterfly's action function PC value
#define BUTTERFLY_SPAWN_OFFSET   0x130      // The offset to add to the butterfly's action function to reach the GI_NOTHING test
#define HOOK_PC_OFFSET    0xE64C9		    // Instruction offset to hook for the PC updating
#define HOOK_PC_SIZE 7                      // Original instruction size of the hooked PC updating code
#define HOOK_ROM_LOAD_OFFSET 0xD5A9B        // Instruction offset to hook ROM loading to physical RAM address
#define HOOK_ROM_LOAD_SIZE  11              // Original instruction size of the hooked ROM loading code 
#define BUFFER_SIZE 1024
#define PC_MASK_SIZE (1 << 24)              // 24-bit address space
#define OOT_PLAY_MAIN 0x8009CAC8            // The OoT Play_Main function used to know if RAM is fully loaded.
#define MM_PLAY_MAIN  0x80168F64            // The MM Play_Main function used to know if RAM is fully loaded.


enum GameID : uint16_t
{
    GAME_OOT = 0x0000,
    GAME_MM = 0x0001,
    GAME_UNKNOWN = 0x0200
};

// ==============================
// Event buffer (lock-free)
// ==============================


enum PCType : uint8_t
{
    TYPE_NONE = 0,
    TYPE_COMBO = 1,
    TYPE_XFLAG = 2,
    TYPE_SHOP = 3,
    TYPE_BUTTERFLY = 4
};


typedef struct Event
{
    uint32_t PC;
    uint32_t Mem;
    uint32_t Query[3];
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
extern uint64_t gGameVersion;
extern uint16_t typemask[PC_MASK_SIZE];

bool IsPayloadPC(uint32_t PC);
void SetPCType(uint32_t pc, uint16_t type);
void InitTypeMask();
uintptr_t FindGameRAM();
uint64_t GetGameVersion();
GameID DetectCurrentGame();
void PeriodicGameCheck();
void HandlePCHook(uint32_t PC);
void PCHook();
void ROMHook();
void InstallPCHook();
void InstallROMHook();
void InstallHook(size_t HookOffset, size_t HookSize, uintptr_t HookFunction, void** gateway, BYTE originalBytes[]);
void InstallHooks();