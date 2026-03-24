#include "pch.h"
#include "Hooking.h"
#include "PatternScanner.h"
#include "Cache.h"

// PC addresses to target
constexpr int MAX_TARGETS = 128;
constexpr uint32_t targetsPC_A1[MAX_TARGETS] = {
    0x8040C35C, // comboItemAddRawEx:   used to track all other objects for OoT
    0x8073BC50  // comboItemAddRawEx:   used to track all other objects for MM
};
constexpr uint32_t targetsPC_A1Count = sizeof(targetsPC_A1) / sizeof(targetsPC_A1[0]);

constexpr uint32_t targetsPC_SP[MAX_TARGETS] = {
    0x80400D60,  // EnItem00_DropCustom: if (o.gi == GI_NOTHING), used for tracking objects that drop a "Nothing" item for OoT. Here we capture a XFlag and used SP - 0x68
    0x80730FE0   // EnItem00_DropCustom: if (o.gi == GI_NOTHING), used for tracking objects that drop a "Nothing" item for MM. Here we capture a XFlag and used SP - 0x68
};
constexpr uint32_t targetsPC_SPCount = sizeof(targetsPC_SP) / sizeof(targetsPC_SP[0]);

constexpr uint32_t targetsPC_Shop[MAX_TARGETS] = {
    0x8040C168,  // comboItemPrecond: if (o.gi == GI_NOTHING), used for tracking buyable "Nothing" item for OoT. Here we capture a XFlag and used SP - 0x40
    0x8073BA5C   // comboItemPrecond: if (o.gi == GI_NOTHING), used for tracking buyable "Nothing" item for MM. Here we capture a XFlag and used SP - 0x40
};
constexpr uint32_t targetsPC_ShopCount = sizeof(targetsPC_Shop) / sizeof(targetsPC_Shop[0]);

constexpr uint32_t targetsPC_Butterfly[MAX_TARGETS] = {
    0x8041A040,  // Actor_Run_Update: used for tracking butterfly "Nothing" item for OoT. Here we check if the actor is a butterfly (id: 0x1E) using S0, if yes we look at the actor function to get the EnButte_SpawnFairy address and then apply an offset to get the EnButte_ShouldSpawnFairy address and then to check if the GI is "GI_NOTHING".
    0x8041E360  // Actor_Run_Update: used for tracking butterfly "Nothing" item for MM. Here we check if the actor is a butterfly (id: 0x15) using S0, if yes we look at the actor function to get the EnButte_SpawnFairy address and then apply an offset to get the EnButte_ShouldSpawnFairy address and then to check if the GI is "GI_NOTHING".
};
constexpr uint32_t targetsPC_ButterflyCount = sizeof(targetsPC_Butterfly) / sizeof(targetsPC_Butterfly[0]);

#define PC_MASK_SIZE (1 << 24)           // 24-bit address space
//#define TYPEMASK_SIZE (PC_MASK_SIZE / 4) // 4 entries per byte

uint16_t typemask[PC_MASK_SIZE];

// Hooking installation
void* gatewayPC = NULL;
void* gatewayROM = NULL;
BYTE originalBytesPC[HOOK_PC_SIZE];
BYTE originalBytesROM[HOOK_ROM_LOAD_SIZE];
//uintptr_t returnAddress = 0;

// Game data / shared memeory data
SharedData* gData = nullptr;
GameID gGame = GAME_OOT;
bool gIsRAMLoaded = false;
uint64_t gGameVersion = 0;
uintptr_t moduleBase = 0;
uintptr_t regBase = 0;
uintptr_t romBase = 0;
uintptr_t gameRAMBase = 0;
uintptr_t currButterflyPC = 0;
uintptr_t gSP = 0;
uintptr_t gV0 = 0;
uintptr_t gV1 = 0;
uintptr_t gS0 = 0;


bool IsPayloadPC(uint32_t PC)
{
    return PC >= PAYLOAD_START && PC < PAYLOAD_END;
}


void SetPCType(uint32_t pc, uint16_t type)
{
    typemask[pc & 0x00FFFFFF] = type;
}


uint64_t GetGameVersion()
{
    //printf("Get Game Version\n");
    if (!gData)
    {
        return 0;
    }
    
    uint64_t version = ((uint64_t)gData->GameVersion[0] << 32) | gData->GameVersion[1];

    if (version == 0)
    {
        version = *(uint64_t*)(romBase + 0x10);
        gData->GameVersion[0] = (uint32_t)version;
        gData->GameVersion[1] = version >> 32;
        printf("Game Version = %llu\n", version);
        gGameVersion = version;
    }
    
    return version;
}


GameID DetectCurrentGame()
{
    // OoT
    uintptr_t AddrCheck = gameRAMBase + (0x8011A5EC & 0x00FFFFFF);
    uint64_t val = *(uint64_t*)AddrCheck;

    if (val == 0x415A00005A454C44)
    {
        return GAME_OOT;
    }

    // MM
    AddrCheck = gameRAMBase + (0x801EF694 & 0x00FFFFFF);
    val = *(uint64_t*)AddrCheck;
    
    if (val == 0x413300015A454C44)
    {
        //printf("MM Game\n");
        return GAME_MM;
    }

    return GAME_UNKNOWN;
}


void PeriodicGameCheck()
{
    static uint32_t counter;

    counter++;

    if ((counter & 0x1FFF) != 0)
    {
        return;
    }

    counter = 0;

    GameID g = DetectCurrentGame();

    if (g != gGame && g != GAME_UNKNOWN)
    {
        gGame = g;
        gIsRAMLoaded = false;
        //BuildTypeMaskFromPatterns();
    }
}


void InitTypeMask()
{
    memset(typemask, 0, sizeof(typemask));

    // A1 cases
    for (size_t i = 0; i < targetsPC_A1Count; i++)
    {
        SetPCType(targetsPC_A1[i], TYPE_COMBO);
    }

    // Xflag cases
    for (size_t i = 0; i < targetsPC_SPCount; i++)
    {
        SetPCType(targetsPC_SP[i], TYPE_XFLAG);
    }

    // Shop cases
    for (size_t i = 0; i < targetsPC_ShopCount; i++)
    {
        SetPCType(targetsPC_Shop[i], TYPE_SHOP);
    }

    // Butterfly cases
    for (size_t i = 0; i < targetsPC_ButterflyCount; i++)
    {
        SetPCType(targetsPC_Butterfly[i], TYPE_BUTTERFLY);
    }
}


uintptr_t FindGameRAM()
{
    while (true)
    {
        uintptr_t obj = *(uintptr_t*)(moduleBase + 0x1B00BC);

        if (obj)
        {
            uintptr_t gameRAM = *(uintptr_t*)(obj + 0x1074);

            if (gameRAM && gameRAM > 0x10000000) // sanity check
                return gameRAM;
        }

        Sleep(100);
    }
}

void HandlePCHook(uint32_t PC)
{
    if (gGame != GAME_UNKNOWN)
    {
        if (gIsRAMLoaded)
        {   // The RAM is loaded

            PeriodicGameCheck();

            PCType type = (PCType)typemask[PC & 0x00FFFFFF];
            switch (type) {
                case TYPE_COMBO:
                    // read A1, SP, etc...
                    break;
                case TYPE_XFLAG:
                    // read SP...
                    break;
                case TYPE_SHOP:
                    // read V0, V1, SP...
                    break;
                case TYPE_BUTTERFLY:
                    // read S0...
                    break;
                default: break;
            }
        }
        else if ((gGame == GAME_OOT && PC == OOT_PLAY_MAIN) || (gGame == GAME_MM && PC == MM_PLAY_MAIN))
        {   // The RAM is loaded. We can check for patterns

            gIsRAMLoaded = true;

            printf("RAM Loaded : 0x%08X\n", PC);

            uint64_t version = GetGameVersion();

            GamePatternState* state = &gPatternState[gGame];

            if (!state->Resolved || state->Version != version)
            {
                BuildTypeMaskFromPatterns();
            }
        }
    }
    else
    {
        gGame = DetectCurrentGame();
        if (gGame != GAME_UNKNOWN)
        {   // The RAM is loaded so is the ROM

            gIsRAMLoaded = false;
            gGameVersion = GetGameVersion();
            if (gPatternState[GAME_OOT].Version != gGameVersion)
            {
                gPatternState[GAME_OOT].Resolved = false;
                gPatternState[GAME_MM].Resolved = false;
            }
            gPatternState[GAME_OOT].Version = gGameVersion;
            gPatternState[GAME_MM].Version = gGameVersion;
        }
    }
}




__declspec(naked) void ROMHook()
{
    __asm
    {
        // -------------------------
        // Save context (pushad, flags handled)
        // -------------------------
        pushad

        // -------------------------
        // ROM base / gData setup
        // -------------------------
        mov esi, ebx
        mov romBase, esi
        mov edi, gData
        mov eax, [esi]
        mov ecx, [esi + 4]
        mov[edi], eax
        mov[edi + 4], ecx

        // -------------------------
        // Call C++ initialization
        // -------------------------
        //call GetGameVersion
        //nop

        // -------------------------
        // Initialize typemask (lazy, will resolve patterns)
        // -------------------------
        //call InitHooking
        //nop

        
        popad
        jmp gatewayROM // trampoline to original code
    }
}

__declspec(naked) void PCHook()
{
    __asm
    {
        pushad

        // -------------------------
        // Read PC
        // -------------------------
        mov eax, regBase
        mov eax, [eax]
        mov ecx, [eax + PC_OFFSET]   // PC
        mov edx, [eax + SP_OFFSET]   // SP
        mov esi, ecx                 // save PC

        // -------------------------
        // Call C++ dispatcher
        // -------------------------
        push esi       // push PC
        call HandlePCHook
        add esp, 4     // cleanup

        popad
        jmp gatewayPC
    }
}
/*
__declspec(naked) void PCHook()
{
    __asm
    {
        // =========================
        // Save context. ! Important: do not save flags with pushfd otherwise it will crash !
        // =========================
        pushad

        // =========================
        // Get CPU struct
        // =========================
        mov eax, regBase
        mov eax, [eax]

        // =========================
        // Get PC / SP
        // =========================
        mov ecx, [eax + PC_OFFSET]   // PC
        mov edx, [eax + A1_OFFSET]   // A1
        mov esi, [eax + SP_OFFSET]   // SP
        mov edi, ecx                 // Save PC value in edi
        mov ebp, edx                 // Save A1 value in ebp
        mov gSP, esi                 // Save SP
        mov esi, [eax + V0_OFFSET]   // V0
        mov gV0, esi                 // Save V0
        mov esi, [eax + V1_OFFSET]   // V1
        mov gV1, esi                 // Save V1
        mov esi, [eax + S0_OFFSET]   // S0
        mov gS0, esi                 // Save S0

        // We need to do this before the type mask as the butterfly PC mask will always be TYPE_NONE as the PC is gathered on the fly
        cmp currButterflyPC, edi     // Use V0, V1 and SP register
        je USE_BUTTERFLY

        // =========================
        // Compute index
        // =========================
        mov eax, edi
        and eax, 0x00FFFFFF

        // =========================
        // Load typemask entry
        // =========================
        mov eax, edi
        and eax, 0x00FFFFFF

        mov esi, offset typemask
        mov bl, [esi + eax]

        cmp bl, TYPE_NONE            // Not tracked
        je EXIT

        cmp bl, TYPE_COMBO           // Use A1 register
        je USE_A1

        cmp bl, TYPE_XFLAG           // Use SP register
        je USE_SP

        cmp bl, TYPE_SHOP            // Use V0, V1 and SP register 
        je USE_SHOP

        cmp bl, TYPE_BUTTERFLY       // Use S0 register 
        je CHECK_BUTTERFLY

        jmp EXIT

        // =========================
        // USE A1 (ComboItemQuery)
        // =========================
        USE_A1:

            // =========================
            // Check A1 Range : Between 0x80000000 - 0x80FFFFFF
            // =========================
            mov eax, ebp
            and eax, 0xFF000000
            cmp eax, 0x80000000
            jne EXIT

            // =========================
            // Convert A1 to RAM offset
            // =========================
            mov eax, ebp
            and eax, 0x00FFFFFF

            mov esi, gameRAMBase    // The real game RAM base address 
            add esi, eax            // Add the offset to the game RAM base address

            // Read ComboItemQuery (12 bytes)
            mov eax, [esi + 4]   // q0
            mov edx, [esi + 8]   // q1
            mov ebx, [esi + 12]  // q2

            jmp STORE
            
        // =========================
        // USE SP (Xflag)
        // =========================
        USE_SP:

            mov ebp, gSP
            add ebp, DROP_CUSTOM

            // =========================
            // Check SP Range : Between 0x80000000 - 0x80FFFFFF
            // =========================
            mov eax, ebp
            and eax, 0xFF000000
            cmp eax, 0x80000000
            jne EXIT

            jmp CAPTURE_XFLAG

        // =========================
        // USE Shop (Xflag)
        // =========================
        USE_SHOP:

            // The item is buyable, therefore it is not a "Nothing item", we can exit
            mov eax, gV0
            cmp eax, 0x02
            jne EXIT

            // The item is not a nothing object, we can exit
            mov eax, gV1
            cmp eax, 0x033C
            jne EXIT

            mov ebp, gSP
            add ebp, SHOP_CUSTOM

            // =========================
            // Check SP Range : Between 0x80000000 - 0x80FFFFFF
            // =========================
            mov eax, ebp
            and eax, 0xFF000000
            cmp eax, 0x80000000
            jne EXIT

            jmp CAPTURE_XFLAG


        // =========================
        // Check Butterfly
        // =========================
        CHECK_BUTTERFLY:

            // =========================
            // Check S0 Range : Between 0x80000000 - 0x80FFFFFF
            // =========================
            mov ebp, gS0
            and ebp, 0xFF000000
            cmp ebp, 0x80000000
            jne EXIT

            // =========================
            // Convert S0 to RAM offset
            // =========================
            mov ebp, gS0
            and ebp, 0x00FFFFFF

            mov esi, gameRAMBase    // The real game RAM base address 
            add esi, ebp            // Add the offset to the game RAM base address

            // Check if the actor is a butterfly
            mov eax, [esi]
            and eax, 0xFFFF0000
            cmp eax, 0x001E0000     // OoT
            je MATCH_BUTTERFLY

            cmp eax, 0x00150000     // MM
            je MATCH_BUTTERFLY_MM

            jmp EXIT                // Not a butterfly

        MATCH_BUTTERFLY_MM:
            mov gGame, GAME_MM
            add esi, 0x08           // The MM actor structure has 8 bytes more than the OoT one

        MATCH_BUTTERFLY:

            // Set the butterfly PC
            mov eax, [esi + BUTTERFLY_FUNCTION]
            add eax, BUTTERFLY_SPAWN_OFFSET
            mov currButterflyPC, eax

            jmp EXIT

        // =========================
        // USE Butterfly (Xflag)
        // =========================
        USE_BUTTERFLY:
            // Reset the butterfly PC
            mov currButterflyPC, 0

            // The item is not a nothing object, we can exit
            mov eax, gV1
            cmp eax, 0x033C
            jne EXIT

            mov ebp, gSP
            add ebp, BUTTERFLY_CUSTOM

            // =========================
            // Check SP Range : Between 0x80000000 - 0x80FFFFFF
            // =========================
            mov eax, ebp
            and eax, 0xFF000000
            cmp eax, 0x80000000
            jne EXIT

            jmp CAPTURE_XFLAG

        // =========================
        // CAPTURE XFLAG (COMMON)
        // =========================
        CAPTURE_XFLAG:
            
            mov gGame, GAME_OOT

            // ========================
            // Check Game Version
            // ========================
            mov eax, 0x801EF694     // MM gPlayerState
            and eax, 0x00FFFFFF

            mov esi, gameRAMBase
            add esi, eax

            // Check "DLEZ"
            mov eax, [esi]
            cmp eax, 0x5A454C44
            jne CAPTURE

            // Check "3A"
            mov eax, [esi + 4]
            and eax, 0xFFFF0000     // Get ride of the death count
            cmp eax, 0x41330000
            jne CAPTURE

            mov gGame, GAME_MM

        CAPTURE:

            // =========================
            // Convert SP to RAM offset
            // =========================
            mov eax, ebp
            and eax, 0x00FFFFFF
            
            mov esi, gameRAMBase    // The real game RAM base address 
            add esi, eax            // Add the offset to the game RAM base address
            
            mov eax, [esi]          // Key
            mov edx, [esi + 4]      // GI
            mov ebx, 0xFFFF0000     // IsConsumed.
            add bx, gGame          // The game the XFlag comes from 
            
            jmp STORE


        // =========================
        // STORE (COMMON)
        // =========================
        STORE:
            mov ecx, edi // Restore PC value

            // Get gData
            mov esi, gData

            // Test if NULL
            test esi, esi
            jz EXIT

            // Temp Save q values
            push eax
            push edx
            push ebx

            // Get CurrIndex
            mov ebx, [esi + 4]

            // Compute slot = idx * 20
            mov eax, ebx
            shl eax, 4                  // 16
            mov edx, ebx
            shl edx, 2                  // 4
            add eax, edx                // eax = idx * 20
            lea edi, [esi + 16 + eax]

            // Restore q values
            pop ebx
            pop edx
            pop eax

            // Fill the buffer at the correct index
            mov[edi], ecx       // Store PC
            mov[edi + 4], ebp   // Store Mem
            mov[edi + 8], eax   // q0
            mov[edi + 12], edx  // q1
            mov[edi + 16], ebx  // q2

            // Increment CurrIndex
            mov ebx, [esi + 4]
            inc ebx
            and ebx, BUFFER_SIZE - 1
            mov[esi + 4], ebx

        EXIT :

            // Restore saved context
            popad

            // Get back to the orignal PJ execution flow
            jmp gateway
    }
}
*/

void InstallPCHook()
{
    InstallHook(HOOK_PC_OFFSET, HOOK_PC_SIZE, (uintptr_t)&PCHook, &gatewayPC, originalBytesPC);
}


void InstallROMHook()
{
    InstallHook(HOOK_ROM_LOAD_OFFSET, HOOK_ROM_LOAD_SIZE, (uintptr_t)&ROMHook, &gatewayROM, originalBytesROM);
}


void InstallHook(size_t HookOffset, size_t HookSize, uintptr_t HookFunction, void ** gateway, BYTE originalBytes[])
{
    uintptr_t target = moduleBase + HookOffset;

    DWORD oldProtect;
    VirtualProtect((LPVOID)target, HookSize, PAGE_EXECUTE_READWRITE, &oldProtect);

    // sauvegarde instructions originales
    memcpy(originalBytes, (void*)target, HookSize);

    // trampoline
    *gateway = VirtualAlloc(nullptr, 64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    /*for (size_t i = 0; i < HookSize; i++)
    {
        printf("%02X ", originalBytes[i]);
    }*/
    memcpy(*gateway, originalBytes, HookSize);

    uintptr_t gatewayEnd = (uintptr_t)*gateway + HookSize;
    uintptr_t returnAddr = target + HookSize;

    *(BYTE*)(gatewayEnd) = 0xE9;
    *(uintptr_t*)(gatewayEnd + 1) = returnAddr - gatewayEnd - 5;

    // hook
    uintptr_t rel = (uintptr_t)HookFunction - target - 5;

    *(BYTE*)target = 0xE9;
    *(uintptr_t*)(target + 1) = rel;

    for (size_t i = 5; i < HookSize; i++)
        *(BYTE*)(target + i) = 0x90;

    VirtualProtect((LPVOID)target, HookSize, oldProtect, &oldProtect);
}


void InstallHooks()
{
}