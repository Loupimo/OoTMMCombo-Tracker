#pragma once

#include "Multi/Multi.h"

class MemoryReader
{

#pragma region Attributes

#pragma endregion

public:

	DWORD PJ64PID = -1;
	HANDLE PJ64Handle = 0;
	uintptr_t GameRamBaseAddress = 0;
	uint32_t EntranceID = 0;

#pragma region Class creation

public:

	MemoryReader();
	~MemoryReader();

	void ResetMemoryReader();

#pragma endregion

#pragma region Methods

public:

	DWORD GetProcessIdByName(const char* processName);
	HANDLE OpenDesiredProcess(DWORD PID);
	void StartMemoryReader();
	uintptr_t FindN64RAM(HANDLE process);
	void ReadEntranceID();

#pragma endregion
};