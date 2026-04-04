#include <windows.h>
#include <tlhelp32.h>
#include <iostream>

/*
*   Inject the desired DLL into the given process.
* 
*   @param  PID         The process ID to inject the DLL to.
*   @param  DLLPath     The absolute path to the DLL to inject.
*/
bool Inject(DWORD PID, const char* DLLPath)
{
    // Open the desired process with the required rights
    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, TRUE, PID);
    
    if (!hProc)
    {   // Failed to open the target process
        return false;
    }

    // Allocate the memory where to store our DLL code.
    void* alloc = VirtualAllocEx(hProc, nullptr, strlen(DLLPath), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    
    if (alloc)
    {   // We have succeed in allocate space for our DLL

        // Write the actual code into our allocated memory
        WriteProcessMemory(hProc, alloc, DLLPath, strlen(DLLPath), nullptr);

        // Create a remote thread to start executing our DLL main function
        HANDLE hThread = CreateRemoteThread(hProc, nullptr, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, alloc, 0, nullptr);

        if (!hThread)
        {   // We failed to launch our DLL main function

            return false;
        }

        // Wait for everything to complete
        WaitForSingleObject(hThread, INFINITE);

        // Free used memory by the injector
        VirtualFreeEx(hProc, alloc, 0, MEM_RELEASE);
        CloseHandle(hThread);
        CloseHandle(hProc);

        return true;
    }

    return false;
}


int main(int argc, char** argv)
{
    if (argc < 3)
    {   // The program need to arguments

        std::cout << "Usage: PJ64Injector.exe <process ID> <dll path>\n";
        return 1;
    }

    DWORD pid = atoi(argv[1]);

    if (!pid)
    {   // Cannot find the process to inject or DLL to

        std::cout << "Process not found\n";
        return 1;
    }

    if (Inject(pid, argv[2]))
    {   // Successfully injected our DLL to the process

        std::cout << "Injected\n";
        return 0;
    }
    else
    {   // Failed to inject or DLL to the process

        std::cout << "Injection failed\n";
    }
    return 0;
}