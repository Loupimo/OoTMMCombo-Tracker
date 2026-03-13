#include "Combo/Entrances.h"
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
                        this->LastTouchedEntranceID = this->GetSceneGrotto(Game, RAMData);
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
                    {   // The RAM is not fully loaded

                        break;  // We break in order to get it on the next loop
                    }
                    this->LastGameTouchedEntrance = Game;
                    this->IsEntranceTouched = false;
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
                        this->LastTouchedEntranceID = this->GetSceneGrotto(Game, RAMData);
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

                        break;  // We break in order to get it on the next loop
                    }
                    this->LastGameTouchedEntrance = Game;
                    this->IsEntranceTouched = false;
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


bool EntranceHelper::IsGrottoExit()
{
    switch (this->LastTouchedEntranceID)
    {
        case 0x03f:     // Generic grotto entry
        case 0x36d:     // Fairy grotto entry
        case 0x5bc:     // Double scrub grotto entry
        case 0x5a4:     // Triple scrub grotto entry
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


uint32_t EntranceHelper::GetSceneGrotto(int Game, uint8_t* RAMData)
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
                    case 0x0c: return OOT_GROTTO_KOKIRI_FOREST_STORMS;
                    case 0x14: return OOT_GROTTO_LOST_WOODS_GENERIC;
                    case 0x08: return OOT_GROTTO_KAKARIKO_OPEN;
                    case 0x17: return OOT_GROTTO_DEATH_TRIAL_STORMS;
                    case 0x1a: return OOT_GROTTO_DEATH_CRATER_GENERIC;
                    case 0x09: return OOT_GROTTO_ZORA_RIVER_GENERIC;
                    case 0x02: return OOT_GROTTO_HYRULE_SE;
                    case 0x03: return OOT_GROTTO_HYRULE_OPEN;
                    case 0x00: return OOT_GROTTO_HYRULE_MARKET;
                }
                break;
            }
            case 0x01: return OOT_GROTTO_HYRULE_SCRUBS;
            case 0x02: return OOT_GROTTO_KAKARIKO_REDEAD;
            case 0x03: return OOT_GROTTO_DEATH_TRIAL_COW;
            case 0x04: return OOT_GROTTO_HYRULE_GERUDO;
            case 0x05: return OOT_GROTTO_VALLEY_OCTOROK;
            case 0x06: return OOT_GROTTO_LOST_WOODS_SCRUB_UPGRADE;
            case 0x07: return OOT_GROTTO_SACRED_MEADOW_WOLFOS;
            case 0x08: return OOT_GROTTO_CASTLE_STORMS;
            case 0x09:  // Double scrubs
            case 0x0c:  // Triple scrubs
            {
                memcpy(&gLastScene, &RAMData[0x441A68], sizeof(uint32_t));   // gLastScene = 0x80441A68

                switch (gLastScene)
                {
                    // Double scrubs
                    case SACRED_FOREST_MEADOW: return OOT_GROTTO_SACRED_MEADOW_STORMS;
                    case ZORA_RIVER: return OOT_GROTTO_ZORA_RIVER_STORMS;
                    case GERUDO_VALLEY: return OOT_GROTTO_VALLEY_STORMS;
                    case DESERT_COLOSSUS: return OOT_GROTTO_DESERT_SCRUBS;

                    // Triple scrubs
                    case LON_LON_RANCH: return OOT_GROTTO_LON_LON_SCRUBS;
                    case GORON_CITY: return OOT_GROTTO_GORON_CITY_SCRUBS;
                    case DEATH_MOUNTAIN_CRATER: return OOT_GROTTO_DEATH_CRATER_SCRUBS;
                    case LAKE_HYLIA: return OOT_GROTTO_LAKE_HYLIA_SCRUBS;
                }
                break;
            }
            case 0x0a: return OOT_GROTTO_HYRULE_TEKTITE;
            case 0x0b: return OOT_GROTTO_LOST_WOODS_THEATER;
            case 0x0d: return OOT_GROTTO_HYRULE_MARKET;

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
            case 0x00: return MM_GROTTO_TERMINA_OCEAN_GOSSIP;
            case 0x01: return MM_GROTTO_TERMINA_SWAMP_GOSSIP;
            case 0x02: return MM_GROTTO_TERMINA_CANYON_GOSSIP;
            case 0x03: return MM_GROTTO_TERMINA_MOUNTAIN_GOSSIP;
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
                    case 0x13: return MM_GROTTO_PATH_TO_SNOWHEAD_GENERIC;
                    case 0x14: return MM_GROTTO_IKANA_VALLEY_OPEN;
                    case 0x15: return MM_GROTTO_ZORA_CAPE_GENERIC;
                    case 0x16: return MM_GROTTO_IKANA_ROAD_GENERIC;
                    case 0x17: return MM_GROTTO_GREAT_BAY_COAST_FISHERMAN;
                    case 0x18: return MM_GROTTO_IKANA_GRAVEYARD_GENERIC;
                    case 0x19: return MM_GROTTO_TWIN_ISLANDS_RAMP;
                    case 0x1a: return MM_GROTTO_TERMINA_PILLAR;
                    case 0x1b: return MM_GROTTO_MOUNTAIN_VILLAGE_GENERIC;
                    case 0x1c: return MM_GROTTO_WOODS_OF_MYSTERY_OPEN;
                    case 0x1d: return MM_GROTTO_SOUTHERN_SWAMP_OPEN;
                    case 0x1e: return MM_GROTTO_SOUTHERN_SWAMP_ROAD_OPEN;
                    case 0x1f: return MM_GROTTO_TERMINA_TALL_GRASS;
                }
                break;
            }
            case 0x07: return MM_GROTTO_TERMINA_DODONGO;
            case 0x09: return MM_GROTTO_TERMINA_SCRUB;
            case 0x0a:
            {
                memcpy(&gLastScene, &RAMData[0x80770EC0], sizeof(uint32_t));   // gLastScene = 0x80770EC0

                switch (gLastScene)
                {
                    case TERMINA_FIELD: return MM_GROTTO_TERMINA_COW;
                    case GREAT_BAY_COAST: return MM_GROTTO_GREAT_BAY_COAST_COW;
                }
                break;
            }
            case 0x0b: return MM_GROTTO_TERMINA_BIO_BABA;
            case 0x0d: return MM_GROTTO_TERMINA_PEEHAT;
            case 0x0e: return MM_GROTTO_TWIN_ISLANDS_FROZEN;

            default:
            {
                break;
            }
        }
    }

    MultiLogger::LogMessage("Unknown grotto scene !");
    return 0;
}