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
    const char* serverHost;
    uint16_t    serverPort;
    SOCKET      socketPj64;
    SOCKET      socketAres;
    Game        games[MAX_GAMES];

    bool IsRunning;
    bool IsNetEnabled;

#pragma endregion

public:
    App();
    ~App();

    int appInit();
    int appStartPj64(const char* host, uint16_t port);
    int appStartAres(const char* host, uint16_t port);
    void appGameInit(SOCKET sock, int apiProtocol);
    void appCheckPj64();
    void appCheckAres();
    static void sigHandler(int signum);
    int appRun(bool NetState, QLineEdit* host, uint16_t port);
    int appQuit();

};

#endif
