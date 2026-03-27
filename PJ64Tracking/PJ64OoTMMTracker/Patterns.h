#include "PatternScanner.h"
#include "Hooking.h"

#pragma region OoT

#pragma region Actor_RunUpdate

uint8_t Pattern_Actor_RunUpdate_OoT[] =
{
    0x27,0xBD,0xFF,0xC0,            // ADDIU    SP, SP, -0x40           <-- Actor_RunUpdate_Start, Hook here
    0xAF,0xB2,0x00,0x1C,            // SW       S2, 0x001C (SP)
    0xAF,0xB1,0x00,0x18,            // SW       S1, 0x0018 (SP)
    0xAF,0xB0,0x00,0x14,            // SW       S0, 0x0014 (SP)
    0xAF,0xBF,0x00,0x24,            // SW       RA, 0x0024 (SP)
    0xAF,0xB3,0x00,0x20             // SW       S3, 0x0020 (SP)
};

uint32_t Mask_Actor_RunUpdate_OoT[] =
{
    0xFFFFFFFF,                     // ADDIU    SP, SP, -0x40           <-- Actor_RunUpdate_Start, Hook here
    0xFFFFFFFF,                     // SW       S2, 0x001C (SP)
    0xFFFFFFFF,                     // SW       S1, 0x0018 (SP)
    0xFFFFFFFF,                     // SW       S0, 0x0014 (SP)
    0xFFFFFFFF,                     // SW       RA, 0x0024 (SP)
    0xFFFFFFFF                      // SW       S3, 0x0020 (SP)
};

PCSignature Sig_Actor_RunUpdate_OoT = { TYPE_BUTTERFLY, 24, Pattern_Actor_RunUpdate_OoT, Mask_Actor_RunUpdate_OoT, 0x00 };

#pragma endregion   // Actor_RunUpdate

#pragma region comboAddItemRawEx

uint8_t Pattern_comboAddItemRawEx_OoT[] =
{
    0x27,0xBD,0xFF,0xC8,            // ADDIU    SP, SP, -0x38           <-- comboAddItemRawEx_Start, Hook here
    0xAF,0xB3,0x00,0x30,            // SW       S3, 0x0030 (SP)
    0x00,0x80,0x98,0x25,            // OR       S3, A0, R0
    0x27,0xA4,0x00,0x18,            // ADDIU    A0, SP, 0x0018
    0xAF,0xB2,0x00,0x2C,            // SW       S2, 0x002C (SP)
    0xAF,0xB0,0x00,0x24,            // SW       S0, 0x0024 (SP)
    0xAF,0xBF,0x00,0x34             // SW       RA, 0x0034 (SP)
};

uint32_t Mask_comboAddItemRawEx_OoT[] =
{
    0xFFFFFFFF,                     // ADDIU    SP, SP, -0x38           <-- comboAddItemRawEx_Start, Hook here
    0xFFFFFFFF,                     // SW       S3, 0x0030 (SP)
    0xFFFFFFFF,                     // OR       S3, A0, R0
    0xFFFFFFFF,                     // ADDIU    A0, SP, 0x0018
    0xFFFFFFFF,                     // SW       S2, 0x002C (SP)
    0xFFFFFFFF,                     // SW       S0, 0x0024 (SP)
    0xFFFFFFFF                      // SW       RA, 0x0034 (SP)
};                             

PCSignature Sig_comboAddItemRawEx_OoT = { TYPE_COMBO, 28, Pattern_comboAddItemRawEx_OoT, Mask_comboAddItemRawEx_OoT, 0x40 };

#pragma endregion   // comboAddItemRawEx

#pragma region EnItem00_DropCustom

uint8_t Pattern_EnItem00_DropCustom_OoT[] =
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

uint32_t Mask_EnItem00_DropCustom_OoT[] =
{
    0xFFFFFFFF,                     // ADDIU    SP, SP, -0x30           <-- EnItem00_DropCustom_Start     
    0xFFFFFFFF,                     // SW       S1, 0x0020 (SP)
    0xFFFFFFFF,                     // LW       S1, 0x1C74 (A0)
    0xFFFFFFFF,                     // SW       S3, 0x0028 (SP)
    0xFFFFFFFF,                     // SW       S2, 0x0024 (SP)
    0xFFFFFFFF,                     // SW       S0, 0x001C (SP)
    0xFFFFFFFF,                     // SW       RA, 0x002C (SP)
    0xFFFFFFFF,                     // OR       S2, A0, R0
    0xFFFFFFFF,                     // SW       A1, 0x0034 (SP)
    0xFFFFFFFF,                     // OR       S0, A2, R0
    0xFFFFFFFF,                     // ADDIU    S3, R0, 0x0015
    0xFFFFFFFF,                     // BNEZL    S1, 0x80400D70
    0xFFFFFFFF,                     // LHU      V0, 0x0000 (S1)
    0xFFFFFFFF,                     // OR       A2, R0, R0
    0xFFFFFFFF,                     // OR       A1, S0, R0
    0xFF000000,                     // JAL      comboItemOverride
    0xFFFFFFFF,                     // ADDIU    A0, SP, 0x0010
    0xFFFFFFFF,                     // LH       V1, 0x0014 (SP)
    0xFFFFFFFF,                     // ADDIU    V0, R0, 0x033C
    0xFFFFFFFF,                     // BNE      V1, V0, 0x80400DB4
    0xFFFFFFFF,                     // OR       A1, S0, R0
    0xFF000000                      // JAL      comboXflagsSet          <-- Hook here
};
PCSignature Sig_EnItem00_DropCustom_OoT = { TYPE_XFLAG, 88, Pattern_EnItem00_DropCustom_OoT, Mask_EnItem00_DropCustom_OoT, 0x54 };

#pragma endregion // EnItem00_DropCustom

#pragma region comboItemPrecond

uint8_t Pattern_comboItemPrecond_OoT[] =
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

uint32_t Mask_comboItemPrecond_OoT[] =
{
    0xFFFFFFFF,                     // ADDIU    SP, SP, -0x20           <-- comboItemPrecond_Start
    0xFFFFFFFF,                     // SW       RA, 0x001C (SP)
    0xFFFFFFFF,                     // SW       S0, 0x0018 (SP)
    0xFFFFFFFF,                     // SW       A0, 0x0020 (SP)
    0xFF000000,                     // JAL      comboSyncItems
    0xFFFFFFFF,                     // OR       S0, A1, R0
    0xFFFFFFFF,                     // LW       A1, 0x0020 (SP)
    0xFF000000,                     // JAL      comboItemOverride
    0xFFFFFFFF,                     // ADDIU    A0, SP, 0x0010
    0xFFFFFFFF,                     // LH       V1, 0x0014 (SP)
    0xFFFFFFFF,                     // ADDIU    V0, R0, 0x0002
    0xFFFFFFFF,                     // BNE      V1, V0, 0x8040C178
    0xFFFFFFFF,                     // ADDIU    V0, R0, 0x0002
    0xFFFFFFFF                      // LW       S0, 0x0018 (SP)         <-- Hook here
};

PCSignature Sig_comboItemPrecond_OoT = { TYPE_SHOP, 56, Pattern_comboItemPrecond_OoT, Mask_comboItemPrecond_OoT, 0x34 };

#pragma endregion   // comboItemPrecond

PCFastResolver OoTSignatures[] =
{
    { 0x80000000, 1, { 0 }, &Sig_Actor_RunUpdate_OoT },
    { 0x803A4AD0, 2, { 0, 0x84 }, &Sig_comboAddItemRawEx_OoT },
    { 0x80000000, 1, { 0 }, &Sig_EnItem00_DropCustom_OoT },
    { 0x80000000, 1, { 0 }, &Sig_comboItemPrecond_OoT }
};
size_t OoTSignatureCount = sizeof(OoTSignatures) / sizeof(OoTSignatures[0]);

#pragma endregion

#pragma region MM

#pragma region Actor_RunUpdate

uint8_t Pattern_Actor_RunUpdate_MM[] =
{
    0x27,0xBD,0xFF,0xE0,            // ADDIU    SP, SP, -0x20           <-- Actor_RunUpdate_Start, Hook here
    0xAF,0xB1,0x00,0x18,            // SW       S1, 0x0018 (SP)
    0xAF,0xB0,0x00,0x14,            // SW       S0, 0x0014 (SP)
    0xAF,0xBF,0x00,0x1C,            // SW       RA, 0x001C (SP)
    0x00,0x80,0x80,0x25             // OR       S0, A0, R0
};

uint32_t Mask_Actor_RunUpdate_MM[] =
{
    0xFFFFFFFF,                     // ADDIU    SP, SP, -0x20           <-- Actor_RunUpdate_Start, Hook here
    0xFFFFFFFF,                     // SW       S1, 0x0018 (SP)
    0xFFFFFFFF,                     // SW       S0, 0x0014 (SP)
    0xFFFFFFFF,                     // SW       RA, 0x001C (SP)
    0xFFFFFFFF                      // OR       S0, A0, R0
};

PCSignature Sig_Actor_RunUpdate_MM = { TYPE_BUTTERFLY, 20, Pattern_Actor_RunUpdate_MM, Mask_Actor_RunUpdate_MM, 0x00 };

#pragma endregion   // Actor_RunUpdate

#pragma region comboAddItemRawEx

uint8_t Pattern_comboAddItemRawEx_MM[] =
{
    0x27,0xBD,0xFF,0xC8,            // ADDIU    SP, SP, -0x38           <-- comboAddItemRawEx_Start, Hook here
    0xAF,0xB3,0x00,0x30,            // SW       S3, 0x0030 (SP)
    0x00,0x80,0x98,0x25,            // OR       S3, A0, R0
    0x27,0xA4,0x00,0x18,            // ADDIU    A0, SP, 0x0018
    0xAF,0xB2,0x00,0x2C,            // SW       S2, 0x002C (SP)
    0xAF,0xB0,0x00,0x24,            // SW       S0, 0x0024 (SP)
    0xAF,0xBF,0x00,0x34             // SW       RA, 0x0034 (SP)
};

uint32_t Mask_comboAddItemRawEx_MM[] =
{
    0xFFFFFFFF,                     // ADDIU    SP, SP, -0x38           <-- comboAddItemRawEx_Start, Hook here
    0xFFFFFFFF,                     // SW       S3, 0x0030 (SP)
    0xFFFFFFFF,                     // OR       S3, A0, R0
    0xFFFFFFFF,                     // ADDIU    A0, SP, 0x0018
    0xFFFFFFFF,                     // SW       S2, 0x002C (SP)
    0xFFFFFFFF,                     // SW       S0, 0x0024 (SP)
    0xFFFFFFFF                      // SW       RA, 0x0034 (SP)
};

PCSignature Sig_comboAddItemRawEx_MM = { TYPE_COMBO, 28, Pattern_comboAddItemRawEx_MM, Mask_comboAddItemRawEx_MM, 0x40 };

#pragma endregion   // comboAddItemRawEx

#pragma region EnItem00_DropCustom

uint8_t Pattern_EnItem00_DropCustom_MM[] =
{
    0x27,0xBD,0xFF,0xD0,            // ADDIU    SP, SP, -0x30           <-- EnItem00_DropCustom_Start
    0xAF,0xB1,0x00,0x20,            // SW       S1, 0x0020 (SP)
    0x8C,0x91,0x1D,0x14,            // LW       S1, 0x1D14 (A0)
    0xAF,0xB3,0x00,0x28,            // SW       S3, 0x0028 (SP)
    0xAF,0xB2,0x00,0x24,            // SW       S2, 0x0024 (SP)
    0xAF,0xB0,0x00,0x1C,            // SW       S0, 0x001C (SP)
    0xAF,0xBF,0x00,0x2C,            // SW       RA, 0x002C (SP)
    0x00,0x80,0x90,0x25,            // OR       S2, A0, R0
    0xAF,0xA5,0x00,0x34,            // SW       A1, 0x0034 (SP)
    0x00,0xC0,0x80,0x25,            // OR       S0, A2, R0
    0x24,0x13,0x00,0x0E,            // ADDIU    S3, R0, 0x000E
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

uint32_t Mask_EnItem00_DropCustom_MM[] =
{
    0xFFFFFFFF,                     // ADDIU    SP, SP, -0x30           <-- EnItem00_DropCustom_Start     
    0xFFFFFFFF,                     // SW       S1, 0x0020 (SP)
    0xFFFFFFFF,                     // LW       S1, 0x1D14 (A0)
    0xFFFFFFFF,                     // SW       S3, 0x0028 (SP)
    0xFFFFFFFF,                     // SW       S2, 0x0024 (SP)
    0xFFFFFFFF,                     // SW       S0, 0x001C (SP)
    0xFFFFFFFF,                     // SW       RA, 0x002C (SP)
    0xFFFFFFFF,                     // OR       S2, A0, R0
    0xFFFFFFFF,                     // SW       A1, 0x0034 (SP)
    0xFFFFFFFF,                     // OR       S0, A2, R0
    0xFFFFFFFF,                     // ADDIU    S3, R0, 0x000E
    0xFFFFFFFF,                     // BNEZL    S1, 0x80400D70
    0xFFFFFFFF,                     // LHU      V0, 0x0000 (S1)
    0xFFFFFFFF,                     // OR       A2, R0, R0
    0xFFFFFFFF,                     // OR       A1, S0, R0
    0xFF000000,                     // JAL      comboItemOverride
    0xFFFFFFFF,                     // ADDIU    A0, SP, 0x0010
    0xFFFFFFFF,                     // LH       V1, 0x0014 (SP)
    0xFFFFFFFF,                     // ADDIU    V0, R0, 0x033C
    0xFFFFFFFF,                     // BNE      V1, V0, 0x80400DB4
    0xFFFFFFFF,                     // OR       A1, S0, R0
    0xFF000000                      // JAL      comboXflagsSet          <-- Hook here
};
PCSignature Sig_EnItem00_DropCustom_MM = { TYPE_XFLAG, 88, Pattern_EnItem00_DropCustom_MM, Mask_EnItem00_DropCustom_MM, 0x54 };

#pragma endregion // EnItem00_DropCustom

#pragma region comboItemPrecond

uint8_t Pattern_comboItemPrecond_MM[] =
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

uint32_t Mask_comboItemPrecond_MM[] =
{
    0xFFFFFFFF,                     // ADDIU    SP, SP, -0x20           <-- comboItemPrecond_Start
    0xFFFFFFFF,                     // SW       RA, 0x001C (SP)
    0xFFFFFFFF,                     // SW       S0, 0x0018 (SP)
    0xFFFFFFFF,                     // SW       A0, 0x0020 (SP)
    0xFF000000,                     // JAL      comboSyncItems
    0xFFFFFFFF,                     // OR       S0, A1, R0
    0xFFFFFFFF,                     // LW       A1, 0x0020 (SP)
    0xFF000000,                     // JAL      comboItemOverride
    0xFFFFFFFF,                     // ADDIU    A0, SP, 0x0010
    0xFFFFFFFF,                     // LH       V1, 0x0014 (SP)
    0xFFFFFFFF,                     // ADDIU    V0, R0, 0x0002
    0xFFFFFFFF,                     // BNE      V1, V0, 0x8040C178
    0xFFFFFFFF,                     // ADDIU    V0, R0, 0x0002
    0xFFFFFFFF                      // LW       S0, 0x0018 (SP)         <-- Hook here
};

PCSignature Sig_comboItemPrecond_MM = { TYPE_SHOP, 56, Pattern_comboItemPrecond_MM, Mask_comboItemPrecond_MM, 0x34 };

#pragma endregion   // comboItemPrecond

PCFastResolver MMSignatures[] =
{
    { 0x80000000, 1, { 0 }, &Sig_Actor_RunUpdate_MM },
    { 0x806D57E4, 2, { 0, 0x84 }, &Sig_comboAddItemRawEx_MM },
    { 0x80000000, 1, { 0 }, &Sig_EnItem00_DropCustom_MM },
    { 0x80000000, 1, { 0 }, &Sig_comboItemPrecond_MM }
};
size_t MMSignatureCount = sizeof(MMSignatures) / sizeof(MMSignatures[0]);

#pragma endregion