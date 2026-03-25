#include "pch.h"
#include "Hooking.h"
#include "PatternScanner.h"

alignas(64)
uint8_t typemask[2][PC_RANGE_SIZE]; // One type mask per game

// Hooking installation
void* gatewayPC = NULL;
void* gatewayROM = NULL;
BYTE originalBytesPC[HOOK_PC_SIZE];
BYTE originalBytesROM[HOOK_ROM_LOAD_SIZE];
//uintptr_t returnAddress = 0;

// Game data / shared memeory data
SharedData* gData = nullptr;
GameID gGame = GAME_OOT;
bool gIsRAMLoaded = false;
uint64_t gGameVersion = 0;
uintptr_t moduleBase = 0;
uintptr_t regBase = 0;
uintptr_t romBase = 0;
uintptr_t gameRAMBase = 0;
uintptr_t currButterflyPC = 0;
uint32_t gSP = 0;
uint32_t gA1 = 0;
uint32_t gV0 = 0;
uint32_t gV1 = 0;
uint32_t gS0 = 0;


__forceinline bool IsValidAddr(uint32_t Addr)
{
    return (Addr & 0xFF000000) == 0x80000000;
}

__forceinline uint64_t GetGameVersion()
{
    if (!gData)
    {
        return 0;
    }

    uint64_t version = ((uint64_t)gData->GameVersion[0] << 32) | gData->GameVersion[1];

    if (version == 0)
    {
        version = *(uint64_t*)(romBase + 0x10);
        gData->GameVersion[0] = (uint32_t)version;
        gData->GameVersion[1] = version >> 32;
        gGameVersion = version;

        LOG("Game Version = %llu", version);
    }
    
    return version;
}


__forceinline GameID DetectCurrentGame()
{
    // OoT
    uintptr_t AddrCheck = gameRAMBase + (0x8011A5EC & 0x00FFFFFF);
    uint64_t val = *(uint64_t*)AddrCheck & 0xFFFF0000FFFFFFFF;

    if (val == 0x415A00005A454C44)
    {
        return GAME_OOT;
    }

    // MM
    AddrCheck = gameRAMBase + (0x801EF694 & 0x00FFFFFF);
    val = *(uint64_t*)AddrCheck & 0xFFFF0000FFFFFFFF;
    
    if (val == 0x413300005A454C44)
    {
        return GAME_MM;
    }

    return GAME_UNKNOWN;
}


__forceinline void PeriodicGameCheck()
{
    static uint32_t counter;

    counter++;

    if ((counter & 0x1FFF) != 0)
    {
        return;
    }

    counter = 0;

    GameID g = DetectCurrentGame();

    if (g != gGame && g != GAME_UNKNOWN)
    {
        LOG("Game changed");

        gGame = g;
        gIsRAMLoaded = false;
    }
}


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


__forceinline void StoreResults(uint32_t PC, uint32_t Mem, uint32_t Buf1, uint32_t Buf2, uint32_t Buf3)
{
    gData->Buffer[gData->CurrIndex].PC = PC;
    gData->Buffer[gData->CurrIndex].Mem = Mem;
    gData->Buffer[gData->CurrIndex].Query[0] = Buf1;
    gData->Buffer[gData->CurrIndex].Query[1] = Buf2;
    gData->Buffer[gData->CurrIndex].Query[2] = Buf3;

    gData->CurrIndex = (gData->CurrIndex + 1) & (BUFFER_SIZE - 1);
}


__forceinline void CaptureXFlag(uintptr_t PC, uintptr_t Mem)
{
    if (IsValidAddr(Mem))
    {
        uintptr_t addr = gameRAMBase + (Mem & 0x00FFFFFF);
        StoreResults(PC, Mem, *(uint32_t*)(addr), *(uint32_t*)(addr + 4), 0xFFFF0000 + gGame);
    }
}

/*
__forceinline void HandlePCHook(uint32_t PC)
{
    if (gGame != GAME_UNKNOWN)
    {
        PeriodicGameCheck();

        if (gIsRAMLoaded)
        {   // The RAM is loaded

            if (currButterflyPC != 0 && currButterflyPC == PC)
            {   // Use buttefly

                currButterflyPC = 0;
                uint32_t base = *(uint32_t*)regBase;
                gV1 = *(uint32_t*)(base + V1_OFFSET);

                LOG("Addr = 0x%08X, V1 = 0x%08X", base + V1_OFFSET, gV1);

                if (gV1 == 0x033C)
                {
                    gSP = *(uint32_t*)(base + SP_OFFSET) + BUTTERFLY_CUSTOM;
                    LOG("Addr = 0x%08X, SP = 0x%08X", base + SP_OFFSET, gSP);
                    CaptureXFlag(PC, gSP);
                }
            }
            else
            {
                PCType type = (PCType)typemask[gGame][PC - PC_RANGE_START];

                if (type == TYPE_NONE)
                {   // Done outside the switch as it represent 99% of the PCs

                    return;
                }

                uintptr_t addr = gameRAMBase;
                uint32_t base = *(uint32_t*)regBase;
                switch (type)
                {
                    // Butterfly uses Actor_RunUpdate function which occurs a lot compare to the others, need to be put first to optimize
                    case TYPE_BUTTERFLY:
                    {    // read S0...
                        gS0 = *(uint32_t*)(base + S0_OFFSET);

                        if (IsValidAddr(gS0))
                        {
                            addr += gS0 & 0x00FFFFFF;
                            uint32_t tmp = *(uint32_t*)addr & 0xFFFF0000;

                            if (gGame == GAME_OOT && tmp == 0x001E0000)
                            {   // This is a butterfly

                                LOG("Addr = 0x%08X, S0 = 0x%08X", base + S0_OFFSET, gS0);
                                currButterflyPC = *(uint32_t*)(addr + BUTTERFLY_FUNCTION) + BUTTERFLY_SPAWN_OFFSET;
                                LOG("Addr = 0x%08X, CurrButterFlyPC = 0x%08X", addr + BUTTERFLY_FUNCTION + BUTTERFLY_SPAWN_OFFSET, currButterflyPC);
                            }
                            else if (gGame == GAME_MM && tmp == 0x00150000)
                            {
                                LOG("Addr = 0x%08X, S0 = 0x%08X", base + S0_OFFSET, gS0);
                                currButterflyPC = *(uint32_t*)(addr + BUTTERFLY_FUNCTION + 0x08) + BUTTERFLY_SPAWN_OFFSET;
                                LOG("Addr = 0x%08X, CurrButterFlyPC = 0x%08X", addr + BUTTERFLY_FUNCTION + BUTTERFLY_SPAWN_OFFSET + 0x08, currButterflyPC);
                            }

                        }

                        break;
                    }

                    case TYPE_COMBO:
                    {    // read A1, S0, etc...

                        gS0 = *(uint32_t*)(base + S0_OFFSET);

                        LOG("Addr = 0x%08X, S0 = 0x%08X", base + S0_OFFSET, gS0);

                        if (IsValidAddr(gS0))
                        {
                            gA1 = *(uint32_t*)(base + A1_OFFSET);
                            LOG("Addr = 0x%08X, A1 = 0x%08X", base + A1_OFFSET, gA1);
                            addr += (gS0 & 0x00FFFFFF);
                            StoreResults(PC, gS0, *(uint32_t*)(addr + 4), *(uint32_t*)(addr + 8), gA1);
                        }

                        return;
                    }
                    case TYPE_XFLAG:
                    {    // read SP...
                        
                        gSP = *(uint32_t*)(base + SP_OFFSET) + DROP_CUSTOM;

                        LOG("Addr = 0x%08X, SP = 0x%08X", base + SP_OFFSET, gSP);

                        CaptureXFlag(PC, gSP);

                        return;
                    }
                    case TYPE_SHOP:
                    {    // read V0, V1, SP...
                        gV0 = *(uint32_t*)(base + V0_OFFSET);
                        gV1 = *(uint32_t*)(base + V1_OFFSET);

                        LOG("Addr = 0x%08X, V0 = 0x%08X, Addr = 0x%08X, V1 = 0x%08X", base + V0_OFFSET, gV0, base + V1_OFFSET, gV1);

                        if (gV0 == 2 && gV1 == 0x033C)
                        {
                            gSP = *(uint32_t*)(base + SP_OFFSET) + SHOP_CUSTOM;
                            LOG("Addr = 0x%08X, SP = 0x%08X", base + SP_OFFSET, gSP);
                            CaptureXFlag(PC, gSP);
                        }

                        return;
                    }
                    
                    default:
                    {
                        return;
                    }
                }
            }
        }
        else if ((gGame == GAME_OOT && PC == OOT_PLAY_MAIN) || (gGame == GAME_MM && PC == MM_PLAY_MAIN))
        {   // The RAM is loaded. We can check for patterns

            gIsRAMLoaded = true;

            LOG("Game = %d, RAM Loaded : 0x%08X", gGame, PC);

            gGameVersion = GetGameVersion();

            GamePatternState* state = &gPatternState[gGame];

            if (gGameVersion != gPatternState[gGame].Version)
            {
                gPatternState[GAME_OOT].Resolved = false;
                gPatternState[GAME_MM].Resolved = false;
            }

            if (!state->Resolved || state->Version != gGameVersion)
            {
                gPatternState[GAME_OOT].Version = gGameVersion;
                gPatternState[GAME_MM].Version = gGameVersion;
                BuildTypeMaskFromPatterns();
            }
        }
    }
    else
    {
        gGame = DetectCurrentGame();
        if (gGame != GAME_UNKNOWN)
        {   // The RAM is loaded so is the ROM

            gIsRAMLoaded = false;
            gGameVersion = GetGameVersion();
            if (gPatternState[GAME_OOT].Version != gGameVersion)
            {
                gPatternState[GAME_OOT].Resolved = false;
                gPatternState[GAME_MM].Resolved = false;
            }
            gPatternState[GAME_OOT].Version = gGameVersion;
            gPatternState[GAME_MM].Version = gGameVersion;
        }
    }
}

__declspec(naked) void PCHook()
{
    __asm
    {
        push eax
        push ecx

        mov eax, regBase
        mov eax, [eax]
        mov ecx, [eax + PC_OFFSET]

        push ecx
        call HandlePCHook
        add esp, 4

        pop ecx
        pop eax

        jmp gatewayPC
    }
}*/

__forceinline void HandlePCHookRare(uint32_t PC)
{
    if (gGame == GAME_UNKNOWN)
    {
        gGame = DetectCurrentGame();
        if (gGame != GAME_UNKNOWN)
        {
            gIsRAMLoaded = false;
            gGameVersion = GetGameVersion();
            gPatternState[GAME_OOT].Resolved = false;
            gPatternState[GAME_MM].Resolved = false;
            gPatternState[GAME_OOT].Version = gGameVersion;
            gPatternState[GAME_MM].Version = gGameVersion;
        }
        return;
    }


    PCType type = (PCType)typemask[gGame][PC - PC_RANGE_START];
    if (type == TYPE_NONE)
    {
        PeriodicGameCheck();
        return;
    }

    uintptr_t base = *(uint32_t*)regBase;
    uintptr_t addr = gameRAMBase;

    switch (type)
    {
        case TYPE_BUTTERFLY:
        {
            gS0 = *(uint32_t*)(base + S0_OFFSET);

            if (IsValidAddr(gS0))
            {
                addr += gS0 & 0x00FFFFFF;
                uint32_t tmp = *(uint32_t*)addr & 0xFFFF0000;

                if (gGame == GAME_OOT && tmp == 0x001E0000)
                {   // This is a butterfly

                    LOG("Addr = 0x%08X, S0 = 0x%08X", base + S0_OFFSET, gS0);
                    currButterflyPC = *(uint32_t*)(addr + BUTTERFLY_FUNCTION) + BUTTERFLY_SPAWN_OFFSET;
                    LOG("Addr = 0x%08X, CurrButterFlyPC = 0x%08X", addr + BUTTERFLY_FUNCTION + BUTTERFLY_SPAWN_OFFSET, currButterflyPC);
                }
                else if (gGame == GAME_MM && tmp == 0x00150000)
                {
                    LOG("Addr = 0x%08X, S0 = 0x%08X", base + S0_OFFSET, gS0);
                    currButterflyPC = *(uint32_t*)(addr + BUTTERFLY_FUNCTION + 0x08) + BUTTERFLY_SPAWN_OFFSET;
                    LOG("Addr = 0x%08X, CurrButterFlyPC = 0x%08X", addr + BUTTERFLY_FUNCTION + BUTTERFLY_SPAWN_OFFSET + 0x08, currButterflyPC);
                }

            }

            break;
        }

        case TYPE_COMBO:
        {
            gS0 = *(uint32_t*)(base + S0_OFFSET);
            if (IsValidAddr(gS0))
            {
                gA1 = *(uint32_t*)(base + A1_OFFSET);
                addr += gS0 & 0x00FFFFFF;
                StoreResults(PC, gS0, *(uint32_t*)(addr + 4), *(uint32_t*)(addr + 8), gA1);
            }
            break;
        }
        case TYPE_XFLAG:
        {
            gSP = *(uint32_t*)(base + SP_OFFSET) + DROP_CUSTOM;
            CaptureXFlag(PC, gSP);
            break;
        }
        case TYPE_SHOP:
        {
            gV0 = *(uint32_t*)(base + V0_OFFSET);
            gV1 = *(uint32_t*)(base + V1_OFFSET);
            if (gV0 == 2 && gV1 == 0x033C)
            {
                gSP = *(uint32_t*)(base + SP_OFFSET) + SHOP_CUSTOM;
                CaptureXFlag(PC, gSP);
            }
            break;
        }
        default:
            break;
    }
    PeriodicGameCheck();
}



__declspec(naked) void CaptureXFlagASM()
{
    __asm
    {
        mov eax, ebx
        and eax, 0xFF000000
        cmp eax, 0x80000000
        jne Done

        push edi
        push ecx; PC
        push ebx; SP

        mov eax, gData

        // Get CurrIndex
        mov edx, [eax + 12]

        // Compute slot = idx * 20
        mov ebx, edx
        shl ebx, 4          // idx * 16
        mov ecx, edx
        shl ecx, 2          // idx * 4
        add ebx, ecx        // idx * 20
        lea edi, [eax + 16 + ebx]

        pop ebx

        mov eax, ebx
        and eax, 0x00FFFFFF

        //push esi

        mov edx, gameRAMBase    // The real game RAM base address 
        add edx, eax            // Add the offset to the game RAM base address

        pop ecx

        // Fill the buffer at the correct index
        mov[edi], ecx             // Store PC
        mov[edi + 4],  ebx        // Store Mem
        mov eax, [edx]
        mov[edi + 8],  eax     // Key
        mov eax, [edx + 4]
        mov[edi + 12], eax  // GI

        movzx eax, byte ptr[gGame]        // The game the XFlag comes from 
        add eax, 0xFFFF0000       // IsConsumed

        mov[edi + 16], eax  // q2

        // Increment CurrIndex
        mov eax, gData
        mov ecx, [eax + 12]
        inc ecx
        and ecx, BUFFER_SIZE - 1
        mov[eax + 12], ecx

        pop edi

        Done:
            ret
    }
}


__declspec(naked) void PCHook()
{
    __asm
    {
        // == = Préparation == =
        push eax
        push ecx
        push edx
        push ebx

        // --- Lire PC-- -
        mov ecx, [esi] // ; ecx = PC

        // --- Test hot path butterfly-- -
        mov eax, [currButterflyPC]
        test eax, eax
        je CheckIsRAMLoaded // ; si currButterflyPC = 0, passe aux autres tests
        cmp eax, ecx
        jne CheckIsRAMLoaded

        // --- Butterfly fast path-- -
        //mov eax, [currButterflyPC]  // ; lire l'ancienne valeur (si besoin)
        //xor eax, eax    //; mettre à 0
        mov[currButterflyPC], 0   //; écrire 0
        mov edx, regBase
        mov edx, [edx]
        mov eax, [edx + V1_OFFSET]
        cmp eax, 0x033C
        jne CheckIsRAMLoaded

        mov ebx, [edx + SP_OFFSET]
        add ebx, BUTTERFLY_CUSTOM
        //push ecx; PC
        //push ebx; SP
        call CaptureXFlagASM
        nop
        //add esp, 8
        jmp Done

        CheckIsRAMLoaded :
        // Vérifier si RAM est chargée
            cmp gIsRAMLoaded, 0
            jne CallRarePath

            // Test si Play_Main(RAM loaded)
            mov eax, ecx
            cmp gGame, GAME_OOT
            jne CheckMM
            cmp eax, OOT_PLAY_MAIN
            je LoadRAM
            jmp CallRarePath

        CheckMM :
            cmp gGame, GAME_MM
            jne CallRarePath
            cmp eax, MM_PLAY_MAIN
            je LoadRAM
            jmp CallRarePath

        LoadRAM :
            mov gIsRAMLoaded, 1

            // Forcer résolution des patterns si nécessaire
            movzx edx, byte ptr[gGame]              //; gGame dans edx
            imul edx, edx, 16 //; edx = gGame * taille structure
            lea eax, [gPatternState]    //; base de gPatternState
            add eax, edx                //; eax = &gPatternState[gGame]

            cmp [eax + 8], 1  // gPatternState[gGame].resolved
            jne ResetPatterns

            // Test game version
            push ecx
            call GetGameVersion //; gGameVersion update
            add esp, 4


            // maintenant eax pointe sur GamePatternState[gGame]
            mov edx, [eax] //; bas 32 bits
            cmp edx, dword ptr[gGameVersion] //; compare bas 32 bits
            jne ResetPatterns
            mov edx, [eax + 4] // haut 32 bits
            cmp edx, dword ptr[gGameVersion + 4] // compare haut 32 bits
            jne ResetPatterns
            jmp CallRarePath

        ResetPatterns :
            // gPatternState[GAME_OOT].Resolved = 0
            mov byte ptr[gPatternState + 0x08], 0
            // gPatternState[GAME_MM].Resolved = 0
            mov byte ptr[gPatternState + 0x18], 0
            push ecx
            push gGame
            call BuildTypeMaskFromPatterns
            add esp, 4
            pop ecx
            //jmp CallRarePath //Call RarePath anyway

        CallRarePath :
        // --- Appel C++ pour TYPE_NONE et cases rares-- -
            push ecx; push PC
            call HandlePCHookRare
            add esp, 4

                Done:
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
        mov romBase, esi
        mov edi, gData
        mov eax, [esi]
        mov ecx, [esi + 4]
        mov[edi], eax
        mov[edi + 4], ecx
        mov gGame, GAME_UNKNOWN
        mov gIsRAMLoaded, 0
        
        popad
        jmp gatewayROM // trampoline to original code
    }
}
/*
__declspec(naked) void PCHook()
{
    __asm
    {
        // =========================
        // Save context. ! Important: do not save flags with pushfd otherwise it will crash !
        // =========================
        pushad

        // =========================
        // Get CPU struct
        // =========================
        mov eax, regBase
        mov eax, [eax]

        // =========================
        // Get PC / SP
        // =========================
        mov ecx, [eax + PC_OFFSET]   // PC
        mov edx, [eax + A1_OFFSET]   // A1
        mov esi, [eax + SP_OFFSET]   // SP
        mov edi, ecx                 // Save PC value in edi
        mov ebp, edx                 // Save A1 value in ebp
        mov gSP, esi                 // Save SP
        mov esi, [eax + V0_OFFSET]   // V0
        mov gV0, esi                 // Save V0
        mov esi, [eax + V1_OFFSET]   // V1
        mov gV1, esi                 // Save V1
        mov esi, [eax + S0_OFFSET]   // S0
        mov gS0, esi                 // Save S0

        // We need to do this before the type mask as the butterfly PC mask will always be TYPE_NONE as the PC is gathered on the fly
        cmp currButterflyPC, edi     // Use V0, V1 and SP register
        je USE_BUTTERFLY

        // =========================
        // Compute index
        // =========================
        mov eax, edi
        and eax, 0x00FFFFFF

        // =========================
        // Load typemask entry
        // =========================
        mov eax, edi
        and eax, 0x00FFFFFF

        mov esi, offset typemask
        mov bl, [esi + eax]

        cmp bl, TYPE_NONE            // Not tracked
        je EXIT

        cmp bl, TYPE_COMBO           // Use A1 register
        je USE_A1

        cmp bl, TYPE_XFLAG           // Use SP register
        je USE_SP

        cmp bl, TYPE_SHOP            // Use V0, V1 and SP register 
        je USE_SHOP

        cmp bl, TYPE_BUTTERFLY       // Use S0 register 
        je CHECK_BUTTERFLY

        jmp EXIT

        // =========================
        // USE A1 (ComboItemQuery)
        // =========================
        USE_A1:

            // =========================
            // Check A1 Range : Between 0x80000000 - 0x80FFFFFF
            // =========================
            mov eax, ebp
            and eax, 0xFF000000
            cmp eax, 0x80000000
            jne EXIT

            // =========================
            // Convert A1 to RAM offset
            // =========================
            mov eax, ebp
            and eax, 0x00FFFFFF

            mov esi, gameRAMBase    // The real game RAM base address 
            add esi, eax            // Add the offset to the game RAM base address

            // Read ComboItemQuery (12 bytes)
            mov eax, [esi + 4]   // q0
            mov edx, [esi + 8]   // q1
            mov ebx, [esi + 12]  // q2

            jmp STORE
            
        // =========================
        // USE SP (Xflag)
        // =========================
        USE_SP:

            mov ebp, gSP
            add ebp, DROP_CUSTOM

            // =========================
            // Check SP Range : Between 0x80000000 - 0x80FFFFFF
            // =========================
            mov eax, ebp
            and eax, 0xFF000000
            cmp eax, 0x80000000
            jne EXIT

            jmp CAPTURE_XFLAG

        // =========================
        // USE Shop (Xflag)
        // =========================
        USE_SHOP:

            // The item is buyable, therefore it is not a "Nothing item", we can exit
            mov eax, gV0
            cmp eax, 0x02
            jne EXIT

            // The item is not a nothing object, we can exit
            mov eax, gV1
            cmp eax, 0x033C
            jne EXIT

            mov ebp, gSP
            add ebp, SHOP_CUSTOM

            // =========================
            // Check SP Range : Between 0x80000000 - 0x80FFFFFF
            // =========================
            mov eax, ebp
            and eax, 0xFF000000
            cmp eax, 0x80000000
            jne EXIT

            jmp CAPTURE_XFLAG


        // =========================
        // Check Butterfly
        // =========================
        CHECK_BUTTERFLY:

            // =========================
            // Check S0 Range : Between 0x80000000 - 0x80FFFFFF
            // =========================
            mov ebp, gS0
            and ebp, 0xFF000000
            cmp ebp, 0x80000000
            jne EXIT

            // =========================
            // Convert S0 to RAM offset
            // =========================
            mov ebp, gS0
            and ebp, 0x00FFFFFF

            mov esi, gameRAMBase    // The real game RAM base address 
            add esi, ebp            // Add the offset to the game RAM base address

            // Check if the actor is a butterfly
            mov eax, [esi]
            and eax, 0xFFFF0000
            cmp eax, 0x001E0000     // OoT
            je MATCH_BUTTERFLY

            cmp eax, 0x00150000     // MM
            je MATCH_BUTTERFLY_MM

            jmp EXIT                // Not a butterfly

        MATCH_BUTTERFLY_MM:
            mov gGame, GAME_MM
            add esi, 0x08           // The MM actor structure has 8 bytes more than the OoT one

        MATCH_BUTTERFLY:

            // Set the butterfly PC
            mov eax, [esi + BUTTERFLY_FUNCTION]
            add eax, BUTTERFLY_SPAWN_OFFSET
            mov currButterflyPC, eax

            jmp EXIT

        // =========================
        // USE Butterfly (Xflag)
        // =========================
        USE_BUTTERFLY:
            // Reset the butterfly PC
            mov currButterflyPC, 0

            // The item is not a nothing object, we can exit
            mov eax, gV1
            cmp eax, 0x033C
            jne EXIT

            mov ebp, gSP
            add ebp, BUTTERFLY_CUSTOM

            // =========================
            // Check SP Range : Between 0x80000000 - 0x80FFFFFF
            // =========================
            mov eax, ebp
            and eax, 0xFF000000
            cmp eax, 0x80000000
            jne EXIT

            jmp CAPTURE_XFLAG

        // =========================
        // CAPTURE XFLAG (COMMON)
        // =========================
        CAPTURE_XFLAG:
            
            mov gGame, GAME_OOT

            // ========================
            // Check Game Version
            // ========================
            mov eax, 0x801EF694     // MM gPlayerState
            and eax, 0x00FFFFFF

            mov esi, gameRAMBase
            add esi, eax

            // Check "DLEZ"
            mov eax, [esi]
            cmp eax, 0x5A454C44
            jne CAPTURE

            // Check "3A"
            mov eax, [esi + 4]
            and eax, 0xFFFF0000     // Get ride of the death count
            cmp eax, 0x41330000
            jne CAPTURE

            mov gGame, GAME_MM

        CAPTURE:

            // =========================
            // Convert SP to RAM offset
            // =========================
            mov eax, ebp
            and eax, 0x00FFFFFF
            
            mov esi, gameRAMBase    // The real game RAM base address 
            add esi, eax            // Add the offset to the game RAM base address
            
            mov eax, [esi]          // Key
            mov edx, [esi + 4]      // GI
            mov ebx, 0xFFFF0000     // IsConsumed.
            add bx, gGame          // The game the XFlag comes from 
            
            jmp STORE


        // =========================
        // STORE (COMMON)
        // =========================
        STORE:
            mov ecx, edi // Restore PC value

            // Get gData
            mov esi, gData

            // Test if NULL
            test esi, esi
            jz EXIT

            // Temp Save q values
            push eax
            push edx
            push ebx

            // Get CurrIndex
            mov ebx, [esi + 4]

            // Compute slot = idx * 20
            mov eax, ebx
            shl eax, 4                  // 16
            mov edx, ebx
            shl edx, 2                  // 4
            add eax, edx                // eax = idx * 20
            lea edi, [esi + 16 + eax]

            // Restore q values
            pop ebx
            pop edx
            pop eax

            // Fill the buffer at the correct index
            mov[edi], ecx       // Store PC
            mov[edi + 4], ebp   // Store Mem
            mov[edi + 8], eax   // q0
            mov[edi + 12], edx  // q1
            mov[edi + 16], ebx  // q2

            // Increment CurrIndex
            mov ebx, [esi + 4]
            inc ebx
            and ebx, BUFFER_SIZE - 1
            mov[esi + 4], ebx

        EXIT :

            // Restore saved context
            popad

            // Get back to the orignal PJ execution flow
            jmp gateway
    }
}
*/

void InstallPCHook()
{
    InstallHook(HOOK_PC_OFFSET, HOOK_PC_SIZE, (uintptr_t)&PCHook, &gatewayPC, originalBytesPC);
}


void InstallROMHook()
{
    InstallHook(HOOK_ROM_LOAD_OFFSET, HOOK_ROM_LOAD_SIZE, (uintptr_t)&ROMHook, &gatewayROM, originalBytesROM);
}


void InstallHook(size_t HookOffset, size_t HookSize, uintptr_t HookFunction, void ** gateway, BYTE originalBytes[])
{
    uintptr_t target = moduleBase + HookOffset;

    DWORD oldProtect;
    VirtualProtect((LPVOID)target, HookSize, PAGE_EXECUTE_READWRITE, &oldProtect);

    // sauvegarde instructions originales
    memcpy(originalBytes, (void*)target, HookSize);

    // trampoline
    *gateway = VirtualAlloc(nullptr, 64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    /*for (size_t i = 0; i < HookSize; i++)
    {
        printf("%02X ", originalBytes[i]);
    }*/
    memcpy(*gateway, originalBytes, HookSize);

    uintptr_t gatewayEnd = (uintptr_t)*gateway + HookSize;
    uintptr_t returnAddr = target + HookSize;

    *(BYTE*)(gatewayEnd) = 0xE9;
    *(uintptr_t*)(gatewayEnd + 1) = returnAddr - gatewayEnd - 5;

    // hook
    uintptr_t rel = (uintptr_t)HookFunction - target - 5;

    *(BYTE*)target = 0xE9;
    *(uintptr_t*)(target + 1) = rel;

    for (size_t i = 5; i < HookSize; i++)
        *(BYTE*)(target + i) = 0x90;

    VirtualProtect((LPVOID)target, HookSize, oldProtect, &oldProtect);
}


void InstallHooks()
{
}