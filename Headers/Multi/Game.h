#pragma once

#include "Multi.h"
#include "Buffer.h"

#define OOT_GAME 0x00
#define MM_GAME 0x01
#define NO_GAME 0x02

class App;

enum GameLayout
{
    all,
    oot,
    oot_mq,
    mm,
    mm_jp
};

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

    /* Last from / to world ids observed on a local ITEM OUT. In coop the local player is always
       the sender, so these are reused to attribute hook-captured "nothing" drops that carry no
       player info of their own. Defaults to 0 (unknown) until the first item is collected. */
    uint8_t LocalPlayerFrom;
    uint8_t LocalPlayerTo;
#pragma endregion

public:
    /*
    *   Default constructor. Leaves the game in an invalid state until gameInit is called.
    */
    Game();

    /*
    *   Constructs and initializes the game with the given socket and API protocol.
    *
    *   @param Sock           The socket connected to the emulator API.
    *   @param ApiProtocol    The API protocol to use (PJ64 or Ares).
    */
    Game(SOCKET sock, int apiProtocol);

    /*
    *   Default destructor. Closes the game and releases its resources.
    */
    ~Game();

    /*
    *   Close the server socket and clear the pending transmit buffer.
    */
    void gameServerClose();

    /*
    *   Close the game, release sockets, ledger entries, send queue, and log file.
    */
    void gameClose();

    /*
    *   Close the server socket and queue a reconnection attempt.
    */
    void gameServerReconnect();

    /*
    *   Initialize the game state, buffers, send queue and log file for the given socket.
    *
    *   @param Sock           The socket connected to the emulator API.
    *   @param ApiProtocol    The API protocol to use (PJ64 or Ares).
    */
    void gameInit(SOCKET sock, int apiProtocol);

    /*
    *   Placeholder hook reserved for loading the ledger from disk (currently a no-op).
    */
    void gameLoadLedger();

    /*
    *   Load the UUID and ledger from the emulator, then move to the next state.
    */
    void gameLoadApiData();

    /*
    *   Compute the CRC-64 checksum of the given buffer (polynomial 0x42f0e1eba9ea3693).
    *
    *   @param Data    The buffer to checksum.
    *   @param Size    The number of bytes in the buffer.
    *
    *   @return The computed CRC-64 value.
    */
    static uint64_t crc64(const void* data, size_t size);

    /*
    *   Copy bytes from source to destination in reverse order.
    *
    *   @param Dest    The destination buffer.
    *   @param Src     The source buffer.
    *   @param N       The number of bytes to copy.
    */
    static void memcpy_rev(void* dest, void* src, size_t n);

    /*
    *   Build the ledger key for an item by hashing its check key, game, sender and optional counter.
    *
    *   @param CheckKey        The item check key.
    *   @param GameId          The game ID the item belongs to.
    *   @param PlayerFrom      The player sending the item.
    *   @param EntriesCount    The entries counter used to disambiguate duplicates.
    *
    *   @return The computed 64-bit item key.
    */
    static uint64_t itemKey(uint32_t checkKey, uint8_t gameId, uint8_t playerFrom, uint32_t entriesCount);

    /*
    *   Build a ledger entry for the given item and append it to the send queue.
    *
    *   @param PlayerFrom    The player sending the item.
    *   @param PlayerTo      The player receiving the item.
    *   @param GameId        The game ID the item belongs to.
    *   @param K             The item check key.
    *   @param Gi            The item ID.
    *   @param Flags         The item flags.
    *
    *   @return 1 if the entry was appended, 0 otherwise.
    */
    int writeItemLedger(uint8_t playerFrom, uint8_t playerTo, uint8_t gameId, uint32_t k, uint16_t gi, uint16_t flags);

    /*
    *   Handle an outgoing item from the emulator: log it and push it to the ledger when networking is enabled.
    */
    void gameApiItemOut();

    /*
    *   Apply the next pending ledger entry to the emulator memory and notify the tracker.
    */
    void gameApiApplyLedger();

    /*
    *   Insert a network message into the first free emulator message slot.
    *
    *   @param Msg    The message to insert.
    *
    *   @return 0 on success, -1 if no free slot is available.
    */
    int insertMessage(NetMsg* msg);

    /*
    *   Push pending inbound messages from the message buffer into the emulator slots.
    */
    void gameApiProcessMessagesIn();

    /*
    *   Read outbound messages from the emulator and append them to the transmit buffer.
    */
    void gameApiProcessMessagesOut();

    /*
    *   Run one tick of the emulator API state machine (load, item out, ledger apply, messages).
    */
    void gameApiTick();

    /*
    *   Receive bytes from the server socket until the receive buffer holds the requested size.
    *
    *   @param Size    The total number of bytes that should be available in the receive buffer.
    *
    *   @return 1 when enough data has been read, 0 if the data is not yet available or the connection was lost.
    */
    int gameProcessInputRx(uint32_t size);

    /*
    *   Parse a ledger entry from the receive buffer, store it and acknowledge it on the send queue.
    *
    *   @return 1 if an entry was parsed, 0 if more data is needed.
    */
    int gameProcessRxLedgerEntry();

    /*
    *   Parse a message from the receive buffer and store it in the first free message slot.
    *
    *   @return 1 if a message was parsed, 0 if more data is needed.
    */
    int gameProcessRxMessage();

    /*
    *   Dispatch incoming bytes to the matching opcode handler (transfer, message, nop).
    */
    void gameProcessInput();

    /*
    *   Send the join message (UUID + ledger base) to the server and move to the ready state.
    */
    void gameServerJoin();

    /*
    *   Resolve and connect to the multiplayer server, negotiate the handshake and join it.
    *
    *   @param App    The owning multiplayer app that holds the server host and port.
    */
    void gameServerConnect(App* app);

    /*
    *   Run one tick of the server state machine (connect, join, ready, NOP heartbeat, send queue).
    *
    *   @param App    The owning multiplayer app.
    */
    void gameServerTick(App* app);

    /*
    *   Run one full game tick: emulator API tick and, when networking is enabled, server tick.
    *
    *   @param App    The owning multiplayer app.
    */
    void gameTick(App* app);

    /*
    *   Drain the app's pending "nothing" drops (captured by the hook) and push each one to the
    *   ledger so the coop team receives it. Runs on the network thread; uses the last observed
    *   local ITEM OUT from / to world ids. No-op outside coop or when no local player is known yet.
    *
    *   @param App    The owning multiplayer app holding the pending-nothing queue.
    */
    void gameFlushTrackerNothings(App* app);

    /*
    *   Parse a raw ledger entry, resolve the matching object and item, log it and notify the tracker.
    *
    *   @param LedgerData    The raw ledger entry data.
    *   @param IsGoingOut    True for an outgoing item (local check, ItemSource::NetOut),
    *                        false for an incoming item (ledger apply, ItemSource::NetIn).
    *   @param PlayerFrom    The world that collected the item.
    *   @param PlayerTo      The world the item is destined to.
    */
    void ParseLedgerFullEntry(char* LedgerData, bool IsGoingOut, uint8_t playerFrom, uint8_t playerTo);
};
