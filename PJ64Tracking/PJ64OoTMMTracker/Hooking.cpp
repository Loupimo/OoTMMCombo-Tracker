#include "pch.h"
#include "Hooking.h"

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

uint8_t typemask[PC_MASK_SIZE];

// Hooking installation
void* gateway = NULL;
BYTE originalBytes[HOOK_SIZE];
uintptr_t returnAddress = 0;

// Game data / shared memeory data
SharedData* gData = nullptr;
uint16_t gGame = OOT_GAME;
uintptr_t moduleBase = 0;
uintptr_t regBase = 0;
uintptr_t gameRAMBase = 0;
uintptr_t currButterflyPC = 0;
uintptr_t gSP = 0;
uintptr_t gV0 = 0;
uintptr_t gV1 = 0;
uintptr_t gS0 = 0;


void SetPCType(uint32_t pc, uint8_t type)
{
    /*uint32_t index = pc & 0x00FFFFFF;

    uint32_t byteIndex = index >> 2;      // /4
    uint32_t shift = (index & 3) * 2;     // *2 bits

    typemask[byteIndex] &= ~(3 << shift); // clear
    typemask[byteIndex] |= (type << shift);*/
    typemask[pc & 0x00FFFFFF] = type;
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


__declspec(naked) void Hook()
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
            mov gGame, MM_GAME
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
            
            mov gGame, OOT_GAME

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

            mov gGame, MM_GAME

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
            lea edi, [esi + 8 + eax]

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


void InstallHook()
{
    uintptr_t target = moduleBase + HOOK_OFFSET;

    DWORD oldProtect;
    VirtualProtect((LPVOID)target, HOOK_SIZE, PAGE_EXECUTE_READWRITE, &oldProtect);

    // sauvegarde instructions originales
    memcpy(originalBytes, (void*)target, HOOK_SIZE);

    // trampoline
    gateway = VirtualAlloc(nullptr, 64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    memcpy(gateway, originalBytes, HOOK_SIZE);

    uintptr_t gatewayEnd = (uintptr_t)gateway + HOOK_SIZE;
    uintptr_t returnAddr = target + HOOK_SIZE;

    *(BYTE*)(gatewayEnd) = 0xE9;
    *(uintptr_t*)(gatewayEnd + 1) = returnAddr - gatewayEnd - 5;

    // hook
    uintptr_t rel = (uintptr_t)&Hook - target - 5;

    *(BYTE*)target = 0xE9;
    *(uintptr_t*)(target + 1) = rel;

    for (int i = 5; i < HOOK_SIZE; i++)
        *(BYTE*)(target + i) = 0x90;

    VirtualProtect((LPVOID)target, HOOK_SIZE, oldProtect, &oldProtect);
}