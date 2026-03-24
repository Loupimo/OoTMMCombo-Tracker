#include "PatternScanner.h"
#include "Hooking.h"

#pragma region comboAddItemRawEx

uint8_t Pattern_comboAddItemRawEx[] =
{
    0x27,0xBD,0xFF,0xC8,            // ADDIU    SP, SP, -0x38           <-- comboAddItemRawEx_Start, Hook here
    0xAF,0xB3,0x00,0x30,            // SW       S3, 0x0030 (SP)
    0x00,0x80,0x98,0x25,            // OR       S3, A0, R0
    0x27,0xA4,0x00,0x18,            // ADDIU    A0, SP, 0x0018
    0xAF,0xB2,0x00,0x2C,            // SW       S2, 0x002C (SP)
    0xAF,0xB0,0x00,0x24,            // SW       S0, 0x0024 (SP)
    0xAF,0xBF,0x00,0x34             // SW       RA, 0x0034 (SP)
};

const char* Mask_comboAddItemRawEx =
"xxxx"                              // ADDIU    SP, SP, -0x38           <-- comboAddItemRawEx_Start, Hook here
"xxxx"                              // SW       S3, 0x0030 (SP)
"xxxx"                              // OR       S3, A0, R0
"xxxx"                              // ADDIU    A0, SP, 0x0018
"xxxx"                              // SW       S2, 0x002C (SP)
"xxxx"                              // SW       S0, 0x0024 (SP)
"xxxx";                             // SW       RA, 0x0034 (SP)

PCSignature Sig_comboAddItemRawEx = { TYPE_COMBO, Pattern_comboAddItemRawEx, Mask_comboAddItemRawEx, 0x00 };

#pragma endregion   // comboAddItemRawEx

#pragma region EnItem00_DropCustom

uint8_t Pattern_EnItem00_DropCustom[] =
{
    0x27,0xBD,0xFF,0xD0,            // ADDIU    SP, SP, -0x30           <-- EnItem00_DropCustom_Start
    0xAF,0xB1,0x00,0x20,            // SW       S1, 0x0020 (SP)
    0x8C,0x91,0x1C,0x74,            // LW       S1, 0x1C74 (A0)
    0xAF,0xB3,0x00,0x28,            // SW       S3, 0x0028 (SP)
    0xAF,0xB2,0x00,0x24,            // SW       S2, 0x0024 (SP)
    0xAF,0xB0,0x00,0x1C,            // SW       S0, 0x001C (SP)
    0xAF,0xBF,0x00,0x2C,            // SW       RA, 0x002C (SP)
    0x00,0x80,0x90,0x25,            // OR       S2, A0, R0
    0xAF,0xA5,0x00,0x34,            // SW       A1, 0x0034 (SP)
    0x00,0xC0,0x80,0x25,            // OR       S0, A2, R0
    0x24,0x13,0x00,0x15,            // ADDIU    S3, R0, 0x0015
    0x56,0x20,0x00,0x0D,            // BNEZL    S1, 0x80400D70
    0x96,0x22,0x00,0x00,            // LHU      V0, 0x0000 (S1)
    0x00,0x00,0x30,0x25,            // OR       A2, R0, R0
    0x02,0x00,0x28,0x25,            // OR       A1, S0, R0
    0x0C,0x00,0x00,0x00,            // JAL      comboItemOverride
    0x27,0xA4,0x00,0x10,            // ADDIU    A0, SP, 0x0010
    0x87,0xA3,0x00,0x14,            // LH       V1, 0x0014 (SP)
    0x24,0x02,0x03,0x3C,            // ADDIU    V0, R0, 0x033C
    0x14,0x62,0x00,0x16,            // BNE      V1, V0, 0x80400DB4
    0x02,0x00,0x28,0x25,            // OR       A1, S0, R0
    0x0C,0x00,0x00,0x00             // JAL      comboXflagsSet          <-- Hook here
};

const char* Mask_EnItem00_DropCustom =
"xxxx"                              // ADDIU    SP, SP, -0x30           <-- EnItem00_DropCustom_Start     
"xxxx"                              // SW       S1, 0x0020 (SP)
"xxxx"                              // LW       S1, 0x1C74 (A0)
"xxxx"                              // SW       S3, 0x0028 (SP)
"xxxx"                              // SW       S2, 0x0024 (SP)
"xxxx"                              // SW       S0, 0x001C (SP)
"xxxx"                              // SW       RA, 0x002C (SP)
"xxxx"                              // OR       S2, A0, R0
"xxxx"                              // SW       A1, 0x0034 (SP)
"xxxx"                              // OR       S0, A2, R0
"xxxx"                              // ADDIU    S3, R0, 0x0015
"xxxx"                              // BNEZL    S1, 0x80400D70
"xxxx"                              // LHU      V0, 0x0000 (S1)
"xxxx"                              // OR       A2, R0, R0
"xxxx"                              // OR       A1, S0, R0
"x???"                              // JAL      comboItemOverride
"xxxx"                              // ADDIU    A0, SP, 0x0010
"xxxx"                              // LH       V1, 0x0014 (SP)
"xxxx"                              // ADDIU    V0, R0, 0x033C
"xxxx"                              // BNE      V1, V0, 0x80400DB4
"xxxx"                              // OR       A1, S0, R0
"x???";                             // JAL      comboXflagsSet          <-- Hook here

PCSignature Sig_EnItem00_DropCustom = { TYPE_XFLAG, Pattern_EnItem00_DropCustom, Mask_EnItem00_DropCustom, 0x54 };

#pragma endregion // EnItem00_DropCustom

#pragma region comboItemPrecond

uint8_t Pattern_comboItemPrecond[] =
{
    0x27,0xBD,0xFF,0xE0,            // ADDIU    SP, SP, -0x20           <-- comboItemPrecond_Start
    0xAF,0xBF,0x00,0x1C,            // SW       RA, 0x001C (SP)
    0xAF,0xB0,0x00,0x18,            // SW       S0, 0x0018 (SP)
    0xAF,0xA4,0x00,0x20,            // SW       A0, 0x0020 (SP)
    0x0C,0x00,0x00,0x00,            // JAL      comboSyncItems
    0x00,0xA0,0x80,0x25,            // OR       S0, A1, R0
    0x8F,0xA5,0x00,0x20,            // LW       A1, 0x0020 (SP)
    0x0C,0x00,0x00,0x00,            // JAL      comboItemOverride
    0x27,0xA4,0x00,0x10,            // ADDIU    A0, SP, 0x0010
    0x87,0xA3,0x00,0x14,            // LH       V1, 0x0014 (SP)
    0x24,0x02,0x03,0x3C,            // ADDIU    V0, R0, 0x0002
    0x14,0x62,0x00,0x05,            // BNE      V1, V0, 0x8040C178
    0x24,0x02,0x00,0x02,            // ADDIU    V0, R0, 0x0002
    0x8F,0xBF,0x00,0x1C             // LW       S0, 0x0018 (SP)         <-- Hook here
};

const char* Mask_comboItemPrecond =
"xxxx"                              // ADDIU    SP, SP, -0x20           <-- comboItemPrecond_Start
"xxxx"                              // SW       RA, 0x001C (SP)
"xxxx"                              // SW       S0, 0x0018 (SP)
"xxxx"                              // SW       A0, 0x0020 (SP)
"x???"                              // JAL      comboSyncItems
"xxxx"                              // OR       S0, A1, R0
"xxxx"                              // LW       A1, 0x0020 (SP)
"x???"                              // JAL      comboItemOverride
"xxxx"                              // ADDIU    A0, SP, 0x0010
"xxxx"                              // LH       V1, 0x0014 (SP)
"xxxx"                              // ADDIU    V0, R0, 0x0002
"xxxx"                              // BNE      V1, V0, 0x8040C178
"xxxx"                              // ADDIU    V0, R0, 0x0002
"xxxx";                             // LW       S0, 0x0018 (SP)         <-- Hook here

PCSignature Sig_comboItemPrecond = { TYPE_SHOP, Pattern_comboItemPrecond, Mask_comboItemPrecond, 0x34 };

#pragma endregion   // comboItemPrecond

#pragma region Actor_RunUpdate

uint8_t Pattern_Actor_RunUpdate[] =
{
    0x27,0xBD,0xFF,0xC0,            // ADDIU    SP, SP, -0x40           <-- Actor_RunUpdate_Start, Hook here
    0xAF,0xB2,0x00,0x1C,            // SW       S2, 0x001C (SP)
    0xAF,0xB1,0x00,0x18,            // SW       S1, 0x0018 (SP)
    0xAF,0xB0,0x00,0x14,            // SW       S0, 0x0014 (SP)
    0xAF,0xBF,0x00,0x24,            // SW       RA, 0x0024 (SP)
    0xAF,0xB3,0x00,0x20             // SW       S3, 0x0020 (SP)
};

const char* Mask_Actor_RunUpdate =
"xxxx"                              // ADDIU    SP, SP, -0x40           <-- Actor_RunUpdate_Start, Hook here
"xxxx"                              // SW       S2, 0x001C (SP)
"xxxx"                              // SW       S1, 0x0018 (SP)
"xxxx"                              // SW       S0, 0x0014 (SP)
"xxxx"                              // SW       RA, 0x0024 (SP)
"xxxx";                             // SW       S3, 0x0020 (SP)

PCSignature Sig_Actor_RunUpdate = { TYPE_BUTTERFLY, Pattern_Actor_RunUpdate, Mask_Actor_RunUpdate, 0x00 };

#pragma endregion   // comboAddItemRawEx

PCFastResolver OoTSignatures[] =
{
    { 0x803A4AD0, 2, { 0, 0x84 }, &Sig_comboAddItemRawEx },
    { 0x80000000, 1, { 0 }, &Sig_EnItem00_DropCustom },
    { 0x80000000, 1, { 0 }, &Sig_comboItemPrecond },
    { 0x80000000, 1, { 0 }, &Sig_Actor_RunUpdate }
};
size_t OoTSignatureCount = sizeof(OoTSignatures) / sizeof(OoTSignatures[0]);