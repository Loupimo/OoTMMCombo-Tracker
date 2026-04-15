// dllmain.cpp : Définit le point d'entrée de l'application DLL.
#include "pch.h"
#include "Hooking.h"

DWORD WINAPI MainThread(LPVOID)
{
#ifdef _DEBUG

    AllocConsole();
    
    FILE* fp = nullptr;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    if (fp == nullptr)
    {
        OutputDebugStringA("Failed to redirect stdout\n");
        return 1;
    }

    printf("DLL injected!\n");

#endif // _DEBUG

    HANDLE hMap = CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(SharedData), "PJ64_SHARED_MEM");

    if (hMap == 0)
    {
        return -1;
    }

    gData = (SharedData*)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedData));

    if (gData)
    {
        //gData->Base = 0;
        //gData->GameRAMBase = 0;
        gData->MaxSize = BUFFER_SIZE;
        gData->CurrIndex = 0;
        for (size_t i = 0; i < BUFFER_SIZE; i++)
        {
            gData->Buffer[i].PC = 0;
            gData->Buffer[i].Mem = 0;
            gData->Buffer[i].Query[0] = 0;
            gData->Buffer[i].Query[1] = 0;
            gData->Buffer[i].Query[2] = 0;
        }

        HMODULE hModule = GetModuleHandle(NULL);
        moduleBase = (uintptr_t)hModule;
        regBase = moduleBase + REG_PTR_OFFSET;

        InstallROMHook();
        TryResolveROMBase();
        gameRAMBase = FindGameRAM();

        LOG("Init Hook");

        InstallPCHook();
    }

    return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        {
            DisableThreadLibraryCalls(hModule);
            CreateThread(nullptr, 0, MainThread, nullptr, 0, nullptr);
            break;
        }
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
        default:
        break;

    }
    return TRUE;
}