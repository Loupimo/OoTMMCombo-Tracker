#pragma once

#include "Multi/Multi.h"
#include "Combo/Entrances.h"

#define ENTRANCE_MAGIC 0xF0000000	// A flag used to determine if the message is an entrance message or not
#define BUFFER_SIZE 1024			// The maximum number of events the buffer can holds at the same time

class LogTab;

typedef struct ComboItemQuery
{
	uint8_t  SceneId;   // 1 byte
	uint8_t  OVType;    // 1 byte
	uint8_t  From;      // 1 byte
	uint8_t  IsTreated; // 1 byte. This one does not exist in the original structure, we just use it here as flag to check if we have already treated the event
	uint16_t OVFlags;   // 2 bytes
	uint8_t  ID;        // 1 byte
	uint8_t  RoomId;    // 1 byte
	int16_t  GI;        // 2 bytes
	int16_t  GIRenew;   // 2 bytes
} ComboItemQuery;

typedef struct Event
{
	uint32_t PC;		// The program counter value.
	uint32_t Mem;		// The memory address read. It is also used by the entrance tracking to store information.
	uint32_t Query[6];	// The gathered data. The first 3 are combo key, gi, scene, entrance and the last 3 are always the last respawn player coordinates.
} Event;

// Tracker -> DLL : version deduite du spoiler ("Version:"). Doit rester en phase avec Hooking.h (DLL).
#define HOST_VER_UNKNOWN 0
#define HOST_VER_DEV     1
#define HOST_VER_STABLE  2

typedef struct SharedData
{
	uint32_t GameVersion[2];	// The game version of the currently tracked game.
	LONG MaxSize;				// The maximum number elements the buffer can holds at the same time.
	volatile LONG CurrIndex;	// The index at which the next tracked element should be added.
	Event Buffer[BUFFER_SIZE];	// The buffer containing all tracked events.
	volatile int32_t HostROMVersion;	// Tracker -> DLL : voir HOST_VER_* (0 = inconnu / pas de spoiler).
} SharedData;

class MemoryReader
{

#pragma region Attributes

public:

	DWORD PJ64PID = 0;						// The Project64 process ID.
	HANDLE PJ64Handle = 0;					// The handle used to read Project64 memory.
	int LoadedGame = 2;						// The current loaded game in RAM (0 = OoT, 1 = MM, 2 = No game / incorrect game loaded)
	bool IsRunning = false;					// Tells if the process memory should be read.
	EntranceHelper EntHelper;				// The module that will handle entrance tracking.
	std::string CurrDirectory;				// The current directory the tracker is located at.
	SharedData * DLLData = nullptr;			// A pointer to the shared memory with the injected DLL.
	LogTab* Owner = nullptr;				// The log tab that owns this memory reader.

#pragma endregion

#pragma region Class creation

public:


	/*
	*   Default memory reader constructor.
	*/
	MemoryReader(LogTab * Owner = nullptr);

	/*
	*   Default memory reader destructor.
	*/
	~MemoryReader();

	/*
	*   Reset all the memory reader to its default state.
	*/
	void ResetMemoryReader();

#pragma endregion

#pragma region Methods

public:

	/*
	*   Try to find the process ID matching the desired process name.
	*
	*   @param ProcessName       The desired process name.
	* 
    *   @return The process ID matching the desired process name, -1 if not found.
	*/
	DWORD GetProcessIdByName(const char* ProcessName);

	/*
	*   Create a handle to access the given process ID.
	*
	*   @param PID       The process ID of the process to watch.
	*
	*   @return The handle to access the desired process ID, NULL if a problem occured.
	*/
	HANDLE OpenDesiredProcess(DWORD PID);

	/*
	*   Tells if the process associated to the given handle is alive.
	*
	*   @param Process       The process handle to check.
	*
	*   @return <b>True</b> if the process is alive, <b>false</b> otherwise.
	*/
	bool IsProcessAlive(HANDLE Process);

	/*
	*   Start to read the memory of the correct process.
	*/
	void StartMemoryReader();

	/*
	*   The main routine of the reader.
	*/
	void RunMemoryReader();

	/*
	*   Check the given event to call the correct function to process it.
	* 
	*   @param CollectedEvent       The event to check / process.
	*/
	void CheckEvent(Event* CollectedEvent);

	/*
	*   Inject the tracker's DLL into the project 64 process.
	* 
	*   @return <b>True</b> if the DLL has been injected, <b>false</b> otherwise.
	*/
	bool InjectTrackerDLL();

	/*
	*   Open the shared memory and set up the shared data pointer.
	* 
	*   @return <b>True</b> if the shared memory can be opened, <b>false</b> otherwise.
	*/
	bool OpenSharedMemory();

#pragma endregion
};