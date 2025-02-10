#include "Multi/Multi.h"
#include "Multi/Game.h"
#include "Multi/App.h"
#include "Multi/API.h"
#include "Multi/Protocol.h"
#include "Multi/Sendq.h"
#include "Combo/Items.h"
#include "Combo/Objects.h"
#include "Combo/OvTypes.h"
#include "UI/SceneRenderer.h"

FILE * game_file_log = nullptr;

Game::Game()
{
}

Game::Game(SOCKET sock, int apiProtocol)
{
    this->gameInit(sock, apiProtocol);
}

Game::~Game()
{
    this->gameClose();
}

void Game::gameServerClose()
{
    if (this->socketServer != INVALID_SOCKET)
    {
        closesocket(this->socketServer);
        this->socketServer = INVALID_SOCKET;
    }

    this->timeout = 0;
    this->rxBufferSize = 0;

    this->tx.netBufClear();
}

void Game::gameClose()
{
    if (this->valid == 0)
        return;
    this->valid = 0;
    if (this->socketServer)
    {
        closesocket(this->socketServer);
        this->socketServer = INVALID_SOCKET;
    }

    if (this->socketApi)
    {
        closesocket(this->socketApi);
        this->socketApi = INVALID_SOCKET;
    }

    this->tx.netBufFree();
    free(this->rxBuffer);
    free(this->entries);
    sendqClose(&this->sendq);
    if (game_file_log != nullptr)
    {
        fflush(game_file_log);
        fclose(game_file_log);
    }
    game_file_log = nullptr;
}

void Game::gameServerReconnect()
{
    printf("Disconnected from server, reconnecting...\n");
    this->gameServerClose();
    this->state = STATE_CONNECT;
}

void Game::gameInit(SOCKET sock, int apiProtocol)
{
    /* Set initial values */
    this->valid = 1;
    this->state = STATE_INIT;
    this->delay = 0;
    this->nopAcc = 0;
    this->timeout = 0;
    this->apiError = 0;
    this->apiProtocol = apiProtocol;
    this->socketApi = sock;
    this->socketServer = INVALID_SOCKET;
    protocolInit(this);

    this->tx.netBufInit();

    this->rxBuffer = (char*)malloc(sizeof(char) * 256);
    this->rxBufferSize = 0;

    this->entriesCount = 0;
    this->entriesCapacity = 256;
    this->entries = (LedgerFullEntry*)malloc(this->entriesCapacity * sizeof(LedgerFullEntry));

    sendqInit(&this->sendq);

    memset(&this->msg, 0, sizeof(this->msg));

    /* Log */
    printf("Game created\n");

    if (game_file_log == nullptr)
    {
        _mkdir("./Logs");

        /* Ensure the file exists */
        fopen_s(&game_file_log, "./Logs/Game_Log.csv", "a+");
        fprintf(game_file_log, "Object;OvType;SceneID;RoomID;ObjectID;Item;ItemID\n");
        fflush(game_file_log);
    }

}

void Game::gameLoadLedger()
{
    (void)this;
}

void Game::gameLoadApiData()
{
    uint32_t uuidAddr;

    uuidAddr = protocolRead32(this, this->apiNetAddr + 0x00);
    protocolReadBuffer(this, uuidAddr, 16, this->uuid);

    if (!this->apiError)
    {
        /* Load the send queue */
        sendqOpen(&this->sendq, this->uuid);

        /* Load the ledger */
        this->gameLoadLedger();
        this->state = STATE_CONNECT;
    }

    if (!this->IsNetEnabled)
    {
        this->state = STATE_READY;
    }
}

uint64_t Game::crc64(const void* data, size_t size)
{
    uint64_t crc;
    crc = 0;
    for (size_t i = 0; i < size; ++i)
    {
        crc ^= ((uint64_t)((uint8_t*)data)[i] << 56);
        for (int j = 0; j < 8; ++j)
        {
            if (crc & 0x8000000000000000)
                crc = (crc << 1) ^ 0x42f0e1eba9ea3693;
            else
                crc <<= 1;
        }
    }

    return crc;
}

void Game::memcpy_rev(uint8_t* dest, uint8_t *src, size_t n) {
    for (int i = 0; i < n; i++) dest[n - 1 - i] = src[i];
}

uint64_t Game::itemKey(uint32_t checkKey, uint8_t gameId, uint8_t playerFrom, uint32_t entriesCount)
{
    char buffer[0x10];

    memset(buffer, 0, sizeof(buffer));
    memcpy(buffer + 0x00, &checkKey, 4);
    memcpy(buffer + 0x04, &gameId, 1);
    memcpy(buffer + 0x05, &playerFrom, 1);
    memcpy(buffer + 0x06, &entriesCount, 4);

    return crc64(buffer, sizeof(buffer));
}

int Game::writeItemLedger(uint8_t playerFrom, uint8_t playerTo, uint8_t gameId, uint32_t k, uint16_t gi, uint16_t flags)
{
    LedgerFullEntry e;

    memset(&e, 0, sizeof(e));

    /* Build the key */
    e.key = itemKey(k, gameId, playerFrom, (flags & (1 << 2)) ? this->entriesCount : 0xffffffff);

    /* Build the payload */
    e.size = 0x10;
    memcpy(e.data + 0x00, &playerFrom, 1);
    memcpy(e.data + 0x01, &playerTo, 1);
    memcpy(e.data + 0x02, &gameId, 1);
    memcpy(e.data + 0x04, &k, 4);
    memcpy(e.data + 0x08, &gi, 2);
    memcpy(e.data + 0x0a, &flags, 2);

    /* Write the ledger */
    if (sendqAppend(&this->sendq, &e))
        return 0;
    return 1;
}

void Game::gameApiItemOut()
{
    uint32_t itemBase;
    uint8_t playerFrom;
    uint8_t playerTo;
    uint8_t gameId;
    uint32_t key;
    uint16_t gi;
    uint16_t flags;
    uint8_t buffer[16];

    itemBase = this->apiNetAddr + 0x0c;
    protocolReadBuffer(this, itemBase, 16, buffer);
    playerFrom = buffer[0];
    playerTo = buffer[1];
    gameId = buffer[2];
    key = (buffer[4] << 24) | (buffer[5] << 16) | (buffer[6] << 8) | buffer[7];
    gi = (buffer[8] << 8) | buffer[9];
    flags = (buffer[10] << 8) | buffer[11];

    if (this->apiError)
        return;

    MultiLogger::LogMessage("ITEM OUT - FROM: %d, TO: %d, GAME: %d, KEY: %04X, GI: %04X, FLAGS: %04X", playerFrom, playerTo, gameId, key, gi, flags);

    ParseLedgerFullEntry((char*)buffer, true);
    if (this->IsNetEnabled)
    {
        /* Write and flag as sent */
        if (this->writeItemLedger(playerFrom, playerTo, gameId, key, gi, flags))
        {
            protocolWrite8(this, this->apiNetAddr + 0x08, 0x00);
        }
    }
    else
    {   /* Write and flag as sent */

        protocolWrite8(this, this->apiNetAddr + 0x08, 0x00);
    }
}

void Game::ParseLedgerFullEntry(char* LedgerData, bool IsGoingOut)
{
    uint8_t keyArr[5];
    uint16_t gameItem = 0;

    if (IsGoingOut)
    {   // Outgoing item

        keyArr[0] = (uint8_t)LedgerData[2];
        keyArr[1] = (uint8_t)LedgerData[4];
        keyArr[2] = (uint8_t)LedgerData[5];
        keyArr[3] = (uint8_t)LedgerData[6];
        keyArr[4] = (uint8_t)LedgerData[7];

        gameItem = (uint8_t)LedgerData[8] << 8 | (uint8_t)LedgerData[9];
    }
    else
    {   // Incoming item

        keyArr[0] = (uint8_t)LedgerData[2];
        keyArr[1] = (uint8_t)LedgerData[7];
        keyArr[2] = (uint8_t)LedgerData[6];
        keyArr[3] = (uint8_t)LedgerData[5];
        keyArr[4] = (uint8_t)LedgerData[4];

        gameItem = (uint8_t)LedgerData[9] << 8 | (uint8_t)LedgerData[8];
    }

    ComboItem recievedItem;
    ParseKey(keyArr, &recievedItem);
    CorrectComboItem(&recievedItem);

    MultiLogger::LogMessage("OvType : %d, Scene ID: %d, Room ID: %d, Object ID: %d", recievedItem.OvType, recievedItem.SceneID, recievedItem.RoomID, recievedItem.ObjectID);

    ObjectInfo * matchObject = FindObject(recievedItem);
    const ItemInfo * matchItem = FindItem(gameItem);

    if (recievedItem.GameID == OOT_GAME)
    {
        fprintf(game_file_log, "OoT %s;%02X;%02X;%02X;%04X;%s;%02X\n", matchObject->Location, recievedItem.OvType, recievedItem.SceneID, recievedItem.RoomID, recievedItem.ObjectID, matchItem->ItemName, gameItem);
        MultiLogger::LogMessage("OoT World Object: %s - Item : %s\n", matchObject->Location, matchItem->ItemName);
        //emit MultiLogger::GetLogger()->NotifyObjectFound(OOT_GAME, matchObject, matchItem);
    }
    else
    {   // Majora's Mask

        fprintf(game_file_log, "MM %s;%02X;%02X;%02X;%04X;%s;%02X\n", matchObject->Location, recievedItem.OvType, recievedItem.SceneID, recievedItem.RoomID, recievedItem.ObjectID, matchItem->ItemName, gameItem);
        MultiLogger::LogMessage("MM World Object: %s - Item : %s\n", matchObject->Location, matchItem->ItemName);
        //emit MultiLogger::GetLogger()->NotifyObjectFound(MM_GAME, matchObject, matchItem);
    }

    matchObject->Status = ObjectState::Collected;
    emit GetGameSceneObjects(recievedItem.GameID)[matchObject->RenderScene].Owner->NotifyItemFound(matchObject, matchItem);

    fflush(game_file_log);
}

void Game::gameApiApplyLedger()
{
    uint32_t entryId;
    uint32_t cmdBase;
    uint8_t tmp[8];
    LedgerFullEntry* fe;

    entryId = protocolRead32(this, this->apiNetAddr + 0x04);
    if (entryId == 0xffffffff)
        return;
    if (entryId >= this->entriesCount)
        return;
    if (this->apiError)
        return;

    /* Apply the ledger entry */
    LOGF("LEDGER APPLY #%d\n", entryId);
    fe = this->entries + entryId;
    protocolWrite8(this, this->apiNetAddr + 0x18, 0x01);
    cmdBase = this->apiNetAddr + 0x1c;

    // uint8_t playerFrom, playerTo, gameId
    protocolWriteBuffer(this, cmdBase + 0x00, 3, (uint8_t *)fe->data);

    memcpy_rev(tmp + 0x00, (uint8_t*)fe->data + 0x04, 4); // uint32_t key
    memcpy_rev(tmp + 0x04, (uint8_t*)fe->data + 0x08, 2); // uint16_t gi
    memcpy_rev(tmp + 0x06, (uint8_t*)fe->data + 0x0a, 2); // uint16_t flags
    protocolWriteBuffer(this, cmdBase + 0x04, 8, tmp);

    uint8_t playerFrom;
    uint8_t playerTo;
    uint8_t gameId;
    uint32_t key;
    uint16_t gi;
    uint16_t flags;
    uint8_t buffer[16];

    memcpy(buffer, (uint8_t*)fe->data, 16);

    playerFrom = buffer[0];
    playerTo = buffer[1];
    gameId = buffer[2];
    key = (buffer[4] << 24) | (buffer[5] << 16) | (buffer[6] << 8) | buffer[7];
    gi = (buffer[8] << 8) | buffer[9];
    flags = (buffer[10] << 8) | buffer[11];

    MultiLogger::LogMessage("ITEM IN - FROM: %d, TO: %d, GAME: %d, KEY: %04X, GI: %04X, FLAGS: %04X", playerFrom, playerTo, gameId, key, gi, flags);

    ParseLedgerFullEntry(fe->data, false);
}

int Game::insertMessage(NetMsg* msg)
{
    int index;
    uint8_t sizes[32];

    protocolReadBuffer(this, this->apiNetAddr + 0x28, 32, sizes);

    index = -1;
    for (int i = 0; i < 32; ++i)
    {
        if (sizes[i] == 0)
        {
            index = i;
            break;
        }
    }

    if (index < 0)
        return -1;

    
    protocolWrite8(this, this->apiNetAddr + 0x28 + index, msg->size);
    protocolWrite16(this, this->apiNetAddr + 0x68 + index * 2, msg->clientId);
    if (msg->size > 0) protocolWriteBuffer(this, this->apiNetAddr + 0xa8 + index * 32, msg->size, (uint8_t*)msg->data);
    return 0;
}

void Game::gameApiProcessMessagesIn()
{
    for (int i = 0; i < 128; ++i)
    {
        if (this->msg[i].size)
        {
            if (this->insertMessage(this->msg + i))
                return;
            this->msg[i].size = 0;
        }
    }
}

void Game::gameApiProcessMessagesOut()
{
    uint8_t data[34];
    uint8_t size;
    uint8_t sizes[32];

    protocolReadBuffer(this, this->apiNetAddr + 0x48, 32, sizes);

    for (int i = 0; i < 32; ++i)
    {
        size = sizes[i];
        if (size)
        {
            data[0] = OP_MSG;
            data[1] = size;
            protocolReadBuffer(this, this->apiNetAddr + 0xa8 + i * 32, size, &data[2]);

            /* Send */
            this->tx.netBufAppend(data, size + 2);

            /* Clear */
            protocolWrite8(this, this->apiNetAddr + 0x48 + i, 0x00);
        }
    }
}

void Game::gameApiTick()
{
    uint8_t gameOpOut;
    uint8_t gameOpIn;

    if (this->state == STATE_INIT)
        this->gameLoadApiData();
    if (this->state == STATE_READY)
    {
        /* Handle transfers */
        gameOpOut = protocolRead8(this, this->apiNetAddr + 0x08);
        gameOpIn = protocolRead8(this, this->apiNetAddr + 0x18);
        if (this->apiError)
            return;

        if (gameOpOut == 0x02)
        {
            this->gameApiItemOut();
        }

        if (this->IsNetEnabled)
        {
            if (gameOpIn == 0x00)
            {
                this->gameApiApplyLedger();
            }

            this->gameApiProcessMessagesOut();
            this->gameApiProcessMessagesIn();
        }
    }
}

int Game::gameProcessInputRx(uint32_t size)
{
    int ret;

    while (this->rxBufferSize < size)
    {
        if (this->IsNetEnabled)
        {
            ret = recv(this->socketServer, this->rxBuffer + this->rxBufferSize, size - this->rxBufferSize, 0);
            if (ret == 0)
            {
                this->gameServerReconnect();
                return 0;
            }
            if (ret < 0)
                return 0;
            this->rxBufferSize += ret;
            this->timeout = 0;
        }
    }

    //printf("DATA: ");
    //for (uint32_t i = 0; i < game->rxBufferSize; ++i)
    //    printf("%02x ", (uint8_t)game->rxBuffer[i]);
    //printf("\n");

    return 1;
}

int Game::gameProcessRxLedgerEntry()
{
    LedgerFullEntry fe;
    uint8_t extraSize;

    /* Fetch the full header */
    if (!this->gameProcessInputRx(10))
        return 0;

    extraSize = (uint8_t)this->rxBuffer[9];
    if (!this->gameProcessInputRx(10 + extraSize))
        return 0;

    /* We have a full ledger entry */
    memset(&fe, 0, sizeof(fe));
    memcpy(&fe.key, this->rxBuffer + 1, 8);
    fe.size = extraSize;
    memcpy(fe.data, this->rxBuffer + 10, extraSize);

    LOGF("LEDGER ENTRY: %d bytes\n", extraSize);
    this->rxBufferSize = 0;

    /* Save the ledger entry */
    if (this->entriesCount == this->entriesCapacity)
    {
        this->entriesCapacity *= 2;
        this->entries = (LedgerFullEntry*)realloc(this->entries, this->entriesCapacity * sizeof(LedgerFullEntry));
    }
    memcpy(this->entries + this->entriesCount, &fe, sizeof(LedgerFullEntry));
    this->entriesCount++;

    /* Ack */
    sendqAck(&this->sendq, fe.key);

    return 1;
}

int Game::gameProcessRxMessage()
{
    uint8_t size;
    uint16_t clientId;
    char data[32];

    /* Fetch the header */
    if (!this->gameProcessInputRx(4))
        return 0;

    size = (uint8_t)this->rxBuffer[1];
    memcpy(&clientId, this->rxBuffer + 2, 2);
    if (!this->gameProcessInputRx(4 + size))
        return 0;

    /* We have a full message entry */
    memcpy(data, this->rxBuffer + 4, size);
    this->rxBufferSize = 0;

    /* Store the message */
    for (int i = 0; i < 128; ++i)
    {
        if (this->msg[i].size == 0)
        {
            this->msg[i].size = size;
            this->msg[i].clientId = clientId;
            memcpy(this->msg[i].data, data, size);
            break;
        }
    }

    return 1;
}

void Game::gameProcessInput()
{
    uint8_t op;

    for (;;)
    {
        if (this->rxBufferSize == 0)
        {
            if (!this->gameProcessInputRx(1))
                return;
        }

        op = (uint8_t)this->rxBuffer[0];

        switch (op)
        {
        case OP_NONE:
            this->rxBufferSize = 0;
            break;
        case OP_TRANSFER:
            if (!this->gameProcessRxLedgerEntry())
                return;
            break;
        case OP_MSG:
            if (!this->gameProcessRxMessage())
                return;
            break;
        default:
            fprintf(stderr, "Unknown opcode: %02x\n", op);
            this->rxBufferSize = 0;
            break;
        }
    }
}

void Game::gameServerJoin()
{
    char buf[20];
    uint32_t ledgerBase;

    ledgerBase = this->entriesCount;
    memcpy(buf, this->uuid, 16);
    memcpy(buf + 16, &ledgerBase, 4);
    if (send(this->socketServer, buf, 20, 0) != 20)
    {
        MultiLogger::LogMessage("Unable to send join message\n");
        this->gameServerReconnect();
        return;
    }
    this->state = STATE_READY;
}

void Game::gameServerConnect(App* app)
{
    struct addrinfo hints;
    struct addrinfo* result;
    struct addrinfo* ptr;
    int ret;
    char buf[16];
    uint32_t tmp32;

    /* Resolve the server address and port */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family     = AF_UNSPEC;
    hints.ai_socktype   = SOCK_STREAM;
    hints.ai_protocol   = IPPROTO_TCP;
    snprintf(buf, sizeof(buf), "%d", app->serverPort);

    ret = getaddrinfo(app->serverHost, buf, &hints, &result);
    if (ret != 0)
    {
        MultiLogger::LogMessage("getaddrinfo failed: %d\n", ret);
        return;
    }

    /* Attempt to connect to an address until one succeeds */
    this->socketServer = INVALID_SOCKET;
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
    {
        /* Create a SOCKET for connecting to server */
        this->socketServer = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (this->socketServer == INVALID_SOCKET)
            continue;

        /* Connect to server */
        ret = connect(this->socketServer, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (ret == SOCKET_ERROR)
        {
            closesocket(this->socketServer);
            this->socketServer = INVALID_SOCKET;
            continue;
        }

        /* Make the socket blocking */
        sockasync(this->socketServer, 0);

        /* Send the initial message */
        memcpy(buf, "OOMM2", 5);
        tmp32 = VERSION;
        memcpy(buf + 5, &tmp32, 4);
        if (send(this->socketServer, buf, 9, 0) != 9)
        {
            closesocket(this->socketServer);
            this->socketServer = INVALID_SOCKET;
            continue;
        }

        /* Get the initial reply */
        if (recv(this->socketServer, buf, 11, 0) != 11)
        {
            closesocket(this->socketServer);
            this->socketServer = INVALID_SOCKET;
            continue;
        }
        if (memcmp(buf, "OOMM2", 5))
        {
            closesocket(this->socketServer);
            this->socketServer = INVALID_SOCKET;
            continue;
        }
        memcpy(&this->clientId, buf + 9, 2);

        /* Make the socket non blocking */
        sockasync(this->socketServer, 1);

        break;
    }
    freeaddrinfo(result);

    if (this->socketServer == INVALID_SOCKET)
    {
        MultiLogger::LogMessage("Unable to connect to server at %s:%d\n", app->serverHost, app->serverPort);
        this->delay = 100;
        return;
    }

    /* Log */
    MultiLogger::LogMessage("Connected to server at %s:%d\n", app->serverHost, app->serverPort);
    this->state = STATE_JOIN;
    this->gameServerJoin();
}

void Game::gameServerTick(App* app)
{
    if (this->apiError)
        return;

    if (this->delay)
    {
        --this->delay;
        return;
    }

    this->timeout++;
    if (this->timeout >= 1500)
    {
        this->gameServerReconnect();
        return;
    }

    switch (this->state)
    {
    case STATE_INIT:
        break;
    case STATE_CONNECT:
        this->gameServerConnect(app);
        break;
    case STATE_JOIN:
        break;
    case STATE_READY:
        /* NOP reqs */
        if (this->tx.netBufIsEmpty() && this->nopAcc >= 100)
        {
            this->nopAcc = 0;
            this->tx.netBufAppend("\x00", 1);
        }
        else
            ++this->nopAcc;
        sendqTick(&this->sendq, &this->tx);
        this->tx.netBufTransfer(this->socketServer, this->IsNetEnabled);
        this->gameProcessInput();
        break;
    }
}

void Game::gameTick(App* app)
{
    this->IsNetEnabled = app->IsNetEnabled;
    LOGF("Game tick\n");
    if (apiContextLock(this))
    {
        LOGF("Game API tick\n");
        this->gameApiTick();
        apiContextUnlock(this);
    }

    if (this->IsNetEnabled)
    {   // Net Enabled

        this->gameServerTick(app);
        if (this->apiError)
        {
            MultiLogger::LogMessage("Game disconnected\n");
            this->gameClose();
        }
    }
    LOGF("Game tick end\n");
}
