#pragma once

#include "pch.h"

#define PAYLOAD_START 0x80400000
#define PAYLOAD_END   0x80800000
#define MAX_JAL 10


typedef struct PCSignature
{
    uint8_t Type;       // The friendly function name

    const uint8_t* Pattern; // The function pattern to find
    const char* Mask;       // 

    int PCOffset;           // Offset to target instruction
} PCSignature;

typedef struct PCFastResolver
{
    uintptr_t BaseAddr;             // The fixed base address
    uint32_t NumOfJals;             // The number of JAL offsets in the array
    uint32_t JALOffsets[MAX_JAL];   // The offset to add to find the next desired JAL instruction
    PCSignature* Signature;           // The associated pattern to find
} PCFastResolver;

bool MatchPattern(uintptr_t addr, const uint8_t* pattern, const char* mask);
uintptr_t FindPatternInPayload(const uint8_t* Pattern, const char* Mask);
bool IsJAL(uint32_t InstrucVal);
uintptr_t ResolveJAL(uint32_t InstrucVal, uint32_t JALAddr);
uintptr_t FastPatternResolver(const PCFastResolver& Target);
void BuildTypeMaskFromPatterns();