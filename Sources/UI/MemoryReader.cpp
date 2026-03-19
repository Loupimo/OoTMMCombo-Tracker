#include "UI/MemoryReader.h"
#include "Multi/Game.h"
#include <bit>

std::string GetErrorAsString(DWORD ErrorMessageID)
{
    // Get the error message ID, if any.
    if (ErrorMessageID == 0) {
        return std::string(); // No error message has been recorded
    }

    LPSTR messageBuffer = nullptr;

    // Ask Win32 to give us the string version of that message ID.
    // The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, ErrorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    // Copy the error message into a std::string.
    std::string message(messageBuffer, size);

    // Free the Win32's string's buffer.
    LocalFree(messageBuffer);

    return message;
}

MemoryReader::MemoryReader()
{
    this->ResetMemoryReader();
}


MemoryReader::~MemoryReader()
{
    this->ResetMemoryReader();
}


void MemoryReader::ResetMemoryReader()
{
    this->IsRunning = false;

    if (this->PJ64Handle != 0)
    {
        //this->DLLData->IsRunning = false;
        WaitForSingleObject(this->DLLThread, INFINITE);
        VirtualFreeEx(this->PJ64Handle, this->DLLAlloc, 0, MEM_RELEASE);
        CloseHandle(this->DLLThread);
        CloseHandle(this->PJ64Handle);
        this->PJ64Handle = 0;
    }

    this->PJ64PID = 0;
    this->ModuleBaseAddress = 0;
    this->GameRamBaseAddress = 0;
    this->EntHelper.ResetEntranceHelper();
}


DWORD MemoryReader::GetProcessIdByName(const char* ProcessName)
{
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    char* currProcess = (char*)malloc(sizeof(char) * 250);
    int i = 0;
    size_t sz = 0;
    if (currProcess && Process32First(snapshot, &entry))
    {
        MultiLogger::LogMessage("Searching for %s. (Only the first instance will be tracked).", ProcessName);
        do
        {
            wcstombs_s(&sz, currProcess, 250, entry.szExeFile, 250);
            QString lazyString(currProcess);
            if (lazyString.startsWith(ProcessName) && lazyString.endsWith(".exe"))
            //if (!_stricmp(currProcess, processName))
            {
                MultiLogger::LogMessage("Process Found : %d", entry.th32ProcessID);
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


HANDLE MemoryReader::OpenDesiredProcess(DWORD PID)
{   // Open the desired process with the right to access the process and reading its memory

    return OpenProcess(PROCESS_ALL_ACCESS, TRUE, this->PJ64PID);
    //return OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, false, PID);
}


bool MemoryReader::IsProcessAlive(HANDLE Process)
{
    DWORD exitCode;

    if (!GetExitCodeProcess(Process, &exitCode))
    {
        return false;
    }

    return exitCode == STILL_ACTIVE;
}


void MemoryReader::StartMemoryReader()
{
    this->IsRunning = true;
    const char* processName = "Project64";

    do
    {
        this->PJ64PID = this->GetProcessIdByName(processName);

        if (this->PJ64PID == 0)
        {   // PJ64-EM not found. Wait 1 second before retrying

            MultiLogger::LogMessage("No %s process found. Retrying in 1 second...\n", processName);
            Sleep(1000);
        }

    } while (this->PJ64PID == 0 && this->IsRunning);

    if (this->PJ64PID != 0)
    {
        this->PJ64Handle = this->OpenDesiredProcess(this->PJ64PID);
        if (this->PJ64Handle != 0)
        {   // We have permission to watch the process memory


            if (this->InjectTrackerDLL())
            {
                /*this->ModuleBaseAddress = this->GetModuleBase(this->PJ64PID, processName);
                this->GameRamBaseAddress = this->FindN64RAM(this->PJ64Handle);

                do
                {
                    if (this->ModuleBaseAddress == 0)
                    {
                        MultiLogger::LogMessage("Module base address not found. Retrying in 1 second...");
                        Sleep(1000);
                        this->ModuleBaseAddress = this->GetModuleBase(this->PJ64PID, processName);
                    }
                    if (this->GameRamBaseAddress == 0)
                    {   // The game has not be launched yet

                        MultiLogger::LogMessage("Game RAM not found, please launch your game first. Retrying in 1 second...");
                        Sleep(1000);
                        this->GameRamBaseAddress = this->FindN64RAM(this->PJ64Handle);
                    }

                } while (this->ModuleBaseAddress == 0 && this->GameRamBaseAddress == 0 && this->IsRunning && this->IsProcessAlive(this->PJ64Handle));

                if (this->ModuleBaseAddress != 0)
                {
                    while (this->OpenSharedMemory() == false && this->IsProcessAlive(this->PJ64Handle))
                    {
                        Sleep(10);
                    }
                    //this->PCAddress = this->FindPCAddress(this->PJ64Handle);
                }
                if (this->GameRamBaseAddress != 0)
                {   // We have the emulator game allocated RAM address

                    MultiLogger::LogMessage("Game RAM Found, start address : %x", this->GameRamBaseAddress);
                    this->DLLData->GameRAMBase = this->GameRamBaseAddress;
                    this->RunMemoryReader();
                }*/
                while (this->OpenSharedMemory() == false && this->IsProcessAlive(this->PJ64Handle))
                {
                    Sleep(10);
                }
                this->RunMemoryReader();
            }
            else
            {
                MultiLogger::LogMessage("Cannot inject tracker dll into %s.\nPlease check your process, ensure that % is in the same folder as the tracker and restart the tracker.", processName, this->PJTrackerDLL);
            }
        }
        else
        {   // An error occured while trying to get a valid handler to read the process memory

            DWORD errorMessageID = GetLastError();
            MultiLogger::LogMessage("Cannot access %s process. Error : %s (%d).\nPlease check your process and restart the tracker.", processName, GetErrorAsString(errorMessageID), errorMessageID);
        }
    }
}


void MemoryReader::RunMemoryReader()
{
    MultiLogger::LogMessage("Reading game memory...");
    //uint32_t PC = 0;
    LONG i = 0;
    do
    {
        while (i < this->DLLData->CurrIndex && i < this->DLLData->MaxSize)
        {
            MultiLogger::LogMessage("PC = 0x%08X, A1 = 0x%08X, GI = 0x%04X, GIRenew = 0x%04X, OVFlags = 0x%04X, OVType = 0x%02X, SceneID = 0x%02X, RoomID = 0x%02X, ID = 0x%02X, From = 0x%02X\n", this->DLLData->Buffer[i].PC, this->DLLData->Buffer[i].A1, this->DLLData->Buffer[i].Query.GI, this->DLLData->Buffer[i].Query.GIRenew, this->DLLData->Buffer[i].Query.OVFlags, this->DLLData->Buffer[i].Query.OVType, this->DLLData->Buffer[i].Query.SceneId, this->DLLData->Buffer[i].Query.RoomId, this->DLLData->Buffer[i].Query.ID, this->DLLData->Buffer[i].Query.From);
            i++;
        }

        if (i > this->DLLData->CurrIndex)
        {
            while (i < this->DLLData->MaxSize)
            {
                MultiLogger::LogMessage("PC = 0x%08X, A1 = 0x%08X, GI = 0x%04X, GIRenew = 0x%04X, OVFlags = 0x%04X, OVType = 0x%02X, SceneID = 0x%02X, RoomID = 0x%02X, ID = 0x%02X, From = 0x%02X\n", this->DLLData->Buffer[i].PC, this->DLLData->Buffer[i].A1, this->DLLData->Buffer[i].Query.GI, this->DLLData->Buffer[i].Query.GIRenew, this->DLLData->Buffer[i].Query.OVFlags, this->DLLData->Buffer[i].Query.OVType, this->DLLData->Buffer[i].Query.SceneId, this->DLLData->Buffer[i].Query.RoomId, this->DLLData->Buffer[i].Query.ID, this->DLLData->Buffer[i].Query.From);
                i++;
            }
            i = 0;
        }
        //ReadProcessMemory(this->PJ64Handle, (LPCVOID)(this->PCAddress), &PC, sizeof(PC), 0);
        /*if (this->DLLData->isValid)//pc == 0x80400BE4)
        {//if (PC == 0x80400BE4)
            MultiLogger::LogMessage("PC = 0x%X", this->DLLData->pc);
            this->DLLData->isValid = 0;
        }*/
        //else
        //if (PC == 0x80400BE4)
        //    MultiLogger::LogMessage("PC = 0x%X", this->DLLData->pc);

        //Sleep(10);
        /*ReadProcessMemory(this->PJ64Handle, (LPCVOID)(this->GameRamBaseAddress), this->RAMData, RAM_SIZE, 0);
        this->CheckCurrentLoadedGame();

        if (this->LoadedGame == NO_GAME)
        {   // The game is not loaded, incorrect or changing between OoT and MM

            MultiLogger::LogMessage("No game loaded. This can occurs for the following reasons:\n- No game is launched\n- Incorrect game is launched\n- The game is switching between OoT and MM game code.\nRetrying in 1 second...");
            Sleep(1000);
        }
        else
        {
            this->ReadEntranceID(this->LoadedGame);
            Sleep(100);
        }*/

    } while (this->IsRunning && this->IsProcessAlive(this->PJ64Handle));

    this->IsRunning = false;
}


uintptr_t MemoryReader::GetModuleBase(DWORD PID, const char* ModuleName)
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
            QString lazyString(currProcess);
            if (lazyString.startsWith(ModuleName) && lazyString.endsWith(".exe"))
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


uintptr_t MemoryReader::FindN64RAM(HANDLE process)
{
    MEMORY_BASIC_INFORMATION mbi;
    uintptr_t addr = 0;

    while (VirtualQueryEx(process, (LPCVOID)addr, &mbi, sizeof(mbi)))
    {
        if (mbi.State == MEM_COMMIT && mbi.Type == MEM_PRIVATE && mbi.RegionSize == 0x800000 && (mbi.Protect & PAGE_READWRITE))
        {
            return (uintptr_t)mbi.BaseAddress;
        }

        addr += mbi.RegionSize;
    }

    return 0;
}


uintptr_t MemoryReader::FindPCAddress(HANDLE process)
{
    uint32_t addr = 0;

    ReadProcessMemory(process, (LPCVOID)(this->ModuleBaseAddress + 0x1B00C4), &addr, sizeof(addr), 0);

    return addr + 0x220;
}


bool MemoryReader::OpenSharedMemory()
{
    HANDLE hMap = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, "PJ64_SHARED_MEM");

    if (!hMap)
    {
        std::cout << "Shared memory not found\n";
        return false;
    }

    this->DLLData = (SharedData*)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedData));

    if (this->DLLData != nullptr)
    {
        //this->DLLData->Base = this->ModuleBaseAddress;
        return true;
    }

    return false;
}


bool MemoryReader::InjectTrackerDLL()
{
    if (this->PJ64Handle == 0)
    {
        return false;
    }

    this->DLLAlloc = VirtualAllocEx(this->PJ64Handle, nullptr, strlen(this->PJTrackerDLL), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    if (!this->DLLAlloc)
    {
        return false;
    }

    bool ret = WriteProcessMemory(this->PJ64Handle, this->DLLAlloc, this->PJTrackerDLL, strlen(this->PJTrackerDLL), nullptr);

    /*HMODULE lpStart = GetModuleHandle(L"kernel32.dll");
    LPVOID st = GetProcAddress(lpStart, "LoadLibraryA");*/

    this->DLLThread = CreateRemoteThread(this->PJ64Handle, nullptr, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, this->DLLAlloc, 0, nullptr );

    if (!this->DLLThread)
        return false;

    Sleep(10);  // Let the time for the DLL to be initialized
/*    WaitForSingleObject(this->DLLThread, INFINITE);
    DWORD exitCode = 0;
    GetExitCodeThread(this->DLLThread, &exitCode);

    MultiLogger::LogMessage("LoadLibrary result = 0x%X\n", exitCode);

    if (GetFileAttributesA(this->PJTrackerDLL) == INVALID_FILE_ATTRIBUTES)
    {
        printf("DLL NOT FOUND\n");
    }*/

    //VirtualFreeEx(hProcess, alloc, 0, MEM_RELEASE);
    //CloseHandle(hThread);
    //CloseHandle(hProcess);

    return true;
}


void MemoryReader::CheckCurrentLoadedGame()
{
    union
    {
        uint32_t Bytes[2]; uint64_t Name;
    } u;

    uint64_t ToFound = 0x5A454C44415A0000; // ZELDAZ

    if (std::endian::native == std::endian::little)
    {
        memcpy(&u.Bytes[1], &this->RAMData[0x11A5EC], sizeof(uint32_t));
        memcpy(&u.Bytes[0], &this->RAMData[0x11A5EC + sizeof(uint32_t)], sizeof(uint32_t));

        // Get rid of the death count
        u.Bytes[0] &= 0xFFFF0000;
    }
    else
    {
        memcpy(&u.Bytes[0], &this->RAMData[0x11A5EC], sizeof(uint32_t));
        memcpy(&u.Bytes[1], &this->RAMData[0x11A5EC + sizeof(uint32_t)], sizeof(uint32_t));

        // Get rid of the death count
        u.Bytes[1] &= 0xFFFF0000;
    }

    if (u.Name == ToFound)
    {   // The current loaded game is Ocarina of Time

        this->LoadedGame = OOT_GAME;
        return;
    }

    ToFound = 0x5A454c4441330000; // ZELDA3

    if (std::endian::native == std::endian::little)
    {
        memcpy(&u.Bytes[1], &this->RAMData[0x1EF694], sizeof(uint32_t));
        memcpy(&u.Bytes[0], &this->RAMData[0x1EF694 + sizeof(uint32_t)], sizeof(uint32_t));

        // Get rid of the death count
        u.Bytes[0] &= 0xFFFF0000;
    }
    else
    {
        memcpy(&u.Bytes[0], &this->RAMData[0x1EF694], sizeof(uint32_t));
        memcpy(&u.Bytes[1], &this->RAMData[0x1EF694 + sizeof(uint32_t)], sizeof(uint32_t));

        // Get rid of the death count
        u.Bytes[1] &= 0xFFFF0000;
    }

    if (u.Name == ToFound)
    {   // The current loaded game is Majora's Mask

        this->LoadedGame = MM_GAME;
        return;
    }

    // No / incorrect game loaded.
    this->LoadedGame = NO_GAME;
}


void MemoryReader::ReadEntranceID(int Game)
{
    this->EntHelper.ReadEntranceID(Game, this->RAMData);
}