#ifndef BUFFER_H
#define BUFFER_H

#include "Multi.h"

class NetBuffer
{

#pragma region Attributes

public:

	char* data;
	uint32_t    capacity;
	uint32_t    size;
	uint32_t    pos;

#pragma endregion

public:

	NetBuffer();
	~NetBuffer();

	int netBufInit();
	void netBufFree();
	void netBufClear();
	int netBufTransfer(SOCKET sockb, bool IsNetEnabled);
	int netBufIsEmpty();
	void* netBufReserve(uint32_t size);
	int netBufAppend(const void* data, uint32_t size);
};

#endif