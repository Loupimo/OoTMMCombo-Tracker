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

	/*
	*   Constructs the buffer and allocates its initial backing storage.
	*/
	NetBuffer();

	/*
	*   Default destructor. Frees the backing storage and resets the cursors.
	*/
	~NetBuffer();

	/*
	*   Allocate the initial backing storage of the buffer.
	*
	*   @return 0 on success, -1 if the allocation failed.
	*/
	int netBufInit();

	/*
	*   Release the backing storage of the buffer.
	*/
	void netBufFree();

	/*
	*   Reset the buffer size and position without releasing its storage.
	*/
	void netBufClear();

	/*
	*   Send the pending buffer content over the given socket, or discard it if networking is disabled.
	*
	*   @param Sockb          The socket used to send the buffer content.
	*   @param IsNetEnabled   True to actually send the data, false to drop it locally.
	*
	*   @return 0 on success, -1 on socket error.
	*/
	int netBufTransfer(SOCKET sockb, bool IsNetEnabled);

	/*
	*   Check if the buffer currently holds no data.
	*
	*   @return A non-zero value if the buffer is empty, 0 otherwise.
	*/
	int netBufIsEmpty();

	/*
	*   Reserve the given number of bytes at the end of the buffer and return a pointer to them.
	*
	*   @param Size    The number of bytes to reserve.
	*
	*   @return A pointer to the reserved region, or NULL on allocation failure.
	*/
	void* netBufReserve(uint32_t size);

	/*
	*   Append the given bytes to the end of the buffer.
	*
	*   @param Data    The source bytes to append.
	*   @param Size    The number of bytes to append.
	*
	*   @return 0 on success, -1 on allocation failure.
	*/
	int netBufAppend(const void* data, uint32_t size);
};

#endif