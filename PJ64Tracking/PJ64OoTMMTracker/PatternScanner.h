#pragma once

#include "pch.h"

#define PAYLOAD_START 0x80400000
#define PAYLOAD_END   0x80800000
#define MAX_JAL 10

typedef struct GamePatternState
{
    bool Resolved = false;
    uint32_t PCs[4] = { 0 };    // ID 0 = comboAddItemRawEx, ID 1 = EnItem00_DropCustom, ID 2 = comboItemPrecond, ID 3 = Actor_RunUpdate
} GamePatternState;

extern GamePatternState gPatternState[2];

typedef struct PCSignature
{
    uint8_t Type;           // The PC type associated to this pattern 
    size_t PatternSize;     // The number of bytes in the pattern
    const uint8_t* Pattern; // The function pattern to find
    const uint32_t* Mask;   // The mask to apply to each pattern instruction 

    int PCOffset;           // Offset to target instruction
} PCSignature;

typedef struct PCFastResolver
{
    uintptr_t BaseAddr;             // The fixed base address
    uint32_t NumOfJals;             // The number of JAL offsets in the array
    uint32_t JALOffsets[MAX_JAL];   // The offset to add to find the next desired JAL instruction
    PCSignature* Signature;           // The associated pattern to find
} PCFastResolver;


void SetPCType(uint32_t pc, uint8_t type);
bool MatchPattern(uintptr_t addr, const PCSignature* Sig);
uintptr_t FindPatternInPayload(const PCSignature* Sig);
bool IsJAL(uint32_t InstrucVal);
uintptr_t ResolveJAL(uint32_t InstrucVal, uint32_t JALAddr);
uintptr_t FastPatternResolver(const PCFastResolver& Target);
void BuildTypeMaskFromPatterns();