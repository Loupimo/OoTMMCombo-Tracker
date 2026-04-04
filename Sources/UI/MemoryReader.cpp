#include "UI/MemoryReader.h"
#include "Combo/OvTypes.h"
#include "Combo/Objects.h"
#include "Combo/Items.h"
#include <bit>

uint32_t byteswap32(uint32_t x)
{
    uint32_t y = (x >> 24) & 0xff;
    y |= ((x >> 16) & 0xff) << 8;
    y |= ((x >> 8) & 0xff) << 16;
    y |= (x & 0xff) << 24;

    return y;
}

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

std::string GetExecutableDirectory()
{
    char path[MAX_PATH];

    GetModuleFileNameA(
        NULL,
        path,
        MAX_PATH
    );

    std::string fullPath = path;

    size_t pos = fullPath.find_last_of("\\/");

    return fullPath.substr(0, pos);
}

std::string GetDLLPath()
{
    std::string dir = GetExecutableDirectory();

    return dir + "\\PJ64OoTMMTracker.dll";
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
    this->CurrDirectory = GetExecutableDirectory();

    if (this->PJ64Handle != 0)
    {
        CloseHandle(this->PJ64Handle);
        this->PJ64Handle = 0;
    }

    this->PJ64PID = 0;
    this->EntHelper.ResetEntranceHelper();
    this->DLLData = nullptr;
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


void MemoryReader::CheckEvent(Event * CollectedEvent)
{
    ComboItem finalItem = { 0 };
    uint32_t collectedItem = 0;
    bool isTreated = 0;

    if (CollectedEvent)
    {   // Check that the collected event is valid

        if ((CollectedEvent->Mem & 0xFFFF0000) == ENTRANCE_MAGIC)
        {   // The message is an entrance message

            this->EntHelper.ParseEntranceMessage(CollectedEvent->Mem, CollectedEvent->Query);
            CollectedEvent->Mem = ENTRANCE_MAGIC;  // Mark the event as treated

            return;
        }
        else if ((CollectedEvent->Query[2] & 0xFFFF0000) == 0xFFFF0000)
        {   // The event comes from a drop nothing actor

            if ((CollectedEvent->Query[2] & 0x0000FF00) >> 2 == false)
            {   // We can treat the event
                

                uint8_t keyArr[5];
                uint32_t key = byteswap32(CollectedEvent->Query[0]);
                keyArr[0] = (CollectedEvent->Query[2] & 0x000000FF) == 0 ? OOT_GAME : MM_GAME;
                memcpy(keyArr + 1, &key, sizeof(uint32_t));

                ParseKey(keyArr, &finalItem);
                CorrectComboItem(&finalItem);
                collectedItem = CollectedEvent->Query[1] & 0x0000FFFF;
                CollectedEvent->Query[2] = 0xFFFFFFFF; // Update the treated flag in the shared memory
            }
            else
            {   // The event is already treated

                isTreated = true;
            }
        }
        else
        {   // The event come from a collected item

            ComboItemQuery q;
            memcpy(&q, CollectedEvent->Query, sizeof(q));

            if (q.IsTreated == false)
            {   // We can treat the event

                q.IsTreated = true;
                memcpy(CollectedEvent->Query, &q, sizeof(q)); // Update the treated flag in the shared memory

                if (q.OVType == OV_NONE)
                {   // We don't want treat an object that don't have an OV_Type (e.g: deku stick collect on a deku baba that will trigger the comboAddItemRawEx Function) 
                    
                    return;
                }

                finalItem.GameID = OOT_GAME;
                finalItem.ObjectID = q.ID;
                finalItem.OvType = q.OVType;
                finalItem.SceneID = q.SceneId;
                finalItem.RoomID = q.RoomId;

                if (CollectedEvent->PC > 0x80700000)
                {   // The event is from MM.

                    finalItem.GameID = MM_GAME;
                }

                CorrectComboItem(&finalItem);
                collectedItem = CollectedEvent->Query[2] & 0x0000FFFF;
            }
            else
            {   // The event is already treated

                isTreated = true;
            }
        }
    }

    if (isTreated == false)
    {   // The event is valid

        ObjectInfo* matchObject = FindObject(finalItem);
        const ItemInfo* matchItem = FindItem(collectedItem);

        if (finalItem.GameID == OOT_GAME)
        {
            MultiLogger::LogMessage("OoT World Object: %s - Item : %s\n", matchObject->Location, matchItem->ItemName);
            emit MultiLogger::GetLogger()->NotifyObjectFound(OOT_GAME, matchObject, matchItem);
        }
        else
        {
            MultiLogger::LogMessage("MM World Object: %s - Item : %s\n", matchObject->Location, matchItem->ItemName);
            emit MultiLogger::GetLogger()->NotifyObjectFound(MM_GAME, matchObject, matchItem);
        }
    }
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
            {   // Injection successful

                while (this->OpenSharedMemory() == false && this->IsProcessAlive(this->PJ64Handle))
                {   // The DLL is not active. The game is probably not launched yet

                    Sleep(10);
                }
                this->RunMemoryReader();
            }
            else
            {
                MultiLogger::LogMessage("Cannot inject tracker dll into %s.\nPlease check your process and ensure that %s and %s are in the same folder as the tracker.\nBe sure that your antivirus did not put it in quarantine.", processName, "PJ64Injector.exe", "PJ64OoTMMTracker.dll");
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
    LONG i = 0;
    do
    {
        while (i < this->DLLData->CurrIndex && i < this->DLLData->MaxSize)
        {
            MultiLogger::LogMessage("PC = 0x%08X, Mem = 0x%08X, Buffer[0] = 0x%08X, Buffer[1] = 0x%08X, Buffer[2] = 0x%08X, Buffer[3] = 0x%08X, Buffer[4] = 0x%08X, Buffer[5] = 0x%08X", this->DLLData->Buffer[i].PC, this->DLLData->Buffer[i].Mem, this->DLLData->Buffer[i].Query[0], this->DLLData->Buffer[i].Query[1], this->DLLData->Buffer[i].Query[2], this->DLLData->Buffer[i].Query[3], this->DLLData->Buffer[i].Query[4], this->DLLData->Buffer[i].Query[5]);
            CheckEvent(&this->DLLData->Buffer[i]);
            i++;
        }

        if (i > this->DLLData->CurrIndex)
        {
            while (i < this->DLLData->MaxSize)
            {
                
                MultiLogger::LogMessage("PC = 0x%08X, Mem = 0x%08X, Buffer[0] = 0x%08X, Buffer[1] = 0x%08X, Buffer[2] = 0x%08X, Buffer[3] = 0x%08X, Buffer[4] = 0x%08X, Buffer[5] = 0x%08X", this->DLLData->Buffer[i].PC, this->DLLData->Buffer[i].Mem, this->DLLData->Buffer[i].Query[0], this->DLLData->Buffer[i].Query[1], this->DLLData->Buffer[i].Query[2], this->DLLData->Buffer[i].Query[3], this->DLLData->Buffer[i].Query[4], this->DLLData->Buffer[i].Query[5]);
                CheckEvent(&this->DLLData->Buffer[i]);
                i++;
            }
            i = 0;
        }

    } while (this->IsRunning && this->IsProcessAlive(this->PJ64Handle));

    this->IsRunning = false;
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
        return true;
    }

    return false;
}


bool MemoryReader::InjectTrackerDLL()
{
    char cmd[512];

    std::string injector = this->CurrDirectory + "\\PJ64Injector.exe";
    std::string dll = this->CurrDirectory + "\\PJ64OoTMMTracker.dll";

    snprintf(cmd, sizeof(cmd), "%s %d \"%s\"", injector.c_str(), this->PJ64PID, dll.c_str());
    return !system(cmd);
}