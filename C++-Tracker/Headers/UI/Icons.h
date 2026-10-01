#pragma once

#include "Common.h"

class QPixmap;
class QIcon;

#pragma region Game Icons

enum class EGameIcon : uint16_t
{
    //-------------------------- Object Category. They must follow the same order as ObjectType. --------------------------//

    none = 0,
    chest = 1,
    collectible = 2,
    npc = 3,
    gs = 4,			// Gold skulltula
    sf = 5,			// Stray fairy
    cow = 6,
    shop = 7,
    scrub = 8,
    sr = 9,			// Silver Rupee
    fish = 10,
    wonder = 11,
    grass = 12,
    crate = 13,
    pot = 14,
    hive = 15,
    butterfly = 16,
    rupee = 17,
    snowball = 18,
    barrel = 19,
    heart = 20,
    fairy_spot = 21,
    fairy = 22,
    icicle = 23,
    redboulder = 24,
    redice = 25,
    rock = 26,
    soil = 27,
    tree = 28,
    bush = 29,
    silverboulder = 30,
    boulder = 31,
    gossip = 32,
    gossip_big = 33,

    // Extra type used for rendering
    song = gossip_big + 1,
    heart_piece = song + 1,
    heart_container = heart_piece + 1,
    small_key = heart_container + 1,
    boss_key = small_key + 1,
    map = boss_key + 1,
    compass = map + 1,
    sword = compass + 1,
    ocarina = sword + 1,
    merchant = ocarina + 1,
    mask = merchant + 1,
    egg = mask + 1,
    owl = egg + 1,

    //-------------------------- Other Game Icon. Can have any order. --------------------------//

    // Masks
    skull,
    spooky,
    gerudo,
    keaton,
    bunny,
    truth,
    blast,
    bremen,
    captain,
    couple,
    deity,
    fairy_mask,
    garo,
    gero,
    giant,
    gibdo,
    goht,
    goron,
    gyorg,
    kafei,
    kamaro,
    night,
    odolwa,
    postman,
    romani,
    scents,
    stone,
    troupe,
    twinmold,
    deku,
    zora,
    adult_mask,

    // Swords
    kokiri,
    master,
    broken_big,
    biggoron,
    kokiri_mm,
    razor,
    gilded,
    fairy_sword,

    // Quest
    agony,
    card,
    emerald,
    ruby,
    sapphire,
    light,
    forest,
    fire,
    water,
    shadow,
    spirit,
    gs_token,
    clock_oot,
    clock_d1,
    clock_d2,
    clock_d3,
    clock_n1,
    clock_n2,
    clock_n3,
    bombers,
    spin,
    sf_green,
    sf_blue,
    sf_yellow,
    sf_orange,
    sf_transcendent,
    swamp_token,
    ocean_token,
    platinum_token,
    defense,
    magic_upgrade,
    large_magic_upgrade,
    soul_of_animal,
    soul_of_boss,
    soul_of_foe,
    soul_of_npc,
    a_button,
    c_left,
    c_right,
    c_down,
    c_up,
    song_green,
    song_red,
    song_blue,
    song_purple,
    song_orange,
    song_yellow,
    triforce_piece,
    triforce_power,
    triforce_courage,
    triforce_wisdom,
    shovel,

    // Equipment
    big_seed,
    biggest_seed,
    silver,
    golden,
    bracelet,
    silver_gauntlet,
    golden_gauntlet,
    stick_upgrade_1,
    stick_upgrade_2,
    nut_upgrade_1,
    nut_upgrade_2,
    goron_tunic,
    zora_tunic,
    hover,
    iron,
    deku_shield,
    hylian_shield,
    mirror_oot,
    hero_shield,
    mirror_mm,
    bronze_scale,
    bombchu_bag,
    bomb_bag,
    big_bomb,
    biggest_bomb,
    big_quiver,
    biggest_quiver,
    child_wallet,
    wallet,
    big_wallet,
    colossal_wallet,
    bottomless_wallet,
    silver_pouch,

    // Trade
    letter,
    cucco,
    cojiro,
    mushroom,
    potion,
    saw,
    prescription,
    frog,
    eye_drops,
    claim,
    land,
    swamp,
    mountain,
    ocean,
    key,
    letter_kafei,
    letter_mama,
    pendant,

    // Items
    seeds,
    fairy_ocarina,
    boomerang,
    slingshot,
    bow_oot,
    din,
    farore,
    nayru,
    hammer,
    hookshot_oot,
    longshot,
    ruto,
    blue_fire,
    bow_mm,
    chateau,
    dust,
    bottled_mushroom,
    seahorse,
    hookshot_mm,
    moon,
    picto,
    powder,
    bean,
    bottle,
    milk,
    red_potion,
    green_potion,
    blue_potion,
    bottled_poe,
    bottled_big_poe,
    bottled_fairy,
    bottled_bugs,
    bottled_fish,
    lens,
    fire_arrow,
    ice_arrow,
    light_arrow,
    bombchu,
    bomb,
    stick,
    nut,
    arrow,
    blue_rupee,
    red_rupee,
    purple_rupee,
    gold_rupee,
    rainbow_rupee,
    magical_rupee,
    rupoor,
    magic,
    double_magic,
    skeleton_key,
    key_ring,
    rusty_key,
    red_coin,
    green_coin,
    blue_coin,
    yellow_coin,
    carp_fish,
    ice_trap,
    fire_trap,
    drain_trap,
    magic_trap,
    shock_trap,
    knockback_trap,
    nothing,


    // Special : must be at the end
    type,			// Tells to use the ID matching the type of the object
    render_type		// Tells to use the ID matching the render type of the object
};


typedef struct IconMetaInf
{
    const char* IconPath;           // The path to the object icon.
    int Scale[2];                   // The scale of the image. ID 0 = width, ID 1 = height.
    int MaxScale[2] = { 40, 40 };   // The maximum scale of the image. ID 0 = width, ID 1 = height.
    double Alpha = 1.0;             // The defaut alpha to applay to the image.
} IconMetaInf;

static const size_t NumOfGameIcons = (const size_t)EGameIcon::type;
extern const IconMetaInf IconsMetaInfo[NumOfGameIcons];

#pragma endregion Game Icons

#pragma region Entrances

enum EntranceIcons
{
    No_Entry,
    In_Out,		 // In and Out Arrow
    In_Only,     // In arrow / One way in
    Out_Only,    // Out arrow / One way out

    Generic_Entry,
    Open_Grotto,
    Storms_Grotto,
    Bomb_Grotto,
    Boulder_Grotto,
    Silver_Boudler_Grotto,
    Red_Boulder_Grotto,
    Frozen_Grotto,
    Door,
    Tomb,
    Owl_Pick,
    Owl_Drop,
    Warp_Song,
    Wallmaster,
    Gerudo_Catch,
    Guard_Catch,
    Trapdoor,
    Telescope,

    Entrance_Last
};

extern const IconMetaInf EntranceIconsMetaInfo[EntranceIcons::Entrance_Last];

#pragma endregion

#pragma region GameIcons

/*
* The class containing all game icons and their pixmap reprensentation.
*/
class GameIcons
{
public:

    QIcon* Icons[NumOfGameIcons];         // The matching icon for common item and list category.
    QPixmap* Pixmaps[NumOfGameIcons];     // The matching pixmap for common item and list category.

    QIcon* EntIcons[EntranceIcons::Entrance_Last];        // The matching icon for entrances icons meta information.
    QPixmap* EntPixmaps[EntranceIcons::Entrance_Last];    // The matching pixmap for entrances icons.

public:

    /*
    *   Default constructor.
    */
    GameIcons();

    /*
    *   Default destructor.
    */
    ~GameIcons();


    /*
    *   Get the meta information of the given icon.
    *
    *   @param Icon    The icon to retrieve meta information for.
    *
    *   @return The icon meta information, or nullptr if the icon is out of range.
    */
    static const IconMetaInf* GetIconMetaInf(EGameIcon Icon);

    /*
    *   Get the desired game icon.
    *
    *   @return The desired game icon.
    */
    static QIcon* GetGameIcon(EGameIcon Icon);

    /*
    *   Get the pixmap corresponding to the desired icon.
    *
    *   @return The pixmap corresponding to the desired icon.
    */
    static QPixmap* GetGamePixmap(EGameIcon Icon);

    /*
    *   Get the desired entrance icon.
    *
    *   @return The desired entrance icon.
    */
    static QIcon* GetEntranceIcon(EntranceIcons Icon);

    /*
    *   Get the pixmap corresponding to the desired entrance icon.
    *
    *   @return The pixmap corresponding to the desired entrance icon.
    */
    static QPixmap* GetEntrancePixmap(EntranceIcons Icon);
};

#pragma endregion GameIcons
