#include "Combo/Entrances.h"
#include "Combo/OoTEntrances.h"
#include "Combo/MMEntrances.h"
#include "Multi/Multi.h"

EntranceHelper::EntranceHelper()
{
    this->ResetEntranceHelper();
}


EntranceHelper::~EntranceHelper()
{
    this->ResetEntranceHelper();
}


void EntranceHelper::ResetEntranceHelper()
{
    this->EntranceID = 0;
    this->LastTouchedEntranceID = 0;
    this->IsEntranceTouched = false;
}


void EntranceHelper::ReadEntranceID(int Game, uint8_t * RAMData)
{
    uint32_t tmpLast = 0;
    uint8_t transitionTrigger = 0;

    if (Game == OOT_GAME)
    {   // Read entrance ID for OoT

        if (std::endian::native == std::endian::little)
        {
            transitionTrigger = RAMData[0x1DA2B6];    // Transition trigger = 0x801DA2B6
        }
        else
        {
            transitionTrigger = RAMData[0x1DA2B5];    // Transition trigger = 0x801DA2B5
        }

        switch (transitionTrigger)
        {
            case 0x14:
            {   // Touched loading zone

                memcpy(&tmpLast, &RAMData[0x1DA2B8], sizeof(uint32_t)); // Next entrance ID = 0x801DA2B8
                this->IsEntranceTouched = true;
                this->LastGameTouchedEntrance = Game;
                if (tmpLast != this->LastTouchedEntranceID)
                {
                    this->LastTouchedEntranceID = tmpLast;
                    if (this->IsGrottoExit())
                    {
                        this->LastTouchedEntranceID = this->GetGrottoExit(Game, RAMData);
                    }
                    MultiLogger::LogMessage("Loading zone 0x%X touched !", this->LastTouchedEntranceID);
                }
                break;
            }

            case 0xEC:
            {   // Zone loaded

                if (this->IsEntranceTouched && this->LastGameTouchedEntrance == Game)
                {
                    this->IsEntranceTouched = false;
                    memcpy(&this->EntranceID, &RAMData[0x1DA2B8], sizeof(uint32_t));    // Next entrance ID = 0x801DA2B8
                    if (this->IsGrottoEntrance())
                    {
                        this->EntranceID = this->GetGrottoEntrance(Game, RAMData);
                    }
                    MultiLogger::LogMessage("Scene Loaded ! Entrance ID : 0x%X, Last loading zone ID = 0x%X", this->EntranceID, this->LastTouchedEntranceID);
                }
                break;
            }

            default:
            {

                if (this->LastGameTouchedEntrance != Game && this->IsEntranceTouched)
                {   // We have switched game with the last touched entrance

                    memcpy(&this->EntranceID, &RAMData[0x441A6C], sizeof(uint32_t));   // gLastEntrance = 0x80441A6C
                    if (this->EntranceID == 0)
                    {   // The RAM is not fully loaded. TODO Deku Tree ENTRANCE is equal to 0 need to take it into account

                        return;  // We return in order to get it on the next loop
                    }
                    this->LastGameTouchedEntrance = Game;
                    this->IsEntranceTouched = false;
                    if (this->IsGrottoEntrance())
                    {
                        this->EntranceID = this->GetGrottoEntrance(Game, RAMData);
                    }
                    MultiLogger::LogMessage("Scene Loaded ! Entrance ID : 0x%X, Last loading zone ID = 0x%X", this->EntranceID, this->LastTouchedEntranceID);
                }
                else
                {   // We are on the same game as the last touched entrance

                    memcpy(&this->EntranceID, &RAMData[0x11A5D0], sizeof(uint32_t));   // gSaveContext.entrance = 0x8011A5D0
                }
                //memcpy(&this->LastTouchedEntranceID, &this->RAMData[0x1DA2B8], sizeof(uint32_t));
                //MultiLogger::LogMessage("Entrance ID : 0x%X, Last Entrance ID = 0x%X", this->EntranceID, this->LastTouchedEntranceID);
                break;
            }
        }
    }
    else
    {   // Read entrance ID for MM

        if (std::endian::native == std::endian::little)
        {
            transitionTrigger = RAMData[0x3FF396];    // Transition trigger = 0x803FF396
        }
        else
        {
            transitionTrigger = RAMData[0x3FF395];    // Transition trigger = 0x803FF395
        }

        switch (transitionTrigger)
        {
            case 0x14:
            {   // Touched loading zone

                memcpy(&tmpLast, &RAMData[0x3FF398], sizeof(uint32_t)); // playState.nextEntrance = 0x803FF39A8
                this->IsEntranceTouched = true;
                this->LastGameTouchedEntrance = Game;
                if (tmpLast != this->LastTouchedEntranceID)
                {
                    this->LastTouchedEntranceID = tmpLast;
                    if (this->IsGrottoExit())
                    {
                        this->LastTouchedEntranceID = this->GetGrottoExit(Game, RAMData);
                    }
                    MultiLogger::LogMessage("Loading zone 0x%X touched !", this->LastTouchedEntranceID);
                }
                break;
            }

            case 0xEC:
            {   // Zone loaded. On MM next entrance ID is always 0 when the transition trigger is 0xEC

                if (this->IsEntranceTouched && this->LastGameTouchedEntrance == Game)
                {
                    this->IsEntranceTouched = false;
                    memcpy(&this->EntranceID, &RAMData[0x1EF670], sizeof(uint32_t)); // gSaveContext.entrance = 0x801EF670
                    if (this->IsGrottoEntrance())
                    {
                        this->EntranceID = this->GetGrottoEntrance(Game, RAMData);
                    }
                    MultiLogger::LogMessage("Scene Loaded ! Entrance ID : 0x%X, Last loading zone ID = 0x%X", this->EntranceID, this->LastTouchedEntranceID);
                }
                break;
            }

            default:
            {
                if (this->LastGameTouchedEntrance != Game && this->IsEntranceTouched)
                {   // We have switched game with the last touched entrance

                    memcpy(&this->EntranceID, &RAMData[0x770EC4], sizeof(uint32_t));   // gLastEntrance = 0x80770EC4
                    if (this->EntranceID == 0)
                    {   // The RAM is not fully loaded

                        return;  // We return in order to get it on the next loop
                    }
                    this->LastGameTouchedEntrance = Game;
                    this->IsEntranceTouched = false;
                    if (this->IsGrottoEntrance())
                    {
                        this->EntranceID = this->GetGrottoEntrance(Game, RAMData);
                    }
                    MultiLogger::LogMessage("Scene Loaded ! Entrance ID : 0x%X, Last loading zone ID = 0x%X", this->EntranceID, this->LastTouchedEntranceID);
                }
                else
                {   // We are on the same game as the last touched entrance
                    
                    memcpy(&this->EntranceID, &RAMData[0x1EF670], sizeof(uint32_t));   // gSaveContext.entrance = 0x801EF670
                }
                //memcpy(&this->LastTouchedEntranceID, &this->RAMData[0x1DA2B8], sizeof(uint32_t));
                //MultiLogger::LogMessage("Entrance ID : 0x%X, Last Entrance ID = 0x%X", this->EntranceID, this->LastTouchedEntranceID);
                //MultiLogger::LogMessage("Entrance ID : 0x%X", RAMData[0x3FF394]);
                break;
            }
        }
        //memcpy(&this->EntranceID, &RAMData[0x1EF670], sizeof(uint32_t));            // gSaveContext start = 0x1EF670
        //ReadProcessMemory(this->PJ64Handle, (LPCVOID)(this->GameRamBaseAddress + 0x1EF670), &this->EntranceID, sizeof(this->EntranceID), 0);
    }
}


bool EntranceHelper::IsGrottoEntrance()
{
    switch (this->EntranceID)
    {
        case 0x03f:     // Generic grotto entry
        case 0x36d:     // Fairy grotto entry
        case 0x5bc:     // Double scrub grotto entry
        case 0x5a4:     // Triple scrub grotto entry
        {
            return true;
        }

        default:
        {
            return false;
        }
    }
}


bool EntranceHelper::IsGrottoExit()
{
    switch (this->LastTouchedEntranceID)
    {
        case 0x7fff:    // Any grotto exit touched (OoT)
        case 0xffff:    // Any grotto exit touched (MM)
        {   
            return true;
        }

        default:
        {
            return false;
        }
    }
}



uint32_t EntranceHelper::GetGrottoEntrance(int Game, uint8_t* RAMData)
{
    uint8_t currRoomNum = 0;
    uint8_t gGrottoData = 0;
    uint32_t gLastScene = 0;

    if (Game == OOT_GAME)
    {
        if (std::endian::native == std::endian::little)
        {
            currRoomNum = RAMData[0x1DA15F];    // Current room number = 0x801DA15D
        }
        else
        {
            currRoomNum = RAMData[0x1DA15C];    // Current room number = 0x801DA15C
        }

        switch (currRoomNum)
        {
            case 0x00:
            {
                if (std::endian::native == std::endian::little)
                {
                    gGrottoData = RAMData[0x11B964];    // Current room number = 0x8011B964
                }
                else
                {
                    gGrottoData = RAMData[0x11B967];    // Current room number = 0x8011B967
                }

                switch (gGrottoData & 0x1f)
                {
                    case 0x0c: return OOT_GROTTO_GENERIC_KOKIRI_FOREST;
                    case 0x14: return OOT_GROTTO_GENERIC_LOST_WOODS;
                    case 0x08: return OOT_GROTTO_GENERIC_KAKARIKO;
                    case 0x17: return OOT_GROTTO_GENERIC_DMT;
                    case 0x1a: return OOT_GROTTO_GENERIC_DMC;
                    case 0x09: return OOT_GROTTO_GENERIC_RIVER;
                    case 0x02: return OOT_GROTTO_GENERIC_HF_SOUTHEAST;
                    case 0x03: return OOT_GROTTO_GENERIC_HF_OPEN;
                    case 0x00: return OOT_GROTTO_GENERIC_HF_MARKET;
                }
                break;
            }
            case 0x01: return OOT_GROTTO_SCRUB_HEART_PIECE;
            case 0x02: return OOT_GROTTO_REDEAD;
            case 0x03: return OOT_GROTTO_TRAIL_COW;
            case 0x04: return OOT_GROTTO_FIELD_COW;
            case 0x05: return OOT_GROTTO_OCTOROK;
            case 0x06: return OOT_GROTTO_SCRUB_UPGRADE;
            case 0x07: return OOT_GROTTO_WOLFOS;
            case 0x08: return OOT_GROTTO_CASTLE;
            case 0x09:  // Double scrubs
            case 0x0c:  // Triple scrubs
            {
                memcpy(&gLastScene, &RAMData[0x441A68], sizeof(uint32_t));   // gLastScene = 0x80441A68

                switch (gLastScene)
                {
                    // Double scrubs
                    case OOT_SACRED_FOREST_MEADOW: return OOT_GROTTO_SCRUBS2_SFM;
                    case OOT_ZORA_RIVER: return OOT_GROTTO_SCRUBS2_RIVER;
                    case OOT_GERUDO_VALLEY: return OOT_GROTTO_SCRUBS2_VALLEY;
                    case OOT_DESERT_COLOSSUS: return OOT_GROTTO_SCRUBS2_COLOSSUS;

                    // Triple scrubs
                    case OOT_LON_LON_RANCH: return OOT_GROTTO_SCRUBS3_RANCH;
                    case OOT_GORON_CITY: return OOT_GROTTO_SCRUBS3_GORON_CITY;
                    case OOT_DEATH_MOUNTAIN_CRATER: return OOT_GROTTO_SCRUBS3_DMC;
                    case OOT_LAKE_HYLIA: return OOT_GROTTO_SCRUBS3_LAKE;
                }
                break;
            }
            case 0x0a: return OOT_GROTTO_TEKTITE;
            case 0x0b: return OOT_GROTTO_DEKU_THEATER;
            case 0x0d: return OOT_GROTTO_FIELD_TREE;

            case OOT_FAIRY_FOUNTAIN:
            {
                switch (gLastScene)
                {
                    case OOT_SACRED_FOREST_MEADOW: return OOT_GROTTO_FAIRY_SFM;
                    case OOT_HYRULE_FIELD: return OOT_GROTTO_FAIRY_HF;
                    case OOT_ZORA_RIVER: return OOT_GROTTO_FAIRY_RIVER;
                    case OOT_ZORA_DOMAIN: return OOT_GROTTO_FAIRY_DOMAIN;
                    case OOT_GERUDO_FORTRESS: return OOT_GROTTO_FAIRY_FORTRESS;
                }
                break;
            }

            case OOT_TOMB_FAIRY: return OOT_GRAVE_SHIELD;
            case OOT_TOMB_REDEAD: return OOT_GRAVE_REDEAD;
            case OOT_TOMB_ROYAL: return OOT_GRAVE_ROYAL;
            case OOT_TOMB_DAMPE_WINDMILL: return OOT_GRAVE_DAMPE;

            default:
            {
                break;
            }
        }
    }
    else
    {   // MM

        if (std::endian::native == std::endian::little)
        {
            currRoomNum = RAMData[0x3FF203];    // Current room number = 0x803FF203
        }
        else
        {
            currRoomNum = RAMData[0x3FF200];    // Current room number = 0x803FF200
        }

        switch (currRoomNum)
        {
            case 0x00: return MM_GROTTO_GOSSIPS_OCEAN;
            case 0x01: return MM_GROTTO_GOSSIPS_SWAMP;
            case 0x02: return MM_GROTTO_GOSSIPS_CANYON;
            case 0x03: return MM_GROTTO_GOSSIPS_MOUNTAIN;
            case 0x04:
            {
                if (std::endian::native == std::endian::little)
                {
                    gGrottoData = RAMData[0x1F3394];    // gGrottoData = 0x801F3394
                }
                else
                {
                    gGrottoData = RAMData[0x1F3397];    // gGrottoData = 0x801F3397
                }

                switch (gGrottoData & 0x1f)
                {
                    case 0x13: return MM_GROTTO_GENERIC_PATH_SNOWHEAD;
                    case 0x14: return MM_GROTTO_GENERIC_VALLEY;
                    case 0x15: return MM_GROTTO_GENERIC_ZORA_CAPE;
                    case 0x16: return MM_GROTTO_GENERIC_PATH_IKANA;
                    case 0x17: return MM_GROTTO_GENERIC_GREAT_BAY_COAST;
                    case 0x18: return MM_GROTTO_GENERIC_GRAVEYARD;
                    case 0x19: return MM_GROTTO_GENERIC_TWIN_ISLANDS;
                    case 0x1a: return MM_GROTTO_GENERIC_FIELD_PILLAR;
                    case 0x1b: return MM_GROTTO_GENERIC_MOUNTAIN_VILLAGE;
                    case 0x1c: return MM_GROTTO_GENERIC_WOODS;
                    case 0x1d: return MM_GROTTO_GENERIC_SWAMP;
                    case 0x1e: return MM_GROTTO_GENERIC_PATH_SWAMP;
                    case 0x1f: return MM_GROTTO_GENERIC_GRASS;
                }
                break;
            }
            case 0x07: return MM_GROTTO_DODONGO;
            case 0x09: return MM_GROTTO_SCRUB;
            case 0x0a:
            {
                memcpy(&gLastScene, &RAMData[0x80770EC0], sizeof(uint32_t));   // gLastScene = 0x80770EC0

                switch (gLastScene)
                {
                    case MM_TERMINA_FIELD: return MM_GROTTO_COW_FIELD;
                    case MM_GREAT_BAY_COAST: return MM_GROTTO_COW_COAST;
                }
                break;
            }
            case 0x0b: return MM_GROTTO_BIO_BABA;
            case 0x0d: return MM_GROTTO_PEAHAT;
            case 0x0e: return MM_GROTTO_HOT_WATER;

            default:
            {
                break;
            }
        }
    }

    MultiLogger::LogMessage("Unknown grotto scene !");
    return 0;
}



uint32_t EntranceHelper::GetGrottoExit(int Game, uint8_t* RAMData)
{
    uint8_t currRoomNum = 0;
    uint8_t gGrottoData = 0;
    uint32_t gLastScene = 0;

    if (Game == OOT_GAME)
    {
        if (std::endian::native == std::endian::little)
        {
            currRoomNum = RAMData[0x1DA15F];    // Current room number = 0x801DA15D
        }
        else
        {
            currRoomNum = RAMData[0x1DA15C];    // Current room number = 0x801DA15C
        }

        switch (currRoomNum)
        {
            case 0x00:
            {
                if (std::endian::native == std::endian::little)
                {
                    gGrottoData = RAMData[0x11B964];    // Current room number = 0x8011B964
                }
                else
                {
                    gGrottoData = RAMData[0x11B967];    // Current room number = 0x8011B967
                }

                switch (gGrottoData & 0x1f)
                {
                    case 0x0c: return OOT_GROTTO_EXIT_GENERIC_KOKIRI_FOREST;
                    case 0x14: return OOT_GROTTO_EXIT_GENERIC_LOST_WOODS;
                    case 0x08: return OOT_GROTTO_EXIT_GENERIC_KAKARIKO;
                    case 0x17: return OOT_GROTTO_EXIT_GENERIC_DMT;
                    case 0x1a: return OOT_GROTTO_EXIT_GENERIC_DMC;
                    case 0x09: return OOT_GROTTO_EXIT_GENERIC_RIVER;
                    case 0x02: return OOT_GROTTO_EXIT_GENERIC_HF_SOUTHEAST;
                    case 0x03: return OOT_GROTTO_EXIT_GENERIC_HF_OPEN;
                    case 0x00: return OOT_GROTTO_EXIT_GENERIC_HF_MARKET;
                }
                break;
            }
            case 0x01: return OOT_GROTTO_EXIT_SCRUB_HEART_PIECE;
            case 0x02: return OOT_GROTTO_EXIT_REDEAD;
            case 0x03: return OOT_GROTTO_EXIT_TRAIL_COW;
            case 0x04: return OOT_GROTTO_EXIT_FIELD_COW;
            case 0x05: return OOT_GROTTO_EXIT_OCTOROK;
            case 0x06: return OOT_GROTTO_EXIT_SCRUB_UPGRADE;
            case 0x07: return OOT_GROTTO_EXIT_WOLFOS;
            case 0x08: return OOT_GROTTO_EXIT_CASTLE;
            case 0x09:  // Double scrubs
            case 0x0c:  // Triple scrubs
            {
                memcpy(&gLastScene, &RAMData[0x441A68], sizeof(uint32_t));   // gLastScene = 0x80441A68

                switch (gLastScene)
                {
                    // Double scrubs
                    case OOT_SACRED_FOREST_MEADOW: return OOT_GROTTO_EXIT_SCRUBS2_SFM;
                    case OOT_ZORA_RIVER: return OOT_GROTTO_EXIT_SCRUBS2_RIVER;
                    case OOT_GERUDO_VALLEY: return OOT_GROTTO_EXIT_SCRUBS2_VALLEY;
                    case OOT_DESERT_COLOSSUS: return OOT_GROTTO_EXIT_SCRUBS2_COLOSSUS;

                    // Triple scrubs
                    case OOT_LON_LON_RANCH: return OOT_GROTTO_EXIT_SCRUBS3_RANCH;
                    case OOT_GORON_CITY: return OOT_GROTTO_EXIT_SCRUBS3_GORON_CITY;
                    case OOT_DEATH_MOUNTAIN_CRATER: return OOT_GROTTO_EXIT_SCRUBS3_DMC;
                    case OOT_LAKE_HYLIA: return OOT_GROTTO_EXIT_SCRUBS3_LAKE;
                }
                break;
            }
            case 0x0a: return OOT_GROTTO_EXIT_TEKTITE;
            case 0x0b: return OOT_GROTTO_EXIT_DEKU_THEATER;
            case 0x0d: return OOT_GROTTO_EXIT_FIELD_TREE;

            case OOT_FAIRY_FOUNTAIN:
            {
                switch (gLastScene)
                {
                    case OOT_SACRED_FOREST_MEADOW: return OOT_GROTTO_EXIT_FAIRY_SFM;
                    case OOT_HYRULE_FIELD: return OOT_GROTTO_EXIT_FAIRY_HF;
                    case OOT_ZORA_RIVER: return OOT_GROTTO_EXIT_FAIRY_RIVER;
                    case OOT_ZORA_DOMAIN: return OOT_GROTTO_EXIT_FAIRY_DOMAIN;
                    case OOT_GERUDO_FORTRESS: return OOT_GROTTO_EXIT_FAIRY_FORTRESS;
                }
                break;
            }

            case OOT_TOMB_FAIRY: return OOT_GRAVE_EXIT_SHIELD;
            case OOT_TOMB_REDEAD: return OOT_GRAVE_EXIT_REDEAD;
            case OOT_TOMB_ROYAL: return OOT_GRAVE_EXIT_ROYAL;
            case OOT_TOMB_DAMPE_WINDMILL: return OOT_GRAVE_EXIT_DAMPE;

            default:
            {
                break;
            }
        }
    }
    else
    {   // MM

        if (std::endian::native == std::endian::little)
        {
            currRoomNum = RAMData[0x3FF203];    // Current room number = 0x803FF203
        }
        else
        {
            currRoomNum = RAMData[0x3FF200];    // Current room number = 0x803FF200
        }

        switch (currRoomNum)
        {
            case 0x00: return MM_GROTTO_EXIT_GOSSIPS_OCEAN;
            case 0x01: return MM_GROTTO_EXIT_GOSSIPS_SWAMP;
            case 0x02: return MM_GROTTO_EXIT_GOSSIPS_CANYON;
            case 0x03: return MM_GROTTO_EXIT_GOSSIPS_MOUNTAIN;
            case 0x04:
            {
                if (std::endian::native == std::endian::little)
                {
                    gGrottoData = RAMData[0x1F3394];    // gGrottoData = 0x801F3394
                }
                else
                {
                    gGrottoData = RAMData[0x1F3397];    // gGrottoData = 0x801F3397
                }

                switch (gGrottoData & 0x1f)
                {
                    case 0x13: return MM_GROTTO_EXIT_GENERIC_PATH_SNOWHEAD;
                    case 0x14: return MM_GROTTO_EXIT_GENERIC_VALLEY;
                    case 0x15: return MM_GROTTO_EXIT_GENERIC_ZORA_CAPE;
                    case 0x16: return MM_GROTTO_EXIT_GENERIC_PATH_IKANA;
                    case 0x17: return MM_GROTTO_EXIT_GENERIC_GREAT_BAY_COAST;
                    case 0x18: return MM_GROTTO_EXIT_GENERIC_GRAVEYARD;
                    case 0x19: return MM_GROTTO_EXIT_GENERIC_TWIN_ISLANDS;
                    case 0x1a: return MM_GROTTO_EXIT_GENERIC_FIELD_PILLAR;
                    case 0x1b: return MM_GROTTO_EXIT_GENERIC_MOUNTAIN_VILLAGE;
                    case 0x1c: return MM_GROTTO_EXIT_GENERIC_WOODS;
                    case 0x1d: return MM_GROTTO_EXIT_GENERIC_SWAMP;
                    case 0x1e: return MM_GROTTO_EXIT_GENERIC_PATH_SWAMP;
                    case 0x1f: return MM_GROTTO_EXIT_GENERIC_GRASS;
                }
                break;
            }
            case 0x07: return MM_GROTTO_EXIT_DODONGO;
            case 0x09: return MM_GROTTO_EXIT_SCRUB;
            case 0x0a:
            {
                memcpy(&gLastScene, &RAMData[0x80770EC0], sizeof(uint32_t));   // gLastScene = 0x80770EC0

                switch (gLastScene)
                {
                    case MM_TERMINA_FIELD: return MM_GROTTO_EXIT_COW_FIELD;
                    case MM_GREAT_BAY_COAST: return MM_GROTTO_EXIT_COW_COAST;
                }
                break;
            }
            case 0x0b: return MM_GROTTO_EXIT_BIO_BABA;
            case 0x0d: return MM_GROTTO_EXIT_PEAHAT;
            case 0x0e: return MM_GROTTO_EXIT_HOT_WATER;

            default:
            {
                break;
            }
        }
    }

    MultiLogger::LogMessage("Unknown grotto scene !");
    return 0;
}