#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "multi.h"


static int sockSend(SOCKET s, const void* data, uint32_t size);
static int sockRecv(SOCKET s, void* data, uint32_t size);
static void pj64Read(Game* game, void* buf, uint8_t op, uint32_t size, uint32_t addr);
static void pj64Write(Game* game, const void* buf, uint8_t op, uint32_t size, uint32_t addr);
static void aresStart(Game* game);
static uint8_t unhex(char ch);
static int aresCommandRead(Game* game, uint32_t addr, int count, uint8_t* value);
static int aresCommandWrite(Game* game, uint32_t addr, int size, uint8_t* value);
static uint32_t aresReadInt(Game* game, uint32_t addr, int size);
static void aresWriteInt(Game* game, uint32_t addr, uint32_t value, int size);
uint8_t protocolRead8(Game* game, uint32_t addr);
uint16_t protocolRead16(Game* game, uint32_t addr);
uint32_t protocolRead32(Game* game, uint32_t addr);
void protocolReadBuffer(Game* game, uint32_t addr, int count, uint8_t* buffer);
void protocolWrite8(Game* game, uint32_t addr, uint8_t value);
void protocolWrite16(Game* game, uint32_t addr, uint16_t value);
void protocolWrite32(Game* game, uint32_t addr, uint32_t value);
void protocolWriteBuffer(Game* game, uint32_t addr, int count, uint8_t* buffer);
void protocolInit(Game* game);

#endif