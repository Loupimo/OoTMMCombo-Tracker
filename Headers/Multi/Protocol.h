#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "Multi.h"
#include "Game.h"


/*
*   Send the given bytes on the socket, looping until all bytes are transmitted.
*
*   @param S       The socket to send the data on.
*   @param Data    The bytes to send.
*   @param Size    The number of bytes to send.
*
*   @return 1 on success, 0 if the socket reported an error.
*/
static int sockSend(SOCKET s, const void* data, uint32_t size);

/*
*   Receive the given number of bytes from the socket, looping until the buffer is full.
*
*   @param S       The socket to receive the data from.
*   @param Data    The buffer that will receive the bytes.
*   @param Size    The number of bytes to read.
*
*   @return 1 on success, 0 if the socket reported an error.
*/
static int sockRecv(SOCKET s, void* data, uint32_t size);

/*
*   Issue a Project64 read command on the API socket and store the response in the given buffer.
*
*   @param Game    The game whose API socket is used.
*   @param Buf     The destination buffer for the read value.
*   @param Op      The PJ64 read opcode (byte / half / word).
*   @param Size    The number of bytes to read.
*   @param Addr    The emulator memory address to read from.
*/
static void pj64Read(Game* game, void* buf, uint8_t op, uint32_t size, uint32_t addr);

/*
*   Issue a Project64 write command on the API socket with the given payload.
*
*   @param Game    The game whose API socket is used.
*   @param Buf     The source buffer holding the value to write.
*   @param Op      The PJ64 write opcode (byte / half / word).
*   @param Size    The number of bytes to write.
*   @param Addr    The emulator memory address to write to.
*/
static void pj64Write(Game* game, const void* buf, uint8_t op, uint32_t size, uint32_t addr);

/*
*   Send the Ares GDB-style start acknowledgement ('+') on the API socket.
*
*   @param Game    The game whose API socket is used.
*/
static void aresStart(Game* game);

/*
*   Convert a single hexadecimal character to its numeric value.
*
*   @param Ch    The hexadecimal character to decode.
*
*   @return The numeric value in [0, 15], or 0 for any non-hex character.
*/
static uint8_t unhex(char ch);

/*
*   Issue an Ares GDB-style memory read command and decode the hex response into the given buffer.
*
*   @param Game     The game whose API socket is used.
*   @param Addr     The emulator memory address to read from.
*   @param Count    The number of bytes to read.
*   @param Value    The destination buffer that will receive the decoded bytes.
*
*   @return 1 on success, 0 if the transaction failed.
*/
static int aresCommandRead(Game* game, uint32_t addr, int count, uint8_t* value);

/*
*   Issue an Ares GDB-style memory write command with the given payload and wait for the "OK" acknowledgement.
*
*   @param Game     The game whose API socket is used.
*   @param Addr     The emulator memory address to write to.
*   @param Size     The number of bytes to write.
*   @param Value    The source buffer holding the bytes to write.
*
*   @return 1 on success, 0 if the transaction failed.
*/
static int aresCommandWrite(Game* game, uint32_t addr, int size, uint8_t* value);

/*
*   Read an unsigned integer from Ares memory as big-endian bytes.
*
*   @param Game    The game whose API socket is used.
*   @param Addr    The emulator memory address to read from.
*   @param Size    The size of the integer in bytes (1, 2 or 4).
*
*   @return The decoded integer value, or 0 on failure.
*/
static uint32_t aresReadInt(Game* game, uint32_t addr, int size);

/*
*   Write an unsigned integer to Ares memory as big-endian bytes.
*
*   @param Game     The game whose API socket is used.
*   @param Addr     The emulator memory address to write to.
*   @param Value    The integer value to write.
*   @param Size     The size of the integer in bytes (1, 2 or 4).
*/
static void aresWriteInt(Game* game, uint32_t addr, uint32_t value, int size);

/*
*   Read an 8-bit value from emulator memory using the configured API protocol.
*
*   @param Game    The game whose API socket is used.
*   @param Addr    The emulator memory address to read from.
*
*   @return The byte read at the given address.
*/
uint8_t protocolRead8(Game* game, uint32_t addr);

/*
*   Read a 16-bit value from emulator memory using the configured API protocol.
*
*   @param Game    The game whose API socket is used.
*   @param Addr    The emulator memory address to read from.
*
*   @return The half-word read at the given address.
*/
uint16_t protocolRead16(Game* game, uint32_t addr);

/*
*   Read a 32-bit value from emulator memory using the configured API protocol.
*
*   @param Game    The game whose API socket is used.
*   @param Addr    The emulator memory address to read from.
*
*   @return The word read at the given address.
*/
uint32_t protocolRead32(Game* game, uint32_t addr);

/*
*   Read a raw buffer from emulator memory using the configured API protocol.
*
*   @param Game      The game whose API socket is used.
*   @param Addr      The emulator memory address to read from.
*   @param Count     The number of bytes to read.
*   @param Buffer    The destination buffer.
*/
void protocolReadBuffer(Game* game, uint32_t addr, int count, uint8_t* buffer);

/*
*   Write an 8-bit value to emulator memory using the configured API protocol.
*
*   @param Game     The game whose API socket is used.
*   @param Addr     The emulator memory address to write to.
*   @param Value    The byte to write.
*/
void protocolWrite8(Game* game, uint32_t addr, uint8_t value);

/*
*   Write a 16-bit value to emulator memory using the configured API protocol.
*
*   @param Game     The game whose API socket is used.
*   @param Addr     The emulator memory address to write to.
*   @param Value    The half-word to write.
*/
void protocolWrite16(Game* game, uint32_t addr, uint16_t value);

/*
*   Write a 32-bit value to emulator memory using the configured API protocol.
*
*   @param Game     The game whose API socket is used.
*   @param Addr     The emulator memory address to write to.
*   @param Value    The word to write.
*/
void protocolWrite32(Game* game, uint32_t addr, uint32_t value);

/*
*   Write a raw buffer to emulator memory using the configured API protocol.
*
*   @param Game      The game whose API socket is used.
*   @param Addr      The emulator memory address to write to.
*   @param Count     The number of bytes to write.
*   @param Buffer    The source buffer.
*/
void protocolWriteBuffer(Game* game, uint32_t addr, int count, uint8_t* buffer);

/*
*   Initialize the API protocol for the given game (sends the Ares handshake if needed).
*
*   @param Game    The game to initialize.
*/
void protocolInit(Game* game);

#endif