#ifndef APP_H
#define APP_H

#include "multi.h"

int appInit(App* app);
int appStartPj64(App* app, const char* host, uint16_t port);
int appStartAres(App* app, const char* host, uint16_t port);
static void appGameInit(App* app, SOCKET sock, int apiProtocol);
static void appCheckPj64(App* app);
static void appCheckAres(App* app);
static void sigHandler(int signum);
int appRun(App* app, const char* host, uint16_t port);
int appQuit(App* app);

#endif
