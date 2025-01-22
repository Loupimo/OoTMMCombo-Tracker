#pragma once

#include "Multi.h"
#include "Buffer.h"

#define OOT_GAME 0x00
#define MM_GAME 0x01

class App;

class Game
{

#pragma region Attributes

public:
    int         valid;
    int         state;
    int         delay;
    unsigned    nopAcc;
    unsigned    timeout;
    int         apiError;
    int         apiProtocol;

    SOCKET      socketApi;
    SOCKET      socketServer;

    uint32_t    apiNetAddr;
    uint8_t     uuid[16];
    uint32_t    ledgerBaseLocal;
    uint32_t    ledgerBaseGame;

    NetBuffer   tx;

    char* rxBuffer;
    uint32_t    rxBufferSize;

    uint16_t         clientId;
    LedgerFullEntry* entries;
    uint32_t         entriesCount;
    uint32_t         entriesCapacity;

    SendQueue   sendq;
    NetMsg      msg[128];

    bool IsNetEnabled;
#pragma endregion

public:
    Game();
    Game(SOCKET sock, int apiProtocol);
    ~Game();

    void gameServerClose();
    void gameClose();
    void gameServerReconnect();
    void gameInit(SOCKET sock, int apiProtocol);
    void gameLoadLedger();
    void gameLoadApiData();
    static uint64_t crc64(const void* data, size_t size);
    static void memcpy_rev(uint8_t* dest, uint8_t* src, size_t n);
    static uint64_t itemKey(uint32_t checkKey, uint8_t gameId, uint8_t playerFrom, uint32_t entriesCount);
    int writeItemLedger(uint8_t playerFrom, uint8_t playerTo, uint8_t gameId, uint32_t k, uint16_t gi, uint16_t flags);
    void gameApiItemOut();
    void gameApiApplyLedger();
    int insertMessage(NetMsg* msg);
    void gameApiProcessMessagesIn();
    void gameApiProcessMessagesOut();
    void gameApiTick();
    int gameProcessInputRx(uint32_t size);
    int gameProcessRxLedgerEntry();
    int gameProcessRxMessage();
    void gameProcessInput();
    void gameServerJoin();
    void gameServerConnect(App* app);
    void gameServerTick(App* app);
    void gameTick(App* app);
    void ParseLedgerFullEntry(char* LedgerData, bool IsGoingOut);
};
