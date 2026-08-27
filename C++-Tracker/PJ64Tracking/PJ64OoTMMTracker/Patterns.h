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
    0xFFFFFF00,                     // LUI      S6, 0x8044
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
    0xFFFFFF00                      // LUI      V0, 0x8044
};

PCSignature Sig_hookPlay_Init_OoT = { 64, Pattern_hookPlay_Init_OoT, Mask_hookPlay_Init_OoT, 0x04 }; // PC offset was 0x4D0 but it is now resolved by the subpattern function

/* Ancre du site de hook OoT (identique stable/dev). */
uint8_t Pattern_hookInit_Site_OoT[] =
{
    0xAC,0x43,0xB3,0xEC,            // SW       V1, 0xB3EC (V0)   
    0x96,0x22,0x00,0xA4,            // LHU      V0, 0x00A4      <-- HOOK
    0x24,0x03,0x00,0x3C,            // ADDIU    V1, R0, 0x003C
    0x30,0x44,0xFF,0xFD,            // ANDI     A0, V0, 0xFFFD
    0x54,0x83,0x00,0x3A             // BNEL     A0, V1, 0x80431C9C
};

uint32_t Mask_hookInit_Site_OoT[] =
{
    0xFFFF0000,                     // SW       V1, 0xB3EC (V0)
    0xFFFFFFFF,                     // LHU      V0, 0x00A4
    0xFFFFFFFF,                     // ADDIU    V1, R0, 0x003C
    0xFFFFFFFF,                     // ANDI     A0, V0, 0xFFFD
    0xFFFFFFFF                      // BNEL     A0, V1, 0x80431C9C
};

PCSignature Sig_hookInit_Site_OoT = { 20, Pattern_hookInit_Site_OoT, Mask_hookInit_Site_OoT, 0 };

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

//PCSignature Sig_Play_TransitionDone_OoT = { 44, Pattern_Play_TransitionDone_OoT, Mask_Play_TransitionDone_OoT, 0x90 };
PCSignature Sig_Play_TransitionDone_OoT = { 44, Pattern_Play_TransitionDone_OoT, Mask_Play_TransitionDone_OoT, 0x00 };

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

/* Ancre du site de hook MM (identique stable/dev). L'adresse de match EST le PC a hooker. */
uint8_t Pattern_hookInit_Site_MM[] =
{
    0x96,0x22,0x00,0xA4,            // LHU      V0, 0x00A4 (S1)   <-- HOOK
    0x38,0x43,0x00,0x07,            // XORI     V1, V0, 0x0007
    0x2C,0x63,0x00,0x01,            // SLTIU    V1, V1, 0x0001
    0xA2,0x63,0x00,0x08,            // SB       V1, 0x0008 (S3)
    0x24,0x03,0x00,0x07             // ADDIU    V1, R0, 0x0007
};

uint32_t Mask_hookInit_Site_MM[] =
{
    0xFFFFFFFF,                     // LHU      V0, 0x00A4 (S1)
    0xFFFFFFFF,                     // XORI     V1, V0, 0x0007
    0xFFFFFFFF,                     // SLTIU    V1, V1, 0x0001
    0xFFFFFFFF,                     // SB       V1, 0x0008 (S3)
    0xFFFFFFFF                      // ADDIU    V1, R0, 0x0007
};

PCSignature Sig_hookInit_Site_MM = { 20, Pattern_hookInit_Site_MM, Mask_hookInit_Site_MM, 0 };

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
    0xFFFF0000,                     // BEQL     V0, V1, 0x80759E94
    0xFFFFFFFF,                     // LUI      V0, 0x8077
    0xFFFFFFFF,                     // ORI      V1, R0, 0xFFFF
    0xFFFF0000                      // BNEL     V0, V1, 0x80759E00
};

//PCSignature Sig_Play_TransitionDone_MM = { 76, Pattern_Play_TransitionDone_MM, Mask_Play_TransitionDone_MM, 0x1D4 };
PCSignature Sig_Play_TransitionDone_MM = { 76, Pattern_Play_TransitionDone_MM, Mask_Play_TransitionDone_MM, 0x00 };

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

#pragma region Version Profiles

/*
*   Convention squelette : chaque fonction V33 pas encore trouvee a une PCSignature dediee dont
*   PatternSize vaut 0 => elle est traitee comme un stub (PC laisse a 0, ignoree, aucun faux positif).
*   Pour l'activer une fois la fonction desassemblee :
*     1. colle les octets du prologue (big-endian, 4 par instruction) dans Pattern_* ;
*     2. un mask par instruction dans Mask_* (0xFFFFFFFF ; 0xFF000000 pour un JAL ; 0xFFFF0000 pour un immediat 16 bits) ;
*     3. passe le 1er champ de la PCSignature de 0 a la taille reelle (= sizeof(Pattern_*)) ;
*     4. renseigne le PCOffset (offset du hook depuis le debut du match ; hint Legacy entre parentheses).
*/

#pragma region V33 OoT

uint8_t Pattern_comboAddItemRawEx_OoT_V33[] =
{
    0x27,0xBD,0xFF,0xB8,            // ADDIU    SP, SP, -0x48           <-- comboAddItemRawEx_Start
    0xAF,0xB2,0x00,0x3C,            // SW       S2, 0x003C (SP)
    0x00,0x80,0x90,0x25,            // OR       S2, A0, R0
    0x27,0xA4,0x00,0x18,            // ADDIU    A0, SP, 0x0018
    0xAF,0xB3,0x00,0x40,            // SW       S3, 0x0040 (SP)
    0xAF,0xB0,0x00,0x34,            // SW       S0, 0x0034 (SP)
    0xAF,0xBF,0x00,0x44,            // SW       RA, 0x0044 (SP)
    0xAF,0xB1,0x00,0x38             // SW       S1, 0x0038 (SP)
};

uint32_t Mask_comboAddItemRawEx_OoT_V33[] =
{
    0xFFFFFFFF,                     // ADDIU    SP, SP, -0x48
    0xFFFFFFFF,                     // SW       S2, 0x003C (SP)
    0xFFFFFFFF,                     // OR       S2, A0, R0
    0xFFFFFFFF,                     // ADDIU    A0, SP, 0x0018
    0xFFFFFFFF,                     // SW       S3, 0x0040 (SP)
    0xFFFFFFFF,                     // SW       S0, 0x0034 (SP)
    0xFFFFFFFF,                     // SW       RA, 0x0044 (SP)
    0xFFFFFFFF                      // SW       S1, 0x0038 (SP)
};

/* Hook au meme PCOffset (0x40) que la version Legacy : entree + 0x40 = delay-slot du
   JAL comboAddItemRaw, ou S0 = ComboItemQuery* (registre et decodage inchanges). */
PCSignature Sig_comboAddItemRawEx_OoT_V33 = { 32, Pattern_comboAddItemRawEx_OoT_V33, Mask_comboAddItemRawEx_OoT_V33, 0x40 };

/* ---- Squelettes OoT V33 a completer (PatternSize 0 => ignore tant que non rempli) ---- */

/* EnItem00_DropCustom  (hint PCOffset Legacy: 0x54) */
uint8_t  Pattern_EnItem00_DropCustom_OoT_V33[] =
{
    0x27,0xBD,0xFF,0xC8,            // ADDIU    SP, SP, -0x38           <-- EnItem00_DropCustom_Start
    0xAF,0xB1,0x00,0x28,            // SW       S1, 0x0028 (SP)
    0x8C,0x91,0x1C,0x74,            // LW       S1, 0x1C74 (A0)
    0xAF,0xB3,0x00,0x30,            // SW       S3, 0x0030 (SP)
    0xAF,0xB2,0x00,0x2C,            // SW       S2, 0x002C (SP)
    0xAF,0xB0,0x00,0x24,            // SW       S0, 0x0024 (SP)
    0xAF,0xBF,0x00,0x34,            // SW       RA, 0x0034 (SP)
    0x00,0x80,0x90,0x25,            // OR       S2, A0, R0
    0xAF,0xA5,0x00,0x3C,            // SW       A1, 0x003C (SP)
    0x00,0xC0,0x80,0x25,            // OR       S0, A2, R0
    0x24,0x13,0x00,0x15,            // ADDIU    S3, R0, 0x0015
    0x56,0x20,0x00,0x0D,            // BNEZL    S1, 0x80400E24
    0x96,0x22,0x00,0x00,            // LHU      V0, 0x0000 (S1)
    0x00,0x00,0x30,0x25,            // OR       A2, R0, R0
    0x02,0x00,0x28,0x25,            // OR       A1, S0, R0
    0x0C,0x10,0x72,0x8B,            // JAL      comboItemOverride
    0x27,0xA4,0x00,0x10,            // ADDIU    A0, SP, 0x0010
    0x87,0xA3,0x00,0x1C,            // LH       V1, 0x001C (SP)
    0x24,0x02,0x03,0xAE,            // ADDIU    V0, R0, 0x03AE
    0x14,0x62,0x00,0x16,            // BNE      V1, V0, 0x80400E68
    0x02,0x00,0x28,0x25,            // OR       A1, S0, R0
    0x0C,0x10,0x72,0x3F             // JAL      comboXflagsSet          <-- Hook here
};

uint32_t Mask_EnItem00_DropCustom_OoT_V33[]    =
{
    0xFFFFFFFF,                     // ADDIU    SP, SP, -0x38           <-- EnItem00_DropCustom_Start     
    0xFFFFFFFF,                     // SW       S1, 0x0028 (SP)
    0xFFFFFFFF,                     // LW       S1, 0x1C74 (A0)
    0xFFFFFFFF,                     // SW       S3, 0x0030 (SP)
    0xFFFFFFFF,                     // SW       S2, 0x002C (SP)
    0xFFFFFFFF,                     // SW       S0, 0x0024 (SP)
    0xFFFFFFFF,                     // SW       RA, 0x0034 (SP)
    0xFFFFFFFF,                     // OR       S2, A0, R0
    0xFFFFFFFF,                     // SW       A1, 0x003C (SP)
    0xFFFFFFFF,                     // OR       S0, A2, R0
    0xFFFFFFFF,                     // ADDIU    S3, R0, 0x0015
    0xFFFFFFFF,                     // BNEZL    S1, 0x80400E24
    0xFFFFFFFF,                     // LHU      V0, 0x0000 (S1)
    0xFFFFFFFF,                     // OR       A2, R0, R0
    0xFFFFFFFF,                     // OR       A1, S0, R0
    0xFF000000,                     // JAL      comboItemOverride
    0xFFFFFFFF,                     // ADDIU    A0, SP, 0x0010
    0xFFFFFFFF,                     // LH       V1, 0x001C (SP)
    0xFFFF0000,                     // ADDIU    V0, R0, 0x03AE
    0xFFFFFFFF,                     // BNE      V1, V0, 0x80400E68
    0xFFFFFFFF,                     // OR       A1, S0, R0
    0xFF000000                      // JAL      comboXflagsSet          <-- Hook here
};
PCSignature Sig_EnItem00_DropCustom_OoT_V33    = { 88, Pattern_EnItem00_DropCustom_OoT_V33, Mask_EnItem00_DropCustom_OoT_V33, 0x54 };

/* comboItemPrecond  (hint PCOffset Legacy: 0x34) */
uint8_t  Pattern_comboItemPrecond_OoT_V33[] =
{
    0x27,0xBD,0xFF,0xD8,            // ADDIU    SP, SP, -0x28           <-- comboItemPrecond_Start
    0xAF,0xBF,0x00,0x24,            // SW       RA, 0x0024 (SP)
    0xAF,0xB0,0x00,0x20,            // SW       S0, 0x0020 (SP)
    0xAF,0xA4,0x00,0x28,            // SW       A0, 0x0028 (SP)
    0x0C,0x10,0x30,0xA6,            // JAL      comboSyncItems
    0x00,0xA0,0x80,0x25,            // OR       S0, A1, R0
    0x8F,0xA5,0x00,0x28,            // LW       A1, 0x0028 (SP)
    0x0C,0x10,0x31,0x89,            // JAL      comboItemOverride
    0x27,0xA4,0x00,0x10,            // ADDIU    A0, SP, 0x0010
    0x87,0xA3,0x00,0x1C,            // LH       V1, 0x001C (SP)
    0x24,0x02,0x03,0xAE,            // ADDIU    V0, R0, 0x03AE
    0x14,0x62,0x00,0x05,            // BNE      V1, V0, 0x8040C8B0
    0x24,0x02,0x00,0x02,            // ADDIU    V0, R0, 0x0002
    0x8F,0xBF,0x00,0x24,            // LW       RA, 0x0024 (SP)         
    0x8F,0xB0,0x00,0x20             // LW       S0, 0x0020 (SP)         <-- Hook here
};
uint32_t Mask_comboItemPrecond_OoT_V33[]    =
{
    0xFFFFFFFF,                     // ADDIU    SP, SP, -0x28           <-- comboItemPrecond_Start
    0xFFFFFFFF,                     // SW       RA, 0x0024 (SP)
    0xFFFFFFFF,                     // SW       S0, 0x0020 (SP)
    0xFFFFFFFF,                     // SW       A0, 0x0028 (SP)
    0xFF000000,                     // JAL      comboSyncItems
    0xFFFFFFFF,                     // OR       S0, A1, R0
    0xFFFFFFFF,                     // LW       A1, 0x0028 (SP)
    0xFF000000,                     // JAL      comboItemOverride
    0xFFFFFFFF,                     // ADDIU    A0, SP, 0x0010
    0xFFFFFFFF,                     // LH       V1, 0x001C (SP)
    0xFFFF0000,                     // ADDIU    V0, R0, 0x03AE
    0xFFFFFFFF,                     // BNE      V1, V0, 0x8040C8B0
    0xFFFFFFFF,                     // ADDIU    V0, R0, 0x0002
    0xFFFFFFFF,                     // LW       RA, 0x0024 (SP)         
    0xFFFFFFFF                      // LW       S0, 0x0020 (SP)         <-- Hook here
};
PCSignature Sig_comboItemPrecond_OoT_V33    = { 60, Pattern_comboItemPrecond_OoT_V33, Mask_comboItemPrecond_OoT_V33, 0x38 };

/* hookPlay_Init  (PCOffset resolu dynamiquement via Sig_hookInit_Site_OoT au i==4 ; laisser 0.
   NB: le site de hook a peut-etre aussi change en V33 -> prevoir un Sig_hookInit_Site_OoT_V33 si besoin) */
uint8_t  Pattern_hookPlay_Init_OoT_V33[] =
{
    0x27,0xBD,0xFF,0xA8,            // ADDIU    SP, SP, -0x58           <-- hookPlay_Init_Start
    0x3C,0x02,0x80,0x45,            // LUI      V0, 0x8045
    0xAF,0xB6,0x00,0x48,            // SW       S6, 0x0048 (SP)
    0x3C,0x16,0x80,0x44,            // LUI      S6, 0x8044
    0xAF,0xB3,0x00,0x3C,            // SW       S3, 0x003C (SP)
    0xAC,0x44,0x27,0xFC,            // SW       A0, 0x27FC (V0)
    0x26,0xD3,0xBE,0xD8,            // ADDIU    S3, S6, 0xBED8
    0x3C,0x02,0x80,0x45,            // LUI      V0, 0x8045
    0xA0,0x40,0xBC,0xE0,            // SB       R0, 0xBCE0 (V0)
    0x92,0x62,0x00,0x15,            // LBU      V0, 0x0015 (S3)
    0xAF,0xBF,0x00,0x54,            // SW       RA, 0x0054 (SP)
    0x30,0x42,0x00,0xF7,            // ANDI     V0, V0, 0x00F7
    0xAF,0xB2,0x00,0x38,            // SW       S2, 0x0038 (SP)
    0xAF,0xB1,0x00,0x34,            // SW       S1, 0x0034 (SP)
    0xAF,0xB0,0x00,0x30,            // SW       S0, 0x0030 (SP)
    0xAF,0xBE,0x00,0x50,            // SW       FP, 0x0050 (SP)
    0xAF,0xB7,0x00,0x4C,            // SW       S7, 0x004C (SP)
    0xAF,0xB5,0x00,0x44,            // SW       S5, 0x0044 (SP)
    0xAF,0xB4,0x00,0x40,            // SW       S4, 0x0040 (SP)
    0xA2,0x62,0x00,0x15,            // SB       V0, 0x0015 (S3)
    0x3C,0x02,0x80,0x45,            // LUI      V0, 0x8045
    0xAC,0x40,0xC3,0x68             // SW       R0, 0xC368 (V0)
};
uint32_t Mask_hookPlay_Init_OoT_V33[]    =
{
    0xFFFFFFFF,                     // ADDIU    SP, SP, -0x58           <-- hookPlay_Init_Start
    0xFFFFFFFF,                     // LUI      V0, 0x8045
    0xFFFFFFFF,                     // SW       S6, 0x0048 (SP)
    0xFFFFFF00,                     // LUI      S6, 0x8044
    0xFFFFFFFF,                     // SW       S3, 0x003C (SP)
    0xFFFF0000,                     // SW       A0, 0x27FC (V0)
    0xFFFF0000,                     // ADDIU    S3, S6, 0xBED8
    0xFFFFFFFF,                     // LUI      V0, 0x8045
    0xFFFF0000,                     // SB       R0, 0xBCE0 (V0)
    0xFFFFFFFF,                     // LBU      V0, 0x0015 (S3)
    0xFFFFFFFF,                     // SW       RA, 0x0054 (SP)
    0xFFFFFFFF,                     // ANDI     V0, V0, 0x00F7
    0xFFFFFFFF,                     // SW       S2, 0x0038 (SP)
    0xFFFFFFFF,                     // SW       S1, 0x0034 (SP)
    0xFFFFFFFF,                     // SW       S0, 0x0030 (SP)
    0xFFFFFFFF,                     // SW       FP, 0x0050 (SP)
    0xFFFFFFFF,                     // SW       S7, 0x004C (SP)
    0xFFFFFFFF,                     // SW       S5, 0x0044 (SP)
    0xFFFFFFFF,                     // SW       S4, 0x0040 (SP)
    0xFFFFFFFF,                     // SB       V0, 0x0015 (S3)
    0xFFFFFFFF,                     // LUI      V0, 0x8045
    0xFFFF0000                      // SW       R0, 0xC368 (V0)
};
PCSignature Sig_hookPlay_Init_OoT_V33    = { 88, Pattern_hookPlay_Init_OoT_V33, Mask_hookPlay_Init_OoT_V33, 0x04 }; // Was 0x46C but resolved by sub pattern

/* Play_TransitionDone  (hint PCOffset Legacy: 0x00) */
PCSignature Sig_Play_TransitionDone_OoT_V33 = Sig_Play_TransitionDone_OoT;

/* EnButte_TransformIntoFairy  (hint PCOffset Legacy: 0x130) */
uint8_t  Pattern_EnButte_TransformIntoFairy_OoT_V33[] =
{
    0x27,0xBD,0xFF,0xA8,             // ADD       SP, SP, -0x58         <-- EnButte_TransformIntoFairy_Start
    0xAF,0xB0,0x00,0x48,             // SW        S0, 0x0048 (SP)
    0x00,0x80,0x80,0x25,             // OR        S0, A0, R0
    0x24,0x84,0x01,0x9C,             // ADDIU     A0, A0, 0x019C
    0xAF,0xBF,0x00,0x54,             // SW        RA, 0x0054 (SP)
    0xAF,0xB2,0x00,0x50,             // SW        S2, 0x0050 (SP)
    0xAF,0xB1,0x00,0x4C,             // SW        S1, 0x004C (SP)
    0x0C,0x02,0x32,0x70,             // JAL       SkelAnime_Update
    0x00,0xA0,0x90,0x25,             // OR        S2, A1, R0
    0x0C,0x07,0x84,0x45,             // JAL       EnButte_UpdateTransformationEffect
    0x24,0x03,0x00,0x05,             // ADDIU     V1, R0, 0x0005
    0x86,0x02,0x02,0x44              // LH        V0, 0x0244 (S0)       <-- Hook here
};
uint32_t Mask_EnButte_TransformIntoFairy_OoT_V33[]    =
{
    0xFFFFFFFF,                      // ADD       SP, SP, -0x58         <-- EnButte_TransformIntoFairy_Start
    0xFFFFFFFF,                      // SW        S0, 0x0048 (SP)
    0xFFFFFFFF,                      // OR        S0, A0, R0
    0xFFFFFFFF,                      // ADDIU     A0, A0, 0x019C
    0xFFFFFFFF,                      // SW        RA, 0x0054 (SP)
    0xFFFFFFFF,                      // SW        S2, 0x0050 (SP)
    0xFFFFFFFF,                      // SW        S1, 0x004C (SP)
    0xFF000000,                      // JAL       SkelAnime_Update
    0xFFFFFFFF,                      // OR        S2, A1, R0
    0xFF000000,                      // JAL       EnButte_UpdateTransformationEffect
    0xFFFFFFFF,                      // ADDIU     V1, R0, 0x0005
    0xFFFFFFFF                       // LH        V0, 0x0244 (S0)       <-- Hook here
};
PCSignature Sig_EnButte_TransformIntoFairy_OoT_V33    = { 48, Pattern_EnButte_TransformIntoFairy_OoT_V33, Mask_EnButte_TransformIntoFairy_OoT_V33, 0x130 };

/* BaseAddr 0x80000000 => FastPatternResolver echoue volontairement, on se repose sur le scan lent
   FindPatternInPayload (une seule fois par ROM). Les entrees a PatternSize 0 sont ignorees. */
PCFastResolver OoTSignatures_V33[] =
{
    { 0x800253E0, 0, { 0 }, &Sig_Actor_Spawn_OoT },               // 0 Actor_Spawn
    { 0x801DCCC0, 3, { 0, 0xBC, 0x68 }, &Sig_comboAddItemRawEx_OoT_V33 },   // 1 comboAddItemRawEx
    { 0x80000000, 1, { 0 }, &Sig_EnItem00_DropCustom_OoT_V33 },       // 2 EnItem00_DropCustom
    { 0x80000000, 1, { 0 }, &Sig_comboItemPrecond_OoT_V33 },          // 3 comboItemPrecond
    { 0x80000000, 1, { 0 }, &Sig_hookPlay_Init_OoT_V33 },             // 4 hookPlay_Init
    { 0x80000000, 1, { 0 }, &Sig_Play_TransitionDone_OoT_V33 },       // 5 Play_TransitionDone
    { 0x80000000, 1, { 0 }, &Sig_EnButte_TransformIntoFairy_OoT_V33 } // 6 EnButte_TransformIntoFairy
};
size_t OoTSignatures_V33_Count = sizeof(OoTSignatures_V33) / sizeof(OoTSignatures_V33[0]);

#pragma endregion   // V33 OoT

#pragma region V33 MM

/* ---- Squelettes MM V33 a completer (PatternSize 0 => ignore tant que non rempli) ---- */

/* Actor_Spawn  (hint PCOffset Legacy: 0x00)
uint8_t  Pattern_Actor_Spawn_MM_V33[] = { 0x00,0x00,0x00,0x00 };
uint32_t Mask_Actor_Spawn_MM_V33[]    = { 0xFFFFFFFF };
PCSignature Sig_Actor_Spawn_MM_V33    = { 0, Pattern_Actor_Spawn_MM_V33, Mask_Actor_Spawn_MM_V33, 0x00 };
*/
/* comboAddItemRawEx  (hint PCOffset Legacy/OoT V33: 0x40 ; verifier que q reste dans S0)
uint8_t  Pattern_comboAddItemRawEx_MM_V33[] = { 0x00,0x00,0x00,0x00 };
uint32_t Mask_comboAddItemRawEx_MM_V33[]    = { 0xFFFFFFFF };
PCSignature Sig_comboAddItemRawEx_MM_V33    = { 0, Pattern_comboAddItemRawEx_MM_V33, Mask_comboAddItemRawEx_MM_V33, 0x40 };
*/
/* EnItem00_DropCustom  (hint PCOffset Legacy: 0x54) */
uint8_t  Pattern_EnItem00_DropCustom_MM_V33[] =
{
    0x27,0xBD,0xFF,0xC8,            // ADDIU    SP, SP, -0x38           <-- EnItem00_DropCustom_Start
    0xAF,0xB1,0x00,0x28,            // SW       S1, 0x0028 (SP)
    0x8C,0x91,0x1D,0x14,            // LW       S1, 0x1D14 (A0)
    0xAF,0xB3,0x00,0x30,            // SW       S3, 0x0030 (SP)
    0xAF,0xB2,0x00,0x2C,            // SW       S2, 0x002C (SP)
    0xAF,0xB0,0x00,0x24,            // SW       S0, 0x0024 (SP)
    0xAF,0xBF,0x00,0x34,            // SW       RA, 0x0034 (SP)
    0x00,0x80,0x90,0x25,            // OR       S2, A0, R0
    0xAF,0xA5,0x00,0x3C,            // SW       A1, 0x003C (SP)
    0x00,0xC0,0x80,0x25,            // OR       S0, A2, R0
    0x24,0x13,0x00,0x0E,            // ADDIU    S3, R0, 0x000E
    0x56,0x20,0x00,0x0D,            // BNEZL    S1, 0x80721090
    0x96,0x22,0x00,0x00,            // LHU      V0, 0x0000 (S1)
    0x00,0x00,0x30,0x25,            // OR       A2, R0, R0
    0x02,0x00,0x28,0x25,            // OR       A1, S0, R0
    0x0C,0x1C,0xF0,0xDE,            // JAL      comboItemOverride
    0x27,0xA4,0x00,0x10,            // ADDIU    A0, SP, 0x0010
    0x87,0xA3,0x00,0x1C,            // LH       V1, 0x001C (SP)
    0x24,0x02,0x03,0xAE,            // ADDIU    V0, R0, 0x03AE
    0x14,0x62,0x00,0x16,            // BNE      V1, V0, 0x80400E68
    0x02,0x00,0x28,0x25,            // OR       A1, S0, R0
    0x0C,0x1C,0xF0,0xAA             // JAL      comboXflagsSet          <-- Hook here
};
uint32_t Mask_EnItem00_DropCustom_MM_V33[]    =
{
    0xFFFFFFFF,                     // ADDIU    SP, SP, -0x38           <-- EnItem00_DropCustom_Start     
    0xFFFFFFFF,                     // SW       S1, 0x0028 (SP)
    0xFFFFFFFF,                     // LW       S1, 0x1D14 (A0)
    0xFFFFFFFF,                     // SW       S3, 0x0030 (SP)
    0xFFFFFFFF,                     // SW       S2, 0x002C (SP)
    0xFFFFFFFF,                     // SW       S0, 0x0024 (SP)
    0xFFFFFFFF,                     // SW       RA, 0x0034 (SP)
    0xFFFFFFFF,                     // OR       S2, A0, R0
    0xFFFFFFFF,                     // SW       A1, 0x003C (SP)
    0xFFFFFFFF,                     // OR       S0, A2, R0
    0xFFFFFFFF,                     // ADDIU    S3, R0, 0x000E
    0xFFFFFFFF,                     // BNEZL    S1, 0x80721090
    0xFFFFFFFF,                     // LHU      V0, 0x0000 (S1)
    0xFFFFFFFF,                     // OR       A2, R0, R0
    0xFFFFFFFF,                     // OR       A1, S0, R0
    0xFF000000,                     // JAL      comboItemOverride
    0xFFFFFFFF,                     // ADDIU    A0, SP, 0x0010
    0xFFFFFFFF,                     // LH       V1, 0x001C (SP)
    0xFFFF0000,                     // ADDIU    V0, R0, 0x03AE
    0xFFFFFFFF,                     // BNE      V1, V0, 0x80400E68
    0xFFFFFFFF,                     // OR       A1, S0, R0
    0xFF000000                      // JAL      comboXflagsSet          <-- Hook here
};
PCSignature Sig_EnItem00_DropCustom_MM_V33    = { 88, Pattern_EnItem00_DropCustom_MM_V33, Mask_EnItem00_DropCustom_MM_V33, 0x54 };

/* comboItemPrecond  (hint PCOffset Legacy: 0x34) 
uint8_t  Pattern_comboItemPrecond_MM_V33[] = { 0x00,0x00,0x00,0x00 };
uint32_t Mask_comboItemPrecond_MM_V33[]    = { 0xFFFFFFFF };
PCSignature Sig_comboItemPrecond_MM_V33    = { 0, Pattern_comboItemPrecond_MM_V33, Mask_comboItemPrecond_MM_V33, 0x34 };
*/

/* hookPlay_Init  (PCOffset resolu dynamiquement via Sig_hookInit_Site_MM au i==4 ; laisser 0.
   NB: le site de hook a peut-etre aussi change en V33 -> prevoir un Sig_hookInit_Site_MM_V33 si besoin) */
uint8_t  Pattern_hookPlay_Init_MM_V33[] =
{
    0x27,0xBD,0xFF,0xB0,            // ADDIU    SP, SP, -0x50           <-- hookPlay_Init_Start
    0x3C,0x02,0x80,0x77,            // LUI      V0, 0x8077
    0xAF,0xB4,0x00,0x40,            // SW       S4, 0x0040 (SP)
    0x3C,0x14,0x80,0x77,            // LUI      S4, 0x8077
    0xAF,0xB3,0x00,0x3C,            // SW       S3, 0x003C (SP)
    0xAC,0x44,0x2C,0xF0,            // SW       A0, 0x2CF0 (V0)
    0x26,0x93,0xF9,0xF8,            // ADDIU    S3, S4, 0xF9F8
    0x24,0x02,0xFF,0xFF,            // ADDIU    V0, R0, 0xFFFF
    0xAF,0xBF,0x00,0x4C,            // SW       RA, 0x004C (SP)
    0xAF,0xB5,0x00,0x44,            // SW       S5, 0x0044 (SP)
    0xAF,0xB1,0x00,0x34,            // SW       S1, 0x0034 (SP)
    0xAF,0xB0,0x00,0x30,            // SW       S0, 0x0030 (SP)
    0xAF,0xB6,0x00,0x48,            // SW       S6, 0x0048 (SP)
    0xAF,0xB2,0x00,0x38,            // SW       S2, 0x0038 (SP)
    0xA2,0x62,0x00,0x38,            // SB       V0, 0x0038 (S3)
    0x3C,0x02,0x80,0x77,            // LUI      V0, 0x8077
    0xA0,0x40,0xF8,0xE8             // SB       R0, 0xF8E8 (V0)
};
uint32_t Mask_hookPlay_Init_MM_V33[] =
{
    0xFFFFFFFF,                     // ADDIU    SP, SP, -0x50           <-- hookPlay_Init_Start
    0xFFFFFFFF,                     // LUI      V0, 0x8077
    0xFFFFFFFF,                     // SW       S4, 0x0040 (SP)
    0xFFFFFFFF,                     // LUI      S4, 0x8077
    0xFFFFFFFF,                     // SW       S3, 0x003C (SP)
    0xFFFF0000,                     // SW       A0, 0x2CF0 (V0)
    0xFFFF0000,                     // ADDIU    S3, S4, 0xF9F8
    0xFFFFFFFF,                     // ADDIU    V0, R0, 0xFFFF
    0xFFFFFFFF,                     // SW       RA, 0x004C (SP)
    0xFFFFFFFF,                     // SW       S5, 0x0044 (SP)
    0xFFFFFFFF,                     // SW       S1, 0x0034 (SP)
    0xFFFFFFFF,                     // SW       S0, 0x0030 (SP)
    0xFFFFFFFF,                     // SW       S6, 0x0048 (SP)
    0xFFFFFFFF,                     // SW       S2, 0x0038 (SP)
    0xFFFFFFFF,                     // SB       V0, 0x0038 (S3)
    0xFFFFFFFF,                     // LUI      V0, 0x8077
    0xFFFF0000                      // SB       R0, 0xF8E8 (V0)
};
PCSignature Sig_hookPlay_Init_MM_V33    = { 68, Pattern_hookPlay_Init_MM_V33, Mask_hookPlay_Init_MM_V33, 0x58C };

/* Play_TransitionDone  (hint PCOffset Legacy: 0x00) */
uint8_t  Pattern_Play_TransitionDone_MM_V33[] =
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
uint32_t Mask_Play_TransitionDone_MM_V33[] =
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
    0xFFFF0000,                     // BEQL     V0, V1, 0x80759E94
    0xFFFFFFFF,                     // LUI      V0, 0x8077
    0xFFFFFFFF,                     // ORI      V1, R0, 0xFFFF
    0xFFFF0000                      // BNEL     V0, V1, 0x80759E00
};
PCSignature Sig_Play_TransitionDone_MM_V33    = { 76, Pattern_Play_TransitionDone_MM_V33, Mask_Play_TransitionDone_MM_V33, 0x00 };

/* EnButte_TransformIntoFairy  (hint PCOffset Legacy: 0x130) */
uint8_t  Pattern_EnButte_TransformIntoFairy_MM_V33[] =
{
    0x27,0xBD,0xFF,0xA8,             // ADD       SP, SP, -0x58         <-- EnButte_TransformIntoFairy_Start
    0xAF,0xB0,0x00,0x48,             // SW        S0, 0x0048 (SP)
    0x00,0x80,0x80,0x25,             // OR        S0, A0, R0
    0x24,0x84,0x01,0xA4,             // ADDIU     A0, A0, 0x01A4
    0xAF,0xBF,0x00,0x54,             // SW        RA, 0x0054 (SP)
    0xAF,0xB2,0x00,0x50,             // SW        S2, 0x0050 (SP)
    0xAF,0xB1,0x00,0x4C,             // SW        S1, 0x004C (SP)
    0x0C,0x02,0x32,0x70,             // JAL       SkelAnime_Update
    0x00,0xA0,0x90,0x25,             // OR        S2, A1, R0
    0x0C,0x07,0x84,0x45,             // JAL       EnButte_UpdateTransformationEffect
    0x24,0x03,0x00,0x05,             // ADDIU     V1, R0, 0x0005
    0x86,0x02,0x02,0x4C              // LH        V0, 0x024C (S0)       <-- Hook here
};
uint32_t Mask_EnButte_TransformIntoFairy_MM_V33[]    =
{
    0xFFFFFFFF,                      // ADD       SP, SP, -0x58         <-- EnButte_TransformIntoFairy_Start
    0xFFFFFFFF,                      // SW        S0, 0x0048 (SP)
    0xFFFFFFFF,                      // OR        S0, A0, R0
    0xFFFFFFFF,                      // ADDIU     A0, A0, 0x01A4
    0xFFFFFFFF,                      // SW        RA, 0x0054 (SP)
    0xFFFFFFFF,                      // SW        S2, 0x0050 (SP)
    0xFFFFFFFF,                      // SW        S1, 0x004C (SP)
    0xFF000000,                      // JAL       SkelAnime_Update
    0xFFFFFFFF,                      // OR        S2, A1, R0
    0xFF000000,                      // JAL       EnButte_UpdateTransformationEffect
    0xFFFFFFFF,                      // ADDIU     V1, R0, 0x0005
    0xFFFFFFFF                       // LH        V0, 0x024C (S0)       <-- Hook here
};
PCSignature Sig_EnButte_TransformIntoFairy_MM_V33    = { 48, Pattern_EnButte_TransformIntoFairy_MM_V33, Mask_EnButte_TransformIntoFairy_MM_V33, 0x130 };

PCFastResolver MMSignatures_V33[] =
{
    { 0x800BB0B4, 0, { 0 }, &Sig_Actor_Spawn_MM },               // 0 Actor_Spawn
    { 0x806C57E4, 2, { 0, 0x84 }, &Sig_comboAddItemRawEx_OoT_V33 },         // 1 comboAddItemRawEx
    { 0x80000000, 1, { 0 }, &Sig_EnItem00_DropCustom_MM_V33 },       // 2 EnItem00_DropCustom
    { 0x80000000, 1, { 0 }, &Sig_comboItemPrecond_OoT_V33 },          // 3 comboItemPrecond
    { 0x80000000, 1, { 0 }, &Sig_hookPlay_Init_MM_V33 },             // 4 hookPlay_Init
    { 0x80000000, 1, { 0 }, &Sig_Play_TransitionDone_MM_V33 },       // 5 Play_TransitionDone
    { 0x80000000, 1, { 0 }, &Sig_EnButte_TransformIntoFairy_MM_V33 } // 6 EnButte_TransformIntoFairy
};
size_t MMSignatures_V33_Count = sizeof(MMSignatures_V33) / sizeof(MMSignatures_V33[0]);

#pragma endregion   // V33 MM

/*
*   Table des profils de version. La detection (cf. DetectVersionProfile) se fait :
*     1. CRC ROM (romBase+0x10/0x14) : SEULEMENT un fast-path pour LE build de reference exact.
*        Le CRC OoTMM varie avec la config du joueur (des #define retirent/ajoutent du code), il
*        ne peut donc PAS servir de cle de version. Il ne matche qu'une reference exacte, jamais
*        un mauvais profil -> aucune detection erronee possible.
*     2. Probe = le vrai detecteur : prologue de comboAddItemRawEx (index 1) du jeu courant. Sa frame
*        est imposee par la taille de ComboItemOverride (= version) et le bloc multi est gate au
*        runtime (toujours compile) -> robuste a la config.
*     3. Repli sur le profil 0 (Legacy).
*   Le hint spoiler (gData->HostROMVersion) reste optionnel (cf. ApplyHostVersion).
*
*   NothingID == 0        => on conserve la logique CRC/hookInit existante (stable/dev).
*   CrcLo/CrcHi == 0      => pas de reference exacte : profil detecte par probe uniquement.
*/
VersionProfile gProfiles[] =
{
    // Legacy (<= v32.3 : stable + dev, meme payload ; NothingID gere par la logique CRC/hookInit)
    { "Legacy", 0x69F7A146, 0x224AFE45, 0x000003A7, 0x800, OoTSignatures,     OoTSignatureCount,       MMSignatures,     MMSignatureCount },

    // V33 (payload recompile ; CRC + NothingID a renseigner quand connus)
    { "V33",    0x00000000, 0x00000000, 0x000003AE, 0x464, OoTSignatures_V33, OoTSignatures_V33_Count, MMSignatures_V33, MMSignatures_V33_Count }
};
size_t gProfileCount = sizeof(gProfiles) / sizeof(gProfiles[0]);

#pragma endregion   // Version Profiles
