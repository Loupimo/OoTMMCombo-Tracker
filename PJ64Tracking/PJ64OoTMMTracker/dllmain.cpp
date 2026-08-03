// dllmain.cpp : Définit le point d'entrée de l'application DLL.
#include "pch.h"
#include "Hooking.h"
#include <string.h>

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

        // Resolve the Project64-EM fork version so the rest of the DLL can adapt its offsets.
        DetectPJVersion();

        switch (gPJVersion)
        {
            case PJVersion::EM_1_1_0:
            {
                regBase += OFFSET_PTR_V_1_1_0;
                break;
            }
        }

        InstallROMHook();
        TryResolveROMBase();
        gameRAMBase = FindGameRAM();

        LOG("Init Hook");

        InstallPCHook();
    }

    return 0;
}

/*
*   Message hook procedure exported for SetWindowsHookEx based injection. Its only purpose
*   is to provide a valid address inside this module so Windows maps the DLL into the target
*   process. It performs no work and simply forwards to the next hook in the chain.
*
*   @param  Code    The hook code passed by Windows.
*   @param  WParam  Message specific information.
*   @param  LParam  Message specific information.
*   @return         The result of the next hook in the chain.
*/
extern "C" LRESULT CALLBACK TrackerHookProc(int Code, WPARAM WParam, LPARAM LParam)
{
    return CallNextHookEx(NULL, Code, WParam, LParam);
}


/*
*   Tell whether the current host process is Project64. The DLL is loaded both into the
*   injector (to obtain its module handle) and into Project64. The hooks must only be
*   installed in the latter, never in the injector.
*
*   @return true if the host executable name starts with "Project64", false otherwise.
*/
static bool IsHostProject64()
{
    char path[MAX_PATH] = { 0 };

    if (GetModuleFileNameA(NULL, path, MAX_PATH) == 0)
    {   // Cannot resolve the host executable, stay inert to be safe

        return false;
    }

    const char* name = strrchr(path, '\\');
    name = name ? name + 1 : path;

    // Same prefix the tracker uses to locate the emulator process.
    return _strnicmp(name, "Project64", 9) == 0;
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

            if (IsHostProject64())
            {   // Only install the hooks when running inside the emulator

                // Pin the module so it stays loaded once the injector removes its hook.
                HMODULE pinned = nullptr;
                GetModuleHandleExW(
                    GET_MODULE_HANDLE_EX_FLAG_PIN | GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                    (LPCWSTR)&TrackerHookProc,
                    &pinned);

                CreateThread(nullptr, 0, MainThread, nullptr, 0, nullptr);
            }

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
