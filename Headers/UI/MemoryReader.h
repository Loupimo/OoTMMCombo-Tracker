#pragma once

#include "Multi/Multi.h"
#include "Combo/Entrances.h"

#define RAM_SIZE 0x800000
#define ENTRANCE_MAGIC 0xFFFFFFFF	// A flag used to determine if the message is an entrance message or not

#define BUFFER_SIZE 1024
/*
typedef struct ComboItemQuery
{
	int16_t  GI;        // 2 bytes
	int16_t  GIRenew;   // 2 bytes
	uint16_t OVFlags;   // 2 bytes
	uint8_t  OVType;    // 1 byte
	uint8_t  SceneId;   // 1 byte
	uint8_t  RoomId;    // 1 byte
	uint8_t  ID;        // 1 byte
	uint8_t  From;      // 1 byte
	uint8_t  Padding;   // 1 byte
} ComboItemQuery;
*/
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
	uint32_t PC;
	uint32_t Mem;
	uint32_t Query[6];
} Event;

typedef struct SharedData
{
	uint32_t GameVersion[2];
	LONG MaxSize;
	volatile LONG CurrIndex;
	Event Buffer[BUFFER_SIZE];
} SharedData;

class MemoryReader
{

#pragma region Attributes

public:

	DWORD PJ64PID = 0;						// The Project64 process ID.
	HANDLE PJ64Handle = 0;					// The handle used to read Project64 memory.
	uintptr_t ModuleBaseAddress = 0;		// The base address of the Project64 module.
	uintptr_t PCAddress = 0;				// The address of the program counter.
	uintptr_t GameRamBaseAddress = 0;		// The base game RAM address (correspond to the 0x80000000 in PJ64 debugger).
	int LoadedGame = 2;						// The current loaded game in RAM (0 = OoT, 1 = MM, 2 = No game / incorrect game loaded)
	bool IsRunning = false;					// Tells if the process memory should be read.
	uint8_t RAMData[RAM_SIZE] = { 0 };		// A current snapshot of the RAM.
	EntranceHelper EntHelper;				// The module that will handle entrance tracking.
	std::string CurrDirectory;				// The current directory the tracker is located at.
	SharedData * DLLData = nullptr;

#pragma endregion

#pragma region Class creation

public:


	/*
	*   Default memory reader constructor.
	*/
	MemoryReader();

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
	*   Find the base address of the main Project64 module.
	*
	*   @param PID			The process ID the module belongs to.
	*   @param ModuleName   The module name to find.
	*
	*   @return The module base address if valid, 0 otherwise.
	*/
	uintptr_t GetModuleBase(DWORD PID, const char* ModuleName);

	/*
	*   Find the RAM address of the N64 game.
	*
	*   @param Process     The process handler we want to find the game address from.
	*
	*   @return The N64 game RAM address if valid, 0 otherwise.
	*/
	uintptr_t FindN64RAM(HANDLE Process);

	/*
	*   Find the program counter RAM address.
	*
	*   @param Process     The process handler we want to find the program counter address from.
	*
	*   @return The program counter RAM address if valid, 0 otherwise.
	*/
	uintptr_t FindPCAddress(HANDLE process);


	void CheckEvent(Event* CollectedEvent);

	bool InjectTrackerDLL();

	bool OpenSharedMemory();

	/*
	*	Check the current loaded game and update the LoadedGame attribut.
	*/
	void CheckCurrentLoadedGame();

	/*
	*   Read the current entrance ID for the desired game and store the result in the EntranceID attribute.
	*
	*   @param Game       The game to read the entrance ID from.
	*/
	void ReadEntranceID(int Game);

#pragma endregion
};