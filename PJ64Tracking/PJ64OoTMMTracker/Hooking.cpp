#include "pch.h"
#include "Hooking.h"
#include "PatternScanner.h"

// Hooking installation
void* gatewayPC = NULL;                                     // The gateway to the original hook PC code. Called when the PC hook is finished.
void* gatewayROM = NULL;                                    // The gateway to the original hook ROM code. Called when the ROM hook is finished.
BYTE originalBytesPC[HOOK_PC_SIZE];                         // The original bytes codes loacted at the PC hook.
BYTE originalBytesROM[HOOK_ROM_LOAD_SIZE];                  // The original bytes codes located at the ROM hook.

// Game data / shared memeory data
SharedData* gData = nullptr;                                // The shared data with the tracker.
GameID gGame = GAME_OOT;                                    // The current running game.
bool gIsRAMLoaded = false;                                  // Tells if the current game RAM is ready.
bool forceGameCheck = false;                                // Force a game check in order to be sure to track the correct PCs.
uintptr_t moduleBase = 0;                                   // The module base address of Project 64.
uintptr_t regBase = 0;                                      // The RAM address where the Project 64 registers are stored.
uintptr_t romBase = 0;                                      // The RAM address where the Project 64 ROM section is stored.
uintptr_t gameRAMBase = 0;                                  // The RAM address where the game data are stored.
uint32_t* gActivePCs = nullptr;                             // The current set of program counters that are tracked.
uint32_t gActiveButterflyID = OOT_BUTTERFLY_ID;             // The current butterfly actor ID to compare to.
uint32_t gActiveFairyID = OOT_FAIRY_ID;                     // The current fairy actor ID to compare to.
uint32_t gActiveBigFairyID = OOT_BIG_FAIRY_ID;              // The current big fairy actor ID to compare to.
uint32_t gActiveActorOff = OOT_ACTOR_ID;                    // The current offset to apply to reach the actor ID value.
uint32_t gActiveFairyActorCombo = OOT_FAIRY_COMBO_OFFSET;   // The current offset to add to reach the combo query item of fairy.
uint32_t gActiveNextEntrance = OOT_NEXT_ENTRANCE;           // The current offset to add to reach the next entrance value.
uint32_t gActiveSongOffset = OOT_LAST_SONG_ID;              // The current offset to add to reach the last played song ID.
uint32_t gActiveOwlOffset = OOT_OWL_CHOICE_ID;              // The current offset to add to reach the selected owl ID.
uint32_t gActiveRoomOffset = OOT_CURR_ROOM;                 // The current offset to add to reach the current room ID.
uint32_t gActiveGrottoOffset = OOT_GROTTO_DATA;             // The current offset to add to reach the grotto data value.
uint32_t gActiveCoordOffset = OOT_PLAYER_COORD;             // The current offset to add to reach the last respawned player corrdinates.
uint32_t gActiveSceneOffset = OOT_SCENE_OFFSET;             // The current offset to add to reach the last scene offset.
uint32_t gActiveFaroreOffset = OOT_FARORE_STATE;            // The current offset to add to reach the farore state offset.
uint32_t gOOTActiveGlobalOffset = 0;                        // An offset to add to the active scene offset to reach the gLastScene variable for OoT.
uint32_t gMMActiveGlobalOffset = 0;                         // An offset to add to the active scene offset to reach the gLastScene variable for MM.
//uint32_t gActiveEntranceReg = S1_OFFSET;                    // The current register to use to get the next entrance value.
uint32_t gDetectCounter = 0;                                // The counter used to trigger a game check.
uint32_t gSP = 0;                                           // The last value of the stack pointer.
uint32_t gA1 = 0;                                           // The last value of the A1 register.
uint32_t gV0 = 0;                                           // The last value of the V0 register.
uint32_t gV1 = 0;                                           // The last value of the V1 register.
uint32_t gS0 = 0;                                           // the last value of the S0 register.


void TryResolveROMBase()
{
    if (romBase)
    {
        return;
    }

    uintptr_t romPtrAddr = moduleBase + ROM_PTR_OFFSET;
    uintptr_t romStruct = *(uintptr_t*)romPtrAddr;

    if (!romStruct)
    {
        return;
    }

    uintptr_t rom = *(uintptr_t*)(romStruct + ROM_OFFSET);

    if (!rom)
    {
        return;
    }

    romBase = rom;

    LOG("ROM base resolved via pointer: 0x%08X", romBase);
}


uintptr_t FindGameRAM()
{
    while (true)
    {
        uintptr_t obj = *(uintptr_t*)(moduleBase + 0x1B00BC);

        if (obj)
        {
            uintptr_t gameRAM = *(uintptr_t*)(obj + 0x1074);

            if (gameRAM && gameRAM > 0x10000000) // sanity check
                return gameRAM;
        }

        Sleep(100);
    }
}


/*
*   Check if the given address is in range 0x80000000 and 0x80FFFFFF.
*
*   @param Addr     The address to test.
*   @param Tmp      The temporary register to use.
*   @param Jump     The label to jump to in case the address in not in the range.
*/
#define IS_ADDR_VALID(Addr, Tmp, Jump)  \
__asm mov Tmp, Addr                     \
__asm and Tmp, 0FF000000h               \
__asm cmp Tmp, 080000000h               \
__asm jne Jump


/*
*   Compute and store the address matching the desired index of a SharedData structure.
*
*   @param DataStart     The address to start at.
*   @param CurrIndex     The index to reach.
*   @param Dst           The register to store the address to.
*/
#define COMPUTE_INDEX(DataStart, CurrIndex, Dst) \
__asm lea Dst, [CurrIndex * 8]                   \
__asm lea Dst, [DataStart + Dst * 4 + 16]


/*
*   Increment the index of a SharedData structure.
*
*   @param Target     The SharedData Structure to increment.
*   @param Tmp        The temporary register to use.
*/
#define INC_INDEX(Target, Tmp)                   \
__asm mov Tmp, [Target]                          \
__asm inc dword ptr[Tmp + 12]                    \
__asm and dword ptr[Tmp + 12], BUFFER_SIZE - 1


/*
*   Read the value of the desired N64 register.
*
*   @param N64Register     The N64 register to read.
*   @param DstReg          The register to store the read value.
*/
#define READ_N64_REG(N64Register, DstReg)  \
__asm mov DstReg, [regBase]                \
__asm mov DstReg, [DstReg]                 \
__asm mov DstReg, [DstReg + N64Register]


/*
*   Compute and store the real RAM address matching the given virtual game RAM address.
*
*   @param GameAddr     The virtual game RAM address (0x80000000 - 0x80FFFFFF).
*   @param DstReg       The register to store the real RAM address to.
*/
#define COMPUTE_RAM_ADDR(GameAddr, DstReg)  \
__asm mov DstReg, GameAddr                  \
__asm and DstReg, 00FFFFFFh                 \
__asm add DstReg, [gameRAMBase]


/*
*   Set all active settings based on the given game.
*
*   @param Game     The game to activate the settings on.
*/
#define SET_ACTIVE_SETTINGS(Game)                                 \
__asm mov[gActiveButterflyID], ##Game##_BUTTERFLY_ID              \
__asm mov[gActiveFairyID], ##Game##_FAIRY_ID                      \
__asm mov[gActiveBigFairyID], ##Game##_BIG_FAIRY_ID               \
__asm mov[gActiveActorOff], ##Game##_ACTOR_ID                     \
__asm mov[gActiveFairyActorCombo], ##Game##_FAIRY_COMBO_OFFSET    \
__asm mov[gActiveNextEntrance], ##Game##_NEXT_ENTRANCE            \
__asm mov[gActiveSongOffset], ##Game##_LAST_SONG_ID               \
__asm mov[gActiveOwlOffset], ##Game##_OWL_CHOICE_ID               \
__asm mov[gActiveRoomOffset], ##Game##_CURR_ROOM                  \
__asm mov[gActiveGrottoOffset], ##Game##_GROTTO_DATA              \
__asm mov[gActiveCoordOffset], ##Game##_PLAYER_COORD              \
__asm mov[gActiveSceneOffset], ##Game##_SCENE_OFFSET              \
__asm mov[gActiveFaroreOffset], ##Game##_FARORE_STATE             \
__asm mov eax, g##Game##ActiveGlobalOffset                        \
__asm add[gActiveSceneOffset], eax


__declspec(naked) void CaptureXFlagASM()
{
    __asm
    {
        IS_ADDR_VALID(ebx, eax, Done)

        push edi

        mov eax, [gData]

        // Get CurrIndex
        mov edx, [eax + 12]

        COMPUTE_INDEX(eax, edx, edi)
        COMPUTE_RAM_ADDR(ebx, edx)

        // Fill the buffer at the correct index
        mov[edi], ecx       // Store PC
        mov[edi + 4], ebx   // Store Mem

        mov eax, [edx]      // Load Key
        mov ecx, [edx + 4]  // Load GI

        mov[edi + 8], eax   // Store Key
        mov[edi + 12], ecx  // Store GI

        // Build flags
        movzx eax, byte ptr[gGame] // The game the XFlag comes from 
        or eax, 0FFFF0000h         // Set the IsConsume flag
        mov[edi + 16], eax         // Store flags

        mov[edi + 20], 0  // q3
        mov[edi + 24], 0  // q4
        mov[edi + 28], 0  // q5

        INC_INDEX(gData, eax)

        pop edi

        Done:
            ret
    }
}


__declspec(naked) void DetectCurrentGameASM()
{
    __asm
    {
        mov eax, [gameRAMBase]

        // ====================
        // Check OoT
        // ====================

        mov ecx, [eax + (0x8011A5F0 & 0x00FFFFFF)]
        and ecx, 0FFFF0000h

        // ZELD
        cmp dword ptr [eax + (0x8011A5EC & 0x00FFFFFF)], 0x5A454C44
        jne CHECK_MM

        // AZ
        cmp ecx, 0x415A0000
        jne CHECK_MM

        mov byte ptr [gGame], GAME_OOT
        ret

        // ====================
        // Check MM
        // ====================

        CHECK_MM:

            mov ecx, [eax + (0x801EF698 & 0x00FFFFFF)]
            and ecx, 0FFFF0000h

            // ZELD
            cmp dword ptr [eax + (0x801EF694 & 0x00FFFFFF)], 0x5A454C44
            jne DONE

            // A3
            cmp ecx, 0x41330000
            jne DONE

            mov byte ptr [gGame], GAME_MM
            ret

       DONE:

            mov byte ptr [gGame], GAME_UNKNOWN
            ret
    }
}


__declspec(naked) void CheckGameVersionASM()
{
    __asm
    {
        mov ecx, [gData]
        mov edx, [romBase]
        mov eax, [edx + 0x10]
        mov ebx, [edx + 0x14]

        // Compare game version
        cmp[ecx], eax // low
        jne STORE_VERSION

        cmp[ecx + 4], ebx // high
        jne STORE_VERSION

        mov eax, 1
        ret

        STORE_VERSION :
            mov [ecx], eax
            mov [ecx + 4], ebx
            mov ecx, [gPatternState]
            mov [ecx], 0                  // Reset gPatternState[GAME_OOT].Resolved
            mov [ecx + 20], 0             // Reset gPatternState[GAME_MM].Resolved
    }
}


__declspec(naked) void PCHook()
{
    __asm
    {
        // Setup
        push eax
        push ecx
        push edx
        push ebx

        // ====================
        // Check if game is known
        // ====================

        // if gGame = GAME_UNKNOWN then check game
        cmp byte ptr [gGame], GAME_UNKNOWN
        jne PERIODIC_GAME_CHECK

        // Detect which game is loaded and if valid, gather the version
        call DetectCurrentGameASM
        cmp byte ptr [gGame], GAME_UNKNOWN
        je DONE

        // The game has changed gIsRAMLoaded should be set to false
        mov ecx, [esi]
        mov byte ptr [gIsRAMLoaded], 0
        jmp IS_RAM_LOADED
        
        PERIODIC_GAME_CHECK:
            
            // Check if we are forcing game detection
            cmp [forceGameCheck], 1
            je GAME_CHECK

            // Trigger a game check when condition are met
            inc dword ptr [gDetectCounter]
            cmp dword ptr [gDetectCounter], DETECT_THROTTLE
            jb IS_RAM_LOADED

        GAME_CHECK:

            // Reset the counter
            mov dword ptr [gDetectCounter], 0

            // ebx is not used by the detect game function
            mov bl, byte ptr [gGame]            // Store the current game state
            call DetectCurrentGameASM           // Get the current game and store it to gGame

            // if curr game = prev game -> check that RAM is loaded
            cmp byte ptr [gGame], bl
            je IS_RAM_LOADED

            // The game has changed, we need to check for RAM status and pattern first
            mov byte ptr [gIsRAMLoaded], 0
            call ResetButterflyTransform
            jmp IS_RAM_LOADED

        IS_RAM_LOADED :

            // Read PC
            mov ecx, [esi]

            // if RAM loaded -> start test
            cmp byte ptr [gIsRAMLoaded], 0
            jne TEST_DISPATCHER

            // Check if game is OoT and Play_Main active
            cmp byte ptr [gGame], GAME_OOT
            jne CHECK_MM

            // Set OoT active settings
            SET_ACTIVE_SETTINGS(OOT)
            //mov [gActiveEntranceReg], S1_OFFSET
            
            // Check that the RAM is loaded
            cmp ecx, OOT_PLAY_MAIN
            je RAM_LOADED

            // Check that the RAM is loaded via play init
            cmp ecx, OOT_PLAY_INIT
            je RAM_LOADED

            jmp DONE

        CHECK_MM :
            
            // Set the MM active settings
            SET_ACTIVE_SETTINGS(MM)
            //mov[gActiveEntranceReg], V1_OFFSET

            // Check that the RAM is loaded
            cmp ecx, MM_PLAY_MAIN
            je RAM_LOADED

            // Check that the RAM is loaded via play init
            cmp ecx, MM_PLAY_INIT
            je RAM_LOADED

            jmp DONE

        RAM_LOADED :
            mov [gIsRAMLoaded], 1
            mov [forceGameCheck], 0

            // Get the current game pattern state
            lea eax, [gPatternState]
            movzx edx, byte ptr [gGame]
            imul edx, PATTERN_STATE_SIZE

            // Apply the gPatternState[gGame].PCs array to the activePCs array in any cases
            lea ebx, [eax + edx + 4]
            mov [gActivePCs], ebx

            // if patterns are resolved -> Test Dispatcher
            cmp byte ptr [eax + edx], 0
            jne TEST_DISPATCHER

            // The pattern is not built
            call BuildPCsPatterns

        TEST_DISPATCHER :

            // Get the active PCs. If a crash occurs here it has a high probability that gActivePCs = nullptr. No check is done to not impact perf so be sure it points somewhere valid before calling the dispatcher
            mov eax, [gActivePCs]

            // if PC = Actor_Spawn -> check if it is a FAIRY, BIG_FAIRY or BUTTERFLY
            cmp ecx, [eax]
            je CHECK_ACTOR

            // if PC = comboItemAddRawEx -> add item test
            cmp ecx, [eax + 4]
            je ADD_ITEM_TEST

            // if PC = En_Item00_DropCustom -> Drop custom test ("Nothing" items from boulders, trees, bushes, grass, rocks, pots, ...)
            cmp ecx, [eax + 8]
            je DROP_CUSTOM_TEST

            // if PC = comboItemPrecond -> Shop test (Buying a "Nothing" item at the shop)
            cmp ecx, [eax + 12]
            je SHOP_TEST

            // if PC = hookPlay_Init -> Play init test
            cmp ecx, [eax + 16]
            je PLAY_INIT_TEST

            // if PC = Play_TransitionDone -> Transition test
            cmp ecx, [eax + 20]
            je TRANSITION_TEST

            // if PC = EnButte_TransformIntoFairy -> Butterfly test
            cmp ecx, [eax + 24]
            je BUTTERFLY_TEST

            // Not a tracked PC
            jmp DONE

        CHECK_ACTOR:

            READ_N64_REG(SP_OFFSET, eax)
            add eax, [gActiveActorOff]
            IS_ADDR_VALID(eax, ebx, DONE)
            COMPUTE_RAM_ADDR(eax, ebx)

            mov edx, [ebx]

            cmp dx, word ptr[gActiveButterflyID]
            je CHECK_BUTTERFLY

            // check if the actor is a fairy
            cmp dx, word ptr [gActiveFairyID]
            je FAIRY_TEST

            // check if the actor is a big fairy
            cmp dx, word ptr [gActiveBigFairyID]
            jne DONE

        CHECK_BUTTERFLY :

            call ResolveButterflyTransform
            jmp DONE

        FAIRY_TEST:

            // check that the object is "Nothing"
            sub eax, [gActiveActorOff]
            add eax, [gActiveFairyActorCombo]
            COMPUTE_RAM_ADDR(eax, ebx)
            cmp word ptr [ebx + 8], 0x033C
            jne DONE

            push edi

            mov edx, [gData]

            // Get CurrIndex
            mov edi, [edx + 12]

            COMPUTE_INDEX(edx, edi, edi)

            mov[edi], ecx       // Store PC
            mov[edi + 4], eax   // Store Mem

            // Read ComboItemQuery (12 bytes)
            mov eax, [ebx]      // q0
            mov edx, [ebx + 4]  // q1
            mov ecx, [ebx + 8]  // q1

            // Fill the buffer at the correct index
            mov[edi + 8], eax   // q0
            mov[edi + 12], edx  // q1
            mov[edi + 16], ecx  // q2
            mov[edi + 20], 0  // q3
            mov[edi + 24], 0  // q4
            mov[edi + 28], 0  // q5

            INC_INDEX(gData, eax)
            pop edi

            jmp DONE

        ADD_ITEM_TEST:

            // Read the S0 register to retreive the ComboItemQuery address
            READ_N64_REG(S0_OFFSET, eax)
            IS_ADDR_VALID(eax, ebx, DONE)
            COMPUTE_RAM_ADDR(eax, ebx)

            push edi
            
            mov edx, [gData]

            // Get CurrIndex
            mov edi, [edx + 12]

            COMPUTE_INDEX(edx, edi, edi)

            mov[edi], ecx       // Store PC
            mov[edi + 4], eax   // Store Mem

            // Read ComboItemQuery (12 bytes)
            mov eax, [ebx + 4]   // q0
            mov edx, [ebx + 8]   // q1
            
            READ_N64_REG(A1_OFFSET, ecx)

            // Fill the buffer at the correct index
            mov[edi + 8], eax   // q0
            mov[edi + 12], edx  // q1
            mov[edi + 16], ecx  // q2
            mov[edi + 20], 0  // q3
            mov[edi + 24], 0  // q4
            mov[edi + 28], 0  // q5

            INC_INDEX(gData, eax)
            pop edi

            jmp DONE

        DROP_CUSTOM_TEST:

            READ_N64_REG(SP_OFFSET, ebx)
            add ebx, DROP_CUSTOM

            call CaptureXFlagASM
            jmp DONE

        SHOP_TEST:

            // The item is buyable, therefore it is not a "Nothing item", we can exit
            READ_N64_REG(V0_OFFSET, eax)
            cmp eax, 0x02
            jne DONE

            // The item is not a nothing object, we can exit
            READ_N64_REG(V1_OFFSET, eax)
            cmp eax, 0x033C
            jne DONE

            READ_N64_REG(SP_OFFSET, ebx)
            add ebx, SHOP_CUSTOM

            call CaptureXFlagASM
            jmp DONE

        PLAY_INIT_TEST:

            mov [forceGameCheck], 0

            push edi

            mov edx, [gData]

            // Get CurrIndex
            mov edi, [edx + 12]

            COMPUTE_INDEX(edx, edi, edi)

            mov[edi], ecx                        // Store PC

            // Build entrance flag + farore wind state + owl choice ID
            mov ecx, IN_MAGIC
            COMPUTE_RAM_ADDR([gActiveOwlOffset], edx)
            mov cl, byte ptr[edx]
            mov [edi + 4], ecx    // Store Mem = entrance flag + farore wind state + owl choice

            // Spawned scene ID
            READ_N64_REG(V0_OFFSET, eax)

            // Build last played song
            COMPUTE_RAM_ADDR([gActiveSongOffset], edx)
            mov dl, byte ptr[edx]
            and edx, 000000FFh
            shl edx, 8

            // Build current room index
            COMPUTE_RAM_ADDR([gActiveRoomOffset], ebx)
            mov dl, byte ptr[ebx]
            shl edx, 8

            // Build grotto data
            COMPUTE_RAM_ADDR([gActiveGrottoOffset], ebx)
            mov dl, byte ptr[ebx]
            shl edx, 8

            // Build game data
            mov dl, byte ptr[gGame]

            // Entrance spawn ID
            READ_N64_REG(V1_OFFSET, ebx)

            // Fill the buffer at the correct index
            mov[edi + 8], edx   // Message direction + current room + grotto data + Game 
            mov[edi + 12], eax  // Scene ID
            mov[edi + 16], ebx  // Entrance ID

            // Player coordinates data
            COMPUTE_RAM_ADDR([gActiveCoordOffset], ebx)
            mov eax, [ebx]      // X
            mov ecx, [ebx + 4]  // Y
            mov edx, [ebx + 8]  // Z
            mov[edi + 20], eax  // X
            mov[edi + 24], ecx  // Y
            mov[edi + 28], edx  // Z

            INC_INDEX(gData, eax)
            pop edi

            jmp DONE

        TRANSITION_TEST :

            mov[forceGameCheck], 1

            push edi

            mov edx, [gData]

            // Get CurrIndex
            mov edi, [edx + 12]

            COMPUTE_INDEX(edx, edi, edi)

            mov[edi], ecx                        // Store PC

            // Build entrance flag + farore wind state + owl choice ID
            mov ecx, OUT_MAGIC
            COMPUTE_RAM_ADDR([gActiveOwlOffset], edx)
            mov cl, byte ptr[edx]

            // Build farore wind state
            COMPUTE_RAM_ADDR([gActiveFaroreOffset], edx)
            mov edx, [edx]
            cmp edx, FARORE_USED
            jne FARORE_END
            mov edx, 1
            shl edx, 8
            add ecx, edx

        FARORE_END :
            mov [edi + 4], ecx    // Store Mem = entrance flag + farore wind state + owl choice

            // gLastScene = Current scene ID
            //mov ebx, [gActiveEntranceReg]
            //READ_N64_REG(ebx, eax)
            //add eax, [gActiveSceneOffset]
            COMPUTE_RAM_ADDR([gActiveSceneOffset], eax)
            mov eax, [eax]

            // Build last played song
            COMPUTE_RAM_ADDR([gActiveSongOffset], edx)
            mov dl, byte ptr[edx]
            and edx, 000000FFh
            shl edx, 8

            // Build current room index
            COMPUTE_RAM_ADDR([gActiveRoomOffset], ebx)
            mov dl, byte ptr[ebx]
            shl edx, 8

            // Build grotto data
            COMPUTE_RAM_ADDR([gActiveGrottoOffset], ebx)
            mov dl, byte ptr[ebx]
            shl edx, 8

            // Build game data
            mov dl, byte ptr[gGame]

            // gNextEntrance
            COMPUTE_RAM_ADDR([gActiveNextEntrance], ebx)
            mov ebx, [ebx]

            // Fill the buffer at the correct index
            mov[edi + 8], edx   // Last song + curr room index + grotto data + Game
            mov[edi + 12], eax  // Scene ID
            mov[edi + 16], ebx  // Entrance ID

            // Player coordinates data
            COMPUTE_RAM_ADDR([gActiveCoordOffset], ebx)
            mov eax, [ebx]      // X
            mov ecx, [ebx + 4]  // Y
            mov edx, [ebx + 8]  // Z
            mov[edi + 20], eax  // X
            mov[edi + 24], ecx  // Y
            mov[edi + 28], edx  // Z

            INC_INDEX(gData, eax)
            pop edi

        BUTTERFLY_TEST:

            // Handle "Nothing" Butterflies
            mov edx, [regBase]
            mov edx, [edx]
            mov eax, [edx + V1_OFFSET]  // Get the buttlerfly object ID

            // if butterfly item != Nothing -> done
            cmp eax, 0x033C
            jne DONE

            mov ebx, [edx + SP_OFFSET]
            add ebx, BUTTERFLY_CUSTOM
            call CaptureXFlagASM
            jmp DONE

        DONE:
            pop ebx
            pop edx
            pop ecx
            pop eax
            jmp gatewayPC
    }
}

__declspec(naked) void ROMHook()
{
    __asm
    {
        // -------------------------
        // Save context
        // -------------------------
        pushad

        // -------------------------
        // ROM base / gData setup
        // -------------------------
        mov esi, ebx
        mov [romBase], esi
        call CheckGameVersionASM
        mov byte ptr [gGame], GAME_UNKNOWN
        mov byte ptr [gIsRAMLoaded], 0
        
        popad
        jmp gatewayROM // trampoline to original code
    }
}


void InstallPCHook()
{
    InstallHook(HOOK_PC_OFFSET, HOOK_PC_SIZE, (uintptr_t)&PCHook, &gatewayPC, originalBytesPC);
}


void InstallROMHook()
{
    InstallHook(HOOK_ROM_LOAD_OFFSET, HOOK_ROM_LOAD_SIZE, (uintptr_t)&ROMHook, &gatewayROM, originalBytesROM);
}


void InstallHook(size_t HookOffset, size_t HookSize, uintptr_t HookFunction, void ** Gateway, BYTE OriginalBytes[])
{
    uintptr_t target = moduleBase + HookOffset;

    DWORD oldProtect;
    VirtualProtect((LPVOID)target, HookSize, PAGE_EXECUTE_READWRITE, &oldProtect);

    // Save original instructions
    memcpy(OriginalBytes, (void*)target, HookSize);

    // trampoline
    *Gateway = VirtualAlloc(nullptr, 64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    /*for (size_t i = 0; i < HookSize; i++)
    {
        printf("%02X ", OriginalBytes[i]);
    }*/
    memcpy(*Gateway, OriginalBytes, HookSize);

    uintptr_t gatewayEnd = (uintptr_t)*Gateway + HookSize;
    uintptr_t returnAddr = target + HookSize;

    *(BYTE*)(gatewayEnd) = 0xE9;
    *(uintptr_t*)(gatewayEnd + 1) = returnAddr - gatewayEnd - 5;

    // Hook instruction
    uintptr_t rel = (uintptr_t)HookFunction - target - 5;

    *(BYTE*)target = 0xE9;              // jmp
    *(uintptr_t*)(target + 1) = rel;

    for (size_t i = 5; i < HookSize; i++)
        *(BYTE*)(target + i) = 0x90;

    VirtualProtect((LPVOID)target, HookSize, oldProtect, &oldProtect);
}