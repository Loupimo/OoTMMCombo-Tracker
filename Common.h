#pragma once

#ifdef _WIN32
#define _WIN32_LEAN_AND_MEAN 1
#define _CRT_SECURE_NO_WARNINGS 1
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <stdio.h>
#include <stdarg.h>

#else

typedef int SOCKET;
# include <unistd.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <sys/time.h>
# include <arpa/inet.h>
# include <netdb.h>
# include <netinet/tcp.h>
# include <fcntl.h>
# include <errno.h>
# include <stdio.h>
# include <stdarg.h>
# include <time.h>

# define INVALID_SOCKET          (-1)
# define SOCKET_ERROR            (-1)

# define WSADATA                 int
# define WSAStartup(x,y)         (0)
# define WSACleanup()            (0)
# define WSAGetLastError()       (errno)
# define GetErrorMsg(x)          strerror(x)
# define WSAEWOULDBLOCK          EWOULDBLOCK
# define WSAEINPROGRESS          EINPROGRESS

# define Sleep(x)                usleep((x)*1000)
# define closesocket(fd)         close(fd)
# define _mkdir(x)               mkdir(x, 0777)
# define _fileno(x)              fileno(x)
# define _chsize_s(fd, size)     ftruncate(fd, size)
# define MAX_PATH                16384
# define TIMEVAL                 struct timeval

#endif

#include <stdlib.h>
#include <string.h>
#include <stdint.h>