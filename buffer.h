#ifndef BUFFER_H
#define BUFFER_H

#include "multi.h"

int netBufInit(NetBuffer* nb);
void netBufFree(NetBuffer* nb);
void netBufClear(NetBuffer* nb);
int netBufTransfer(SOCKET sock, NetBuffer* nb);
int netBufIsEmpty(const NetBuffer* nb);
void* netBufReserve(NetBuffer* nb, uint32_t size);
int netBufAppend(NetBuffer* nb, const void* data, uint32_t size);

#endif