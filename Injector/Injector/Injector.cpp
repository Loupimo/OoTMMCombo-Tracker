// Injector.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>
#include <bit>
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <stdarg.h>
typedef struct SharedData
{
    uint32_t pc;
    uint32_t isValid;
    uintptr_t Base;
    bool IsRunning;
} SharedData;

    DWORD PJ64PID = 0;						// The Project64 process ID.
    HANDLE PJ64Handle = 0;					// The handle used to read Project64 memory.
    uintptr_t ModuleBaseAddress = 0;		// The base address of the Project64 module.
    uintptr_t PCAddress = 0;				// The address of the program counter.
    uintptr_t GameRamBaseAddress = 0;		// The base game RAM address (correspond to the 0x80000000 in PJ64 debugger).
    int LoadedGame = 2;						// The current loaded game in RAM (0 = OoT, 1 = MM, 2 = No game / incorrect game loaded)
    bool IsRunning = false;					// Tells if the process memory should be read.
    const char* PJTrackerDLL = "D:\\Emulation\\OoTMMCombo-Tracker\\Debug\\PJ64OoTMMTracker.dll";	// The dll to inject into the Project 64 process
    void* DLLAlloc = nullptr;
    SharedData* DLLData = nullptr;
    HANDLE DLLThread = 0;


void RunMemoryReader()
{
    printf("Reading game memory...");
    //uint32_t PC = 0;
    do
    {
        //ReadProcessMemory(PJ64Handle, (LPCVOID)(PCAddress), &PC, sizeof(PC), 0);
        if (DLLData->pc == 0x80400BE4)
        //if (PC == 0x80400BE4)
            printf("PC = 0x%X", DLLData->pc);

    } while (1);

}


void ResetMemoryReader()
{
    if (PJ64Handle != 0)
    {
        DLLData->IsRunning = false;
        WaitForSingleObject(DLLThread, INFINITE);
        VirtualFreeEx(PJ64Handle, DLLAlloc, 0, MEM_RELEASE);
        CloseHandle(DLLThread);
        CloseHandle(PJ64Handle);
        PJ64Handle = 0;
    }

    PJ64PID = 0;
    ModuleBaseAddress = 0;
}


DWORD GetProcessIdByName(const char* ProcessName)
{
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    char* currProcess = (char*)malloc(sizeof(char) * 250);
    int i = 0;
    size_t sz = 0;
    if (currProcess && Process32First(snapshot, &entry))
    {
        printf("Searching for %s. (Only the first instance will be tracked).", ProcessName);
        do
        {
            wcstombs_s(&sz, currProcess, 250, entry.szExeFile, 250);
            std::string lazyString(currProcess);
            if (!strcmp(lazyString.c_str(), ProcessName))
            //if (!_stricmp(currProcess, processName))
            {
                printf("Process Found : %d", entry.th32ProcessID);
                CloseHandle(snapshot);
                free(currProcess);
                return entry.th32ProcessID;
            }
            i++;
        } while (Process32Next(snapshot, &entry));
    }

    CloseHandle(snapshot);
    free(currProcess);

    return 0;
}


HANDLE OpenDesiredProcess(DWORD PID)
{   // Open the desired process with the right to access the process and reading its memory

    return OpenProcess(PROCESS_ALL_ACCESS, TRUE, PJ64PID);
    //return OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, false, PID);
}


bool IsProcessAlive(HANDLE Process)
{
    DWORD exitCode;

    if (!GetExitCodeProcess(Process, &exitCode))
    {
        return false;
    }

    return exitCode == STILL_ACTIVE;
}





uintptr_t GetModuleBase(DWORD PID, const char* ModuleName)
{
    MODULEENTRY32 mod;
    mod.dwSize = sizeof(mod);
    char* currProcess = (char*)malloc(sizeof(char) * 256);
    size_t sz = 0;

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, PID);

    if (currProcess && Module32First(snapshot, &mod))
    {
        do
        {
            wcstombs_s(&sz, currProcess, 256, mod.szModule, 256);
            std::string lazyString(currProcess);
            if (lazyString.c_str() == ModuleName)
            //if (!_stricmp(mod.szModule, moduleName))
            {
                CloseHandle(snapshot);
                return (uintptr_t)mod.modBaseAddr;
            }

        } while (Module32Next(snapshot, &mod));
    }

    CloseHandle(snapshot);
    return 0;
}


bool OpenSharedMemory()
{
    HANDLE hMap = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, "PJ64_SHARED_MEM");

    if (!hMap)
    {
        std::cout << "Shared memory not found\n";
        return false;
    }

    DLLData = (SharedData*)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedData));

    if (DLLData != nullptr)
    {
        DLLData->Base = ModuleBaseAddress;
        return true;
    }

    return false;
}


bool InjectTrackerDLL()
{
    if (PJ64Handle == 0)
    {
        return false;
    }

    DLLAlloc = VirtualAllocEx(PJ64Handle, nullptr, strlen(PJTrackerDLL), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    if (!DLLAlloc)
    {
        return false;
    }

    bool ret = WriteProcessMemory(PJ64Handle, DLLAlloc, PJTrackerDLL, strlen(PJTrackerDLL), nullptr);

    HMODULE lpStart = GetModuleHandle(L"kernel32.dll");
    LPVOID st = GetProcAddress(lpStart, "LoadLibraryA");

    DLLThread = CreateRemoteThread(PJ64Handle, nullptr, 0, (LPTHREAD_START_ROUTINE)st, DLLAlloc, 0, nullptr);

    if (!DLLThread)
        return false;

    WaitForSingleObject(DLLThread, INFINITE);
    DWORD exitCode = 0;
    GetExitCodeThread(DLLThread, &exitCode);

    printf("LoadLibrary result = 0x%X\n", exitCode);

    if (GetFileAttributesA(PJTrackerDLL) == INVALID_FILE_ATTRIBUTES)
    {
        printf("DLL NOT FOUND\n");
    }

    //VirtualFreeEx(hProcess, alloc, 0, MEM_RELEASE);
    //CloseHandle(hThread);
    //CloseHandle(hProcess);

    return true;
}
void StartMemoryReader()
{
    IsRunning = true;
    const char* processName = "Project64-EM.exe";

    do
    {
        PJ64PID = GetProcessIdByName(processName);

        if (PJ64PID == 0)
        {   // PJ64-EM not found. Wait 1 second before retrying

            printf("No %s process found. Retrying in 1 second...\n", processName);
            Sleep(1000);
        }

    } while (PJ64PID == 0 && IsRunning);

    if (PJ64PID != 0)
    {
        PJ64Handle = OpenDesiredProcess(PJ64PID);
        if (PJ64Handle != 0)
        {   // We have permission to watch the process memory


            if (InjectTrackerDLL())
            {
                ModuleBaseAddress = GetModuleBase(PJ64PID, processName);

                do
                {
                    if (ModuleBaseAddress == 0)
                    {
                        printf("Module base address not found. Retrying in 1 second...");
                        Sleep(1000);
                        ModuleBaseAddress = GetModuleBase(PJ64PID, processName);
                    }
                } while (ModuleBaseAddress == 0);

                if (ModuleBaseAddress != 0)
                {
                    OpenSharedMemory();
                    RunMemoryReader();
                    //PCAddress = FindPCAddress(PJ64Handle);
                }
            }
            else
            {
                printf("Cannot inject tracker dll into %s.\nPlease check your process, ensure that %s is in the same folder as the tracker and restart the tracker.", processName, PJTrackerDLL);
            }
        }
        else
        {   // An error occured while trying to get a valid handler to read the process memory

            DWORD errorMessageID = GetLastError();
            printf("Cannot access %s process. Error : (%d).\nPlease check your process and restart the tracker.", processName, errorMessageID);
        }
    }
}


int main()
{
    std::cout << "Hello World!\n";
    StartMemoryReader();
}

// Exécuter le programme : Ctrl+F5 ou menu Déboguer > Exécuter sans débogage
// Déboguer le programme : F5 ou menu Déboguer > Démarrer le débogage

// Astuces pour bien démarrer : 
//   1. Utilisez la fenêtre Explorateur de solutions pour ajouter des fichiers et les gérer.
//   2. Utilisez la fenêtre Team Explorer pour vous connecter au contrôle de code source.
//   3. Utilisez la fenêtre Sortie pour voir la sortie de la génération et d'autres messages.
//   4. Utilisez la fenêtre Liste d'erreurs pour voir les erreurs.
//   5. Accédez à Projet > Ajouter un nouvel élément pour créer des fichiers de code, ou à Projet > Ajouter un élément existant pour ajouter des fichiers de code existants au projet.
//   6. Pour rouvrir ce projet plus tard, accédez à Fichier > Ouvrir > Projet et sélectionnez le fichier .sln.
