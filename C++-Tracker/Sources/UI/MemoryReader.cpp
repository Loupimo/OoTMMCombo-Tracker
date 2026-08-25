#include "UI/MemoryReader.h"
#include "UI/LogTab.h"
#include "UI/OoTMMComboTracker.h"
#include "Combo/OvTypes.h"
#include "Combo/Objects.h"
#include "Combo/Items.h"
#include "Common.h"
#include <bit>

// SharedData::GameVersion words written by a stable OoTMM build. Any other value is a dev build.
// Used to translate stable-build item IDs into the tracker's internal numbering (ResolveRawItemID).
#define STABLE_GAME_VERSION_0 0x69F7A146
#define STABLE_GAME_VERSION_1 0x224AFE45

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

MemoryReader::MemoryReader(LogTab* Owner)
{
    this->ResetMemoryReader();
    this->Owner = Owner;
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


std::string MemoryReader::GetProcessPath(DWORD pid)
{
    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);

    if (!process)
        return {};

    char path[MAX_PATH];
    DWORD size = MAX_PATH;

    if (!QueryFullProcessImageNameA(process, 0, path, &size))
    {
        CloseHandle(process);
        return {};
    }

    CloseHandle(process);

    return std::string(path, size);
}


HANDLE MemoryReader::OpenDesiredProcess(DWORD PID)
{
    return OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, PID);
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


bool MemoryReader::OpenSharedMemory()
{
    this->SharedMemoryHandle = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, "PJ64_SHARED_MEM");

    if (!this->SharedMemoryHandle)
    {
        MultiLogger::LogMessage("Shared memory not found.");
        return false;
    }

    this->DLLData = (SharedData*)MapViewOfFile(this->SharedMemoryHandle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedData));

    if (this->DLLData != nullptr)
    {
        MultiLogger::LogMessage("Shared memory found !");
        return true;
    }

    return false;
}



bool MemoryReader::CopyTrackerDLL()
{
    std::string pj64Path = this->GetProcessPath(this->PJ64PID);

    if (pj64Path.empty())
    {
        MultiLogger::LogMessage("Cannot retrieve Project64 executable path.");

        return false;
    }

    std::filesystem::path project64Path(pj64Path);
    std::filesystem::path project64Directory = project64Path.parent_path();
    std::filesystem::path pluginDirectory = project64Directory / "Plugin";
    std::filesystem::path source = std::filesystem::path(this->CurrDirectory) / "PJ64OoTMMTracker.dll";

    this->PJTrackerDLLPath = pluginDirectory / "PJ64OoTMMTracker.dll";

    MultiLogger::LogMessage("Project64 path: %s", project64Path.string().c_str());
    MultiLogger::LogMessage("Plugin directory: %s", pluginDirectory.string().c_str());
    MultiLogger::LogMessage("Tracker DLL: %s", source.string().c_str());

    // Check that tracker's DLL exist.
    if (!std::filesystem::exists(source))
    {
        MultiLogger::LogMessage("Tracker DLL does not exist: %s", source.string().c_str());

        return false;
    }

    // Check that Plugin folder exist.
    if (!std::filesystem::exists(pluginDirectory))
    {
        MultiLogger::LogMessage("Project64 Plugin directory does not exist: %s", pluginDirectory.string().c_str());

        return false;
    }

    try
    {
        std::filesystem::copy_file(source, this->PJTrackerDLLPath, std::filesystem::copy_options::overwrite_existing);
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        MultiLogger::LogMessage("Failed to copy tracker DLL: %s", e.what());

        return false;
    }

    MultiLogger::LogMessage("Tracker DLL copied successfully to %s", this->PJTrackerDLLPath);

    return true;
}


bool MemoryReader::LoadTrackerPlugin()
{
    return this->CopyTrackerDLL();
}


bool MemoryReader::IsModuleLoaded(DWORD processId, const char* moduleName)
{
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);

    if (snapshot == INVALID_HANDLE_VALUE)
        return false;

    MODULEENTRY32W entry{};
    entry.dwSize = sizeof(entry);

    bool found = false;

    if (Module32FirstW(snapshot, &entry))
    {
        do
        {
            if (_stricmp(QString::fromWCharArray(entry.szModule).toStdString().c_str(), moduleName) == 0)
            {
                found = true;
                break;
            }

        } while (Module32NextW(snapshot, &entry));
    }

    CloseHandle(snapshot);

    return found;
}


bool MemoryReader::RemoveTrackerDLL()
{
    if (this->PJTrackerDLLPath.empty())
    {
        return true;
    }

    std::error_code error;

    bool removed = std::filesystem::remove(this->PJTrackerDLLPath, error);

    if (removed)
    {
        MultiLogger::LogMessage("Tracker DLL removed from Project64 Plugin folder.");

        return true;
    }

    if (error)
    {
        MultiLogger::LogMessage("Cannot remove tracker DLL: %s", error.message().c_str());
    }
    else
    {
        MultiLogger::LogMessage("Tracker DLL was not found.");
    }

    return false;
}


HWND MemoryReader::GetProject64Window()
{
    std::vector<HWND> windows = this->GetProject64Windows();

    for (HWND hwnd : windows)
    {
        if (!IsWindowVisible(hwnd))
            continue;

        // A window without an owner is generally the application's main window.
        if (GetWindow(hwnd, GW_OWNER) != nullptr)
            continue;

        return hwnd;
    }

    return nullptr;
}


BOOL CALLBACK MemoryReader::EnumProject64WindowsProc(HWND hwnd, LPARAM lParam)
{
    auto* search = reinterpret_cast<Project64WindowSearch*>(lParam);

    DWORD pid = 0;

    GetWindowThreadProcessId(hwnd, &pid);

    if (pid == search->pid)
    {
        search->windows->push_back(hwnd);
    }

    return TRUE;
}


std::vector<HWND> MemoryReader::GetProject64Windows()
{
    std::vector<HWND> windows;

    Project64WindowSearch search;
    search.pid = this->PJ64PID;
    search.windows = &windows;

    EnumWindows(MemoryReader::EnumProject64WindowsProc, reinterpret_cast<LPARAM>(&search));

    return windows;
}


bool MemoryReader::OpenProject64Settings()
{
    HWND hwnd = this->GetProject64Window();

    if (hwnd == nullptr)
    {
        MultiLogger::LogMessage("Cannot find Project64 main window.");

        return false;
    }

    MultiLogger::LogMessage("Project64 main window found: 0x%p", hwnd);

    // Restaurer la fenêtre si elle est minimisée.
    if (IsIconic(hwnd))
    {
        ShowWindow(hwnd, SW_RESTORE);
    }

    // Donner le focus à Project64.
    SetForegroundWindow(hwnd);

    // Petite attente pour laisser Windows effectuer
    // le changement de fenêtre active.
    Sleep(100);

    INPUT inputs[4] = {};

    // CTRL down
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_CONTROL;

    // T down
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = 'T';

    // T up
    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = 'T';
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;

    // CTRL up
    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = VK_CONTROL;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

    UINT sent = SendInput(4, inputs, sizeof(INPUT));

    if (sent != 4)
    {
        MultiLogger::LogMessage("SendInput failed. Sent %u/4 events. Error: %lu", sent, GetLastError());

        return false;
    }

    MultiLogger::LogMessage("Ctrl+T sent to Project64.");

    return true;
}


HWND MemoryReader::WaitForSettingsWindow()
{
    constexpr DWORD timeout = 3000;
    constexpr DWORD interval = 10;

    DWORD elapsed = 0;

    while (elapsed < timeout && this->IsRunning)
    {
        std::vector<HWND> currentWindows = this->GetProject64Windows();

        for (HWND hwnd : currentWindows)
        {
            bool existedBefore = false;

            for (HWND oldHwnd :
            this->PJ64WindowsBeforeSettings)
            {
                if (oldHwnd == hwnd)
                {
                    existedBefore = true;
                    break;
                }
            }

            if (!existedBefore)
            {
                // A new window belonging to P64 has just appeared.
                if (IsWindowVisible(hwnd))
                {
                    // Skip the debug DLL console window (AllocConsole). It is also a new top-level
                    // window owned by Project64, so closing it later with WM_CLOSE (step 8) would
                    // terminate the whole emulator process instead of just the Settings dialog.
                    char className[64] = { 0 };

                    if (GetClassNameA(hwnd, className, sizeof(className)) != 0
                        && lstrcmpA(className, "ConsoleWindowClass") == 0)
                    {
                        continue;
                    }

                    return hwnd;
                }
            }
        }

        Sleep(interval);
        elapsed += interval;
    }

    return nullptr;
}



void MemoryReader::CheckEvent(Event * CollectedEvent)
{
    ComboItem finalItem = { 0 };
    uint32_t collectedItem = 0;
    bool isTreated = 0;
    // The hook owns "nothing" drops (which never travel over the network); real items
    // captured by the hook are only authoritative in single mode. The tracker uses the
    // source to route the event (see OoTMMComboTracker::UpdateTrackedObject).
    ItemSource source = ItemSource::HookItem;

    if (CollectedEvent)
    {   // Check that the collected event is valid

        if ((CollectedEvent->Mem & 0xF0000000) == ENTRANCE_MAGIC)
        {   // The message is an entrance message

            this->EntHelper.ParseEntranceMessage(CollectedEvent->Mem, CollectedEvent->Query);
            CollectedEvent->Mem = ENTRANCE_MAGIC;  // Mark the event as treated

            return;
        }
        else if ((CollectedEvent->Query[2] & 0xFFFF0000) == 0xFFFF0000)
        {   // The event comes from a drop nothing actor

            source = ItemSource::HookNothing;

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

                // Coop: "nothing" drops are never sent over the wire by OoTMM, so push them to the
                // ledger ourselves (via the network thread) to keep the whole team's shared map in
                // sync. The Game module fills in the from / to worlds and broadcasts + dedups it.
                if (this->Owner != nullptr
                    && this->Owner->EnableMultiplayer
                    && this->Owner->Tracker != nullptr
                    && this->Owner->WinOwner != nullptr
                    && this->Owner->WinOwner->ROMSettings.Mode == GameMode::coop)
                {
                    TrackerNothing nothing;
                    nothing.gameId = keyArr[0];
                    // The ledger round-trip assembles the key from k's big-endian bytes (writeItemLedger
                    // stores k little-endian, ParseLedgerFullEntry reads it back reversed). The raw
                    // Query[0] is already in that big-endian order, so send it as-is. Sending the local
                    // byteswap32(Query[0]) would double-reverse and resolve to the wrong object.
                    nothing.key = CollectedEvent->Query[0];
                    nothing.gi = (uint16_t)collectedItem;
                    this->Owner->Tracker->QueueTrackerNothing(nothing);
                }
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
        // On a stable ROM the game reports the lower stable item IDs; translate them into the
        // tracker's internal (dev) numbering so the lookup matches. nothing.gi above keeps the
        // raw native ID on purpose (it is replayed to the game / network unchanged).
        const ItemInfo* matchItem = FindItem(ResolveRawItemID(collectedItem));

        // The hook only observes the local game, so the collecting / receiving world is
        // unknown here (-1). The tracker resolves the routing from the source and mode.
        if (finalItem.GameID == OOT_GAME)
        {
            MultiLogger::LogMessage("OoT World Object: %s - Item : %s\n", matchObject->Location, matchItem->ItemName);
            emit MultiLogger::GetLogger()->NotifyObjectFound(OOT_GAME, matchObject, matchItem, source, -1, -1);
        }
        else
        {
            MultiLogger::LogMessage("MM World Object: %s - Item : %s\n", matchObject->Location, matchItem->ItemName);
            emit MultiLogger::GetLogger()->NotifyObjectFound(MM_GAME, matchObject, matchItem, source, -1, -1);
        }
    }
}


void MemoryReader::StartMemoryReader()
{
    this->IsRunning = true;

    const char* processName = "Project64";

    // ---------------------------------------------------------
    // 1. Wait for Project64
    // ---------------------------------------------------------

    do
    {
        this->PJ64PID = this->GetProcessIdByName(processName);

        if (this->PJ64PID == 0)
        {
            MultiLogger::LogMessage("No %s-EM 1.0.3-PJ-3.0.1 or %s-EM 1.1.0-PJ-3.0.1 process found. Retrying in 1 second...\n",  processName, processName);
            Sleep(1000);
        }

    } while (this->PJ64PID == 0 && this->IsRunning);

    if (!this->IsRunning)
        return;

    // ---------------------------------------------------------
    // 2. Open Project64 with minimum privileges.
    // ---------------------------------------------------------

    this->PJ64Handle = this->OpenDesiredProcess(this->PJ64PID);

    if (this->PJ64Handle == nullptr)
    {
        DWORD errorMessageID = GetLastError();
        MultiLogger::LogMessage("Cannot access %s process. Error : %s (%d).\n", processName, GetErrorAsString(errorMessageID), errorMessageID);
        return;
    }

    // ---------------------------------------------------------
    // 3. Copy DLL into Project64/Plugin
    // ---------------------------------------------------------

    if (!this->LoadTrackerPlugin() && !IsModuleLoaded(this->PJ64PID, "PJ64OoTMMTracker.dll"))
    {
        MultiLogger::LogMessage("Cannot install tracker DLL.");

        CloseHandle(this->PJ64Handle);
        this->PJ64Handle = nullptr;

        return;
    }

    // ---------------------------------------------------------
    // 4. Remember all existing P64 windows.
    // ---------------------------------------------------------

    this->PJ64WindowsBeforeSettings = this->GetProject64Windows();

    // ---------------------------------------------------------
    // 5. Open Settings with Ctrl+T to force PJ64 to load or dll.
    // ---------------------------------------------------------

    if (!this->OpenProject64Settings())
    {
        MultiLogger::LogMessage("Cannot open Project64 Settings.");

        CloseHandle(this->PJ64Handle);
        this->PJ64Handle = nullptr;

        return;
    }

    // ---------------------------------------------------------
    // 6. Immediately start looking for Settings.
    //
    //    The function runs quickly and remembers the HWND.
    // ---------------------------------------------------------

    HWND settingsWindow = this->WaitForSettingsWindow();

    // ---------------------------------------------------------
    // 7. Wait until the DLL creates the shared memory.
    // ---------------------------------------------------------

    bool sharedMemoryOpened = false;

    while (this->IsRunning && this->IsProcessAlive(this->PJ64Handle))
    {
        if (this->OpenSharedMemory())
        {
            sharedMemoryOpened = true;
            break;
        }

        Sleep(10);
    }

    if (!this->IsRunning)
        return;

    if (!sharedMemoryOpened)
    {
        MultiLogger::LogMessage("Tracker DLL failed to initialize.");

        return;
    }

    MultiLogger::LogMessage("Tracker DLL successfully loaded.");

    // ---------------------------------------------------------
    // 8. Close Settings.
    // ---------------------------------------------------------

    if (settingsWindow != nullptr)
    {
        PostMessage(settingsWindow, WM_CLOSE, 0, 0);

        MultiLogger::LogMessage("Project64 Settings closed.");
    }
    else
    {
        MultiLogger::LogMessage("Warning: Project64 Settings window could not be identified. The dll may have not been loaded correctly.");
    }

    // ---------------------------------------------------------
    // 9. Normal tracking.
    // ---------------------------------------------------------

    this->RunMemoryReader();

    // ---------------------------------------------------------
    // 10. Ask the DLL to unload itself.
    // ---------------------------------------------------------

    bool dllUnloaded = false;

    if (this->DLLData != nullptr)
    {
        MultiLogger::LogMessage("Requesting tracker DLL shutdown...");

        this->DLLData->Command = TrackerCommand::Shutdown;

        // Wait for DLL to disappear from Project64.
        for (int i = 0; i < 100; ++i)
        {
            if (!IsModuleLoaded(this->PJ64PID, "PJ64OoTMMTracker.dll"))
            {
                dllUnloaded = true;
                break;
            }

            Sleep(50);
        }
    }

    // ---------------------------------------------------------
    // 11. Now that the DLL is gone, close shared memory.
    // ---------------------------------------------------------

    if (this->DLLData != nullptr)
    {
        UnmapViewOfFile(this->DLLData);
        this->DLLData = nullptr;
    }

    if (this->SharedMemoryHandle != nullptr)
    {
        CloseHandle(this->SharedMemoryHandle);
        this->SharedMemoryHandle = nullptr;
    }

    // ---------------------------------------------------------
    // 12. Remove DLL from Plugin directory.
    // ---------------------------------------------------------

    if (dllUnloaded)
    {
        MultiLogger::LogMessage("Tracker DLL successfully unloaded.");

        this->RemoveTrackerDLL();
    }
    else
    {
        MultiLogger::LogMessage("Tracker DLL is still loaded. Cannot safely remove it.");
    }

    if (this->IsRunning && this->PJ64Handle != 0 && !this->IsProcessAlive(this->PJ64Handle))
    {   // PJ64 was alive and died unexpectedly: ask the GUI thread to stop tracking.
        // Without the IsRunning check, a manual stop would also reach this branch
        // and re-fire pressed(), causing the tracker to restart itself.

        MultiLogger::LogMessage("Project 64 has been closed. Stop tracking...");
        this->Owner->LaunchButton->pressed();
    }
}


void MemoryReader::RunMemoryReader()
{
    MultiLogger::LogMessage("Reading game memory...");
    LONG i = 0;
    bool versionDetected = false;
    do
    {
        if (!versionDetected && this->DLLData->GameVersion[0] != 0)
        {   // The ROM is loaded (GameVersion is populated): derive the build from the DLL-reported
            // game version so stable-build item IDs get translated (see ResolveRawItemID). A spoiler
            // log, if loaded, overrides this with its (more reliable) "Version:" line.

            SetActiveROMVersion(
                (this->DLLData->GameVersion[0] == STABLE_GAME_VERSION_0 &&
                 this->DLLData->GameVersion[1] == STABLE_GAME_VERSION_1)
                    ? ROMVersion::stable : ROMVersion::dev);
            versionDetected = true;
        }

        // Si un spoiler a fixe la version (source la plus fiable), la pousser au DLL pour qu'il
        // choisisse le bon "Nothing" ID meme en session OoT-only (ou le hook MM ne tranche pas).
        if (IsActiveROMVersionFromSpoiler())
        {
            this->DLLData->HostROMVersion = (GetActiveROMVersion() == ROMVersion::stable_30_1) ? HOST_VER_STABLE : HOST_VER_DEV;
        }

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
}
