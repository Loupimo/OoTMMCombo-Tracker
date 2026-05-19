#include "Combo/Entrances.h"
#include "Combo/OoTEntrances.h"
#include "Combo/MMEntrances.h"
#include "UI/SceneEntrance.h"
#include "Multi/Multi.h"
#include <math.h>
#include <set>

// Alternative on-image anchor for MM_GROTTO_EXIT_BEAN in MM_DEKU_PALACE when the active layout
// is mm_jp. The single entry kept in MMEntrances uses the mm coordinates; GetEntranceAnchorPos
// substitutes these when rendering on the JP layout image.
const int MM_BEAN_GROTTO_JP_ANCHOR[3] = { 544, 379, 0 };

void InitializeEntranceCosts()
{
    auto FillForGame = [](std::map<int, EntranceMetaInfo>& Map)
    {
        // 1) Collect the set of entrance IDs that physically live in each scene.
        //    Each entry describes a transition From -> To, where FromEntranceID
        //    lives in FromSceneID and ToEntranceID lives in ToSceneID.
        std::map<uint32_t, std::set<uint32_t>> EntrancesInScene;
        for (auto& Pair : Map)
        {
            const EntranceMetaInfo& V = Pair.second;
            EntrancesInScene[V.FromSceneID].insert(V.FromEntranceID);
            EntrancesInScene[V.ToSceneID].insert(V.ToEntranceID);
        }

        // 2) For each entry, populate its Cost table with every other entrance
        //    in the same scene. Default cost is 1 for now; real travel times will
        //    replace this later, with UINT32_MAX meaning "unreachable".
        for (auto& Pair : Map)
        {
            EntranceMetaInfo& V = Pair.second;
            V.Cost.EntranceID = V.FromEntranceID;
            V.Cost.Costs.clear();
            auto It = EntrancesInScene.find(V.FromSceneID);
            if (It == EntrancesInScene.end()) continue;
            for (uint32_t Other : It->second)
            {
                if (Other == V.FromEntranceID) continue;
                V.Cost.Costs[Other] = 1;
            }
        }
    };

    FillForGame(OoTEntrances);
    FillForGame(MMEntrances);
}


/*
*   Look up the EntranceMetaInfo associated with an entrance ID in the per-game map. Entrance IDs
*   are unique within a single game (OoT and MM are independent namespaces), so a plain map find()
*   is enough. Returns nullptr when the ID does not exist.
*/
static const EntranceMetaInfo* LookupEntrance(int Game, uint32_t EntranceID)
{
    const std::map<int, EntranceMetaInfo>& Entrances = (Game == OOT_GAME) ? OoTEntrances : MMEntrances;
    auto It = Entrances.find(static_cast<int>(EntranceID));
    return It != Entrances.end() ? &It->second : nullptr;
}

void EntranceMessage::SetMessage(uint32_t MsgDirection, uint32_t OwlID, uint32_t Buffer[6])
{
    this->ResetMessage();
    this->Buffer = Buffer;
    this->Direction = MsgDirection;
    this->Age = (LinkAge)(OwlID >> 16);
    this->FaroreWind = (uint8_t)(OwlID >> 8);
    this->OwlID = (uint8_t)OwlID;
    this->GameID = this->Buffer[0] & 0xFF;
    this->OoTSongID = (OoTSongs)((Buffer[0] >> 24) & 0xFF);
    this->CurrRoom = (this->Buffer[0] >> 16) & 0xFF;
    this->GrottoData = (this->Buffer[0] >> 8) & 0xFF;
    this->CurrSceneID = (uint16_t)(this->Buffer[1] >> 16);
    this->SceneID = (uint16_t)this->Buffer[1];
    this->EntranceID = this->Buffer[2];
    memcpy(&this->X, &this->Buffer[3], sizeof(float));
    memcpy(&this->Y, &this->Buffer[4], sizeof(float));
    memcpy(&this->Z, &this->Buffer[5], sizeof(float));

    if (this->GameID == MM_GAME)
    {
        this->Age = (LinkAge)((uint8_t)this->Age + (uint8_t)LinkAge::Child);
    }
}

void EntranceMessage::ResetMessage()
{
    this->Buffer = nullptr;
    this->Direction = 0;
    this->GameID = NO_GAME;
    this->OoTSongID = OoTSongs::Minuet_of_Forest;
    this->Age = LinkAge::Adult;
    this->FaroreWind = 0;
    this->OwlID = 0;
    this->CurrRoom = 0;
    this->GrottoData = 0;
    this->CurrSceneID = UINT16_MAX;
    this->SceneID = UINT32_MAX;
    this->EntranceID = UINT32_MAX;
    this->X = 0;
    this->Y = 0;
    this->Z = 0;
    this->MetaInf = nullptr;
    this->EntranceStr = "";
}

// Contains all grotto entrances positions used to determine the correct entrance when spawning in a zone that has at least one grotto
static const std::map<int, std::vector<GrottoEntrance>> GrottoEntrances =
{ 

#pragma region OoT Grottos

    { OOT_KOKIRI_FOREST_FROM_LOST_WOODS_ENTR,       std::vector<GrottoEntrance>() = { { OOT_KOKIRI_FOREST_FROM_LOST_WOODS_ENTR,                -314,   380, -1362 },
                                                                                      { OOT_FOREST_FROM_LOST_WOODS_BRIDGE_ENTR,               -1413,   -74,  -283 },
                                                                                      { OOT_KOKIRI_FOREST_FROM_DEKU_TREE_ENTR,                 3844,  -161, -1080 },
                                                                                      { OOT_KOKIRI_FOREST_FROM_MIDO_ENTR,                      -445,     0,  -486 },
                                                                                      { OOT_KOKIRI_FOREST_FROM_SARIA_ENTR,                      516,     0,   629 },
                                                                                      { OOT_KOKIRI_FOREST_FROM_TWINS_ENTR,                     1036,     0,   524 },
                                                                                      { OOT_KOKIRI_FOREST_FROM_KNOW_IT_ALL_ENTR,              -1034,   120,   394 },
                                                                                      { OOT_KOKIRI_FOREST_FROM_SHOP_ENTR,                       854,     0,  -272 },
                                                                                      { OOT_KOKIRI_FOREST_FROM_LINK_ENTR,                       -31,   100,  1073 },
                                                                                      { OOT_GROTTO_EXIT_GENERIC_KOKIRI_FOREST,                 -512,   380, -1224 } } },
    { OOT_LOST_WOODS_FROM_KOKIRI_FOREST_ENTR,       std::vector<GrottoEntrance>() = { { OOT_LOST_WOODS_FROM_KOKIRI_FOREST_ENTR,                  -1,     0,   305 },
                                                                                      { OOT_LOST_WOODS_FROM_MEADOW_ENTR,                        802,     0, -2589 },
                                                                                      { OOT_LOST_WOODS_BRIDGE_FROM_FOREST_ENTR,                -901,  -200,  1602 },
                                                                                      { OOT_LOST_WOODS_BRIDGE_FROM_FIELD_ENTR,                -1502,  -200,  1600 },
                                                                                      { OOT_LOST_WOODS_FROM_ZORA_RIVER_ENTR,                   2134,  -196,  -851 },
                                                                                      { OOT_LOST_WOODS_FROM_GORON_CITY_ENTR ,                   797,   -15, -1091 },
                                                                                      { OOT_GROTTO_EXIT_GENERIC_LOST_WOODS,                     915,     0,  -925 },
                                                                                      { OOT_GROTTO_EXIT_SCRUB_UPGRADE,                          670,     0, -2520 },
                                                                                      { OOT_GROTTO_EXIT_DEKU_THEATER,                            80,   -20, -1600 } } },
    { OOT_SACRED_FOREST_MEADOW_ENTR,                std::vector<GrottoEntrance>() = { { OOT_SACRED_FOREST_MEADOW_ENTR,                         -201,     0,  2183 },
                                                                                      { OOT_WARP_SONG_MEADOW_ENTR,                               10,   500, -2610 },
                                                                                      { OOT_SACRED_MEADOW_FROM_TEMPLE_FOREST_ENTR,               -2,   680, -3180 },
                                                                                      { OOT_GROTTO_EXIT_FAIRY_SFM,                               45,     0,   220 },
                                                                                      { OOT_GROTTO_EXIT_WOLFOS,                                -195,     0,  1900 },
                                                                                      { OOT_GROTTO_EXIT_SCRUBS2_SFM,                            310,   480, -2300 } } },
    { OOT_KAKARIKO_FROM_FIELD_ENTR,                 std::vector<GrottoEntrance>() = { { OOT_KAKARIKO_FROM_FIELD_ENTR,                         -2638,   138,  1065 },
                                                                                      { OOT_KAKARIKO_FROM_GRANNY_ENTR,                          780,   200,    24 },
                                                                                      { OOT_KAKARIKO_FROM_BOTTOM_OF_THE_WELL_ENTR,              846,  -500,   520 },
                                                                                      { OOT_KAKARIKO_FROM_DEATH_MOUNTAIN_ENTR,                  -97,   511, -1787 },
                                                                                      { OOT_KAKARIKO_FROM_GRAVEYARD_ENTR,                      1901,   189,  1385 },
                                                                                      { OOT_KAKARIKO_FROM_CARPENTER_ENTR,                       -51,     0,   224 },
                                                                                      { OOT_KAKARIKO_FROM_SKULLTULA_ENTR,                      -287,    80,  1017 },
                                                                                      { OOT_KAKARIKO_FROM_IMPA_ENTR,                           -373,   240,  1702 },
                                                                                      { OOT_KAKARIKO_FROM_IMPA_BACK_ENTR,                       260,   240,  1730 },
                                                                                      { OOT_KAKARIKO_FROM_WINDMILL_ENTR,                       1183,   260,   523 },
                                                                                      { OOT_KAKARIKO_FROM_SHOP_POTION_ENTR,                     119,   320,  -567 },
                                                                                      { OOT_KAKARIKO_FROM_SHOP_POTION_BACK_ENTR,                592,   320,  -586 },
                                                                                      { OOT_KAKARIKO_FROM_BAZAAR_ENTR,                          -191,  320,  -650 },
                                                                                      { OOT_KAKARIKO_FROM_ARCHERY_ENTR,                         323,   160,   784 },
                                                                                      { OOT_VILLAGE_OWL_ENTR,                                   -65,   620,  1520 },
                                                                                      { OOT_GROTTO_EXIT_REDEAD,                                -400,     0,   400 },
                                                                                      { OOT_GROTTO_EXIT_GENERIC_KAKARIKO,                       860,    80,  -260 } } },
    { OOT_LAKE_HYLIA_FROM_FIELD_ENTR,               std::vector<GrottoEntrance>() = { { OOT_LAKE_HYLIA_FROM_FIELD_ENTR,                       -2093, -1037,   705 },
                                                                                      { OOT_WARP_SONG_LAKE_ENTR,                              -1045, -1223,  7460 },
                                                                                      { OOT_LAKE_HYLIA_FROM_LABORATORY_ENTR,                  -2422, -1033,  3858 },
                                                                                      { OOT_LAKE_HYLIA_FROM_VALLEY_ENTR,                      -3918, -1142,  2533 },
                                                                                      { OOT_LAKE_HYLIA_FROM_FISHING_POND_ENTR,                 1322, -1218,  3871 },
                                                                                      { OOT_LAKE_HYLIA_FROM_TEMPLE_WATER_ENTR,                 -929, -1313,  6555 }, // Child
                                                                                      { OOT_LAKE_HYLIA_FROM_TEMPLE_WATER_ENTR,                 -917, -2201,  6359 }, // Adult
                                                                                      { OOT_LAKE_HYLIA_FROM_ZORA_DOMAIN_ENTR,                  -912, -1544,  3759 },
                                                                                      { OOT_GROTTO_EXIT_SCRUBS3_LAKE,                         -3040, -1033,  6075 } } },
    { OOT_DEATH_MOUNTAIN_FROM_KAKARIKO_ENTR,        std::vector<GrottoEntrance>() = { { OOT_DEATH_MOUNTAIN_FROM_KAKARIKO_ENTR,                -1344,   118,  2221 },
                                                                                      { OOT_DEATH_MOUNTAIN_FROM_GORON_CITY_ENTR,               -407,  1191, -1860 },
                                                                                      { OOT_MOUNTAIN_TRAIL_FROM_DODONGO_CAVERN_ENTR,          -1695,   683,  -535 },
                                                                                      { OOT_TRAIL_SUMMIT_FROM_CRATER_ENTR,                      230,  3263, -4670 },
                                                                                      { OOT_DEATH_MOUNTAIN_FROM_FAIRY_ENTR,                    -206,  3295, -4391 },
                                                                                      { OOT_GROTTO_EXIT_TRAIL_COW,                             -688,  1946,  -285 },
                                                                                      { OOT_GROTTO_EXIT_GENERIC_DMT,                           -383,  1386, -1206 } } },
    { OOT_DEATH_MOUNTAIN_CRATER_ENTR,               std::vector<GrottoEntrance>() = { { OOT_DEATH_MOUNTAIN_CRATER_ENTR,                       -1121,  1360,  2076 },
                                                                                      { OOT_CRATER_FROM_GORON_CITY_ENTR,                      -1749,   722,    26 },
                                                                                      { OOT_DEATH_CRATER_FROM_FAIRY_ENTR,                     -1287,   829,   941 },
                                                                                      { OOT_DEATH_CRATER_FROM_TEMPLE_FIRE_ENTR,                  12,  -350, -1419 },
                                                                                      { OOT_WARP_SONG_CRATER_ENTR,                                0,   441,     0 },
                                                                                      { OOT_GROTTO_EXIT_GENERIC_DMC,                             40,  1233,  1770 },
                                                                                      { OOT_GROTTO_EXIT_SCRUBS3_DMC,                          -1699,   722,  -472 } } },
    { OOT_GORON_CITY_ENTR,                          std::vector<GrottoEntrance>() = { { OOT_GORON_CITY_ENTR,                                     56,   600,  1104 },
                                                                                      { OOT_GORON_CITY_FROM_LOST_WOODS_ENTR,                    441,   196,  1153 },
                                                                                      { OOT_GORON_CITY_FROM_CRATER_ENTR,                         47,    40, -1523 },
                                                                                      { OOT_GORON_CITY_FROM_SHOP_ENTR,                         -134,    -3,   -42  },
                                                                                      { OOT_GROTTO_EXIT_SCRUBS3_GORON_CITY,                    1100,   580, -1190 } } },
    { OOT_ZORA_RIVER_FROM_FIELD_ENTR,               std::vector<GrottoEntrance>() = { { OOT_ZORA_RIVER_FROM_FIELD_ENTR,                       -1506,   -20,  1510 },
                                                                                      { OOT_ZORA_RIVER_FROM_FIELD_WATER_ENTR,                 -1357,   -86,  1542 },
                                                                                      { OOT_ZORA_RIVER_FROM_LOST_WOODS_ENTR,                   4084,   620,  -932 },
                                                                                      { OOT_RIVER_FROM_DOMAIN_ENTR,                            4413,   920, -1403 },
                                                                                      { OOT_GROTTO_EXIT_GENERIC_RIVER,                          360,   570,   130 },
                                                                                      { OOT_GROTTO_EXIT_SCRUBS2_RIVER,                        -1630,   100,  -130 },
                                                                                      { OOT_GROTTO_EXIT_SCRUBS2_RIVER,                        -1630,   100,  -130 } } },
    { OOT_ZORA_DOMAIN_ENTR,                         std::vector<GrottoEntrance>() = { { OOT_ZORA_DOMAIN_ENTR,                                 -1112,   210,  -160 },
                                                                                      { OOT_ZORA_DOMAIN_FROM_LAKE_HYLIA_ENTR,                  -208,  -210,  -280 },
                                                                                      { OOT_DOMAIN_FROM_FOUNTAIN_ENTR,                          524,    52,   254 },
                                                                                      { OOT_ZORA_DOMAIN_FROM_SHOP_ENTR,                         540,   996, -2503 },
                                                                                      { OOT_GROTTO_EXIT_FAIRY_DOMAIN,                          -860,    14,  -470 } } },
    { OOT_LON_LON_RANCH_FROM_FIELD_ENTR,            std::vector<GrottoEntrance>() = { { OOT_LON_LON_RANCH_FROM_FIELD_ENTR,                     1107,     0, -3740 },
                                                                                      { OOT_LON_LON_RANCH_FROM_HOUSE_ENTR,                      988,     1, -3097 },
                                                                                      { OOT_LON_LON_RANCH_FROM_STABLES_ENTR,                    728,     1, -2896 },
                                                                                      { OOT_LON_LON_RANCH_FROM_SILO_ENTR,                     -1506,     0,  1491 },
                                                                                      { OOT_GROTTO_EXIT_SCRUBS3_RANCH,                         1800,     0,  1500 } } },
    { OOT_HYRULE_CASTLE_ENTR,                       std::vector<GrottoEntrance>() = { { OOT_HYRULE_CASTLE_ENTR,                                -225,  1086,  3743 },
                                                                                      { OOT_HYRULE_CASTLE_FROM_FAIRY_ENTR,                     2627,  1142,  2567 },
                                                                                      { OOT_CASTLE_STEALTH_FROM_COURTYARD_ENTR,                 575,  1617,  -310 },
                                                                                      { OOT_CASTLE_CAUGHT_ENTR,                                 913,  1109,  2937 },
                                                                                      { OOT_GROTTO_EXIT_CASTLE,                                 996,  1571,   844 } } },
    { OOT_FIELD_FROM_MARKET_ENTRANCE_ENTR,          std::vector<GrottoEntrance>() = { { OOT_FIELD_FROM_MARKET_ENTRANCE_ENTR,                      1,     0,   697 },      // Child
                                                                                      { OOT_FIELD_FROM_MARKET_ENTRANCE_ENTR,                     -6,     0,  1182 },      // Adult
                                                                                      { OOT_FIELD_FROM_LAKE_HYLIA_ENTR,                       -5891,  -881, 15676 },
                                                                                      { OOT_FIELD_FROM_GERUDO_VALLEY_ENTR,                    -9645,  -155,  6850 },
                                                                                      { OOT_FIELD_FROM_KAKARIKO_ENTR,                          3759,   252,   232 },
                                                                                      { OOT_FIELD_FROM_ZORA_RIVER_WATER_ENTR,                  5977,  -135,  3982 },
                                                                                      { OOT_FIELD_FROM_ZORA_RIVER_ENTR,                        5984,   -20,  3773 },
                                                                                      { OOT_FIELD_FROM_LOST_WOODS_BRIDGE_ENTR,                 5137,  -160,  8467 },
                                                                                      { OOT_FIELD_FROM_LON_LON_RANCH_ENTR,                    -1850,   196,  5956 },
                                                                                      { OOT_FIELD_FROM_LON_LON_EPONA_JUMP_LEFT,               -1391,   100,  6865 },
                                                                                      { OOT_FIELD_FROM_LON_LON_EPONA_JUMP_MIDDLE,             -2953,   100,  8336 },
                                                                                      { OOT_FIELD_FROM_LON_LON_EPONA_JUMP_RIGHT,              -4668,  -122,  6865 },
                                                                                      { OOT_FIELD_OWL_ENTR,                                       0,    70,  1380 },
                                                                                      { OOT_GROTTO_EXIT_GENERIC_HF_MARKET,                    -1425,     0,   810 },
                                                                                      { OOT_GROTTO_EXIT_FAIRY_HF,                             -4450,  -300,  -425 },
                                                                                      { OOT_GROTTO_EXIT_TEKTITE,                              -4945,  -300,  2835 },
                                                                                      { OOT_GROTTO_EXIT_GENERIC_HF_SOUTHEAST,                  -270,  -500, 12350 },
                                                                                      { OOT_GROTTO_EXIT_GENERIC_HF_OPEN,                      -4030,  -700, 13860 },
                                                                                      { OOT_GROTTO_EXIT_SCRUB_HEART_PIECE,                    -4990,  -700, 13820 },
                                                                                      { OOT_GROTTO_EXIT_FIELD_COW,                            -7870,  -300,  6920 },
                                                                                      { OOT_GROTTO_EXIT_FIELD_TREE,                            2060,    20,  -170 } } },
    { OOT_GERUDO_VALLEY_FROM_FIELD_ENTR,            std::vector<GrottoEntrance>() = { { OOT_GERUDO_VALLEY_FROM_FIELD_ENTR,                     2664,  -269,   778 },
                                                                                      { OOT_GERUDO_VALLEY_FROM_TENT_ENTR,                      -973,    15,  -362 },
                                                                                      { OOT_VALLEY_FROM_GERUDO_FORTRESS_ENTR,                 -3264,   239,  -757 },
                                                                                      { OOT_GERUDO_FORTRESS_CAUGHT_NO_HOOK_ENTR,                187, -2828,  2447 },
                                                                                      { OOT_GROTTO_EXIT_OCTOROK,                                280,  -555,  1470 },
                                                                                      { OOT_GROTTO_EXIT_SCRUBS2_VALLEY,                       -1323,    15,  -969 } } },
    { OOT_GERUDO_FORTRESS_FROM_VALLEY_ENTR,         std::vector<GrottoEntrance>() = { { OOT_GERUDO_FORTRESS_FROM_VALLEY_ENTR,                  -842,     3,   -84 },
                                                                                      { OOT_GERUDO_FORTRESS_FROM_GERUDO_TRAINING_ENTR,           40,   333, -1022 },
                                                                                      { OOT_FORTRESS_FROM_WASTELAND_ENTR,                     -1786,    12, -3382 },
                                                                                      { OOT_GERUDO_FORTRESS_CAUGHT_ENTR,                        188,   733, -2919 },
                                                                                      { OOT_FORTRESS_FROM_HIDEOUT_BREAKOUT,                     763,   640, -2662 },
                                                                                      { OOT_FORTRESS_FROM_HIDEOUT_BREAKOUT_JAIL,                314,  1113, -2982 },
                                                                                      { OOT_FORTRESS_FROM_HIDEOUT_LEFT_JAIL_1,                  200,   333, -2554 },
                                                                                      { OOT_FORTRESS_FROM_HIDEOUT_RIGHT_JAIL_1,                 410,   333, -1794 },
                                                                                      { OOT_FORTRESS_FROM_HIDEOUT_KITCHEN_BOTTOM,               626,   333, -1718 },
                                                                                      { OOT_FORTRESS_FROM_HIDEOUT_KITCHEN_BOTTOM_TO_TOP,        629,   533, -1401 },
                                                                                      { OOT_FORTRESS_FROM_HIDEOUT_KITCHEN_TOP_RIGHT,            931,   733, -1404 },
                                                                                      { OOT_FORTRESS_FROM_HIDEOUT_KITCHEN_TOP_LEFT,            1230,   834, -1769 },
                                                                                      { OOT_FORTRESS_FROM_HIDEOUT_JAIL_2_TOP,                   947,   733, -1207 },
                                                                                      { OOT_FORTRESS_FROM_HIDEOUT_JAIL_2_BOTTOM,                325,   572, -1084 },
                                                                                      { OOT_FORTRESS_FROM_HIDEOUT_JAIL_3_TOP,                   679,   533, -2056 },
                                                                                      { OOT_FORTRESS_FROM_HIDEOUT_JAIL_3_BOTTOM,                222,   333, -1347 },
                                                                                      { OOT_FORTRESS_FROM_HIDEOUT_JAIL_4,                      1249,   653, -2061 },
                                                                                      { OOT_GROTTO_EXIT_FAIRY_FORTRESS,                         376,   333, -1564 } } },
    { OOT_COLOSSUS_ENTR,                            std::vector<GrottoEntrance>() = { { OOT_COLOSSUS_ENTR,                                     3789,   -38, -1101 },
                                                                                      { OOT_DESERT_FROM_GAUNTLET_ENTR,                        -1664,   733,   593 },
                                                                                      { OOT_DESERT_FROM_MIROR_ENTR,                           -1648,   738,  -516 },
                                                                                      { OOT_DESERT_COLOSSUS_FROM_FAIRY_ENTR,                   2260,   105, -1425 },
                                                                                      { OOT_DESERT_COLOSSUS_FROM_FAIRY_ENTR,                   2260,   105, -1425 },
                                                                                      { OOT_DESERT_COLOSSUS_FROM_TEMPLE_SPIRIT_ENTR,          -1548,    48,     2 },
                                                                                      { OOT_WARP_SONG_DESERT_ENTR,                             -850,    20,  1542 },
                                                                                      { OOT_GROTTO_EXIT_SCRUBS2_COLOSSUS,                        60,   -32, -1300 } } },

#pragma endregion // OoT Grottos

#pragma region MM Grottos                                                                     
                                                                                      
    { MM_TERMINA_FIELD_FROM_CLOCK_TOWN_SOUTH_ENTR,  std::vector<GrottoEntrance>() = { { MM_TERMINA_FIELD_FROM_CLOCK_TOWN_NORTH_ENTR,           -400,    48, -2520 },
                                                                                      { MM_TERMINA_FIELD_FROM_CLOCK_TOWN_WEST_ENTR,           -2400,    68,  -400 },
                                                                                      { MM_TERMINA_FIELD_FROM_CLOCK_TOWN_EAST_ENTR,            1672,    68,  -394 },
                                                                                      { MM_TERMINA_FIELD_FROM_CLOCK_TOWN_SOUTH_ENTR,           -412,   -77,  1681 },
                                                                                      { MM_TERMINA_FIELD_FROM_GREAT_BAY_COAST_ENTR,           -6823,  -378,  -719 },
                                                                                      { MM_TERMINA_FIELD_FROM_ROAD_TO_IKANA_ENTR,              4588,    -2,  -819 },
                                                                                      { MM_TERMINA_FIELD_FROM_PATH_TO_MOUNTAIN_VILLAGE_ENTR,   -717,   300, -5514 },
                                                                                      { MM_TERMINA_FIELD_FROM_ROAD_TO_SWAMP_ENTR,              -293,  -222,  5636 },
                                                                                      { MM_TERMINA_FIELD_FROM_MILK_ROAD_ENTR,                 -3697,  -222,  4335 },
                                                                                      { MM_FIELD_FROM_ASTRAL_OBSERVATORY_ENTR,                 3758,   254,  1392 },
                                                                                      { MM_GROTTO_EXIT_GOSSIPS_MOUNTAIN,                        192,    48, -3138 },
                                                                                      { MM_GROTTO_EXIT_DODONGO,                               -2425,  -281, -3291 },
                                                                                      { MM_GROTTO_EXIT_GOSSIPS_OCEAN,                         -2782,    48, -1654 },
                                                                                      { MM_GROTTO_EXIT_BIO_BABA,                              -5159,  -281,  -571 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_FIELD_PILLAR,                   2367,   315,  -192 },
                                                                                      { MM_GROTTO_EXIT_GOSSIPS_CANYON,                         4450,   254,   925 },
                                                                                      { MM_GROTTO_EXIT_SCRUB,                                  3223,   219,  1417 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_GRASS,                          1012,  -221,  3642 },
                                                                                      { MM_GROTTO_EXIT_COW_FIELD,                              -375,  -222,  3976 },
                                                                                      { MM_GROTTO_EXIT_GOSSIPS_SWAMP,                         -1592,  -222,  4622 },
                                                                                      { MM_GROTTO_EXIT_PEAHAT,                                -2317,  -221,  3418 } } },
    { MM_SWAMP_ROAD_FROM_FIELD_ENTR,                std::vector<GrottoEntrance>() = { { MM_SWAMP_ROAD_FROM_FIELD_ENTR,                          331,  -143,   245 },
                                                                                      { MM_SWAMP_ROAD_FROM_ARCHERY_ENTR,                       2970,   -72,  3302 },
                                                                                      { MM_SWAMP_ROAD_FROM_SWAMP_ENTR,                          413,  -236,  3853 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_PATH_SWAMP,                      104,  -182,  2202 } } },
    { MM_SWAMP_FROM_SPIDER_HOUSE_ENTR,              std::vector<GrottoEntrance>() = { { MM_SWAMP_FROM_SPIDER_HOUSE_ENTR,                      -1049,    12,  2042 },
                                                                                      { MM_SWAMP_FROM_ROAD_ENTR,                               -191,    61, -1410 },
                                                                                      { MM_KOUME_RIDE_FROM_TOURIST_ENTR,                        -42,    24,  -150 },
                                                                                      { MM_SWAMP_FROM_PALACE_MAIN_ENTRANCE_ENTR,               -825,    49,  4044 },
                                                                                      { MM_SWAMP_FROM_PALACE_LEDGE_ENTR,                      -1647,   255,  1346 },
                                                                                      { MM_SWAMP_FROM_WOODFALL_ENTR,                            240,   393,  3201 },
                                                                                      { MM_SWAMP_FROM_MYSTERY_WOODS_ENTR,                      3735,    27, -2094 },
                                                                                      { MM_SWAMP_FROM_IKANA_CANYON_ENTR,                       4357,   393, -1141 },
                                                                                      { MM_SWAMP_FROM_POTION_SHOP_ENTR,                        3670,   301,  -942 },
                                                                                      { MM_SWAMP_FROM_TOURIST_INFORMATION_ENTR,                 -50,   170,    30 },
                                                                                      { MM_WARP_OWL_SOUTHERN_SWAMP_ENTR,                       -849,     4,  -295 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_SWAMP,                         -1700,    38,  1800 }} },
    { MM_MYSTERY_WOODS_ENTR,                        std::vector<GrottoEntrance>() = { { MM_MYSTERY_WOODS_ENTR,                                  274,     0,     0 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_WOODS,                             2,     0,  -889 } } },
    { MM_MOUNTAIN_VILLAGE_FROM_PATH_ENTR,           std::vector<GrottoEntrance>() = { { MM_MOUNTAIN_VILLAGE_FROM_PATH_ENTR,                    -226,  -147,  1942 },
                                                                                      { MM_MOUNTAIN_VILLAGE_FROM_SNOWHEAD_PATH_ENTR,          -1237,   268, -1041 },
                                                                                      { MM_MOUNTAIN_VILLAGE_FROM_TWIN_ISLANDS_ENTR,            2071,    15,   951 },
                                                                                      { MM_MOUNTAIN_VILLAGE_FROM_GORON_GRAVEYARD_ENTR,          464,  1408, -1519 },
                                                                                      { MM_MOUNTAIN_VILLAGE_FROM_BLACKSMITH_ENTR,                80,    71,   800 },
                                                                                      { MM_WARP_OWL_MOUNTAIN_VILLAGE_ENTR,                     -649,     8,  -196 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_MOUNTAIN_VILLAGE,               2406,  1168, -1197 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_MOUNTAIN_VILLAGE_WINTER,         345,     8,  -150 } } },
    { MM_TWIN_ISLANDS_FROM_MOUNTAIN_VILLAGE_ENTR,   std::vector<GrottoEntrance>() = { { MM_TWIN_ISLANDS_FROM_MOUNTAIN_VILLAGE_ENTR,           -2044,   200,  1288 },
                                                                                      { MM_TWIN_ISLANDS_FROM_GORON_VILLAGE_ENTR,               3574,   148,   316 },
                                                                                      { MM_TWIN_ISLANDS_FROM_GORON_RACETRACK_ENTR,             -731,   560,  -845 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_TWIN_ISLANDS,                  -1309,   320,   143 },
                                                                                      { MM_GROTTO_EXIT_HOT_WATER,                               589,   195,    53 } } },
    { MM_PATH_SNOWHEAD_FROM_SNOWHEAD_ENTR,          std::vector<GrottoEntrance>() = { { MM_PATH_SNOWHEAD_FROM_SNOWHEAD_ENTR,                  -2518,   550, -3441 },
                                                                                      { MM_SNOWHEAD_PATH_FROM_MOUNTAIN_VILLAGE_ENTR,            442,  -128,  2324 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_PATH_SNOWHEAD,                  -987,   360, -2339 } } },
    { MM_GREAT_BAY_COAST_FROM_FIELD_ENTR,           std::vector<GrottoEntrance>() = { { MM_GREAT_BAY_COAST_FROM_FIELD_ENTR,                    3585,    80,  4394 },
                                                                                      { MM_GREAT_BAY_COAST_FROM_FISHER_HUT_ENTR,               1137,    92,  4635 },
                                                                                      { MM_GREAT_BAY_COAST_FROM_LABORATORY_ENTR,              -3080,   240,  4080 },
                                                                                      { MM_GREAT_BAY_FROM_ENTRANCE_CAUGHT,                    -1783,   350, -3081 },
                                                                                      { MM_COAST_FROM_MIKAU_CS_ENTR,                            783,    80,  3586 },
                                                                                      { MM_VOID_GREAT_BAY_BY_PINNACLE_ROCK_ENTR,               1321,   -16, -1029 },
                                                                                      { MM_VOID_GREAT_BAY_ENTR,                                -804,   -17,  4442 },
                                                                                      { MM_WARP_OWL_GREAT_BAY_ENTR,                           -2881,    15,  3978 },
                                                                                      { MM_GREAT_BAY_FROM_PINNACLE_ROCK_ENTR,                 -5208,   -39,  -125 },
                                                                                      { MM_GREAT_BAY_COAST_FROM_ZORA_CAPE_ENTR,                -339,    56,  6193 },
                                                                                      { MM_GREAT_BAY_FROM_PIRATE_FORTRESS_ENTR,                -164,  -445, -3052 },
                                                                                      { MM_GREAT_BAY_FROM_SPIDER_HOUSE_ENTR,                   1470,    92,  4158 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_GREAT_BAY_COAST,                1359,    80,  5018 },
                                                                                      { MM_GROTTO_EXIT_COW_COAST,                              2077,   333,  -215 } } },
    { MM_ZORA_CAPE_FROM_GREAT_BAY_COAST_ENTR,       std::vector<GrottoEntrance>() = { { MM_ZORA_CAPE_FROM_GREAT_BAY_COAST_ENTR,                  92,    12,   333 },
                                                                                      { MM_ZORA_CAPE_FROM_HALL_WATER_ENTR,                    -4033,  -318,  1328 },
                                                                                      { MM_ZORA_CAPE_PENINSULA_ENTR,                          -5437,    14,  1616 },
                                                                                      { MM_GREAT_BAY_FROM_FAIRY_FOUNTAIN_ENTR,                -1650,   200,  4426 },
                                                                                      { MM_ZORA_CAPE_FROM_BEAVERS_ENTR,                        4131,  1255,  5079 },
                                                                                      { MM_WARP_OWL_ZORA_CAPE_ENTR,                           -5457,    14,  1929 },
                                                                                      { MM_VOID_ZORA_CAPE_ENTR,                                -256,    -8,   999 },
                                                                                      { MM_GREAT_BAY_FROM_TEMPLE_ENTR,                        -5606,    14,  1642 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_ZORA_CAPE,                      -562,    80,  2707 } } },
    { MM_IKANA_ROAD_FROM_FIELD_ENTR,                std::vector<GrottoEntrance>() = { { MM_IKANA_ROAD_FROM_FIELD_ENTR,                        -3006,     0,  -305 },
                                                                                      { MM_IKANA_ROAD_FROM_VALLEY_ENTR,                        5154,   575,  -240 },
                                                                                      { MM_IKANA_ROAD_FROM_IKANA_GRAVEYARD_ENTR,               1234,   340, -2297 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_PATH_IKANA,                     -428,   200,  -335 } } },
    { MM_IKANA_GRAVEYARD_FROM_DAMPE_ENTR,           std::vector<GrottoEntrance>() = { { MM_IKANA_GRAVEYARD_FROM_DAMPE_ENTR,                    -504,   314, -2159 },
                                                                                      { MM_IKANA_GRAVEYARD_ENTR,                                -45,   -49,   864 },
                                                                                      { MM_GRAVE_EXIT_NIGHT1,                                   192,    24,   446 },
                                                                                      { MM_GRAVE_EXIT_NIGHT2,                                   215,    64,  -113 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_GRAVEYARD,                       106,   314, -1777 } } },
    { MM_IKANA_VALLEY_FROM_ROAD_ENTR,               std::vector<GrottoEntrance>() = { { MM_IKANA_VALLEY_FROM_ROAD_ENTR,                         -64,  -371,  4870 },
                                                                                      { MM_IKANA_CANYON_FROM_WELL_ENTR,                       -1563,   128,  1548 },
                                                                                      { MM_IKANA_VALLEY_FROM_SHRINE_ENTR,                      3060,  -505,  2690 },
                                                                                      { MM_IKANA_CANYON_FROM_CASTLE_GARDENS_ENTR,               705,   201,  1681 },
                                                                                      { MM_IKANA_CANYON_FROM_STONE_TOWER_ENTR,                 1360,   823,  -220 },
                                                                                      { MM_IKANA_CANYON_FROM_GHOST_HUT_ENTR,                  -1028,   639,    93 },
                                                                                      { MM_IKANA_CANYON_FROM_MUSIC_BOX_HOUSE_ENTR,             -343,   440,   963 },
                                                                                      { MM_IKANA_CANYON_FROM_FAIRY_FOUNTAIN_ENTR,               546,   400,   858 },
                                                                                      { MM_IKANA_CANYON_FROM_CAVERN_ENTR,                      -344,   562,  -711 },
                                                                                      { MM_IKANA_CANYON_FROM_SAKON_HIDEOUT_ENTR,               2260,  -160,  4726 },
                                                                                      { MM_WARP_OWL_IKANA_CANYON_ENTR,                         -852,   200,  2700 },
                                                                                      { MM_WARP_BOSS_STONE_TOWER_INVERTED_ENTR,                 376,  1400,   136 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_VALLEY,                        -2475,  -505,  2475 } } }

#pragma endregion // MM Grottos

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
    this->OutMessage.ResetMessage();
    this->InMessage.ResetMessage();
    this->IsEntranceTouched = false;
}


bool EntranceHelper::IsNewCycle(EntranceMessage& PrevMessage, EntranceMessage& CurrMessage)
{
    if (PrevMessage.GameID == MM_GAME && CurrMessage.GameID == MM_GAME && CurrMessage.SceneID == 0)
    {
        return CurrMessage.EntranceID == PrevMessage.EntranceID;
    }
    return false;
}


bool EntranceHelper::IsMMExtra(EntranceMessage& Message)
{
    switch (Message.GameID)
    {
        case MM_GAME:
        {
            switch (Message.SceneID)
            {

                case MM_EXTRA:
                case MM_CUTSCENE_MAP:
                {
                    return true;
                }

                default:
                {
                    break;
                }
            }

            switch (Message.EntranceID)
            {   // Entrance ID

                case MM_CLOCK_TOWER_MOON_CRASH_ENTR:
                case MM_CLOCK_TOWN_FROM_SONG_OF_TIME_ENTR:
                {
                    return true;
                }

                default:
                {
                    break;
                }
            }

            break;
        }

        default:
        {
            break;
        }
    }
    return false;
}


bool EntranceHelper::IsDeath(EntranceMessage& PrevMessage, EntranceMessage& CurrMessage)
{
    return (uint8_t)PrevMessage.Age > 1;
    /*if (PrevMessage.SceneID == MM_INSIDE_CASTLE_IKANA || PrevMessage.SceneID == MM_LAIR_IKANA)
    {   // Special case for dying inside castle of Ikana

        return CurrMessage.EntranceID == MM_IKANA_CASTLE_EXTERIOR_FROM_WELL_ENTR;
    }
    else if (CurrMessage.EntranceID == OOT_TEMPLE_OF_TIME_MASTER_SWORD_CS_ENTR)
    {   // It is our only self loop allowed

        return false;
    }

    return PrevMessage.CurrRoom == CurrMessage.CurrRoom && CurrMessage.SceneID == PrevMessage.SceneID && CurrMessage.EntranceID == PrevMessage.EntranceID;*/
    //return PrevMessage.MetaInf->Type == EntranceType::Normal && CurrMessage.SceneID == PrevMessage.SceneID && CurrMessage.EntranceID == PrevMessage.EntranceID;
}


bool EntranceHelper::IsFaroreWind(EntranceMessage& Message)
{
    return Message.FaroreWind;
    /*if (PrevMessage.GameID == MM_GAME)
    {   // MM

        return PrevMessage.FaroreWind;
    }
    else
    {   // OoT

        switch (PrevMessage.SceneID)
        {
            case OOT_DEKU_TREE:
            case OOT_DODONGO_CAVERN:
            case OOT_INSIDE_JABU_JABU:
            case OOT_TEMPLE_FOREST:
            case OOT_TEMPLE_FIRE:
            case OOT_TEMPLE_WATER:
            case OOT_TEMPLE_SHADOW:
            case OOT_TEMPLE_SPIRIT:
            {
                return CurrMessage.SceneID == PrevMessage.SceneID;
            }
        }
    }

    return false;*/
}


bool EntranceHelper::IsSonataWoodfall(EntranceMessage& PrevMessage, EntranceMessage& CurrMessage)
{
    if (PrevMessage.GameID == MM_GAME && CurrMessage.GameID == MM_GAME)
    {
        return PrevMessage.MMSongID == MMSongs::Sonata_of_Awakening && PrevMessage.SceneID == MM_WOODFALL && CurrMessage.SceneID == MM_WOODFALL && PrevMessage.EntranceID == CurrMessage.EntranceID;
    }

    return false;
}


bool EntranceHelper::IsSpawn(EntranceMessage& CurrMessage)
{
    switch (CurrMessage.EntranceID)
    {
        case OOT_SPAWN_CHILD:
        case OOT_SPAWN_ADULT:
        {
            return true;
        }

    }
    return false;
}


bool EntranceHelper::IsSunSong(EntranceMessage& PrevMessage, EntranceMessage& CurrMessage)
{
    switch (CurrMessage.GameID)
    {
        case OOT_GAME:
        {
            switch (CurrMessage.SceneID)
            {   // Handle Special cases

                case OOT_MARKET_ADULT:
                case OOT_MARKET_CHILD_DAY:
                case OOT_MARKET_CHILD_NIGHT:
                {
                    return PrevMessage.OoTSongID == OoTSongs::Sun_Song && OOT_MARKET == PrevMessage.SceneID && CurrMessage.EntranceID == PrevMessage.Buffer[2];
                }

                case OOT_BACK_ALLEY_DAY:
                case OOT_BACK_ALLEY_NIGHT:
                {
                    return PrevMessage.OoTSongID == OoTSongs::Sun_Song && OOT_BACK_ALLEY == PrevMessage.SceneID && CurrMessage.EntranceID == PrevMessage.Buffer[2];
                }

                case OOT_MARKET_ENTRANCE_ADULT:
                case OOT_MARKET_ENTRANCE_CHILD_DAY:
                case OOT_MARKET_ENTRANCE_CHILD_NIGHT:
                {
                    return PrevMessage.OoTSongID == OoTSongs::Sun_Song && OOT_MARKET_ENTRANCE == PrevMessage.SceneID && CurrMessage.EntranceID == PrevMessage.Buffer[2];
                }

                case OOT_TEMPLE_OF_TIME_EXTERIOR_ADULT:
                case OOT_TEMPLE_OF_TIME_EXTERIOR_CHILD_DAY:
                case OOT_TEMPLE_OF_TIME_EXTERIOR_CHILD_NIGHT:
                {
                    return PrevMessage.OoTSongID == OoTSongs::Sun_Song && OOT_TEMPLE_OF_TIME_ENTRYWAY == PrevMessage.SceneID && CurrMessage.EntranceID == PrevMessage.Buffer[2];
                }
            }
            return PrevMessage.OoTSongID == OoTSongs::Sun_Song && CurrMessage.SceneID == PrevMessage.Buffer[1] && CurrMessage.EntranceID == PrevMessage.Buffer[2];
        }
    }

    return false;
}

bool EntranceHelper::IsSongOfTime(EntranceMessage& PrevMessage, EntranceMessage& CurrMessage)
{
    return PrevMessage.OoTSongID == OoTSongs::Song_of_Time
        && PrevMessage.Buffer[1] == CurrMessage.Buffer[1]
        && PrevMessage.Buffer[2] == CurrMessage.Buffer[2]
        && PrevMessage.Buffer[3] == CurrMessage.Buffer[3]
        && PrevMessage.Buffer[4] == CurrMessage.Buffer[4]
        && PrevMessage.Buffer[5] == CurrMessage.Buffer[5];
}

bool EntranceHelper::IsSongOfDoubleTime(EntranceMessage& PrevMessage, EntranceMessage& CurrMessage)
{
    return PrevMessage.MMSongID == MMSongs::Song_of_Double_Time && CurrMessage.SceneID == PrevMessage.Buffer[1] && CurrMessage.EntranceID == PrevMessage.Buffer[2];
}


bool EntranceHelper::IsGrottoEntrance(EntranceMessage& Message)
{
    switch (Message.EntranceID)
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


bool EntranceHelper::IsGrottoExit(EntranceMessage& Message)
{
    switch (Message.EntranceID)
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


bool EntranceHelper::IsWarpEntrance(EntranceMessage& Message)
{
    switch (Message.EntranceID)
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


uint32_t EntranceHelper::GetGrottoEntrance(EntranceMessage& Message, uint32_t LastScene)
{
    if (Message.GameID == OOT_GAME)
    {
        switch (Message.EntranceID)
        {
            case OOT_GROTTO_TYPE_GENERIC_ENTR:
            {
                switch (Message.GrottoData & 0x1f)
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

        entranceKey = (Message.EntranceID >> 9);
        switch (entranceKey)
        {
            case 0x06: entranceKey = 0x42; break;
            case 0x57: entranceKey = 0x4d; break;
            case 0x45: entranceKey = 0x4a; break;
            case 0x5b: entranceKey = 0x5a; break;
        }
        Message.EntranceID = (entranceKey << 9) | (Message.EntranceID & 0x1ff);

        switch (Message.EntranceID)
        {
            case MM_GROTTO_TYPE_GENERIC_ENTR:
            {
                switch (Message.GrottoData & 0x1f)
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


uint32_t EntranceHelper::GetGrottoExit(EntranceMessage& Message)
{
    uint8_t currRoomNum = 0;

    if (Message.GameID == OOT_GAME)
    {
        switch (Message.CurrSceneID)
        {
            case OOT_GROTTOS:
            {
                switch (Message.CurrRoom)
                {
                    case 0x00:
                    {

                        switch (Message.GrottoData & 0x1f)
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

                        switch (Message.SceneID)
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
                }
                break;
            }

            case OOT_FAIRY_FOUNTAIN:
            {
                switch (Message.SceneID)
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

        switch (Message.CurrSceneID)
        {
            case MM_GROTTOS:
            {
                switch (Message.CurrRoom)
                {
                    case 0x00: return MM_GROTTO_EXIT_GOSSIPS_OCEAN;
                    case 0x01: return MM_GROTTO_EXIT_GOSSIPS_SWAMP;
                    case 0x02: return MM_GROTTO_EXIT_GOSSIPS_CANYON;
                    case 0x03: return MM_GROTTO_EXIT_GOSSIPS_MOUNTAIN;
                    case 0x04:
                    {
                        switch (Message.GrottoData & 0x1f)
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
                        switch (Message.SceneID)
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


uint32_t EntranceHelper::CorrectGrottoScene(EntranceMessage& Message)
{
    if (Message.GameID == OOT_GAME)
    {
        switch (Message.EntranceID)
        {
            case OOT_GROTTO_EXIT_GENERIC_KOKIRI_FOREST:
            case OOT_GROTTO_GENERIC_KOKIRI_FOREST_ENTR:
            {
                return OOT_GROTTO_KOKIRI_FOREST_STORMS;
            }


            case OOT_GROTTO_EXIT_GENERIC_LOST_WOODS:
            case OOT_GROTTO_GENERIC_LOST_WOODS_ENTR:
            {
                return OOT_GROTTO_LOST_WOODS_GENERIC;
            }

            case OOT_GROTTO_EXIT_GENERIC_KAKARIKO:
            case OOT_GROTTO_GENERIC_KAKARIKO_ENTR:
            {
                return OOT_GROTTO_KAKARIKO_OPEN;
            }

            case OOT_GROTTO_EXIT_GENERIC_DMT:
            case OOT_GROTTO_GENERIC_DMT_ENTR:
            {
                return OOT_GROTTO_DEATH_TRIAL_STORMS;
            }

            case OOT_GROTTO_EXIT_GENERIC_DMC:
            case OOT_GROTTO_GENERIC_DMC_ENTR:
            {
                return OOT_GROTTO_DEATH_CRATER_GENERIC;
            }

            case OOT_GROTTO_EXIT_GENERIC_RIVER:
            case OOT_GROTTO_GENERIC_RIVER_ENTR:
            {
                return OOT_GROTTO_ZORA_RIVER_GENERIC;
            }

            case OOT_GROTTO_EXIT_GENERIC_HF_SOUTHEAST:
            case OOT_GROTTO_GENERIC_HF_SOUTHEAST_ENTR:
            {
                return OOT_GROTTO_HYRULE_SE;
            }

            case OOT_GROTTO_EXIT_GENERIC_HF_OPEN:
            case OOT_GROTTO_GENERIC_HF_OPEN_ENTR:
            {
                return OOT_GROTTO_HYRULE_OPEN;
            }

            case OOT_GROTTO_EXIT_GENERIC_HF_MARKET:
            case OOT_GROTTO_GENERIC_HF_MARKET_ENTR:
            {
                return OOT_GROTTO_HYRULE_MARKET;
            }

            case OOT_GROTTO_EXIT_FAIRY_SFM:
            case OOT_GROTTO_FAIRY_SFM_ENTR:
            {
                return OOT_FAIRY_SACRED_MEADOW;
            }

            case OOT_GROTTO_EXIT_FAIRY_HF:
            case OOT_GROTTO_FAIRY_HF_ENTR:
            {
                return OOT_FAIRY_HYRULE;
            }

            case OOT_GROTTO_EXIT_FAIRY_RIVER:
            case OOT_GROTTO_FAIRY_RIVER_ENTR:
            {
                return OOT_FAIRY_ZORA_RIVER;
            }

            case OOT_GROTTO_EXIT_FAIRY_DOMAIN:
            case OOT_GROTTO_FAIRY_DOMAIN_ENTR:
            {
                return OOT_FAIRY_ZORA_DOMAIN;
            }

            case OOT_GROTTO_EXIT_FAIRY_FORTRESS:
            case OOT_GROTTO_FAIRY_FORTRESS_ENTR:
            {
                return OOT_FAIRY_GERUDO_FORTRESS;
            }

            case OOT_GROTTO_EXIT_SCRUBS2_SFM:
            case OOT_GROTTO_SCRUBS2_SFM_ENTR:
            {
                return OOT_GROTTO_SACRED_MEADOW_STORMS;
            }

            case OOT_GROTTO_EXIT_SCRUBS2_RIVER:
            case OOT_GROTTO_SCRUBS2_RIVER_ENTR:
            {
                return OOT_GROTTO_ZORA_RIVER_STORMS;
            }

            case OOT_GROTTO_EXIT_SCRUBS2_VALLEY:
            case OOT_GROTTO_SCRUBS2_VALLEY_ENTR:
            {
                return OOT_GROTTO_VALLEY_STORMS;
            }

            case OOT_GROTTO_EXIT_SCRUBS2_COLOSSUS:
            case OOT_GROTTO_SCRUBS2_COLOSSUS_ENTR:
            {
                return OOT_GROTTO_DESERT_SCRUBS;
            }

            case OOT_GROTTO_EXIT_SCRUBS3_RANCH:
            case OOT_GROTTO_SCRUBS3_RANCH_ENTR:
            {
                return OOT_GROTTO_LON_LON_SCRUBS;
            }

            case OOT_GROTTO_EXIT_SCRUBS3_GORON_CITY:
            case OOT_GROTTO_SCRUBS3_GORON_CITY_ENTR:
            {
                return OOT_GROTTO_GORON_CITY_SCRUBS;
            }

            case OOT_GROTTO_EXIT_SCRUBS3_DMC:
            case OOT_GROTTO_SCRUBS3_DMC_ENTR:
            {
                return OOT_GROTTO_DEATH_CRATER_SCRUBS;
            }

            case OOT_GROTTO_EXIT_SCRUBS3_LAKE:
            case OOT_GROTTO_SCRUBS3_LAKE_ENTR:
            {
                return OOT_GROTTO_LAKE_HYLIA_SCRUBS;
            }

            case OOT_GROTTO_EXIT_SCRUB_UPGRADE:
            case OOT_GROTTO_SCRUB_UPGRADE_ENTR:
            {
                return OOT_GROTTO_LOST_WOODS_SCRUB_UPGRADE;
            }

            case OOT_GROTTO_EXIT_SCRUB_HEART_PIECE:
            case OOT_GROTTO_SCRUB_HEART_PIECE_ENTR:
            {
                return OOT_GROTTO_HYRULE_SCRUBS;
            }

            case OOT_GROTTO_EXIT_DEKU_THEATER:
            case OOT_GROTTO_DEKU_THEATER_ENTR:
            {
                return OOT_GROTTO_LOST_WOODS_THEATER;
            }

            case OOT_GROTTO_EXIT_WOLFOS:
            case OOT_GROTTO_WOLFOS_ENTR:
            {
                return OOT_GROTTO_SACRED_MEADOW_WOLFOS;
            }

            case OOT_GROTTO_EXIT_TEKTITE:
            case OOT_GROTTO_TEKTITE_ENTR:
            {
                return OOT_GROTTO_HYRULE_TEKTITE;
            }

            case OOT_GROTTO_EXIT_FIELD_COW:
            case OOT_GROTTO_FIELD_COW_ENTR:
            {
                return OOT_GROTTO_HYRULE_GERUDO;
            }

            case OOT_GROTTO_EXIT_FIELD_TREE:
            case OOT_GROTTO_FIELD_TREE_ENTR:
            {
                return OOT_GROTTO_HYRULE_KAKARIKO;
            }

            case OOT_GROTTO_EXIT_TRAIL_COW:
            case OOT_GROTTO_TRAIL_COW_ENTR:
            {
                return OOT_GROTTO_DEATH_TRIAL_COW;
            }

            case OOT_GROTTO_EXIT_CASTLE:
            case OOT_GROTTO_CASTLE_ENTR:
            {
                return OOT_GROTTO_CASTLE_STORMS;
            }

            case OOT_GROTTO_EXIT_REDEAD:
            case OOT_GROTTO_REDEAD_ENTR:
            {
                return OOT_GROTTO_KAKARIKO_REDEAD;
            }

            case OOT_GROTTO_EXIT_OCTOROK:
            case OOT_GROTTO_OCTOROK_ENTR:
            {
                return OOT_GROTTO_VALLEY_OCTOROK;
            }
        }
    }
    else if (Message.GameID == MM_GAME)
    {
        switch (Message.EntranceID)
        {
            case MM_GROTTO_GENERIC_FIELD_PILLAR_ENTR:
            case MM_GROTTO_EXIT_GENERIC_FIELD_PILLAR:
            {
                return MM_GROTTO_TERMINA_PILLAR;
            }

            case MM_GROTTO_GENERIC_GRASS_ENTR:
            case MM_GROTTO_EXIT_GENERIC_GRASS:
            {
                return MM_GROTTO_TERMINA_TALL_GRASS;
            }

            case MM_GROTTO_GENERIC_PATH_SWAMP_ENTR:
            case MM_GROTTO_EXIT_GENERIC_PATH_SWAMP:
            {
                return MM_GROTTO_SOUTHERN_SWAMP_ROAD_OPEN;
            }

            case MM_GROTTO_GENERIC_WOODS_ENTR:
            case MM_GROTTO_EXIT_GENERIC_WOODS:
            {
                return MM_GROTTO_WOODS_OF_MYSTERY_OPEN;
            }

            case MM_GROTTO_GENERIC_SWAMP_ENTR:
            case MM_GROTTO_EXIT_GENERIC_SWAMP:
            {
                return MM_GROTTO_SOUTHERN_SWAMP_OPEN;
            }

            case MM_GROTTO_GENERIC_MOUNTAIN_VILLAGE_ENTR:
            case MM_GROTTO_EXIT_GENERIC_MOUNTAIN_VILLAGE:
            {
                return MM_GROTTO_MOUNTAIN_VILLAGE_GENERIC;
            }

            case MM_GROTTO_GENERIC_TWIN_ISLANDS_ENTR:
            case MM_GROTTO_EXIT_GENERIC_TWIN_ISLANDS:
            {
                return MM_GROTTO_TWIN_ISLANDS_RAMP;
            }

            case MM_GROTTO_GENERIC_PATH_SNOWHEAD_ENTR:
            case MM_GROTTO_EXIT_GENERIC_PATH_SNOWHEAD:
            {
                return MM_GROTTO_PATH_TO_SNOWHEAD_GENERIC;
            }

            case MM_GROTTO_GENERIC_GREAT_BAY_COAST_ENTR:
            case MM_GROTTO_EXIT_GENERIC_GREAT_BAY_COAST:
            {
                return MM_GROTTO_GREAT_BAY_COAST_FISHERMAN;
            }

            case MM_GROTTO_GENERIC_ZORA_CAPE_ENTR:
            case MM_GROTTO_EXIT_GENERIC_ZORA_CAPE:
            {
                return MM_GROTTO_ZORA_CAPE_GENERIC;
            }

            case MM_GROTTO_GENERIC_PATH_IKANA_ENTR:
            case MM_GROTTO_EXIT_GENERIC_PATH_IKANA:
            {
                return MM_GROTTO_IKANA_ROAD_GENERIC;
            }

            case MM_GROTTO_GENERIC_GRAVEYARD_ENTR:
            case MM_GROTTO_EXIT_GENERIC_GRAVEYARD:
            {
                return MM_GROTTO_IKANA_GRAVEYARD_GENERIC;
            }

            case MM_GROTTO_GENERIC_VALLEY_ENTR:
            case MM_GROTTO_EXIT_GENERIC_VALLEY:
            {
                return MM_GROTTO_IKANA_VALLEY_OPEN;
            }

            case MM_GROTTO_COW_FIELD_ENTR:
            case MM_GROTTO_EXIT_COW_FIELD:
            {
                return MM_GROTTO_TERMINA_COW;
            }

            case MM_GROTTO_COW_COAST_ENTR:
            case MM_GROTTO_EXIT_COW_COAST:
            {
                return MM_GROTTO_GREAT_BAY_COAST_COW;
            }

            case MM_GROTTO_GOSSIPS_OCEAN_ENTR:
            case MM_GROTTO_EXIT_GOSSIPS_OCEAN:
            {
                return MM_GROTTO_TERMINA_OCEAN_GOSSIP;
            }

            case MM_GROTTO_GOSSIPS_SWAMP_ENTR:
            case MM_GROTTO_EXIT_GOSSIPS_SWAMP:
            {
                return MM_GROTTO_TERMINA_SWAMP_GOSSIP;
            }

            case MM_GROTTO_GOSSIPS_CANYON_ENTR:
            case MM_GROTTO_EXIT_GOSSIPS_CANYON:
            {
                return MM_GROTTO_TERMINA_CANYON_GOSSIP;
            }

            case MM_GROTTO_GOSSIPS_MOUNTAIN_ENTR:
            case MM_GROTTO_EXIT_GOSSIPS_MOUNTAIN:
            {
                return MM_GROTTO_TERMINA_MOUNTAIN_GOSSIP;
            }

            case MM_GROTTO_HOT_WATER_ENTR:
            case MM_GROTTO_EXIT_HOT_WATER:
            {
                return MM_GROTTO_TWIN_ISLANDS_FROZEN;
            }

            case MM_GROTTO_JP_LINE_END_ENTR:
            case MM_GROTTO_EXIT_JP_LINE_END:
            {
                return MM_GROTTO_DEKU_PALACE_GENERIC;
            }

            case MM_GROTTO_DODONGO_ENTR:
            case MM_GROTTO_EXIT_DODONGO:
            {
                return MM_GROTTO_TERMINA_DODONGO;
            }

            case MM_GROTTO_JP_CLIMB_RIGHT_ENTR:
            case MM_GROTTO_EXIT_JP_CLIMB_RIGHT:
            {
                return MM_GROTTO_DEKU_PALACE_CLIMB;
            }

            case MM_GROTTO_SCRUB_ENTR:
            case MM_GROTTO_EXIT_SCRUB:
            {
                return MM_GROTTO_TERMINA_SCRUB;
            }

            case MM_GROTTO_BIO_BABA_ENTR:
            case MM_GROTTO_EXIT_BIO_BABA:
            {
                return MM_GROTTO_TERMINA_BIO_BABA;
            }

            case MM_GROTTO_BEAN_ENTR:
            case MM_GROTTO_EXIT_BEAN:
            {
                return MM_GROTTO_DEKU_PALACE_BEANS;
            }

            case MM_GROTTO_PEAHAT_ENTR:
            case MM_GROTTO_EXIT_PEAHAT:
            {
                return MM_GROTTO_TERMINA_PEAHAT;
            }

            case MM_GROTTO_JP_LINE_START_ENTR:
            case MM_GROTTO_EXIT_JP_LINE_START:
            {
                return MM_GROTTO_DEKU_PALACE_GENERIC;
            }

            case MM_GROTTO_JP_CLIMB_LEFT_ENTR:
            case MM_GROTTO_EXIT_JP_CLIMB_LEFT:
            {
                return MM_GROTTO_DEKU_PALACE_CLIMB;
            }
        }
    }

    return Message.EntranceID;
}


float EntranceHelper::GetDistanceGrottoEntrance(GrottoEntrance Grotto, float X, float Y, float Z)
{
    float xDist = fabsf(X - Grotto.SpawnPos[0]);
    float yDist = fabsf(Y - Grotto.SpawnPos[1]);
    float zDist = fabsf(Z - Grotto.SpawnPos[2]);

    MultiLogger::LogMessage("Entrance ID = 0x%08X, X dist = %f, Y dist = %f, Z dist = %f, Total = %f", Grotto.EntranceID, xDist, yDist, zDist, xDist + yDist + zDist);
    return xDist + yDist + zDist;
}


uint32_t EntranceHelper::GetWarpSong(EntranceMessage& Message, bool * IsWarpSong)
{
    *IsWarpSong = true;

    if (Message.GameID == OOT_GAME)
    {
        if (Message.SceneID >= OOT_LAIR_GOHMA && Message.SceneID <= OOT_LAIR_GANONDORF)
        {   // It is not possible to pull out the ocarina in OoT lair's scene

            *IsWarpSong = false;
            return Message.EntranceID;
        }

        switch (Message.OoTSongID)
        {
            case OoTSongs::Minuet_of_Forest:
            {
                return OOT_MINUET_OF_FOREST_SONG;
            }

            case OoTSongs::Bolero_of_Fire:
            {
                return OOT_BOLERO_OF_FIRE_SONG;
            }

            case OoTSongs::Serenade_of_Water:
            {
                return OOT_SERENADE_OF_WATER_SONG;
            }

            case OoTSongs::Requiem_of_Spirit:
            {
                return OOT_REQUIEM_OF_SPIRIT_SONG;
            }

            case OoTSongs::Nocturne_of_Shadow:
            {
                return OOT_NOCTURNE_OF_SHADOW_SONG;
            }

            case OoTSongs::Prelude_of_Light:
            {
                return OOT_PRELUDE_OF_LIGHT_SONG;
            }

            case OoTSongs::Song_of_Soaring:
            {
                Message.GameID = MM_GAME;
                switch (Message.OwlID)
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
                        Message.GameID = OOT_GAME;
                        *IsWarpSong = false;
                        break;
                    }
                }
            }

            default:
            {
                *IsWarpSong = false;
                break;
            }
        }
    }
    else if (Message.GameID == MM_GAME)
    {
        Message.GameID = OOT_GAME;
        switch (Message.MMSongID)
        {
            case MMSongs::Minuet_of_Forest:
            {
                return OOT_MINUET_OF_FOREST_SONG;
            }

            case MMSongs::Bolero_of_Fire:
            {
                return OOT_BOLERO_OF_FIRE_SONG;
            }

            case MMSongs::Serenade_of_Water:
            {
                return OOT_SERENADE_OF_WATER_SONG;
            }

            case MMSongs::Requiem_of_Spirit:
            {
                return OOT_REQUIEM_OF_SPIRIT_SONG;
            }

            case MMSongs::Nocturne_of_Shadow:
            {
                return OOT_NOCTURNE_OF_SHADOW_SONG;
            }

            case MMSongs::Prelude_of_Light:
            {
                return OOT_PRELUDE_OF_LIGHT_SONG;
            }

            case MMSongs::Song_of_Soaring:
            {
                Message.GameID = MM_GAME;
                switch (Message.OwlID)
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
                        *IsWarpSong = false;
                        break;
                    }
                }
            }

            default:
            {
                Message.GameID = MM_GAME;
                *IsWarpSong = false;
                break;
            }
        }
    }

    return Message.EntranceID;
}


uint32_t EntranceHelper::CheckGrottoSpawn(EntranceMessage& Message)
{
    switch (Message.EntranceID)
    {

#pragma region OoT Grottos

        // Death Mountain Crater
        case OOT_GROTTO_EXIT_SCRUBS3_DMC:
        case OOT_GROTTO_EXIT_GENERIC_DMC:
        case OOT_WARP_SONG_CRATER_ENTR:
        case OOT_DEATH_CRATER_FROM_TEMPLE_FIRE_ENTR:
        case OOT_DEATH_CRATER_FROM_FAIRY_ENTR:
        case OOT_DEATH_MOUNTAIN_CRATER_ENTR:
        case OOT_CRATER_FROM_GORON_CITY_ENTR:
        {
            Message.EntranceID = OOT_DEATH_MOUNTAIN_CRATER_ENTR;
            break;
        }

        // Death Mount Trail
        case OOT_GROTTO_EXIT_GENERIC_DMT:
        case OOT_GROTTO_EXIT_TRAIL_COW:
        case OOT_DEATH_MOUNTAIN_FROM_FAIRY_ENTR:
        case OOT_TRAIL_SUMMIT_FROM_CRATER_ENTR:
        case OOT_MOUNTAIN_TRAIL_FROM_DODONGO_CAVERN_ENTR:
        case OOT_DEATH_MOUNTAIN_FROM_KAKARIKO_ENTR:
        case OOT_DEATH_MOUNTAIN_FROM_GORON_CITY_ENTR:
        {
            Message.EntranceID = OOT_DEATH_MOUNTAIN_FROM_KAKARIKO_ENTR;
            break;
        }

        // Goron City
        case OOT_GROTTO_EXIT_SCRUBS3_GORON_CITY:
        case OOT_GORON_CITY_FROM_SHOP_ENTR:
        case OOT_GORON_CITY_FROM_CRATER_ENTR:
        case OOT_GORON_CITY_FROM_LOST_WOODS_ENTR:
        case OOT_GORON_CITY_ENTR:
        {
            Message.EntranceID = OOT_GORON_CITY_ENTR;
            break;
        }

        // Sacred Forest Meadow
        case OOT_GROTTO_EXIT_FAIRY_SFM:
        case OOT_GROTTO_EXIT_SCRUBS2_SFM:
        case OOT_GROTTO_EXIT_WOLFOS:
        case OOT_SACRED_MEADOW_FROM_TEMPLE_FOREST_ENTR:
        case OOT_SACRED_FOREST_MEADOW_ENTR:
        case OOT_WARP_SONG_MEADOW_ENTR:
        {
            Message.EntranceID = OOT_SACRED_FOREST_MEADOW_ENTR;
            break;
        }

        // Lost Woods
        case OOT_GROTTO_EXIT_DEKU_THEATER:
        case OOT_GROTTO_EXIT_SCRUB_UPGRADE:
        case OOT_GROTTO_EXIT_GENERIC_LOST_WOODS:
        case OOT_LOST_WOODS_FROM_GORON_CITY_ENTR:
        case OOT_LOST_WOODS_FROM_ZORA_RIVER_ENTR:
        case OOT_LOST_WOODS_BRIDGE_FROM_FOREST_ENTR:
        case OOT_LOST_WOODS_BRIDGE_FROM_FIELD_ENTR:
        case OOT_LOST_WOODS_FROM_KOKIRI_FOREST_ENTR:
        case OOT_LOST_WOODS_FROM_MEADOW_ENTR:
        {
            Message.EntranceID = OOT_LOST_WOODS_FROM_KOKIRI_FOREST_ENTR;
            break;
        }

        // Kokiri Forest
        case OOT_GROTTO_EXIT_GENERIC_KOKIRI_FOREST:
        case OOT_KOKIRI_FOREST_FROM_DEKU_TREE_ENTR:
        case OOT_FOREST_FROM_LOST_WOODS_BRIDGE_ENTR:
        case OOT_KOKIRI_FOREST_FROM_MIDO_ENTR:
        case OOT_KOKIRI_FOREST_FROM_SARIA_ENTR:
        case OOT_KOKIRI_FOREST_FROM_TWINS_ENTR:
        case OOT_KOKIRI_FOREST_FROM_KNOW_IT_ALL_ENTR:
        case OOT_KOKIRI_FOREST_FROM_SHOP_ENTR:
        case OOT_KOKIRI_FOREST_FROM_LINK_ENTR:
        case OOT_KOKIRI_FOREST_FROM_LOST_WOODS_ENTR:
        {
            Message.EntranceID = OOT_KOKIRI_FOREST_FROM_LOST_WOODS_ENTR;
            break;
        }

        // Kakariko
        case OOT_GROTTO_EXIT_GENERIC_KAKARIKO:
        case OOT_GROTTO_EXIT_REDEAD:
        case OOT_KAKARIKO_FROM_BOTTOM_OF_THE_WELL_ENTR:
        case OOT_KAKARIKO_FROM_DEATH_MOUNTAIN_ENTR:
        case OOT_KAKARIKO_FROM_GRAVEYARD_ENTR:
        case OOT_KAKARIKO_FROM_CARPENTER_ENTR:
        case OOT_KAKARIKO_FROM_SKULLTULA_ENTR:
        case OOT_KAKARIKO_FROM_IMPA_ENTR:
        case OOT_KAKARIKO_FROM_IMPA_BACK_ENTR:
        case OOT_KAKARIKO_FROM_WINDMILL_ENTR:
        case OOT_KAKARIKO_FROM_SHOP_POTION_ENTR:
        case OOT_KAKARIKO_FROM_SHOP_POTION_BACK_ENTR:
        case OOT_KAKARIKO_FROM_BAZAAR_ENTR:
        case OOT_KAKARIKO_FROM_ARCHERY_ENTR:
        case OOT_KAKARIKO_FROM_GRANNY_ENTR:
        case OOT_KAKARIKO_FROM_FIELD_ENTR:
        case OOT_VILLAGE_OWL_ENTR:
        {
            Message.EntranceID = OOT_KAKARIKO_FROM_FIELD_ENTR;
            break;
        }

        // Hyrule Field
        case OOT_GROTTO_EXIT_GENERIC_HF_MARKET:
        case OOT_GROTTO_EXIT_FAIRY_HF:
        case OOT_GROTTO_EXIT_TEKTITE:
        case OOT_GROTTO_EXIT_GENERIC_HF_SOUTHEAST:
        case OOT_GROTTO_EXIT_GENERIC_HF_OPEN:
        case OOT_GROTTO_EXIT_SCRUB_HEART_PIECE:
        case OOT_GROTTO_EXIT_FIELD_COW:
        case OOT_GROTTO_EXIT_FIELD_TREE:
        case OOT_FIELD_OWL_ENTR:
        case OOT_FIELD_FROM_ZORA_RIVER_WATER_ENTR:
        case OOT_FIELD_FROM_ZORA_RIVER_ENTR:
        case OOT_FIELD_FROM_LOST_WOODS_BRIDGE_ENTR:
        case OOT_FIELD_FROM_KAKARIKO_ENTR:
        case OOT_FIELD_FROM_LON_LON_RANCH_ENTR:
        case OOT_FIELD_FROM_LON_LON_EPONA_JUMP_LEFT:
        case OOT_FIELD_FROM_LON_LON_EPONA_JUMP_MIDDLE:
        case OOT_FIELD_FROM_LON_LON_EPONA_JUMP_RIGHT:
        case OOT_FIELD_FROM_LAKE_HYLIA_ENTR:
        case OOT_FIELD_FROM_GERUDO_VALLEY_ENTR:
        case OOT_FIELD_FROM_MARKET_ENTRANCE_ENTR:
        {
            Message.EntranceID = OOT_FIELD_FROM_MARKET_ENTRANCE_ENTR;
            break;
        }

        // Gerudo Valley
        case OOT_GROTTO_EXIT_OCTOROK:
        case OOT_GROTTO_EXIT_SCRUBS2_VALLEY:
        case OOT_VALLEY_FROM_GERUDO_FORTRESS_ENTR:
        case OOT_GERUDO_FORTRESS_CAUGHT_NO_HOOK_ENTR:
        case OOT_GERUDO_VALLEY_FROM_FIELD_ENTR:
        case OOT_GERUDO_VALLEY_FROM_TENT_ENTR:
        {
            Message.EntranceID = OOT_GERUDO_VALLEY_FROM_FIELD_ENTR;
            break;
        }
        
        // Lake Hylia
        case OOT_GROTTO_EXIT_SCRUBS3_LAKE:
        case OOT_LAKE_HYLIA_FROM_TEMPLE_WATER_ENTR:
        case OOT_LAKE_HYLIA_FROM_FIELD_ENTR:
        case OOT_LAKE_HYLIA_FROM_ZORA_DOMAIN_ENTR:
        case OOT_LAKE_HYLIA_FROM_VALLEY_ENTR:
        case OOT_LAKE_HYLIA_FROM_LABORATORY_ENTR:
        case OOT_LAKE_HYLIA_FROM_FISHING_POND_ENTR:
        case OOT_WARP_SONG_LAKE_ENTR:
        {
            Message.EntranceID = OOT_LAKE_HYLIA_FROM_FIELD_ENTR;
            break;
        }

        // Zora Domain
        case OOT_GROTTO_EXIT_FAIRY_DOMAIN:
        case OOT_ZORA_DOMAIN_FROM_SHOP_ENTR:
        case OOT_DOMAIN_FROM_FOUNTAIN_ENTR:
        case OOT_ZORA_DOMAIN_FROM_LAKE_HYLIA_ENTR:
        case OOT_ZORA_DOMAIN_ENTR:
        {
            Message.EntranceID = OOT_ZORA_DOMAIN_ENTR;
            break;
        }

        // Zora River
        case OOT_GROTTO_EXIT_SCRUBS2_RIVER:
        case OOT_GROTTO_EXIT_FAIRY_RIVER:
        case OOT_GROTTO_EXIT_GENERIC_RIVER:
        case OOT_ZORA_RIVER_FROM_FIELD_WATER_ENTR:
        case OOT_ZORA_RIVER_FROM_LOST_WOODS_ENTR:
        case OOT_RIVER_FROM_DOMAIN_ENTR:
        case OOT_ZORA_RIVER_FROM_FIELD_ENTR:
        {
            Message.EntranceID = OOT_ZORA_RIVER_FROM_FIELD_ENTR;
            break;
        }

        // Desert Colossus
        case OOT_GROTTO_EXIT_SCRUBS2_COLOSSUS:
        case OOT_DESERT_FROM_MIROR_ENTR:
        case OOT_DESERT_FROM_GAUNTLET_ENTR:
        case OOT_DESERT_COLOSSUS_FROM_TEMPLE_SPIRIT_ENTR:
        case OOT_WARP_SONG_DESERT_ENTR:
        case OOT_DESERT_COLOSSUS_FROM_FAIRY_ENTR:
        case OOT_COLOSSUS_ENTR:
        {
            Message.EntranceID = OOT_COLOSSUS_ENTR;
            break;
        }

        // Gerudo Fortress
        case OOT_GROTTO_EXIT_FAIRY_FORTRESS:
        case OOT_GERUDO_FORTRESS_FROM_GERUDO_TRAINING_ENTR:
        case OOT_FORTRESS_FROM_WASTELAND_ENTR:
        case OOT_GERUDO_FORTRESS_CAUGHT_ENTR:
        case OOT_FORTRESS_FROM_HIDEOUT_BREAKOUT:
        case OOT_FORTRESS_FROM_HIDEOUT_BREAKOUT_JAIL:
        case OOT_FORTRESS_FROM_HIDEOUT_LEFT_JAIL_1:
        case OOT_FORTRESS_FROM_HIDEOUT_RIGHT_JAIL_1:
        case OOT_FORTRESS_FROM_HIDEOUT_KITCHEN_BOTTOM:
        case OOT_FORTRESS_FROM_HIDEOUT_KITCHEN_BOTTOM_TO_TOP:
        case OOT_FORTRESS_FROM_HIDEOUT_KITCHEN_TOP_RIGHT:
        case OOT_FORTRESS_FROM_HIDEOUT_KITCHEN_TOP_LEFT:
        case OOT_FORTRESS_FROM_HIDEOUT_JAIL_2_TOP:
        case OOT_FORTRESS_FROM_HIDEOUT_JAIL_2_BOTTOM:
        case OOT_FORTRESS_FROM_HIDEOUT_JAIL_3_TOP:
        case OOT_FORTRESS_FROM_HIDEOUT_JAIL_3_BOTTOM:
        case OOT_FORTRESS_FROM_HIDEOUT_JAIL_4:
        case OOT_GERUDO_FORTRESS_FROM_VALLEY_ENTR:
        {
            Message.EntranceID = OOT_GERUDO_FORTRESS_FROM_VALLEY_ENTR;
            break;
        }

        // Lon Lon's Ranch
        case OOT_GROTTO_EXIT_SCRUBS3_RANCH:
        case OOT_LON_LON_RANCH_FROM_STABLES_ENTR:
        case OOT_LON_LON_RANCH_FROM_SILO_ENTR:
        case OOT_LON_LON_RANCH_FROM_HOUSE_ENTR:
        case OOT_LON_LON_RANCH_FROM_FIELD_ENTR:
        {
            Message.EntranceID = OOT_LON_LON_RANCH_FROM_FIELD_ENTR;
            break;
        }
        
        // Hyrule Castle
        case OOT_GROTTO_EXIT_CASTLE:
        case OOT_CASTLE_CAUGHT_ENTR:
        case OOT_CASTLE_STEALTH_FROM_COURTYARD_ENTR:
        case OOT_HYRULE_CASTLE_FROM_FAIRY_ENTR:
        case OOT_HYRULE_CASTLE_ENTR:
        {
            Message.EntranceID = OOT_HYRULE_CASTLE_ENTR;
            break;
        }

#pragma endregion // OoT Grottos
        
#pragma region MM Grottos

        // Path to Snowhead
        case MM_GROTTO_EXIT_GENERIC_PATH_SNOWHEAD:
        case MM_PATH_SNOWHEAD_FROM_SNOWHEAD_ENTR:
        case MM_SNOWHEAD_PATH_FROM_MOUNTAIN_VILLAGE_ENTR:
        {
            Message.EntranceID = MM_PATH_SNOWHEAD_FROM_SNOWHEAD_ENTR;
            break;
        }

        // Termina Field
        case MM_GROTTO_EXIT_GENERIC_FIELD_PILLAR:
        case MM_GROTTO_EXIT_GENERIC_GRASS:
        case MM_GROTTO_EXIT_COW_FIELD:
        case MM_GROTTO_EXIT_GOSSIPS_MOUNTAIN:
        case MM_GROTTO_EXIT_GOSSIPS_CANYON:
        case MM_GROTTO_EXIT_GOSSIPS_OCEAN:
        case MM_GROTTO_EXIT_GOSSIPS_SWAMP:
        case MM_GROTTO_EXIT_DODONGO:
        case MM_GROTTO_EXIT_SCRUB:
        case MM_GROTTO_EXIT_PEAHAT:
        case MM_GROTTO_EXIT_BIO_BABA:
        case MM_TERMINA_FIELD_FROM_GREAT_BAY_COAST_ENTR:
        case MM_TERMINA_FIELD_FROM_ROAD_TO_IKANA_ENTR:
        case MM_TERMINA_FIELD_FROM_PATH_TO_MOUNTAIN_VILLAGE_ENTR:
        case MM_TERMINA_FIELD_FROM_ROAD_TO_SWAMP_ENTR:
        case MM_TERMINA_FIELD_FROM_MILK_ROAD_ENTR:
        case MM_FIELD_FROM_ASTRAL_OBSERVATORY_ENTR:
        case MM_TERMINA_FIELD_FROM_CLOCK_TOWN_NORTH_ENTR:
        case MM_TERMINA_FIELD_FROM_CLOCK_TOWN_WEST_ENTR:
        case MM_TERMINA_FIELD_FROM_CLOCK_TOWN_EAST_ENTR:
        case MM_TERMINA_FIELD_FROM_CLOCK_TOWN_SOUTH_ENTR:
        {
            Message.EntranceID = MM_TERMINA_FIELD_FROM_CLOCK_TOWN_SOUTH_ENTR;
            break;
        }

        // Great Bay Coast
        case MM_GROTTO_EXIT_GENERIC_GREAT_BAY_COAST:
        case MM_GROTTO_EXIT_COW_COAST:
        case MM_GREAT_BAY_FROM_SPIDER_HOUSE_ENTR:
        case MM_GREAT_BAY_FROM_PIRATE_FORTRESS_ENTR:
        case MM_GREAT_BAY_COAST_FROM_ZORA_CAPE_ENTR:
        case MM_GREAT_BAY_FROM_PINNACLE_ROCK_ENTR:
        case MM_WARP_OWL_GREAT_BAY_ENTR:
        case MM_VOID_GREAT_BAY_ENTR:
        case MM_VOID_GREAT_BAY_BY_PINNACLE_ROCK_ENTR:
        case MM_COAST_FROM_MIKAU_CS_ENTR:
        case MM_GREAT_BAY_FROM_ENTRANCE_CAUGHT:
        case MM_GREAT_BAY_COAST_FROM_LABORATORY_ENTR:
        case MM_GREAT_BAY_COAST_FROM_FISHER_HUT_ENTR:
        case MM_GREAT_BAY_COAST_FROM_FIELD_ENTR:
        {
            Message.EntranceID = MM_GREAT_BAY_COAST_FROM_FIELD_ENTR;
            break;
        }

        // Southern Swamp
        case MM_GROTTO_EXIT_GENERIC_SWAMP:
        case MM_KOUME_RIDE_FROM_TOURIST_ENTR:
        case MM_SWAMP_FROM_PALACE_MAIN_ENTRANCE_ENTR:
        case MM_SWAMP_FROM_PALACE_LEDGE_ENTR:
        case MM_SWAMP_FROM_WOODFALL_ENTR:
        case MM_SWAMP_FROM_MYSTERY_WOODS_ENTR:
        case MM_SWAMP_FROM_IKANA_CANYON_ENTR:
        case MM_SWAMP_FROM_POTION_SHOP_ENTR:
        case MM_SWAMP_FROM_TOURIST_INFORMATION_ENTR:
        case MM_WARP_OWL_SOUTHERN_SWAMP_ENTR:
        case MM_SWAMP_FROM_ROAD_ENTR:
        case MM_SWAMP_FROM_SPIDER_HOUSE_ENTR:
        {
            Message.EntranceID = MM_SWAMP_FROM_SPIDER_HOUSE_ENTR;
            break;
        }

        // Road to Southern Swamp
        case MM_GROTTO_EXIT_GENERIC_PATH_SWAMP:
        case MM_SWAMP_ROAD_FROM_ARCHERY_ENTR:
        case MM_SWAMP_ROAD_FROM_SWAMP_ENTR:
        case MM_SWAMP_ROAD_FROM_FIELD_ENTR:
        {
            Message.EntranceID = MM_SWAMP_ROAD_FROM_FIELD_ENTR;
            break;
        }

        // Mountain Village
        case MM_GROTTO_EXIT_GENERIC_MOUNTAIN_VILLAGE_WINTER:
        case MM_GROTTO_EXIT_GENERIC_MOUNTAIN_VILLAGE:
        case MM_WARP_OWL_MOUNTAIN_VILLAGE_ENTR:
        case MM_MOUNTAIN_VILLAGE_FROM_GORON_GRAVEYARD_ENTR:
        case MM_MOUNTAIN_VILLAGE_FROM_BLACKSMITH_ENTR:
        case MM_MOUNTAIN_VILLAGE_FROM_SNOWHEAD_PATH_ENTR:
        case MM_MOUNTAIN_VILLAGE_FROM_TWIN_ISLANDS_ENTR:
        case MM_MOUNTAIN_VILLAGE_FROM_PATH_ENTR:
        {
            Message.EntranceID = MM_MOUNTAIN_VILLAGE_FROM_PATH_ENTR;
            break;
        }

        // Twin Islands
        case MM_TWIN_ISLANDS_FROM_MOUNTAIN_VILLAGE_ENTR:
        case MM_TWIN_ISLANDS_FROM_GORON_VILLAGE_ENTR:
        case MM_TWIN_ISLANDS_FROM_GORON_RACETRACK_ENTR:
        case MM_GROTTO_EXIT_GENERIC_TWIN_ISLANDS:
        case MM_GROTTO_EXIT_HOT_WATER:
        {
            Message.EntranceID = MM_TWIN_ISLANDS_FROM_MOUNTAIN_VILLAGE_ENTR;
            break;
        }

        // Woods of mystery
        case MM_MYSTERY_WOODS_ENTR:
        {
            break;
        }

        // Zora Cape
        case MM_GROTTO_EXIT_GENERIC_ZORA_CAPE:
        case MM_ZORA_CAPE_FROM_HALL_WATER_ENTR:
        case MM_ZORA_CAPE_PENINSULA_ENTR:
        case MM_GREAT_BAY_FROM_FAIRY_FOUNTAIN_ENTR:
        case MM_ZORA_CAPE_FROM_BEAVERS_ENTR:
        case MM_WARP_OWL_ZORA_CAPE_ENTR:
        case MM_VOID_ZORA_CAPE_ENTR:
        case MM_GREAT_BAY_FROM_TEMPLE_ENTR:
        case MM_ZORA_CAPE_FROM_GREAT_BAY_COAST_ENTR:
        {
            Message.EntranceID = MM_ZORA_CAPE_FROM_GREAT_BAY_COAST_ENTR;
            break;
        }

        // Road to Ikana
        case MM_GROTTO_EXIT_GENERIC_PATH_IKANA:
        case MM_IKANA_ROAD_FROM_VALLEY_ENTR:
        case MM_IKANA_ROAD_FROM_IKANA_GRAVEYARD_ENTR:
        case MM_IKANA_ROAD_FROM_FIELD_ENTR:
        {
            Message.EntranceID = MM_IKANA_ROAD_FROM_FIELD_ENTR;
            break;
        }


        // Ikana Graveyard
        case MM_GROTTO_EXIT_GENERIC_GRAVEYARD:
        case MM_IKANA_GRAVEYARD_ENTR:
        case MM_GRAVE_EXIT_NIGHT1:
        case MM_GRAVE_EXIT_NIGHT2:
        case MM_IKANA_GRAVEYARD_FROM_DAMPE_ENTR:
        {
            Message.EntranceID = MM_IKANA_GRAVEYARD_FROM_DAMPE_ENTR;
            break;
        }

        // Ikana Canyon
        case MM_GROTTO_EXIT_GENERIC_VALLEY:
        case MM_WARP_BOSS_STONE_TOWER_INVERTED_ENTR:
        case MM_IKANA_CANYON_FROM_WELL_ENTR:
        case MM_IKANA_VALLEY_FROM_SHRINE_ENTR:
        case MM_IKANA_CANYON_FROM_CASTLE_GARDENS_ENTR:
        case MM_IKANA_CANYON_FROM_STONE_TOWER_ENTR:
        case MM_IKANA_CANYON_WATERFALLS_ENTR:
        case MM_IKANA_CANYON_FROM_GHOST_HUT_ENTR:
        case MM_IKANA_CANYON_FROM_MUSIC_BOX_HOUSE_ENTR:
        case MM_IKANA_CANYON_FROM_FAIRY_FOUNTAIN_ENTR:
        case MM_IKANA_CANYON_FROM_CAVERN_ENTR:
        case MM_IKANA_CANYON_FROM_SAKON_HIDEOUT_ENTR:
        case MM_WARP_OWL_IKANA_CANYON_ENTR:
        case MM_IKANA_VALLEY_FROM_ROAD_ENTR:
        {
                Message.EntranceID = MM_IKANA_VALLEY_FROM_ROAD_ENTR;
            break;
        }

#pragma endregion MM Grottos

        default:
        {
            return Message.EntranceID;
        }
    }

    const std::vector<GrottoEntrance> entrances = GrottoEntrances.at(Message.EntranceID);
    float tmpDist = 0.0f, bestDist = this->GetDistanceGrottoEntrance(entrances[0], Message.X, Message.Y, Message.Z);
    uint32_t bestEntID = entrances[0].EntranceID;

    if (bestDist == 0)
    {
        return bestEntID;
    }

    for (size_t i = 1; i < entrances.size(); i++)
    {
        tmpDist = this->GetDistanceGrottoEntrance(entrances[i], Message.X, Message.Y, Message.Z);

        if (tmpDist == 0.0)
        {
            return entrances[i].EntranceID;
        }
        else if (tmpDist < bestDist)
        {
            bestDist = tmpDist;
            bestEntID = entrances[i].EntranceID;
        }
    }

    return bestEntID;
}


uint32_t EntranceHelper::CheckSpecialCase(EntranceMessage& Message)
{
    if (Message.GameID == OOT_GAME)
    {
        switch (Message.SceneID)
        {
            case OOT_GERUDO_VALLEY:
            {
                if (Message.Direction == OUT_MAGIC && Message.EntranceID == OOT_GERUDO_FORTRESS_CAUGHT_NO_HOOK_ENTR)
                {
                    return OOT_GERUDO_VALLEY_CAUGHT_ENTR;
                }
            }

            case OOT_GERUDO_FORTRESS:
            {
                if (Message.Direction == OUT_MAGIC && Message.EntranceID == OOT_GERUDO_FORTRESS_CAUGHT_ENTR)
                {
                    return OOT_GERUDO_FORTRESS_JAIL_ENTR;
                }
            }

            case OOT_BAZAAR:
            {
                switch (Message.EntranceID)
                {
                    case OOT_KAKARIKO_BAZAAR_ENTR:
                    {
                        Message.SceneID = OOT_KAKARIKO_BAZAAR;
                        break;
                    }

                    case OOT_MARKET_BAZAAR_ENTR:
                    {
                        Message.SceneID = OOT_MARKET_BAZAAR;
                        break;
                    }
                }
                break;
            }

            case OOT_SHOOTING_GALLERY:
            {
                switch (Message.EntranceID)
                {
                    case OOT_ADULT_ARCHERY_ENTR:
                    {
                        Message.SceneID = OOT_KAKARIKO_SHOOTING;
                        break;
                    }

                    case OOT_CHILD_ARCHERY_ENTR:
                    {
                        Message.SceneID = OOT_MARKET_SHOOTING;
                        break;
                    }
                }

                break;
            }

            case OOT_GREAT_FAIRY_FOUNTAIN_UPGRADES:
            case OOT_GREAT_FAIRY_FOUNTAIN_SPELLS:
            {
                switch (Message.EntranceID)
                {
                    // First magic upgrade
                    case OOT_FAIRY_MAGIC_ENTR:
                    {
                        Message.SceneID = OOT_GREAT_FAIRY_MAGIC;
                        break;
                    }

                    // Second magic upgrade
                    case OOT_FAIRY_MAGIC2_ENTR:
                    {
                        Message.SceneID = OOT_GREAT_FAIRY_MAGIC2;
                        break;
                    }

                    // Double defense
                    case OOT_FAIRY_DEFENSE_ENTR:
                    {
                        Message.SceneID = OOT_GREAT_FAIRY_DEFENSE;
                        break;
                    }

                    // Din's fire
                    case OOT_FAIRY_DIN_ENTR:
                    {
                        if (Message.Age == LinkAge::Adult)
                        {   // Very special case as castle and ganon exterior shares the same entrance IDs...

                            Message.SceneID = OOT_GREAT_FAIRY_DEFENSE;
                            return OOT_FAIRY_DEFENSE_ENTR;
                        }

                        Message.SceneID = OOT_GREAT_FAIRY_CASTLE;
                        break;
                    }

                    // Farore's wind
                    case OOT_FAIRY_FARORE_ENTR:
                    {
                        Message.SceneID = OOT_GREAT_FAIRY_FARORE;
                        break;
                    }

                    // Nayru's love
                    case OOT_FAIRY_NAYRU_ENTR:
                    {
                        Message.SceneID = OOT_GREAT_FAIRY_NAYRU;
                        break;
                    }

                    case OOT_HYRULE_CASTLE_FROM_FAIRY_ENTR:
                    {
                        if (Message.Age == LinkAge::Adult)
                        {   // Very special case as castle and ganon exterior shares the same entrance IDs...

                            Message.SceneID = OOT_GREAT_FAIRY_DEFENSE;
                            return OOT_OUTSIDE_GANON_FROM_FAIRY_ENTR;
                        }
                    }
                }
                break;
            }

            case OOT_CASTLE_COURTYARD:
            {
                switch (Message.EntranceID)
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
                switch (Message.EntranceID)
                {
                    case 0x23d:
                    {   // Castle Courtyard -> Hyrule Castle

                        return OOT_CASTLE_STEALTH_FROM_COURTYARD_ENTR;
                    }

                    case OOT_CASTLE_STEALTH_ENTR:
                    {   // Hyrule Castle -> Castle Courtyard

                        return OOT_CASTLE_COURTYARD_ENTR;
                    }

                    case OOT_CASTLE_CAUGHT_ENTR:
                    {
                        if (Message.Direction == OUT_MAGIC)
                        {
                            return OOT_CASTLE_GATE_ENTR;
                        }
                        break;
                    }
                }
                break;
            }

            case OOT_GANON_CASTLE_EXTERIOR:
            {
                switch (Message.EntranceID)
                {   // Ganon castle extrior shares the same entrance IDs as hyrule castle ones

                    case OOT_MARKET_FROM_CASTLE_ENTR:
                    {
                        return OOT_MARKET_ADULT_FROM_GANON_CASTLE_EXTERIOR_ENTR;
                    }

                    case OOT_HYRULE_CASTLE_ENTR:
                    {
                        return OOT_GANON_CASTLE_EXTERIOR_ENTR;
                    }

                    case OOT_HYRULE_CASTLE_FROM_FAIRY_ENTR:
                    {
                        return OOT_OUTSIDE_GANON_FROM_FAIRY_ENTR;
                    }
                }
                break;
            }


            case OOT_MARKET_ADULT:
            {
                Message.SceneID = OOT_MARKET;
                switch (Message.EntranceID)
                {
                    case OOT_MARKET_FROM_CASTLE_ENTR:
                    {
                        return OOT_MARKET_ADULT_FROM_GANON_CASTLE_EXTERIOR_ENTR;
                    }
                    case OOT_HYRULE_CASTLE_ENTR:
                    {
                        return OOT_GANON_CASTLE_EXTERIOR_ENTR;
                    }
                }
                break;
            }

            case OOT_MARKET_CHILD_DAY:
            case OOT_MARKET_CHILD_NIGHT:
            {
                Message.SceneID = OOT_MARKET;
                break;
            }

            case OOT_BACK_ALLEY_DAY:
            case OOT_BACK_ALLEY_NIGHT:
            {
                Message.SceneID = OOT_BACK_ALLEY;
                break;
            }

            case OOT_TEMPLE_OF_TIME_EXTERIOR_CHILD_DAY:
            case OOT_TEMPLE_OF_TIME_EXTERIOR_CHILD_NIGHT:
            case OOT_TEMPLE_OF_TIME_EXTERIOR_ADULT:
            {
                Message.SceneID = OOT_TEMPLE_OF_TIME_ENTRYWAY;
                break;
            }

            case OOT_MARKET_ENTRANCE_CHILD_DAY:
            case OOT_MARKET_ENTRANCE_CHILD_NIGHT:
            case OOT_MARKET_ENTRANCE_ADULT:
            {
                Message.SceneID = OOT_MARKET_ENTRANCE;
                break;
            }

            case OOT_TOMB_DAMPE_WINDMILL:
            {
                switch (Message.EntranceID)
                {
                    case OOT_KAKARIKO_FROM_WINDMILL_ENTR:
                    case OOT_WINDMILL_ENTR:
                    {
                        Message.SceneID = OOT_WINDMILL;
                        break;
                    }
                }
                break;
            }

            case OOT_RANCH_HOUSE_SILO:
            {
                if (Message.EntranceID == OOT_SILO_ENTR)
                {
                    Message.SceneID = OOT_SILO;
                }
                break;
            }

            case OOT_GROTTOS:
            {
                switch (Message.EntranceID)
                {
                    case OOT_GROTTO_SCRUB_UPGRADE_ENTR:
                    {   // Lost Woods scrub nuts upgrade grotto entry 

                        Message.SceneID = OOT_GROTTO_LOST_WOODS_SCRUB_UPGRADE;
                        break;
                    }

                    case OOT_GROTTO_DEKU_THEATER_ENTR:  
                    {   // Deku's Theater grotto entry

                        Message.SceneID = OOT_GROTTO_LOST_WOODS_THEATER;
                        break;
                    }

                    case OOT_GROTTO_WOLFOS_ENTR:        
                    {   // Sacred Forest Meadow wolfos grotto entry

                        Message.SceneID = OOT_GROTTO_SACRED_MEADOW_WOLFOS;
                        break;
                    }

                    case OOT_GROTTO_SCRUB_HEART_PIECE_ENTR:
                    {   // Hyrule Field scrub grotto entry

                        Message.SceneID = OOT_GROTTO_HYRULE_SCRUBS;
                        break;
                    }

                    case OOT_GROTTO_FIELD_COW_ENTR:
                    {   // Hyrule Field cow grotto entry

                        Message.SceneID = OOT_GROTTO_HYRULE_GERUDO;
                        break;
                    }

                    case OOT_GROTTO_FIELD_TREE_ENTR:
                    {   // Hyrule Field tree near kakariko grotto entry

                        Message.SceneID = OOT_GROTTO_HYRULE_KAKARIKO;
                        break;
                    }

                    case OOT_GROTTO_CASTLE_ENTR:
                    {   // Hyrule Castle grotto entry

                        Message.SceneID = OOT_GROTTO_CASTLE_STORMS;
                        break;
                    }

                    case OOT_GROTTO_TEKTITE_ENTR:
                    {   // Hyrule Field tektite grotto entry

                        Message.SceneID = OOT_GROTTO_HYRULE_TEKTITE;
                        break;
                    }

                    case OOT_GROTTO_TRAIL_COW_ENTR:
                    {   // Death Mountain Trail cow grotto entry

                        Message.SceneID = OOT_GROTTO_DEATH_TRIAL_COW;
                        break;
                    }

                    case OOT_GROTTO_REDEAD_ENTR:
                    {   // Kakariko redead grotto entry

                        Message.SceneID = OOT_GROTTO_KAKARIKO_REDEAD;
                        break;
                    }

                    case OOT_GROTTO_OCTOROK_ENTR:
                    {   // Gerudo Valley octorok grotto entry

                        Message.SceneID = OOT_GROTTO_VALLEY_OCTOROK;
                        break;
                    }
                }
                break;
            }
        }
    }
    else if (Message.GameID == MM_GAME)
    {
        switch (Message.SceneID)
        {
            case MM_SOUTHERN_SWAMP_CLEAR:
            {
                Message.SceneID = MM_SOUTHERN_SWAMP;
                switch (Message.EntranceID)
                {   // We need to check for the entrance ID as southern swamp clear is equal to 0 and scene 0 is also used for new cycle detection.

                    case MM_SWAMP_CLEARED_FROM_SPIDER_HOUSE_ENTR:
                    {
                        return MM_SWAMP_FROM_SPIDER_HOUSE_ENTR;
                    }

                    case MM_SWAMP_CLEARED_FROM_ROAD_ENTR:
                    {
                        return MM_SWAMP_FROM_ROAD_ENTR;
                    }

                    case MM_SWAMP_CLEARED_FROM_PALACE_MAIN_ENTRANCE_ENTR:
                    {
                        return MM_SWAMP_FROM_PALACE_MAIN_ENTRANCE_ENTR;
                    }

                    case MM_SWAMP_CLEARED_FROM_PALACE_LEDGE_ENTR:
                    {
                        return MM_SWAMP_FROM_PALACE_LEDGE_ENTR;
                    }

                    case MM_SWAMP_CLEARED_FROM_WOODFALL_ENTR:
                    {
                        return MM_SWAMP_FROM_WOODFALL_ENTR;
                    }

                    case MM_SWAMP_CLEARED_FROM_MYSTERY_WOODS_ENTR:
                    {
                        return MM_SWAMP_FROM_MYSTERY_WOODS_ENTR;
                    }

                    case MM_SWAMP_CLEARED_FROM_IKANA_CANYON_ENTR:
                    {
                        return MM_SWAMP_FROM_IKANA_CANYON_ENTR;
                    }

                    case MM_SWAMP_CLEARED_FROM_POTION_SHOP_ENTR:
                    {
                        return MM_SWAMP_FROM_POTION_SHOP_ENTR;
                    }

                    case MM_SWAMP_CLEARED_FROM_TOURIST_INFORMATION_ENTR:
                    {
                        return MM_SWAMP_FROM_TOURIST_INFORMATION_ENTR;
                    }

                    case MM_WARP_OWL_SOUTHERN_SWAMP_CLEARED_ENTR:
                    {
                        return MM_WARP_OWL_SOUTHERN_SWAMP_ENTR;
                    }

                    case MM_KOUME_TARGET_FROM_TOURIST_ENTR:
                    {
                        return MM_KOUME_RIDE_FROM_TOURIST_ENTR;
                    }
                }

                // The entrance doesn't match ! Probably a new cycle
                Message.SceneID = 0;
                break;
            }

            case MM_MOUNTAIN_VILLAGE_WINTER:
            case MM_MOUNTAIN_VILLAGE_SPRING:
            {
                Message.SceneID = MM_MOUNTAIN_VILLAGE;

                switch (Message.EntranceID)
                {
                    case MM_MOUNTAIN_VILLAGE_SPRING_FROM_BLACKSMITH_ENTR:
                    {
                        return MM_MOUNTAIN_VILLAGE_FROM_BLACKSMITH_ENTR;
                    }

                    case MM_WARP_OWL_MOUNTAIN_VILLAGE_SPRING_ENTR:
                    {
                        return MM_WARP_OWL_MOUNTAIN_VILLAGE_ENTR;
                    }

                    case MM_MOUNTAIN_VILLAGE_SPRING_FROM_SNOWHEAD_PATH_ENTR:
                    {
                        return MM_MOUNTAIN_VILLAGE_FROM_SNOWHEAD_PATH_ENTR;
                    }

                    case MM_MOUNTAIN_VILLAGE_SPRING_FROM_GORON_GRAVEYARD_ENTR:
                    {
                        return MM_MOUNTAIN_VILLAGE_FROM_GORON_GRAVEYARD_ENTR;
                    }

                    case MM_MOUNTAIN_VILLAGE_SPRING_FROM_TWIN_ISLANDS_ENTR:
                    {
                        return MM_MOUNTAIN_VILLAGE_FROM_TWIN_ISLANDS_ENTR;
                    }

                    case MM_TWIN_ISLANDS_SPRING_FROM_MOUNTAIN_VILLAGE_ENTR:
                    {
                        return MM_TWIN_ISLANDS_FROM_MOUNTAIN_VILLAGE_ENTR;
                    }

                    break;
                }

                break;
            }

            case MM_TWIN_ISLANDS_SPRING:
            case MM_TWIN_ISLANDS_WINTER:
            {
                Message.SceneID = MM_TWIN_ISLANDS;
                
                switch (Message.EntranceID)
                {
                    case MM_TWIN_ISLANDS_SPRING_FROM_MOUNTAIN_VILLAGE_ENTR:
                    {
                        return MM_TWIN_ISLANDS_FROM_MOUNTAIN_VILLAGE_ENTR;
                    }

                    case MM_TWIN_ISLANDS_SPRING_FROM_GORON_VILLAGE_ENTR:
                    {
                        return MM_TWIN_ISLANDS_FROM_GORON_VILLAGE_ENTR;
                    }

                    case MM_TWIN_ISLANDS_SPRING_FROM_GORON_RACETRACK_ENTR:
                    {
                        return MM_TWIN_ISLANDS_FROM_GORON_RACETRACK_ENTR;
                    }

                    break;
                }
                break;
            }

            case MM_GORON_VILLAGE_SPRING:
            case MM_GORON_VILLAGE_WINTER:
            {
                Message.SceneID = MM_GORON_VILLAGE;

                switch (Message.EntranceID)
                {
                    case MM_GORON_VILLAGE_SPRING_FROM_TWIN_ISLANDS_ENTR:
                    {
                        return MM_GORON_VILLAGE_FROM_TWIN_ISLANDS_ENTR;
                    }

                    case MM_GORON_VILLAGE_SPRING_FROM_GORON_SHRINE_ENTR:
                    {
                        return MM_GORON_VILLAGE_FROM_GORON_SHRINE_ENTR;
                    }

                    case MM_GORON_VILLAGE_SPRING_FROM_LONE_PEAK_SHRINE_ENTR:
                    {
                        return MM_GORON_VILLAGE_FROM_LONE_PEAK_SHRINE_ENTR;
                    }
                }

                break;
            }

            case MM_GROTTOS:
            {
                switch (Message.EntranceID)
                {
                    case MM_LONE_PEAK_SHRINE_ENTR:
                    {   // Lone peak. Don't know why this is considered as a grotto...

                        Message.SceneID = MM_LONE_PEAK;
                        break;
                    }

                    case MM_GROTTO_GENERIC_FIELD_PILLAR_ENTR:
                    {   // Termina pillar grotto entry

                        Message.SceneID = MM_GROTTO_TERMINA_PILLAR;
                        break;
                    }

                    case MM_GROTTO_GENERIC_GRASS_ENTR:
                    {   // Termina tall grass grotto entry

                        Message.SceneID = MM_GROTTO_TERMINA_TALL_GRASS;
                        break;
                    }

                    case MM_GROTTO_GENERIC_PATH_SWAMP_ENTR:
                    {   // Road to southern swamp open grotto entry

                        Message.SceneID = MM_GROTTO_SOUTHERN_SWAMP_ROAD_OPEN;
                        break;
                    }

                    case MM_GROTTO_GENERIC_WOODS_ENTR:
                    {   // Woods of mystery open grotto entry

                        Message.SceneID = MM_GROTTO_WOODS_OF_MYSTERY_OPEN;
                        break;
                    }

                    case MM_GROTTO_GENERIC_SWAMP_ENTR:
                    {   // Southern swamp open grotto entry

                        Message.SceneID = MM_GROTTO_SOUTHERN_SWAMP_OPEN;
                        break;
                    }

                    case MM_GROTTO_GENERIC_MOUNTAIN_VILLAGE_ENTR:
                    {   // Mountain village open grotto entry

                        Message.SceneID = MM_GROTTO_MOUNTAIN_VILLAGE_GENERIC;
                        break;
                    }

                    case MM_GROTTO_GENERIC_TWIN_ISLANDS_ENTR:
                    {   // Twin islands ramp grotto entry

                        Message.SceneID = MM_GROTTO_TWIN_ISLANDS_RAMP;
                        break;
                    }

                    case MM_GROTTO_GENERIC_PATH_SNOWHEAD_ENTR:
                    {   // Path to snowhead grotto entry

                        Message.SceneID = MM_GROTTO_PATH_TO_SNOWHEAD_GENERIC;
                        break;
                    }

                    case MM_GROTTO_GENERIC_GREAT_BAY_COAST_ENTR:
                    {   // Great bay coast open grotto

                        Message.SceneID = MM_GROTTO_GREAT_BAY_COAST_FISHERMAN;
                        break;
                    }

                    case MM_GROTTO_GENERIC_ZORA_CAPE_ENTR:
                    {   // Zora cape grotto entry

                        Message.SceneID = MM_GROTTO_ZORA_CAPE_GENERIC;
                        break;
                    }

                    case MM_GROTTO_GENERIC_PATH_IKANA_ENTR:
                    {   // Road to ikana grotto entry

                        Message.SceneID = MM_GROTTO_IKANA_ROAD_GENERIC;
                        break;
                    }

                    case MM_GROTTO_GENERIC_GRAVEYARD_ENTR:
                    {   // Ikana graveyard grotto entry

                        Message.SceneID = MM_GROTTO_IKANA_GRAVEYARD_GENERIC;
                        break;
                    }

                    case MM_GROTTO_GENERIC_VALLEY_ENTR:
                    {   // Ikana canyon open grotto entry

                        Message.SceneID = MM_GROTTO_IKANA_VALLEY_OPEN;
                        break;
                    }

                    case MM_GROTTO_COW_FIELD_ENTR:
                    {   // Termina field cow grotto entry

                        Message.SceneID = MM_GROTTO_TERMINA_COW;
                        break;
                    }

                    case MM_GROTTO_COW_COAST_ENTR:
                    {   // great bay coast cow grotto entry

                        Message.SceneID = MM_GROTTO_GREAT_BAY_COAST_COW;
                        break;
                    }

                    case MM_GROTTO_GOSSIPS_OCEAN_ENTR:
                    {   // Termina ocean gossip grotto entry

                        Message.SceneID = MM_GROTTO_TERMINA_OCEAN_GOSSIP;
                        break;
                    }

                    case MM_GROTTO_GOSSIPS_SWAMP_ENTR:
                    {   // Termina swamp gossip grotto entry

                        Message.SceneID = MM_GROTTO_TERMINA_SWAMP_GOSSIP;
                        break;
                    }

                    case MM_GROTTO_GOSSIPS_CANYON_ENTR:
                    {   // Termina canyon gossip grotto entry

                        Message.SceneID = MM_GROTTO_TERMINA_CANYON_GOSSIP;
                        break;
                    }

                    case MM_GROTTO_GOSSIPS_MOUNTAIN_ENTR:
                    {   // Termina mountain gossip grotto entry

                        Message.SceneID = MM_GROTTO_TERMINA_MOUNTAIN_GOSSIP;
                        break;
                    }

                    case MM_GROTTO_HOT_WATER_ENTR:
                    {   // Twin islands frozen grotto entry

                        Message.SceneID = MM_GROTTO_TWIN_ISLANDS_FROZEN;
                        break;
                    }

                    case MM_GROTTO_JP_LINE_START_ENTR:
                    case MM_GROTTO_JP_LINE_END_ENTR:
                    {   // MM JP layout deku palace line grotto entry

                        Message.SceneID = MM_GROTTO_DEKU_PALACE_GENERIC;
                        break;
                    }

                    case MM_GROTTO_DODONGO_ENTR:
                    {   // Termina dodongo grotto entry

                        Message.SceneID = MM_GROTTO_TERMINA_DODONGO;
                        break;
                    }

                    case MM_GROTTO_JP_CLIMB_LEFT_ENTR:
                    case MM_GROTTO_JP_CLIMB_RIGHT_ENTR:
                    {   // MM JP layout deku palace climb grotto entry

                        Message.SceneID = MM_GROTTO_DEKU_PALACE_CLIMB;
                        break;
                    }

                    case MM_GROTTO_SCRUB_ENTR:
                    {   // Termina field scrub grotto entry

                        Message.SceneID = MM_GROTTO_TERMINA_SCRUB;
                        break;
                    }
                    
                    case MM_GROTTO_BIO_BABA_ENTR:
                    {   // Termina bio baba grotto entry

                        Message.SceneID = MM_GROTTO_TERMINA_BIO_BABA;
                        break;
                    }

                    case MM_GROTTO_BEAN_ENTR:
                    {   // Deku palace beans grotto entry

                        Message.SceneID = MM_GROTTO_DEKU_PALACE_BEANS;
                        break;
                    }

                    case MM_GROTTO_PEAHAT_ENTR:
                    {   // Termina peahat grotto entry

                        Message.SceneID = MM_GROTTO_TERMINA_PEAHAT;
                        break;
                    }

                    case MM_GROTTO_DEKU_PLAYGROUND_ENTR:
                    {   // Clock town deku playground grotto entry

                        Message.SceneID = MM_DEKU_PLAYGROUND;
                        break;
                    }
                }

                break;
            }

            case MM_CUTSCENE_MAP:
            {
                Message.SceneID = MM_EXTRA;
                break;
            }

            case MM_ZORA_HALL_ROOMS:
            {
                switch (Message.EntranceID)
                {
                    case MM_ROOM_EVANS_ENTR:
                    {
                        Message.SceneID = MM_ZORA_EVANS_ROOM;
                        break;
                    }

                    case MM_ROOM_JAPAS_ENTR:
                    {
                        Message.SceneID = MM_ZORA_JAPAS_ROOM;
                        break;
                    }

                    case MM_ROOM_TIJO_ENTR:
                    {
                        Message.SceneID = MM_ZORA_TIJO_ROOM;
                        break;
                    }

                    case MM_ROOM_LULU_ENTR:
                    {
                        Message.SceneID = MM_ZORA_LULU_ROOM;
                        break;
                    }

                    case MM_ZORA_SHOP_ENTR:
                    {
                        Message.SceneID = MM_ZORA_SHOP;
                        break;
                    }
                }
            }

            case MM_CASTLE_IKANA:
            {
                if (Message.CurrRoom != 0)
                {
                    Message.SceneID = MM_INSIDE_CASTLE_IKANA;
                }
                break;
            }

            case MM_MOON_DEKU:
            {
                if (Message.EntranceID == MM_MOON_ENTR)
                {
                    return MM_MOON_FROM_MOON_DEKU_ENTR;
                }
            }

            case MM_MOON_GORON:
            {
                if (Message.EntranceID == MM_MOON_ENTR)
                {
                    return MM_MOON_FROM_MOON_GORON_ENTR;
                }
            }

            case MM_MOON_ZORA:
            {
                if (Message.EntranceID == MM_MOON_ENTR)
                {
                    return MM_MOON_FROM_MOON_ZORA_ENTR;
                }
                else if (Message.EntranceID == MM_MOON_ZORA_FROM_WRONG_PIPE_ENTR && Message.Direction == OUT_MAGIC)
                {
                    return MM_MOON_ZORA_WRONG_PIPE_ENTR;
                }
            }

            case MM_MOON_LINK:
            {
                if (Message.EntranceID == MM_MOON_ENTR)
                {
                    return MM_MOON_FROM_MOON_LINK_ENTR;
                }
            }

            case MM_LAIR_MAJORA:
            {
                if (Message.Direction == OUT_MAGIC)
                {   // Beating majora should not bring to a shuffuled entrance

                    Message.SceneID = WARP_SCENE;
                }
              
                break;
            }

            case MM_DAMPE_HOUSE:
            {
                if (Message.EntranceID == MM_IKANA_GRAVEYARD_FROM_DAMPE_ENTR)
                {
                    return MM_DAMPE_TO_GRAVEYARD_ENTR;
                }
                break;
            }

            case MM_IKANA_GRAVEYARD:
            {
                if (Message.EntranceID == MM_GRAVE_NIGHT3_ENTR)
                {
                    return MM_GRAVE_EXIT_NIGHT3;
                }
                break;
            }

            case MM_FAIRY_FOUNTAIN:
            {
                switch (Message.EntranceID)
                {
                    case MM_FAIRY_FOUNTAIN_TOWN_ENTR:
                    {
                        return MM_FAIRY_CLOCK_TOWN;
                    }

                    case MM_FAIRY_FOUNTAIN_WOODFALL_ENTR:
                    {
                        return MM_FAIRY_WOODFALL;
                    }

                    case MM_FAIRY_FOUNTAIN_SNOWHEAD_ENTR:
                    {
                        return MM_FAIRY_SNOWHEAD;
                    }

                    case MM_FAIRY_FOUNTAIN_GREAT_BAY_ENTR:
                    {
                        return MM_FAIRY_GREAT_BAY_COAST;
                    }

                    case MM_FAIRY_FOUNTAIN_IKANA_ENTR:
                    {
                        return MM_FAIRY_IKANA;
                    }
                }
            }

            case MM_RANCH_HOUSE_BARN:
            {
                if (Message.EntranceID == MM_RANCH_HOUSE_ENTR)
                {
                    return MM_ROMANI_RANCH_BARN;
                }
            }
        }
    }

    return Message.EntranceID;
}


uint32_t EntranceHelper::CheckWrapScene(EntranceMessage& Message)
{
    if (Message.GameID == OOT_GAME)
    {
        switch (Message.SceneID)
        {
            case OOT_LAIR_GOHMA:
            {
                return OOT_BOSS_DEKU_TREE_WARP_OUT;
            }

            case OOT_LAIR_KING_DODONGO:
            {
                return OOT_BOSS_DODONGO_CAVERN_WARP_OUT;
            }
            
            case OOT_LAIR_BARINADE:
            {
                return OOT_BOSS_JABU_JABU_WARP_OUT;
            }

            case OOT_LAIR_PHANTOM_GANON:
            {
                return OOT_BOSS_FOREST_TEMPLE_WARP_OUT;
            }

            case OOT_LAIR_VOLVAGIA:
            {
                return OOT_BOSS_FIRE_TEMPLE_WARP_OUT;
            }

            case OOT_LAIR_MORPHA:
            {
                return OOT_BOSS_WATER_TEMPLE_WARP_OUT;
            }

            case OOT_LAIR_BONGO_BONGO:
            {
                return OOT_BOSS_SHADOW_TEMPLE_WARP_OUT;
            }

            case OOT_LAIR_TWINROVA:
            {
                return OOT_BOSS_SPIRIT_TEMPLE_WARP_OUT;
            }
        }
    }
    else if (Message.GameID == MM_GAME)
    {
        switch (Message.SceneID)
        {
            case MM_DEKU_PALACE:
            case MM_DEKU_KING_CHAMBER:
            {
                return MM_DEKU_PALACE_CAUGHT;
            }

            case MM_PIRATE_FORTRESS_ENTRANCE:
            {
                return MM_PIRATE_ENTRANCE_CAUGHT;
            }

            case MM_PIRATE_FORTRESS_INTERIOR:
            case MM_PIRATE_FORTRESS_EXTERIOR:
            {
                return MM_PIRATE_ENTRANCE_FROM_EXTERIOR_CAUGHT;
            }

            case MM_LAIR_ODOLWA:
            {
                return MM_BOSS_TEMPLE_WOODFALL_WARP_OUT;
            }

            case MM_LAIR_GOHT:
            {
                return MM_BOSS_TEMPLE_SNOWHEAD_WARP_OUT;
            }

            case MM_LAIR_GYORG:
            {
                return MM_BOSS_TEMPLE_GREAT_BAY_WARP_OUT;
            }

            case MM_LAIR_TWINMOLD:
            {
                return MM_BOSS_TEMPLE_STONE_TOWER_INVERTED_WARP_OUT;
            }

            case MM_CLOCK_TOWN_SOUTH:
            {
                Message.SceneID = WARP_SCENE;
                return Message.EntranceID;
            }
        }
    }


    // Fallback to spawn loading
    Message.SceneID = WARP_SCENE;
    return Message.EntranceID;
}


void EntranceHelper::ParseEntranceMessage(uint32_t EntranceFlag, uint32_t Buffer[6])
{
    if ((EntranceFlag & 0xFF000000) == IN_MAGIC)
    {
        this->InMessage.SetMessage(IN_MAGIC, EntranceFlag, Buffer);
        this->ParseIncomingMessage(this->InMessage);
    }
    else if ((EntranceFlag & 0xFF000000) == OUT_MAGIC)
    {
        this->OutMessage.SetMessage(OUT_MAGIC, EntranceFlag, Buffer);
        this->ParseOutgoingMessage(this->OutMessage);
    }
}


void EntranceHelper::ParseIncomingMessage(EntranceMessage& Message)
{
    if (this->IsEntranceTouched)
    {
        Message.EntranceID = this->CheckSpecialCase(Message);

        if (this->IsDeath(this->OutMessage, Message) ||
            this->IsNewCycle(this->OutMessage, Message) ||
            this->IsSongOfDoubleTime(this->OutMessage, Message) ||
            this->IsSongOfTime(this->OutMessage, Message) ||
            this->IsSunSong(this->OutMessage, Message) ||
            this->IsSonataWoodfall(this->OutMessage, Message) ||
            this->IsSpawn(Message))
        {   
            this->IsEntranceTouched = false;
            return;
        }

        if (this->IsGrottoEntrance(Message))
        {
            Message.EntranceID = this->GetGrottoEntrance(Message, this->OutMessage.SceneID);
            Message.SceneID = this->CorrectGrottoScene(Message);
        }
        else if (this->IsGrottoExit(Message))
        {
            Message.EntranceID = this->GetGrottoExit(Message);
        }
        else
        {
            Message.EntranceID = this->CheckGrottoSpawn(Message);
        }

        Message.MetaInf = const_cast<EntranceMetaInfo*>(LookupEntrance(Message.GameID, Message.EntranceID));

        Message.EntranceStr = Message.MetaInf->ToName + std::string(" - ") + Message.MetaInf->FromName;
        MultiLogger::LogMessage("X = %f, Y = %f, Z = %f", Message.X, Message.Y, Message.Z);
        MultiLogger::LogMessage("New scene Loaded ! From : %s (0x%X), To : %s (0x%X)", this->OutMessage.EntranceStr.c_str(), this->OutMessage.EntranceID, Message.EntranceStr.c_str(), Message.EntranceID);
        
        if (this->OutMessage.MetaInf->Type == EntranceType::One_Way_In)
        {
            MultiLogger::LogMessage("Warning ! Entrance %s (0x%X) is one way in only !", this->OutMessage.EntranceStr.c_str(), this->OutMessage.EntranceID);
        }
        if (Message.MetaInf->Type == EntranceType::One_Way_Out)
        {
            MultiLogger::LogMessage("Warning ! Entrance %s (0x%X) is one way out only !", Message.EntranceStr.c_str(), Message.EntranceID);
        }

        if (this->OutMessage.MetaInf->Type == EntranceType::One_Way_Out)
        {
            this->OutMessage.SceneID = this->OutMessage.MetaInf->ToSceneID;
            this->OutMessage.EntranceID = this->OutMessage.MetaInf->ToEntranceID;
        }
        else
        {
            this->OutMessage.SceneID = this->OutMessage.MetaInf->FromSceneID;
            this->OutMessage.EntranceID = this->OutMessage.MetaInf->FromEntranceID;
        }

        SceneEntranceMetaInf * tmp = GetSceneEntranceMetaInf(this->OutMessage.GameID, this->OutMessage.SceneID);
        EntranceLink * tmpOutLink = &tmp->EntranceIDs.find(this->OutMessage.EntranceID)->second;
        tmpOutLink->OutLink = Message.EntranceID;
        tmpOutLink->OutLinkGame = Message.GameID;

        SceneEntranceUpdate tmpOut = { this->OutMessage.GameID, this->OutMessage.SceneID, this->OutMessage.EntranceID, tmpOutLink };

        tmp = GetSceneEntranceMetaInf(Message.GameID, Message.SceneID);
        EntranceLink * tmpInLink = &tmp->EntranceIDs.find(Message.EntranceID)->second;
        tmpInLink->InLink = this->OutMessage.EntranceID;
        tmpInLink->InLinkGame = this->OutMessage.GameID;

        SceneEntranceUpdate tmpIn = { Message.GameID, Message.SceneID, Message.EntranceID, tmpInLink };

        emit MultiLogger::GetLogger()->NotifyEntranceFound(&tmpOut, &tmpIn);

        MultiLogger::LogMessage("              -----------------------------------");
        MultiLogger::LogMessage("              |      FROM      |       TO       |");
        MultiLogger::LogMessage("-------------------------------------------------");
        MultiLogger::LogMessage("%-13s | %14.6f | %14.6f |", "X", this->OutMessage.X, Message.X);
        MultiLogger::LogMessage("-------------------------------------------------");
        MultiLogger::LogMessage("%-13s | %14.6f | %14.6f |", "Y", this->OutMessage.Y, Message.Y);
        MultiLogger::LogMessage("-------------------------------------------------");
        MultiLogger::LogMessage("%-13s | %14.6f | %14.6f |", "Z", this->OutMessage.Z, Message.Z);
        MultiLogger::LogMessage("-------------------------------------------------");
        MultiLogger::LogMessage("%-13s | %14s | %14s |", "Game", this->OutMessage.GameID == OOT_GAME ? "OoT" : "MM", Message.GameID == OOT_GAME ? "OoT" : "MM");
        MultiLogger::LogMessage("-------------------------------------------------");
        MultiLogger::LogMessage("%-13s |     0x%08X |     0x%08X |", "Scene", this->OutMessage.SceneID, Message.SceneID);
        MultiLogger::LogMessage("-------------------------------------------------");
        MultiLogger::LogMessage("%-13s |     0x%08X |     0x%08X |", "Entrance", this->OutMessage.EntranceID, Message.EntranceID);
        MultiLogger::LogMessage("-------------------------------------------------");
        MultiLogger::LogMessage("%-13s |           0x%02X |           0x%02X |", "Room ID", this->OutMessage.CurrRoom, Message.CurrRoom);
        MultiLogger::LogMessage("-------------------------------------------------");
        MultiLogger::LogMessage("%-13s |           0x%02X |           0x%02X |", "Grotto Data", this->OutMessage.GrottoData, Message.GrottoData);
        MultiLogger::LogMessage("-------------------------------------------------");
        MultiLogger::LogMessage("%-13s |           0x%02X |           0x%02X |", "Age", this->OutMessage.Age, Message.Age);
        MultiLogger::LogMessage("-------------------------------------------------");
        MultiLogger::LogMessage("%-13s |           0x%02X |           0x%02X |", "Farore's Wind", this->OutMessage.FaroreWind, Message.FaroreWind);
        MultiLogger::LogMessage("-------------------------------------------------");
        MultiLogger::LogMessage("%-13s |           0x%02X |           0x%02X |", "Owl ID", this->OutMessage.OwlID, Message.OwlID);
        MultiLogger::LogMessage("-------------------------------------------------");
        MultiLogger::LogMessage("%-13s |           0x%02X |           0x%02X |", "Song ID", this->OutMessage.OoTSongID, Message.OoTSongID);
        MultiLogger::LogMessage("-------------------------------------------------");
    }

    this->IsEntranceTouched = false;
}


void EntranceHelper::ParseOutgoingMessage(EntranceMessage& Message)
{
    if (this->IsMMExtra(Message))
    {
        this->IsEntranceTouched = false;
        return;
    }

    this->IsEntranceTouched = true;

    // Check that the entrance is not a special case
    Message.EntranceID = this->CheckSpecialCase(Message);
    
    if (Message.SceneID == WARP_SCENE)
    {   // We don't want to catch this

        this->IsEntranceTouched = false;
        return;
    }

    if (this->IsGrottoEntrance(Message))
    {   // The current entrance is a grotto entrance

        Message.EntranceID = this->GetGrottoEntrance(Message, Message.SceneID);
        //this->OutScene = this->CorrectGrottoScene(this->OutGame, this->OutEntrance);
    }
    else if (this->IsGrottoExit(Message))
    {   // The current entrance is a grotto exit

        Message.EntranceID = this->GetGrottoExit(Message);
        Message.SceneID = this->CorrectGrottoScene(Message);
    }
    else if (this->IsWarpEntrance(Message))
    {   // The current entrance is a warp zone

        bool isWarpSong = false;
        Message.EntranceID = this->GetWarpSong(Message, &isWarpSong);
        
        if (!isWarpSong)
        {
            Message.EntranceID = this->CheckWrapScene(Message);

            if (Message.SceneID == WARP_SCENE)
            {   // We don't want to catch this

                this->IsEntranceTouched = false;
                return;
            }
        }
    }
    else if (this->IsFaroreWind(Message))
    {   // We don't want to catch this

        this->IsEntranceTouched = false;
        return;
    }

    // Retreive the entrance meta information. Entrance IDs are unique per game so a single lookup
    // is enough; no layout disambiguation needed.
    Message.MetaInf = const_cast<EntranceMetaInfo*>(LookupEntrance(Message.GameID, Message.EntranceID));
    Message.EntranceStr = Message.MetaInf->FromName + std::string(" \xE2\x86\x92 ") + Message.MetaInf->ToName;

    if (Message.MetaInf->FromSceneID != Message.SceneID)
    {   // Spawning location or other inconsistency

        this->IsEntranceTouched = false;
    }
}


const char* EntranceHelper::GetEntranceFromName(int Game, uint32_t EntranceID)
{
    const EntranceMetaInfo* entrance = LookupEntrance(Game, EntranceID);
    return entrance != nullptr ? entrance->FromName : nullptr;
}


const char* EntranceHelper::GetEntranceToName(int Game, uint32_t EntranceID)
{
    const EntranceMetaInfo* entrance = LookupEntrance(Game, EntranceID);
    return entrance != nullptr ? entrance->ToName : nullptr;
}


std::string EntranceHelper::GetOneWayInName(int Game, uint32_t EntranceID)
{
    const EntranceMetaInfo* entrance = LookupEntrance(Game, EntranceID);

    switch (entrance->Type)
    {
        case EntranceType::Normal:
        {   // We need to build the string

            return std::string(entrance->FromName + std::string(" \xE2\x86\x92 ") + entrance->ToName);
        }

        default:
        {
            return std::string(entrance->ToName);
        }
    }
}



std::string EntranceHelper::GetOneWayOutName(int Game, uint32_t EntranceID)
{
    const EntranceMetaInfo* entrance = LookupEntrance(Game, EntranceID);

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
    const EntranceMetaInfo* entrance = LookupEntrance(Game, EntranceID);

    switch (entrance->Type)
    {
        case EntranceType::One_Way_Out:
        {   // We need to build the string

            return std::string(entrance->FromName + std::string(" \xE2\x86\x92 ") + entrance->ToName);
        }

        default:
        {
            return std::string(entrance->ToName + std::string(" \xE2\x86\x92 ") + entrance->FromName);
        }
    }

}


std::string EntranceHelper::GetEntranceLeadsString(int Game, uint32_t EntranceID)
{
    const EntranceMetaInfo* entrance = LookupEntrance(Game, EntranceID);

    return std::string(entrance->ToName + std::string(" - ") + entrance->FromName);
}

const EntranceMetaInfo* EntranceHelper::GetEntranceMetaInf(int Game, uint32_t EntranceID)
{
    return LookupEntrance(Game, EntranceID);
}


bool EntranceMetaInfo::HasCorrectLayout(GameLayout Layout) const
{
    return this->ActiveLayout == GameLayout::all || this->ActiveLayout == Layout;
}


const int* GetEntranceAnchorPos(const EntranceMetaInfo& Meta, int Game, uint32_t SceneID, GameLayout Layout)
{
    // Bean Grotto in Deku Palace is the only entrance whose on-image anchor differs between MM
    // layouts. Keeping the override here (instead of in EntranceMetaInfo) avoids polluting every
    // entry with an unused "alt" field. If a second such case ever appears, replace the if with a
    // small lookup table keyed on (Game, SceneID, EntranceID, Layout).
    if (Game == MM_GAME && Meta.ToEntranceID == MM_GROTTO_EXIT_BEAN && SceneID == MM_DEKU_PALACE && Layout == GameLayout::mm_jp)
    {
        return MM_BEAN_GROTTO_JP_ANCHOR;
    }
    return Meta.AnchorPos;
}
