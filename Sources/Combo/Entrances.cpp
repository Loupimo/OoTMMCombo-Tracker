#include "Combo/Entrances.h"
#include "Combo/OoTEntrances.h"
#include "Combo/MMEntrances.h"
#include "UI/SceneEntrance.h"
#include "Multi/Multi.h"
#include <math.h>

// Contains all warp positions used to determine the correct entrance when spawning using a warp method (boss warp, songs, caught, ...)
/*static const std::vector<GrottoEntrance> SpecialWarps =
{
    { MM_DEKU_KING_CAUGHT, -6, 0, -323 },                   // Deku's King caught
    { MM_PIRATE_ENTRANCE_CAUGHT, -2808, 14, 130 },          // Pirate entrance caught
    { MM_BOSS_TEMPLE_SNOWHEAD_WARP_OUT, 1384, 0, -1396 }    // Goht's win warp crystal
};*/

// Contains all grotto entrances positions used to determine the correct entrance when spawning in a zone that has at least one grotto
static const std::map<int, std::vector<GrottoEntrance>> GrottoEntrances =
{ 
    { OOT_KOKIRI_FOREST_FROM_LOST_WOODS_ENTR,       std::vector<GrottoEntrance>() = { { OOT_FOREST_FROM_LOST_WOODS_BRIDGE_ENTR,          -1413,   -74,  -283 },
                                                                                      { OOT_KOKIRI_FOREST_FROM_DEKU_TREE_ENTR,            3844,  -161, -1080 },
                                                                                      { OOT_KOKIRI_FOREST_FROM_MIDO_ENTR,                 -445,     0,  -486 },
                                                                                      { OOT_KOKIRI_FOREST_FROM_SARIA_ENTR,                 516,     0,   629 },
                                                                                      { OOT_KOKIRI_FOREST_FROM_TWINS_ENTR,                1036,     0,   524 },
                                                                                      { OOT_KOKIRI_FOREST_FROM_KNOW_IT_ALL_ENTR,         -1034,   120,   394 },
                                                                                      { OOT_KOKIRI_FOREST_FROM_SHOP_ENTR,                  854,     0,  -272 },
                                                                                      { OOT_KOKIRI_FOREST_FROM_LINK_ENTR,                  -31,   100,  -114 },
                                                                                      { OOT_GROTTO_EXIT_GENERIC_KOKIRI_FOREST,            -512,   380, -1224 } } },
    { OOT_LOST_WOODS_FROM_KOKIRI_FOREST_ENTR,       std::vector<GrottoEntrance>() = { { OOT_LOST_WOODS_FROM_KOKIRI_FOREST_ENTR,             -1,     0,   305 },
                                                                                      { OOT_LOST_WOODS_FROM_MEADOW_ENTR,                   802,     0, -2589 },
                                                                                      { OOT_LOST_WOODS_BRIDGE_FROM_FOREST_ENTR,           -901,  -200,  1602 },
                                                                                      { OOT_LOST_WOODS_BRIDGE_FROM_FIELD_ENTR,           -1502,  -200,  1600 },
                                                                                      { OOT_LOST_WOODS_FROM_ZORA_RIVER_ENTR,              2134,  -196,  -851 },
                                                                                      { OOT_LOST_WOODS_FROM_GORON_CITY_ENTR ,              797,   -15, -1091 },
                                                                                      { OOT_GROTTO_EXIT_GENERIC_LOST_WOODS,                915,     0,  -925 },
                                                                                      { OOT_GROTTO_EXIT_SCRUB_UPGRADE,                     670,     0, -2520 },
                                                                                      { OOT_GROTTO_EXIT_DEKU_THEATER,                       80,   -20, -1600 } } },
    { OOT_SACRED_FOREST_MEADOW_ENTR,                std::vector<GrottoEntrance>() = { { OOT_SACRED_FOREST_MEADOW_ENTR,                    -201,     0,  2183 },
                                                                                      { OOT_WARP_SONG_MEADOW_ENTR,                          10,   500, -2610 },
                                                                                      { OOT_SACRED_MEADOW_FROM_TEMPLE_FOREST_ENTR,          -2,   680, -3180 },
                                                                                      { OOT_GROTTO_EXIT_FAIRY_SFM,                          45,     0,   220 },
                                                                                      { OOT_GROTTO_EXIT_WOLFOS,                           -195,     0,  1900 },
                                                                                      { OOT_GROTTO_EXIT_SCRUBS2_SFM,                       310,   480, -2300 } } },
    { OOT_KAKARIKO_FROM_FIELD_ENTR,                 std::vector<GrottoEntrance>() = { { OOT_KAKARIKO_FROM_FIELD_ENTR,                    -2638,   138,  1065 },
                                                                                      { OOT_KAKARIKO_FROM_GRANNY_ENTR,                     780,   200,    24 },
                                                                                      { OOT_KAKARIKO_FROM_BOTTOM_OF_THE_WELL_ENTR,         846,  -500,   520 },
                                                                                      { OOT_KAKARIKO_FROM_DEATH_MOUNTAIN_ENTR,             -97,   511, -1787 },
                                                                                      { OOT_KAKARIKO_FROM_GRAVEYARD_ENTR,                 1901,   189,  1385 },
                                                                                      { OOT_KAKARIKO_FROM_CARPENTER_ENTR,                  -51,     0,   224 },
                                                                                      { OOT_KAKARIKO_FROM_SKULLTULA_ENTR,                 -287,    80,  1017 },
                                                                                      { OOT_KAKARIKO_FROM_IMPA_ENTR,                      -373,   240,  1702 },
                                                                                      { OOT_KAKARIKO_FROM_IMPA_BACK_ENTR,                  260,   240,  1730 },
                                                                                      { OOT_KAKARIKO_FROM_WINDMILL_ENTR,                  1183,   260,   523 },
                                                                                      { OOT_KAKARIKO_FROM_SHOP_POTION_ENTR,                119,   320,  -567 },
                                                                                      { OOT_KAKARIKO_FROM_SHOP_POTION_BACK_ENTR,           592,   320,  -586 },
                                                                                      { OOT_KAKARIKO_FROM_BAZAAR_ENTR,                     -191,  320,  -650 },
                                                                                      { OOT_KAKARIKO_FROM_ARCHERY_ENTR,                    323,   160,   784 },
                                                                                      { OOT_VILLAGE_OWL_ENTR,                              -65,   620,  1520 },
                                                                                      { OOT_GROTTO_EXIT_REDEAD,                           -400,     0,   400 },
                                                                                      { OOT_GROTTO_EXIT_GENERIC_KAKARIKO,                  860,    80,  -260 } } },
    { OOT_LAKE_HYLIA_FROM_FIELD_ENTR,               std::vector<GrottoEntrance>() = { { OOT_LAKE_HYLIA_FROM_FIELD_ENTR,                  -2093, -1037,   705 },
                                                                                      { OOT_WARP_SONG_LAKE_ENTR,                         -1045, -1218,  3871 },
                                                                                      { OOT_LAKE_HYLIA_FROM_LABORATORY_ENTR,             -2422, -1033,  3858 },
                                                                                      { OOT_LAKE_HYLIA_FROM_VALLEY_ENTR,                 -3918, -1142,  2533 },
                                                                                      { OOT_LAKE_HYLIA_FROM_FISHING_POND_ENTR,            1322, -1218,  3871 },
                                                                                      { OOT_LAKE_HYLIA_FROM_TEMPLE_WATER_ENTR,            -929, -1313,  6555 }, // Child
                                                                                      { OOT_LAKE_HYLIA_FROM_TEMPLE_WATER_ENTR,            -917, -2201,  6359 }, // Adult
                                                                                      { OOT_LAKE_HYLIA_FROM_ZORA_DOMAIN_ENTR,             -912, -1544,  -280 },
                                                                                      { OOT_GROTTO_EXIT_SCRUBS3_LAKE,                    -3040, -1033,  6075 } } },
    { OOT_DEATH_MOUNTAIN_FROM_KAKARIKO_ENTR,        std::vector<GrottoEntrance>() = { { OOT_DEATH_MOUNTAIN_FROM_KAKARIKO_ENTR,           -1344,   118,  2221 },
                                                                                      { OOT_DEATH_MOUNTAIN_FROM_GORON_CITY_ENTR,          -407,  1191, -1860 },
                                                                                      { OOT_MOUNTAIN_TRAIL_FROM_DODONGO_CAVERN_ENTR,     -1695,   683,  -535 },
                                                                                      { OOT_TRAIL_SUMMIT_FROM_CRATER_ENTR,                 230,  3263, -4670 },
                                                                                      { OOT_DEATH_MOUNTAIN_FROM_FAIRY_ENTR,               -206,  3295, -4391 },
                                                                                      { OOT_GROTTO_EXIT_TRAIL_COW,                        -688,  1946,  -285 },
                                                                                      { OOT_GROTTO_EXIT_GENERIC_DMT,                      -383,  1386, -1206 } } },
    { OOT_DEATH_MOUNTAIN_CRATER_ENTR,               std::vector<GrottoEntrance>() = { { OOT_DEATH_MOUNTAIN_CRATER_ENTR,                  -1121,  1360,  2076 },
                                                                                      { OOT_CRATER_FROM_GORON_CITY_ENTR,                 -1749,   722,    26 },
                                                                                      { OOT_DEATH_CRATER_FROM_FAIRY_ENTR,                -1287,   829,   941 },
                                                                                      { OOT_DEATH_CRATER_FROM_TEMPLE_FIRE_ENTR,             12,  -350, -1419 },
                                                                                      { OOT_WARP_SONG_CRATER_ENTR,                           0,   441,     0 },
                                                                                      { OOT_GROTTO_EXIT_GENERIC_DMC,                        40,  1233,  1770 },
                                                                                      { OOT_GROTTO_EXIT_SCRUBS3_DMC,                     -1699,   722,  -472 } } },
    { OOT_GORON_CITY_ENTR,                          std::vector<GrottoEntrance>() = { { OOT_GORON_CITY_ENTR,                                56,   600,  1104 },
                                                                                      { OOT_GORON_CITY_FROM_LOST_WOODS_ENTR,               441,   196,  1153 },
                                                                                      { OOT_GORON_CITY_FROM_CRATER_ENTR,                    47,    40, -1523 },
                                                                                      { OOT_GORON_CITY_FROM_SHOP_ENTR,                    -134,    -3,   -42  },
                                                                                      { OOT_GROTTO_EXIT_SCRUBS3_GORON_CITY,               1100,   580, -1190 } } },
    { OOT_ZORA_RIVER_FROM_FIELD_ENTR,               std::vector<GrottoEntrance>() = { { OOT_ZORA_RIVER_FROM_FIELD_ENTR,                  -1506,   -20,  1510 },
                                                                                      { OOT_ZORA_RIVER_FROM_FIELD_WATER_ENTR,            -1357,   -86,  1542 },
                                                                                      { OOT_ZORA_RIVER_FROM_LOST_WOODS_ENTR,              4084,   620,  -932 },
                                                                                      { OOT_RIVER_FROM_DOMAIN_ENTR,                       4413,   920, -1403 },
                                                                                      { OOT_GROTTO_EXIT_GENERIC_RIVER,                     360,   570,   130 },
                                                                                      { OOT_GROTTO_EXIT_SCRUBS2_RIVER,                   -1630,   100,  -130 },
                                                                                      { OOT_GROTTO_EXIT_SCRUBS2_RIVER,                   -1630,   100,  -130 } } },
    { OOT_ZORA_DOMAIN_ENTR,                         std::vector<GrottoEntrance>() = { { OOT_ZORA_DOMAIN_ENTR,                            -1112,   210,  -160 },
                                                                                      { OOT_ZORA_DOMAIN_FROM_LAKE_HYLIA_ENTR,             -208,  -210,  -280 },
                                                                                      { OOT_DOMAIN_FROM_FOUNTAIN_ENTR,                     524,    52,   254 },
                                                                                      { OOT_ZORA_DOMAIN_FROM_SHOP_ENTR,                    540,   996, -2503 },
                                                                                      { OOT_GROTTO_EXIT_FAIRY_DOMAIN,                     -860,    14,  -470 } } },
    { OOT_LON_LON_RANCH_FROM_FIELD_ENTR,            std::vector<GrottoEntrance>() = { { OOT_LON_LON_RANCH_FROM_FIELD_ENTR,                1107,     0, -3740 },
                                                                                      { OOT_LON_LON_RANCH_FROM_HOUSE_ENTR,                 988,     1, -3097 },
                                                                                      { OOT_LON_LON_RANCH_FROM_STABLES_ENTR,               728,     1, -2896 },
                                                                                      { OOT_LON_LON_RANCH_FROM_SILO_ENTR,                -1506,     0,  1491 },
                                                                                      { OOT_GROTTO_EXIT_SCRUBS3_RANCH,                    1800,     0,  1500 } } },
    { OOT_HYRULE_CASTLE_ENTR,                       std::vector<GrottoEntrance>() = { { OOT_HYRULE_CASTLE_ENTR,                           -225,  1086,  3743 },
                                                                                      { OOT_HYRULE_CASTLE_FROM_FAIRY_ENTR,                2627,  1142,  2567 },
                                                                                      { OOT_CASTLE_STEALTH_FROM_COURTYARD_ENTR,            575,  1617,  -310 },
                                                                                      { OOT_CASTLE_CAUGHT_ENTR,                            913,  1109,  2937 },
                                                                                      { OOT_GROTTO_EXIT_CASTLE,                            996,  1571,   844 } } },
    { OOT_FIELD_FROM_MARKET_ENTRANCE_ENTR,          std::vector<GrottoEntrance>() = { { OOT_FIELD_FROM_MARKET_ENTRANCE_ENTR,                 1,     0,   697 },      // Child
                                                                                      { OOT_FIELD_FROM_MARKET_ENTRANCE_ENTR,                -6,     0,  1182 },      // Adult
                                                                                      { OOT_FIELD_FROM_LAKE_HYLIA_ENTR,                  -5891,  -881, 15676 },
                                                                                      { OOT_FIELD_FROM_GERUDO_VALLEY_ENTR,               -9645,  -155,  6850 },
                                                                                      { OOT_FIELD_FROM_KAKARIKO_ENTR,                     3759,   252,   232 },
                                                                                      { OOT_FIELD_FROM_ZORA_RIVER_WATER_ENTR,             5977,  -135,  3982 },
                                                                                      { OOT_FIELD_FROM_ZORA_RIVER_ENTR,                   5984,   -20,  3773 },
                                                                                      { OOT_FIELD_FROM_LOST_WOODS_BRIDGE_ENTR,            5137,  -160,  8467 },
                                                                                      { OOT_FIELD_FROM_LON_LON_RANCH_ENTR,               -1850,   196,  5956 },
                                                                                      { OOT_FIELD_FROM_LON_LON_EPONA_JUMP_LEFT,          -1391,   100,  6865 },
                                                                                      { OOT_FIELD_FROM_LON_LON_EPONA_JUMP_MIDDLE,        -2953,   100,  8336 },
                                                                                      { OOT_FIELD_FROM_LON_LON_EPONA_JUMP_RIGHT,         -4668,  -122,  6865 },
                                                                                      { OOT_FIELD_OWL_ENTR,                                  0,    70,  1380 },
                                                                                      { OOT_GROTTO_EXIT_GENERIC_HF_MARKET,               -1425,     0,   810 },
                                                                                      { OOT_GROTTO_EXIT_FAIRY_HF,                        -4450,  -300,  -425 },
                                                                                      { OOT_GROTTO_EXIT_TEKTITE,                         -4945,  -300,  2835 },
                                                                                      { OOT_GROTTO_EXIT_GENERIC_HF_SOUTHEAST,             -270,  -500, 12350 },
                                                                                      { OOT_GROTTO_EXIT_GENERIC_HF_OPEN,                 -4030,  -700, 13860 },
                                                                                      { OOT_GROTTO_EXIT_SCRUB_HEART_PIECE,               -4990,  -700, 13820 },
                                                                                      { OOT_GROTTO_EXIT_FIELD_COW,                       -7870,  -300,  6920 },
                                                                                      { OOT_GROTTO_EXIT_FIELD_TREE,                       2060,    20,  -170 } } },
    { OOT_GERUDO_VALLEY_FROM_FIELD_ENTR,            std::vector<GrottoEntrance>() = { { OOT_GERUDO_VALLEY_FROM_FIELD_ENTR,                2664,  -269,   778 },
                                                                                      { OOT_GERUDO_VALLEY_FROM_TENT_ENTR,                 -973,    15,  -362 },
                                                                                      { OOT_VALLEY_FROM_GERUDO_FORTRESS_ENTR,            -3264,   239,  -757 },
                                                                                      { OOT_GROTTO_EXIT_OCTOROK,                           280,  -555,  1470 },
                                                                                      { OOT_GROTTO_EXIT_SCRUBS2_VALLEY,                  -1323,    15,  -969 } } },
    { OOT_GERUDO_FORTRESS_FROM_VALLEY_ENTR,         std::vector<GrottoEntrance>() = { { OOT_GERUDO_FORTRESS_FROM_VALLEY_ENTR,             -842,     3,   -84 },
                                                                                      { OOT_GERUDO_FORTRESS_FROM_GERUDO_TRAINING_ENTR,      40,   333, -1022 },
                                                                                      { OOT_FORTRESS_FROM_WASTELAND_ENTR,                -1786,    12, -3382 },
                                                                                      { OOT_GERUDO_FORTRESS_CAUGHT_ENTR,                   188,   733, -2919 },
                                                                                      { OOT_FORTRESS_FROM_HIDEOUT_BREAKOUT,                763,   640, -2662 },
                                                                                      { OOT_FORTRESS_FROM_HIDEOUT_BREAKOUT_JAIL,           314,  1113, -2982 },
                                                                                      { OOT_FORTRESS_FROM_HIDEOUT_LEFT_JAIL_1,             200,   333, -2554 },
                                                                                      { OOT_FORTRESS_FROM_HIDEOUT_RIGHT_JAIL_1,            410,   333, -1794 },
                                                                                      { OOT_FORTRESS_FROM_HIDEOUT_KITCHEN_BOTTOM,          626,   333, -1718 },
                                                                                      { OOT_FORTRESS_FROM_HIDEOUT_KITCHEN_BOTTOM_TO_TOP,   629,   533, -1401 },
                                                                                      { OOT_FORTRESS_FROM_HIDEOUT_KITCHEN_TOP_RIGHT,       931,   733, -1404 },
                                                                                      { OOT_FORTRESS_FROM_HIDEOUT_KITCHEN_TOP_LEFT,       1230,   834, -1769 },
                                                                                      { OOT_FORTRESS_FROM_HIDEOUT_JAIL_2_TOP,              947,   733, -1207 },
                                                                                      { OOT_FORTRESS_FROM_HIDEOUT_JAIL_2_BOTTOM,           325,   572, -1084 },
                                                                                      { OOT_FORTRESS_FROM_HIDEOUT_JAIL_3_TOP,              679,   533, -2056 },
                                                                                      { OOT_FORTRESS_FROM_HIDEOUT_JAIL_3_BOTTOM,           222,   333, -1347 },
                                                                                      { OOT_FORTRESS_FROM_HIDEOUT_JAIL_4,                 1249,   653, -2061 },
                                                                                      { OOT_GROTTO_EXIT_FAIRY_FORTRESS,                    376,   333, -1564 } } },
    { OOT_COLOSSUS_ENTR,                            std::vector<GrottoEntrance>() = { { OOT_COLOSSUS_ENTR,                                3789,   -38, -1101 },
                                                                                      { OOT_DESERT_COLOSSUS_FROM_FAIRY_ENTR,              2260,   105, -1425 },
                                                                                      { OOT_DESERT_COLOSSUS_FROM_TEMPLE_SPIRIT_ENTR,     -1548,    48,     2 },
                                                                                      { OOT_WARP_SONG_DESERT_ENTR,                        -850,    20,  1542 },
                                                                                      { OOT_GROTTO_EXIT_SCRUBS2_COLOSSUS,                   60,   -32, -1300 } } },



    { MM_TERMINA_FIELD_FROM_CLOCK_TOWN_SOUTH_ENTR,  std::vector<GrottoEntrance>() = { { MM_TERMINA_FIELD_FROM_CLOCK_TOWN_NORTH_ENTR,      -400,    48, -2520 },
                                                                                      { MM_GROTTO_EXIT_GOSSIPS_MOUNTAIN,                   192,    48, -3138 },
                                                                                      { MM_GROTTO_EXIT_DODONGO,                          -2425,  -281, -3291 },
                                                                                      { MM_TERMINA_FIELD_FROM_CLOCK_TOWN_WEST_ENTR,      -2400,    68,  -400 },
                                                                                      { MM_GROTTO_EXIT_GOSSIPS_OCEAN,                    -2782,    48, -1654 },
                                                                                      { MM_GROTTO_EXIT_BIO_BABA,                         -5159,  -281,  -571 },
                                                                                      { MM_TERMINA_FIELD_FROM_CLOCK_TOWN_EAST_ENTR,       1672,    68,  -394 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_FIELD_PILLAR,              2367,   315,  -192 },
                                                                                      { MM_GROTTO_EXIT_GOSSIPS_CANYON,                    4450,   254,   925 },
                                                                                      { MM_GROTTO_EXIT_SCRUB,                             3223,   219,  1417 },
                                                                                      { MM_TERMINA_FIELD_FROM_CLOCK_TOWN_SOUTH_ENTR,      -412,   -77,  1681 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_GRASS,                     1012,  -221,  3642 },
                                                                                      { MM_GROTTO_EXIT_COW_FIELD,                         -375,  -222,  3976 },
                                                                                      { MM_GROTTO_EXIT_GOSSIPS_SWAMP,                    -1592,  -222,  4622 },
                                                                                      { MM_GROTTO_EXIT_PEAHAT,                           -2317,  -221,  3418 } } },
    { MM_SWAMP_ROAD_FROM_FIELD_ENTR,                std::vector<GrottoEntrance>() = { { MM_SWAMP_ROAD_FROM_FIELD_ENTR,                     331,  -143,   245 },
                                                                                      { MM_SWAMP_ROAD_FROM_SWAMP_ENTR,                     413,  -236,  3853 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_PATH_SWAMP,                 104,  -182,  2202 }, } },
    { MM_SWAMP_FROM_SPIDER_HOUSE_ENTR,              std::vector<GrottoEntrance>() = { { MM_SWAMP_FROM_SPIDER_HOUSE_ENTR,                 -1049,    12,  2042 },
                                                                                      { MM_SWAMP_FROM_ROAD_ENTR,                          -191,    61, -1410 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_SWAMP,                    -1700,    38,  1800 }} },
    { MM_MYSTERY_WOODS_ENTR,                        std::vector<GrottoEntrance>() = { { MM_MYSTERY_WOODS_ENTR,                             274,     0,     0 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_WOODS,                        2,     0,  -889 } } },
    { MM_WARP_OWL_MOUNTAIN_VILLAGE_ENTR,            std::vector<GrottoEntrance>() = { { MM_WARP_OWL_MOUNTAIN_VILLAGE_ENTR,                -649,     8,  -196 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_MOUNTAIN_VILLAGE,          2406,  1168, -1197 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_MOUNTAIN_VILLAGE_WINTER,    345,     8,  -150 } } },
    { MM_TWIN_ISLANDS_FROM_MOUNTAIN_VILLAGE_ENTR,   std::vector<GrottoEntrance>() = { { MM_TWIN_ISLANDS_FROM_MOUNTAIN_VILLAGE_ENTR,      -2044,   200,  1288 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_TWIN_ISLANDS,             -1309,   320,   143 },
                                                                                      { MM_GROTTO_EXIT_HOT_WATER,                          589,   195,    53 } } },
    { MM_PATH_SNOWHEAD_FROM_SNOWHEAD_ENTR,          std::vector<GrottoEntrance>() = { { MM_PATH_SNOWHEAD_FROM_SNOWHEAD_ENTR,             -2518,   550, -3441 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_PATH_SNOWHEAD,             -987,   360, -2339 }, } },
    { MM_GREAT_BAY_COAST_FROM_FISHER_HUT_ENTR,      std::vector<GrottoEntrance>() = { { MM_GREAT_BAY_COAST_FROM_FISHER_HUT_ENTR,          1137,    92,  4635 },
                                                                                      { MM_GREAT_BAY_COAST_FROM_FIELD_ENTR,               3585,    80,  4394 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_GREAT_BAY_COAST,           1359,    80,  5018 },
                                                                                      { MM_GREAT_BAY_COAST_FROM_LABORATORY_ENTR,         -3080,   240,  4080 },
                                                                                      { MM_GROTTO_EXIT_COW_COAST,                         2077,   333,  -215 } } },
    { MM_ZORA_CAPE_FROM_GREAT_BAY_COAST_ENTR,       std::vector<GrottoEntrance>() = { { MM_ZORA_CAPE_FROM_GREAT_BAY_COAST_ENTR,             92,    12,   333 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_ZORA_CAPE,                 -562,    80,  2707 } } },
    { MM_IKANA_ROAD_FROM_FIELD_ENTR,                std::vector<GrottoEntrance>() = { { MM_IKANA_ROAD_FROM_FIELD_ENTR,                   -3006,     0,  -305 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_PATH_IKANA,                -428,   200,  -335 } } },
    { MM_IKANA_GRAVEYARD_FROM_DAMPE_ENTR,           std::vector<GrottoEntrance>() = { { MM_IKANA_GRAVEYARD_FROM_DAMPE_ENTR,               -504,   314, -2159 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_GRAVEYARD,                  106,   314, -1777 } } },
    { MM_IKANA_VALLEY_FROM_ROAD_ENTR,               std::vector<GrottoEntrance>() = { { MM_IKANA_VALLEY_FROM_ROAD_ENTR,                    -64,  -371,  4870 },
                                                                                      { MM_GROTTO_EXIT_GENERIC_VALLEY,                   -2475,  -505,  2475 } } }
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


bool EntranceHelper::IsNewCycle(uint32_t Buffer[6])
{
    if ((Buffer[0] & 0xFF) == MM_GAME && Buffer[1] == 0)
    {
        return true;
    }
    return false;
}


bool EntranceHelper::IsMMExtra(uint32_t Buffer[6])
{
    switch (Buffer[0] & 0xFF)
    {
        case MM_GAME:
        {
            switch (Buffer[1])
            {   // Scene ID

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

            switch (Buffer[2])
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

bool EntranceHelper::IsSunSong(uint32_t Scene, uint32_t EntranceID)
{
    // WARNING ! Sun song OoT and Song of Soaring MM have the same ID, this could lead to owl warp missing.
    return ((this->OutBuffer[0] & 0xFF000000) >> 16) == WarpSong::Sun_Song && Scene == this->OutBuffer[1] && EntranceID == this->OutBuffer[2];
}

bool EntranceHelper::IsSongOfTime(uint32_t Buffer[6])
{
    return ((this->OutBuffer[0] & 0xFF000000) >> 24) == WarpSong::Song_of_Time
            && Buffer[1] == this->OutBuffer[1]
            && Buffer[2] == this->OutBuffer[2]
            && Buffer[3] == this->OutBuffer[3]
            && Buffer[4] == this->OutBuffer[4]
            && Buffer[5] == this->OutBuffer[5];
}

bool EntranceHelper::IsSongOfDoubleTime(uint32_t Scene, uint32_t EntranceID)
{
    return ((this->OutBuffer[0] & 0xFF000000) >> 24) == WarpSong::Song_of_Double_Time && Scene == this->OutBuffer[1] && EntranceID == this->OutBuffer[2];
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


uint32_t EntranceHelper::CorrectGrottoScene(uint8_t Game, uint32_t ID)
{
    if (Game == OOT_GAME)
    {
        switch (ID)
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
    else if (Game == MM_GAME)
    {

    }

    return ID;
}


uint32_t EntranceHelper::GetWarpSong(uint8_t * Game, uint32_t ID, uint8_t SongIndex, uint8_t OwlID, bool * IsWarpSong)
{
    *IsWarpSong = true;

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

    return ID;
}


float EntranceHelper::GetDistanceGrottoEntrance(GrottoEntrance Grotto, float X, float Y, float Z)
{
    float xDist = fabsf(X - Grotto.SpawnPos[0]);
    float yDist = fabsf(Y - Grotto.SpawnPos[1]);
    float zDist = fabsf(Z - Grotto.SpawnPos[2]);

    MultiLogger::LogMessage("Entrance ID = 0x%08X, X dist = %f, Y dist = %f, Z dist = %f, Total = %f", Grotto.EntranceID, xDist, yDist, zDist, xDist + yDist + zDist);
    return xDist + yDist + zDist;
}


uint32_t EntranceHelper::CheckGrottoSpawn(uint32_t ID, uint32_t Buffer[6])
{
    float x = 0.0f, y = 0.0f, z = 0.0f;

    switch (ID)
    {
        // Death Mountain Crater
        case OOT_GROTTO_EXIT_SCRUBS3_DMC:
        case OOT_GROTTO_EXIT_GENERIC_DMC:
        case OOT_WARP_SONG_CRATER_ENTR:
        case OOT_DEATH_CRATER_FROM_TEMPLE_FIRE_ENTR:
        case OOT_DEATH_CRATER_FROM_FAIRY_ENTR:
        case OOT_DEATH_MOUNTAIN_CRATER_ENTR:
        case OOT_CRATER_FROM_GORON_CITY_ENTR:
        {
            ID = OOT_DEATH_MOUNTAIN_CRATER_ENTR;
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
            ID = OOT_DEATH_MOUNTAIN_FROM_KAKARIKO_ENTR;
            break;
        }

        // Goron City
        case OOT_GROTTO_EXIT_SCRUBS3_GORON_CITY:
        case OOT_GORON_CITY_FROM_SHOP_ENTR:
        case OOT_GORON_CITY_FROM_CRATER_ENTR:
        case OOT_GORON_CITY_FROM_LOST_WOODS_ENTR:
        case OOT_GORON_CITY_ENTR:
        {
            ID = OOT_GORON_CITY_ENTR;
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
            ID = OOT_SACRED_FOREST_MEADOW_ENTR;
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
            ID = OOT_LOST_WOODS_FROM_KOKIRI_FOREST_ENTR;
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
            ID = OOT_KOKIRI_FOREST_FROM_LOST_WOODS_ENTR;
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
            ID = OOT_KAKARIKO_FROM_FIELD_ENTR;
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
            ID = OOT_FIELD_FROM_MARKET_ENTRANCE_ENTR;
            break;
        }

        // Gerudo Valley
        case OOT_GROTTO_EXIT_OCTOROK:
        case OOT_GROTTO_EXIT_SCRUBS2_VALLEY:
        case OOT_VALLEY_FROM_GERUDO_FORTRESS_ENTR:
        case OOT_GERUDO_VALLEY_FROM_FIELD_ENTR:
        case OOT_GERUDO_VALLEY_FROM_TENT_ENTR:
        {
            ID = OOT_GERUDO_VALLEY_FROM_FIELD_ENTR;
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
            ID = OOT_LAKE_HYLIA_FROM_FIELD_ENTR;
            break;
        }

        // Zora Domain
        case OOT_GROTTO_EXIT_FAIRY_DOMAIN:
        case OOT_ZORA_DOMAIN_FROM_SHOP_ENTR:
        case OOT_DOMAIN_FROM_FOUNTAIN_ENTR:
        case OOT_ZORA_DOMAIN_FROM_LAKE_HYLIA_ENTR:
        case OOT_ZORA_DOMAIN_ENTR:
        {
            ID = OOT_ZORA_DOMAIN_ENTR;
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
            ID = OOT_ZORA_RIVER_FROM_FIELD_ENTR;
            break;
        }

        // Desert Colossus
        case OOT_GROTTO_EXIT_SCRUBS2_COLOSSUS:
        case OOT_DESERT_COLOSSUS_FROM_TEMPLE_SPIRIT_ENTR:
        case OOT_WARP_SONG_DESERT_ENTR:
        case OOT_DESERT_COLOSSUS_FROM_FAIRY_ENTR:
        case OOT_COLOSSUS_ENTR:
        {
            ID = OOT_COLOSSUS_ENTR;
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
            ID = OOT_GERUDO_FORTRESS_FROM_VALLEY_ENTR;
            break;
        }

        // Lon Lon's Ranch
        case OOT_GROTTO_EXIT_SCRUBS3_RANCH:
        case OOT_LON_LON_RANCH_FROM_STABLES_ENTR:
        case OOT_LON_LON_RANCH_FROM_SILO_ENTR:
        case OOT_LON_LON_RANCH_FROM_HOUSE_ENTR:
        case OOT_LON_LON_RANCH_FROM_FIELD_ENTR:
        {
            ID = OOT_LON_LON_RANCH_FROM_FIELD_ENTR;
            break;
        }
        
        // Hyrule Castle
        case OOT_GROTTO_EXIT_CASTLE:
        case OOT_CASTLE_CAUGHT_ENTR:
        case OOT_CASTLE_STEALTH_FROM_COURTYARD_ENTR:
        case OOT_HYRULE_CASTLE_FROM_FAIRY_ENTR:
        case OOT_HYRULE_CASTLE_ENTR:
        {
            ID = OOT_HYRULE_CASTLE_ENTR;
            break;
        }

        case MM_PATH_SNOWHEAD_FROM_SNOWHEAD_ENTR:
        case MM_SNOWHEAD_PATH_FROM_MOUNTAIN_VILLAGE_ENTR:
        {
            ID = MM_PATH_SNOWHEAD_FROM_SNOWHEAD_ENTR;
            break;
        }

        case MM_TERMINA_FIELD_FROM_CLOCK_TOWN_NORTH_ENTR:
        case MM_TERMINA_FIELD_FROM_CLOCK_TOWN_WEST_ENTR:
        case MM_TERMINA_FIELD_FROM_CLOCK_TOWN_EAST_ENTR:
        case MM_TERMINA_FIELD_FROM_CLOCK_TOWN_SOUTH_ENTR:
        {
            ID = MM_TERMINA_FIELD_FROM_CLOCK_TOWN_SOUTH_ENTR;
            break;
        }

        case MM_GREAT_BAY_COAST_FROM_LABORATORY_ENTR:
        case MM_GREAT_BAY_COAST_FROM_FISHER_HUT_ENTR:
        case MM_GREAT_BAY_COAST_FROM_FIELD_ENTR:
        {
            ID = MM_GREAT_BAY_COAST_FROM_FISHER_HUT_ENTR;
            break;
        }

        case MM_SWAMP_FROM_ROAD_ENTR:
        case MM_SWAMP_FROM_SPIDER_HOUSE_ENTR:
        {
            ID = MM_SWAMP_FROM_SPIDER_HOUSE_ENTR;
            break;
        }

        case MM_SWAMP_ROAD_FROM_SWAMP_ENTR:
        case MM_SWAMP_ROAD_FROM_FIELD_ENTR:
        {
            ID = MM_SWAMP_ROAD_FROM_FIELD_ENTR;
            break;
        }

        case MM_MYSTERY_WOODS_ENTR:
        case MM_WARP_OWL_MOUNTAIN_VILLAGE_ENTR:
        case MM_TWIN_ISLANDS_FROM_MOUNTAIN_VILLAGE_ENTR:
        case MM_ZORA_CAPE_FROM_GREAT_BAY_COAST_ENTR:
        case MM_IKANA_ROAD_FROM_FIELD_ENTR:
        case MM_IKANA_GRAVEYARD_FROM_DAMPE_ENTR:
        case MM_IKANA_VALLEY_FROM_ROAD_ENTR:
        {
            break;
        }

        default:
        {
            return ID;
        }
    }

    memcpy(&x, &Buffer[3], sizeof(float));
    memcpy(&y, &Buffer[4], sizeof(float));
    memcpy(&z, &Buffer[5], sizeof(float));

    const std::vector<GrottoEntrance> entrances = GrottoEntrances.at(ID);
    float tmpDist = 0.0f, bestDist = this->GetDistanceGrottoEntrance(entrances[0], x, y, z);
    uint32_t bestEntID = entrances[0].EntranceID;

    for (size_t i = 1; i < entrances.size(); i++)
    {
        tmpDist = this->GetDistanceGrottoEntrance(entrances[i], x, y, z);

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

    /*
    for (size_t i = 0; i < entrances.size(); i++)
    {
        GrottoEntrance currEntrance = entrances[i];
        if (x == currEntrance.SpawnPos[0] && y == currEntrance.SpawnPos[1] && z == currEntrance.SpawnPos[2])
        {   // The respawn coordinates match the grotto entrance

            return entrances[i].EntranceID;
        }
    }*/

    return bestEntID;
}


uint32_t EntranceHelper::CheckSpecialCase(uint8_t Game, uint32_t ID, uint32_t * SceneID)
{
    if (Game == OOT_GAME)
    {
        switch (*SceneID)
        {
            case OOT_GREAT_FAIRY_FOUNTAIN_UPGRADES:
            case OOT_GREAT_FAIRY_FOUNTAIN_SPELLS:
            {
                *SceneID = OOT_GREAT_FAIRY;
                break;
            }

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

            case OOT_TOMB_DAMPE_WINDMILL:
            {
                switch (ID)
                {
                    case OOT_KAKARIKO_FROM_WINDMILL_ENTR:
                    case OOT_WINDMILL_ENTR:
                    {
                        *SceneID = OOT_WINDMILL;
                        break;
                    }
                }
                break;
            }

            case OOT_RANCH_HOUSE_SILO:
            {
                if (ID == OOT_SILO_ENTR)
                {
                    *SceneID = OOT_SILO;
                }
                break;
            }
        }
    }
    else if (Game == MM_GAME)
    {
        switch (*SceneID)
        {
            case MM_MOUNTAIN_VILLAGE_SPRING:
            {
                *SceneID = MM_MOUNTAIN_VILLAGE;

                switch (ID)
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
            {
                *SceneID = MM_TWIN_ISLANDS;
                
                switch (ID)
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
            }

            case MM_GORON_VILLAGE_SPRING:
            case MM_GORON_VILLAGE_WINTER:
            {
                *SceneID = MM_GORON_VILLAGE;

                switch (ID)
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
                switch (ID)
                {
                    case MM_LONE_PEAK_SHRINE_ENTR:
                    {
                        *SceneID = MM_LONE_PEAK;
                        break;
                    }

                    /*case MM_GROTTO_JP_CLIMB_RIGHT_ENTR:
                    case MM_GROTTO_JP_CLIMB_LEFT_ENTR:
                    {
                        *SceneID = MM_GROTTO_DEKU_PALACE_CLIMB;
                        break;
                    }

                    case MM_GROTTO_JP_LINE_START_ENTR:
                    case MM_GROTTO_JP_LINE_END_ENTR:
                    {
                        *SceneID = MM_GROTTO_DEKU_PALACE_GENERIC;
                        * break;
                    }*/
                }

                break;
            }

            case MM_CUTSCENE_MAP:
            {
                *SceneID = MM_EXTRA;
                break;
            }

            case MM_ZORA_HALL_ROOMS:
            {
                switch (ID)
                {
                    case MM_ROOM_EVANS_ENTR:
                    {
                        *SceneID = MM_ZORA_EVANS_ROOM;
                        break;
                    }

                    case MM_ROOM_JAPAS_ENTR:
                    {
                        *SceneID = MM_ZORA_JAPAS_ROOM;
                        break;
                    }

                    case MM_ROOM_TIJO_ENTR:
                    {
                        *SceneID = MM_ZORA_TIJO_ROOM;
                        break;
                    }

                    case MM_ROOM_LULU_ENTR:
                    {
                        *SceneID = MM_ZORA_LULU_ROOM;
                        break;
                    }

                    case MM_ZORA_SHOP_ENTR:
                    {
                        *SceneID = MM_ZORA_SHOP;
                        break;
                    }
                }
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
        switch (*SceneID)
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

            case MM_LAIR_GOHT:
            {
                return MM_BOSS_TEMPLE_SNOWHEAD_WARP_OUT;
            }

            case MM_CLOCK_TOWN_SOUTH:
            {
                *SceneID = WARP_SCENE;
                return ID;
            }
        }

        /*float tmpDist = 0.0f, bestDist = this->GetDistanceGrottoEntrance(SpecialWarps[0], x, y, z);
        ID = SpecialWarps[0].EntranceID;

        for (size_t i = 1; i < SpecialWarps.size(); i++)
        {
            tmpDist = this->GetDistanceGrottoEntrance(SpecialWarps[i], x, y, z);

            if (tmpDist == 0.0)
            {
                ID = SpecialWarps[i].EntranceID;
                break;
            }
            else if (tmpDist < bestDist)
            {
                bestDist = tmpDist;
                ID = SpecialWarps[i].EntranceID;
            }
        }

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

            case MM_BOSS_TEMPLE_SNOWHEAD_WARP_OUT:
            {
                *SceneID = MM_LAIR_GOHT;
                break;
            }
        }*/
        /*
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

                    case MM_BOSS_TEMPLE_SNOWHEAD_WARP_OUT:
                    {
                        *SceneID = MM_LAIR_GOHT;
                        break;
                    }
                }
            }
        }*/
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
    if (this->IsEntranceTouched)
    {
        if (this->IsNewCycle(Buffer) || this->IsSongOfDoubleTime(Buffer[1], Buffer[2]) || this->IsSongOfTime(Buffer) || this->IsSunSong(Buffer[1], Buffer[2]))
        {   
            this->IsEntranceTouched = false;
            return;
        }

        uint8_t game = Buffer[0] & 0xFF;
        uint8_t currRoom = (Buffer[0] >> 16) & 0xFF;
        uint8_t grottoData = (Buffer[0] >> 8) & 0xFF;
        uint32_t inScene = Buffer[1];
        uint32_t inEntrance = Buffer[2];
        EntranceMetaInfo entranceMeta = {};

        inEntrance = this->CheckSpecialCase(game, inEntrance, &inScene);

        if (this->IsGrottoEntrance(inEntrance))
        {
            inEntrance = this->GetGrottoEntrance(game, grottoData, inEntrance, this->OutScene);
            inScene = this->CorrectGrottoScene(game, inEntrance);
        }
        else if (this->IsGrottoExit(inEntrance))
        {
            inEntrance = this->GetGrottoExit(game, currRoom, grottoData, inScene);
        }
        else
        {
            inEntrance = this->CheckGrottoSpawn(inEntrance, Buffer);
        }

        if (game == OOT_GAME)
        {
            entranceMeta = OoTEntrances.at(inEntrance);
        }
        else
        {
            entranceMeta = MMEntrances.at(inEntrance);
        }

        this->EntranceStr = entranceMeta.ToName + std::string(" - ") + entranceMeta.FromName;
        float x, y, z;
        memcpy(&x, &Buffer[3], sizeof(float));
        memcpy(&y, &Buffer[4], sizeof(float));
        memcpy(&z, &Buffer[5], sizeof(float));
        MultiLogger::LogMessage("X = %f, Y = %f, Z = %f", x, y, z);

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
    this->OutBuffer = Buffer;
    if (this->IsMMExtra(Buffer))
    //if (Buffer[1] == WARP_SCENE && Buffer[2] != WARP_LOADING)
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

    // Check that the entrance is not a special case
    this->OutEntrance = this->CheckSpecialCase(this->OutGame, this->OutEntrance, &this->OutScene);

    if (this->IsGrottoEntrance(this->OutEntrance))
    {   // The current entrance is a grotto entrance

        this->OutEntrance = this->GetGrottoEntrance(this->OutGame, grottoData, this->OutEntrance, this->OutScene);
        //this->OutScene = this->CorrectGrottoScene(this->OutGame, this->OutEntrance);
    }
    else if (this->IsGrottoExit(this->OutEntrance))
    {   // The current entrance is a grotto exit

        this->OutEntrance = this->GetGrottoExit(this->OutGame, currRoom, grottoData, this->OutScene);
    }
    else if (this->IsWarpEntrance(this->OutEntrance))
    {   // The current entrance is a warp zone

        bool isWarpSong = false;
        this->OutEntrance = this->GetWarpSong(&this->OutGame, this->OutEntrance, songIndex, OwlID, &isWarpSong);
        
        if (!isWarpSong)
        {
            this->OutEntrance = this->CheckWrapScene(this->OutGame, this->OutEntrance, &this->OutScene, Buffer[3], Buffer[4], Buffer[5]);

            if (this->OutScene == WARP_SCENE)
            {   // We don't want to catch this

                this->IsEntranceTouched = false;
                return;
            }
        }
    }

    // Retreive the entrance meta information
    if (this->OutGame == OOT_GAME)
    {
        this->OutMetaInf = &OoTEntrances.at(this->OutEntrance);
    }
    else
    {
        this->OutMetaInf = &MMEntrances.at(this->OutEntrance);
    }
    this->LastTouchedStr = OutMetaInf->FromName + std::string(" -> ") + OutMetaInf->ToName;
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