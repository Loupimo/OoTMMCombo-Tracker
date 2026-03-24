#include "pch.h"
#include "PatternScanner.h"
#include "Hooking.h"
#include "Patterns.h"
#include "Cache.h"


bool MatchPattern(uintptr_t addr, const uint8_t* pattern, const char* mask)
{
    size_t len = strlen(mask);
    for (size_t i = 0; i < len; i++)
    {
        if (mask[i] == 'x' && pattern[i] != *(uint8_t*)(addr + i))
        {
            return false;
        }
    }
    return true;
}


uintptr_t FindPatternInPayload(const uint8_t* Pattern, const char* Mask)
{
    uintptr_t base = gameRAMBase + (PAYLOAD_START & 0x00FFFFFF);
    uintptr_t res = PAYLOAD_START;

    size_t size = PAYLOAD_END - PAYLOAD_START;

    uint32_t firstWord = *(uint32_t*)Pattern;

    size_t len = strlen(Mask);

    for (size_t i = 0; i < size - len; i += 4, res += 4)
    {
        uint32_t word = *(uint32_t*)(base + i);

        if (word != firstWord)
        {   // The first word does not match we can continue without having to overloop

            //printf("first Word = 0x%08X, curr Word = 0x%08X\n", firstWord, word);
            continue;
        }

        if (MatchPattern(base + i, Pattern, Mask))
        {
            return res;
        }
    }

    return 0;
}


void BuildTypeMaskFromPatterns()
{
    size_t count = 0;
    int gameVal = 0;
    bool fullResolved = true;
    PCFastResolver* sigs = NULL;

    if (gGame == GameID::GAME_OOT)
    {
        sigs = OoTSignatures;
        count = OoTSignatureCount;
        gameVal = 0;
    }
    else
    {
        return;
    }
    printf("Resolved = %d\n", gPatternState[gameVal].Resolved);

    if (gPatternState[gameVal].Resolved || gGameVersion == 0)
    {   // The patterns are already resolved

        return;
    }

    memset(typemask, 0, sizeof(typemask));

    for (size_t i = 0; i < count; i++)
    {
        uintptr_t base = FastPatternResolver(sigs[i]);
        if (base == 0)
        {
            printf("[FAIL] Fast Pattern %d\nTrying slow pattern.\n", i);

            base = FindPatternInPayload(sigs[i].Signature->Pattern, sigs[i].Signature->Mask);

            if (!base)
            {
                printf("[FAIL] Pattern %d\n", i);
                fullResolved = false;
                continue;
            }
        }
        else
        {

        }

        uintptr_t PC = base + sigs[i].Signature->PCOffset;

        SetPCType(PC, sigs[i].Signature->Type);

        printf("[OK] PC 0x%08X\n", PC);

        gPatternState[gameVal].PCs[i] = PC;
        gPatternState[gameVal].Types[i] = sigs[i].Signature->Type;
    }

    // Mark resolved
    gPatternState[gameVal].Resolved = fullResolved;
    gPatternState[gameVal].Version = gGameVersion;
    SaveCache("ootmm_pccache.bin");
}



bool IsJAL(uint32_t InstrucVal)
{
    return ((InstrucVal >> 26) == 0x03);
}

uintptr_t ResolveJAL(uint32_t InstrucVal, uint32_t JALAddr)
{
    //--------------------------------
    // Extract 26-bit target
    //--------------------------------
    uint32_t target = InstrucVal & 0x03FFFFFF;

    //--------------------------------
    // Shift left (word aligned)
    //--------------------------------
    target <<= 2;

    //--------------------------------
    // Merge upper PC bits
    //--------------------------------
    uintptr_t resolved = (JALAddr & 0xF0000000) | target;

    return resolved;
}


uintptr_t FastPatternResolver(const PCFastResolver& Target)
{
    uintptr_t currAddr = gameRAMBase + (Target.BaseAddr & 0x00FFFFFF);  // The real RAM address
    uint32_t jal = 0;                       // The current JAL instruction
    uint32_t jalAddr = Target.BaseAddr;     // The JAL address using virtual addresses

    for (size_t i = 0; i < Target.NumOfJals; i++)
    {   // Browse all JAL to find the correct address

        currAddr += Target.JALOffsets[i];
        jalAddr += Target.JALOffsets[i];
        jal = *(uint32_t*)currAddr;
        
        printf("JAL Inst : 0x%08X, JAL Addr : 0x%08X\n", jal, jalAddr);

        if (IsJAL(jal))
        {   // The current instruction is a JAL, we can resolve it

            jalAddr = ResolveJAL(jal, jalAddr);
            currAddr = gameRAMBase + (jalAddr & 0x00FFFFFF);
        }
        else
        {   // The current instruction is not a JAL we can stop here

            return 0;
        }
    }

    return jalAddr;
}