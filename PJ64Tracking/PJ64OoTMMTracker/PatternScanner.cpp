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

__forceinline bool MatchPattern(uintptr_t addr, const PCSignature * Sig)
{
    for (size_t i = 0, j = 0; i < Sig->PatternSize; i += 4, j++)
    {   // Browse all pattern instructions

        uint32_t currWord = *(uint32_t*)(addr + i); 
        uint32_t targetWord = ByteSwap32(*(uint32_t*)(Sig->Pattern + i));   // We need to swap byte order as they are store in big endian order

        LOG("Curr PC = 0x80%06X, Target = 0x%08X, Curr = 0x%08X, i = %zu, j = %zu", addr + i - gameRAMBase, targetWord, currWord, i, j);

        if ((currWord & Sig->Mask [j]) != (targetWord & Sig->Mask[j]))
        {
            return false;
        }
    }
    return true;
}


__forceinline uintptr_t FindPatternInPayload(const PCSignature* Sig, size_t PayloadStart, size_t PayloadEnd)
{
    uintptr_t base = gameRAMBase + (PayloadStart & 0x00FFFFFF);
    uintptr_t res = PayloadStart;

    size_t size = PayloadEnd - PayloadStart;

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


void ResolveButterflyTransform()
{
    PCFastResolver* sigs = NULL;
    if (gGame == GAME_OOT)
    {
        sigs = OoTSignatures;
    }
    else if (gGame == GAME_MM)
    {
        sigs = MMSignatures;
    }
    else
    {
        return;
    }

    uintptr_t base = FindPatternInPayload(sigs[6].Signature, PC_RANGE_START, PAYLOAD_END);

    if (base != 0)
    {   // We have find the desired start address of the function

        uintptr_t PC = base + sigs[6].Signature->PCOffset;  // The specific instruction to track

        gPatternState[gGame].PCs[6] = PC;
        LOG("[OK] PC 0x%08X", PC);
    }
}


void ResetButterflyTransform()
{
    gPatternState[GAME_OOT].PCs[6] = 0;
    gPatternState[GAME_MM].PCs[6] = 0;
}


uintptr_t FindSubPattern(uintptr_t base, const PCSignature* sig, uint32_t limit)
{
    for (uint32_t off = 0; off <= limit; off += 4)
    {
        if (MatchPattern(gameRAMBase + ((base + off) & 0x00FFFFFF), sig))
        {
            return base + off;
        }
    }

    return 0;
}


void BuildPCsPatterns()
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


    for (size_t i = 0; i < count - 1; i++)
    {   // Browse all static payload patterns (le butterfly i==count-1 est resolu a la demande par ResolveButterflyTransform)

        uintptr_t base = FastPatternResolver(sigs[i]);
        if (base == 0)
        {   // The fast pattern failed to get the PC counter. Try the slow method

            LOG("[FAIL] Fast Pattern %zu\nTrying slow pattern.", i);

            base = FindPatternInPayload(sigs[i].Signature, i == count - 3 ? PC_RANGE_START : PAYLOAD_START, PAYLOAD_END);

            if (!base)
            {   // The function cannot be found

                LOG("[FAIL] Pattern %zu", i);

                fullResolved = false;
                continue;
            }
        }
        else
        {   // The fast resolver found an address, we need to check that the function is correct

            if (!MatchPattern(gameRAMBase + (base & 0x00FFFFFF), sigs[i].Signature))
            {   // The found address was wrong. Try the slow method

                base = FindPatternInPayload(sigs[i].Signature, i >= count - 3 ? PC_RANGE_START : PAYLOAD_START, PAYLOAD_END);

                if (!base)
                {   // The function cannot be found

                    LOG("[FAIL] Pattern %zu", i);

                    fullResolved = false;
                    continue;
                }
            }
        }

        // We have find the desired start address of the function

        if (i == 4)
        {
            if (gGame == GAME_OOT)
            {   // Resout dynamiquement le site du hook (independant du build : stable, dev, futurs)

                uintptr_t site = FindSubPattern(base, &Sig_hookInit_Site_OoT, 0x800);
                if (site)
                {
                    uint32_t off = (uint32_t)(site - base);
                    sigs[i].Signature->PCOffset = off;

                    // Le variant decoule de l'emplacement du hook -> selectionne le bon "Nothing" ID,
                    // sauf si le tracker a fourni la version via le spoiler (elle fait foi, cf ApplyHostVersion).
                    if (!gData || gData->HostROMVersion == HOST_VER_UNKNOWN)
                    {
                        isStable = (off == OOT_HOOK_INIT_STABLE_PCOFF);
                        gNothingID = isStable ? STABLE_NOTHING : DEV_NOTHING;
                    }
                }
                else
                {   // Repli : ancien comportement base sur le CRC (isStable deja positionne par CheckGameVersionASM)

                    sigs[i].Signature->PCOffset = isStable ? OOT_HOOK_INIT_STABLE_PCOFF : OOT_HOOK_INIT_DEV_PCOFF;
                }
            }

            else if (gGame == GAME_MM)
            {   // Resout dynamiquement le site du hook (independant du build : stable, dev, futurs)

                uintptr_t site = FindSubPattern(base, &Sig_hookInit_Site_MM, 0x800);
                if (site)
                {
                    uint32_t off = (uint32_t)(site - base);
                    sigs[i].Signature->PCOffset = off;

                    // Le variant decoule de l'emplacement du hook -> selectionne le bon "Nothing" ID,
                    // sauf si le tracker a fourni la version via le spoiler (elle fait foi, cf ApplyHostVersion).
                    if (!gData || gData->HostROMVersion == HOST_VER_UNKNOWN)
                    {
                        isStable = (off == MM_HOOK_INIT_STABLE_PCOFF);
                        gNothingID = isStable ? STABLE_NOTHING : DEV_NOTHING;
                    }
                }
                else
                {   // Repli : ancien comportement base sur le CRC (isStable deja positionne par CheckGameVersionASM)

                    sigs[i].Signature->PCOffset = isStable ? MM_HOOK_INIT_STABLE_PCOFF : MM_HOOK_INIT_DEV_PCOFF;
                }
            }
        }

        uintptr_t PC = base + sigs[i].Signature->PCOffset;  // The specific instruction to track

        gPatternState[gGame].PCs[i] = PC;
        LOG("[OK] PC 0x%08X", PC);
        if (i == 4)
        {   // We successfully found the PC for play transition done. We need to find the gLastScene offset

            FindLastSceneAddress();
        }
    }

    // Mark resolved
    gPatternState[gGame].Resolved = fullResolved;
    gActivePCs = gPatternState[gGame].PCs;
}


__forceinline bool IsJAL(uint32_t InstrucVal)
{
    return ((InstrucVal >> 26) == 0x03);
}


__forceinline uintptr_t ResolveJAL(uint32_t InstrucVal, uint32_t JALAddr)
{
    // Extract 26-bit target
    uint32_t target = InstrucVal & 0x03FFFFFF;

    // Shift left (word aligned)
    target <<= 2;

    // Merge upper PC bits
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

__forceinline void FindLastSceneAddress()
{
    if (gGame == GAME_OOT)
    {
        gOOTLastSceneAddr = (uint16_t) * (uint32_t*)((gPatternState[gGame].PCs[4] & 0x00FFFFFF) + gameRAMBase - 0x04);
        gOOTLastSceneAddr <<= 16;
        gOOTActiveGlobalOffset = (int16_t) * (uint32_t*)((gPatternState[gGame].PCs[4] & 0x00FFFFFF) + gameRAMBase) - 0x04;
        gOOTLastSceneAddr = gOOTLastSceneAddr + gOOTActiveGlobalOffset;
        gActiveSceneOffset = gOOTLastSceneAddr;
    }
    else if (gGame == GAME_MM)
    {
        gMMLastSceneAddr = (uint16_t) * (uint32_t*)((gPatternState[gGame].PCs[4] & 0x00FFFFFF) + gameRAMBase - 0x08);
        gMMLastSceneAddr <<= 16;
        gMMActiveGlobalOffset = (int16_t) * (uint32_t*)((gPatternState[gGame].PCs[4] & 0x00FFFFFF) + gameRAMBase - 0x04) - 0x04;
        gMMLastSceneAddr = gMMLastSceneAddr + gMMActiveGlobalOffset;
        gActiveSceneOffset = gMMLastSceneAddr;
    }
    else
    {
        return;
    }
}
