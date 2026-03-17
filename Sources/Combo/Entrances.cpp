#include "Combo/Entrances.h"
#include "Combo/OoTEntrances.h"
#include "Combo/MMEntrances.h"
#include "Multi/Multi.h"
#include <math.h>


static const std::map<int, std::vector<GrottoEntrance>> GrottoEntrances = {
   
    { OOT_KOKIRI_FOREST_FROM_LOST_WOODS_ENTR,   std::vector<GrottoEntrance>() = { { OOT_GROTTO_EXIT_GENERIC_KOKIRI_FOREST, -512,  380, -1224 } } },
    { OOT_LOST_WOODS_FROM_KOKIRI_FOREST_ENTR,   std::vector<GrottoEntrance>() = { { OOT_GROTTO_EXIT_GENERIC_LOST_WOODS,     915,    0,  -925 }, 
                                                                                  { OOT_GROTTO_EXIT_SCRUB_UPGRADE,          670,    0, -2520 } } },
    { OOT_LOST_WOODS_FROM_MEADOW_ENTR,          std::vector<GrottoEntrance>() = { { OOT_GROTTO_EXIT_DEKU_THEATER,            80,  -20, -1600 } } },
    { OOT_SACRED_FOREST_MEADOW_ENTR,            std::vector<GrottoEntrance>() = { { OOT_GROTTO_EXIT_FAIRY_SFM,               45,    0,   220 },
                                                                                  { OOT_GROTTO_EXIT_WOLFOS,                -195,    0,  1900 } } },
    { OOT_WARP_SONG_MEADOW_ENTR,                std::vector<GrottoEntrance>() = { { OOT_GROTTO_EXIT_SCRUBS2_SFM,            310,  480, -2300 } } },
    { OOT_WARP_SONG_LAKE_ENTR,                  std::vector<GrottoEntrance>() = { { OOT_GROTTO_EXIT_SCRUBS3_LAKE,         -3040, -1033, 6075 } } },
    { OOT_KAKARIKO_FROM_GRANNY_ENTR,            std::vector<GrottoEntrance>() = { { OOT_GROTTO_EXIT_GENERIC_KAKARIKO,       860,   80,  -260 } } },
    { OOT_KAKARIKO_FROM_FIELD_ENTR,             std::vector<GrottoEntrance>() = { { OOT_GROTTO_EXIT_REDEAD,                -400,    0,   400 } } },
    { OOT_DEATH_MOUNTAIN_FROM_KAKARIKO_ENTR,    std::vector<GrottoEntrance>() = { { OOT_GROTTO_EXIT_TRAIL_COW,             -688, 1946,  -285 } } },
    { OOT_DEATH_MOUNTAIN_FROM_GORON_CITY_ENTR,  std::vector<GrottoEntrance>() = { { OOT_GROTTO_EXIT_GENERIC_DMT,           -383, 1386, -1206 } } },
    { OOT_DEATH_MOUNTAIN_CRATER_ENTR,           std::vector<GrottoEntrance>() = { { OOT_GROTTO_EXIT_GENERIC_DMC,             40, 1233,  1770 } } },
    { OOT_CRATER_FROM_GORON_CITY_ENTR,          std::vector<GrottoEntrance>() = { { OOT_GROTTO_EXIT_SCRUBS3_DMC,          -1699,  722,  -472 } } },
    { OOT_GORON_CITY_ENTR,                      std::vector<GrottoEntrance>() = { { OOT_GROTTO_EXIT_SCRUBS3_GORON_CITY,    1100,  580, -1190 } } },
    { OOT_ZORA_RIVER_FROM_FIELD_ENTR,           std::vector<GrottoEntrance>() = { { OOT_GROTTO_EXIT_GENERIC_RIVER,          360,  570,   130 },
                                                                                  { OOT_GROTTO_EXIT_FAIRY_RIVER,            670,  570,  -365 },
                                                                                  { OOT_GROTTO_EXIT_SCRUBS2_RIVER,        -1630,  100,  -130 } } },
    { OOT_ZORA_DOMAIN_ENTR,                     std::vector<GrottoEntrance>() = { { OOT_GROTTO_EXIT_FAIRY_DOMAIN,          -860,   14,  -470 } } },
    { OOT_LON_LON_RANCH_FROM_HOUSE_ENTR,        std::vector<GrottoEntrance>() = { { OOT_GROTTO_EXIT_SCRUBS3_RANCH,         1800,    0,  1500 } } },
    { OOT_HYRULE_CASTLE_ENTR,                   std::vector<GrottoEntrance>() = { { OOT_GROTTO_EXIT_CASTLE,                 996, 1571,   844 } } },
    { OOT_FIELD_FROM_LAKE_HYLIA_ENTR,           std::vector<GrottoEntrance>() = { { OOT_GROTTO_EXIT_GENERIC_HF_SOUTHEAST,  -270, -500, 12350 },
                                                                                  { OOT_GROTTO_EXIT_GENERIC_HF_OPEN,      -4030, -700, 13860 },
                                                                                  { OOT_GROTTO_EXIT_SCRUB_HEART_PIECE,    -4990, -700, 13820 } } },
    { OOT_FIELD_FROM_GERUDO_VALLEY_ENTR,        std::vector<GrottoEntrance>() = { { OOT_GROTTO_EXIT_FIELD_COW,            -7870, -300,  6920 } } },
    { OOT_FIELD_FROM_KAKARIKO_ENTR,             std::vector<GrottoEntrance>() = { { OOT_GROTTO_EXIT_FIELD_TREE,            2060,   20,  -170 } } },
    { OOT_FIELD_FROM_MARKET_ENTRANCE_ENTR,      std::vector<GrottoEntrance>() = { { OOT_GROTTO_EXIT_GENERIC_HF_MARKET,    -1425,    0,   810 },
                                                                                  { OOT_GROTTO_EXIT_FAIRY_HF,             -4450, -300,  -425 },
                                                                                  { OOT_GROTTO_EXIT_TEKTITE,              -4945, -300,  2835 } } },
    { OOT_GERUDO_VALLEY_FROM_FIELD_ENTR,        std::vector<GrottoEntrance>() = { { OOT_GROTTO_EXIT_OCTOROK,                280, -555,  1470 } } },
    { OOT_GERUDO_VALLEY_FROM_TENT_ENTR,         std::vector<GrottoEntrance>() = { { OOT_GROTTO_EXIT_SCRUBS2_VALLEY,       -1323,   15,  -969 } } },
    { OOT_GERUDO_FORTRESS_FROM_VALLEY_ENTR,     std::vector<GrottoEntrance>() = { { OOT_GROTTO_EXIT_FAIRY_FORTRESS,         376,  333, -1564 } } },
    { OOT_DESERT_COLOSSUS_FROM_FAIRY_ENTR,      std::vector<GrottoEntrance>() = { { OOT_GROTTO_EXIT_SCRUBS2_COLOSSUS,        60,  -32, -1300 } } },



    { MM_TERMINA_FIELD_FROM_CLOCK_TOWN_NORTH_ENTR,  std::vector<GrottoEntrance>() = { { MM_TERMINA_FIELD_FROM_CLOCK_TOWN_NORTH_ENTR,       -400,   48, -2520 },
                                                                                      { MM_GROTTO_EXIT_GOSSIPS_MOUNTAIN,                    192,   48, -3138 },
                                                                                      { MM_GROTTO_EXIT_DODONGO,                           -2425, -281, -3291 } } },
    { MM_TERMINA_FIELD_FROM_CLOCK_TOWN_WEST_ENTR,   std::vector<GrottoEntrance>() = { { MM_TERMINA_FIELD_FROM_CLOCK_TOWN_WEST_ENTR,       -2400,   68,  -400 },
                                                                                      { MM_GROTTO_EXIT_GOSSIPS_OCEAN,                     -2782,   48, -1654 },
                                                                                      { MM_GROTTO_EXIT_BIO_BABA,                          -5159, -281,  -571 } } },
    { MM_TERMINA_FIELD_FROM_CLOCK_TOWN_EAST_ENTR,   std::vector<GrottoEntrance>() = { { MM_TERMINA_FIELD_FROM_CLOCK_TOWN_EAST_ENTR,        1672,   68,  -394 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_FIELD_PILLAR,               2367,  315,  -192 },
                                                                                      { MM_GROTTO_EXIT_GOSSIPS_CANYON,                     4450,  254,   925 },
                                                                                      { MM_GROTTO_EXIT_SCRUB,                              3223,  219,  1417 } } },
    { MM_TERMINA_FIELD_FROM_CLOCK_TOWN_SOUTH_ENTR,  std::vector<GrottoEntrance>() = { { MM_TERMINA_FIELD_FROM_CLOCK_TOWN_SOUTH_ENTR,       -412,  -77,  1681 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_GRASS,                      1012, -221,  3642 },
                                                                                      { MM_GROTTO_EXIT_COW_FIELD,                          -375, -222,  3976 },
                                                                                      { MM_GROTTO_EXIT_GOSSIPS_SWAMP,                     -1592, -222,  4622 },
                                                                                      { MM_GROTTO_EXIT_PEAHAT,                            -2317, -221,  3418 } } },
    { MM_SWAMP_ROAD_FROM_FIELD_ENTR,                std::vector<GrottoEntrance>() = { { MM_SWAMP_ROAD_FROM_FIELD_ENTR,                      331, -143,   245 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_PATH_SWAMP,                  104, -182,  2202 }, } },
    { MM_SWAMP_FROM_SPIDER_HOUSE_ENTR,              std::vector<GrottoEntrance>() = { { MM_SWAMP_FROM_SPIDER_HOUSE_ENTR,                  -1049,   12,  2042 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_SWAMP,                     -1700,   38,  1800 }} },
    { MM_MYSTERY_WOODS_ENTR,                        std::vector<GrottoEntrance>() = { { MM_MYSTERY_WOODS_ENTR,                              274,    0,     0 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_WOODS,                         2,    0,  -889 } } },
    { MM_WARP_OWL_MOUNTAIN_VILLAGE_ENTR,            std::vector<GrottoEntrance>() = { { MM_WARP_OWL_MOUNTAIN_VILLAGE_ENTR,                 -649,    8,  -196 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_MOUNTAIN_VILLAGE,           2406, 1168, -1197 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_MOUNTAIN_VILLAGE_WINTER,     345,    8,  -150 } } },
    { MM_TWIN_ISLANDS_FROM_MOUNTAIN_VILLAGE_ENTR,   std::vector<GrottoEntrance>() = { { MM_TWIN_ISLANDS_FROM_MOUNTAIN_VILLAGE_ENTR,       -2044,  200,  1288 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_TWIN_ISLANDS,              -1309,  320,   143 },
                                                                                      { MM_GROTTO_EXIT_HOT_WATER,                           589,  195,    53 } } },
    { MM_PATH_SNOWHEAD_FROM_SNOWHEAD_ENTR,          std::vector<GrottoEntrance>() = { { MM_PATH_SNOWHEAD_FROM_SNOWHEAD_ENTR,              -2518,  550, -3441 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_PATH_SNOWHEAD,              -987,  360, -2339 }, } },
    { MM_GREAT_BAY_COAST_FROM_FISHER_HUT_ENTR,      std::vector<GrottoEntrance>() = { { MM_GREAT_BAY_COAST_FROM_FISHER_HUT_ENTR,           1137,   92,  4635 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_GREAT_BAY_COAST,            1359,   80,  5018 } } },
    { MM_GREAT_BAY_COAST_FROM_LABORATORY_ENTR,      std::vector<GrottoEntrance>() = { { MM_GREAT_BAY_COAST_FROM_LABORATORY_ENTR,          -3080,  240,  4080 },
                                                                                      { MM_GROTTO_EXIT_COW_COAST,                          2077,  333,  -215 } } },
    { MM_ZORA_CAPE_FROM_GREAT_BAY_COAST_ENTR,       std::vector<GrottoEntrance>() = { { MM_ZORA_CAPE_FROM_GREAT_BAY_COAST_ENTR,              92,   12,   333 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_ZORA_CAPE,                  -562,   80,  2707 } } },
    { MM_IKANA_ROAD_FROM_FIELD_ENTR,                std::vector<GrottoEntrance>() = { { MM_IKANA_ROAD_FROM_FIELD_ENTR,                    -3006,    0,  -305 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_PATH_IKANA,                 -428,  200,  -335 } } },
    { MM_IKANA_GRAVEYARD_FROM_DAMPE_ENTR,           std::vector<GrottoEntrance>() = { { MM_IKANA_GRAVEYARD_FROM_DAMPE_ENTR,                -504,  314, -2159 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_GRAVEYARD,                   106,  314, -1777 } } },
    { MM_IKANA_VALLEY_FROM_ROAD_ENTR,               std::vector<GrottoEntrance>() = { { MM_IKANA_VALLEY_FROM_ROAD_ENTR,                     -64, -371,  4870 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_VALLEY,                    -2475, -505,  2475 } } }
};


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
    const uint32_t transitionFrames = 0x14;  // The number of time to wait before link can be movable
    uint32_t tmpLast = 0;
    uint32_t frameCount = 0;
    uint8_t respawnFlag = 0;
    uint8_t transitionTrigger = 0;
    EntranceMetaInfo entranceMeta = {};

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

                memcpy(&tmpLast, &RAMData[0x1DA2B8], sizeof(uint32_t));      // playState.nextEntrance = 0x801DA2B8
                memcpy(&frameCount, &RAMData[0x1C853C], sizeof(uint32_t));   // gPlay.state.frameCount = 0x801C853C
                this->IsEntranceTouched = true;
                this->LastGameTouchedEntrance = Game;
                if (tmpLast != this->LastTouchedEntranceID && this->LastFrameCount == 0)
                {
                    this->LastTouchedEntranceID = tmpLast;
                    this->LastFrameCount = frameCount;                          // This avoid triggering the function multiple times due to grottos changing the last entrance ID
                    if (this->IsGrottoEntrance(this->LastTouchedEntranceID))
                    {
                        this->LastTouchedEntranceID = this->GetGrottoEntrance(Game, RAMData, this->LastTouchedEntranceID);
                    }
                    else if (this->IsGrottoExit(this->LastTouchedEntranceID))
                    {
                        this->LastTouchedEntranceID = this->GetGrottoExit(Game, RAMData);
                    }
                    entranceMeta = OoTEntrances.at(this->LastTouchedEntranceID);
                    this->LastTouchedStr = entranceMeta.FromName + std::string(" -> ") + entranceMeta.ToName;
                    MultiLogger::LogMessage("Loading zone %s (0x%X) touched ! 0x%X", this->LastTouchedStr.c_str(), this->LastTouchedEntranceID, this->LastFrameCount);
                }
                break;
            }

            default:
            {
                this->LastFrameCount = 0;
                memcpy(&frameCount, &RAMData[0x1C853C], sizeof(uint32_t));   // gPlay.state.frameCount = 0x801C853C
                if (this->IsEntranceTouched)
                {   // We have switched game with the last touched entrance

                    if (frameCount <= transitionFrames)
                    {   // The RAM is not fully loaded. We need to wait 20 frames in order for Link to be movable

                        return;  // We return in order to get it on the next loop
                    }

                    if (this->LastGameTouchedEntrance != Game)
                    {
                        memcpy(&this->EntranceID, &RAMData[0x441A6C], sizeof(uint32_t));   // gLastEntrance = 0x80441A6C
                    }
                    else
                    {
                        memcpy(&this->EntranceID, &RAMData[0x1DA2B8], sizeof(uint32_t));   // gSaveContext.entrance = 0x801DA2B8
                    }
                    this->LastGameTouchedEntrance = Game;
                    this->IsEntranceTouched = false;
                    if (this->IsGrottoEntrance(this->EntranceID))
                    {
                        this->EntranceID = this->GetGrottoEntrance(Game, RAMData, this->EntranceID);
                    }
                    else if (this->IsGrottoExit(this->EntranceID))
                    {
                        this->EntranceID = this->GetGrottoExit(Game, RAMData);
                    }
                    else
                    {
                        this->EntranceID = this->CheckGrottoSpawn(this->EntranceID, RAMData);
                    }
                    entranceMeta = OoTEntrances.at(this->EntranceID);
                    this->EntranceStr = entranceMeta.FromName + std::string(" -> ") + entranceMeta.ToName;
                    MultiLogger::LogMessage("Scene Loaded ! Entrance : %s (0x%X), Last loading zone : %s (0x%X)", this->EntranceStr.c_str(), this->EntranceID, this->LastTouchedStr.c_str(), this->LastTouchedEntranceID);
                }
                else
                {   // We are on the same game as the last touched entrance

                    memcpy(&this->EntranceID, &RAMData[0x11A5D0], sizeof(uint32_t));   // gSaveContext.entrance = 0x8011A5D0
                }
                break;
            }
        }
    }
    else
    {   // Read entrance ID for MM

        if (std::endian::native == std::endian::little)
        {
            transitionTrigger = RAMData[0x3FF396];    // Transition trigger = 0x803FF396
            respawnFlag = RAMData[0x1F3320];          // Respawn flag = 0x04 when coming from a grotto entrance (only for a short amount of time)
        }
        else
        {
            transitionTrigger = RAMData[0x3FF395];    // Transition trigger = 0x803FF395
            respawnFlag = RAMData[0x1F3323];          // Respawn flag = 0x04 when coming from a grotto entrance (only for a short amount of time)
        }

        switch (transitionTrigger)
        {
            case 0x14:
            {   // Touched loading zone

                memcpy(&tmpLast, &RAMData[0x3FF398], sizeof(uint32_t));      // playState.nextEntrance = 0x803FF39A8
                memcpy(&frameCount, &RAMData[0x3E6BBC], sizeof(uint32_t));   // gPlay.state.frameCount = 0x803E6BBC
                this->IsEntranceTouched = true;
                this->LastGameTouchedEntrance = Game;
                if (tmpLast != this->LastTouchedEntranceID && this->LastFrameCount == 0)
                {
                    this->LastTouchedEntranceID = tmpLast;
                    this->LastFrameCount = frameCount;                          // This avoid triggering the function multiple times due to grottos changing the last entrance ID
                    if (this->IsGrottoEntrance(this->LastTouchedEntranceID))
                    {
                        this->LastTouchedEntranceID = this->GetGrottoEntrance(Game, RAMData, this->LastTouchedEntranceID);
                    }
                    else if (this->IsGrottoExit(this->LastTouchedEntranceID))
                    {
                        this->LastTouchedEntranceID = this->GetGrottoExit(Game, RAMData);
                    }
                    entranceMeta = MMEntrances.at(this->LastTouchedEntranceID);
                    this->LastTouchedStr = entranceMeta.FromName + std::string(" -> ") + entranceMeta.ToName;
                    MultiLogger::LogMessage("Loading zone %s (0x%X) touched !", this->LastTouchedStr.c_str(), this->LastTouchedEntranceID);
                }
                break;
            }

            default:
            {
                this->LastFrameCount = 0;
                if (this->IsEntranceTouched)
                {   // We have switched game with the last touched entrance
                    
                    memcpy(&frameCount, &RAMData[0x3E6BBC], sizeof(uint32_t));   // gPlay.state.frameCount = 0x803E6BBC

                    if (frameCount <= transitionFrames)
                    {   // The RAM is not fully loaded. We need to wait 20 frames in order for Link to be movable

                        return;  // We return in order to get it on the next loop
                    }

                    memcpy(&this->EntranceID, &RAMData[0x1EF670], sizeof(uint32_t)); // gSaveContext.entrance = 0x801EF670

                    this->LastGameTouchedEntrance = Game;
                    this->IsEntranceTouched = false;
                    if (this->IsGrottoEntrance(this->EntranceID))
                    {
                        this->EntranceID = this->GetGrottoEntrance(Game, RAMData, this->EntranceID);
                    }
                    else if (this->IsGrottoExit(this->EntranceID))
                    {
                        this->EntranceID = this->GetGrottoExit(Game, RAMData);
                    }
                    else //if (respawnFlag == 0x04)
                    {
                        this->EntranceID = this->CheckGrottoSpawn(this->EntranceID, RAMData);
                    }
                    entranceMeta = MMEntrances.at(this->EntranceID);
                    this->EntranceStr = entranceMeta.FromName + std::string(" -> ") + entranceMeta.ToName;
                    MultiLogger::LogMessage("Scene Loaded ! Entrance : %s (0x%X), Last loading zone : %s (0x%X), Respawn flag = 0x%X", this->EntranceStr.c_str(), this->EntranceID, this->LastTouchedStr.c_str(), this->LastTouchedEntranceID, respawnFlag);
                }
                else
                {   // We are on the same game as the last touched entrance
                    
                    memcpy(&this->EntranceID, &RAMData[0x1EF670], sizeof(uint32_t));   // gSaveContext.entrance = 0x801EF670
                }
                break;
            }
        }
    }
}


bool EntranceHelper::IsGrottoEntrance(uint32_t ID)
{
    switch (ID)
    {
        case 0x03f:     // Generic grotto entry (OoT)
        case 0x36d:     // Fairy grotto entry (OoT)
        case 0x5bc:     // Double scrub grotto entry (OoT)
        case 0x5a4:     // Triple scrub grotto entry (OoT)
        case 0x1440:    // Generic grotto entry (MM)
        case 0x14a0:    // Cow grotto entry (MM)
        {
            return true;
        }

        default:
        {
            return false;
        }
    }
}


bool EntranceHelper::IsGrottoExit(uint32_t ID)
{
    switch (ID)
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



uint32_t EntranceHelper::GetGrottoEntrance(int Game, uint8_t* RAMData, uint32_t ID)
{
    uint8_t currRoomNum = 0;
    uint8_t gGrottoData = 0;
    uint32_t gLastScene = 0;

    if (Game == OOT_GAME)
    {
        switch (ID)
        {
            case OOT_GROTTO_TYPE_GENERIC_ENTR:
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
                    case 0x0c: return OOT_GROTTO_GENERIC_KOKIRI_FOREST_ENTR;
                    case 0x14: return OOT_GROTTO_GENERIC_LOST_WOODS_ENTR;
                    case 0x08: return OOT_GROTTO_GENERIC_KAKARIKO_ENTR;
                    case 0x17: return OOT_GROTTO_GENERIC_DMT_ENTR;
                    case 0x1a: return OOT_GROTTO_GENERIC_DMC_ENTR;
                    case 0x09: return OOT_GROTTO_GENERIC_RIVER_ENTR;
                    case 0x02: return OOT_GROTTO_GENERIC_HF_SOUTHEAST_ENTR;
                    case 0x03: return OOT_GROTTO_GENERIC_HF_OPEN_ENTR;
                    case 0x00: return OOT_GROTTO_GENERIC_HF_MARKET_ENTR;
                }
                break;
            }
            case OOT_GROTTO_TYPE_FAIRY_ENTR:
            {
                memcpy(&gLastScene, &RAMData[0x441A68], sizeof(uint32_t));   // gLastScene = 0x80441A68
                switch (gLastScene)
                {
                    case OOT_SACRED_FOREST_MEADOW: return OOT_GROTTO_FAIRY_SFM_ENTR;
                    case OOT_HYRULE_FIELD: return OOT_GROTTO_FAIRY_HF_ENTR;
                    case OOT_ZORA_RIVER: return OOT_GROTTO_FAIRY_RIVER_ENTR;
                    case OOT_ZORA_DOMAIN: return OOT_GROTTO_FAIRY_DOMAIN_ENTR;
                    case OOT_GERUDO_FORTRESS: return OOT_GROTTO_FAIRY_FORTRESS_ENTR;
                }
                break;
            }
            case OOT_GROTTO_TYPE_SCRUB2_ENTR:
            case OOT_GROTTO_TYPE_SCRUB3_ENTR:
            {
                memcpy(&gLastScene, &RAMData[0x441A68], sizeof(uint32_t));   // gLastScene = 0x80441A68
                switch (gLastScene)
                {
                    // Double Scrubs
                    case OOT_SACRED_FOREST_MEADOW: return OOT_GROTTO_SCRUBS2_SFM_ENTR;
                    case OOT_ZORA_RIVER: return OOT_GROTTO_SCRUBS2_RIVER_ENTR;
                    case OOT_GERUDO_VALLEY: return OOT_GROTTO_SCRUBS2_VALLEY_ENTR;
                    case OOT_DESERT_COLOSSUS: return OOT_GROTTO_SCRUBS2_COLOSSUS_ENTR;

                    // Triple Scrubs
                    case OOT_LON_LON_RANCH: return OOT_GROTTO_SCRUBS3_RANCH_ENTR;
                    case OOT_GORON_CITY: return OOT_GROTTO_SCRUBS3_GORON_CITY_ENTR;
                    case OOT_DEATH_MOUNTAIN_CRATER: return OOT_GROTTO_SCRUBS3_DMC_ENTR;
                    case OOT_LAKE_HYLIA: return OOT_GROTTO_SCRUBS3_LAKE_ENTR;
                }
                break;
            }
            default:
            {
                break;
            }
        }
    }
    else
    {   // MM

        uint32_t entranceKey;

        entranceKey = (ID >> 9);
        switch (entranceKey)
        {
            case 0x06: entranceKey = 0x42; break;
            case 0x57: entranceKey = 0x4d; break;
            case 0x45: entranceKey = 0x4a; break;
            case 0x5b: entranceKey = 0x5a; break;
        }
        ID = (entranceKey << 9) | (ID & 0x1ff);

        switch (ID)
        {
            case MM_GROTTO_TYPE_GENERIC_ENTR:
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
                    case 0x13: return MM_GROTTO_GENERIC_PATH_SNOWHEAD_ENTR;
                    case 0x14: return MM_GROTTO_GENERIC_VALLEY_ENTR;
                    case 0x15: return MM_GROTTO_GENERIC_ZORA_CAPE_ENTR;
                    case 0x16: return MM_GROTTO_GENERIC_PATH_IKANA_ENTR;
                    case 0x17: return MM_GROTTO_GENERIC_GREAT_BAY_COAST_ENTR;
                    case 0x18: return MM_GROTTO_GENERIC_GRAVEYARD_ENTR;
                    case 0x19: return MM_GROTTO_GENERIC_TWIN_ISLANDS_ENTR;
                    case 0x1a: return MM_GROTTO_GENERIC_FIELD_PILLAR_ENTR;
                    case 0x1b: return MM_GROTTO_GENERIC_MOUNTAIN_VILLAGE_ENTR;
                    case 0x1c: return MM_GROTTO_GENERIC_WOODS_ENTR;
                    case 0x1d: return MM_GROTTO_GENERIC_SWAMP_ENTR;
                    case 0x1e: return MM_GROTTO_GENERIC_PATH_SWAMP_ENTR;
                    case 0x1f: return MM_GROTTO_GENERIC_GRASS_ENTR;
                }
                break;
            }
            case MM_GROTTO_TYPE_COW_ENTR:
            {
                memcpy(&gLastScene, &RAMData[0x80770EC0], sizeof(uint32_t));   // gLastScene = 0x80770EC0

                switch (gLastScene)
                {
                    case MM_TERMINA_FIELD: return MM_GROTTO_COW_FIELD_ENTR;
                    case MM_GREAT_BAY_COAST: return MM_GROTTO_COW_COAST_ENTR;
                }
                break;
            }
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
                memcpy(&gLastScene, &RAMData[0x770EC0], sizeof(uint32_t));   // gLastScene = 0x80770EC0

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


float EntranceHelper::GetDistanceGrottoEntrance(GrottoEntrance Grotto, uint8_t* RAMData)
{
    float x = 0, y = 0, z = 0;

    if (this->LastGameTouchedEntrance == OOT_GAME)
    {   // OoT

        memcpy(&x, &RAMData[0x1C8714], sizeof(float));                       // X position = 0x801C8714
        memcpy(&y, &RAMData[0x1C8714 + sizeof (float)],    sizeof(float));   // Y position = 0x801C8718
        memcpy(&z, &RAMData[0x1C8714 + sizeof(float) * 2], sizeof(float));   // Z position = 0x801C871C
    }
    else
    {   // MM

        memcpy(&x, &RAMData[0x3E6DD4], sizeof(float));                       // X position = 0x803E6DD4
        memcpy(&y, &RAMData[0x3E6DD4 + sizeof(float)], sizeof(float));       // Y position = 0x803E6DD8
        memcpy(&z, &RAMData[0x3E6DD4 + sizeof(float) * 2], sizeof(float));   // Z position = 0x803E6DDC
    }

    float xDist = fabsf(Grotto.SpawnPos[0] - x);
    float yDist = fabsf(Grotto.SpawnPos[1] - y);
    float zDist = fabsf(Grotto.SpawnPos[2] - z);

    MultiLogger::LogMessage("X dist = %f, Y dist = %f, Z dist = %f, Total = %f, Frame Count = %d", xDist, yDist, zDist, xDist+yDist+zDist, this->LastFrameCount);
    return xDist + yDist + zDist;
}


uint32_t EntranceHelper::CheckGrottoSpawn(uint32_t ID, uint8_t * RAMData)
{
    float xRespawn = 0, yRespawn = 0, zRespawn = 0;

    switch (ID)
    {
        case OOT_KOKIRI_FOREST_FROM_LOST_WOODS_ENTR:
        case OOT_LOST_WOODS_FROM_KOKIRI_FOREST_ENTR:
        case OOT_LOST_WOODS_FROM_MEADOW_ENTR:
        case OOT_SACRED_FOREST_MEADOW_ENTR:
        case OOT_WARP_SONG_MEADOW_ENTR:
        case OOT_WARP_SONG_LAKE_ENTR:
        case OOT_KAKARIKO_FROM_GRANNY_ENTR:
        case OOT_KAKARIKO_FROM_FIELD_ENTR:
        case OOT_DEATH_MOUNTAIN_FROM_KAKARIKO_ENTR:
        case OOT_DEATH_MOUNTAIN_FROM_GORON_CITY_ENTR:
        case OOT_DEATH_MOUNTAIN_CRATER_ENTR:
        case OOT_CRATER_FROM_GORON_CITY_ENTR:
        case OOT_GORON_CITY_ENTR:
        case OOT_ZORA_RIVER_FROM_FIELD_ENTR:
        case OOT_ZORA_DOMAIN_ENTR:
        case OOT_LON_LON_RANCH_FROM_HOUSE_ENTR:
        case OOT_HYRULE_CASTLE_ENTR:
        case OOT_FIELD_FROM_LAKE_HYLIA_ENTR:
        case OOT_FIELD_FROM_GERUDO_VALLEY_ENTR:
        case OOT_FIELD_FROM_KAKARIKO_ENTR:
        case OOT_FIELD_FROM_MARKET_ENTRANCE_ENTR:
        case OOT_GERUDO_VALLEY_FROM_FIELD_ENTR:
        case OOT_GERUDO_VALLEY_FROM_TENT_ENTR:
        case OOT_GERUDO_FORTRESS_FROM_VALLEY_ENTR:
        case OOT_DESERT_COLOSSUS_FROM_FAIRY_ENTR:
        {
            memcpy(&xRespawn, &RAMData[0x11B938], sizeof(float));                      // gSaveContext.respawn[0].pos.x = 0x8011B938
            memcpy(&yRespawn, &RAMData[0x11B938 + sizeof(float)], sizeof(float));      // gSaveContext.respawn[0].pos.y = 0x8011B93C
            memcpy(&zRespawn, &RAMData[0x11B938 + sizeof(float) * 2], sizeof(float));  // gSaveContext.respawn[0].pos.z = 0x8011B940

            break;
        }

        case MM_TERMINA_FIELD_FROM_CLOCK_TOWN_NORTH_ENTR:
        case MM_TERMINA_FIELD_FROM_CLOCK_TOWN_WEST_ENTR:
        case MM_TERMINA_FIELD_FROM_CLOCK_TOWN_EAST_ENTR:
        case MM_TERMINA_FIELD_FROM_CLOCK_TOWN_SOUTH_ENTR:
        case MM_SWAMP_ROAD_FROM_FIELD_ENTR:
        case MM_SWAMP_FROM_SPIDER_HOUSE_ENTR:
        case MM_MYSTERY_WOODS_ENTR:
        case MM_WARP_OWL_MOUNTAIN_VILLAGE_ENTR:
        case MM_TWIN_ISLANDS_FROM_MOUNTAIN_VILLAGE_ENTR:
        case MM_PATH_SNOWHEAD_FROM_SNOWHEAD_ENTR:
        case MM_GREAT_BAY_COAST_FROM_FISHER_HUT_ENTR:
        case MM_GREAT_BAY_COAST_FROM_LABORATORY_ENTR:
        case MM_ZORA_CAPE_FROM_GREAT_BAY_COAST_ENTR:
        case MM_IKANA_ROAD_FROM_FIELD_ENTR:
        case MM_IKANA_GRAVEYARD_FROM_DAMPE_ENTR:
        case MM_IKANA_VALLEY_FROM_ROAD_ENTR:
        {
            memcpy(&xRespawn, &RAMData[0x1F3324], sizeof(float));                      // gSaveContext.respawn[0].pos.x = 0x801F3324
            memcpy(&yRespawn, &RAMData[0x1F3324 + sizeof(float)], sizeof(float));      // gSaveContext.respawn[0].pos.y = 0x801F3328
            memcpy(&zRespawn, &RAMData[0x1F3324 + sizeof(float) * 2], sizeof(float));  // gSaveContext.respawn[0].pos.z = 0x801F332C

            float xRespawn3 = 0, yRespawn3 = 0, zRespawn3 = 0;

            memcpy(&xRespawn3, &RAMData[0x1F3384], sizeof(float));                      // gSaveContext.respawn[3].pos.x = 0x801F3384
            memcpy(&yRespawn3, &RAMData[0x1F3384 + sizeof(float)], sizeof(float));      // gSaveContext.respawn[3].pos.y = 0x801F3388
            memcpy(&zRespawn3, &RAMData[0x1F3384 + sizeof(float) * 2], sizeof(float));  // gSaveContext.respawn[3].pos.z = 0x801F338C

            const std::vector<GrottoEntrance> entrances = GrottoEntrances.at(ID);
            if (xRespawn != entrances[0].SpawnPos[0] || yRespawn != entrances[0].SpawnPos[1] || zRespawn != entrances[0].SpawnPos[2])
            {   // The first respawn coordinates does not match the area coordinate, it is mandatory that we are coming from respawn3 corredinates

                xRespawn = xRespawn3;
                yRespawn = yRespawn3;
                zRespawn = zRespawn3;

                break;
            }

            float tmpDist = 0.0f, bestDist = 0.0f;
            uint32_t bestEntID = ID;

            for (size_t i = 0; i < entrances.size(); i++)
            {
                if (i == 0)
                {
                    bestDist = this->GetDistanceGrottoEntrance(entrances[i], RAMData);
                }
                else if (xRespawn3 == entrances[0].SpawnPos[0] && yRespawn3 == entrances[0].SpawnPos[1] && zRespawn3 == entrances[0].SpawnPos[2])
                {   // We only want to test the specific grotto

                    tmpDist = this->GetDistanceGrottoEntrance(entrances[i], RAMData);

                    if (tmpDist < bestDist)
                    {
                        bestDist = tmpDist;
                        bestEntID = entrances[i].EntranceID;
                    }

                    break;  // There is no other possibility
                }
            }

            return bestEntID;
        }

        default:
        {
            return ID;
        }
    }

    const std::vector<GrottoEntrance> entrances = GrottoEntrances.at(ID);

    for (size_t i = 0; i < entrances.size(); i++)
    {
        GrottoEntrance currEntrance = entrances[i];
        if (xRespawn == currEntrance.SpawnPos[0] && yRespawn == currEntrance.SpawnPos[1] && zRespawn == currEntrance.SpawnPos[2])
        {   // The respawn coordinates match the grotto entrance

            return entrances[i].EntranceID;
        }
    }

    return ID;
}