// dllmain.cpp : Définit le point d'entrée de l'application DLL.
#include "pch.h"
#include "Hooking.h"


DWORD WINAPI MainThread(LPVOID)
{
    AllocConsole();
    
    FILE* fp = nullptr;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    if (fp == nullptr)
    {
        OutputDebugStringA("Failed to redirect stdout\n");
        return 1;
    }

    printf("DLL injected!\n");

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
            gData->Buffer[i].A1 = 0;
            gData->Buffer[i].Query[0] = 0;
            gData->Buffer[i].Query[1] = 0;
            gData->Buffer[i].Query[2] = 0;
        }

        HMODULE hModule = GetModuleHandle(NULL);
        moduleBase = (uintptr_t)hModule;
        regBase = moduleBase + REG_PTR_OFFSET;
        gameRAMBase = FindGameRAM();

        /*while (gData->Base == 0 || gData->GameRAMBase == 0)
        {
            Sleep(1);
        }*/

        printf("Init Hook\n");

        //moduleBase = gData->Base;
        //gameRAMBase = gData->GameRAMBase;
        InitTypeMask();
        InstallHook();
        /*
        printf("Sizeof Event = %zu\n", sizeof(Event));
        LONG readIndex = 0;
        while (true)
        {
            LONG currIndex = gData->CurrIndex;

            while (readIndex != currIndex)
            {
                Event evt = gData->Buffer[readIndex];
                printf("Hit ! PC = 0x%08X, A1 = 0x%08X, GI = 0x%04X, GIRenew = 0x%04X, OVFlags = 0x%04X, OVType = 0x%02X, SceneID = 0x%02X, RoomID = 0x%02X, ID = 0x%02X, From = 0x%02X\n", evt.PC, evt.A1, evt.Query.GI, evt.Query.GIRenew, evt.Query.OVFlags, evt.Query.OVType, evt.Query.SceneId, evt.Query.RoomId, evt.Query.ID, evt.Query.From);
                    //sendToTracker(evt.pc, evt.sp);

                readIndex = (readIndex + 1) % BUFFER_SIZE; // wrap-around
            }

            Sleep(10); // ou Yield pour laisser CPU aux autres threads
        }*/
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