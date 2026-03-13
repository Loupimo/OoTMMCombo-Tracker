#pragma once

#include "Multi/Multi.h"
#include "Combo/Entrances.h"

#define RAM_SIZE 0x800000

class MemoryReader
{

#pragma region Attributes

public:

	DWORD PJ64PID = 0;						// The Project64 process ID.
	HANDLE PJ64Handle = 0;					// The handle used to read Project64 memory.
	uintptr_t GameRamBaseAddress = 0;		// The base game RAM address (correspond to the 0x80000000 in PJ64 debugger).
	int LoadedGame = 2;						// The current loaded game in RAM (0 = OoT, 1 = MM, 2 = No game / incorrect game loaded)
	bool IsRunning = false;					// Tells if the process memory should be read.
	uint8_t RAMData[RAM_SIZE] = { 0 };		// A current snapshot of the RAM.
	EntranceHelper EntHelper;				// The module that will handle entrance tracking.

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
	*   Find the RAM address of the N64 game.
	*
	*   @param Process     The process handler we want to find the game address from.
	*
	*   @return The N64 game RAM address if valid, 0 otherwise.
	*/
	uintptr_t FindN64RAM(HANDLE Process);

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