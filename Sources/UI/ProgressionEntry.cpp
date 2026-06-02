#include "UI/ProgressionEntry.h"
#include "Combo/Items.h"

// Helper to compute the static count of an entries array at compile-time.
#define MAKE_SECTION(Title, Arr) { Title, Arr, sizeof(Arr) / sizeof(Arr[0]) }

#pragma region OoT Page

const ProgEntry OoTQuestStatus[] =
{
    // Stones / Medallions
    { EGameIcon::emerald,   "Kokiri's Emerald",  { OOT_STONE_EMERALD },          false },
    { EGameIcon::ruby,      "Goron's Ruby",      { OOT_STONE_RUBY },             false },
    { EGameIcon::sapphire,  "Zora's Sapphire",   { OOT_STONE_SAPPHIRE },         false },
    { EGameIcon::forest,    "Forest Medallion",  { OOT_MEDALLION_FOREST }, false },
    { EGameIcon::fire,      "Fire Medallion",    { OOT_MEDALLION_FIRE },   false },
    { EGameIcon::water,     "Water Medallion",   { OOT_MEDALLION_WATER },  false },
    { EGameIcon::shadow,    "Shadow Medallion",  { OOT_MEDALLION_SHADOW }, false },
    { EGameIcon::spirit,    "Spirit Medallion",  { OOT_MEDALLION_SPIRIT }, false },
    { EGameIcon::light,     "Light Medallion",   { OOT_MEDALLION_LIGHT },  false },

    // Heart / Magic
    { EGameIcon::heart_container, "Heart Container", { OOT_HEART_CONTAINER, OOT_HEART_CONTAINER2, SHARED_HEART_CONTAINER }, true },
    { EGameIcon::heart_piece, "Piece of Heart", { OOT_HEART_PIECE, OOT_TC_HEART_PIECE, SHARED_HEART_PIECE }, true },
    { EGameIcon::defense,            "Double Defense",   { OOT_DEFENSE_UPGRADE, SHARED_DEFENSE_UPGRADE },   false },
    { EGameIcon::magic_upgrade,      "Magic Upgrade",    { OOT_MAGIC_UPGRADE, SHARED_MAGIC_UPGRADE },    false },
    { EGameIcon::large_magic_upgrade,"Double Magic",     { OOT_MAGIC_UPGRADE2, SHARED_MAGIC_UPGRADE },     false },

    // Buttons
    { EGameIcon::a_button, "A Button", { OOT_BUTTON_A, SHARED_BUTTON_A }, false },
    { EGameIcon::c_down, "C-Down Button", { OOT_BUTTON_C_DOWN, SHARED_BUTTON_C_DOWN }, false },
    { EGameIcon::c_left, "C-Left Button", { OOT_BUTTON_C_LEFT, SHARED_BUTTON_C_LEFT }, false },
    { EGameIcon::c_right, "C-Right Button", { OOT_BUTTON_C_RIGHT, SHARED_BUTTON_C_RIGHT }, false },
    { EGameIcon::c_up, "C-Up Button", { OOT_BUTTON_C_UP, SHARED_BUTTON_C_UP }, false },

    // Songs
    { EGameIcon::song,        "Zelda's Lullaby",     { OOT_SONG_ZELDA, OOT_SONG_NOTE_ZELDA, SHARED_SONG_NOTE_ZELDA, SHARED_SONG_ZELDA },         true },
    { EGameIcon::song,        "Epona's Song",        { OOT_SONG_EPONA, OOT_SONG_NOTE_EPONA, SHARED_SONG_EPONA, SHARED_SONG_NOTE_EPONA },           true },
    { EGameIcon::song,        "Saria's Song",        { OOT_SONG_SARIA, OOT_SONG_NOTE_SARIA, SHARED_SONG_NOTE_SARIA, SHARED_SONG_SARIA },           true },
    { EGameIcon::song,        "Sun's Song",          { OOT_SONG_SUN, OOT_SONG_NOTE_SUN, SHARED_SONG_SUN, SHARED_SONG_NOTE_SUN },      true },
    { EGameIcon::song,        "Song of Time",        { OOT_SONG_TIME, OOT_SONG_NOTE_TIME, SHARED_SONG_TIME, SHARED_SONG_NOTE_TIME },    true },
    { EGameIcon::song,        "Song of Storms",      { OOT_SONG_STORMS, OOT_SONG_NOTE_STORMS, SHARED_SONG_STORMS, SHARED_SONG_NOTE_STORMS },  true },
    { EGameIcon::song_green,  "Minuet of Forest",    { OOT_SONG_TP_FOREST, OOT_SONG_NOTE_TP_FOREST, SHARED_SONG_NOTE_TP_FOREST, SHARED_SONG_TP_FOREST },          true },
    { EGameIcon::song_red,    "Bolero of Fire",      { OOT_SONG_TP_FIRE, OOT_SONG_NOTE_TP_FIRE, SHARED_SONG_NOTE_TP_FIRE, SHARED_SONG_TP_FIRE },          true },
    { EGameIcon::song_blue,   "Serenade of Water",   { OOT_SONG_TP_WATER, OOT_SONG_NOTE_TP_FIRE, SHARED_SONG_NOTE_TP_WATER, SHARED_SONG_TP_WATER },        true },
    { EGameIcon::song_purple, "Nocturne of Shadow",  { OOT_SONG_TP_SHADOW, OOT_SONG_NOTE_TP_FIRE, SHARED_SONG_NOTE_TP_SHADOW, SHARED_SONG_TP_SHADOW },        true },
    { EGameIcon::song_orange, "Requiem of Spirit",   { OOT_SONG_TP_SPIRIT, OOT_SONG_NOTE_TP_FIRE, SHARED_SONG_NOTE_TP_SPIRIT, SHARED_SONG_TP_SPIRIT },         true },
    { EGameIcon::song_yellow, "Prelude of Light",    { OOT_SONG_TP_LIGHT, OOT_SONG_NOTE_TP_FIRE, SHARED_SONG_NOTE_TP_LIGHT, SHARED_SONG_TP_LIGHT },         true },
    { EGameIcon::song,        "Elegy of Emptiness",  { OOT_SONG_EMPTINESS, OOT_SONG_NOTE_EMPTINESS, SHARED_SONG_EMPTINESS, SHARED_SONG_NOTE_EMPTINESS },  true },
    { EGameIcon::song,        "Song of Healing",     { OOT_SONG_HEALING, OOT_SONG_NOTE_HEALING, SHARED_SONG_NOTE_HEALING, SHARED_SONG_HEALING }, true },
    { EGameIcon::song,        "Song of Soaring",     { OOT_SONG_SOARING, OOT_SONG_NOTE_SOARING, SHARED_SONG_NOTE_SOARING, SHARED_SONG_SOARING }, true },
    { EGameIcon::song_green,  "Sonata of Awakening", { OOT_SONG_AWAKENING, OOT_SONG_NOTE_AWAKENING, SHARED_SONG_NOTE_AWAKENING, SHARED_SONG_AWAKENING },          true },
    { EGameIcon::song_red,    "Goron Lullaby",       { OOT_SONG_GORON_HALF, OOT_SONG_GORON, OOT_SONG_NOTE_GORON, SHARED_SONG_NOTE_GORON, SHARED_SONG_GORON, SHARED_SONG_GORON_HALF },   true },
    { EGameIcon::song_blue,   "New Wave Bossa Nova", { OOT_SONG_ZORA, OOT_SONG_NOTE_ZORA, SHARED_SONG_NOTE_ZORA, SHARED_SONG_ZORA },      true },
    { EGameIcon::song_purple, "Oath to Order",       { OOT_SONG_ORDER, OOT_SONG_NOTE_ORDER, SHARED_SONG_NOTE_ORDER, SHARED_SONG_ORDER },   true },

    // Others
    { EGameIcon::gs_token,       "Gold Skulltula Token", { OOT_GS_TOKEN }, true },
    { EGameIcon::platinum_token, "Platinum Token", { OOT_PLATINUM_TOKEN, SHARED_PLATINUM_TOKEN }, false },
    { EGameIcon::agony, "Stone of Agony", { OOT_STONE_OF_AGONY, SHARED_STONE_OF_AGONY }, false },
    { EGameIcon::card, "Gerudo's Membership Card", { OOT_GERUDO_CARD }, false },
    { EGameIcon::triforce_piece, "Triforce Piece",       { OOT_TRIFORCE, OOT_TRIFORCE_FULL, SHARED_TRIFORCE }, true },
    { EGameIcon::triforce_courage, "Triforce of Courage",       { OOT_TRIFORCE_COURAGE, SHARED_TRIFORCE_COURAGE }, false },
    { EGameIcon::triforce_power,   "Triforce of Power",       { OOT_TRIFORCE_POWER, SHARED_TRIFORCE_POWER }, false },
    { EGameIcon::triforce_wisdom,  "Triforce of Wisdom",       { OOT_TRIFORCE_WISDOM, SHARED_TRIFORCE_WISDOM }, false },
};

const ProgEntry OoTDungeonsItems[] =
{
    // Map
    { EGameIcon::map, "Map (Deku Tree)", { OOT_MAP_DT, OOT_MAP }, false },
    { EGameIcon::map, "Map (Dodongo's Cavern)", { OOT_MAP_DC, OOT_MAP }, false },
    { EGameIcon::map, "Map (Jabu-Jabu's Belly)", { OOT_MAP_JJ, OOT_MAP }, false },
    { EGameIcon::map, "Map (Forest Temple)", { OOT_MAP_FOREST, OOT_MAP }, false },
    { EGameIcon::map, "Map (Fire Temple)", { OOT_MAP_FIRE, OOT_MAP }, false },
    { EGameIcon::map, "Map (Water Temple)", { OOT_MAP_WATER, OOT_MAP }, false },
    { EGameIcon::map, "Map (Shadow Temple)", { OOT_MAP_SHADOW, OOT_MAP }, false },
    { EGameIcon::map, "Map (Spirit Temple)", { OOT_MAP_SPIRIT, OOT_MAP }, false },
    { EGameIcon::map, "Map (Ice Cavern)", { OOT_MAP_IC, OOT_MAP }, false },
    { EGameIcon::map, "Map (Bottom of the Well)", { OOT_MAP_BOTW, OOT_MAP }, false },

    // Compass
    { EGameIcon::compass, "Compass (Deku Tree)", { OOT_COMPASS_DT, OOT_COMPASS }, false },
    { EGameIcon::compass, "Compass (Dodongo's Cavern)", { OOT_COMPASS_DC, OOT_COMPASS }, false },
    { EGameIcon::compass, "Compass (Jabu-Jabu's Belly)", { OOT_COMPASS_JJ, OOT_COMPASS }, false },
    { EGameIcon::compass, "Compass (Fire Temple)", { OOT_COMPASS_FIRE, OOT_COMPASS }, false },
    { EGameIcon::compass, "Compass (Forest Temple)", { OOT_COMPASS_FOREST, OOT_COMPASS }, false },
    { EGameIcon::compass, "Compass (Water Temple)", { OOT_COMPASS_WATER, OOT_COMPASS }, false },
    { EGameIcon::compass, "Compass (Shadow Temple)", { OOT_COMPASS_SHADOW, OOT_COMPASS }, false },
    { EGameIcon::compass, "Compass (Spirit Temple)", { OOT_COMPASS_SPIRIT, OOT_COMPASS }, false },
    { EGameIcon::compass, "Compass (Ice Cavern)", { OOT_COMPASS_IC, OOT_COMPASS }, false },
    { EGameIcon::compass, "Compass (Bottom of the Well)", { OOT_COMPASS_BOTW, OOT_COMPASS }, false },

    // Boss key
    { EGameIcon::boss_key,       "Boss Key (Forest Temple)",             { OOT_BOSS_KEY_FOREST, OOT_BOSS_KEY },       false },
    { EGameIcon::boss_key,       "Boss Key (Fire Temple)",             { OOT_BOSS_KEY_FIRE, OOT_BOSS_KEY },       false },
    { EGameIcon::boss_key,       "Boss Key (Water Temple)",             { OOT_BOSS_KEY_WATER, OOT_BOSS_KEY },       false },
    { EGameIcon::boss_key,       "Boss Key (Shadow Temple)",             { OOT_BOSS_KEY_SHADOW, OOT_BOSS_KEY },       false },
    { EGameIcon::boss_key,       "Boss Key (Spirit Temple)",             { OOT_BOSS_KEY_SPIRIT, OOT_BOSS_KEY },       false },
    { EGameIcon::boss_key,       "Boss Key (Ganon 's Castle)",             { OOT_BOSS_KEY_GANON, OOT_BOSS_KEY },       false },

    // Small Keys / Key Rings
    { EGameIcon::small_key,      "Small Key (Forest Temple)",            { OOT_SMALL_KEY_FOREST },      true },
    { EGameIcon::small_key,      "Small Key (Fire Temple)",            { OOT_SMALL_KEY_FIRE },      true },
    { EGameIcon::small_key,      "Small Key (Water Temple)",            { OOT_SMALL_KEY_WATER },      true },
    { EGameIcon::small_key,      "Small Key (Shadow Temple)",            { OOT_SMALL_KEY_SHADOW },      true },
    { EGameIcon::small_key,      "Small Key (Spirit Temple)",            { OOT_SMALL_KEY_SPIRIT },      true },
    { EGameIcon::small_key,      "Small Key (Bottom of the Well)",            { OOT_SMALL_KEY_BOTW },      true },
    { EGameIcon::small_key,      "Small Key (Gerudo's Training Ground)",            { OOT_SMALL_KEY_GTG },      true },
    { EGameIcon::small_key,      "Small Key (Gerudo's Fortress)",            { OOT_SMALL_KEY_GF },      true },
    { EGameIcon::small_key,      "Small Key (Ganon's Castle)",            { OOT_SMALL_KEY_GANON },      true },
    { EGameIcon::small_key,      "Small Key (Chest Game)",            { OOT_SMALL_KEY_TCG },      true },
    { EGameIcon::key_ring, "Key Ring (Forest)", { OOT_KEY_RING_FOREST, OOT_KEY_RING }, false },
    { EGameIcon::key_ring, "Key Ring (Fire)", { OOT_KEY_RING_FIRE, OOT_KEY_RING }, false },
    { EGameIcon::key_ring, "Key Ring (Shadow)", { OOT_KEY_RING_SHADOW, OOT_KEY_RING }, false },
    { EGameIcon::key_ring, "Key Ring (Spirit)", { OOT_KEY_RING_SPIRIT, OOT_KEY_RING }, false },
    { EGameIcon::key_ring, "Key Ring (Water)", { OOT_KEY_RING_WATER, OOT_KEY_RING }, false },
    { EGameIcon::key_ring, "Key Ring (Well)", { OOT_KEY_RING_BOTW, OOT_KEY_RING }, false },
    { EGameIcon::key_ring, "Key Ring (Hideout)", { OOT_KEY_RING_GF, OOT_KEY_RING }, false },
    { EGameIcon::key_ring, "Key Ring (GTG)", { OOT_KEY_RING_GTG, OOT_KEY_RING }, false },
    { EGameIcon::key_ring, "Key Ring (Ganon)", { OOT_KEY_RING_GANON, OOT_KEY_RING }, false },
    { EGameIcon::key_ring, "Key Ring (Chest Game)", { OOT_KEY_RING_TCG, OOT_KEY_RING }, false },
    { EGameIcon::skeleton_key, "Skeleton Key", { OOT_SKELETON_KEY, SHARED_SKELETON_KEY }, false },

    // Silver rupees / Silver pouches
    { EGameIcon::sr, "Silver Rupee (Dodongo's Cavern)", { OOT_RUPEE_SILVER_DC }, true },
    { EGameIcon::sr, "Silver Rupee (Shadow Temple - Invisible Blades)", { OOT_RUPEE_SILVER_SHADOW_BLADES }, true },
    { EGameIcon::sr, "Silver Rupee (Shadow Temple - Large Pits)", { OOT_RUPEE_SILVER_SHADOW_PIT }, true },
    { EGameIcon::sr, "Silver Rupee (Shadow Temple - Scythe)", { OOT_RUPEE_SILVER_SHADOW_SCYTHE }, true },
    { EGameIcon::sr, "Silver Rupee (Shadow Temple - Spikes)", { OOT_RUPEE_SILVER_SHADOW_SPIKES }, true },
    { EGameIcon::sr, "Silver Rupee (Spirit Temple - Adult)", { OOT_RUPEE_SILVER_SPIRIT_ADULT }, true },
    { EGameIcon::sr, "Silver Rupee (Spirit Temple - Boulders)", { OOT_RUPEE_SILVER_SPIRIT_BOULDERS }, true },
    { EGameIcon::sr, "Silver Rupee (Spirit Temple - Child)", { OOT_RUPEE_SILVER_SPIRIT_CHILD }, true },
    { EGameIcon::sr, "Silver Rupee (Spirit Temple - Lobby)", { OOT_RUPEE_SILVER_SPIRIT_LOBBY }, true },
    { EGameIcon::sr, "Silver Rupee (Spirit Temple - Sun Block)", { OOT_RUPEE_SILVER_SPIRIT_SUN }, true },
    { EGameIcon::sr, "Silver Rupee (Ice Cavern - Block)", { OOT_RUPEE_SILVER_IC_BLOCK }, true },
    { EGameIcon::sr, "Silver Rupee (Ice Cavern - Scythe)", { OOT_RUPEE_SILVER_IC_SCYTHE }, true },
    { EGameIcon::sr, "Silver Rupee (Bottom of the Well)", { OOT_RUPEE_SILVER_BOTW }, true },
    { EGameIcon::sr, "Silver Rupee (Gerudo Training Ground - Lava)", { OOT_RUPEE_SILVER_GTG_LAVA }, true },
    { EGameIcon::sr, "Silver Rupee (Gerudo Training Ground - Slopes)", { OOT_RUPEE_SILVER_GTG_SLOPES }, true },
    { EGameIcon::sr, "Silver Rupee (Gerudo Training Ground - Water)", { OOT_RUPEE_SILVER_GTG_WATER }, true },
    { EGameIcon::sr, "Silver Rupee (Ganon's Castle - Forest Trial)", { OOT_RUPEE_SILVER_GANON_FOREST }, true },
    { EGameIcon::sr, "Silver Rupee (Ganon's Castle - Fire Trial)", { OOT_RUPEE_SILVER_GANON_FIRE }, true },
    { EGameIcon::sr, "Silver Rupee (Ganon's Castle - Light Trial)", { OOT_RUPEE_SILVER_GANON_LIGHT }, true },
    { EGameIcon::sr, "Silver Rupee (Ganon's Castle - Water Trial)", { OOT_RUPEE_SILVER_GANON_WATER }, true },
    { EGameIcon::sr, "Silver Rupee (Ganon's Castle - Shadow Trial)", { OOT_RUPEE_SILVER_GANON_SHADOW }, true },
    { EGameIcon::sr, "Silver Rupee (Ganon's Castle - Spirit Trial)", { OOT_RUPEE_SILVER_GANON_SPIRIT }, true },
    { EGameIcon::magical_rupee, "Magical Silver Rupee", { OOT_RUPEE_MAGICAL }, false },
    { EGameIcon::silver_pouch, "Silver Rupee Pouch (Dodongo's Cavern)", { OOT_POUCH_SILVER_DC }, false },
    { EGameIcon::silver_pouch, "Silver Rupee Pouch (Ice Cavern - Block)", { OOT_POUCH_SILVER_IC_BLOCK }, false },
    { EGameIcon::silver_pouch, "Silver Rupee Pouch (Ice Cavern - Scythe)", { OOT_POUCH_SILVER_IC_SCYTHE }, false },
    { EGameIcon::silver_pouch, "Silver Rupee Pouch (Bottom of the Well)", { OOT_POUCH_SILVER_BOTW }, false },
    { EGameIcon::silver_pouch, "Silver Rupee Pouch (Shadow Temple - Invisible Blades)", { OOT_POUCH_SILVER_SHADOW_BLADES }, false },
    { EGameIcon::silver_pouch, "Silver Rupee Pouch (Shadow Temple - Large Pits)", { OOT_POUCH_SILVER_SHADOW_PIT }, false },
    { EGameIcon::silver_pouch, "Silver Rupee Pouch (Shadow Temple - Scythe)", { OOT_POUCH_SILVER_SHADOW_SCYTHE }, false },
    { EGameIcon::silver_pouch, "Silver Rupee Pouch (Shadow Temple - Spikes)", { OOT_POUCH_SILVER_SHADOW_SPIKES }, false },
    { EGameIcon::silver_pouch, "Silver Rupee Pouch (Spirit Temple - Adult)", { OOT_POUCH_SILVER_SPIRIT_ADULT }, false },
    { EGameIcon::silver_pouch, "Silver Rupee Pouch (Spirit Temple - Boulders)", { OOT_POUCH_SILVER_SPIRIT_BOULDERS }, false },
    { EGameIcon::silver_pouch, "Silver Rupee Pouch (Spirit Temple - Child)", { OOT_POUCH_SILVER_SPIRIT_CHILD }, false },
    { EGameIcon::silver_pouch, "Silver Rupee Pouch (Spirit Temple - Lobby)", { OOT_POUCH_SILVER_SPIRIT_LOBBY }, false },
    { EGameIcon::silver_pouch, "Silver Rupee Pouch (Spirit Temple - Sun Block)", { OOT_POUCH_SILVER_SPIRIT_SUN }, false },
    { EGameIcon::silver_pouch, "Silver Rupee Pouch (Gerudo Training Ground - Lava)", { OOT_POUCH_SILVER_GTG_LAVA }, false },
    { EGameIcon::silver_pouch, "Silver Rupee Pouch (Gerudo Training Ground - Slopes)", { OOT_POUCH_SILVER_GTG_SLOPES }, false },
    { EGameIcon::silver_pouch, "Silver Rupee Pouch (Gerudo Training Ground - Water)", { OOT_POUCH_SILVER_GTG_WATER }, false },
    { EGameIcon::silver_pouch, "Silver Rupee Pouch (Ganon's Castle - Forest Trial)", { OOT_POUCH_SILVER_GANON_FOREST }, false },
    { EGameIcon::silver_pouch, "Silver Rupee Pouch (Ganon's Castle - Fire Trial)", { OOT_POUCH_SILVER_GANON_FIRE }, false },
    { EGameIcon::silver_pouch, "Silver Rupee Pouch (Ganon's Castle - Light Trial)", { OOT_POUCH_SILVER_GANON_LIGHT }, false },
    { EGameIcon::silver_pouch, "Silver Rupee Pouch (Ganon's Castle - Water Trial)", { OOT_POUCH_SILVER_GANON_WATER }, false },
    { EGameIcon::silver_pouch, "Silver Rupee Pouch (Ganon's Castle - Shadow Trial)", { OOT_POUCH_SILVER_GANON_SHADOW }, false },
    { EGameIcon::silver_pouch, "Silver Rupee Pouch (Ganon's Castle - Spirit Trial)", { OOT_POUCH_SILVER_GANON_SPIRIT }, false },
};

const ProgEntry OoTEquipments[] =
{
    // MM stuff
    { EGameIcon::spin, "Spin Attack Upgrade", { OOT_SPIN_UPGRADE, SHARED_SPIN_UPGRADE }, false },

    // Swords
    { EGameIcon::kokiri,        "Kokiri Sword",         { OOT_SWORD_KOKIRI, OOT_SWORD, SHARED_SWORD },     false},
    { EGameIcon::razor, "Razor Sword", { OOT_SWORD_RAZOR, OOT_SWORD, SHARED_SWORD }, false },
    { EGameIcon::master,        "Master Sword",         { OOT_SWORD_MASTER, OOT_SWORD, SHARED_SWORD },     false },
    { EGameIcon::biggoron, "Giant's Knife", { OOT_SWORD_KNIFE, OOT_SWORD_GORON, OOT_SWORD, SHARED_SWORD }, false },
    { EGameIcon::biggoron,      "Biggoron's Sword",     { OOT_SWORD_BIGGORON, OOT_SWORD_GORON, OOT_SWORD, SHARED_SWORD },         false },
    { EGameIcon::gilded, "Gilded Sword", { OOT_SWORD_GILDED, OOT_SWORD, SHARED_SWORD }, false },

    // Shields
    { EGameIcon::deku_shield,        "Deku Shield",      { OOT_SHIELD_DEKU, OOT_PROGRESSIVE_SHIELD_DEKU, OOT_SHIELD, SHARED_SHIELD, SHARED_SHIELD_DEKU },      false},
    { EGameIcon::hylian_shield,      "Hylian Shield",    { OOT_SHIELD_HYLIAN, OOT_PROGRESSIVE_SHIELD_HYLIAN, OOT_SHIELD, SHARED_SHIELD, SHARED_SHIELD_HYLIAN },    false },
    { EGameIcon::mirror_oot,         "Mirror Shield",    { OOT_SHIELD_MIRROR, OOT_SHIELD, SHARED_SHIELD, SHARED_SHIELD_MIRROR },    false },

    // Tunic
    { EGameIcon::goron_tunic,        "Goron Tunic",      { OOT_TUNIC_GORON, SHARED_TUNIC_GORON },      false },
    { EGameIcon::zora_tunic,         "Zora Tunic",       { OOT_TUNIC_ZORA, SHARED_TUNIC_ZORA },       false },

    // Boots
    { EGameIcon::iron,               "Iron Boots",       { OOT_BOOTS_IRON, SHARED_BOOTS_IRON },       false },
    { EGameIcon::hover,              "Hover Boots",      { OOT_BOOTS_HOVER, SHARED_BOOTS_HOVER },      false },

    // Gauntlets
    { EGameIcon::bracelet, "Goron's Bracelet", { OOT_GORON_BRACELET, OOT_STRENGTH, SHARED_STRENGTH }, false },
    { EGameIcon::silver_gauntlet,    "Silver Gauntlets", { OOT_SILVER_GAUNTLETS, OOT_STRENGTH, SHARED_STRENGTH },  false },
    { EGameIcon::golden_gauntlet,    "Golden Gauntlets", { OOT_GOLDEN_GAUNTLETS, OOT_STRENGTH, SHARED_STRENGTH },  false },

    // Scales
    { EGameIcon::bronze_scale,       "Bronze Scale",     { OOT_SCALE_BRONZE, OOT_SCALE, SHARED_SCALE },     false },
    { EGameIcon::silver,             "Silver Scale",     { OOT_SCALE_SILVER, OOT_SCALE, SHARED_SCALE },     false },
    { EGameIcon::golden,             "Golden Scale",     { OOT_SCALE_GOLDEN, OOT_SCALE, SHARED_SCALE },     false },

    // Quiver / Bags
    { EGameIcon::big_seed, "Bullet Bag", { OOT_BULLET_BAG }, false },
    { EGameIcon::biggest_seed, "Large Bullet Bag", { OOT_BULLET_BAG2 }, false },
    { EGameIcon::biggest_seed, "Largest Bullet Bag", { OOT_BULLET_BAG3 }, false },
    { EGameIcon::big_quiver,         "Big Quiver",       { OOT_QUIVER2 },       false },
    { EGameIcon::biggest_quiver,     "Biggest Quiver",   { OOT_QUIVER3 },   false },
    { EGameIcon::bomb_bag,           "Bomb Bag",         { OOT_BOMB_BAG, SHARED_BOMB_BAG },         false },
    { EGameIcon::big_bomb,           "Big Bomb Bag",     { OOT_BOMB_BAG2, SHARED_BOMB_BAG },         false },
    { EGameIcon::biggest_bomb,       "Biggest Bomb Bag", { OOT_BOMB_BAG3, SHARED_BOMB_BAG },     false },
    { EGameIcon::bombchu_bag, "Bombchu Bag", { OOT_BOMBCHU_BAG, OOT_BOMBCHU_BAG_FIRST_5, OOT_BOMBCHU_BAG_FIRST_10, OOT_BOMBCHU_BAG_FIRST_20, SHARED_BOMBCHU_BAG }, false },
    { EGameIcon::bombchu_bag, "Big Bombchu Bag", { OOT_BOMBCHU_BAG2, SHARED_BOMBCHU_BAG }, false },
    { EGameIcon::bombchu_bag, "Biggest Bombchu Bag", { OOT_BOMBCHU_BAG3, SHARED_BOMBCHU_BAG }, false },
    { EGameIcon::nut_upgrade_1, "Deku Nut Upgrade", { OOT_NUT_UPGRADE, SHARED_NUT_UPGRADE }, false },
    { EGameIcon::nut_upgrade_2, "Second Deku Nut Upgrade", { OOT_NUT_UPGRADE2, SHARED_NUT_UPGRADE }, false },
    { EGameIcon::stick_upgrade_1, "Deku Stick Upgrade", { OOT_STICK_UPGRADE, SHARED_STICK_UPGRADE }, false },
    { EGameIcon::stick_upgrade_2, "Second Deku Stick Upgrade", { OOT_STICK_UPGRADE2, SHARED_STICK_UPGRADE }, false },

    // Wallets
    { EGameIcon::child_wallet, "Child's Wallet", { OOT_WALLET, SHARED_WALLET }, false },
    { EGameIcon::wallet, "Adult's Wallet", { OOT_WALLET2, SHARED_WALLET }, false },
    { EGameIcon::big_wallet, "Giant's Wallet", { OOT_WALLET3, SHARED_WALLET }, false },
    { EGameIcon::colossal_wallet, "Colossal Wallet", { OOT_WALLET4, SHARED_WALLET }, false },
    { EGameIcon::bottomless_wallet, "Bottomless Wallet", { OOT_WALLET5, SHARED_WALLET }, false },

};

const ProgEntry OoTMajorItems[] =
{
    { EGameIcon::bow_oot,       "Fairy Bow",            { OOT_BOW, SHARED_BOW },        false },
    { EGameIcon::fire_arrow,    "Fire Arrows",          { OOT_ARROW_FIRE, SHARED_ARROW_FIRE },       false },
    { EGameIcon::ice_arrow,     "Ice Arrows",           { OOT_ARROW_ICE, SHARED_ARROW_ICE },        false },
    { EGameIcon::light_arrow,   "Light Arrows",         { OOT_ARROW_LIGHT, SHARED_ARROW_LIGHT },      false },
    { EGameIcon::slingshot,     "Fairy Slingshot",      { OOT_SLINGSHOT },        false },
    { EGameIcon::boomerang,      "Boomerang",            { OOT_BOOMERANG, SHARED_BOOMERANG },        false },
    { EGameIcon::hookshot_oot,  "Hookshot",             { OOT_HOOKSHOT, SHARED_HOOKSHOT },         false },
    { EGameIcon::longshot,      "Longshot",             { OOT_LONGSHOT, OOT_HOOKSHOT, SHARED_HOOKSHOT },         false },
    { EGameIcon::hammer,        "Megaton Hammer",       { OOT_HAMMER, SHARED_HAMMER },           false },
    { EGameIcon::lens,          "Lens of Truth",        { OOT_LENS, SHARED_LENS },             false },
    { EGameIcon::bean,          "Magic Beans",          { OOT_MAGIC_BEAN },       false },
    { EGameIcon::powder,        "Powder Keg",           { OOT_POWDER_KEG, SHARED_POWDER_KEG }, false },
    { EGameIcon::fairy_sword, "Great Fairy's Sword",    { OOT_GREAT_FAIRY_SWORD, SHARED_GREAT_FAIRY_SWORD }, false },
    { EGameIcon::fairy_ocarina, "Fairy Ocarina",        { OOT_OCARINA_FAIRY, OOT_OCARINA, SHARED_OCARINA }, false },
    { EGameIcon::ocarina,       "Ocarina of Time",      { OOT_OCARINA_TIME, OOT_OCARINA, SHARED_OCARINA }, false },
    { EGameIcon::din,           "Din's Fire",           { OOT_SPELL_FIRE, SHARED_SPELL_FIRE },              false },
    { EGameIcon::farore,        "Farore's Wind",        { OOT_SPELL_WIND, SHARED_SPELL_WIND },           false },
    { EGameIcon::nayru,         "Nayru's Love",         { OOT_SPELL_LOVE, SHARED_SPELL_LOVE },            false },
    { EGameIcon::bottle,        "Empty Bottle",         { OOT_BOTTLE_EMPTY, MM_BOTTLE_RUTO_LETTER, OOT_BOTTLE_BIG_POE, OOT_BOTTLE_BLUE_FIRE, OOT_BOTTLE_CHATEAU, OOT_BOTTLE_FAIRY, OOT_BOTTLE_MILK, OOT_BOTTLE_POE, OOT_BOTTLE_POTION_BLUE, OOT_BOTTLE_POTION_GREEN, OOT_BOTTLE_POTION_RED, OOT_BOTTLE_RUTO_LETTER, SHARED_BOTTLE_EMPTY, SHARED_BOTTLE_RUTO_LETTER, SHARED_BOTTLED_GOLD_DUST, SHARED_BOTTLE_POTION_RED, SHARED_BOTTLE_POTION_GREEN, SHARED_BOTTLE_POTION_BLUE, SHARED_BOTTLE_FAIRY, SHARED_BOTTLE_BLUE_FIRE, SHARED_BOTTLE_MILK, SHARED_BOTTLE_CHATEAU, SHARED_BOTTLE_POE, SHARED_BOTTLE_BIG_POE },     true  },
    { EGameIcon::ruto, "Ruto's Letter", { OOT_BOTTLE_RUTO_LETTER, MM_BOTTLE_RUTO_LETTER, SHARED_BOTTLE_RUTO_LETTER }, false },

    // Trade
    { EGameIcon::egg, "Weird Egg", { OOT_WEIRD_EGG }, false },
    { EGameIcon::letter, "Zelda's Letter", { OOT_ZELDA_LETTER }, false },
    { EGameIcon::cucco, "Chicken", { OOT_CHICKEN }, false },
    { EGameIcon::egg, "Pocket Egg", { OOT_POCKET_EGG }, false },
    { EGameIcon::cucco, "Pocket Cucco", { OOT_POCKET_CUCCO }, false },
    { EGameIcon::cojiro, "Cojiro", { OOT_COJIRO }, false },
    { EGameIcon::mushroom, "Odd Mushroom", { OOT_ODD_MUSHROOM }, false },
    { EGameIcon::potion, "Odd Potion", { OOT_ODD_POTION }, false },
    { EGameIcon::saw, "Poacher's Saw", { OOT_POACHER_SAW }, false },
    { EGameIcon::broken_big, "Broken Goron's Sword", { OOT_BROKEN_GORON_SWORD }, false },
    { EGameIcon::prescription, "Prescription", { OOT_PRESCRIPTION }, false },
    { EGameIcon::frog, "Eyeball Frog", { OOT_EYEBALL_FROG }, false },
    { EGameIcon::eye_drops, "Eye Drops", { OOT_EYE_DROPS }, false },
    { EGameIcon::claim, "Claim Check", { OOT_CLAIM_CHECK }, false },

    // Masks
    { EGameIcon::truth, "Mask of Truth", { OOT_MASK_TRUTH, SHARED_MASK_TRUTH }, false },
    { EGameIcon::keaton, "Keaton Mask", { OOT_MASK_KEATON, SHARED_MASK_KEATON }, false },
    { EGameIcon::skull, "Skull Mask", { OOT_MASK_SKULL }, false },
    { EGameIcon::spooky, "Spooky Mask", { OOT_MASK_SPOOKY }, false },
    { EGameIcon::bunny, "Bunny Hood", { OOT_MASK_BUNNY, SHARED_MASK_BUNNY }, false },
    { EGameIcon::gerudo, "Gerudo Mask", { OOT_MASK_GERUDO }, false },
    { EGameIcon::goron, "Goron Mask", { OOT_MASK_GORON, SHARED_MASK_GORON }, false },
    { EGameIcon::zora, "Zora Mask", { OOT_MASK_ZORA, SHARED_MASK_ZORA }, false },
    { EGameIcon::blast, "Blast Mask", { OOT_MASK_BLAST, SHARED_MASK_BLAST }, false },
    { EGameIcon::stone, "Stone Mask", { OOT_MASK_STONE, SHARED_MASK_STONE }, false },
    { EGameIcon::kamaro, "Kamaro's Mask", { OOT_MASK_KAMARO, SHARED_MASK_KAMARO }, false },
};

const ProgSection OoTSections[NumOoTSections] =
{
    MAKE_SECTION("Quest Status",         OoTQuestStatus),
    MAKE_SECTION("Dungeons Items",                OoTDungeonsItems),
    MAKE_SECTION("Equipments", OoTEquipments),
    MAKE_SECTION("Major Items",      OoTMajorItems),
};

#pragma endregion OoT Page

#pragma region MM Page

const ProgEntry MMQuestStatus[] =
{
    // Remains
    { EGameIcon::odolwa,    "Odolwa's Remains",   { MM_REMAINS_ODOLWA },   false },
    { EGameIcon::goht,      "Goht's Remains",     { MM_REMAINS_GOHT },     false },
    { EGameIcon::gyorg,     "Gyorg's Remains",    { MM_REMAINS_GYORG },    false },
    { EGameIcon::twinmold,  "Twinmold's Remains", { MM_REMAINS_TWINMOLD }, false },

    // Maps
    { EGameIcon::map, "World Map of Clock Town", { MM_WORLD_MAP_CLOCK_TOWN }, false },
    { EGameIcon::map, "World Map of Great Bay", { MM_WORLD_MAP_GREAT_BAY }, false },
    { EGameIcon::map, "World Map of Ranch", { MM_WORLD_MAP_ROMANI_RANCH }, false },
    { EGameIcon::map, "World Map of Snowhead", { MM_WORLD_MAP_SNOWHEAD }, false },
    { EGameIcon::map, "World Map of Stone Tower", { MM_WORLD_MAP_STONE_TOWER }, false },
    { EGameIcon::map, "World Map of Woodfall", { MM_WORLD_MAP_WOODFALL }, false },

    // Hearts / Magic
    { EGameIcon::heart_piece, "Piece of Heart", { MM_HEART_PIECE, SHARED_HEART_PIECE }, true },
    { EGameIcon::heart_container, "Heart Container", { MM_HEART_CONTAINER, SHARED_HEART_CONTAINER }, true },
    { EGameIcon::defense, "Double Defense", { MM_DEFENSE_UPGRADE, SHARED_DEFENSE_UPGRADE }, false },
    { EGameIcon::magic_upgrade,       "Magic Upgrade",       { MM_MAGIC_UPGRADE, SHARED_MAGIC_UPGRADE },      false },
    { EGameIcon::large_magic_upgrade, "Large Magic Upgrade Magic",        { MM_MAGIC_UPGRADE2, SHARED_MAGIC_UPGRADE },       false },

    // Buttons
    { EGameIcon::a_button, "A Button", { MM_BUTTON_A, SHARED_BUTTON_A }, false },
    { EGameIcon::c_down, "C-Down Button", { MM_BUTTON_C_DOWN, SHARED_BUTTON_C_DOWN }, false },
    { EGameIcon::c_left, "C-Left Button", { MM_BUTTON_C_LEFT, SHARED_BUTTON_C_LEFT }, false },
    { EGameIcon::c_right, "C-Right Button", { MM_BUTTON_C_RIGHT, SHARED_BUTTON_C_RIGHT }, false },
    { EGameIcon::c_up, "C-Up Button", { MM_BUTTON_C_UP, SHARED_BUTTON_C_UP }, false },

    // Songs
    { EGameIcon::song,        "Song of Time",        { MM_SONG_TIME, MM_SONG_NOTE_TIME, SHARED_SONG_TIME, SHARED_SONG_NOTE_TIME },    true },
    { EGameIcon::song,        "Song of Healing",     { MM_SONG_HEALING, MM_SONG_NOTE_HEALING, SHARED_SONG_NOTE_HEALING, SHARED_SONG_HEALING }, true },
    { EGameIcon::song,        "Epona's Song",        { MM_SONG_EPONA, MM_SONG_NOTE_EPONA, SHARED_SONG_EPONA, SHARED_SONG_NOTE_EPONA },           true },
    { EGameIcon::song,        "Song of Soaring",     { MM_SONG_SOARING, MM_SONG_NOTE_SOARING, SHARED_SONG_NOTE_SOARING, SHARED_SONG_SOARING }, true },
    { EGameIcon::song,        "Song of Storms",      { MM_SONG_STORMS, MM_SONG_NOTE_STORMS, SHARED_SONG_STORMS, SHARED_SONG_NOTE_STORMS },  true },
    { EGameIcon::song,        "Sun's Song",          { MM_SONG_SUN, MM_SONG_NOTE_SUN, SHARED_SONG_SUN, SHARED_SONG_NOTE_SUN },      true },
    { EGameIcon::song_green,  "Sonata of Awakening", { MM_SONG_AWAKENING, MM_SONG_NOTE_AWAKENING, SHARED_SONG_NOTE_AWAKENING, SHARED_SONG_AWAKENING },          true },
    { EGameIcon::song_red,    "Goron Lullaby",       { MM_SONG_GORON_HALF, MM_SONG_GORON, MM_SONG_NOTE_GORON, SHARED_SONG_NOTE_GORON, SHARED_SONG_GORON, SHARED_SONG_GORON_HALF },   true },
    { EGameIcon::song_blue,   "New Wave Bossa Nova", { MM_SONG_ZORA, MM_SONG_NOTE_ZORA, SHARED_SONG_NOTE_ZORA, SHARED_SONG_ZORA },      true },
    { EGameIcon::song_orange, "Elegy of Emptiness",  { MM_SONG_EMPTINESS, MM_SONG_NOTE_EMPTINESS, SHARED_SONG_EMPTINESS, SHARED_SONG_NOTE_EMPTINESS },           true },
    { EGameIcon::song_purple, "Oath to Order",       { MM_SONG_ORDER, MM_SONG_NOTE_ORDER, SHARED_SONG_NOTE_ORDER, SHARED_SONG_ORDER },   true },
    { EGameIcon::song,        "Zelda's Lullaby",     { MM_SONG_ZELDA, MM_SONG_NOTE_ZELDA, SHARED_SONG_NOTE_ZELDA, SHARED_SONG_ZELDA },         true },
    { EGameIcon::song,        "Saria's Song",        { MM_SONG_SARIA, MM_SONG_NOTE_SARIA, SHARED_SONG_NOTE_SARIA, SHARED_SONG_SARIA },           true },
    { EGameIcon::song_green,  "Minuet of Forest",    { MM_SONG_TP_FOREST, MM_SONG_NOTE_TP_FOREST, SHARED_SONG_NOTE_TP_FOREST, SHARED_SONG_TP_FOREST },          true },
    { EGameIcon::song_red,    "Bolero of Fire",      { MM_SONG_TP_FIRE, MM_SONG_NOTE_TP_FIRE, SHARED_SONG_NOTE_TP_FIRE, SHARED_SONG_TP_FIRE },          true },
    { EGameIcon::song_blue,   "Serenade of Water",   { MM_SONG_TP_WATER, MM_SONG_NOTE_TP_FIRE, SHARED_SONG_NOTE_TP_WATER, SHARED_SONG_TP_WATER },        true },
    { EGameIcon::song_purple, "Nocturne of Shadow",  { MM_SONG_TP_SHADOW, MM_SONG_NOTE_TP_FIRE, SHARED_SONG_NOTE_TP_SHADOW, SHARED_SONG_TP_SHADOW },        true },
    { EGameIcon::song_orange, "Requiem of Spirit",   { MM_SONG_TP_SPIRIT, MM_SONG_NOTE_TP_FIRE, SHARED_SONG_NOTE_TP_SPIRIT, SHARED_SONG_TP_SPIRIT },         true },
    { EGameIcon::song_yellow, "Prelude of Light",    { MM_SONG_TP_LIGHT, MM_SONG_NOTE_TP_FIRE, SHARED_SONG_NOTE_TP_LIGHT, SHARED_SONG_TP_LIGHT },         true },

    // Owls
    { EGameIcon::owl, "Owl Statue (Clock Town)", { MM_OWL_CLOCK_TOWN }, false },
    { EGameIcon::owl, "Owl Statue (Milk Road)", { MM_OWL_MILK_ROAD }, false },
    { EGameIcon::owl, "Owl Statue (Southern Swamp)", { MM_OWL_SOUTHERN_SWAMP }, false },
    { EGameIcon::owl, "Owl Statue (Woodfall)", { MM_OWL_WOODFALL }, false },
    { EGameIcon::owl, "Owl Statue (Mountain Village)", { MM_OWL_MOUNTAIN_VILLAGE }, false },
    { EGameIcon::owl, "Owl Statue (Snowhead)", { MM_OWL_SNOWHEAD }, false },
    { EGameIcon::owl, "Owl Statue (Great Bay)", { MM_OWL_GREAT_BAY }, false },
    { EGameIcon::owl, "Owl Statue (Zora Cape)", { MM_OWL_ZORA_CAPE }, false },
    { EGameIcon::owl, "Owl Statue (Ikana Canyon)", { MM_OWL_IKANA_CANYON }, false },
    { EGameIcon::owl, "Owl Statue (Stone Tower)", { MM_OWL_STONE_TOWER }, false },

    // Clocks
    { EGameIcon::clock_d1, "Clock (Day 1)", { MM_CLOCK1, MM_CLOCK }, false },
    { EGameIcon::clock_n1, "Clock (Night 1)", { MM_CLOCK2, MM_CLOCK }, false },
    { EGameIcon::clock_d2, "Clock (Day 2)", { MM_CLOCK3, MM_CLOCK }, false },
    { EGameIcon::clock_n2, "Clock (Night 2)", { MM_CLOCK4, MM_CLOCK }, false },
    { EGameIcon::clock_d3, "Clock (Day 3)", { MM_CLOCK5, MM_CLOCK }, false },
    { EGameIcon::clock_n3, "Clock (Night 3)", { MM_CLOCK6, MM_CLOCK }, false },
};

const ProgEntry MMDungeonsItems[] =
{
    // Maps
    { EGameIcon::map, "Map (Woodfall Temple)", { MM_MAP_WF, MM_MAP }, false },
    { EGameIcon::map, "Map (Snowhead Temple)", { MM_MAP_SH, MM_MAP }, false },
    { EGameIcon::map, "Map (Great Bay Temple)", { MM_MAP_GB, MM_MAP }, false },
    { EGameIcon::map, "Map (Stone Tower Temple)", { MM_MAP_ST, MM_MAP }, false },

    // Compasses
    { EGameIcon::compass, "Compass (Woodfall Temple)", { MM_COMPASS_WF, MM_COMPASS }, false },
    { EGameIcon::compass, "Compass (Snowhead Temple)", { MM_COMPASS_SH, MM_COMPASS }, false },
    { EGameIcon::compass, "Compass (Great Bay Temple)", { MM_COMPASS_GB, MM_COMPASS }, false },
    { EGameIcon::compass, "Compass (Stone Tower Temple)", { MM_COMPASS_ST, MM_COMPASS }, false },

    // Boss keys
    { EGameIcon::boss_key, "Boss Key (Woodfall Temple)", { MM_BOSS_KEY_WF, MM_BOSS_KEY }, false },
    { EGameIcon::boss_key, "Boss Key (Snowhead Temple)", { MM_BOSS_KEY_SH, MM_BOSS_KEY }, false },
    { EGameIcon::boss_key, "Boss Key (Great Bay Temple)", { MM_BOSS_KEY_GB, MM_BOSS_KEY }, false },
    { EGameIcon::boss_key, "Boss Key (Stone Tower Temple)", { MM_BOSS_KEY_ST, MM_BOSS_KEY }, false },

    // Small keys / Key ring
    { EGameIcon::small_key, "Small Key (Woodfall Temple)", { MM_SMALL_KEY_WF, MM_SMALL_KEY }, true },
    { EGameIcon::small_key, "Small Key (Snowhead Temple)", { MM_SMALL_KEY_SH, MM_SMALL_KEY }, true },
    { EGameIcon::small_key, "Small Key (Great Bay Temple)", { MM_SMALL_KEY_GB, MM_SMALL_KEY }, true },
    { EGameIcon::small_key, "Small Key (Stone Tower Temple)", { MM_SMALL_KEY_ST, MM_SMALL_KEY }, true },
    { EGameIcon::skeleton_key, "Skeleton Key", { MM_SKELETON_KEY, SHARED_SKELETON_KEY }, false },
    { EGameIcon::key_ring, "Key Ring (Woodfall)", { MM_KEY_RING_WF, MM_KEY_RING }, false },
    { EGameIcon::key_ring, "Key Ring (Snowhead)", { MM_KEY_RING_SH, MM_KEY_RING }, false },
    { EGameIcon::key_ring, "Key Ring (Great Bay)", { MM_KEY_RING_GB, MM_KEY_RING }, false },
    { EGameIcon::key_ring, "Key Ring (Stone Tower)", { MM_KEY_RING_ST, MM_KEY_RING }, false },

    // Stray Fairy
    { EGameIcon::sf,              "Stray Fairy (Woodfall)",      { MM_STRAY_FAIRY_WF, MM_STRAY_FAIRY },     true },
    { EGameIcon::sf_green,        "Stray Fairy (Snowhead)",      { MM_STRAY_FAIRY_SH, MM_STRAY_FAIRY },     true },
    { EGameIcon::sf_blue,         "Stray Fairy (Great Bay)",     { MM_STRAY_FAIRY_GB, MM_STRAY_FAIRY },    true },
    { EGameIcon::sf_yellow,       "Stray Fairy (Stone Tower)",   { MM_STRAY_FAIRY_ST, MM_STRAY_FAIRY },  true },
    { EGameIcon::sf_orange,       "Stray Fairy (Clock Town)",    { MM_STRAY_FAIRY_TOWN, MM_STRAY_FAIRY },          false },
    { EGameIcon::sf_transcendent, "Transcendent Fairy",          { MM_TRANSCENDENT_FAIRY },          false },

    // Skulltulas
    { EGameIcon::swamp_token,     "Swamp Skulltula Token",       { MM_GS_TOKEN_SWAMP },     true },
    { EGameIcon::ocean_token,     "Ocean Skulltula Token",       { MM_GS_TOKEN_OCEAN },     true },
    { EGameIcon::platinum_token, "Platinum Token", { MM_PLATINUM_TOKEN, SHARED_PLATINUM_TOKEN }, false },
};

const ProgEntry MMEquipments[] =
{
    // Swords
    { EGameIcon::spin, "Spin Attack Upgrade", { MM_SPIN_UPGRADE, SHARED_SPIN_UPGRADE }, false },
    { EGameIcon::kokiri_mm, "Kokiri Sword", { MM_SWORD_KOKIRI, MM_SWORD, SHARED_SWORD }, false },
    { EGameIcon::razor, "Razor Sword", { MM_SWORD_RAZOR, MM_SWORD, SHARED_SWORD }, false },
    { EGameIcon::gilded, "Gilded Sword", { MM_SWORD_GILDED, MM_SWORD, SHARED_SWORD }, false },

    // Shields
    { EGameIcon::deku_shield, "Deku Shield", { MM_SHIELD_DEKU, MM_PROGRESSIVE_SHIELD_DEKU, MM_SHIELD, SHARED_SHIELD, SHARED_SHIELD_DEKU }, false },
    { EGameIcon::hero_shield, "Hero's Shield", { MM_SHIELD_HERO, MM_PROGRESSIVE_SHIELD_HERO, MM_SHIELD, SHARED_SHIELD, SHARED_SHIELD_HYLIAN }, false },
    { EGameIcon::mirror_mm, "Mirror Shield", { MM_SHIELD_MIRROR, MM_SHIELD, SHARED_SHIELD, SHARED_SHIELD_MIRROR }, false },

    // Quivers / Bags
    { EGameIcon::big_quiver, "Big Quiver", { MM_QUIVER2 }, false },
    { EGameIcon::biggest_quiver, "Biggest Quiver", { MM_QUIVER3 }, false },
    { EGameIcon::bomb_bag, "Bomb Bag", { MM_BOMB_BAG, SHARED_BOMB_BAG }, false },
    { EGameIcon::big_bomb, "Big Bomb Bag", { MM_BOMB_BAG2, SHARED_BOMB_BAG }, false },
    { EGameIcon::biggest_bomb, "Biggest Bomb Bag", { MM_BOMB_BAG3, SHARED_BOMB_BAG }, false },
    { EGameIcon::bombchu_bag, "Bombchu Bag", { MM_BOMBCHU_BAG, MM_BOMBCHU_BAG_FIRST_1, MM_BOMBCHU_BAG_FIRST_5, MM_BOMBCHU_BAG_FIRST_10, MM_BOMBCHU_BAG_FIRST_20, SHARED_BOMBCHU_BAG }, false },
    { EGameIcon::bombchu_bag, "Big Bombchu Bag", { MM_BOMBCHU_BAG2, SHARED_BOMBCHU_BAG }, false },
    { EGameIcon::bombchu_bag, "Biggest Bombchu Bag", { MM_BOMBCHU_BAG3, SHARED_BOMBCHU_BAG }, false },

    // Others
    { EGameIcon::bombers, "Bombers' Notebook", { MM_BOMBER_NOTEBOOK }, false },
    { EGameIcon::child_wallet, "Child Wallet", { MM_WALLET, SHARED_WALLET }, false },
    { EGameIcon::wallet, "Adult Wallet", { MM_WALLET2, SHARED_WALLET }, false },
    { EGameIcon::big_wallet, "Giant Wallet", { MM_WALLET3, SHARED_WALLET }, false },
    { EGameIcon::colossal_wallet, "Colossal Wallet", { MM_WALLET4, SHARED_WALLET }, false },
    { EGameIcon::bottomless_wallet, "Bottomless Wallet", { MM_WALLET5, SHARED_WALLET }, false },

    // OoT Upgrades
    { EGameIcon::nut_upgrade_1, "Deku Nut Upgrade", { MM_NUT_UPGRADE, SHARED_NUT_UPGRADE }, false },
    { EGameIcon::nut_upgrade_2, "Second Deku Nut Upgrade", { MM_NUT_UPGRADE2, SHARED_NUT_UPGRADE }, false },
    { EGameIcon::stick_upgrade_1, "Deku Stick Upgrade", { MM_STICK_UPGRADE, SHARED_STICK_UPGRADE }, false },
    { EGameIcon::stick_upgrade_2, "Second Deku Stick Upgrade", { MM_STICK_UPGRADE2, SHARED_STICK_UPGRADE }, false },
    { EGameIcon::bronze_scale, "Bronze Scale", { MM_SCALE_BRONZE, MM_SCALE, SHARED_SCALE }, false },
    { EGameIcon::silver, "Silver Scale", { MM_SCALE_SILVER, MM_SCALE, SHARED_SCALE }, false },
    { EGameIcon::golden, "Golden Scale", { MM_SCALE_GOLDEN, MM_SCALE, SHARED_SCALE }, false },
    { EGameIcon::bracelet, "Goron's Bracelet", { MM_GORON_BRACELET, MM_STRENGTH, SHARED_STRENGTH }, false },
    { EGameIcon::silver_gauntlet, "Silver Gauntlets", { MM_SILVER_GAUNTLETS, MM_STRENGTH, SHARED_STRENGTH }, false },
    { EGameIcon::golden_gauntlet, "Golden Gauntlets", { MM_GOLDEN_GAUNTLETS, MM_STRENGTH, SHARED_STRENGTH }, false },
    { EGameIcon::agony, "Stone of Agony", { MM_STONE_OF_AGONY, SHARED_STONE_OF_AGONY }, false },
};

const ProgEntry MMMasks[] =
{
    { EGameIcon::deku,   "Deku Mask",            { MM_MASK_DEKU },          false },
    { EGameIcon::goron,  "Goron Mask",           { MM_MASK_GORON, SHARED_MASK_GORON },         false },
    { EGameIcon::zora,   "Zora Mask",            { MM_MASK_ZORA, SHARED_MASK_ZORA },          false },
    { EGameIcon::deity,  "Fierce Deity's Mask",  { MM_MASK_FIERCE_DEITY },       false },
    { EGameIcon::giant,  "Giant's Mask", { MM_MASK_GIANT }, false },
    { EGameIcon::night, "All-Night Mask", { MM_MASK_ALL_NIGHT }, false },
    { EGameIcon::blast, "Blast Mask", { MM_MASK_BLAST, SHARED_MASK_BLAST }, false },
    { EGameIcon::bremen, "Bremen Mask", { MM_MASK_BREMEN }, false },
    { EGameIcon::bunny, "Bunny Hood", { MM_MASK_BUNNY, SHARED_MASK_BUNNY }, false },
    { EGameIcon::captain, "Captain's Hat", { MM_MASK_CAPTAIN }, false },
    { EGameIcon::couple, "Couple's Mask", { MM_MASK_COUPLE }, false },
    { EGameIcon::gero, "Don Gero's Mask", { MM_MASK_DON_GERO }, false },
    { EGameIcon::garo, "Garo's Mask", { MM_MASK_GARO }, false },
    { EGameIcon::gibdo, "Gibdo Mask", { MM_MASK_GIBDO }, false },
    { EGameIcon::fairy_mask, "Great Fairy's Mask", { MM_MASK_GREAT_FAIRY }, false },
    { EGameIcon::kafei, "Kafei's Mask", { MM_MASK_KAFEI }, false },
    { EGameIcon::kamaro, "Kamaro's Mask", { MM_MASK_KAMARO, SHARED_MASK_KAMARO }, false },
    { EGameIcon::keaton, "Keaton Mask", { MM_MASK_KEATON, SHARED_MASK_KEATON }, false },
    { EGameIcon::postman, "Postman's Hat", { MM_MASK_POSTMAN }, false },
    { EGameIcon::romani, "Romani's Mask", { MM_MASK_ROMANI }, false },
    { EGameIcon::scents, "Mask of Scents", { MM_MASK_SCENTS }, false },
    { EGameIcon::stone, "Stone Mask", { MM_MASK_STONE, SHARED_MASK_STONE }, false },
    { EGameIcon::troupe, "Circus Leader's Mask", { MM_MASK_TROUPE_LEADER }, false },
    { EGameIcon::truth, "Mask of Truth", { MM_MASK_TRUTH, SHARED_MASK_TRUTH }, false },
};

const ProgEntry MMMajorItems[] =
{
    { EGameIcon::land, "Land Title Deed", { MM_DEED_LAND }, false },
    { EGameIcon::mountain, "Mountain Title Deed", { MM_DEED_MOUNTAIN }, false },
    { EGameIcon::ocean, "Ocean Title Deed", { MM_DEED_OCEAN }, false },
    { EGameIcon::swamp, "Swamp Title Deed", { MM_DEED_SWAMP }, false },
    { EGameIcon::moon, "Moon's Tear", { MM_MOON_TEAR }, false },
    { EGameIcon::letter_kafei, "Letter to Kafei", { MM_LETTER_TO_KAFEI }, false },
    { EGameIcon::letter_mama, "Letter to Mama", { MM_LETTER_TO_MAMA }, false },
    { EGameIcon::key, "Room Key", { MM_ROOM_KEY }, false },
    { EGameIcon::pendant, "Pendant of Memories", { MM_PENDANT_OF_MEMORIES }, false },
    { EGameIcon::goron_tunic, "Goron Tunic", { MM_TUNIC_GORON, SHARED_TUNIC_GORON }, false },
    { EGameIcon::zora_tunic, "Zora Tunic", { MM_TUNIC_ZORA, SHARED_TUNIC_ZORA }, false },
    { EGameIcon::iron, "Iron Boots", { MM_BOOTS_IRON, SHARED_BOOTS_IRON }, false },
    { EGameIcon::hover, "Hover Boots", { MM_BOOTS_HOVER, SHARED_BOOTS_HOVER }, false },
    { EGameIcon::din, "Din's Fire", { MM_SPELL_FIRE, SHARED_SPELL_FIRE }, false },
    { EGameIcon::farore, "Farore's Wind", { MM_SPELL_WIND, SHARED_SPELL_WIND }, false },
    { EGameIcon::nayru, "Nayru's Love", { MM_SPELL_LOVE, SHARED_SPELL_LOVE }, false },
    { EGameIcon::hammer, "Megaton Hammer", { MM_HAMMER, SHARED_HAMMER }, false },
    { EGameIcon::boomerang,      "Boomerang",            { MM_BOOMERANG, SHARED_BOOMERANG },        false },
    { EGameIcon::fairy_ocarina, "Fairy Ocarina", { MM_OCARINA_FAIRY, MM_OCARINA, SHARED_OCARINA }, false },
    { EGameIcon::ocarina, "Ocarina of Time", { MM_OCARINA_OF_TIME, MM_OCARINA, SHARED_OCARINA }, false },
    { EGameIcon::hookshot_oot, "Short Hookshot", { MM_HOOKSHOT_SHORT, SHARED_HOOKSHOT }, false },
    { EGameIcon::hookshot_mm, "Hookshot", { MM_HOOKSHOT, SHARED_HOOKSHOT }, false },
    { EGameIcon::bow_mm,              "Hero's Bow",          { MM_BOW, SHARED_BOW },         false },
    { EGameIcon::fire_arrow,          "Fire Arrows", { MM_ARROW_FIRE, SHARED_ARROW_FIRE }, false },
    { EGameIcon::ice_arrow,           "Ice Arrows", { MM_ARROW_ICE, SHARED_ARROW_ICE }, false },
    { EGameIcon::light_arrow,         "Light Arrows", { MM_ARROW_LIGHT, SHARED_ARROW_LIGHT }, false },
    { EGameIcon::fairy_sword, "Great Fairy's Sword", { MM_GREAT_FAIRY_SWORD, MM_SWORD, SHARED_GREAT_FAIRY_SWORD }, false },
    { EGameIcon::lens, "Lens of Truth", { MM_LENS, SHARED_LENS }, false },
    { EGameIcon::bean, "Magic Beans", { MM_MAGIC_BEAN }, false },
    { EGameIcon::picto,               "Pictograph Box",      { MM_PICTOGRAPH_BOX },         false },
    { EGameIcon::powder, "Powder Keg", { MM_POWDER_KEG, SHARED_POWDER_KEG }, false },
    { EGameIcon::bottle, "Empty Bottle", { MM_BOTTLE_EMPTY, OOT_BOTTLED_GOLD_DUST, MM_BOTTLED_GOLD_DUST, MM_BOTTLE_CHATEAU, MM_BOTTLE_FAIRY, MM_BOTTLE_MILK, MM_BOTTLE_POE, MM_BOTTLE_BIG_POE, MM_BOTTLE_POTION_BLUE, MM_BOTTLE_POTION_GREEN, MM_BOTTLE_POTION_RED, MM_WEIRD_MUSHROOM, MM_SEAHORSE2, OOT_SEAHORSE2, OOT_WEIRD_MUSHROOM, SHARED_BOTTLE_EMPTY, SHARED_BOTTLE_RUTO_LETTER, SHARED_BOTTLED_GOLD_DUST, SHARED_BOTTLE_POTION_RED, SHARED_BOTTLE_POTION_GREEN, SHARED_BOTTLE_POTION_BLUE, SHARED_BOTTLE_FAIRY, SHARED_BOTTLE_BLUE_FIRE, SHARED_BOTTLE_MILK, SHARED_BOTTLE_CHATEAU, SHARED_BOTTLE_POE, SHARED_BOTTLE_BIG_POE }, true },
    { EGameIcon::dust, "Bottle of Gold Dust", { MM_BOTTLED_GOLD_DUST, OOT_BOTTLED_GOLD_DUST, SHARED_BOTTLED_GOLD_DUST }, false },
    { EGameIcon::seahorse, "Seahorse", { MM_SEAHORSE2, OOT_SEAHORSE2 }, false },
    { EGameIcon::bottled_mushroom, "Magic Mushroom", { MM_WEIRD_MUSHROOM, OOT_WEIRD_MUSHROOM }, false },
};

const ProgSection MMSections[NumMMSections] =
{
    MAKE_SECTION("Quest Status",         MMQuestStatus),
    MAKE_SECTION("Dungeons Items",                MMDungeonsItems),
    MAKE_SECTION("Equipments", MMEquipments),
    MAKE_SECTION("Masks",                MMMasks),
    MAKE_SECTION("Major Items",      MMMajorItems),
};

#pragma endregion MM Page

#pragma region Souls

const ProgEntry SoulsOoTBoss[] =
{
    { EGameIcon::soul_of_boss, "Soul of Gohma",         { OOT_SOUL_BOSS_QUEEN_GOHMA }, false },
    { EGameIcon::soul_of_boss, "Soul of King Dodongo",  { OOT_SOUL_BOSS_KING_DODONGO }, false },
    { EGameIcon::soul_of_boss, "Soul of Barinade",      { OOT_SOUL_BOSS_BARINADE },    false },
    { EGameIcon::soul_of_boss, "Soul of Phantom Ganon", { OOT_SOUL_BOSS_PHANTOM_GANON },     false },
    { EGameIcon::soul_of_boss, "Soul of Volvagia",      { OOT_SOUL_BOSS_VOLVAGIA },    false },
    { EGameIcon::soul_of_boss, "Soul of Morpha",        { OOT_SOUL_BOSS_MORPHA },      false },
    { EGameIcon::soul_of_boss, "Soul of Bongo Bongo",   { OOT_SOUL_BOSS_BONGO_BONGO },       false },
    { EGameIcon::soul_of_boss, "Soul of Twinrova",      { OOT_SOUL_BOSS_TWINROVA },    false },
};

const ProgEntry SoulsMMBoss[] =
{
    { EGameIcon::soul_of_boss, "Soul of Odolwa",   { MM_SOUL_BOSS_ODOLWA },   false },
    { EGameIcon::soul_of_boss, "Soul of Goht",     { MM_SOUL_BOSS_GOHT },     false },
    { EGameIcon::soul_of_boss, "Soul of Gyorg",    { MM_SOUL_BOSS_GYORG },    false },
    { EGameIcon::soul_of_boss, "Soul of Twinmold", { MM_SOUL_BOSS_TWINMOLD }, false },
    { EGameIcon::soul_of_boss, "Soul of Igos",     { MM_SOUL_BOSS_IGOS }, false },
};

const ProgEntry SoulsOoTEnemies[] =
{
    { EGameIcon::soul_of_foe, "Soul of Anubis", { OOT_SOUL_ENEMY_ANUBIS }, false },
    { EGameIcon::soul_of_foe, "Soul of Armos", { OOT_SOUL_ENEMY_ARMOS, SHARED_SOUL_ENEMY_ARMOS }, false },
    { EGameIcon::soul_of_foe, "Soul of Baby Dodongos", { OOT_SOUL_ENEMY_BABY_DODONGO }, false },
    { EGameIcon::soul_of_foe, "Soul of Beamos", { OOT_SOUL_ENEMY_BEAMOS, SHARED_SOUL_ENEMY_BEAMOS }, false },
    { EGameIcon::soul_of_foe, "Soul of Biris/Baris", { OOT_SOUL_ENEMY_BIRI_BARI }, false },
    { EGameIcon::soul_of_foe, "Soul of Bubbles", { OOT_SOUL_ENEMY_BUBBLE, SHARED_SOUL_ENEMY_BUBBLE }, false },
    { EGameIcon::soul_of_foe, "Soul of Dark Link", { OOT_SOUL_ENEMY_DARK_LINK }, false },
    { EGameIcon::soul_of_foe, "Soul of Dead Hands", { OOT_SOUL_ENEMY_DEAD_HAND }, false },
    { EGameIcon::soul_of_foe, "Soul of Deku Babas", { OOT_SOUL_ENEMY_DEKU_BABA, SHARED_SOUL_ENEMY_DEKU_BABA }, false },
    { EGameIcon::soul_of_foe, "Soul of Deku Scrubs", { OOT_SOUL_ENEMY_DEKU_SCRUB, SHARED_SOUL_ENEMY_DEKU_SCRUB }, false },
    { EGameIcon::soul_of_foe, "Soul of Dodongos", { OOT_SOUL_ENEMY_DODONGO, SHARED_SOUL_ENEMY_DODONGO }, false },
    { EGameIcon::soul_of_foe, "Soul of the Fighting Gerudos", { OOT_SOUL_ENEMY_THIEVES, SHARED_SOUL_ENEMY_THIEVES }, false },
    { EGameIcon::soul_of_foe, "Soul of Flare Dancers", { OOT_SOUL_ENEMY_FLARE_DANCER }, false },
    { EGameIcon::soul_of_foe, "Soul of Floormasters", { OOT_SOUL_ENEMY_FLOORMASTER, SHARED_SOUL_ENEMY_FLOORMASTER }, false },
    { EGameIcon::soul_of_foe, "Soul of Flying Pots", { OOT_SOUL_ENEMY_FLYING_POT, SHARED_SOUL_ENEMY_FLYING_POT }, false },
    { EGameIcon::soul_of_foe, "Soul of Freezards", { OOT_SOUL_ENEMY_FREEZARD, SHARED_SOUL_ENEMY_FREEZARD }, false },
    { EGameIcon::soul_of_foe, "Soul of Gohma Larvaes", { OOT_SOUL_ENEMY_GOHMA_LARVA }, false },
    { EGameIcon::soul_of_foe, "Soul of Guays", { OOT_SOUL_ENEMY_GUAY, SHARED_SOUL_ENEMY_GUAY }, false },
    { EGameIcon::soul_of_foe, "Soul of Iron Knuckles", { OOT_SOUL_ENEMY_IRON_KNUCKLE, SHARED_SOUL_ENEMY_IRON_KNUCKLE }, false },
    { EGameIcon::soul_of_foe, "Soul of Keese", { OOT_SOUL_ENEMY_KEESE, SHARED_SOUL_ENEMY_KEESE }, false },
    { EGameIcon::soul_of_foe, "Soul of Leevers", { OOT_SOUL_ENEMY_LEEVER, SHARED_SOUL_ENEMY_LEEVER }, false },
    { EGameIcon::soul_of_foe, "Soul of Like Likes", { OOT_SOUL_ENEMY_LIKE_LIKE, SHARED_SOUL_ENEMY_LIKE_LIKE }, false },
    { EGameIcon::soul_of_foe, "Soul of Lizalfos/Dinolfos", { OOT_SOUL_ENEMY_LIZALFOS_DINOLFOS, SHARED_SOUL_ENEMY_LIZALFOS_DINOLFOS }, false },
    { EGameIcon::soul_of_foe, "Soul of Moblins", { OOT_SOUL_ENEMY_MOBLIN }, false },
    { EGameIcon::soul_of_foe, "Soul of Octoroks", { OOT_SOUL_ENEMY_OCTOROK, SHARED_SOUL_ENEMY_OCTOROK }, false },
    { EGameIcon::soul_of_foe, "Soul of Jabu-Jabu's Parasites", { OOT_SOUL_ENEMY_PARASITE }, false },
    { EGameIcon::soul_of_foe, "Soul of Peahats", { OOT_SOUL_ENEMY_PEAHAT, SHARED_SOUL_ENEMY_PEAHAT }, false },
    { EGameIcon::soul_of_foe, "Soul of Poes", { OOT_SOUL_ENEMY_POE, SHARED_SOUL_ENEMY_POE }, false },
    { EGameIcon::soul_of_foe, "Soul of ReDeads/Gibdos", { OOT_SOUL_ENEMY_REDEAD_GIBDO, SHARED_SOUL_ENEMY_REDEAD_GIBDO }, false },
    { EGameIcon::soul_of_foe, "Soul of Shaboms", { OOT_SOUL_ENEMY_SHABOM }, false },
    { EGameIcon::soul_of_foe, "Soul of Shell Blades", { OOT_SOUL_ENEMY_SHELL_BLADE, SHARED_SOUL_ENEMY_SHELL_BLADE }, false },
    { EGameIcon::soul_of_foe, "Soul of Skull Kids", { OOT_SOUL_ENEMY_SKULL_KID }, false },
    { EGameIcon::soul_of_foe, "Soul of Skulltulas", { OOT_SOUL_ENEMY_SKULLTULA, SHARED_SOUL_ENEMY_SKULLTULA }, false },
    { EGameIcon::soul_of_foe, "Soul of Skullwalltulas", { OOT_SOUL_ENEMY_SKULLWALLTULA, SHARED_SOUL_ENEMY_SKULLWALLTULA }, false },
    { EGameIcon::soul_of_foe, "Soul of Spikes", { OOT_SOUL_ENEMY_SPIKE }, false },
    { EGameIcon::soul_of_foe, "Soul of Stalchildren", { OOT_SOUL_ENEMY_STALCHILD, SHARED_SOUL_ENEMY_STALCHILD }, false },
    { EGameIcon::soul_of_foe, "Soul of Stalfos", { OOT_SOUL_ENEMY_STALFOS }, false },
    { EGameIcon::soul_of_foe, "Soul of Stingers", { OOT_SOUL_ENEMY_STINGER }, false },
    { EGameIcon::soul_of_foe, "Soul of Tailpasarans", { OOT_SOUL_ENEMY_TAILPASARN }, false },
    { EGameIcon::soul_of_foe, "Soul of Tektites", { OOT_SOUL_ENEMY_TEKTITE, SHARED_SOUL_ENEMY_TEKTITE }, false },
    { EGameIcon::soul_of_foe, "Soul of Torch Slugs", { OOT_SOUL_ENEMY_TORCH_SLUG }, false },
    { EGameIcon::soul_of_foe, "Soul of Wallmasters", { OOT_SOUL_ENEMY_WALLMASTER, SHARED_SOUL_ENEMY_WALLMASTER }, false },
    { EGameIcon::soul_of_foe, "Soul of Wolfoses", { OOT_SOUL_ENEMY_WOLFOS, SHARED_SOUL_ENEMY_WOLFOS }, false },
};

const ProgEntry SoulsMMEnemies[] =
{
    { EGameIcon::soul_of_foe, "Soul of Armos", { MM_SOUL_ENEMY_ARMOS, SHARED_SOUL_ENEMY_ARMOS }, false },
    { EGameIcon::soul_of_foe, "Soul of Bad Bats", { MM_SOUL_ENEMY_BAD_BAT }, false },
    { EGameIcon::soul_of_foe, "Soul of Beamos", { MM_SOUL_ENEMY_BEAMOS, SHARED_SOUL_ENEMY_BEAMOS }, false },
    { EGameIcon::soul_of_foe, "Soul of Bio Babas", { MM_SOUL_ENEMY_BIO_BABA }, false },
    { EGameIcon::soul_of_foe, "Soul of Boes", { MM_SOUL_ENEMY_BOE }, false },
    { EGameIcon::soul_of_foe, "Soul of Bubbles", { MM_SOUL_ENEMY_BUBBLE, SHARED_SOUL_ENEMY_BUBBLE }, false },
    { EGameIcon::soul_of_foe, "Soul of Captain Keeta", { MM_SOUL_ENEMY_CAPTAIN_KEETA }, false },
    { EGameIcon::soul_of_foe, "Soul of Chuchus", { MM_SOUL_ENEMY_CHUCHU }, false },
    { EGameIcon::soul_of_foe, "Soul of Deep Pythons", { MM_SOUL_ENEMY_DEEP_PYTHON }, false },
    { EGameIcon::soul_of_foe, "Soul of Deku Babas", { MM_SOUL_ENEMY_DEKU_BABA, SHARED_SOUL_ENEMY_DEKU_BABA }, false },
    { EGameIcon::soul_of_foe, "Soul of Deku Scrubs", { MM_SOUL_ENEMY_DEKU_SCRUB, SHARED_SOUL_ENEMY_DEKU_SCRUB }, false },
    { EGameIcon::soul_of_foe, "Soul of Dexihands", { MM_SOUL_ENEMY_DEXIHAND }, false },
    { EGameIcon::soul_of_foe, "Soul of Dodongos", { MM_SOUL_ENEMY_DODONGO, SHARED_SOUL_ENEMY_DODONGO }, false },
    { EGameIcon::soul_of_foe, "Soul of Dragonflies", { MM_SOUL_ENEMY_DRAGONFLY }, false },
    { EGameIcon::soul_of_foe, "Soul of Eenoes", { MM_SOUL_ENEMY_EENO }, false },
    { EGameIcon::soul_of_foe, "Soul of Eyegores", { MM_SOUL_ENEMY_EYEGORE }, false },
    { EGameIcon::soul_of_foe, "Soul of the Fighting Pirates", { MM_SOUL_ENEMY_THIEVES, SHARED_SOUL_ENEMY_THIEVES }, false },
    { EGameIcon::soul_of_foe, "Soul of Floormasters", { MM_SOUL_ENEMY_FLOORMASTER, SHARED_SOUL_ENEMY_FLOORMASTER }, false },
    { EGameIcon::soul_of_foe, "Soul of Flying Pots", { MM_SOUL_ENEMY_FLYING_POT, SHARED_SOUL_ENEMY_FLYING_POT }, false },
    { EGameIcon::soul_of_foe, "Soul of Freezards", { MM_SOUL_ENEMY_FREEZARD, SHARED_SOUL_ENEMY_FREEZARD }, false },
    { EGameIcon::soul_of_foe, "Soul of Garo", { MM_SOUL_ENEMY_GARO }, false },
    { EGameIcon::soul_of_foe, "Soul of Gekkos", { MM_SOUL_ENEMY_GEKKO }, false },
    { EGameIcon::soul_of_foe, "Soul of Gomess", { MM_SOUL_ENEMY_GOMESS }, false },
    { EGameIcon::soul_of_foe, "Soul of Guays", { MM_SOUL_ENEMY_GUAY, SHARED_SOUL_ENEMY_GUAY }, false },
    { EGameIcon::soul_of_foe, "Soul of Hiploops", { MM_SOUL_ENEMY_HIPLOOP }, false },
    { EGameIcon::soul_of_foe, "Soul of Iron Knuckles", { MM_SOUL_ENEMY_IRON_KNUCKLE, SHARED_SOUL_ENEMY_IRON_KNUCKLE }, false },
    { EGameIcon::soul_of_foe, "Soul of Keese", { MM_SOUL_ENEMY_KEESE, SHARED_SOUL_ENEMY_KEESE }, false },
    { EGameIcon::soul_of_foe, "Soul of Leevers", { MM_SOUL_ENEMY_LEEVER, SHARED_SOUL_ENEMY_LEEVER }, false },
    { EGameIcon::soul_of_foe, "Soul of Like Likes", { MM_SOUL_ENEMY_LIKE_LIKE, SHARED_SOUL_ENEMY_LIKE_LIKE }, false },
    { EGameIcon::soul_of_foe, "Soul of Lizalfos/Dinolfos", { MM_SOUL_ENEMY_LIZALFOS_DINOLFOS, SHARED_SOUL_ENEMY_LIZALFOS_DINOLFOS }, false },
    { EGameIcon::soul_of_foe, "Soul of Nejirons", { MM_SOUL_ENEMY_NEJIRON }, false },
    { EGameIcon::soul_of_foe, "Soul of Octoroks", { MM_SOUL_ENEMY_OCTOROK, SHARED_SOUL_ENEMY_OCTOROK }, false },
    { EGameIcon::soul_of_foe, "Soul of Peahats", { MM_SOUL_ENEMY_PEAHAT, SHARED_SOUL_ENEMY_PEAHAT }, false },
    { EGameIcon::soul_of_foe, "Soul of Poes", { MM_SOUL_ENEMY_POE, SHARED_SOUL_ENEMY_POE }, false },
    { EGameIcon::soul_of_foe, "Soul of Real Bombchu", { MM_SOUL_ENEMY_REAL_BOMBCHU }, false },
    { EGameIcon::soul_of_foe, "Soul of ReDeads/Gibdos", { MM_SOUL_ENEMY_REDEAD_GIBDO, SHARED_SOUL_ENEMY_REDEAD_GIBDO }, false },
    { EGameIcon::soul_of_foe, "Soul of Shell Blades", { MM_SOUL_ENEMY_SHELL_BLADE, SHARED_SOUL_ENEMY_SHELL_BLADE }, false },
    { EGameIcon::soul_of_foe, "Soul of Skullfish", { MM_SOUL_ENEMY_SKULLFISH }, false },
    { EGameIcon::soul_of_foe, "Soul of Skulltulas", { MM_SOUL_ENEMY_SKULLTULA, SHARED_SOUL_ENEMY_SKULLTULA }, false },
    { EGameIcon::soul_of_foe, "Soul of Skullwalltulas", { MM_SOUL_ENEMY_SKULLWALLTULA, SHARED_SOUL_ENEMY_SKULLWALLTULA }, false },
    { EGameIcon::soul_of_foe, "Soul of Snappers", { MM_SOUL_ENEMY_SNAPPER }, false },
    { EGameIcon::soul_of_foe, "Soul of Stalchildren", { MM_SOUL_ENEMY_STALCHILD, SHARED_SOUL_ENEMY_STALCHILD }, false },
    { EGameIcon::soul_of_foe, "Soul of Takkuri", { MM_SOUL_ENEMY_TAKKURI }, false },
    { EGameIcon::soul_of_foe, "Soul of Tektites", { MM_SOUL_ENEMY_TEKTITE, SHARED_SOUL_ENEMY_TEKTITE }, false },
    { EGameIcon::soul_of_foe, "Soul of Wallmasters", { MM_SOUL_ENEMY_WALLMASTER, SHARED_SOUL_ENEMY_WALLMASTER }, false },
    { EGameIcon::soul_of_foe, "Soul of Warts", { MM_SOUL_ENEMY_WART }, false },
    { EGameIcon::soul_of_foe, "Soul of Wizzrobes", { MM_SOUL_ENEMY_WIZZROBE }, false },
    { EGameIcon::soul_of_foe, "Soul of Wolfos", { MM_SOUL_ENEMY_WOLFOS, SHARED_SOUL_ENEMY_WOLFOS }, false },
};

const ProgEntry SoulsOoTNPCs[] =
{
    { EGameIcon::soul_of_npc, "Soul of Business Scrubs", { OOT_SOUL_MISC_BUSINESS_SCRUB, SHARED_SOUL_MISC_BUSINESS_SCRUB }, false },
    { EGameIcon::soul_of_npc, "Soul of Gold Skulltulas", { OOT_SOUL_MISC_GS, SHARED_SOUL_MISC_GS }, false },
    { EGameIcon::soul_of_npc, "Soul of Cucco Lady", { OOT_SOUL_NPC_ANJU, SHARED_SOUL_NPC_ANJU }, false },
    { EGameIcon::soul_of_npc, "Soul of the Astronomer", { OOT_SOUL_NPC_ASTRONOMER, SHARED_SOUL_NPC_ASTRONOMER }, false },
    { EGameIcon::soul_of_npc, "Soul of the Beggar", { OOT_SOUL_NPC_BANKER, SHARED_SOUL_NPC_BANKER }, false },
    { EGameIcon::soul_of_npc, "Soul of Bazaar Shopkeeper", { OOT_SOUL_NPC_BAZAAR_SHOPKEEPER, SHARED_SOUL_NPC_BAZAAR_SHOPKEEPER }, false },
    { EGameIcon::soul_of_npc, "Soul of the Bean Salesman", { OOT_SOUL_NPC_BEAN_SALESMAN, SHARED_SOUL_NPC_BEAN_SALESMAN }, false },
    { EGameIcon::soul_of_npc, "Soul of Biggoron", { OOT_SOUL_NPC_BIGGORON, SHARED_SOUL_NPC_BIGGORON }, false },
    { EGameIcon::soul_of_npc, "Soul of Bombchu Bowling Lady", { OOT_SOUL_NPC_BOMBCHU_BOWLING_LADY, SHARED_SOUL_NPC_BOMBCHU_BOWLING_LADY }, false },
    { EGameIcon::soul_of_npc, "Soul of Bombchu Shopkeeper", { OOT_SOUL_NPC_BOMBCHU_SHOPKEEPER, SHARED_SOUL_NPC_BOMBCHU_SHOPKEEPER }, false },
    { EGameIcon::soul_of_npc, "Soul of Graveyard Kid", { OOT_SOUL_NPC_BOMBERS, SHARED_SOUL_NPC_BOMBERS }, false },
    { EGameIcon::soul_of_npc, "Soul of Carpenters", { OOT_SOUL_NPC_CARPENTERS, SHARED_SOUL_NPC_CARPENTERS }, false },
    { EGameIcon::soul_of_npc, "Soul of the Carpet Man", { OOT_SOUL_NPC_CARPET_MAN, SHARED_SOUL_NPC_CARPET_MAN }, false },
    { EGameIcon::soul_of_npc, "Soul of Chest Game Owner", { OOT_SOUL_NPC_CHEST_GAME_OWNER, SHARED_SOUL_NPC_CHEST_GAME_OWNER }, false },
    { EGameIcon::soul_of_npc, "Soul of the Citizens", { OOT_SOUL_NPC_CITIZEN, SHARED_SOUL_NPC_CITIZEN }, false },
    { EGameIcon::soul_of_npc, "Soul of Composer Bros.", { OOT_SOUL_NPC_COMPOSER_BROS, SHARED_SOUL_NPC_COMPOSER_BROS }, false },
    { EGameIcon::soul_of_npc, "Soul of Dampe", { OOT_SOUL_NPC_DAMPE, SHARED_SOUL_NPC_DAMPE }, false },
    { EGameIcon::soul_of_npc, "Soul of Darunia", { OOT_SOUL_NPC_DARUNIA }, false },
    { EGameIcon::soul_of_npc, "Soul of the Dog Lady", { OOT_SOUL_NPC_DOG_LADY, SHARED_SOUL_NPC_DOG_LADY }, false },
    { EGameIcon::soul_of_npc, "Soul of Fishing Pond Owner", { OOT_SOUL_NPC_FISHING_POND_OWNER, SHARED_SOUL_NPC_FISHING_POND_OWNER }, false },
    { EGameIcon::soul_of_npc, "Soul of Ingo", { OOT_SOUL_NPC_GORMAN, SHARED_SOUL_NPC_GORMAN }, false },
    { EGameIcon::soul_of_npc, "Soul of Goron", { OOT_SOUL_NPC_GORON, SHARED_SOUL_NPC_GORON }, false },
    { EGameIcon::soul_of_npc, "Soul of Goron Child", { OOT_SOUL_NPC_GORON_CHILD, SHARED_SOUL_NPC_GORON_CHILD }, false },
    { EGameIcon::soul_of_npc, "Soul of Goron Shopkeeper", { OOT_SOUL_NPC_GORON_SHOPKEEPER, SHARED_SOUL_NPC_GORON_SHOPKEEPER }, false },
    { EGameIcon::soul_of_npc, "Soul of the Punk Kid", { OOT_SOUL_NPC_GROG, SHARED_SOUL_NPC_GROG }, false },
    { EGameIcon::soul_of_npc, "Soul of Guru-Guru", { OOT_SOUL_NPC_GURU_GURU, SHARED_SOUL_NPC_GURU_GURU }, false },
    { EGameIcon::soul_of_npc, "Soul of Honey & Darling", { OOT_SOUL_NPC_HONEY_DARLING, SHARED_SOUL_NPC_HONEY_DARLING }, false },
    { EGameIcon::soul_of_npc, "Soul of Hylian Guard", { OOT_SOUL_NPC_HYLIAN_GUARD }, false },
    { EGameIcon::soul_of_npc, "Soul of King Zora", { OOT_SOUL_NPC_KING_ZORA }, false },
    { EGameIcon::soul_of_npc, "Soul of Kokiri", { OOT_SOUL_NPC_KOKIRI }, false },
    { EGameIcon::soul_of_npc, "Soul of Kokiri Shopkeeper", { OOT_SOUL_NPC_KOKIRI_SHOPKEEPER }, false },
    { EGameIcon::soul_of_npc, "Soul of Malon", { OOT_SOUL_NPC_MALON, SHARED_SOUL_NPC_MALON }, false },
    { EGameIcon::soul_of_npc, "Soul of Medigoron", { OOT_SOUL_NPC_MEDIGORON, SHARED_SOUL_NPC_MEDIGORON }, false },
    { EGameIcon::soul_of_npc, "Soul of Mido", { OOT_SOUL_NPC_MIDO }, false },
    { EGameIcon::soul_of_npc, "Soul of the Old Hag", { OOT_SOUL_NPC_OLD_HAG, SHARED_SOUL_NPC_OLD_HAG }, false },
    { EGameIcon::soul_of_npc, "Soul of Poe Collector", { OOT_SOUL_NPC_POE_COLLECTOR, SHARED_SOUL_NPC_POE_COLLECTOR }, false },
    { EGameIcon::soul_of_npc, "Soul of Potion Shopkeeper", { OOT_SOUL_NPC_POTION_SHOPKEEPER }, false },
    { EGameIcon::soul_of_npc, "Soul of the Rooftop Man", { OOT_SOUL_NPC_ROOFTOP_MAN, SHARED_SOUL_NPC_ROOFTOP_MAN }, false },
    { EGameIcon::soul_of_npc, "Soul of Ruto", { OOT_SOUL_NPC_RUTO, SHARED_SOUL_NPC_RUTO }, false },
    { EGameIcon::soul_of_npc, "Soul of Saria", { OOT_SOUL_NPC_SARIA }, false },
    { EGameIcon::soul_of_npc, "Soul of the Scientist", { OOT_SOUL_NPC_SCIENTIST, SHARED_SOUL_NPC_SCIENTIST }, false },
    { EGameIcon::soul_of_npc, "Soul of Sheik", { OOT_SOUL_NPC_SHEIK }, false },
    { EGameIcon::soul_of_npc, "Soul of Shooting Gallery Owner", { OOT_SOUL_NPC_SHOOTING_GALLERY_OWNER, SHARED_SOUL_NPC_SHOOTING_GALLERY_OWNER }, false },
    { EGameIcon::soul_of_npc, "Soul of Talon", { OOT_SOUL_NPC_TALON, SHARED_SOUL_NPC_TALON }, false },
    { EGameIcon::soul_of_npc, "Soul of the Patrolling Gerudos", { OOT_SOUL_NPC_THIEVES, SHARED_SOUL_NPC_THIEVES }, false },
    { EGameIcon::soul_of_npc, "Soul of Zelda", { OOT_SOUL_NPC_ZELDA }, false },
    { EGameIcon::soul_of_npc, "Soul of Zora", { OOT_SOUL_NPC_ZORA, SHARED_SOUL_NPC_ZORA }, false },
    { EGameIcon::soul_of_npc, "Soul of Zora Shopkeeper", { OOT_SOUL_NPC_ZORA_SHOPKEEPER, SHARED_SOUL_NPC_ZORA_SHOPKEEPER }, false },
};

const ProgEntry SoulsOoTAnimals[] =
{
    { EGameIcon::soul_of_animal, "Soul of Butterflies", { OOT_SOUL_ANIMAL_BUTTERFLY, SHARED_SOUL_ANIMAL_BUTTERFLY }, false },
    { EGameIcon::soul_of_animal, "Soul of Cows", { OOT_SOUL_ANIMAL_COW, SHARED_SOUL_ANIMAL_COW }, false },
    { EGameIcon::soul_of_animal, "Soul of Cuccos", { OOT_SOUL_ANIMAL_CUCCO, SHARED_SOUL_ANIMAL_CUCCO }, false },
    { EGameIcon::soul_of_animal, "Soul of Dogs", { OOT_SOUL_ANIMAL_DOG, SHARED_SOUL_ANIMAL_DOG }, false },
};

const ProgEntry SoulsMMNPCs[] =
{
    { EGameIcon::soul_of_npc, "Soul of Business Scrubs", { MM_SOUL_MISC_BUSINESS_SCRUB, SHARED_SOUL_MISC_BUSINESS_SCRUB }, false },
    { EGameIcon::soul_of_npc, "Soul of Gold Skulltulas", { MM_SOUL_MISC_GS, SHARED_SOUL_MISC_GS }, false },
    { EGameIcon::soul_of_npc, "Soul of Anju", { MM_SOUL_NPC_ANJU, SHARED_SOUL_NPC_ANJU }, false },
    { EGameIcon::soul_of_npc, "Soul of Madame Aroma", { MM_SOUL_NPC_AROMA }, false },
    { EGameIcon::soul_of_npc, "Soul of Astronomer", { MM_SOUL_NPC_ASTRONOMER, SHARED_SOUL_NPC_ASTRONOMER }, false },
    { EGameIcon::soul_of_npc, "Soul of the Banker", { MM_SOUL_NPC_BANKER, SHARED_SOUL_NPC_BANKER, SHARED_SOUL_NPC_BAZAAR_SHOPKEEPER }, false },
    { EGameIcon::soul_of_npc, "Soul of Swamp Archery Owner", { MM_SOUL_NPC_BAZAAR_SHOPKEEPER, SHARED_SOUL_NPC_BAZAAR_SHOPKEEPER }, false },
    { EGameIcon::soul_of_npc, "Soul of the Beans Salesman", { MM_SOUL_NPC_BEAN_SALESMAN, SHARED_SOUL_NPC_BEAN_SALESMAN }, false },
    { EGameIcon::soul_of_npc, "Soul of Biggoron", { MM_SOUL_NPC_BIGGORON, SHARED_SOUL_NPC_BIGGORON }, false },
    { EGameIcon::soul_of_npc, "Soul of Blacksmiths", { MM_SOUL_NPC_BLACKSMITHS }, false },
    { EGameIcon::soul_of_npc, "Soul of Chest Game Lady", { MM_SOUL_NPC_BOMBCHU_BOWLING_LADY, SHARED_SOUL_NPC_BOMBCHU_BOWLING_LADY }, false },
    { EGameIcon::soul_of_npc, "Soul of the Bomb Shop Owner", { MM_SOUL_NPC_BOMBCHU_SHOPKEEPER, SHARED_SOUL_NPC_BOMBCHU_SHOPKEEPER }, false },
    { EGameIcon::soul_of_npc, "Soul of Bombers", { MM_SOUL_NPC_BOMBERS, SHARED_SOUL_NPC_BOMBERS }, false },
    { EGameIcon::soul_of_npc, "Soul of the Deku Butler", { MM_SOUL_NPC_BUTLER_DEKU }, false },
    { EGameIcon::soul_of_npc, "Soul of Carpenters", { MM_SOUL_NPC_CARPENTERS, SHARED_SOUL_NPC_CARPENTERS }, false },
    { EGameIcon::soul_of_npc, "Soul of Swordsman", { MM_SOUL_NPC_CARPET_MAN, SHARED_SOUL_NPC_CARPET_MAN }, false },
    { EGameIcon::soul_of_npc, "Soul of the Fisherman", { MM_SOUL_NPC_CHEST_GAME_OWNER, SHARED_SOUL_NPC_CHEST_GAME_OWNER }, false },
    { EGameIcon::soul_of_npc, "Soul of Citizens", { MM_SOUL_NPC_CITIZEN, SHARED_SOUL_NPC_CITIZEN }, false },
    { EGameIcon::soul_of_npc, "Soul of Composer Bros.", { MM_SOUL_NPC_COMPOSER_BROS, SHARED_SOUL_NPC_COMPOSER_BROS }, false },
    { EGameIcon::soul_of_npc, "Soul of Dampe", { MM_SOUL_NPC_DAMPE, SHARED_SOUL_NPC_DAMPE }, false },
    { EGameIcon::soul_of_npc, "Soul of the Deku King", { MM_SOUL_NPC_DEKU_KING }, false },
    { EGameIcon::soul_of_npc, "Soul of the Deku Princess", { MM_SOUL_NPC_DEKU_PRINCESS }, false },
    { EGameIcon::soul_of_npc, "Soul of Dog Lady", { MM_SOUL_NPC_DOG_LADY, SHARED_SOUL_NPC_DOG_LADY }, false },
    { EGameIcon::soul_of_npc, "Soul of Trading Post Owner", { MM_SOUL_NPC_FISHING_POND_OWNER, SHARED_SOUL_NPC_FISHING_POND_OWNER }, false },
    { EGameIcon::soul_of_npc, "Soul of Gorman & Bros.", { MM_SOUL_NPC_GORMAN, SHARED_SOUL_NPC_GORMAN }, false },
    { EGameIcon::soul_of_npc, "Soul of Gorons", { MM_SOUL_NPC_GORON, SHARED_SOUL_NPC_GORON }, false },
    { EGameIcon::soul_of_npc, "Soul of the Goron Baby", { MM_SOUL_NPC_GORON_CHILD, SHARED_SOUL_NPC_GORON_CHILD }, false },
    { EGameIcon::soul_of_npc, "Soul of the Goron Elder", { MM_SOUL_NPC_GORON_ELDER }, false },
    { EGameIcon::soul_of_npc, "Soul of the Goron Shopkeeper", { MM_SOUL_NPC_GORON_SHOPKEEPER, SHARED_SOUL_NPC_GORON_SHOPKEEPER }, false },
    { EGameIcon::soul_of_npc, "Soul of Grog", { MM_SOUL_NPC_GROG, SHARED_SOUL_NPC_GROG }, false },
    { EGameIcon::soul_of_npc, "Soul of Guru-Guru", { MM_SOUL_NPC_GURU_GURU, SHARED_SOUL_NPC_GURU_GURU }, false },
    { EGameIcon::soul_of_npc, "Soul of Honey & Darling", { MM_SOUL_NPC_HONEY_DARLING, SHARED_SOUL_NPC_HONEY_DARLING }, false },
    { EGameIcon::soul_of_npc, "Soul of Kafei", { MM_SOUL_NPC_KAFEI }, false },
    { EGameIcon::soul_of_npc, "Soul of Keaton", { MM_SOUL_NPC_KEATON }, false },
    { EGameIcon::soul_of_npc, "Soul of Koume and Kotake", { MM_SOUL_NPC_KOUME_KOTAKE }, false },
    { EGameIcon::soul_of_npc, "Soul of Romani/Cremia", { MM_SOUL_NPC_MALON, SHARED_SOUL_NPC_MALON }, false },
    { EGameIcon::soul_of_npc, "Soul of Mayor Dotour", { MM_SOUL_NPC_MAYOR_DOTOUR }, false },
    { EGameIcon::soul_of_npc, "Soul of Keg Trial Goron", { MM_SOUL_NPC_MEDIGORON, SHARED_SOUL_NPC_MEDIGORON }, false },
    { EGameIcon::soul_of_npc, "Soul of Moon Children", { MM_SOUL_NPC_MOON_CHILDREN }, false },
    { EGameIcon::soul_of_npc, "Soul of Anju's Grandmother", { MM_SOUL_NPC_OLD_HAG, SHARED_SOUL_NPC_OLD_HAG }, false },
    { EGameIcon::soul_of_npc, "Soul of Playground Scrubs", { MM_SOUL_NPC_PLAYGROUND_SCRUBS }, false },
    { EGameIcon::soul_of_npc, "Soul of Ghost Hut Owner", { MM_SOUL_NPC_POE_COLLECTOR, SHARED_SOUL_NPC_POE_COLLECTOR }, false },
    { EGameIcon::soul_of_npc, "Soul of Part-Timer", { MM_SOUL_NPC_ROOFTOP_MAN, SHARED_SOUL_NPC_ROOFTOP_MAN }, false },
    { EGameIcon::soul_of_npc, "Soul of Lulu", { MM_SOUL_NPC_RUTO, SHARED_SOUL_NPC_RUTO }, false },
    { EGameIcon::soul_of_npc, "Soul of the Scientist", { MM_SOUL_NPC_SCIENTIST, SHARED_SOUL_NPC_SCIENTIST }, false },
    { EGameIcon::soul_of_npc, "Soul of Town Archery Owner", { MM_SOUL_NPC_SHOOTING_GALLERY_OWNER, SHARED_SOUL_NPC_SHOOTING_GALLERY_OWNER }, false },
    { EGameIcon::soul_of_npc, "Soul of Mr. Barten", { MM_SOUL_NPC_TALON, SHARED_SOUL_NPC_TALON }, false },
    { EGameIcon::soul_of_npc, "Soul of the Patrolling Pirates and their Chief", { MM_SOUL_NPC_THIEVES, SHARED_SOUL_NPC_THIEVES }, false },
    { EGameIcon::soul_of_npc, "Soul of Tingle", { MM_SOUL_NPC_TINGLE }, false },
    { EGameIcon::soul_of_npc, "Soul of Toilet Hand", { MM_SOUL_NPC_TOILET_HAND }, false },
    { EGameIcon::soul_of_npc, "Soul of Toto(MM)", { MM_SOUL_NPC_TOTO }, false },
    { EGameIcon::soul_of_npc, "Soul of Tourist Center Owner", { MM_SOUL_NPC_TOURIST_CENTER }, false },
    { EGameIcon::soul_of_npc, "Soul of Zoras", { MM_SOUL_NPC_ZORA, SHARED_SOUL_NPC_ZORA }, false },
    { EGameIcon::soul_of_npc, "Soul of the Zora Musicians", { MM_SOUL_NPC_ZORA_MUSICIANS }, false },
    { EGameIcon::soul_of_npc, "Soul of the Zora Shopkeeper", { MM_SOUL_NPC_ZORA_SHOPKEEPER, SHARED_SOUL_NPC_ZORA_SHOPKEEPER }, false },
};

const ProgEntry SoulsMMAnimals[] =
{
    { EGameIcon::soul_of_animal, "Soul of Butterflies", { MM_SOUL_ANIMAL_BUTTERFLY, SHARED_SOUL_ANIMAL_BUTTERFLY }, false },
    { EGameIcon::soul_of_animal, "Soul of Cows", { MM_SOUL_ANIMAL_COW, SHARED_SOUL_ANIMAL_COW }, false },
    { EGameIcon::soul_of_animal, "Soul of Cuccos", { MM_SOUL_ANIMAL_CUCCO, SHARED_SOUL_ANIMAL_CUCCO }, false },
    { EGameIcon::soul_of_animal, "Soul of Dogs", { MM_SOUL_ANIMAL_DOG, SHARED_SOUL_ANIMAL_DOG }, false },
};

const ProgSection SoulsSections[NumSoulsSections] =
{
    MAKE_SECTION("OoT Boss Souls",   SoulsOoTBoss),
    MAKE_SECTION("OoT Enemy Souls",  SoulsOoTEnemies),
    MAKE_SECTION("OoT NPC Souls",  SoulsOoTNPCs),
    MAKE_SECTION("OoT Animal Souls",  SoulsOoTAnimals),
    MAKE_SECTION("MM Boss Souls",    SoulsMMBoss),
    MAKE_SECTION("MM Enemy Souls",  SoulsMMEnemies),
    MAKE_SECTION("MM NPC Souls",  SoulsMMNPCs),
    MAKE_SECTION("MM Animal Souls",  SoulsMMAnimals),
};

#pragma endregion Souls

#pragma region Collectibles

const ProgEntry OoTItmes[] =
{
    { EGameIcon::stick, "Deku Stick", { OOT_STICK, OOT_STICKS_5, OOT_STICKS_10, SHARED_STICK, SHARED_STICKS_5, SHARED_STICKS_10 }, true },
    { EGameIcon::nut, "Deku Nuts", { OOT_NUTS_5, OOT_NUTS_5_ALT, OOT_NUTS_10, SHARED_NUT, SHARED_NUTS_5, SHARED_NUTS_10 }, true },
    { EGameIcon::bomb, "Bombs", { OOT_BOMB, OOT_BOMBS_5, OOT_BOMBS_10, OOT_BOMBS_20, OOT_BOMBS_30, SHARED_BOMB, SHARED_BOMBS_5, SHARED_BOMBS_10, SHARED_BOMBS_20, SHARED_BOMBS_30 }, true },
    { EGameIcon::bombchu,       "Bombchus",              { OOT_BOMBCHU_5, OOT_BOMBCHU_10, OOT_BOMBCHU_20, SHARED_BOMBCHU, SHARED_BOMBCHU_5, SHARED_BOMBCHU_10, SHARED_BOMBCHU_20 },          true  },
    { EGameIcon::seeds, "Deku Seeds", { OOT_DEKU_SEEDS_5, OOT_DEKU_SEEDS_30 }, true },
    { EGameIcon::arrow, "Arrows", { OOT_ARROWS_5, OOT_ARROWS_10, OOT_ARROWS_30, SHARED_ARROWS_5, SHARED_ARROWS_10, SHARED_ARROWS_30, SHARED_ARROWS_40 }, true },
    { EGameIcon::heart, "Recovery Heart", { OOT_RECOVERY_HEART, SHARED_RECOVERY_HEART }, true },
    { EGameIcon::magic, "Small Magic Jar", { OOT_MAGIC_JAR_SMALL, SHARED_MAGIC_JAR_SMALL }, true },
    { EGameIcon::double_magic, "Large Magic Jar", { OOT_MAGIC_JAR_LARGE, SHARED_MAGIC_JAR_LARGE }, true },
    { EGameIcon::carp_fish, "Child Fish", { OOT_FISHING_POND_CHILD_FISH_2LBS, OOT_FISHING_POND_CHILD_FISH_3LBS, OOT_FISHING_POND_CHILD_FISH_4LBS, OOT_FISHING_POND_CHILD_FISH_5LBS, OOT_FISHING_POND_CHILD_FISH_6LBS, OOT_FISHING_POND_CHILD_FISH_7LBS, OOT_FISHING_POND_CHILD_FISH_8LBS, OOT_FISHING_POND_CHILD_FISH_9LBS, OOT_FISHING_POND_CHILD_FISH_10LBS, OOT_FISHING_POND_CHILD_FISH_11LBS, OOT_FISHING_POND_CHILD_FISH_12LBS, OOT_FISHING_POND_CHILD_FISH_13LBS, OOT_FISHING_POND_CHILD_FISH_14LBS }, true },
    { EGameIcon::carp_fish, "Child Loach", { OOT_FISHING_POND_CHILD_LOACH_14LBS, OOT_FISHING_POND_CHILD_LOACH_15LBS, OOT_FISHING_POND_CHILD_LOACH_16LBS, OOT_FISHING_POND_CHILD_LOACH_17LBS, OOT_FISHING_POND_CHILD_LOACH_18LBS, OOT_FISHING_POND_CHILD_LOACH_19LBS }, true },
    { EGameIcon::carp_fish, "Adult Fish", { OOT_FISHING_POND_ADULT_FISH_4LBS, OOT_FISHING_POND_ADULT_FISH_5LBS, OOT_FISHING_POND_ADULT_FISH_6LBS, OOT_FISHING_POND_ADULT_FISH_7LBS, OOT_FISHING_POND_ADULT_FISH_8LBS, OOT_FISHING_POND_ADULT_FISH_9LBS, OOT_FISHING_POND_ADULT_FISH_10LBS, OOT_FISHING_POND_ADULT_FISH_11LBS, OOT_FISHING_POND_ADULT_FISH_12LBS, OOT_FISHING_POND_ADULT_FISH_13LBS, OOT_FISHING_POND_ADULT_FISH_14LBS, OOT_FISHING_POND_ADULT_FISH_15LBS, OOT_FISHING_POND_ADULT_FISH_16LBS, OOT_FISHING_POND_ADULT_FISH_17LBS, OOT_FISHING_POND_ADULT_FISH_18LBS, OOT_FISHING_POND_ADULT_FISH_19LBS, OOT_FISHING_POND_ADULT_FISH_20LBS, OOT_FISHING_POND_ADULT_FISH_21LBS, OOT_FISHING_POND_ADULT_FISH_22LBS, OOT_FISHING_POND_ADULT_FISH_23LBS, OOT_FISHING_POND_ADULT_FISH_24LBS, OOT_FISHING_POND_ADULT_FISH_25LBS }, true },
    { EGameIcon::carp_fish, "Adult Loach", { OOT_FISHING_POND_ADULT_LOACH_29LBS, OOT_FISHING_POND_ADULT_LOACH_30LBS, OOT_FISHING_POND_ADULT_LOACH_31LBS, OOT_FISHING_POND_ADULT_LOACH_32LBS, OOT_FISHING_POND_ADULT_LOACH_33LBS, OOT_FISHING_POND_ADULT_LOACH_34LBS, OOT_FISHING_POND_ADULT_LOACH_35LBS, OOT_FISHING_POND_ADULT_LOACH_36LBS }, true },
    { EGameIcon::fairy, "Big Fairy", { OOT_FAIRY_BIG, MM_FAIRY_BIG, SHARED_FAIRY_BIG }, true },
};

const ProgEntry MMItems[] =
{
    { EGameIcon::stick, "Deku Sticks", { MM_STICK, MM_UNK_2F, MM_UNK_30, MM_UNK_31, SHARED_STICK, SHARED_STICKS_5, SHARED_STICKS_10 }, true },
    { EGameIcon::nut, "Nuts", { MM_NUT, MM_NUTS_5, MM_NUTS_10, MM_UNK_2B, MM_UNK_2C, MM_UNK_2D, SHARED_NUT, SHARED_NUTS_5, SHARED_NUTS_10 }, true },
    { EGameIcon::bomb, "Bombs", { MM_BOMB, MM_BOMBS_5, MM_BOMBS_10, MM_BOMBS_20, MM_BOMBS_30, SHARED_BOMB, SHARED_BOMBS_5, SHARED_BOMBS_10, SHARED_BOMBS_20, SHARED_BOMBS_30 }, true },
    { EGameIcon::bombchu, "Bombchus", { MM_BOMBCHU, MM_BOMBCHU_5, MM_BOMBCHU_10, MM_BOMBCHU_20, SHARED_BOMBCHU, SHARED_BOMBCHU_5, SHARED_BOMBCHU_10, SHARED_BOMBCHU_20 },            true  },
    { EGameIcon::arrow,   "Arrows", { MM_ARROWS_10, MM_ARROWS_30, MM_ARROWS_40, MM_UNK_21, SHARED_ARROWS_5, SHARED_ARROWS_10, SHARED_ARROWS_30, SHARED_ARROWS_40 }, true },
    { EGameIcon::heart, "Recovery Heart", { MM_RECOVERY_HEART, SHARED_RECOVERY_HEART }, true },
    { EGameIcon::magic, "Small Magic Jar", { MM_MAGIC_JAR_SMALL, SHARED_MAGIC_JAR_SMALL }, true },
    { EGameIcon::double_magic, "Large Magic Jar", { MM_MAGIC_JAR_LARGE, SHARED_MAGIC_JAR_LARGE }, true },

};

const ProgEntry Bottles[] =
{
    { EGameIcon::red_potion, "Red Potion", { OOT_POTION_RED, OOT_BOTTLE_POTION_RED, MM_POTION_RED, MM_BOTTLE_POTION_RED, SHARED_POTION_RED, SHARED_BOTTLE_POTION_RED }, true },
    { EGameIcon::green_potion, "Green Potion", { OOT_POTION_GREEN, OOT_BOTTLE_POTION_GREEN, MM_POTION_GREEN, MM_BOTTLE_POTION_GREEN, SHARED_POTION_GREEN, SHARED_BOTTLE_POTION_GREEN }, true },
    { EGameIcon::blue_potion, "Blue Potion", { OOT_POTION_BLUE, OOT_BOTTLE_POTION_BLUE, MM_POTION_BLUE, MM_BOTTLE_POTION_BLUE, SHARED_POTION_BLUE, SHARED_BOTTLE_POTION_BLUE }, true },
    { EGameIcon::milk, "Milk", { OOT_MILK, OOT_BOTTLE_MILK, MM_MILK, MM_BOTTLE_MILK, SHARED_MILK, SHARED_BOTTLE_MILK }, true },
    { EGameIcon::chateau, "Chateau Romani", { MM_CHATEAU, MM_BOTTLE_CHATEAU, OOT_CHATEAU, OOT_BOTTLE_CHATEAU, SHARED_CHATEAU, SHARED_BOTTLE_CHATEAU }, true },
    { EGameIcon::blue_fire, "Blue Fire", { OOT_BLUE_FIRE, OOT_BOTTLE_BLUE_FIRE, MM_BLUE_FIRE, MM_BOTTLE_BLUE_FIRE, SHARED_BLUE_FIRE, SHARED_BOTTLE_BLUE_FIRE }, true },
    { EGameIcon::bottled_poe, "Poe", { OOT_POE, OOT_BOTTLE_POE, MM_POE, MM_BOTTLE_POE, SHARED_POE, SHARED_BOTTLE_POE }, true },
    { EGameIcon::bottled_big_poe, "Big Poe", { OOT_BIG_POE, OOT_BOTTLE_BIG_POE, MM_BIG_POE, MM_BOTTLE_BIG_POE, SHARED_BIG_POE, SHARED_BOTTLE_BIG_POE }, true },
    { EGameIcon::bottled_fairy, "Fairy", { OOT_FAIRY, OOT_BOTTLE_FAIRY, MM_FAIRY, MM_BOTTLE_FAIRY, SHARED_FAIRY, SHARED_BOTTLE_FAIRY }, true },
    { EGameIcon::bottled_bugs, "Bugs", { OOT_BUGS, MM_BUGS, SHARED_BUGS }, true },
    { EGameIcon::bottled_fish, "Fish", { OOT_FISH, MM_FISH, SHARED_FISH }, true },
    { EGameIcon::dust, "Gold Dust", { MM_GOLD_DUST, MM_BOTTLED_GOLD_DUST, OOT_GOLD_DUST, OOT_BOTTLED_GOLD_DUST, SHARED_BOTTLED_GOLD_DUST }, true },
};

const ProgEntry Rupees[] =
{
    { EGameIcon::green_coin, "Coin (Green)", { OOT_COIN_GREEN }, true },
    { EGameIcon::red_coin, "Coin (Red)", { OOT_COIN_RED }, true },
    { EGameIcon::blue_coin, "Coin (Blue)", { OOT_COIN_BLUE }, true },
    { EGameIcon::yellow_coin, "Coin (Yellow)", { OOT_COIN_YELLOW }, true },
    { EGameIcon::rupee,          "Green Rupee",          { OOT_RUPEE_GREEN, MM_RUPEE_GREEN, SHARED_RUPEE_GREEN },    true },
    { EGameIcon::blue_rupee,     "Blue Rupee",           { OOT_RUPEE_BLUE, OOT_TC_RUPEE_BLUE, MM_RUPEE_BLUE, SHARED_RUPEE_BLUE },     true },
    { EGameIcon::red_rupee,      "Red Rupee",            { OOT_RUPEE_RED, OOT_TC_RUPEE_RED, MM_RUPEE_RED, MM_RUPEE_RED_10, SHARED_RUPEE_RED },      true },
    { EGameIcon::purple_rupee,   "Purple Rupee",         { OOT_RUPEE_PURPLE, OOT_TC_RUPEE_PURPLE, MM_RUPEE_PURPLE, SHARED_RUPEE_PURPLE },   true },
    { EGameIcon::sr,             "Silver Rupee",         { MM_RUPEE_SILVER, SHARED_RUPEE_SILVER },     true },
    { EGameIcon::gold_rupee,     "Huge Rupee",           { OOT_RUPEE_HUGE, MM_RUPEE_GOLD, SHARED_RUPEE_GOLD },     true },
    { EGameIcon::rainbow_rupee,  "Rainbow Rupee",        { OOT_RUPEE_RAINBOW, SHARED_RUPEE_RAINBOW },     true },
};

const ProgEntry Traps[] =
{
    { EGameIcon::fire_trap, "Fire Trap", { OOT_TRAP_FIRE }, true },
    { EGameIcon::ice_trap, "Ice Trap", { OOT_TRAP_ICE, OOT_UNUSED_ICE_TRAP }, true },
    { EGameIcon::shock_trap, "Shock Trap", { OOT_TRAP_SHOCK }, true },
    { EGameIcon::knockback_trap, "Knockback Trap", { OOT_TRAP_KNOCKBACK }, true },
    { EGameIcon::magic_trap, "Anti-Magic Trap", { OOT_TRAP_ANTI_MAGIC }, true },
    { EGameIcon::drain_trap, "Drain Trap", { OOT_TRAP_DRAIN }, true },
    { EGameIcon::rupoor, "Rupoor", { OOT_TRAP_RUPOOR }, true },
    { EGameIcon::nothing, "Nothing", { NOTHING }, true },
};

const ProgSection CollectiblesSections[NumCollectiblesSections] =
{
    MAKE_SECTION("OoT Items",   OoTItmes),
    MAKE_SECTION("MM Items",    MMItems),
    MAKE_SECTION("Bottles",  Bottles),
    MAKE_SECTION("Rupees",  Rupees),
    MAKE_SECTION("Traps",  Traps),
};

#pragma endregion Collectibles
