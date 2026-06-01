#include "PatternScanner.h"
#include "Hooking.h"

#pragma region OoT

#pragma region Actor_Spawn

uint8_t Pattern_Actor_Spawn_OoT[] =
{
    0x8F,0xB0,0x00,0x20             // LW       S0, 0x0020 (SP)
};

uint32_t Mask_Actor_Spawn_OoT[] =
{
    0xFFFFFFFF,                     // LW       S0, 0x0020 (SP)         <-- Actor_Spawn_End, Hook here
};

PCSignature Sig_Actor_Spawn_OoT = { 4, Pattern_Actor_Spawn_OoT, Mask_Actor_Spawn_OoT, 0x00 };

#pragma endregion   // Actor_Spawn

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

PCSignature Sig_comboAddItemRawEx_OoT = { 28, Pattern_comboAddItemRawEx_OoT, Mask_comboAddItemRawEx_OoT, 0x40 };

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
    0xFFFF0000,                     // ADDIU    V0, R0, 0x033C
    0xFFFFFFFF,                     // BNE      V1, V0, 0x80400DB4
    0xFFFFFFFF,                     // OR       A1, S0, R0
    0xFF000000                      // JAL      comboXflagsSet          <-- Hook here
};
PCSignature Sig_EnItem00_DropCustom_OoT = { 88, Pattern_EnItem00_DropCustom_OoT, Mask_EnItem00_DropCustom_OoT, 0x54 };

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
    0xFFFF0000,                     // ADDIU    V0, R0, 0x0002
    0xFFFFFFFF,                     // BNE      V1, V0, 0x8040C178
    0xFFFFFFFF,                     // ADDIU    V0, R0, 0x0002
    0xFFFFFFFF                      // LW       S0, 0x0018 (SP)         <-- Hook here
};

PCSignature Sig_comboItemPrecond_OoT = { 56, Pattern_comboItemPrecond_OoT, Mask_comboItemPrecond_OoT, 0x34 };

#pragma endregion   // comboItemPrecond

#pragma region hookPlay_Init

uint8_t Pattern_hookPlay_Init_OoT[] =
{
    0x27,0xBD,0xFF,0xA8,            // ADDIU    SP, SP, -0x58           <-- hookPlay_Init_Start
    0x3C,0x02,0x80,0x45,            // LUI      V0, 0x8045
    0xAF,0xB6,0x00,0x48,            // SW       S6, 0x0048 (SP)
    0x3C,0x16,0x80,0x44,            // LUI      S6, 0x8044
    0xAF,0xBF,0x00,0x54,            // SW       RA, 0x0054 (SP)
    0xAF,0xB3,0x00,0x3C,            // SW       S3, 0x003C (SP)
    0xAF,0xB2,0x00,0x38,            // SW       S2, 0x0038 (SP)
    0xAF,0xB1,0x00,0x34,            // SW       S1, 0x0034 (SP)
    0xAF,0xB0,0x00,0x30,            // SW       S0, 0x0030 (SP)
    0xAF,0xBE,0x00,0x50,            // SW       FP, 0x0050 (SP)
    0xAF,0xB7,0x00,0x4C,            // SW       S7, 0x004C (SP)
    0xAF,0xB5,0x00,0x44,            // SW       S5, 0x0044 (SP)
    0xAF,0xB4,0x00,0x40,            // SW       S4, 0x0040 (SP)
    0xAC,0x44,0x87,0x28,            // SW       A0, 0x8728 (SP)
    0x26,0xD3,0x1C,0x68,            // ADDIU    S3, S6, 0x1C68
    0x3C,0x02,0x80,0x44             // LUI      V0, 0x8044
};

uint32_t Mask_hookPlay_Init_OoT[] =
{
    0xFFFFFFFF,                     // ADDIU    SP, SP, -0x58           <-- hookPlay_Init_Start
    0xFFFFFFFF,                     // LUI      V0, 0x8045
    0xFFFFFFFF,                     // SW       S6, 0x0048 (SP)
    0xFFFFFFFF,                     // LUI      S6, 0x8044
    0xFFFFFFFF,                     // SW       RA, 0x0054 (SP)
    0xFFFFFFFF,                     // SW       S3, 0x003C (SP)
    0xFFFFFFFF,                     // SW       S2, 0x0038 (SP)
    0xFFFFFFFF,                     // SW       S1, 0x0034 (SP)
    0xFFFFFFFF,                     // SW       S0, 0x0030 (SP)
    0xFFFFFFFF,                     // SW       FP, 0x0050 (SP)
    0xFFFFFFFF,                     // SW       S7, 0x004C (SP)
    0xFFFFFFFF,                     // SW       S5, 0x0044 (SP)
    0xFFFFFFFF,                     // SW       S4, 0x0040 (SP)
    0xFFFF0000,                     // SW       A0, 0x8728 (SP)
    0xFFFF0000,                     // ADDIU    S3, S6, 0x1C68
    0xFFFFFFFF                      // LUI      V0, 0x8044
};

PCSignature Sig_hookPlay_Init_OoT = { 64, Pattern_hookPlay_Init_OoT, Mask_hookPlay_Init_OoT, 0x4D0 };

#pragma endregion   // hookPlay_Init

#pragma region Play_TransitionDone

uint8_t Pattern_Play_TransitionDone_OoT[] =
{
    0x3C,0x02,0x00,0x01,            // LUI      V0, 0x0001             <-- Play_TransitionDone_Start
    0x00,0x82,0x10,0x21,            // ADDU     V0, A0, V0
    0x94,0x42,0x1E,0x1A,            // LHU      V0, 0x1E1A (V0)
    0x27,0xBD,0xFF,0xD8,            // ADDIU    SP, SP, -0x28
    0x34,0x03,0xFF,0xFD,            // ORI      V1, R0, 0xFFFD
    0xAF,0xB1,0x00,0x20,            // SW       S1, 0x0020 (SP)
    0xAF,0xB0,0x00,0x1C,            // SW       S0, 0x001C (SP)
    0xAF,0xBF,0x00,0x24,            // SW       RA, 0x0024 (SP)
    0x00,0x80,0x80,0x25,            // OR       S0, A0, R0
    0xAF,0xA2,0x00,0x10,            // SW       V0, 0x0010 (SP)
    0x10,0x43,0x00,0x32             // BEQ      V0, V1, 0x8042BF88
};

uint32_t Mask_Play_TransitionDone_OoT[] =
{
    0xFFFFFFFF,                     // LUI      V0, 0x0001             <-- Play_TransitionDone_Start
    0xFFFFFFFF,                     // ADDU     V0, A0, V0
    0xFFFFFFFF,                     // LHU      V0, 0x1E1A (V0)
    0xFFFFFFFF,                     // ADDIU    SP, SP, -0x28
    0xFFFFFFFF,                     // ORI      V1, R0, 0xFFFD
    0xFFFFFFFF,                     // SW       S1, 0x0020 (SP)
    0xFFFFFFFF,                     // SW       S0, 0x001C (SP)
    0xFFFFFFFF,                     // SW       RA, 0x0024 (SP)
    0xFFFFFFFF,                     // OR       S0, A0, R0
    0xFFFFFFFF,                     // SW       V0, 0x0010 (SP)
    0xFFFFFFFF                      // BEQ      V0, V1, 0x8042BF88
};

PCSignature Sig_Play_TransitionDone_OoT = { 44, Pattern_Play_TransitionDone_OoT, Mask_Play_TransitionDone_OoT, 0x90 };

#pragma endregion   // Play_TransitionDone

#pragma region EnButte_TransformIntoFairy

uint8_t Pattern_EnButte_TransformIntoFairy_OoT[] =
{
    0x27,0xBD,0xFF,0xB0,             // ADD       SP, SP, -0x50         <-- EnButte_TransformIntoFairy_Start
    0xAF,0xB0,0x00,0x40,             // SW        S0, 0x0040 (SP)
    0x00,0x80,0x80,0x25,             // OR        S0, A0, R0
    0x24,0x84,0x01,0x9C,             // ADDIU     A0, A0, 0x019C
    0xAF,0xBF,0x00,0x4C,             // SW        RA, 0x004C (SP)
    0xAF,0xB2,0x00,0x48,             // SW        S2, 0x0048 (SP)
    0xAF,0xB1,0x00,0x44,             // SW        S1, 0x0044 (SP)
    0x0C,0x02,0x32,0x70,             // JAL       SkelAnime_Update
    0x00,0xA0,0x90,0x25,             // OR        S2, A1, R0
    0x0C,0x07,0x84,0x45,             // JAL       EnButte_UpdateTransformationEffect
    0x24,0x03,0x00,0x05,             // ADDIU     V1, R0, 0x0005
    0x86,0x02,0x02,0x44              // LH        V0, 0x0244 (S0)       <-- Hook here
};

uint32_t Mask_EnButte_TransformIntoFairy_OoT[] =
{
    0xFFFFFFFF,                      // ADD       SP, SP, -0x50         <-- EnButte_TransformIntoFairy_Start
    0xFFFFFFFF,                      // SW        S0, 0x0040 (SP)
    0xFFFFFFFF,                      // OR        S0, A0, R0
    0xFFFFFFFF,                      // ADDIU     A0, A0, 0x019C
    0xFFFFFFFF,                      // SW        RA, 0x004C (SP)
    0xFFFFFFFF,                      // SW        S2, 0x0048 (SP)
    0xFFFFFFFF,                      // SW        S1, 0x0044 (SP)
    0xFF000000,                      // JAL       SkelAnime_Update
    0xFFFFFFFF,                      // OR        S2, A1, R0
    0xFF000000,                      // JAL       EnButte_UpdateTransformationEffect
    0xFFFFFFFF,                      // ADDIU     V1, R0, 0x0005
    0xFFFFFFFF                       // LH        V0, 0x0244 (S0)       <-- Hook here
};

PCSignature Sig_EnButte_TransformIntoFairy_OoT = { 48, Pattern_EnButte_TransformIntoFairy_OoT, Mask_EnButte_TransformIntoFairy_OoT, 0x130 };

#pragma endregion   // EnButte_TransformIntoFairy

PCFastResolver OoTSignatures[] =
{
    { 0x800253E0, 0, { 0 }, &Sig_Actor_Spawn_OoT },
    { 0x803A4AD0, 2, { 0, 0x84 }, &Sig_comboAddItemRawEx_OoT },
    { 0x80000000, 1, { 0 }, &Sig_EnItem00_DropCustom_OoT },
    { 0x80000000, 1, { 0 }, &Sig_comboItemPrecond_OoT },
    { 0x80000000, 1, { 0 }, &Sig_hookPlay_Init_OoT },
    { 0x80000000, 1, { 0 }, &Sig_Play_TransitionDone_OoT },
    { 0x80000000, 1, { 0 }, &Sig_EnButte_TransformIntoFairy_OoT }
};
size_t OoTSignatureCount = sizeof(OoTSignatures) / sizeof(OoTSignatures[0]);

#pragma endregion

#pragma region MM

#pragma region Actor_Spawn

uint8_t Pattern_Actor_Spawn_MM[] =
{
    0x27,0xBD,0x00,0x38             // ADDIU    SP, SP, 0x38           <-- Actor_SpawnAsChildAndCutscene_End, Hook here
};

uint32_t Mask_Actor_Spawn_MM[] =
{
    0xFFFFFFFF,                     // ADDIU    SP, SP, 0x38           <-- Actor_SpawnAsChildAndCutscene_End, Hook here
};

PCSignature Sig_Actor_Spawn_MM = { 4, Pattern_Actor_Spawn_MM, Mask_Actor_Spawn_MM, 0x00 };

#pragma endregion   // Actor_Spawn

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

PCSignature Sig_comboAddItemRawEx_MM = { 28, Pattern_comboAddItemRawEx_MM, Mask_comboAddItemRawEx_MM, 0x40 };

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
    0xFFFF0000,                     // ADDIU    V0, R0, 0x033C
    0xFFFFFFFF,                     // BNE      V1, V0, 0x80400DB4
    0xFFFFFFFF,                     // OR       A1, S0, R0
    0xFF000000                      // JAL      comboXflagsSet          <-- Hook here
};
PCSignature Sig_EnItem00_DropCustom_MM = { 88, Pattern_EnItem00_DropCustom_MM, Mask_EnItem00_DropCustom_MM, 0x54 };

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
    0xFFFF0000,                     // ADDIU    V0, R0, 0x0002
    0xFFFFFFFF,                     // BNE      V1, V0, 0x8040C178
    0xFFFFFFFF,                     // ADDIU    V0, R0, 0x0002
    0xFFFFFFFF                      // LW       S0, 0x0018 (SP)         <-- Hook here
};

PCSignature Sig_comboItemPrecond_MM = { 56, Pattern_comboItemPrecond_MM, Mask_comboItemPrecond_MM, 0x34 };

#pragma endregion   // comboItemPrecond

#pragma region hookPlay_Init

uint8_t Pattern_hookPlay_Init_MM[] =
{
    0x27,0xBD,0xFF,0xB0,            // ADDIU    SP, SP, -0x50           <-- hookPlay_Init_Start
    0x3C,0x02,0x80,0x77,            // LUI      V0, 0x8077
    0xAF,0xB4,0x00,0x40,            // SW       S4, 0x0040 (SP)
    0x3C,0x14,0x80,0x77,            // LUI      S4, 0x8077
    0xAF,0xB3,0x00,0x3C,            // SW       S3, 0x003C (SP)
    0xAC,0x44,0x42,0x48,            // SW       A0, 0x4248 (V0)
    0x26,0x93,0x0F,0xD8,            // ADDIU    S3, S4, 0x0FD8
    0x24,0x02,0xFF,0xFF,            // ADDIU    V0, R0, 0xFFFF
    0xAF,0xBF,0x00,0x4C,            // SW       RA, 0x004C (SP)
    0xAF,0xB5,0x00,0x44,            // SW       S5, 0x0044 (SP)
    0xAF,0xB1,0x00,0x34,            // SW       S1, 0x0034 (SP)
    0xAF,0xB0,0x00,0x30,            // SW       S0, 0x0030 (SP)
    0xAF,0xB6,0x00,0x48,            // SW       S6, 0x0048 (SP)
    0xAF,0xB2,0x00,0x38,            // SW       S2, 0x0038 (SP)
    0xA2,0x62,0x00,0x3C,            // SB       V0, 0x003C (S3)
    0x3C,0x02,0x80,0x77,            // LUI      V0, 0x8077
    0xA0,0x40,0x0E,0xC8             // SB       R0, 0x0EC8 (V0)
};

uint32_t Mask_hookPlay_Init_MM[] =
{
    0xFFFFFFFF,                     // ADDIU    SP, SP, -0x50           <-- hookPlay_Init_Start
    0xFFFFFF00,                     // LUI      V0, 0x8077
    0xFFFFFFFF,                     // SW       S4, 0x0040 (SP)
    0xFFFFFFFF,                     // LUI      S4, 0x8077
    0xFFFFFFFF,                     // SW       S3, 0x003C (SP)
    0xFFFF0000,                     // SW       A0, 0x4248 (V0)
    0xFFFF0000,                     // ADDIU    S3, S4, 0x0FD8
    0xFFFFFFFF,                     // ADDIU    V0, R0, 0xFFFF
    0xFFFFFFFF,                     // SW       RA, 0x004C (SP)
    0xFFFFFFFF,                     // SW       S5, 0x0044 (SP)
    0xFFFFFFFF,                     // SW       S1, 0x0034 (SP)
    0xFFFFFFFF,                     // SW       S0, 0x0030 (SP)
    0xFFFFFFFF,                     // SW       S6, 0x0048 (SP)
    0xFFFFFFFF,                     // SW       S2, 0x0038 (SP)
    0xFFFFFFFF,                     // SB       V0, 0x003C (S3)
    0xFFFFFFFF,                     // LUI      V0, 0x8077
    0xFFFF0000                      // SB       R0, 0x0EC8 (V0)
};

PCSignature Sig_hookPlay_Init_MM = { 68, Pattern_hookPlay_Init_MM, Mask_hookPlay_Init_MM, 0x58C };

#pragma endregion   // hookPlay_Init

#pragma region Play_TransitionDone

uint8_t Pattern_Play_TransitionDone_MM[] =
{
    0x3C,0x02,0x00,0x02,            // LUI      V0, 0x0002           <-- Play_TransitionDone_Start
    0x00,0x82,0x10,0x21,            // ADDU     V0, A0, V0
    0x94,0x42,0x88,0x7A,            // LHU      V0, 0x887A (V0)
    0x27,0xBD,0xFF,0xD8,            // ADDIU    SP, SP, -0x28
    0x34,0x03,0xFF,0xFD,            // ORI      V1, R0, 0xFFFD
    0xAF,0xB1,0x00,0x20,            // SW       S1, 0x0020 (SP)
    0xAF,0xB0,0x00,0x1C,            // SW       S0, 0x001C (SP)
    0xAF,0xBF,0x00,0x24,            // SW       RA, 0x0024 (SP)
    0x00,0x80,0x88,0x25,            // OR       S1, A0, R0
    0xAF,0xA2,0x00,0x10,            // SW       V0, 0x0010 (SP)
    0x10,0x43,0x00,0x20,            // BEQ      V0, V1, 0x80759DDC
    0x3C,0x10,0x80,0x77,            // LUI      S0, 0x8077
    0x34,0x03,0xFF,0xFE,            // ORI      V1, R0, 0xFFFE
    0x10,0x43,0x00,0x19,            // BEQ      V0, C1, 0x80759DDC
    0x34,0x03,0xFF,0xFC,            // ORI      V1, R0, 0xFFFC
    0x50,0x43,0x00,0x49,            // BEQL     V0, V1, 0x80759E94
    0x3C,0x02,0x80,0x77,            // LUI      V0, 0x8077
    0x34,0x03,0xFF,0xFF,            // ORI      V1, R0, 0xFFFF
    0x54,0x43,0x00,0x21             // BNEL     V0, V1, 0x80759E00
};

uint32_t Mask_Play_TransitionDone_MM[] =
{
    0xFFFFFFFF,                     // LUI      V0, 0x0002           <-- Play_TransitionDone_Start
    0xFFFFFFFF,                     // ADDU     V0, A0, V0
    0xFFFFFFFF,                     // LHU      V0, 0x887A (V0)
    0xFFFFFFFF,                     // ADDIU    SP, SP, -0x28
    0xFFFFFFFF,                     // ORI      V1, R0, 0xFFFD
    0xFFFFFFFF,                     // SW       S1, 0x0020 (SP)
    0xFFFFFFFF,                     // SW       S0, 0x001C (SP)
    0xFFFFFFFF,                     // SW       RA, 0x0024 (SP)
    0xFFFFFFFF,                     // OR       S1, A0, R0
    0xFFFFFFFF,                     // SW       V0, 0x0010 (SP)
    0xFFFFFFFF,                     // BEQ      V0, V1, 0x80759DDC
    0xFFFFFFFF,                     // LUI      S0, 0x8077
    0xFFFFFFFF,                     // ORI      V1, R0, 0xFFFE
    0xFFFFFFFF,                     // BEQ      V0, C1, 0x80759DDC
    0xFFFFFFFF,                     // ORI      V1, R0, 0xFFFC
    0xFFFFFFFF,                     // BEQL     V0, V1, 0x80759E94
    0xFFFFFFFF,                     // LUI      V0, 0x8077
    0xFFFFFFFF,                     // ORI      V1, R0, 0xFFFF
    0xFFFFFFFF                      // BNEL     V0, V1, 0x80759E00
};

PCSignature Sig_Play_TransitionDone_MM = { 76, Pattern_Play_TransitionDone_MM, Mask_Play_TransitionDone_MM, 0x1D4 };

#pragma endregion   // Play_TransitionDone

#pragma region EnButte_TransformIntoFairy

uint8_t Pattern_EnButte_TransformIntoFairy_MM[] =
{
    0x27,0xBD,0xFF,0xB0,            // ADDIU      SP, SP, -0x50           <-- EnButte_TransformIntoFairy_Start
    0xAF,0xB0,0x00,0x40,            // SW         S0, 0x0040 (SP)
    0x00,0x80,0x80,0x25,            // OR         S0, A0, R0
    0x24,0x84,0x01,0xA4,            // ADDIU      A0, A0, 0x01A4
    0xAF,0xBF,0x00,0x4C,            // SW         RA, 0x004C (SP)
    0xAF,0xB2,0x00,0x48,            // SW         S2, 0x0048 (SP)
    0xAF,0xB1,0x00,0x44,            // SW         S1, 0x0044 (SP)
    0x0C,0x04,0xDB,0x34,            // JAL        SkelAnime_Update
    0x00,0xA0,0x90,0x25,            // OR         S2, A1, R0
    0x0C,0x10,0x79,0x5E,            // JAL        EnButte_UpdateTransformationEffect
    0x24,0x03,0x00,0x05,            // ADDIU      V1, R0, 0x0005
    0x86,0x02,0x02,0x4C             // LH         V0, 0x024C (S0)
};

uint32_t Mask_EnButte_TransformIntoFairy_MM[] =
{
    0xFFFFFFFF,                      // ADD       SP, SP, -0x50
    0xFFFFFFFF,                      // SW        S0, 0x0040 (SP)
    0xFFFFFFFF,                      // OR        S0, A0, R0
    0xFFFFFFFF,                      // ADDIU     A0, A0, 0x01A4
    0xFFFFFFFF,                      // SW        RA, 0x004C (SP)
    0xFFFFFFFF,                      // SW        S2, 0x0048 (SP)
    0xFFFFFFFF,                      // SW        S1, 0x0044 (SP)
    0xFF000000,                      // JAL       SkelAnime_Update
    0xFFFFFFFF,                      // OR        S2, A1, R0
    0xFF000000,                      // JAL       EnButte_UpdateTransformationEffect
    0xFFFFFFFF,                      // ADDIU     V1, R0, 0x0005
    0xFFFFFFFF                       // LH        V0, 0x024C (S0)
};

PCSignature Sig_EnButte_TransformIntoFairy_MM = { 48, Pattern_EnButte_TransformIntoFairy_MM, Mask_EnButte_TransformIntoFairy_MM, 0x130 };

#pragma endregion   // EnButte_TransformIntoFairy

PCFastResolver MMSignatures[] =
{
    { 0x800BB0B4, 0, { 0 }, &Sig_Actor_Spawn_MM },
    { 0x806D57E4, 2, { 0, 0x84 }, &Sig_comboAddItemRawEx_MM },
    { 0x80000000, 1, { 0 }, &Sig_EnItem00_DropCustom_MM },
    { 0x80000000, 1, { 0 }, &Sig_comboItemPrecond_MM },
    { 0x80000000, 1, { 0 }, &Sig_hookPlay_Init_MM },
    { 0x80000000, 1, { 0 }, &Sig_Play_TransitionDone_MM },
    { 0x80000000, 1, { 0 }, &Sig_EnButte_TransformIntoFairy_MM }
};
size_t MMSignatureCount = sizeof(MMSignatures) / sizeof(MMSignatures[0]);

#pragma endregion
