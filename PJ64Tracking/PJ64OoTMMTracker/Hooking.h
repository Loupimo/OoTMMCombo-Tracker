#pragma once

#define CPU_PTR_OFFSET 0x1B00C4
#define PC_OFFSET      0x220				// PC offset in the CPU structure
#define SP_OFFSET      PC_OFFSET + 0xF0     // SP offset in the CPU structure (relative to PC offset)
#define HOOK_OFFSET    0xE64C9				// Instruction offset to hook
#define HOOK_SIZE 7
#define BUFFER_SIZE 1024

#include <unordered_set>

// ==============================
// Event buffer (lock-free)
// ==============================
typedef struct Event
{
    uint32_t pc;
    uint32_t sp;
} Event;


struct SharedData
{
    LONG MaxSize;
    volatile LONG CurrIndex;
    uintptr_t Base;
    Event Buffer[BUFFER_SIZE];
};
SharedData* gData = nullptr;

// ==============================
// Targets
// ==============================
constexpr int MAX_TARGETS = 128;

uint32_t targetPCs[MAX_TARGETS] = {
    0x80400BE4,
    0x80400BE8,
    0x80400BEC,
};
constexpr size_t targetCount = sizeof(targetPCs) / sizeof(targetPCs[0]);

// ==============================
// BITMASK
// ==============================

// plage: 0x80000000 - 0x80FFFFFF
constexpr uint32_t BITMASK_SIZE = 1 << 24; // 16M bits
uint8_t bitmask[BITMASK_SIZE / 8];         // 2 MB

// ==============================
// INIT BITMASK
// ==============================

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

// ==============================
// Hook
// ==============================
void* gateway;
uintptr_t moduleBase = 0;

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
        add eax, CPU_PTR_OFFSET
        mov eax, [eax]

        // =========================
        // Get PC / SP
        // =========================
        mov ecx, [eax + PC_OFFSET]   // PC
        mov edx, [eax + SP_OFFSET]   // SP
        mov edi, ecx                 // Save PC value in edi
        mov ebp, edx                 // Save SP value in ebp

        // =========================
        // BITMASK CHECK
        // =========================
        mov eax, edi
        and eax, 0x00FFFFFF; index direct

        mov esi, offset bitmask

        mov edx, eax
        shr edx, 3; byte index

        mov bl, [esi + edx]

        and eax, 7; bit index
        mov cl, al
        shr bl, cl
        and bl, 1

        jz EXIT

        // =========================
        // STORE
        // =========================

        mov ecx, edi // Restore PC value
        mov edx, ebp // Restore SP value

        // Get gData
        mov eax, gData
        
        // Test if NULL
        test eax, eax
        jz EXIT
        
        // Get CurrIndex
        mov ebx, [eax + 4]
        
        // Fill the buffer at the correct index with PC and SP values
        lea edi, [eax + 12 + ebx * 8]
        mov[edi], ecx
        mov[edi + 4], edx
        
        // Increment CurrIndex
        inc ebx
        and ebx, BUFFER_SIZE - 1
        mov[eax + 4], ebx

        EXIT :
            // Restore saved context
            popad

            // Get back to the orignal PJ execution flow
            jmp gateway
    }
}

// ==============================
// Installer le hook
// ==============================
BYTE originalBytes[HOOK_SIZE];
uintptr_t returnAddress = 0;

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