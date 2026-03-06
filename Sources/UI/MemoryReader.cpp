#include "UI/MemoryReader.h"

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
    if (this->PJ64Handle != 0)
    {
        CloseHandle(this->PJ64Handle);
        this->PJ64Handle = 0;
    }

    this->PJ64PID = -1;
}


DWORD MemoryReader::GetProcessIdByName(const char* processName)
{
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    char* currProcess = (char*)malloc(sizeof(char) * 250);
    int i = 0;
    size_t sz = 0;
    if (currProcess && Process32First(snapshot, &entry))
    {
        do
        {
            wcstombs_s(&sz, currProcess, 250, entry.szExeFile, 250);
            if (!_stricmp(currProcess, processName))
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
{
    return OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, false, PID);
}


void MemoryReader::StartMemoryReader()
{
    this->PJ64PID = this->GetProcessIdByName("Project64-EM.exe");

    if (this->PJ64PID != -1)
    {
        this->PJ64Handle = this->OpenDesiredProcess(this->PJ64PID);
        if (this->PJ64Handle != 0)
        {
            this->GameRamBaseAddress = this->FindN64RAM(this->PJ64Handle);
            MultiLogger::LogMessage("Game RAM Found, start address : %x", this->GameRamBaseAddress);
            this->ReadEntranceID();
            MultiLogger::LogMessage("Entrance ID : %x", this->EntranceID);
        }
    }
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


void MemoryReader::ReadEntranceID()
{
    ReadProcessMemory(this->PJ64Handle, (LPCVOID)(this->GameRamBaseAddress + 0x11A5D0), &this->EntranceID, sizeof(this->EntranceID), 0);
}