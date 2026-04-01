#pragma once

#define LOG_DEBUG    0

#include "Common.h"

#ifdef _WIN32
#include <QPlainTextEdit>
#include <QScrollBar>


static inline char* GetErrorMsg(int err)
{
    static char buf[256];
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, 0, buf, sizeof(buf), NULL);
    return buf;
}

static inline int sockasync(SOCKET sock, int enable)
{
    u_long mode = enable ? 1 : 0;
    return ioctlsocket(sock, FIONBIO, &mode);
}

static inline void LOGF(const char* fmt, ...)
{
    (void)fmt;

# if LOG_DEBUG
    FILETIME ft;
    GetSystemTimePreciseAsFileTime(&ft);
    SYSTEMTIME st;
    FileTimeToSystemTime(&ft, &st);
    printf("[%02d:%02d:%02d.%03d] ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
# endif
}
# else


static inline int sockasync(SOCKET sock, int enable)
{
    int mode = fcntl(sock, F_GETFL, 0);
    if (mode == -1)
        return -1;
    if (enable)
        mode |= O_NONBLOCK;
    else
        mode &= ~O_NONBLOCK;
    return fcntl(sock, F_SETFL, mode);
}

static inline void LOGF(const char* fmt, ...)
{
#if LOG_DEBUG
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm* tm = localtime(&tv.tv_sec);
    printf("[%02d:%02d:%02d.%03d] ", tm->tm_hour, tm->tm_min, tm->tm_sec, (int)(tv.tv_usec / 1000));
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
#endif
}
#endif

#define MAX_GAMES   8
#define VERSION     0x00000200

#define PROTOCOL_PJ64   0x01
#define PROTOCOL_ARES   0x02

#define STATE_INIT      0x00
#define STATE_CONNECT   0x01
#define STATE_JOIN      0x02
#define STATE_READY     0x03

#define OP_NONE         0x00
#define OP_TRANSFER     0x01
#define OP_MSG          0x02

typedef struct
{
    uint8_t     size;
    uint16_t    clientId;
    char        data[32];
}
NetMsg;

typedef struct
{
    uint64_t key;
    uint8_t  size;
    char     data[128];
}
LedgerFullEntry;

typedef struct
{
    LedgerFullEntry    entry;
    int                ttl;
}
SendQueueEntry;

typedef struct
{
    FILE*               file;
    uint32_t            size;
    uint32_t            capacity;
    SendQueueEntry*     data;
}
SendQueue;

/*
*   This class handles the communication with the main window UI.
*/
class MultiLogger : public QObject
{
    Q_OBJECT

signals:
    /* This function emits a signal to the main thread that a message has arrived. */
    void LogMsgToView(const QString& message);

    /*
    *   Notifies that a new item has been found in the given object.
    *
    *   @param Game           The game the object belong to.
    *   @param ObjectFound    The object in which the item has been found.
    *   @param ItemFound      The item that has been found.
    */
    void NotifyObjectFound(int Game, struct ObjectInfo * ObjectFound, const struct ItemInfo* ItemFound);
    
    /*
    *   Notifies that a new entrance link has been found.
    *
    *   @param OutEntrance      The out entrance to update.
    *   @param InEntrance       The in entrance to update.
    */
    void NotifyEntranceFound(struct SceneEntranceUpdate * OutEntrance, struct SceneEntranceUpdate * InEntrance);

public:

    /* We need this has the Qt connect function cannot take static function as parameter. Also, we don't want to create a new logging object for each class. */
    static MultiLogger* GetLogger();

    /*
    *   Logs the given message to the UI.
    *
    *   @param Format         The string format.
    *   @param ...            The optional arguments that match the string format.
    */
    static inline void LogMessage(const char* Format, ...)
    {
        char tmpBuf[400];
        va_list args;
        va_start(args, Format);
        vsprintf_s(tmpBuf, Format, args);
        va_end(args);

        QString msg(tmpBuf);
        emit GetLogger()->LogMsgToView(msg);
    }
};