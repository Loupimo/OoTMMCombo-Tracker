#pragma once

#define OOT_PAYLOAD_RAM 0x8040000
#define MM_PAYLOAD_RAM 0x8073000
#define PAYLOAD_OFFSET MM_PAYLOAD_RAM - OOT_PAYLOAD_RAM
#define MMU_PTR_OFFSET 0x1B00BC             // The offset to get the MMU pointer structure
#define MMU_OFFSET     0x1074               // The offset to apply the MMU pointer structure to get the physical RAM address of the game
#define REG_PTR_OFFSET 0x1B00C4             // The offset to get the register structure
#define PC_OFFSET      0x220				// PC offset in the CPU structure
#define A1_OFFSET      PC_OFFSET + 0x30     // A1 offset in the CPU structure (relative to PC offset)
#define SP_OFFSET      PC_OFFSET + 0xF0     // SP offset in the CPU structure (relative to PC offset)
#define DROP_CUSTOM    -0x68                // The offset to add to SP to gather "Nothing" item objects on drop custom function
#define HOOK_OFFSET    0xE64C9				// Instruction offset to hook
#define HOOK_SIZE 7
#define BUFFER_SIZE 1024

// ==============================
// Event buffer (lock-free)
// ==============================
/*
typedef struct ComboItemQuery
{
    uint8_t  SceneId;   // 1 byte
    uint8_t  RoomId;    // 1 byte
    uint8_t  ID;        // 1 byte
    uint8_t  From;      // 1 byte
    uint8_t  Padding;   // 1 byte
    uint8_t  OVType;    // 1 byte
    uint16_t OVFlags;   // 2 bytes
    int16_t  GI;        // 2 bytes
    int16_t  GIRenew;   // 2 bytes
} ComboItemQuery;
*/

enum
{
    TYPE_NONE = 0,
    TYPE_COMBO = 1,
    TYPE_XFLAG = 2
};

typedef struct Event
{
    uint32_t PC;
    uint32_t A1;
    uint32_t Query[3];
} Event;


struct SharedData
{
    LONG MaxSize;
    volatile LONG CurrIndex;
    //uintptr_t Base;
    //uintptr_t GameRAMBase;
    Event Buffer[BUFFER_SIZE];
};

extern SharedData* gData;
extern uintptr_t moduleBase;
extern uintptr_t regBase;
extern uintptr_t gameRAMBase;

void SetPCType(uint32_t pc, uint8_t type);
void InitTypeMask();
uintptr_t FindGameRAM();
void Hook();
void InstallHook();