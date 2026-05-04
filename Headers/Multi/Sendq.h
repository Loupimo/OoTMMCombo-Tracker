#ifndef SENDQ_H
#define SENDQ_H

#include "Multi.h"
#include "Buffer.h"
#define SQ_TTL 2000

/*
*   Zero-initialize a SendQueue without allocating any backing storage or opening a file.
*
*   @param Q    The send queue to initialize.
*/
void sendqInit(SendQueue* q);

/*
*   Open (or create) the persistent send queue file for the given UUID and load its entries into memory.
*
*   @param Q      The send queue to open.
*   @param Uuid   The 16-byte UUID used to derive the per-player data directory path.
*
*   @return 0 on success, -1 if the file could not be opened.
*/
int sendqOpen(SendQueue* q, const uint8_t* uuid);

/*
*   Close the send queue file and free the in-memory entry buffer.
*
*   @param Q    The send queue to close.
*/
void sendqClose(SendQueue* q);

/*
*   Search the send queue for an entry with the given key and return its index.
*
*   @param Sq     The send queue to search.
*   @param Key    The 64-bit ledger key to locate.
*
*   @return The zero-based index of the matching entry, or -1 if not found.
*/
static int sendqLocate(const SendQueue* sq, uint64_t key);

/*
*   Append a ledger entry to the send queue in memory and persist it to disk.
*   If an entry with the same key already exists it is not duplicated.
*
*   @param Sq       The send queue to write to.
*   @param Entry    The ledger entry to append.
*
*   @return The index of the entry (existing or newly written), or -1 on allocation failure.
*/
static int sendqWrite(SendQueue* sq, const LedgerFullEntry* entry);

/*
*   Serialize a ledger entry into the given net buffer as an OP_TRANSFER packet.
*
*   @param Nb       The net buffer to append the serialized entry to.
*   @param Entry    The ledger entry to serialize.
*
*   @return 0 on success, -1 if the buffer reservation failed.
*/
static int sendqTransfer(NetBuffer* nb, const LedgerFullEntry* entry);

/*
*   Write a ledger entry to the send queue (in memory and on disk) without immediately transmitting it.
*
*   @param Sq       The send queue to append to.
*   @param Entry    The ledger entry to queue.
*
*   @return 0 on success, -1 on failure.
*/
int sendqAppend(SendQueue* sq, const LedgerFullEntry* entry);

/*
*   Iterate over all pending send queue entries and transmit each one whose TTL has expired.
*
*   @param Sq    The send queue to process.
*   @param Nb    The net buffer used to accumulate outgoing packets.
*/
void sendqTick(SendQueue* sq, NetBuffer* nb);

/*
*   Acknowledge a delivered ledger entry by key: remove it from the queue and truncate the backing file.
*
*   @param Q      The send queue to update.
*   @param Key    The 64-bit ledger key of the entry to acknowledge.
*/
void sendqAck(SendQueue* q, uint64_t key);

#endif