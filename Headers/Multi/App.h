#ifndef APP_H
#define APP_H

#pragma comment(lib, "ws2_32.lib")

#include "Multi.h"
#include "Game.h"
#include <QLineEdit>

class App
{

#pragma region Attributes

public:
    /* Hostname or IP address of the remote multiplayer server. */
    const char* serverHost;
    /* Port number of the remote multiplayer server. */
    uint16_t    serverPort;
    /* Listening socket that accepts incoming Project64 connections. */
    SOCKET      socketPj64;
    /* Socket used for the outgoing connection to the Ares emulator. */
    SOCKET      socketAres;
    /* Array of active game slots, one per connected emulator instance. */
    Game        games[MAX_GAMES];

    bool IsRunning;
    bool IsNetEnabled;

#pragma endregion

public:
    /*
    *   Constructs the multiplayer app in a stopped state.
    */
    App();

    /*
    *   Default destructor. Shuts down the app if it was still running.
    */
    ~App();

    /*
    *   Initialize Winsock and reset the internal socket and game state.
    *
    *   @return 0 on success, 1 if WSAStartup failed.
    */
    int appInit();

    /*
    *   Start a non-blocking TCP listening socket for Project64 connections on the given address.
    *
    *   @param Host    The host name or address to bind to.
    *   @param Port    The port to bind to.
    *
    *   @return 0 on success, 1 on failure.
    */
    int appStartPj64(const char* host, uint16_t port);

    /*
    *   Open a non-blocking TCP connection to the Ares emulator on the given address.
    *
    *   @param Host    The host name or address of the Ares server.
    *   @param Port    The port of the Ares server.
    *
    *   @return 0 on success, 1 on failure.
    */
    int appStartAres(const char* host, uint16_t port);

    /*
    *   Assign the given socket to the first free game slot and initialize it with the given API protocol.
    *
    *   @param Sock           The socket associated with the new game.
    *   @param ApiProtocol    The API protocol to use (PJ64 or Ares).
    */
    void appGameInit(SOCKET sock, int apiProtocol);

    /*
    *   Check the Project64 listening socket for incoming connections and register new games.
    */
    void appCheckPj64();

    /*
    *   Check the Ares socket connection progress and register the game when the connection completes.
    */
    void appCheckAres();

    /*
    *   Signal handler that flags the app to stop on SIGINT / SIGTERM.
    *
    *   @param Signum    The received signal number (unused).
    */
    static void sigHandler(int signum);

    /*
    *   Run the main multiplayer loop: accept connections, tick active games, until stopped.
    *
    *   @param NetState    True if networking is enabled.
    *   @param Host        The host line edit used to read the server host.
    *   @param Port        The server port.
    *
    *   @return 0 when the loop exits normally.
    */
    int appRun(bool NetState, QLineEdit* host, uint16_t port);

    /*
    *   Close all sockets, clean up Winsock and release all active games.
    *
    *   @return 0 on completion.
    */
    int appQuit();

};

#endif
