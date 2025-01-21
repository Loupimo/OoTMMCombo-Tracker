#ifndef MULTI_H
#define MULTI_H

#define LOG_DEBUG    0

#include <QPlainTextEdit>
#include <QScrollBar>

#ifdef _WIN32
    #define _WIN32_LEAN_AND_MEAN 1
    #define _CRT_SECURE_NO_WARNINGS 1
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #include <stdio.h>
    #include <stdarg.h>
    #include <direct.h>
    #include <io.h>

    static inline char *strerror(int err)
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
        #if LOG_DEBUG
        FILETIME ft;
        GetSystemTimePreciseAsFileTime(&ft);
        SYSTEMTIME st;
        FileTimeToSystemTime(&ft, &st);
        printf("[%02d:%02d:%02d.%03d] ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
        #endif
    }

    /*static inline void LogMsgToView(QPlainTextEdit* View, const char* fmt, ...)
    {
        if (View)
        {
            char tmpBuf[400];
            va_list args;
            va_start(args, fmt);
            vsprintf_s(tmpBuf, fmt, args);
            va_end(args);

            // Very hacky way. Sending too many messages at the same time will make the program crash probably to data race between this thread and the GUI thread
            View->appendPlainText(tmpBuf);
            View->verticalScrollBar()->setValue(View->verticalScrollBar()->maximum());
        }
    }*/
#else
    typedef int SOCKET;
    #include <unistd.h>
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <sys/time.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <netinet/tcp.h>
    #include <fcntl.h>
    #include <errno.h>
    #include <stdio.h>
    #include <stdarg.h>
    #include <time.h>

    #define INVALID_SOCKET          (-1)
    #define SOCKET_ERROR            (-1)

    #define WSADATA                 int
    #define WSAStartup(x,y)         (0)
    #define WSACleanup()            (0)
    #define WSAGetLastError()       (errno)
    #define WSAEWOULDBLOCK          EWOULDBLOCK
    #define WSAEINPROGRESS          EINPROGRESS

    #define Sleep(x)                usleep((x)*1000)
    #define closesocket(fd)         close(fd)
    #define _mkdir(x)               mkdir(x, 0777)
    #define _fileno(x)              fileno(x)
    #define _chsize_s(fd, size)     ftruncate(fd, size)
    #define MAX_PATH                16384
    #define TIMEVAL                 struct timeval

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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


class MultiLogger : public QObject
{
    Q_OBJECT

signals:
    /* This function emits a signal to the main thread that a message has arrived. */
    void LogMsgToView(const QString& message);

public:

    /* We need this has the Qt connect function cannot take static function as parameter. Also, we don't want to create a new logging object for each class. */
    static MultiLogger* GetLogger();

    static inline void LogMessage(const char* fmt, ...)
    {
        char tmpBuf[400];
        va_list args;
        va_start(args, fmt);
        vsprintf_s(tmpBuf, fmt, args);
        va_end(args);

        QString msg(tmpBuf);
        emit GetLogger()->LogMsgToView(msg);
    }

};

#endif
