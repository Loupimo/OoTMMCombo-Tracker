#ifndef API_H
#define API_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define NET_GLOBAL_ADDR 0x800001a0
#define NET_MAGIC       0x905AB56A

class Game;

/*
*   Try to acquire the multiplayer network context lock on the given game memory.
*
*   @param Game    The game whose memory holds the shared network context.
*
*   @return 1 if the lock was acquired (and `apiNetAddr` is populated), 0 otherwise.
*/
int apiContextLock(Game* game);

/*
*   Release the multiplayer network context lock previously acquired on the given game.
*
*   @param Game    The game whose memory holds the shared network context.
*/
void apiContextUnlock(Game* game);

#endif
