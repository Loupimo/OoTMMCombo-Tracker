#ifndef API_H
#define API_H

#include "Multi.h"
#include "Game.h"

#define NET_GLOBAL_ADDR 0x800001a0
#define NET_MAGIC       0x905AB56A

int apiContextLock(Game* game);
void apiContextUnlock(Game* game);

#endif
