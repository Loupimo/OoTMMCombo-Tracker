#pragma once

#include "pch.h"

#define PC_MAX              32  // Max PCs / game to observe
#define CACHE_MAGIC 0x50434348  // "PCCH"

typedef struct GamePatternState
{
    bool Resolved = false;
    uint64_t Version = 0;
    uint32_t PCs[PC_MAX] = {};
    uint16_t Types[PC_MAX] = {};
} GamePatternState;

// Cache structure
typedef struct PatternCache
{
    uint32_t Magic;
    struct
    {
        uint64_t Version;
        uint32_t PCs[PC_MAX];
        uint16_t Types[PC_MAX];
    } Game[2];
} PatternCache;

extern GamePatternState gPatternState[2];

bool LoadCache(const char* Filename);
void SaveCache(const char* Filename);