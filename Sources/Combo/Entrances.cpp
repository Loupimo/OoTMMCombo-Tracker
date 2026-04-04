#include "Combo/Entrances.h"
#include "Combo/OoTEntrances.h"
#include "Combo/MMEntrances.h"
#include "UI/SceneEntrance.h"
#include "Multi/Multi.h"
#include <math.h>

static const std::vector<GrottoEntrance> SpecialWarps = {
    { MM_DEKU_KING_CAUGHT, -6, 0, -323 },
    { MM_PIRATE_ENTRANCE_CAUGHT, -2808.5535, 14, 130.12534 }
};

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
    this->OutEntrance = 0;
    this->IsEntranceTouched = false;
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
        case OOT_INTERNAL_EXIT_GROTTO_ENTR:    // Any grotto exit touched (OoT)
        case MM_INTERNAL_EXIT_GROTTO_ENTR:     // Any grotto exit touched (MM)
        {   
            return true;
        }

        default:
        {
            return false;
        }
    }
}


bool EntranceHelper::IsWarpEntrance(uint32_t ID)
{
    switch (ID)
    {
        case WARP_LOADING:    // Any warp song, save, zone (OoT / MM)
        {
            return true;
        }

        default:
        {
            return false;
        }
    }
}


uint32_t EntranceHelper::GetGrottoEntrance(uint8_t Game, uint8_t GrottoData, uint32_t ID, uint32_t LastScene)
{
    if (Game == OOT_GAME)
    {
        switch (ID)
        {
            case OOT_GROTTO_TYPE_GENERIC_ENTR:
            {
                switch (GrottoData & 0x1f)
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
                switch (LastScene)
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
                switch (LastScene)
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
                switch (GrottoData & 0x1f)
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
                switch (LastScene)
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


uint32_t EntranceHelper::GetGrottoExit(uint8_t Game, uint8_t CurrRoom, uint8_t GrottoData, uint32_t LastScene)
{
    uint8_t currRoomNum = 0;

    if (Game == OOT_GAME)
    {
        switch (CurrRoom)
        {
            case 0x00:
            {

                switch (GrottoData & 0x1f)
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
                
                switch (LastScene)
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
                switch (LastScene)
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

        switch (CurrRoom)
        {
            case 0x00: return MM_GROTTO_EXIT_GOSSIPS_OCEAN;
            case 0x01: return MM_GROTTO_EXIT_GOSSIPS_SWAMP;
            case 0x02: return MM_GROTTO_EXIT_GOSSIPS_CANYON;
            case 0x03: return MM_GROTTO_EXIT_GOSSIPS_MOUNTAIN;
            case 0x04:
            {
                switch (GrottoData & 0x1f)
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
                switch (LastScene)
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


uint32_t EntranceHelper::GetWarpSong(uint8_t * Game, uint32_t ID, uint8_t SongIndex, uint8_t OwlID)
{
    if (*Game == OOT_GAME)
    {
        switch ((WarpSong)SongIndex)
        {
            case WarpSong::Minuet_of_Forest:
            {
                return OOT_MINUET_OF_FOREST_SONG;
            }

            case WarpSong::Bolero_of_Fire:
            {
                return OOT_BOLERO_OF_FIRE_SONG;
            }

            case WarpSong::Serenade_of_Water:
            {
                return OOT_SERENADE_OF_WATER_SONG;
            }

            case WarpSong::Requiem_of_Spirit:
            {
                return OOT_REQUIEM_OF_SPIRIT_SONG;
            }

            case WarpSong::Nocturne_of_Shadow:
            {
                return OOT_NOCTURNE_OF_SHADOW_SONG;
            }

            case WarpSong::Prelude_of_Light:
            {
                return OOT_PRELUDE_OF_LIGHT_SONG;
            }

            case 0xFE:
            {
                switch (OwlID)
                {
                    case OwlScene::Great_Bay_Coast:
                    {
                        *Game = MM_GAME;
                        return MM_WARP_OWL_GREAT_BAY_ENTR;
                    }
                    case OwlScene::Zora_Cape:
                    {
                        *Game = MM_GAME;
                        return MM_WARP_OWL_ZORA_CAPE_ENTR;
                    }
                    case OwlScene::Snowhead:
                    {
                        *Game = MM_GAME;
                        return MM_WARP_OWL_SNOWHEAD_ENTR;
                    }
                    case OwlScene::Mountain_Village:
                    {
                        *Game = MM_GAME;
                        return MM_WARP_OWL_MOUNTAIN_VILLAGE_ENTR;
                    }
                    case OwlScene::Clock_Town:
                    {
                        *Game = MM_GAME;
                        return MM_WARP_OWL_CLOCK_TOWN_ENTR;
                    }
                    case OwlScene::Milk_Road:
                    {
                        *Game = MM_GAME;
                        return MM_WARP_OWL_MILK_ROAD_ENTR;
                    }
                    case OwlScene::Woodfall:
                    {
                        *Game = MM_GAME;
                        return MM_WARP_OWL_WOODFALL_ENTR;
                    }
                    case OwlScene::Southern_Swamp:
                    {
                        *Game = MM_GAME;
                        return MM_WARP_OWL_SOUTHERN_SWAMP_ENTR;
                    }
                    case OwlScene::Ikana_Canyon:
                    {
                        *Game = MM_GAME;
                        return MM_WARP_OWL_IKANA_CANYON_ENTR;
                    }
                    case OwlScene::Stone_Tower:
                    {
                        *Game = MM_GAME;
                        return MM_WARP_OWL_STONE_TOWER_ENTR;
                    }

                    default:
                    {
                        break;
                    }
                }
            }

            default:
            {
                break;
            }
        }
    }
    else if (*Game == MM_GAME)
    {
        switch ((WarpSong)SongIndex)
        {
            case WarpSong::Minuet_of_Forest + 0x80:
            {
                *Game = OOT_GAME;
                return OOT_MINUET_OF_FOREST_SONG;
            }

            case WarpSong::Bolero_of_Fire + 0x80:
            {
                *Game = OOT_GAME;
                return OOT_BOLERO_OF_FIRE_SONG;
            }

            case WarpSong::Serenade_of_Water + 0x80:
            {
                *Game = OOT_GAME;
                return OOT_SERENADE_OF_WATER_SONG;
            }

            case WarpSong::Requiem_of_Spirit + 0x80:
            {
                *Game = OOT_GAME;
                return OOT_REQUIEM_OF_SPIRIT_SONG;
            }

            case WarpSong::Nocturne_of_Shadow + 0x80:
            {
                *Game = OOT_GAME;
                return OOT_NOCTURNE_OF_SHADOW_SONG;
            }

            case WarpSong::Prelude_of_Light + 0x80:
            {
                *Game = OOT_GAME;
                return OOT_PRELUDE_OF_LIGHT_SONG;
            }

            case WarpSong::Song_of_Soaring:
            {
                switch (OwlID)
                {
                    case OwlScene::Great_Bay_Coast:
                    {
                        return MM_WARP_OWL_GREAT_BAY_ENTR;
                    }
                    case OwlScene::Zora_Cape:
                    {
                        return MM_WARP_OWL_ZORA_CAPE_ENTR;
                    }
                    case OwlScene::Snowhead:
                    {
                        return MM_WARP_OWL_SNOWHEAD_ENTR;
                    }
                    case OwlScene::Mountain_Village:
                    {
                        return MM_WARP_OWL_MOUNTAIN_VILLAGE_ENTR;
                    }
                    case OwlScene::Clock_Town:
                    {
                        return MM_WARP_OWL_CLOCK_TOWN_ENTR;
                    }
                    case OwlScene::Milk_Road:
                    {
                        return MM_WARP_OWL_MILK_ROAD_ENTR;
                    }
                    case OwlScene::Woodfall:
                    {
                        return MM_WARP_OWL_WOODFALL_ENTR;
                    }
                    case OwlScene::Southern_Swamp:
                    {
                        return MM_WARP_OWL_SOUTHERN_SWAMP_ENTR;
                    }
                    case OwlScene::Ikana_Canyon:
                    {
                        return MM_WARP_OWL_IKANA_CANYON_ENTR;
                    }
                    case OwlScene::Stone_Tower:
                    {
                        return MM_WARP_OWL_STONE_TOWER_ENTR;
                    }

                    default:
                    {
                        break;
                    }
                }
            }

            default:
            {
                break;
            }
        }
    }

    return ID;
}


uint32_t EntranceHelper::CheckGrottoSpawn(uint32_t ID, uint32_t Buffer[6])
{
    float x = 0.0f, y = 0.0f, z = 0.0f;

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
            memcpy(&x, &Buffer[3], sizeof(float));
            memcpy(&y, &Buffer[4], sizeof(float));
            memcpy(&z, &Buffer[5], sizeof(float));
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
            memcpy(&x, &Buffer[3], sizeof(float));
            memcpy(&y, &Buffer[4], sizeof(float));
            memcpy(&z, &Buffer[5], sizeof(float));
            break;
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
        if (x == currEntrance.SpawnPos[0] && y == currEntrance.SpawnPos[1] && z == currEntrance.SpawnPos[2])
        {   // The respawn coordinates match the grotto entrance

            return entrances[i].EntranceID;
        }
    }

    return ID;
}


uint32_t EntranceHelper::CheckSpecialCase(uint8_t Game, uint32_t ID, uint32_t * SceneID)
{
    if (Game == OOT_GAME)
    {
        switch (*SceneID)
        {
            case OOT_CASTLE_COURTYARD:
            {
                switch (ID)
                {
                    case OOT_CASTLE_STEALTH_ENTR:
                    {
                        return OOT_CASTLE_STEALTH_FROM_COURTYARD_ENTR;
                    }
                    break;
                }
                break;
            }

            case OOT_HYRULE_CASTLE:
            {
                switch (ID)
                {
                    case 0x23d:
                    {   // Castle Courtyard -> Hyrule Castle

                        return OOT_CASTLE_STEALTH_FROM_COURTYARD_ENTR;
                    }

                    case OOT_CASTLE_STEALTH_ENTR:
                    {   // Hyrule Castle -> Castle Courtyard

                        return OOT_CASTLE_COURTYARD_ENTR;
                    }
                }
                break;
            }

            case OOT_MARKET_CHILD_DAY:
            case OOT_MARKET_CHILD_NIGHT:
            case OOT_MARKET_ADULT:
            {
                *SceneID = OOT_MARKET;
                break;
            }

            case OOT_BACK_ALLEY_DAY:
            case OOT_BACK_ALLEY_NIGHT:
            {
                *SceneID = OOT_BACK_ALLEY;
                break;
            }

            case OOT_TEMPLE_OF_TIME_EXTERIOR_CHILD_DAY:
            case OOT_TEMPLE_OF_TIME_EXTERIOR_CHILD_NIGHT:
            case OOT_TEMPLE_OF_TIME_EXTERIOR_ADULT:
            {
                *SceneID = OOT_TEMPLE_OF_TIME_ENTRYWAY;
                break;
            }

            case OOT_MARKET_ENTRANCE_CHILD_DAY:
            case OOT_MARKET_ENTRANCE_CHILD_NIGHT:
            case OOT_MARKET_ENTRANCE_ADULT:
            {
                *SceneID = OOT_MARKET_ENTRANCE;
                break;
            }
        }
    }

    return ID;
}


uint32_t EntranceHelper::CheckWrapScene(uint8_t Game, uint32_t ID, uint32_t * SceneID, uint32_t X, uint32_t Y, uint32_t Z)
{
    float x, y, z;
    memcpy(&x, &X, sizeof(float));
    memcpy(&y, &Y, sizeof(float));
    memcpy(&z, &Z, sizeof(float));

    if (Game == OOT_GAME)
    {

    }
    else if (Game == MM_GAME)
    {
        for (size_t i = 0; i < SpecialWarps.size(); i++)
        {
            GrottoEntrance currEntrance = SpecialWarps[i];
            if (x == currEntrance.SpawnPos[0] && y == currEntrance.SpawnPos[1] && z == currEntrance.SpawnPos[2])
            {   // The respawn coordinates match the grotto entrance

                ID = SpecialWarps[i].EntranceID;
                switch (ID)
                {
                    case MM_DEKU_KING_CAUGHT:
                    {
                        *SceneID = MM_DEKU_KING_CHAMBER;
                        break;
                    }

                    case MM_PIRATE_ENTRANCE_CAUGHT:
                    {
                        *SceneID = MM_PIRATE_FORTRESS_ENTRANCE;
                        break;
                    }
                }
            }
        }
    }

    return ID;
}


void EntranceHelper::ParseEntranceMessage(uint32_t EntranceFlag, uint32_t Buffer[6])
{
    if ((EntranceFlag & 0xFFFFFF00) == IN_MAGIC)
    {
        this->ParseIncomingMessage(Buffer);
    }
    else if ((EntranceFlag & 0xFFFFFF00) == OUT_MAGIC)
    {
        this->ParseOutgoingMessage((uint8_t)EntranceFlag, Buffer);
    }
}


void EntranceHelper::ParseIncomingMessage(uint32_t Buffer[6])
{
    if (this->IsEntranceTouched && Buffer[1] != WARP_SCENE)
    {
        uint8_t game = Buffer[0] & 0xFF;
        uint8_t currRoom = (Buffer[0] >> 16) & 0xFF;
        uint8_t grottoData = (Buffer[0] >> 8) & 0xFF;
        uint32_t inScene = Buffer[1];
        uint32_t inEntrance = Buffer[2];
        EntranceMetaInfo entranceMeta = {};

        if (this->IsGrottoEntrance(inEntrance))
        {
            inEntrance = this->GetGrottoEntrance(game, grottoData, inEntrance, inScene);
        }
        else if (this->IsGrottoExit(inEntrance))
        {
            inEntrance = this->GetGrottoExit(game, currRoom, grottoData, inScene);
        }
        else
        {
            inEntrance = this->CheckGrottoSpawn(inEntrance, Buffer);
            inEntrance = this->CheckSpecialCase(game, inEntrance, &inScene);
        }

        if (game == OOT_GAME)
        {
            entranceMeta = OoTEntrances.at(inEntrance);
        }
        else
        {
            entranceMeta = MMEntrances.at(inEntrance);
        }

        this->EntranceStr = entranceMeta.FromName + std::string(" -> ") + entranceMeta.ToName;
        MultiLogger::LogMessage("New scene Loaded ! From : %s (0x%X), To : %s (0x%X)", this->LastTouchedStr.c_str(), this->OutEntrance, this->EntranceStr.c_str(), inEntrance);
        
        if (this->OutMetaInf->Type == EntranceType::One_Way_In)
        {
            MultiLogger::LogMessage("Warning ! Entrance %s (0x%X) is one way in only !", this->LastTouchedStr.c_str(), this->OutEntrance);
        }
        if (entranceMeta.Type == EntranceType::One_Way_Out)
        {
            MultiLogger::LogMessage("Warning ! Entrance %s (0x%X) is one way out only !", this->EntranceStr.c_str(), inEntrance);
        }

        if (this->OutMetaInf->Type == EntranceType::One_Way_Out)
        {
            this->OutScene = this->OutMetaInf->ToSceneID;
            this->OutEntrance = this->OutMetaInf->ToEntranceID;
        }
        else
        {
           /* if (game == OOT_GAME)
            {
                EntranceMetaInfo entranceOutMeta = OoTEntrances.at(inEntrance);
            }
            else
            {
                EntranceMetaInfo entranceOutMeta = MMEntrances.at(inEntrance);
            }

            SceneEntranceMetaInf* tmp = GetSceneEntranceMetaInf(this->OutGame, this->OutMetaInf->ToSceneID);
            */
            this->OutScene = this->OutMetaInf->FromSceneID;
            this->OutEntrance = this->OutMetaInf->FromEntranceID;
        }

        SceneEntranceMetaInf * tmp = GetSceneEntranceMetaInf(this->OutGame, this->OutScene);
        /*if (tmp->EntranceIDs.contains(this->OutEntrance))
        {
            EntranceLink tmpOutLink = tmp->EntranceIDs.find(this->OutEntrance)->second;
        }
        else
        {
            MultiLogger::LogMessage("Asked Key = 0x%08X", this->OutEntrance);
            auto it = tmp->EntranceIDs.begin();
            while (it != tmp->EntranceIDs.end())
            {
                MultiLogger::LogMessage("Key = 0x%08X, Val = 0x%08X", it->first, &it->second);
                it++;
            }
            MultiLogger::LogMessage("Nope !");
            return;
        }*/
        EntranceLink * tmpOutLink = &tmp->EntranceIDs.find(this->OutEntrance)->second;
        tmpOutLink->OutLink = inEntrance;
        tmpOutLink->OutLinkGame = game;

        SceneEntranceUpdate tmpOut = { this->OutGame, this->OutScene, this->OutEntrance, tmpOutLink };

        tmp = GetSceneEntranceMetaInf(game, inScene);
        EntranceLink * tmpInLink = &tmp->EntranceIDs.find(inEntrance)->second;
        tmpInLink->InLink = this->OutEntrance;
        tmpInLink->InLinkGame = this->OutGame;

        SceneEntranceUpdate tmpIn = { game, inScene, inEntrance, tmpInLink };

        emit MultiLogger::GetLogger()->NotifyEntranceFound(&tmpOut, &tmpIn);

        this->OutMetaInf = NULL;
    }

    this->IsEntranceTouched = false;
}


void EntranceHelper::ParseOutgoingMessage(uint8_t OwlID, uint32_t Buffer[6])
{
    if (Buffer[1] == WARP_SCENE && Buffer[2] != WARP_LOADING)
    {   // On MM when warping, the scene is always equal to WARP_SCENE so we still want to be able to catch the WARP_SONG events. However we want to get rid of new clock day

        this->IsEntranceTouched = false;
        return;
    }

    this->IsEntranceTouched = true;
    this->OutGame = Buffer[0] & 0xFF;
    uint8_t songIndex = (Buffer[0] >> 24) & 0xFF;
    uint8_t currRoom = (Buffer[0] >> 16) & 0xFF;
    uint8_t grottoData = (Buffer[0] >> 8) & 0xFF;
    this->OutScene = Buffer[1];
    this->OutEntrance = Buffer[2];

    if (this->IsGrottoEntrance(this->OutEntrance))
    {
        this->OutEntrance = this->GetGrottoEntrance(this->OutGame, grottoData, this->OutEntrance, this->OutScene);
    }
    else if (this->IsGrottoExit(this->OutEntrance))
    {
        this->OutEntrance = this->GetGrottoExit(this->OutGame, currRoom, grottoData, this->OutScene);
    }
    else if (this->IsWarpEntrance(this->OutEntrance))
    {
        this->OutEntrance = this->GetWarpSong(&this->OutGame, this->OutEntrance, songIndex, OwlID);
        this->OutEntrance = this->CheckWrapScene(this->OutGame, this->OutEntrance, &this->OutScene, Buffer[3], Buffer[4], Buffer[5]);
    }
    else
    {
        this->OutEntrance = this->CheckSpecialCase(this->OutGame, this->OutEntrance, &this->OutScene);
    }

    if (this->OutGame == OOT_GAME)
    {
        this->OutMetaInf = &OoTEntrances.at(this->OutEntrance);
    }
    else
    {
        this->OutMetaInf = &MMEntrances.at(this->OutEntrance);
    }
    this->LastTouchedStr = OutMetaInf->FromName + std::string(" -> ") + OutMetaInf->ToName;
    //MultiLogger::LogMessage("Loading zone %s (0x%X) touched !", this->LastTouchedStr.c_str(), this->LastTouchedEntranceID);
}


const char* EntranceHelper::GetEntranceFromName(int Game, uint32_t EntranceID)
{
    if (Game == OOT_GAME)
    {
        return OoTEntrances.at(EntranceID).FromName;
    }
    else
    {
        return MMEntrances.at(EntranceID).FromName;
    }
}


const char* EntranceHelper::GetEntranceToName(int Game, uint32_t EntranceID)
{
    if (Game == OOT_GAME)
    {
        return OoTEntrances.at(EntranceID).ToName;
    }
    else
    {
        return MMEntrances.at(EntranceID).ToName;
    }
}


std::string EntranceHelper::GetOneWayInName(int Game, uint32_t EntranceID)
{
    EntranceMetaInfo* entrance = nullptr;

    if (Game == OOT_GAME)
    {
        entrance = &OoTEntrances.at(EntranceID);
    }
    else
    {
        entrance = &MMEntrances.at(EntranceID);
    }

    switch (entrance->Type)
    {
        case EntranceType::Normal:
        {   // We need to build the string

            return std::string(entrance->FromName + std::string(" -> ") + entrance->ToName);
        }

        default:
        {
            return std::string(entrance->ToName);
        }
    }
}



std::string EntranceHelper::GetOneWayOutName(int Game, uint32_t EntranceID)
{
    EntranceMetaInfo* entrance = nullptr;

    if (Game == OOT_GAME)
    {
        entrance = &OoTEntrances.at(EntranceID);
    }
    else
    {
        entrance = &MMEntrances.at(EntranceID);
    }

    switch (entrance->Type)
    {
        case EntranceType::Normal:
        {   // We need to build the string

            return std::string(entrance->ToName + std::string(" - ") + entrance->FromName);
        }

        default:
        {
            return std::string(entrance->ToName);
        }
    }
}


std::string EntranceHelper::GetEntranceSpawnsString(int Game, uint32_t EntranceID)
{
    const EntranceMetaInfo* entrance = nullptr;
    
    if (Game == OOT_GAME)
    {
        entrance = &OoTEntrances.at(EntranceID);
    }
    else
    {
        entrance = &MMEntrances.at(EntranceID);
    }

    switch (entrance->Type)
    {
        case EntranceType::One_Way_Out:
        {   // We need to build the string

            return std::string(entrance->FromName + std::string(" -> ") + entrance->ToName);
        }

        default:
        {
            return std::string(entrance->ToName + std::string(" -> ") + entrance->FromName);
        }
    }

}


std::string EntranceHelper::GetEntranceLeadsString(int Game, uint32_t EntranceID)
{
    const EntranceMetaInfo* entrance = nullptr;

    if (Game == OOT_GAME)
    {
        entrance = &OoTEntrances.at(EntranceID);
    }
    else
    {
        entrance = &MMEntrances.at(EntranceID);
    }

    return std::string(entrance->ToName + std::string(" - ") + entrance->FromName);
}

const EntranceMetaInfo* EntranceHelper::GetEntranceMetaInf(int Game, uint32_t EntranceID)
{
    if (Game == OOT_GAME)
    {
        return &OoTEntrances.at(EntranceID);
    }
    else
    {
        return &MMEntrances.at(EntranceID);
    }
}