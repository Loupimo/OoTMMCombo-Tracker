#pragma once

#include "pch.h"

#define PAYLOAD_START 0x80400000         // The combo payload start RAM address.
#define PAYLOAD_END   0x80800000         // The combo payload last RAM address.
#define MAX_JAL 10                       // The maximum number of JAL instructions a fast pattern resolver can have.
#define PATTERN_STATE_SIZE 32            // The size of the GamePatternState structure.
#define OOT_HOOK_INIT_STABLE_PCOFF 0x4D0 // The offset to add to the hookInit_Play OoT function to place the hook for stable release.
#define OOT_HOOK_INIT_DEV_PCOFF 0x490    // The offset to add to the hookInit_Play OoT function to place the hook for dev release.
#define MM_HOOK_INIT_STABLE_PCOFF 0x58C  // The offset to add to the hookInit_Play MM function to place the hook for stable release.
#define MM_HOOK_INIT_DEV_PCOFF 0x5C0     // The offset to add to the hookInit_Play MM function to place the hook for dev release.

typedef struct GamePatternState
{
    bool Resolved = false;
    uint32_t PCs[7] = { 0 };    // ID 0 = Actor_Spawn, ID 1 = comboAddItemRawEx, ID 2 = EnItem00_DropCustom, ID 3 = comboItemPrecond, ID 4 = hookPlay_Init, ID 5 = Play_TransitionDone, ID 6 = EnButte_TransformIntoFairy
} GamePatternState;


extern GamePatternState gPatternState[2];   // Contains the pattern status and PCs of both game.


typedef struct PCSignature
{
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
    PCSignature* Signature;         // The associated pattern to find
} PCFastResolver;


typedef struct VersionProfile
{
    const char*      Name;          // Nom lisible du profil (log).
    uint32_t         CrcLo;         // CRC ROM (romBase + 0x10). 0 = inconnu (build dev / futur), detecte par probe.
    uint32_t         CrcHi;         // CRC ROM (romBase + 0x14). 0 = inconnu.
    uint32_t         NothingID;     // "Nothing" combo item ID impose par le profil. 0 = garder la detection CRC/hookInit existante.
    PCFastResolver*  OoTSigs;       // Table des signatures OoT de ce profil (meme ordre/taille que GamePatternState::PCs).
    size_t           OoTSigCount;   // Nombre d'entrees dans OoTSigs.
    PCFastResolver*  MMSigs;        // Table des signatures MM de ce profil.
    size_t           MMSigCount;    // Nombre d'entrees dans MMSigs.
} VersionProfile;


extern VersionProfile gProfiles[];  // Table des profils de version connus.
extern size_t gProfileCount;        // Nombre de profils dans gProfiles.
extern int32_t gActiveProfile;      // Index du profil actif dans gProfiles, -1 tant qu'aucun n'est detecte.


/*
*   Check if the intructions starting at the given RAM address match the desired pattern.
*
*	@param Addr		The real RAM address to start looking at.
*	@param Sig		The signature containing the pattern to find.
*
*   @return <b>True</b> if the all the instructions starting at the given address match the pattern, <b>false</b> otherwise.
*/
bool MatchPattern(uintptr_t Addr, const PCSignature* Sig);

/*
*   Try to find the desired pattern in the given RAM address range.
*
*	@param Sig		        The signature containing the pattern to find.
*	@param PayloadStart		The game RAM address to start looking at.
*	@param PayloadEnd		The game RAM address to stop looking for the pattern.
*
*   @return The program counter address (in game RAM format) if found, 0 otherwise.
*/
uintptr_t FindPatternInPayload(const PCSignature* Sig, size_t PayloadStart, size_t PayloadEnd);

/*
*   Cherche un sous-pattern dans [base, base + limit] (pas de 4 octets), a partir d'une base deja resolue.
*
*	@param base		La base (adresse virtuelle) a partir de laquelle scanner.
*	@param sig		La signature du sous-pattern a trouver.
*	@param limit	La distance maximale (en octets) a parcourir depuis base.
*
*   @return Le PC (adresse virtuelle) du match, 0 si introuvable.
*/
uintptr_t FindSubPattern(uintptr_t base, const PCSignature* sig, uint32_t limit);

/*
*   Tell if the given instruction is a JAL (MIPS) instruction.
*
*	@param InstrucVal		The instruction to check.
*
*   @return <b>True</b> if the given instruction is a JAL (MIPS) instruction, <b>false</b> otherwise.
*/
bool IsJAL(uint32_t InstrucVal);

/*
*   Resolve the address pointed by the given JAL (MIPS) instruction.
*
*	@param InstrucVal	The JAL instruction containing the address to resolve.
*	@param JALAddr		The current address of the given JAL instruction.
*
*   @return The game RAM address pointed by the JAL instruction.
*/
uintptr_t ResolveJAL(uint32_t InstrucVal, uint32_t JALAddr);

/*
*   The to find the desired pattern using the fast method.
*
*	@param Target	The pattern to find.
* 
*   @return The program counter address if found, 0 otherwise.
*/
uintptr_t FastPatternResolver(const PCFastResolver& Target);

/*
*   Resolve the butterfly pattern.
*/
void ResolveButterflyTransform();

/*
*   Reset the program counter used to tracked the butterfly functions.
*/
void ResetButterflyTransform();

/*
*   Detect the version profile matching the currently loaded ROM. The detection order is:
*     1. ROM CRC (gData->GameVersion, available as soon as the ROM is loaded, no sync needed);
*     2. probe of the current game comboAddItemRawEx discriminator (index 1);
*     3. fallback to the first profile (Legacy).
*
*   @return The index of the detected profile in gProfiles.
*/
int32_t DetectVersionProfile();

/*
*   Try to resolve all parttern for the current active game.
*/
void BuildPCsPatterns();

/*
*   Try to resolve the last scene offset.
*/
void FindLastSceneAddress();
