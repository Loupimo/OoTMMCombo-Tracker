#ifndef API_H
#define API_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define NET_GLOBAL_ADDR 0x800001a0
#define NET_MAGIC       0x905AB56A

class Game;

int apiContextLock(Game* game);
void apiContextUnlock(Game* game);

#endif
