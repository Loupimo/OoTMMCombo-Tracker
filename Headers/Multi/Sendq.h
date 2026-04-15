#ifndef SENDQ_H
#define SENDQ_H

#include "Multi.h"
#include "Buffer.h"
#define SQ_TTL 2000

void sendqInit(SendQueue* q);
int sendqOpen(SendQueue* q, const uint8_t* uuid);
void sendqClose(SendQueue* q);
static int sendqLocate(const SendQueue* sq, uint64_t key);
static int sendqWrite(SendQueue* sq, const LedgerFullEntry* entry);
static int sendqTransfer(NetBuffer* nb, const LedgerFullEntry* entry);
int sendqAppend(SendQueue* sq, const LedgerFullEntry* entry);
void sendqTick(SendQueue* sq, NetBuffer* nb);
void sendqAck(SendQueue* q, uint64_t key);

#endif