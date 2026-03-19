#include "pch.h"
#include "Hooking.h"

// PC addresses to target
constexpr int MAX_TARGETS = 128;
constexpr uint32_t targetPCs[MAX_TARGETS] = {
    0x8040C35C
};
constexpr size_t targetCount = sizeof(targetPCs) / sizeof(targetPCs[0]);

// Bitmask to discard non desired PC really fast. Possible range: 0x80000000 - 0x80FFFFFF
constexpr uint32_t BITMASK_SIZE = 1 << 24; // 16M bits
uint8_t bitmask[BITMASK_SIZE / 8];         // 2 MB

// Hooking installation
void* gateway = NULL;
BYTE originalBytes[HOOK_SIZE];
uintptr_t returnAddress = 0;

// Game data / shared memeory data
SharedData* gData = nullptr;
uintptr_t moduleBase = 0;
uintptr_t gameRAMBase = 0;


void InitBitmask()
{
    memset(bitmask, 0, sizeof(bitmask));

    for (size_t i = 0; i < targetCount; i++)
    {
        uint32_t pc = targetPCs[i];

        uint32_t index = pc & 0x00FFFFFF;

        bitmask[index >> 3] |= (1 << (index & 7));
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
        mov eax, moduleBase
        add eax, REG_PTR_OFFSET
        mov eax, [eax]

        // =========================
        // Get PC / SP
        // =========================
        mov ecx, [eax + PC_OFFSET]   // PC
        mov edx, [eax + A1_OFFSET]   // A1
        mov edi, ecx                 // Save PC value in edi
        mov ebp, edx                 // Save A1 value in ebp

        // =========================
        // Bitmask Check with targeted PC addresses
        // =========================
        mov eax, edi
        and eax, 0x00FFFFFF         // Direct index

        mov esi, offset bitmask

        mov edx, eax
        shr edx, 3                  // Byte index

        mov bl, [esi + edx]

        and eax, 7;                 // Bit index
        mov cl, al
            shr bl, cl
            and bl, 1

            jz EXIT

            // =========================
            // Check A1 Range : Between 0x80000000 - 0x80FFFFFF
            // =========================
            mov eax, ebp
            and eax, 0xFF000000
            cmp eax, 0x80000000
            jne EXIT

            // =========================
            // STORE
            // =========================

            mov ecx, edi // Restore PC value

            // Get gData
            mov esi, gData

            // Test if NULL
            test esi, esi
            jz EXIT

            // Get CurrIndex
            mov ebx, [esi + 4]

            // Compute slot = idx * 20
            mov eax, ebx
            shl eax, 4                  // 16
            mov edx, ebx
            shl edx, 2                  // 4
            add eax, edx                // eax = idx * 20
            lea edi, [esi + 8 + eax]

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

            // Fill the buffer at the correct index
            mov[edi], ecx       // Store PC
            mov[edi + 4], ebp   // Store A1
            mov[edi + 8], eax   // q0
            mov[edi + 12], edx  // q1
            mov[edi + 16], ebx  // q2

            // Increment CurrIndex
            mov esi, gData
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