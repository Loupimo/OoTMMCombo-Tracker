#include "UI/ProgressionEntry.h"
#include "Combo/Items.h"

// Helper to compute the static count of an entries array at compile-time.
#define MAKE_SECTION(Title, Arr) { Title, Arr, sizeof(Arr) / sizeof(Arr[0]) }

#pragma region OoT Page

const ProgEntry OoTWeaponsTools[] =
{
    { EGameIcon::kokiri,        "Kokiri Sword",         { OOT_SWORD_KOKIRI },     false},
    { EGameIcon::master,        "Master Sword",         { OOT_SWORD_MASTER },     false },
    { EGameIcon::biggoron,      "Biggoron's Sword",     { OOT_SWORD_BIGGORON, OOT_SWORD_KNIFE },         false },
    { EGameIcon::bow_oot,       "Fairy Bow",            { OOT_BOW },        false },
    { EGameIcon::fire_arrow,    "Fire Arrows",          { OOT_ARROW_FIRE },       false },
    { EGameIcon::ice_arrow,     "Ice Arrows",           { OOT_ARROW_ICE },        false },
    { EGameIcon::light_arrow,   "Light Arrows",         { OOT_ARROW_LIGHT },      false },
    { EGameIcon::slingshot,     "Fairy Slingshot",      { OOT_SLINGSHOT },        false },
    { EGameIcon::boomrang,      "Boomerang",            { OOT_BOOMERANG },        false },
    { EGameIcon::hookshot_oot,  "Hookshot",             { OOT_HOOKSHOT },         false },
    { EGameIcon::hammer,        "Megaton Hammer",       { OOT_HAMMER },           false },
    { EGameIcon::bombchu,       "Bombchu",              { OOT_BOMBCHU_5, OOT_BOMBCHU_10, OOT_BOMBCHU_20 },          true  },
    { EGameIcon::lens,          "Lens of Truth",        { OOT_LENS },             false },
    { EGameIcon::bean,          "Magic Beans",          { OOT_MAGIC_BEAN },       false },
    { EGameIcon::fairy_ocarina, "Fairy Ocarina",        { OOT_OCARINA_FAIRY },    false },
    { EGameIcon::ocarina,       "Ocarina of Time",      { OOT_OCARINA_TIME },  false },
    { EGameIcon::bottle,        "Empty Bottle",         { OOT_BOTTLE_EMPTY },     true  },
    { EGameIcon::din,           "Din's Fire",           { OOT_SPELL_FIRE },              false },
    { EGameIcon::farore,        "Farore's Wind",        { OOT_SPELL_WIND },           false },
    { EGameIcon::nayru,         "Nayru's Love",         { OOT_SPELL_LOVE },            false },
};

const ProgEntry OoTEquipment[] =
{
    { EGameIcon::deku_shield,        "Deku Shield",      { OOT_SHIELD_DEKU, OOT_PROGRESSIVE_SHIELD_DEKU},      false},
    { EGameIcon::hylian_shield,      "Hylian Shield",    { OOT_SHIELD_HYLIAN, OOT_PROGRESSIVE_SHIELD_HYLIAN },    false },
    { EGameIcon::mirror_oot,         "Mirror Shield",    { OOT_SHIELD_MIRROR },    false },
    { EGameIcon::goron_tunic,        "Goron Tunic",      { OOT_TUNIC_GORON },      false },
    { EGameIcon::zora_tunic,         "Zora Tunic",       { OOT_TUNIC_ZORA },       false },
    { EGameIcon::iron,               "Iron Boots",       { OOT_BOOTS_IRON },       false },
    { EGameIcon::hover,              "Hover Boots",      { OOT_BOOTS_HOVER },      false },
    { EGameIcon::silver_gauntlet,    "Silver Gauntlets", { OOT_SILVER_GAUNTLETS },  false },
    { EGameIcon::golden_gauntlet,    "Golden Gauntlets", { OOT_GOLDEN_GAUNTLETS },  false },
    { EGameIcon::bronze_scale,       "Bronze Scale",     { OOT_SCALE_BRONZE },     false },
    { EGameIcon::silver,             "Silver Scale",     { OOT_SCALE_SILVER },     false },
    { EGameIcon::golden,             "Golden Scale",     { OOT_SCALE_GOLDEN },     false },
    { EGameIcon::bomb_bag,           "Bomb Bag",         { OOT_BOMB_BAG },         false },
    { EGameIcon::big_bomb,           "Big Bomb Bag",     { OOT_BOMB_BAG2 },         false },
    { EGameIcon::biggest_bomb,       "Biggest Bomb Bag", { OOT_BOMB_BAG3 },     false },
    { EGameIcon::big_quiver,         "Big Quiver",       { OOT_QUIVER2 },       false },
    { EGameIcon::biggest_quiver,     "Biggest Quiver",   { OOT_QUIVER3 },   false },
    { EGameIcon::magic_upgrade,      "Magic Upgrade",    { OOT_MAGIC_UPGRADE },    false },
    { EGameIcon::large_magic_upgrade,"Double Magic",     { OOT_MAGIC_UPGRADE2 },     false },
    { EGameIcon::defense,            "Double Defense",   { OOT_DEFENSE_UPGRADE },   false },
};

const ProgEntry OoTStonesMedallions[] =
{
    { EGameIcon::emerald,   "Kokiri's Emerald",  { OOT_STONE_EMERALD },          false },
    { EGameIcon::ruby,      "Goron's Ruby",      { OOT_STONE_RUBY },             false },
    { EGameIcon::sapphire,  "Zora's Sapphire",   { OOT_STONE_SAPPHIRE },         false },
    { EGameIcon::forest,    "Forest Medallion",  { OOT_MEDALLION_FOREST }, false },
    { EGameIcon::fire,      "Fire Medallion",    { OOT_MEDALLION_FIRE },   false },
    { EGameIcon::water,     "Water Medallion",   { OOT_MEDALLION_WATER },  false },
    { EGameIcon::shadow,    "Shadow Medallion",  { OOT_MEDALLION_SHADOW }, false },
    { EGameIcon::spirit,    "Spirit Medallion",  { OOT_MEDALLION_SPIRIT }, false },
    { EGameIcon::light,     "Light Medallion",   { OOT_MEDALLION_LIGHT },  false },
};

const ProgEntry OoTSongs[] =
{
    { EGameIcon::song,        "Zelda's Lullaby",     { OOT_SONG_ZELDA },         false },
    { EGameIcon::song,        "Epona's Song",        { OOT_SONG_EPONA, OOT_SONG_NOTE_EPONA },           false },
    { EGameIcon::song,        "Saria's Song",        { OOT_SONG_SARIA, OOT_SONG_NOTE_SARIA },           false },
    { EGameIcon::song,        "Sun's Song",          { OOT_SONG_SUN, OOT_SONG_NOTE_SUN },      false },
    { EGameIcon::song,        "Song of Time",        { OOT_SONG_TIME, OOT_SONG_NOTE_TIME },    false },
    { EGameIcon::song,        "Song of Storms",      { OOT_SONG_STORMS, OOT_SONG_NOTE_STORMS },  false },
    { EGameIcon::song,        "Elegy of Emptiness",  { OOT_SONG_EMPTINESS, OOT_SONG_NOTE_EMPTINESS },  false },
    { EGameIcon::song_green,  "Minuet of Forest",    { OOT_SONG_TP_FOREST, OOT_SONG_NOTE_TP_FOREST },          false },
    { EGameIcon::song_red,    "Bolero of Fire",      { OOT_SONG_TP_FIRE, OOT_SONG_NOTE_TP_FIRE },          false },
    { EGameIcon::song_blue,   "Serenade of Water",   { OOT_SONG_TP_WATER, OOT_SONG_NOTE_TP_FIRE },        false },
    { EGameIcon::song_purple, "Nocturne of Shadow",  { OOT_SONG_TP_SHADOW, OOT_SONG_NOTE_TP_FIRE },        false },
    { EGameIcon::song_orange, "Requiem of Spirit",   { OOT_SONG_TP_SPIRIT, OOT_SONG_NOTE_TP_FIRE },         false },
    { EGameIcon::song_yellow, "Prelude of Light",    { OOT_SONG_TP_LIGHT, OOT_SONG_NOTE_TP_FIRE },         false },
};

const ProgEntry OoTCounters[] =
{
    { EGameIcon::gs_token,       "Gold Skulltula Token", { OOT_GS_TOKEN }, true },
    { EGameIcon::triforce_piece, "Triforce Piece",       { OOT_TRIFORCE, OOT_TRIFORCE_FULL }, true },
    { EGameIcon::triforce_courage, "Triforce of Courage",       { OOT_TRIFORCE_COURAGE }, false },
    { EGameIcon::triforce_power,   "Triforce of Power",       { OOT_TRIFORCE_POWER }, false },
    { EGameIcon::triforce_wisdom,  "Triforce of Wisdom",       { OOT_TRIFORCE_WISDOM }, false },
    { EGameIcon::small_key,      "Small Key (Bottom of the Well)",            { OOT_SMALL_KEY_BOTW },      true },
    { EGameIcon::small_key,      "Small Key (Forest Temple)",            { OOT_SMALL_KEY_FOREST },      true },
    { EGameIcon::small_key,      "Small Key (Fire Temple)",            { OOT_SMALL_KEY_FIRE },      true },
    { EGameIcon::small_key,      "Small Key (Water Temple)",            { OOT_SMALL_KEY_WATER },      true },
    { EGameIcon::small_key,      "Small Key (Shadow Temple)",            { OOT_SMALL_KEY_SHADOW },      true },
    { EGameIcon::small_key,      "Small Key (Spirit Temple)",            { OOT_SMALL_KEY_SPIRIT },      true },
    { EGameIcon::small_key,      "Small Key (Gerudo's Training Ground)",            { OOT_SMALL_KEY_GTG },      true },
    { EGameIcon::small_key,      "Small Key (Gerudo's Fortress)",            { OOT_SMALL_KEY_GF },      true },
    { EGameIcon::small_key,      "Small Key (Ganon's Castle)",            { OOT_SMALL_KEY_GANON },      true },
    { EGameIcon::small_key,      "Small Key (Chest Game)",            { OOT_SMALL_KEY_TCG },      true },
    { EGameIcon::boss_key,       "Boss Key (Forest Temple)",             { OOT_BOSS_KEY_FOREST },       true },
    { EGameIcon::boss_key,       "Boss Key (Fire Temple)",             { OOT_BOSS_KEY_FIRE },       true },
    { EGameIcon::boss_key,       "Boss Key (Water Temple)",             { OOT_BOSS_KEY_WATER },       true },
    { EGameIcon::boss_key,       "Boss Key (Shadow Temple)",             { OOT_BOSS_KEY_SHADOW },       true },
    { EGameIcon::boss_key,       "Boss Key (Spirit Temple)",             { OOT_BOSS_KEY_SPIRIT },       true },
    { EGameIcon::boss_key,       "Boss Key (Ganon 's Castle)",             { OOT_BOSS_KEY_GANON },       true },
    { EGameIcon::rupee,          "Green Rupee",          { OOT_RUPEE_GREEN },    true },
    { EGameIcon::blue_rupee,     "Blue Rupee",           { OOT_RUPEE_BLUE, OOT_TC_RUPEE_BLUE },     true },
    { EGameIcon::red_rupee,      "Red Rupee",            { OOT_RUPEE_RED, OOT_TC_RUPEE_RED },      true },
    { EGameIcon::purple_rupee,   "Purple Rupee",         { OOT_RUPEE_PURPLE, OOT_TC_RUPEE_PURPLE },   true },
    { EGameIcon::gold_rupee,     "Huge Rupee",           { OOT_RUPEE_HUGE },     true },
};

const ProgSection OoTSections[NumOoTSections] =
{
    MAKE_SECTION("Weapons & Tools",     OoTWeaponsTools),
    MAKE_SECTION("Equipment",           OoTEquipment),
    MAKE_SECTION("Stones & Medallions", OoTStonesMedallions),
    MAKE_SECTION("Songs",               OoTSongs),
    MAKE_SECTION("Counters",            OoTCounters),
};

#pragma endregion OoT Page

#pragma region MM Page

const ProgEntry MMTransformationMasks[] =
{
    { EGameIcon::deku,   "Deku Mask",            { MM_MASK_DEKU },          false },
    { EGameIcon::goron,  "Goron Mask",           { MM_MASK_GORON },         false },
    { EGameIcon::zora,   "Zora Mask",            { MM_MASK_ZORA },          false },
    { EGameIcon::deity,  "Fierce Deity's Mask",  { MM_MASK_FIERCE_DEITY },       false },
};

const ProgEntry MMMasks[] =
{
    { EGameIcon::bunny,      "Bunny Hood",            { MM_MASK_BUNNY },          false },
    { EGameIcon::keaton,     "Keaton Mask",           { MM_MASK_KEATON },              false },
    { EGameIcon::truth,      "Mask of Truth",         { MM_MASK_TRUTH },       false },
    { EGameIcon::postman,    "Postman's Hat",         { MM_MASK_POSTMAN },             false },
    { EGameIcon::night,      "All-Night Mask",        { MM_MASK_ALL_NIGHT },           false },
    { EGameIcon::blast,      "Blast Mask",            { MM_MASK_BLAST },          false },
    { EGameIcon::stone,      "Stone Mask",            { MM_MASK_STONE },          false },
    { EGameIcon::fairy_mask, "Great Fairy's Mask",    { MM_MASK_GREAT_FAIRY },  false },
    { EGameIcon::bremen,     "Bremen Mask",           { MM_MASK_BREMEN },              false },
    { EGameIcon::gero,       "Don Gero's Mask",       { MM_MASK_DON_GERO },            false },
    { EGameIcon::kamaro,     "Kamaro's Mask",         { MM_MASK_KAMARO },              false },
    { EGameIcon::romani,     "Romani's Mask",         { MM_MASK_ROMANI },       false },
    { EGameIcon::troupe,     "Circus Leader's Mask",  { MM_MASK_TROUPE_LEADER },       false },
    { EGameIcon::kafei,      "Kafei's Mask",          { MM_MASK_KAFEI },               false },
    { EGameIcon::couple,     "Couple's Mask",         { MM_MASK_COUPLE },              false },
    { EGameIcon::scents,     "Mask of Scents",        { MM_MASK_SCENTS },      false },
    { EGameIcon::garo,       "Garo's Mask",           { MM_MASK_GARO },                false },
    { EGameIcon::captain,    "Captain's Hat",         { MM_MASK_CAPTAIN },             false },
    { EGameIcon::gibdo,      "Gibdo Mask",            { MM_MASK_GIBDO },               false },
    { EGameIcon::giant,      "Giant's Mask",          { MM_MASK_GIANT },        false },
};

const ProgEntry MMRemains[] =
{
    { EGameIcon::odolwa,    "Odolwa's Remains",   { MM_REMAINS_ODOLWA },   false },
    { EGameIcon::goht,      "Goht's Remains",     { MM_REMAINS_GOHT },     false },
    { EGameIcon::gyorg,     "Gyorg's Remains",    { MM_REMAINS_GYORG },    false },
    { EGameIcon::twinmold,  "Twinmold's Remains", { MM_REMAINS_TWINMOLD }, false },
};

const ProgEntry MMWeaponsTools[] =
{
    { EGameIcon::bow_mm,              "Hero's Bow",          { MM_BOW },         false },
    { EGameIcon::hookshot_mm,         "Hookshot",            { MM_HOOKSHOT },           false },
    { EGameIcon::lens,                "Lens of Truth",       { MM_LENS },               false },
    { EGameIcon::picto,               "Pictograph Box",      { MM_PICTOGRAPH_BOX },         false },
    { EGameIcon::powder,              "Powder Keg",          { MM_POWDER_KEG },         false },
    { EGameIcon::fairy_sword,         "Great Fairy's Sword", { MM_GREAT_FAIRY_SWORD }, false },
    { EGameIcon::bombchu,             "Bombchu",             { MM_BOMBCHU, MM_BOMBCHU_5, MM_BOMBCHU_10, MM_BOMBCHU_20 },            true  },
    { EGameIcon::magic_upgrade,       "Magic Upgrade",       { MM_MAGIC_UPGRADE },      false },
    { EGameIcon::large_magic_upgrade, "Double Magic",        { MM_MAGIC_UPGRADE2 },       false },
};

const ProgEntry MMSongs[] =
{
    { EGameIcon::song,        "Song of Time",        { MM_SONG_TIME, MM_SONG_NOTE_TIME },    false },
    { EGameIcon::song,        "Song of Healing",     { MM_SONG_HEALING, MM_SONG_NOTE_HEALING }, false },
    { EGameIcon::song,        "Epona's Song",        { MM_SONG_EPONA, MM_SONG_NOTE_EPONA },           false },
    { EGameIcon::song,        "Song of Soaring",     { MM_SONG_SOARING, MM_SONG_NOTE_SOARING }, false },
    { EGameIcon::song,        "Song of Storms",      { MM_SONG_STORMS, MM_SONG_NOTE_STORMS },  false },
    { EGameIcon::song,        "Sun's Song",          { MM_SONG_SUN, MM_SONG_NOTE_SUN },      false },
    { EGameIcon::song_green,  "Sonata of Awakening", { MM_SONG_AWAKENING, MM_SONG_NOTE_AWAKENING },          false },
    { EGameIcon::song_red,    "Goron Lullaby",       { MM_SONG_GORON_HALF, MM_SONG_GORON, MM_SONG_NOTE_GORON },   false },
    { EGameIcon::song_blue,   "New Wave Bossa Nova", { MM_SONG_ZORA, MM_SONG_NOTE_ZORA },      false },
    { EGameIcon::song_orange, "Elegy of Emptiness",  { MM_SONG_EMPTINESS, MM_SONG_NOTE_EMPTINESS },           false },
    { EGameIcon::song_purple, "Oath to Order",       { MM_SONG_ORDER, MM_SONG_NOTE_ORDER },   false },
};

const ProgEntry MMCounters[] =
{
    { EGameIcon::sf,              "Stray Fairy (Woodfall)",      { MM_STRAY_FAIRY_WF },     true },
    { EGameIcon::sf_green,        "Stray Fairy (Snowhead)",      { MM_STRAY_FAIRY_SH },     true },
    { EGameIcon::sf_blue,         "Stray Fairy (Great Bay)",     { MM_STRAY_FAIRY_GB },    true },
    { EGameIcon::sf_yellow,       "Stray Fairy (Stone Tower)",   { MM_STRAY_FAIRY_ST },  true },
    { EGameIcon::sf_orange,       "Stray Fairy (Clock Town)",    { MM_STRAY_FAIRY_TOWN },          true },
    { EGameIcon::sf_transcendent, "Transcendent Fairy",          { MM_TRANSCENDENT_FAIRY },          true },
    { EGameIcon::swamp_token,     "Swamp Skulltula Token",       { MM_GS_TOKEN_SWAMP },     true },
    { EGameIcon::ocean_token,     "Ocean Skulltula Token",       { MM_GS_TOKEN_OCEAN },     true },
};

const ProgSection MMSections[NumMMSections] =
{
    MAKE_SECTION("Transformation Masks", MMTransformationMasks),
    MAKE_SECTION("Masks",                MMMasks),
    MAKE_SECTION("Boss Remains",         MMRemains),
    MAKE_SECTION("Weapons & Tools",      MMWeaponsTools),
    MAKE_SECTION("Songs",                MMSongs),
    MAKE_SECTION("Counters",             MMCounters),
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
    { EGameIcon::soul_of_foe, "Soul of Armos", { OOT_SOUL_ENEMY_ARMOS }, false },
    { EGameIcon::soul_of_foe, "Soul of Baby Dodongos", { OOT_SOUL_ENEMY_BABY_DODONGO }, false },
    { EGameIcon::soul_of_foe, "Soul of Beamos", { OOT_SOUL_ENEMY_BEAMOS }, false },
    { EGameIcon::soul_of_foe, "Soul of Biris/Baris", { OOT_SOUL_ENEMY_BIRI_BARI }, false },
    { EGameIcon::soul_of_foe, "Soul of Bubbles", { OOT_SOUL_ENEMY_BUBBLE }, false },
    { EGameIcon::soul_of_foe, "Soul of Dark Link", { OOT_SOUL_ENEMY_DARK_LINK }, false },
    { EGameIcon::soul_of_foe, "Soul of Dead Hands", { OOT_SOUL_ENEMY_DEAD_HAND }, false },
    { EGameIcon::soul_of_foe, "Soul of Deku Babas", { OOT_SOUL_ENEMY_DEKU_BABA }, false },
    { EGameIcon::soul_of_foe, "Soul of Deku Scrubs", { OOT_SOUL_ENEMY_DEKU_SCRUB }, false },
    { EGameIcon::soul_of_foe, "Soul of Dodongos", { OOT_SOUL_ENEMY_DODONGO }, false },
    { EGameIcon::soul_of_foe, "Soul of the Fighting Gerudos", { OOT_SOUL_ENEMY_THIEVES }, false },
    { EGameIcon::soul_of_foe, "Soul of Flare Dancers", { OOT_SOUL_ENEMY_FLARE_DANCER }, false },
    { EGameIcon::soul_of_foe, "Soul of Floormasters", { OOT_SOUL_ENEMY_FLOORMASTER }, false },
    { EGameIcon::soul_of_foe, "Soul of Flying Pots", { OOT_SOUL_ENEMY_FLYING_POT }, false },
    { EGameIcon::soul_of_foe, "Soul of Freezards", { OOT_SOUL_ENEMY_FREEZARD }, false },
    { EGameIcon::soul_of_foe, "Soul of Gohma Larvaes", { OOT_SOUL_ENEMY_GOHMA_LARVA }, false },
    { EGameIcon::soul_of_foe, "Soul of Guays", { OOT_SOUL_ENEMY_GUAY }, false },
    { EGameIcon::soul_of_foe, "Soul of Iron Knuckles", { OOT_SOUL_ENEMY_IRON_KNUCKLE }, false },
    { EGameIcon::soul_of_foe, "Soul of Keese", { OOT_SOUL_ENEMY_KEESE }, false },
    { EGameIcon::soul_of_foe, "Soul of Leevers", { OOT_SOUL_ENEMY_LEEVER }, false },
    { EGameIcon::soul_of_foe, "Soul of Like Likes", { OOT_SOUL_ENEMY_LIKE_LIKE }, false },
    { EGameIcon::soul_of_foe, "Soul of Lizalfos/Dinolfos", { OOT_SOUL_ENEMY_LIZALFOS_DINOLFOS }, false },
    { EGameIcon::soul_of_foe, "Soul of Moblins", { OOT_SOUL_ENEMY_MOBLIN }, false },
    { EGameIcon::soul_of_foe, "Soul of Octoroks", { OOT_SOUL_ENEMY_OCTOROK }, false },
    { EGameIcon::soul_of_foe, "Soul of Jabu-Jabu's Parasites", { OOT_SOUL_ENEMY_PARASITE }, false },
    { EGameIcon::soul_of_foe, "Soul of Peahats", { OOT_SOUL_ENEMY_PEAHAT }, false },
    { EGameIcon::soul_of_foe, "Soul of Poes", { OOT_SOUL_ENEMY_POE }, false },
    { EGameIcon::soul_of_foe, "Soul of ReDeads/Gibdos", { OOT_SOUL_ENEMY_REDEAD_GIBDO }, false },
    { EGameIcon::soul_of_foe, "Soul of Shaboms", { OOT_SOUL_ENEMY_SHABOM }, false },
    { EGameIcon::soul_of_foe, "Soul of Shell Blades", { OOT_SOUL_ENEMY_SHELL_BLADE }, false },
    { EGameIcon::soul_of_foe, "Soul of Skull Kids", { OOT_SOUL_ENEMY_SKULL_KID }, false },
    { EGameIcon::soul_of_foe, "Soul of Skulltulas", { OOT_SOUL_ENEMY_SKULLTULA }, false },
    { EGameIcon::soul_of_foe, "Soul of Skullwalltulas", { OOT_SOUL_ENEMY_SKULLWALLTULA }, false },
    { EGameIcon::soul_of_foe, "Soul of Spikes", { OOT_SOUL_ENEMY_SPIKE }, false },
    { EGameIcon::soul_of_foe, "Soul of Stalchildren", { OOT_SOUL_ENEMY_STALCHILD }, false },
    { EGameIcon::soul_of_foe, "Soul of Stalfos", { OOT_SOUL_ENEMY_STALFOS }, false },
    { EGameIcon::soul_of_foe, "Soul of Stingers", { OOT_SOUL_ENEMY_STINGER }, false },
    { EGameIcon::soul_of_foe, "Soul of Tailpasarans", { OOT_SOUL_ENEMY_TAILPASARN }, false },
    { EGameIcon::soul_of_foe, "Soul of Tektites", { OOT_SOUL_ENEMY_TEKTITE }, false },
    { EGameIcon::soul_of_foe, "Soul of Torch Slugs", { OOT_SOUL_ENEMY_TORCH_SLUG }, false },
    { EGameIcon::soul_of_foe, "Soul of Wallmasters", { OOT_SOUL_ENEMY_WALLMASTER }, false },
    { EGameIcon::soul_of_foe, "Soul of Wolfoses", { OOT_SOUL_ENEMY_WOLFOS }, false },
};

const ProgEntry SoulsMMEnemies[] =
{
    { EGameIcon::soul_of_foe, "Soul of Armos", { MM_SOUL_ENEMY_ARMOS }, false },
    { EGameIcon::soul_of_foe, "Soul of Bad Bats", { MM_SOUL_ENEMY_BAD_BAT }, false },
    { EGameIcon::soul_of_foe, "Soul of Beamos", { MM_SOUL_ENEMY_BEAMOS }, false },
    { EGameIcon::soul_of_foe, "Soul of Bio Babas", { MM_SOUL_ENEMY_BIO_BABA }, false },
    { EGameIcon::soul_of_foe, "Soul of Boes", { MM_SOUL_ENEMY_BOE }, false },
    { EGameIcon::soul_of_foe, "Soul of Bubbles", { MM_SOUL_ENEMY_BUBBLE }, false },
    { EGameIcon::soul_of_foe, "Soul of Captain Keeta", { MM_SOUL_ENEMY_CAPTAIN_KEETA }, false },
    { EGameIcon::soul_of_foe, "Soul of Chuchus", { MM_SOUL_ENEMY_CHUCHU }, false },
    { EGameIcon::soul_of_foe, "Soul of Deep Pythons", { MM_SOUL_ENEMY_DEEP_PYTHON }, false },
    { EGameIcon::soul_of_foe, "Soul of Deku Babas", { MM_SOUL_ENEMY_DEKU_BABA }, false },
    { EGameIcon::soul_of_foe, "Soul of Deku Scrubs", { MM_SOUL_ENEMY_DEKU_SCRUB }, false },
    { EGameIcon::soul_of_foe, "Soul of Dexihands", { MM_SOUL_ENEMY_DEXIHAND }, false },
    { EGameIcon::soul_of_foe, "Soul of Dodongos", { MM_SOUL_ENEMY_DODONGO }, false },
    { EGameIcon::soul_of_foe, "Soul of Dragonflies", { MM_SOUL_ENEMY_DRAGONFLY }, false },
    { EGameIcon::soul_of_foe, "Soul of Eenoes", { MM_SOUL_ENEMY_EENO }, false },
    { EGameIcon::soul_of_foe, "Soul of Eyegores", { MM_SOUL_ENEMY_EYEGORE }, false },
    { EGameIcon::soul_of_foe, "Soul of the Fighting Pirates", { MM_SOUL_ENEMY_THIEVES }, false },
    { EGameIcon::soul_of_foe, "Soul of Floormasters", { MM_SOUL_ENEMY_FLOORMASTER }, false },
    { EGameIcon::soul_of_foe, "Soul of Flying Pots", { MM_SOUL_ENEMY_FLYING_POT }, false },
    { EGameIcon::soul_of_foe, "Soul of Freezards", { MM_SOUL_ENEMY_FREEZARD }, false },
    { EGameIcon::soul_of_foe, "Soul of Garo", { MM_SOUL_ENEMY_GARO }, false },
    { EGameIcon::soul_of_foe, "Soul of Gekkos", { MM_SOUL_ENEMY_GEKKO }, false },
    { EGameIcon::soul_of_foe, "Soul of Gomess", { MM_SOUL_ENEMY_GOMESS }, false },
    { EGameIcon::soul_of_foe, "Soul of Guays", { MM_SOUL_ENEMY_GUAY }, false },
    { EGameIcon::soul_of_foe, "Soul of Hiploops", { MM_SOUL_ENEMY_HIPLOOP }, false },
    { EGameIcon::soul_of_foe, "Soul of Iron Knuckles", { MM_SOUL_ENEMY_IRON_KNUCKLE }, false },
    { EGameIcon::soul_of_foe, "Soul of Keese", { MM_SOUL_ENEMY_KEESE }, false },
    { EGameIcon::soul_of_foe, "Soul of Leevers", { MM_SOUL_ENEMY_LEEVER }, false },
    { EGameIcon::soul_of_foe, "Soul of Like Likes", { MM_SOUL_ENEMY_LIKE_LIKE }, false },
    { EGameIcon::soul_of_foe, "Soul of Lizalfos/Dinolfos", { MM_SOUL_ENEMY_LIZALFOS_DINOLFOS }, false },
    { EGameIcon::soul_of_foe, "Soul of Nejirons", { MM_SOUL_ENEMY_NEJIRON }, false },
    { EGameIcon::soul_of_foe, "Soul of Octoroks", { MM_SOUL_ENEMY_OCTOROK }, false },
    { EGameIcon::soul_of_foe, "Soul of Peahats", { MM_SOUL_ENEMY_PEAHAT }, false },
    { EGameIcon::soul_of_foe, "Soul of Poes", { MM_SOUL_ENEMY_POE }, false },
    { EGameIcon::soul_of_foe, "Soul of Real Bombchu", { MM_SOUL_ENEMY_REAL_BOMBCHU }, false },
    { EGameIcon::soul_of_foe, "Soul of ReDeads/Gibdos", { MM_SOUL_ENEMY_REDEAD_GIBDO }, false },
    { EGameIcon::soul_of_foe, "Soul of Shell Blades", { MM_SOUL_ENEMY_SHELL_BLADE }, false },
    { EGameIcon::soul_of_foe, "Soul of Skullfish", { MM_SOUL_ENEMY_SKULLFISH }, false },
    { EGameIcon::soul_of_foe, "Soul of Skulltulas", { MM_SOUL_ENEMY_SKULLTULA }, false },
    { EGameIcon::soul_of_foe, "Soul of Skullwalltulas", { MM_SOUL_ENEMY_SKULLWALLTULA }, false },
    { EGameIcon::soul_of_foe, "Soul of Snappers", { MM_SOUL_ENEMY_SNAPPER }, false },
    { EGameIcon::soul_of_foe, "Soul of Stalchildren", { MM_SOUL_ENEMY_STALCHILD }, false },
    { EGameIcon::soul_of_foe, "Soul of Takkuri", { MM_SOUL_ENEMY_TAKKURI }, false },
    { EGameIcon::soul_of_foe, "Soul of Tektites", { MM_SOUL_ENEMY_TEKTITE }, false },
    { EGameIcon::soul_of_foe, "Soul of Wallmasters", { MM_SOUL_ENEMY_WALLMASTER }, false },
    { EGameIcon::soul_of_foe, "Soul of Warts", { MM_SOUL_ENEMY_WART }, false },
    { EGameIcon::soul_of_foe, "Soul of Wizzrobes", { MM_SOUL_ENEMY_WIZZROBE }, false },
    { EGameIcon::soul_of_foe, "Soul of Wolfos", { MM_SOUL_ENEMY_WOLFOS }, false },
};

const ProgEntry SoulsOoTNPCs[] =
{
    { EGameIcon::soul_of_npc, "Soul of Business Scrubs", { OOT_SOUL_MISC_BUSINESS_SCRUB }, false },
    { EGameIcon::soul_of_npc, "Soul of Gold Skulltulas", { OOT_SOUL_MISC_GS }, false },
    { EGameIcon::soul_of_npc, "Soul of Cucco Lady", { OOT_SOUL_NPC_ANJU }, false },
    { EGameIcon::soul_of_npc, "Soul of the Astronomer", { OOT_SOUL_NPC_ASTRONOMER }, false },
    { EGameIcon::soul_of_npc, "Soul of the Beggar", { OOT_SOUL_NPC_BANKER }, false },
    { EGameIcon::soul_of_npc, "Soul of Bazaar Shopkeeper", { OOT_SOUL_NPC_BAZAAR_SHOPKEEPER }, false },
    { EGameIcon::soul_of_npc, "Soul of the Bean Salesman", { OOT_SOUL_NPC_BEAN_SALESMAN }, false },
    { EGameIcon::soul_of_npc, "Soul of Biggoron", { OOT_SOUL_NPC_BIGGORON }, false },
    { EGameIcon::soul_of_npc, "Soul of Bombchu Bowling Lady", { OOT_SOUL_NPC_BOMBCHU_BOWLING_LADY }, false },
    { EGameIcon::soul_of_npc, "Soul of Bombchu Shopkeeper", { OOT_SOUL_NPC_BOMBCHU_SHOPKEEPER }, false },
    { EGameIcon::soul_of_npc, "Soul of Graveyard Kid", { OOT_SOUL_NPC_BOMBERS }, false },
    { EGameIcon::soul_of_npc, "Soul of Carpenters", { OOT_SOUL_NPC_CARPENTERS }, false },
    { EGameIcon::soul_of_npc, "Soul of the Carpet Man", { OOT_SOUL_NPC_CARPET_MAN }, false },
    { EGameIcon::soul_of_npc, "Soul of Chest Game Owner", { OOT_SOUL_NPC_CHEST_GAME_OWNER }, false },
    { EGameIcon::soul_of_npc, "Soul of the Citizens", { OOT_SOUL_NPC_CITIZEN }, false },
    { EGameIcon::soul_of_npc, "Soul of Composer Bros.", { OOT_SOUL_NPC_COMPOSER_BROS }, false },
    { EGameIcon::soul_of_npc, "Soul of Dampe", { OOT_SOUL_NPC_DAMPE }, false },
    { EGameIcon::soul_of_npc, "Soul of Darunia", { OOT_SOUL_NPC_DARUNIA }, false },
    { EGameIcon::soul_of_npc, "Soul of the Dog Lady", { OOT_SOUL_NPC_DOG_LADY }, false },
    { EGameIcon::soul_of_npc, "Soul of Fishing Pond Owner", { OOT_SOUL_NPC_FISHING_POND_OWNER }, false },
    { EGameIcon::soul_of_npc, "Soul of Ingo", { OOT_SOUL_NPC_GORMAN }, false },
    { EGameIcon::soul_of_npc, "Soul of Goron", { OOT_SOUL_NPC_GORON }, false },
    { EGameIcon::soul_of_npc, "Soul of Goron Child", { OOT_SOUL_NPC_GORON_CHILD }, false },
    { EGameIcon::soul_of_npc, "Soul of Goron Shopkeeper", { OOT_SOUL_NPC_GORON_SHOPKEEPER }, false },
    { EGameIcon::soul_of_npc, "Soul of the Punk Kid", { OOT_SOUL_NPC_GROG }, false },
    { EGameIcon::soul_of_npc, "Soul of Guru-Guru", { OOT_SOUL_NPC_GURU_GURU }, false },
    { EGameIcon::soul_of_npc, "Soul of Honey & Darling", { OOT_SOUL_NPC_HONEY_DARLING }, false },
    { EGameIcon::soul_of_npc, "Soul of Hylian Guard", { OOT_SOUL_NPC_HYLIAN_GUARD }, false },
    { EGameIcon::soul_of_npc, "Soul of King Zora", { OOT_SOUL_NPC_KING_ZORA }, false },
    { EGameIcon::soul_of_npc, "Soul of Kokiri", { OOT_SOUL_NPC_KOKIRI }, false },
    { EGameIcon::soul_of_npc, "Soul of Kokiri Shopkeeper", { OOT_SOUL_NPC_KOKIRI_SHOPKEEPER }, false },
    { EGameIcon::soul_of_npc, "Soul of Malon", { OOT_SOUL_NPC_MALON }, false },
    { EGameIcon::soul_of_npc, "Soul of Medigoron", { OOT_SOUL_NPC_MEDIGORON }, false },
    { EGameIcon::soul_of_npc, "Soul of Mido", { OOT_SOUL_NPC_MIDO }, false },
    { EGameIcon::soul_of_npc, "Soul of the Old Hag", { OOT_SOUL_NPC_OLD_HAG }, false },
    { EGameIcon::soul_of_npc, "Soul of Poe Collector", { OOT_SOUL_NPC_POE_COLLECTOR }, false },
    { EGameIcon::soul_of_npc, "Soul of Potion Shopkeeper", { OOT_SOUL_NPC_POTION_SHOPKEEPER }, false },
    { EGameIcon::soul_of_npc, "Soul of the Rooftop Man", { OOT_SOUL_NPC_ROOFTOP_MAN }, false },
    { EGameIcon::soul_of_npc, "Soul of Ruto", { OOT_SOUL_NPC_RUTO }, false },
    { EGameIcon::soul_of_npc, "Soul of Saria", { OOT_SOUL_NPC_SARIA }, false },
    { EGameIcon::soul_of_npc, "Soul of the Scientist", { OOT_SOUL_NPC_SCIENTIST }, false },
    { EGameIcon::soul_of_npc, "Soul of Sheik", { OOT_SOUL_NPC_SHEIK }, false },
    { EGameIcon::soul_of_npc, "Soul of Shooting Gallery Owner", { OOT_SOUL_NPC_SHOOTING_GALLERY_OWNER }, false },
    { EGameIcon::soul_of_npc, "Soul of Talon", { OOT_SOUL_NPC_TALON }, false },
    { EGameIcon::soul_of_npc, "Soul of the Patrolling Gerudos", { OOT_SOUL_NPC_THIEVES }, false },
    { EGameIcon::soul_of_npc, "Soul of Zelda", { OOT_SOUL_NPC_ZELDA }, false },
    { EGameIcon::soul_of_npc, "Soul of Zora", { OOT_SOUL_NPC_ZORA }, false },
    { EGameIcon::soul_of_npc, "Soul of Zora Shopkeeper", { OOT_SOUL_NPC_ZORA_SHOPKEEPER }, false },
};

const ProgEntry SoulsMMNPCs[] =
{
    { EGameIcon::soul_of_npc, "Soul of Business Scrubs", { MM_SOUL_MISC_BUSINESS_SCRUB }, false },
    { EGameIcon::soul_of_npc, "Soul of Gold Skulltulas", { MM_SOUL_MISC_GS }, false },
    { EGameIcon::soul_of_npc, "Soul of Anju", { MM_SOUL_NPC_ANJU }, false },
    { EGameIcon::soul_of_npc, "Soul of Madame Aroma", { MM_SOUL_NPC_AROMA }, false },
    { EGameIcon::soul_of_npc, "Soul of Astronomer", { MM_SOUL_NPC_ASTRONOMER }, false },
    { EGameIcon::soul_of_npc, "Soul of the Banker", { MM_SOUL_NPC_BANKER }, false },
    { EGameIcon::soul_of_npc, "Soul of Swamp Archery Owner", { MM_SOUL_NPC_BAZAAR_SHOPKEEPER }, false },
    { EGameIcon::soul_of_npc, "Soul of the Beans Salesman", { MM_SOUL_NPC_BEAN_SALESMAN }, false },
    { EGameIcon::soul_of_npc, "Soul of Biggoron", { MM_SOUL_NPC_BIGGORON }, false },
    { EGameIcon::soul_of_npc, "Soul of Blacksmiths", { MM_SOUL_NPC_BLACKSMITHS }, false },
    { EGameIcon::soul_of_npc, "Soul of Chest Game Lady", { MM_SOUL_NPC_BOMBCHU_BOWLING_LADY }, false },
    { EGameIcon::soul_of_npc, "Soul of the Bomb Shop Owner", { MM_SOUL_NPC_BOMBCHU_SHOPKEEPER }, false },
    { EGameIcon::soul_of_npc, "Soul of Bombers", { MM_SOUL_NPC_BOMBERS }, false },
    { EGameIcon::soul_of_npc, "Soul of the Deku Butler", { MM_SOUL_NPC_BUTLER_DEKU }, false },
    { EGameIcon::soul_of_npc, "Soul of Carpenters", { MM_SOUL_NPC_CARPENTERS }, false },
    { EGameIcon::soul_of_npc, "Soul of Swordsman", { MM_SOUL_NPC_CARPET_MAN }, false },
    { EGameIcon::soul_of_npc, "Soul of the Fisherman", { MM_SOUL_NPC_CHEST_GAME_OWNER }, false },
    { EGameIcon::soul_of_npc, "Soul of Citizens", { MM_SOUL_NPC_CITIZEN }, false },
    { EGameIcon::soul_of_npc, "Soul of Composer Bros.", { MM_SOUL_NPC_COMPOSER_BROS }, false },
    { EGameIcon::soul_of_npc, "Soul of Dampe", { MM_SOUL_NPC_DAMPE }, false },
    { EGameIcon::soul_of_npc, "Soul of the Deku King", { MM_SOUL_NPC_DEKU_KING }, false },
    { EGameIcon::soul_of_npc, "Soul of the Deku Princess", { MM_SOUL_NPC_DEKU_PRINCESS }, false },
    { EGameIcon::soul_of_npc, "Soul of Dog Lady", { MM_SOUL_NPC_DOG_LADY }, false },
    { EGameIcon::soul_of_npc, "Soul of Trading Post Owner", { MM_SOUL_NPC_FISHING_POND_OWNER }, false },
    { EGameIcon::soul_of_npc, "Soul of Gorman & Bros.", { MM_SOUL_NPC_GORMAN }, false },
    { EGameIcon::soul_of_npc, "Soul of Gorons", { MM_SOUL_NPC_GORON }, false },
    { EGameIcon::soul_of_npc, "Soul of the Goron Baby", { MM_SOUL_NPC_GORON_CHILD }, false },
    { EGameIcon::soul_of_npc, "Soul of the Goron Elder", { MM_SOUL_NPC_GORON_ELDER }, false },
    { EGameIcon::soul_of_npc, "Soul of the Goron Shopkeeper", { MM_SOUL_NPC_GORON_SHOPKEEPER }, false },
    { EGameIcon::soul_of_npc, "Soul of Grog", { MM_SOUL_NPC_GROG }, false },
    { EGameIcon::soul_of_npc, "Soul of Guru-Guru", { MM_SOUL_NPC_GURU_GURU }, false },
    { EGameIcon::soul_of_npc, "Soul of Honey & Darling", { MM_SOUL_NPC_HONEY_DARLING }, false },
    { EGameIcon::soul_of_npc, "Soul of Kafei", { MM_SOUL_NPC_KAFEI }, false },
    { EGameIcon::soul_of_npc, "Soul of Keaton", { MM_SOUL_NPC_KEATON }, false },
    { EGameIcon::soul_of_npc, "Soul of Koume and Kotake", { MM_SOUL_NPC_KOUME_KOTAKE }, false },
    { EGameIcon::soul_of_npc, "Soul of Romani/Cremia", { MM_SOUL_NPC_MALON }, false },
    { EGameIcon::soul_of_npc, "Soul of Mayor Dotour", { MM_SOUL_NPC_MAYOR_DOTOUR }, false },
    { EGameIcon::soul_of_npc, "Soul of Keg Trial Goron", { MM_SOUL_NPC_MEDIGORON }, false },
    { EGameIcon::soul_of_npc, "Soul of Moon Children", { MM_SOUL_NPC_MOON_CHILDREN }, false },
    { EGameIcon::soul_of_npc, "Soul of Anju's Grandmother", { MM_SOUL_NPC_OLD_HAG }, false },
    { EGameIcon::soul_of_npc, "Soul of Playground Scrubs", { MM_SOUL_NPC_PLAYGROUND_SCRUBS }, false },
    { EGameIcon::soul_of_npc, "Soul of Ghost Hut Owner", { MM_SOUL_NPC_POE_COLLECTOR }, false },
    { EGameIcon::soul_of_npc, "Soul of Part-Timer", { MM_SOUL_NPC_ROOFTOP_MAN }, false },
    { EGameIcon::soul_of_npc, "Soul of Lulu", { MM_SOUL_NPC_RUTO }, false },
    { EGameIcon::soul_of_npc, "Soul of the Scientist", { MM_SOUL_NPC_SCIENTIST }, false },
    { EGameIcon::soul_of_npc, "Soul of Town Archery Owner", { MM_SOUL_NPC_SHOOTING_GALLERY_OWNER }, false },
    { EGameIcon::soul_of_npc, "Soul of Mr. Barten", { MM_SOUL_NPC_TALON }, false },
    { EGameIcon::soul_of_npc, "Soul of the Patrolling Pirates and their Chief", { MM_SOUL_NPC_THIEVES }, false },
    { EGameIcon::soul_of_npc, "Soul of Tingle", { MM_SOUL_NPC_TINGLE }, false },
    { EGameIcon::soul_of_npc, "Soul of Toilet Hand", { MM_SOUL_NPC_TOILET_HAND }, false },
    { EGameIcon::soul_of_npc, "Soul of Toto(MM)", { MM_SOUL_NPC_TOTO }, false },
    { EGameIcon::soul_of_npc, "Soul of Tourist Center Owner", { MM_SOUL_NPC_TOURIST_CENTER }, false },
    { EGameIcon::soul_of_npc, "Soul of Zoras", { MM_SOUL_NPC_ZORA }, false },
    { EGameIcon::soul_of_npc, "Soul of the Zora Musicians", { MM_SOUL_NPC_ZORA_MUSICIANS }, false },
    { EGameIcon::soul_of_npc, "Soul of the Zora Shopkeeper", { MM_SOUL_NPC_ZORA_SHOPKEEPER }, false },
};

const ProgSection SoulsSections[NumSoulsSections] =
{
    MAKE_SECTION("OoT Boss Souls",   SoulsOoTBoss),
    MAKE_SECTION("MM Boss Souls",    SoulsMMBoss),
    MAKE_SECTION("OoT Enemy Souls",  SoulsOoTEnemies),
    MAKE_SECTION("MM Enemy Souls",  SoulsMMEnemies),
    MAKE_SECTION("OoT NPC Souls",  SoulsOoTNPCs),
    MAKE_SECTION("MM NPC Souls",  SoulsMMNPCs),
};

#pragma endregion Souls
