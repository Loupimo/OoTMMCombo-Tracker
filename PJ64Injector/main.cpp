#include <windows.h>
#include <tlhelp32.h>
#include <iostream>

bool Inject(DWORD pid, const char* dllPath)
{
    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, TRUE, pid);
    if (!hProc)
    {
        return false;
    }

    void* alloc = VirtualAllocEx(hProc, nullptr, strlen(dllPath), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    
    if (alloc)
    {
        WriteProcessMemory(hProc, alloc, dllPath, strlen(dllPath), nullptr);

        HANDLE hThread = CreateRemoteThread(hProc, nullptr, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, alloc, 0, nullptr);

        if (!hThread)
        {
            return false;
        }

        WaitForSingleObject(hThread, INFINITE);

        VirtualFreeEx(hProc, alloc, 0, MEM_RELEASE);
        CloseHandle(hThread);
        CloseHandle(hProc);

        return true;
    }

    return false;
}


int main(int argc, char** argv)
{
    /*if (argc < 3)
    {
        std::cout << "Usage: PJ64Injector.exe <process ID> <dll path>\n";
        return 1;
    }*/

    DWORD pid = atoi(argv[1]);
    //const char* dll = "D:\\Emulation\\OoTMMCombo-Tracker\\Debug\\PJ64OoTMMTracker.dll";

    if (!pid)
    {
        std::cout << "Process not found\n";
        return 1;
    }

    //if (Inject(pid, dll))
    if (Inject(pid, argv[2]))
    {
        std::cout << "Injected\n";
        return 0;
    }
    else
    {
        std::cout << "Injection failed\n";
    }
    return 0;
}