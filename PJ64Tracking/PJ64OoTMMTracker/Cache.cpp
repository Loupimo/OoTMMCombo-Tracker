#include "pch.h"
#include "Cache.h"
#include "Hooking.h"

GamePatternState gPatternState[2]; // 0 = OoT, 1 = MM
PatternCache gCache;

bool LoadCache(const char* filename)
{
    gPatternState[0] = { 0 };
    gPatternState[1] = { 0 };

    FILE* f = NULL;
    fopen_s(&f, filename, "rb");
    if (!f)
    {
        return false;
    }
    
    fread(&gCache, sizeof(gCache), 1, f);
    fclose(f);
    if (gCache.Magic != CACHE_MAGIC)
    {
        return false;
    }

    // Fill typemask from cache
    for (int g = 0; g < 2; g++)
    {
        if (gPatternState[g].Resolved)
        {
            continue; // skip if already resolved
        }
        
        gPatternState[g].Version = gCache.Game[g].Version;
        
        for (int i = 0; i < PC_MAX; i++)
        {
            if (gCache.Game[g].PCs[i])
            {
                SetPCType(gCache.Game[g].PCs[i], gCache.Game[g].Types[i]); // type example, can map individually
            }
            gPatternState[g].PCs[i] = gCache.Game[g].PCs[i];
            gPatternState[g].Types[i] = gCache.Game[g].Types[i];
        }
        gPatternState[g].Resolved = true;
    }
    return true;
}


void SaveCache(const char* filename)
{
    gCache.Magic = CACHE_MAGIC;
    for (int g = 0; g < 2; g++)
    {
        gCache.Game[g].Version = gPatternState[g].Version;
        memcpy(gCache.Game[g].PCs, gPatternState[g].PCs, sizeof(gPatternState[g].PCs));
        memcpy(gCache.Game[g].Types, gPatternState[g].Types, sizeof(gPatternState[g].Types));
    }

    FILE* f = NULL;
    fopen_s(&f, filename, "wb");
    if (!f)
    {
        return;
    }
    
    fwrite(&gCache, sizeof(gCache), 1, f);
    fclose(f);
}