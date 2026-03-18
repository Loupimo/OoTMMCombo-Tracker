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
        //gData->IsRunning = true;
        //gData->isValid = 0;

        while (gData->Base == 0)
        {
            Sleep(1);
        }

        moduleBase = gData->Base;
        gData->MaxSize = BUFFER_SIZE;
        gData->CurrIndex = 0;
        for (size_t i = 0; i < BUFFER_SIZE; i++)
        {
            gData->Buffer[i].pc = 0;
            gData->Buffer[i].sp = 0;
        }
        InitBitmask();
        InstallHook();

        LONG readIndex = 0;
        while (true)
        {
            LONG currIndex = gData->CurrIndex;

            while (readIndex != currIndex)
            {
                auto& evt = gData->Buffer[readIndex];
                printf("Hit ! PC = 0x%08X, SP = 0x%08X\n", evt.pc, evt.sp);
                    //sendToTracker(evt.pc, evt.sp);

                readIndex = (readIndex + 1) % BUFFER_SIZE; // wrap-around
            }

            Sleep(1); // ou Yield pour laisser CPU aux autres threads
        }

        /*
        uintptr_t cpuPtr = *(uintptr_t*)(gData->Base + 0x1B00C4);
        cpuPtr = (cpuPtr + 0x220);

        printf("cpuPtr = 0x%08X\n", cpuPtr); // \r = overwrite ligne


        while (gData->IsRunning)
        {
            gData->pc = *(uint32_t*)cpuPtr;

            printf("PC = 0x%08X\r", gData->pc); // \r = overwrite ligne
            if (gData->pc == 0x80400BE4)
            {
                printf("Hit\n"); // \r = overwrite ligne
                gData->isValid = 1;
            }
            Sleep(0);
        }*/
    }
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
        break;
    }
    return TRUE;
}