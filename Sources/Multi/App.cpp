#include "Multi/Multi.h"
#include "Multi/App.h"
#include <signal.h>

App::App()
{
    this->IsRunning = false;
}

App::~App()
{
    this->appQuit();
}

int App::appInit()
{
    WSADATA wsaData;

    /* Init WS2 */
    if (WSAStartup(MAKEWORD(2, 2), &wsaData))
    {
        MultiLogger::LogMessage("WSAStartup failed: %d\n", WSAGetLastError());
        return 1;
    }

    this->socketPj64 = INVALID_SOCKET;
    this->socketAres = INVALID_SOCKET;
    for (int i = 0; i < MAX_GAMES; ++i)
    {
        this->games[i].valid = 0;
    }

    return 0;
}

int App::appStartPj64(const char* host, uint16_t port)
{
    SOCKET sock;
    struct addrinfo hints;
    struct addrinfo* result;
    struct addrinfo* ptr;
    int ret;
    char buf[16];

    /* Resolve the host and port */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family     = AF_UNSPEC;
    hints.ai_socktype   = SOCK_STREAM;
    hints.ai_protocol   = IPPROTO_TCP;

    sprintf_s(buf, "%d", port);
    ret = getaddrinfo(host, buf, &hints, &result);
    sock = INVALID_SOCKET;
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
    {
        /* Create a SOCKET for connecting to server */
        sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (sock == INVALID_SOCKET)
            continue;

        /* Setup the TCP listening socket */
        ret = bind(sock, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (ret == SOCKET_ERROR)
        {
            closesocket(sock);
            sock = INVALID_SOCKET;
            continue;
        }

        /* Listen to the socket */
        ret = listen(sock, SOMAXCONN);
        if (ret == SOCKET_ERROR)
        {
            closesocket(sock);
            sock = INVALID_SOCKET;
            continue;
        }

        /* We have a good socket */
        break;
    }
    freeaddrinfo(result);
    if (sock == INVALID_SOCKET)
    {
        fprintf(stderr, "Unable to listen to %s:%d\n", host, port);
        return 1;
    }

    /* Set the socket to non-blocking */
    ret = sockasync(sock, 1);

    if (ret == SOCKET_ERROR)
    {
        fprintf(stderr, "Unable to set socket to non-blocking\n");
        closesocket(sock);
        return 1;
    }

    this->socketPj64 = sock;
    return 0;
}

int App::appStartAres(const char* host, uint16_t port)
{
    int ret;
    SOCKET sock;
    struct addrinfo hints;
    struct addrinfo* result;
    struct addrinfo* ptr;
    char buf[16];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        fprintf(stderr, "Unable to create socket\n");
        return 1;
    }

    /* Set the socket to non-blocking */
    ret = sockasync(sock, 1);

    if (ret == SOCKET_ERROR)
    {
        fprintf(stderr, "Unable to set socket to non-blocking\n");
        closesocket(sock);
        return 1;
    }

    /* Resolve the host and port */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family     = AF_UNSPEC;
    hints.ai_socktype   = SOCK_STREAM;
    hints.ai_protocol   = IPPROTO_TCP;

    sprintf_s(buf, "%d", port);
    ret = getaddrinfo(host, buf, &hints, &result);
    if (ret != 0)
    {
        fprintf(stderr, "Unable to resolve %s:%d: %s\n", host, port, GetErrorMsg(WSAGetLastError()));
        closesocket(sock);
        return 1;
    }

    if (result == NULL) {
        fprintf_s(stderr, "Unable to resolve to %s:%d\n", host, port);
        closesocket(sock);
        return 1;
    }

    /* Set TCP_NODELAY / disable Nagle algorithm, we need very low latency during an API tick */
    int tcpNodelayOption = 1;
    ret = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&tcpNodelayOption, sizeof(int));

    /* Connect to the server */
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
    {
        ret = connect(sock, ptr->ai_addr, (int)ptr->ai_addrlen);
        // printf("[ares] connect: %d %d %s %p\n", ret, WSAGetLastError(), GetErrorMsg(WSAGetLastError()), ptr);
        if (ret == 0 || (ret == SOCKET_ERROR && (WSAGetLastError() == WSAEINPROGRESS || WSAGetLastError() == WSAEWOULDBLOCK))) {
            // printf("[ares] connection in progress...\n");
            break;
        }
    }

    if (ptr == NULL) {
        fprintf(stderr, "Unable to connect to %s:%d\n", host, port);
        closesocket(sock);
        return 1;
    }

    this->socketAres = sock;
    return 0;
}

void App::appGameInit(SOCKET sock, int apiProtocol)
{
    /* Create game */
    for (int i = 0; i < MAX_GAMES; ++i)
    {
        Game* g;

        g = &this->games[i];
        if (g->valid)
            continue;
        g->gameInit(sock, apiProtocol);
        return;
    }

    /* No game available */
    closesocket(sock);
}

void App::appCheckPj64()
{
    SOCKET sock;
    fd_set readfds;
    int ret;
    TIMEVAL timeout;

    /* Setup the timeout */
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    /* Setup the selector */
    FD_ZERO(&readfds);
    FD_SET(this->socketPj64, &readfds);

    /* Select */
    ret = select(this->socketPj64+1, &readfds, NULL, NULL, &timeout);
    if (ret < 1)
        return;


    /* Accept the connection */
    sock = accept(this->socketPj64, NULL, NULL);
    if (sock == INVALID_SOCKET)
        return;

    /* Make the socket blocking */
    sockasync(sock, 0);

    this->appGameInit(sock, PROTOCOL_PJ64);
}

void App::appCheckAres()
{
    fd_set writefds;
    int ret;
    char error;
    int len;
    TIMEVAL timeout;
    SOCKET sock;
    u_long mode;

    if (this->socketAres == INVALID_SOCKET) {
        this->appStartAres("localhost", 9123);
        if (this->socketAres == INVALID_SOCKET)
            return;
    }

    /* Setup the timeout */
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    /* Setup the selector */
    FD_ZERO(&writefds);
    FD_SET(this->socketAres, &writefds);

    /* Select */
    ret = select(this->socketAres+1, NULL, &writefds, NULL, &timeout);
    if (ret < 1)
        return;

    // printf("[ares] connection completed\n");
    error = 0;
    len = sizeof(error);
    if (getsockopt(this->socketAres, SOL_SOCKET, SO_ERROR, &error, &len) < 0 || error != 0) {
        // fprintf(stderr, "[ares] connection failed: %d %s\n", error, strerror(error));
        closesocket(this->socketAres);
        this->socketAres = INVALID_SOCKET;
        return;
    }

    /* Make the socket blocking */
    sock = this->socketAres;
    this->socketAres = INVALID_SOCKET;

#ifdef _WIN32
    mode = 0;
    ioctlsocket(sock, FIONBIO, &mode);
#else
    mode = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, mode & ~O_NONBLOCK);
#endif

    this->appGameInit(sock, PROTOCOL_ARES);
}

static sig_atomic_t sSignaled = 0;

void App::sigHandler(int signum)
{
    (void)signum;
    sSignaled = 1;
}

int App::appRun(bool NetState, QLineEdit * host, uint16_t port)
{
    this->IsNetEnabled = NetState;

    std::string t = host->text().toStdString();
    this->serverHost = t.c_str();


    /* Set host and port */
   /* if (host != nullptr)
    {
        size_t len = strlen(host);
        this->serverHost = (char*)malloc(sizeof(char) * len);
        strcpy_s(this->serverHost, len, host);
    }*/
    this->serverPort = port;

    /* Set signal handlers */
    signal(SIGINT, &sigHandler);
    signal(SIGTERM, &sigHandler);

    MultiLogger::LogMessage("Tracker Loop Started.");

    while (this->IsRunning)
    {
        if (sSignaled)
            break;
        this->appCheckPj64();
        this->appCheckAres();
        for (int i = 0; i < MAX_GAMES; ++i)
        {
            //Game* g = new Game(this->LogViewer);

            Game* g = &this->games[i];
            if (!g->valid)
                continue;
            g->gameTick(this);
        }
        Sleep(100);
    }
    return 0;
}

int App::appQuit()
{
    /* Close the socket */
    if (this->socketPj64 != INVALID_SOCKET)
    {
        closesocket(this->socketPj64);
        this->socketPj64 = INVALID_SOCKET;
    }
    if (this->socketAres != INVALID_SOCKET)
    {
        closesocket(this->socketAres);
        this->socketAres = INVALID_SOCKET;
    }

    /* Cleanup WS2 */
    WSACleanup();

    for (uint32_t i = 0; i < MAX_GAMES; i++)
    {
        if (this->games[i].valid != 0)
            this->games[i].gameClose();
    }
    MultiLogger::LogMessage("Tracker Loop Stopped.");

    return 0;
}