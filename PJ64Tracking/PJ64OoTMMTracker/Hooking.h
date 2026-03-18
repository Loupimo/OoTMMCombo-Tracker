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
std::unordered_set<uint32_t> targetPCSet{ 0x80400BE4 };

bool IsTarget(uint32_t pc)
{
    return targetPCSet.count(pc) > 0;
}

 void* gateway;


// ==============================
// Hook
// ==============================
uintptr_t moduleBase = 0;
BYTE originalBytes[HOOK_SIZE];
uintptr_t returnAddress = 0;

__declspec(naked) void Hook()
{
    __asm
    {
        pushad

        // 🔥 FIX ICI
        mov eax, offset gData
        mov eax, [eax]

        test eax, eax
        jz skip

        // CPU
        mov ebx, moduleBase
        add ebx, CPU_PTR_OFFSET
        mov ebx, [ebx]

        test ebx, ebx
        jz skip

        mov ecx, [ebx + PC_OFFSET]
        mov edx, [ebx + SP_OFFSET]

        // CurrIndex
        mov esi, eax; esi = gData *
        add esi, 4; &CurrIndex
        
        mov ebx, [esi]; ebx = CurrIndex
        inc ebx
        and ebx, BUFFER_SIZE - 1
        mov[esi], ebx; store back

        mov edi, eax; edi = gData *
        add edi, 12; &Buffer
        lea edi, [edi + ebx * 8]; < --utiliser ebx, pas esi

        mov[edi], ecx
        mov[edi + 4], edx

        skip :
        popad
            jmp gateway
    }
}

// ==============================
// Installer le hook
// ==============================
void InstallHook()
{
    uintptr_t target = moduleBase + HOOK_OFFSET;

    DWORD oldProtect;
    VirtualProtect((LPVOID)target, HOOK_SIZE, PAGE_EXECUTE_READWRITE, &oldProtect);

    // 1️⃣ sauvegarde
    memcpy(originalBytes, (void*)target, HOOK_SIZE);

    // 2️⃣ trampoline
    gateway = VirtualAlloc(nullptr, 32, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    // copier instructions originales
    memcpy(gateway, originalBytes, HOOK_SIZE);

    // jump retour
    uintptr_t gatewayEnd = (uintptr_t)gateway + HOOK_SIZE;
    uintptr_t returnAddr = target + HOOK_SIZE;

    *(BYTE*)(gatewayEnd) = 0xE9;
    *(uintptr_t*)(gatewayEnd + 1) = returnAddr - gatewayEnd - 5;

    // 3️⃣ hook
    uintptr_t rel = (uintptr_t)&Hook - target - 5;

    *(BYTE*)target = 0xE9;
    *(uintptr_t*)(target + 1) = rel;

    // NOP padding
    for (int i = 5; i < HOOK_SIZE; i++)
        *(BYTE*)(target + i) = 0x90;

    VirtualProtect((LPVOID)target, HOOK_SIZE, oldProtect, &oldProtect);
}