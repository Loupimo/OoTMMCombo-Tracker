#include "pch.h"
#include "PatternScanner.h"
#include "Hooking.h"
#include "Patterns.h"

GamePatternState gPatternState[2]; // 0 = OoT, 1 = MM

__forceinline uint32_t ByteSwap32(uint32_t x)
{
    uint32_t y = (x >> 24) & 0xff;
    y |= ((x >> 16) & 0xff) << 8;
    y |= ((x >> 8) & 0xff) << 16;
    y |= (x & 0xff) << 24;

    return y;
}


__forceinline void SetPCType(uint32_t pc, uint8_t type)
{
    LOG("Setting PC : 0x%08X, Type = %d", pc, type);
    typemask[gGame][pc - PC_RANGE_START] = type;
}


__forceinline bool MatchPattern(uintptr_t addr, const PCSignature * Sig)
{
    for (size_t i = 0, j = 0; i < Sig->PatternSize; i += 4, j++)
    {   // Browse all pattern instructions

        uint32_t currWord = *(uint32_t*)(addr + i); 
        uint32_t targetWord = ByteSwap32(*(uint32_t*)(Sig->Pattern + i));   // We need to swap byte order as they are store in big endian order

        LOG("Target = 0x%08X, Curr = 0x%08X, i = %zu, j = %zu", targetWord, currWord, i, j);

        if ((currWord & Sig->Mask [j]) != (targetWord & Sig->Mask[j]))
        {
            return false;
        }
    }
    return true;
}


__forceinline uintptr_t FindPatternInPayload(const PCSignature* Sig)
{
    uintptr_t base = gameRAMBase + (PAYLOAD_START & 0x00FFFFFF);
    uintptr_t res = PAYLOAD_START;

    size_t size = PAYLOAD_END - PAYLOAD_START;

    uint32_t firstWord = ByteSwap32(*(uint32_t*)Sig->Pattern);   // We need to swap byte order as they are store in big endian order

    for (size_t i = 0; i < size - Sig->PatternSize; i += 4, res += 4)
    {
        uint32_t word = *(uint32_t*)(base + i);

        if (word != firstWord)
        {   // The first word does not match we can continue without having to overloop

            continue;
        }

        if (MatchPattern(base + i, Sig))
        {
            return res;
        }
    }

    return 0;
}


void BuildTypeMaskFromPatterns()
{
    size_t count = 0;
    bool fullResolved = true;
    PCFastResolver* sigs = NULL;

    if (gGame == GAME_OOT)
    {
        sigs = OoTSignatures;
        count = OoTSignatureCount;
    }
    else if (gGame == GAME_MM)
    {
        sigs = MMSignatures;
        count = MMSignatureCount;
    }
    else
    {
        return;
    }

    LOG("Resolved = %d", gPatternState[gGame].Resolved);

    if (gPatternState[gGame].Resolved)
    {   // The patterns are already resolved

        return;
    }

    memset(typemask[gGame], 0, PC_RANGE_SIZE);

    for (size_t i = 0; i < count; i++)
    {
        uintptr_t base = FastPatternResolver(sigs[i]);
        if (base == 0)
        {
            LOG("[FAIL] Fast Pattern %zu\nTrying slow pattern.", i);

            base = FindPatternInPayload(sigs[i].Signature);

            if (!base)
            {
                LOG("[FAIL] Pattern %zu", i);

                fullResolved = false;
                continue;
            }
        }
        else
        {   // The fast resolver found an address, we need to check that the function is correct

            if (!MatchPattern(gameRAMBase + (base & 0x00FFFFFF), sigs[i].Signature))
            {   // The found address was wrong

                base = FindPatternInPayload(sigs[i].Signature);

                if (!base)
                {
                    LOG("[FAIL] Pattern %zu", i);

                    fullResolved = false;
                    continue;
                }
            }
        }

        uintptr_t PC = base + sigs[i].Signature->PCOffset;

        SetPCType(PC, sigs[i].Signature->Type);
        gPatternState[gGame].PCs[i] = PC;
        gActivePCs[i] = PC;
        LOG("[OK] PC 0x%08X", PC);
    }

    // Mark resolved
    gPatternState[gGame].Resolved = fullResolved;
}



__forceinline bool IsJAL(uint32_t InstrucVal)
{
    return ((InstrucVal >> 26) == 0x03);
}

__forceinline uintptr_t ResolveJAL(uint32_t InstrucVal, uint32_t JALAddr)
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


__forceinline uintptr_t FastPatternResolver(const PCFastResolver& Target)
{
    uintptr_t currAddr = gameRAMBase + (Target.BaseAddr & 0x00FFFFFF);  // The real RAM address
    uint32_t jal = 0;                       // The current JAL instruction
    uint32_t jalAddr = Target.BaseAddr;     // The JAL address using virtual addresses

    for (size_t i = 0; i < Target.NumOfJals; i++)
    {   // Browse all JAL to find the correct address

        currAddr += Target.JALOffsets[i];
        jalAddr += Target.JALOffsets[i];
        jal = *(uint32_t*)currAddr;
        
        LOG("JAL Inst : 0x%08X, JAL Addr : 0x%08X", jal, jalAddr);

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