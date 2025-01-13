#ifndef GAME_H
#define GAME_H

#include "multi.h"

#define OOT_GAME 0x00
#define MM_GAME 0x01

static void gameServerClose(Game* game);
static void gameClose(Game* game);
static void gameServerReconnect(Game* game);
void gameInit(Game* game, SOCKET sock, int apiProtocol);
static void gameLoadLedger(Game* game);
static void gameLoadApiData(Game* game);
static uint64_t crc64(const void* data, size_t size);
static void memcpy_rev(uint8_t* dest, uint8_t* src, size_t n);
static uint64_t itemKey(uint32_t checkKey, uint8_t gameId, uint8_t playerFrom, uint32_t entriesCount);
static int writeItemLedger(Game* game, uint8_t playerFrom, uint8_t playerTo, uint8_t gameId, uint32_t k, uint16_t gi, uint16_t flags);
static void gameApiItemOut(Game* game);
static void gameApiApplyLedger(Game* game);
static int insertMessage(Game* game, NetMsg* msg);
static void gameApiProcessMessagesIn(Game* game);
static void gameApiProcessMessagesOut(Game* game);
static void gameApiTick(Game* game);
static int gameProcessInputRx(Game* game, uint32_t size);
static int gameProcessRxLedgerEntry(Game* game);
static int gameProcessRxMessage(Game* game);
static void gameProcessInput(Game* game);
static void gameServerJoin(Game* game);
static void gameServerConnect(App* app, Game* game);
static void gameServerTick(App* app, Game* game);
void gameTick(App* app, Game* game);

#endif