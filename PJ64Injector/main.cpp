#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <cstdlib>
#include <string.h>

bool FileExists(const char* filename)
{
    DWORD attrib = GetFileAttributesA(filename);
    return (attrib != INVALID_FILE_ATTRIBUTES && !(attrib & FILE_ATTRIBUTE_DIRECTORY));
}


/*
*   Data used while searching for a GUI thread of the target process.
*/
typedef struct FindThreadData
{
    DWORD Pid;          // The process ID we are looking for.
    DWORD ThreadId;     // The resulting GUI thread ID (0 if none was found).
} FindThreadData;


/*
*   EnumWindows callback used to find a thread of the target process that owns a window.
*
*   @param  Window  The window currently being enumerated.
*   @param  Param   Pointer to a FindThreadData structure.
*   @return         FALSE to stop enumeration once a thread is found, TRUE to continue.
*/
static BOOL CALLBACK FindGuiThreadProc(HWND Window, LPARAM Param)
{
    FindThreadData* data = (FindThreadData*)Param;

    DWORD windowPid = 0;
    DWORD windowThreadId = GetWindowThreadProcessId(Window, &windowPid);

    if (windowPid == data->Pid)
    {   // This window belongs to the target process, keep its owning thread

        data->ThreadId = windowThreadId;
        return FALSE;
    }

    return TRUE;
}


/*
*   Find a GUI thread (a thread owning a top-level window) of the given process.
*
*   @param  PID     The target process ID.
*   @return         A thread ID owning a window, or 0 if none was found.
*/
DWORD FindGuiThreadId(DWORD PID)
{
    FindThreadData data = { PID, 0 };
    EnumWindows(FindGuiThreadProc, (LPARAM)&data);
    return data.ThreadId;
}


/*
*   Check whether a given module is loaded into the target process.
*
*   @param  PID         The target process ID.
*   @param  ModuleName  The module file name to look for.
*   @return             true if the module is present in the process, false otherwise.
*/
bool IsModuleLoaded(DWORD PID, const wchar_t* ModuleName)
{
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, PID);

    if (snapshot == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    MODULEENTRY32 module = { 0 };
    module.dwSize = sizeof(MODULEENTRY32);

    bool found = false;

    if (Module32First(snapshot, &module))
    {
        do
        {
            if (_wcsicmp(module.szModule, ModuleName) == 0)
            {
                found = true;
                break;
            }
        } while (Module32Next(snapshot, &module));
    }

    CloseHandle(snapshot);
    return found;
}


/*
*   Inject the desired DLL into the given process using a standard Windows message hook.
*   Windows itself maps the DLL into the target, so no remote thread or executable memory
*   allocation is involved.
*
*   @param  PID         The process ID to inject the DLL to.
*   @param  DLLPath     The absolute path to the DLL to inject.
*   @return             true if the DLL was successfully loaded into the target.
*/
bool Inject(DWORD PID, const char* DLLPath)
{
    if (!FileExists(DLLPath))
    {   // The dll does not exist

        std::cout << "The dll cannot be found : " << DLLPath << std::endl;
        return false;
    }

    // Load the DLL locally to obtain its module handle and the exported hook procedure.
    // The DLL detects it is not running inside Project64 and stays inert in this process.
    HMODULE hDll = LoadLibraryA(DLLPath);

    if (!hDll)
    {
        std::cout << "LoadLibrary failed : " << GetLastError() << std::endl;
        return false;
    }

    HOOKPROC hookProc = (HOOKPROC)GetProcAddress(hDll, "TrackerHookProc");

    if (!hookProc)
    {
        std::cout << "Cannot find the exported hook procedure." << std::endl;
        FreeLibrary(hDll);
        return false;
    }

    // Find a GUI thread of Project64 to attach the hook to.
    DWORD threadId = FindGuiThreadId(PID);

    if (!threadId)
    {
        std::cout << "No GUI thread found for the target process." << std::endl;
        FreeLibrary(hDll);
        return false;
    }

    // Install a message hook on the target thread. Windows maps our DLL into Project64
    // the next time that thread retrieves a message from its queue.
    HHOOK hHook = SetWindowsHookEx(WH_GETMESSAGE, hookProc, hDll, threadId);

    if (!hHook)
    {
        std::cout << "SetWindowsHookEx failed : " << GetLastError() << std::endl;
        FreeLibrary(hDll);
        return false;
    }

    // Nudge the target thread so it pumps a message and loads the DLL now, then wait until
    // the DLL appears in Project64 (it pins itself in DllMain so it stays resident).
    bool loaded = false;

    for (int i = 0; i < 100; i++)
    {
        PostThreadMessage(threadId, WM_NULL, 0, 0);

        if (IsModuleLoaded(PID, L"PJ64OoTMMTracker.dll"))
        {
            loaded = true;
            break;
        }

        Sleep(50);
    }

    // The hook has done its job, remove it. The DLL remains loaded in Project64.
    UnhookWindowsHookEx(hHook);
    FreeLibrary(hDll);

    if (!loaded)
    {
        std::cout << "The DLL did not load into the target process." << std::endl;
    }

    return loaded;
}


int main(int argc, char** argv)
{
    if (argc < 3)
    {   // The program needs two arguments

        std::cout << "Usage: PJ64Injector.exe <process ID> <dll path>\n";
        return 1;
    }

    DWORD pid = atoi(argv[1]);

    if (!pid)
    {   // Cannot find the process to inject the DLL to

        std::cout << "Process not found\n";
        return 1;
    }

    if (Inject(pid, argv[2]))
    {   // Successfully injected our DLL into the process

        std::cout << "Injected\n";
        return 0;
    }

    // Failed to inject the DLL into the process
    std::cout << "Injection failed\n";
    return 1;
}
