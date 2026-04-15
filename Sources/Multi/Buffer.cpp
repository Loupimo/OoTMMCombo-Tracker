#include "Multi/Multi.h"
#include "Multi/Buffer.h"


NetBuffer::NetBuffer()
{
    this->netBufInit();
}

NetBuffer::~NetBuffer()
{
    this->netBufFree();
    this->netBufClear();
}

int NetBuffer::netBufInit()
{
    this->capacity = 4096;
    this->size = 0;
    this->pos = 0;
    this->data = (char*)malloc(sizeof(char) * this->capacity);

    return this->data ? 0 : -1;
}

void NetBuffer::netBufFree()
{
    free(this->data);
    this->data = NULL;
}

void NetBuffer::netBufClear()
{
    this->size = 0;
    this->pos = 0;
}

int NetBuffer::netBufTransfer(SOCKET sock, bool IsNetEnabled)
{
    int ret;

    for (;;)
    {
        if (this->pos == this->size)
        {
            this->pos = 0;
            this->size = 0;
            return 0;
        }

        if (IsNetEnabled)
        {
            ret = send(sock, this->data + this->pos, this->size - this->pos, 0);
            if (ret == SOCKET_ERROR)
            {
                if (WSAGetLastError() == WSAEWOULDBLOCK)
                    return 0;
                return -1;
            }
            this->pos += ret;
        }
        else
        {
            this->pos += this->size - this->pos;
        }
    }
}

int NetBuffer::netBufIsEmpty()
{
    return this->size == 0;
}

void* NetBuffer::netBufReserve(uint32_t size)
{
    uint32_t newCapacity;
    uint32_t newSize;
    char* newData;

    /* Handle resize */
    newSize = this->size + size;
    if (newSize > this->capacity)
    {
        newCapacity = this->capacity;
        while (newCapacity < newSize)
            newCapacity *= 2;

        newData = (char*)malloc(newCapacity);
        if (!newData)
            return NULL;

        memcpy(newData, this->data, this->size);
        free(this->data);
        this->data = newData;
        this->capacity = newCapacity;
    }

    /* Return the pointer */
    newData = this->data + this->size;
    this->size = newSize;
    return newData;
}

int NetBuffer::netBufAppend(const void* data, uint32_t size)
{
    void* dst;

    dst = this->netBufReserve(size);
    if (!dst)
        return -1;
    memcpy(dst, data, size);
    return 0;
}
