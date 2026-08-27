#include "UI/Icons.h"
#include "Common.h"
#include <QIcon>
#include <QPixmap>

#pragma region Game Icons

const IconMetaInf IconsMetaInfo[NumOfGameIcons] =
{
    //-------------------------- Object Category Icons --------------------------//

    {"", {0, 0}},                                                       // ObjectType::none
    {"./Resources/Common/Chest.png", { 35, 35 }},                       // ObjectType::chest
    {"./Resources/Common/Collectible.png", { 30, 30 }},                 // ObjectType::collectible
    {"./Resources/Common/NPC.png", { 30, 30 }},                         // ObjectType::npc
    {"./Resources/Common/Gold_Skulltula.png", { 40, 40 }},              // ObjectType::gs
    {"./Resources/Common/Stray_Fairy.png", { 30, 30 }},                 // ObjectType::sf
    {"./Resources/Common/Cow.png", { 50, 50 }, { 70, 70 }},             // ObjectType::cow
    {"./Resources/Common/Shop.png", { 30, 30 }},                        // ObjectType::shop
    {"./Resources/Common/Scrub.png", { 50, 50 }, { 70, 70 }},           // ObjectType::scrub
    {"./Resources/Common/Silver_Rupee.png", { 20, 20 }, { 35, 35 }},    // ObjectType::sr
    {"./Resources/Common/Fish.png", { 30, 30 }},                        // ObjectType::fish
    {"./Resources/Common/Wonder.png", { 30, 30 }},                      // ObjectType::wonder
    {"./Resources/Common/Grass.png", { 20, 20 }},                       // ObjectType::grass
    {"./Resources/Common/Crate.png", { 25, 25 }},                       // ObjectType::crate
    {"./Resources/Common/Pot.png", { 25, 25 }, { 35, 35 }},             // ObjectType::pot
    {"./Resources/Common/Hive.png", { 40, 40 }},                        // ObjectType::hive
    {"./Resources/Common/Butterfly.png", { 20, 20 }},                   // ObjectType::butterfly
    {"./Resources/Common/Rupee.png", { 20, 20 }, { 35, 35 }},           // ObjectType::rupee
    {"./Resources/Common/Snowball.png", { 25, 25 }},                    // ObjectType::snowball
    {"./Resources/Common/Barrel.png", { 20, 20 }},                      // ObjectType::barrel
    {"./Resources/Common/Heart.png", { 30, 30 }, { 35, 35 }},           // ObjectType::heart
    {"./Resources/Common/Fairy_Spot.png", { 30, 30 }},                  // ObjectType::fairy_spot
    {"./Resources/Common/Fairy.png", { 25, 25 }, { 35, 35 }},           // ObjectType::fairy
    {"./Resources/Common/Icicle.png", { 25, 25 }, { 25, 25 }, 0.83},    // ObjectType::icicle
    {"./Resources/Common/Red_Boulder.png", { 25, 25 }},                 // ObjectType::redboulder
    {"./Resources/Common/Red_Ice.png", { 25, 25 }, { 25, 25 }, 0.78},   // ObjectType::redice
    {"./Resources/Common/Rock.png", { 20, 20 }},                        // ObjectType::rock
    {"./Resources/Common/Soil.png", { 25, 25 }},                        // ObjectType::soil
    {"./Resources/Common/Tree.png", { 25, 25 }},                        // ObjectType::tree
    {"./Resources/Common/Bush.png", { 25, 25 }},                        // ObjectType::bush
    {"./Resources/Common/Silver_Boulder.png", { 25, 25 }},              // ObjectType::silverboulder
    {"./Resources/Common/Boulder.png", { 25, 25 }},                     // ObjectType::boulder

    // Extra types
    {"./Resources/Common/Song.png", { 30, 30 }},
    {"./Resources/Common/HP.png", { 28, 28 }},
    {"./Resources/Common/HC.png", { 30, 30 }},
    {"./Resources/Common/Small_Key.png", { 30, 30 }},
    {"./Resources/Common/Boss_Key.png", { 30, 30 }},
    {"./Resources/Common/Map.png", { 30, 30 }},
    {"./Resources/Common/Compass.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Equipment/Master.png", { 30, 30 }},
    {"./Resources/Common/Ocarina.png", { 30, 30 }},
    {"./Resources/Common/Shop.png", { 25, 25 }},
    {"./Resources/Common/Mask.png", { 30, 30 }},
    {"./Resources/Common/Egg.png", { 30, 30 }},
    {"./Resources/Common/Owl.png", { 30, 30 }},

    //-------------------------- Other Game Icons --------------------------//

    // Masks
    {"./Resources/OoT/Icons/Masks/Skull.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Masks/Spooky.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Masks/Gerudo.png", { 30, 30 }},
    {"./Resources/Common/Masks/Keaton.png", { 30, 30 }},
    {"./Resources/Common/Masks/Bunny.png", { 30, 30 }},
    {"./Resources/Common/Masks/Truth.png", { 30, 30 }},
    {"./Resources/MM/Icons/Masks/Blast.png", { 30, 30 }},
    {"./Resources/MM/Icons/Masks/Bremen.png", { 30, 30 }},
    {"./Resources/MM/Icons/Masks/Captain.png", { 30, 30 }},
    {"./Resources/MM/Icons/Masks/Couple.png", { 30, 30 }},
    {"./Resources/MM/Icons/Masks/Deity.png", { 30, 30 }},
    {"./Resources/MM/Icons/Masks/Fairy.png", { 30, 30 }},
    {"./Resources/MM/Icons/Masks/Garo.png", { 30, 30 }},
    {"./Resources/MM/Icons/Masks/Gero.png", { 30, 30 }},
    {"./Resources/MM/Icons/Masks/Giant.png", { 30, 30 }},
    {"./Resources/MM/Icons/Masks/Gibdo.png", { 30, 30 }},
    {"./Resources/MM/Icons/Masks/Goht.png", { 30, 30 }},
    {"./Resources/MM/Icons/Masks/Goron.png", { 30, 30 }},
    {"./Resources/MM/Icons/Masks/Gyorg.png", { 30, 30 }},
    {"./Resources/MM/Icons/Masks/Kafei.png", { 30, 30 }},
    {"./Resources/MM/Icons/Masks/Kamaro.png", { 30, 30 }},
    {"./Resources/MM/Icons/Masks/Night.png", { 30, 30 }},
    {"./Resources/MM/Icons/Masks/Odolwa.png", { 30, 30 }},
    {"./Resources/MM/Icons/Masks/Postman.png", { 30, 30 }},
    {"./Resources/MM/Icons/Masks/Romani.png", { 30, 30 }},
    {"./Resources/MM/Icons/Masks/Scents.png", { 30, 30 }},
    {"./Resources/MM/Icons/Masks/Stone.png", { 30, 30 }},
    {"./Resources/MM/Icons/Masks/Troupe.png", { 30, 30 }},
    {"./Resources/MM/Icons/Masks/Twinmold.png", { 30, 30 }},
    {"./Resources/MM/Icons/Masks/Deku.png", { 30, 30 }},
    {"./Resources/MM/Icons/Masks/Zora.png", { 30, 30 }},
    {"./Resources/MM/Icons/Masks/Adult.png", { 30, 30 }},

    // Swords
    {"./Resources/OoT/Icons/Equipment/Kokiri.png", { 40, 40 }},
    {"./Resources/OoT/Icons/Equipment/Master.png", { 40, 40 }},
    {"./Resources/OoT/Icons/Equipment/Broken.png", { 40, 40 }},
    {"./Resources/OoT/Icons/Equipment/Biggoron.png", { 40, 40 }},
    {"./Resources/MM/Icons/Equipment/Kokiri.png", { 40, 40 }},
    {"./Resources/MM/Icons/Equipment/Razor.png", { 40, 40 }},
    {"./Resources/MM/Icons/Equipment/Gilded.png", { 40, 40 }},
    {"./Resources/MM/Icons/Equipment/Fairy.png", { 40, 40 }},

    // Quest
    {"./Resources/OoT/Icons/Quest/Agony.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Quest/Card.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Quest/Emerald.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Quest/Ruby.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Quest/Saphir.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Quest/Light.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Quest/Forest.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Quest/Fire.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Quest/Water.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Quest/Shadow.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Quest/Spirit.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Quest/GS_Token.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Quest/Clock.png", { 30, 30 } },
    {"./Resources/MM/Icons/Quest/Clock_D1.png", { 30, 30 }},
    {"./Resources/MM/Icons/Quest/Clock_D2.png", { 30, 30 }},
    {"./Resources/MM/Icons/Quest/Clock_D3.png", { 30, 30 }},
    {"./Resources/MM/Icons/Quest/Clock_N1.png", { 30, 30 }},
    {"./Resources/MM/Icons/Quest/Clock_N2.png", { 30, 30 }},
    {"./Resources/MM/Icons/Quest/Clock_N3.png", { 30, 30 }},
    {"./Resources/MM/Icons/Quest/Bombers.png", { 30, 30 }},
    {"./Resources/MM/Icons/Quest/Spin.png", { 30, 30 }},
    {"./Resources/MM/Icons/Quest/Stray_Fairy_Green.png", { 30, 30 }},
    {"./Resources/MM/Icons/Quest/Stray_Fairy_Blue.png", { 30, 30 }},
    {"./Resources/MM/Icons/Quest/Stray_Fairy_Yellow.png", { 30, 30 }},
    {"./Resources/MM/Icons/Quest/Stray_Fairy_Orange.png", { 30, 30 }},
    {"./Resources/MM/Icons/Quest/Stray_Fairy_Transcendent.png", { 30, 30 }},
    {"./Resources/MM/Icons/Quest/Swamp_Token.png", { 30, 30 }},
    {"./Resources/MM/Icons/Quest/Ocean_Token.png", { 30, 30 }},
    {"./Resources/Common/Quest/Platinum_Token.png", { 30, 30 } },
    {"./Resources/Common/Quest/Defense.png", { 30, 30 }},
    {"./Resources/Common/Quest/Magic_Upgrade.png", { 30, 30 }},
    {"./Resources/Common/Quest/Large_Magic_Upgrade.png", { 30, 30 }},
    {"./Resources/Common/Quest/Soul_of_Animal.png", { 30, 30 }},
    {"./Resources/Common/Quest/Soul_of_Boss.png", { 30, 30 }},
    {"./Resources/Common/Quest/Soul_of_Foe.png", { 30, 30 }},
    {"./Resources/Common/Quest/Soul_of_NPC.png", { 30, 30 }},
    {"./Resources/Common/Quest/A_Button.png", { 30, 30 }},
    {"./Resources/Common/Quest/C_Left.png", { 30, 30 }},
    {"./Resources/Common/Quest/C_Right.png", { 30, 30 }},
    {"./Resources/Common/Quest/C_Down.png", { 30, 30 }},
    {"./Resources/Common/Quest/C_Up.png", { 30, 30 }},
    {"./Resources/Common/Quest/Song_Green.png", { 30, 30 }},
    {"./Resources/Common/Quest/Song_Red.png", { 30, 30 }},
    {"./Resources/Common/Quest/Song_Blue.png", { 30, 30 }},
    {"./Resources/Common/Quest/Song_Purple.png", { 30, 30 }},
    {"./Resources/Common/Quest/Song_Orange.png", { 30, 30 }},
    {"./Resources/Common/Quest/Song_yellow.png", { 30, 30 }},
    {"./Resources/Common/Quest/Triforce_Piece.png", { 30, 30 } },
    {"./Resources/Common/Quest/Triforce_Power.png", { 30, 30 } },
    {"./Resources/Common/Quest/Triforce_Courage.png", { 30, 30 } },
    {"./Resources/Common/Quest/Triforce_Wisdom.png", { 30, 30 } },
    {"./Resources/Common/Quest/Shovel.png", { 30, 30 } },

    // Equipment
    {"./Resources/OoT/Icons/Equipment/Big_Seed.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Equipment/Biggest_Seed.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Equipment/Silver.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Equipment/Golden.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Equipment/Bracelet.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Equipment/Silver_Gauntlet.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Equipment/Golden_Gauntlet.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Equipment/Stick_Upgrade_1.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Equipment/Stick_Upgrade_2.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Equipment/Nut_Upgrade_1.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Equipment/Nut_Upgrade_2.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Equipment/Goron.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Equipment/Zora.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Equipment/Hover.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Equipment/Iron.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Equipment/Deku.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Equipment/Hylian.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Equipment/Mirror.png", { 30, 30 }},
    {"./Resources/MM/Icons/Equipment/Hero.png", { 30, 30 }},
    {"./Resources/MM/Icons/Equipment/Mirror.png", { 30, 30 }},
    {"./Resources/Common/Equipment/Bronze_Scale.png", { 30, 30 }},
    {"./Resources/Common/Equipment/Bombchu_Bag.png", { 30, 30 } },
    {"./Resources/Common/Equipment/Bomb_Bag.png", { 30, 30 }},
    {"./Resources/Common/Equipment/Big_Bomb_Bag.png", { 30, 30 }},
    {"./Resources/Common/Equipment/Biggest_Bomb_Bag.png", { 30, 30 }},
    {"./Resources/Common/Equipment/Big_Quiver.png", { 30, 30 }},
    {"./Resources/Common/Equipment/Biggest_Quiver.png", { 30, 30 }},
    {"./Resources/Common/Equipment/Child_Wallet.png", { 30, 30 } },
    {"./Resources/Common/Equipment/Wallet.png", { 30, 30 }},
    {"./Resources/Common/Equipment/Big_Wallet.png", { 30, 30 }},
    {"./Resources/Common/Equipment/Colossal_Wallet.png", { 30, 30 } },
    {"./Resources/Common/Equipment/Bottomless_Wallet.png", { 30, 30 } },
    {"./Resources/Common/Equipment/Silver_Pouch.png", { 30, 30 } },

    // Trade
    {"./Resources/OoT/Icons/Trade/Letter.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Trade/Cucco.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Trade/Cojiro.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Trade/Mushroom.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Trade/Potion.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Trade/Saw.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Trade/Prescription.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Trade/Frog.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Trade/Eye.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Trade/Claim.png", { 30, 30 }},
    {"./Resources/MM/Icons/Trade/Land.png", { 30, 30 }},
    {"./Resources/MM/Icons/Trade/Swamp.png", { 30, 30 }},
    {"./Resources/MM/Icons/Trade/Mountain.png", { 30, 30 }},
    {"./Resources/MM/Icons/Trade/Ocean.png", { 30, 30 }},
    {"./Resources/MM/Icons/Trade/Key.png", { 30, 30 }},
    {"./Resources/MM/Icons/Trade/Letter_Kafei.png", { 30, 30 }},
    {"./Resources/MM/Icons/Trade/Letter_Mama.png", { 30, 30 }},
    {"./Resources/MM/Icons/Trade/Pendant.png", { 30, 30 }},

    // Items
    {"./Resources/OoT/Icons/Items/Seeds.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Items/Fairy.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Items/Boomrang.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Items/Slingshot.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Items/Bow.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Items/Din.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Items/Farore.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Items/Nayru.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Items/Hammer.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Items/Hookshot.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Items/Longshot.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Items/Ruto.png", { 30, 30 }},
    {"./Resources/OoT/Icons/Items/Blue_Fire.png", { 30, 30 }},
    {"./Resources/MM/Icons/Items/Bow.png", { 30, 30 }},
    {"./Resources/MM/Icons/Items/Chateau.png", { 30, 30 }},
    {"./Resources/MM/Icons/Items/Dust.png", { 30, 30 }},
    {"./Resources/MM/Icons/Items/Mushroom.png", { 30, 30 } },
    {"./Resources/MM/Icons/Items/Seahorse.png", { 30, 30 } },
    {"./Resources/MM/Icons/Items/Hookshot.png", { 30, 30 }},
    {"./Resources/MM/Icons/Items/Moon.png", { 30, 30 }},
    {"./Resources/MM/Icons/Items/Picto.png", { 30, 30 }},
    {"./Resources/MM/Icons/Items/Powder.png", { 30, 30 }},
    {"./Resources/Common/Items/Bean.png", { 30, 30 }},
    {"./Resources/Common/Items/Bottle.png", { 30, 30 }},
    {"./Resources/Common/Items/Milk.png", { 30, 30 }},
    {"./Resources/Common/Items/Red_Potion.png", { 30, 30 }},
    {"./Resources/Common/Items/Green_Potion.png", { 30, 30 }},
    {"./Resources/Common/Items/Blue_Potion.png", { 30, 30 }},
    {"./Resources/Common/Items/Poe.png", { 30, 30 } },
    {"./Resources/Common/Items/Big_Poe.png", { 30, 30 }},
    {"./Resources/Common/Items/Fairy.png", { 30, 30 }},
    {"./Resources/Common/Items/Bugs.png", { 30, 30 }},
    {"./Resources/Common/Items/Fish.png", { 30, 30 }},
    {"./Resources/Common/Items/Lens.png", { 30, 30 } },
    {"./Resources/Common/Items/Fire.png", { 30, 30 }},
    {"./Resources/Common/Items/Ice.png", { 30, 30 }},
    {"./Resources/Common/Items/Light.png", { 30, 30 }},
    {"./Resources/Common/Items/Bombchu.png", { 30, 30 }},
    {"./Resources/Common/Items/Bomb.png", { 30, 30 }},
    {"./Resources/Common/Items/Stick.png", { 30, 30 }},
    {"./Resources/Common/Items/Nut.png", { 30, 30 }},
    {"./Resources/Common/Items/Arrow.png", { 30, 30 }},
    {"./Resources/Common/Items/Blue_Rupee.png", { 30, 30 }},
    {"./Resources/Common/Items/Red_Rupee.png", { 30, 30 }},
    {"./Resources/Common/Items/Purple_Rupee.png", { 30, 30 }},
    {"./Resources/Common/Items/Gold_Rupee.png", { 30, 30 }},
    {"./Resources/Common/Items/Rainbow_Rupee.png", { 30, 30 }},
    {"./Resources/Common/Items/Magical_Rupee.png", { 30, 30 }},
    {"./Resources/Common/Items/Rupoor.png", { 30, 30 }},
    {"./Resources/Common/Items/Magic.png", { 30, 30 } },
    {"./Resources/Common/Items/Double_Magic.png", { 30, 30 } },
    {"./Resources/Common/Items/Skeleton_Key.png", { 30, 30 } },
    {"./Resources/Common/Items/Key_Ring.png", { 30, 30 } },
    {"./Resources/Common/Items/Rusty_Key.png", { 30, 30 } },
    {"./Resources/Common/Items/Red_Coin.png", { 30, 30 } },
    {"./Resources/Common/Items/Green_Coin.png", { 30, 30 } },
    {"./Resources/Common/Items/Blue_Coin.png", { 30, 30 } },
    {"./Resources/Common/Items/Yellow_Coin.png", { 30, 30 } },
    {"./Resources/Common/Items/Carp_Fish.png", { 30, 30 } },
    {"./Resources/Common/Items/Ice_Trap.png", { 30, 30 } },
    {"./Resources/Common/Items/Fire_Trap.png", { 30, 30 } },
    {"./Resources/Common/Items/Drain_Trap.png", { 30, 30 } },
    {"./Resources/Common/Items/Magic_Trap.png", { 30, 30 } },
    {"./Resources/Common/Items/Shock_Trap.png", { 30, 30 } },
    {"./Resources/Common/Items/Knockback_Trap.png", { 30, 30 } },
    {"./Resources/Common/Items/Nothing.png", { 30, 30 } },
};

#pragma endregion Game Icons

#pragma region Entrances

const IconMetaInf EntranceIconsMetaInfo[EntranceIcons::Entrance_Last] =
{
    {"", { 0, 0 }},                                              // EntranceIcons::No_Entry
    {"./Resources/Common/In_Out_Arrow.png", {44, 32}},           // EntranceIcons::In_Out
    {"./Resources/Common/In_Arrow.png", {30, 21}},               // EntranceIcons::In
    {"./Resources/Common/Out_Arrow.png", {30, 21}},              // EntranceIcons::Out

    {"./Resources/Common/Entrances/Generic.png", {60, 60}, {120, 120}},      // EntranceIcons::Generic_Entry
    {"./Resources/Common/Entrances/Open_Grotto.png", {60, 60}, {120, 120}},      // EntranceIcons::Open_Grotto
    {"./Resources/Common/Entrances/Storms_Grotto.png", {60, 60}, {120, 120}},      // EntranceIcons::Storms_Grotto
    {"./Resources/Common/Entrances/Bomb_Grotto.png", {60, 60}, {120, 120}},      // EntranceIcons::Bomb_Grotto
    {"./Resources/Common/Entrances/Boulder_Grotto.png", {60, 60}, {120, 120}},      // EntranceIcons::Boulder_Grotto
    {"./Resources/Common/Entrances/Silver_Boulder_Grotto", {60, 60}, {120, 120}},      // EntranceIcons::Silver_Boudler_Grotto
    {"./Resources/Common/Entrances/Red_Boulder_Grotto.png", {60, 60}, {120, 120}},      // EntranceIcons::Red_Boulder_Grotto
    {"./Resources/Common/Entrances/Frozen_Grotto.png", {60, 60}, {120, 120}},      // EntranceIcons::Frozen_Grotto
    {"./Resources/Common/Entrances/Door.png", {60, 60}, {120, 120}},      // EntranceIcons::Door
    {"./Resources/Common/Entrances/Tomb.png", {60, 60}, {120, 120}},      // EntranceIcons::Tomb
    {"./Resources/Common/Entrances/Owl_Pick.png", {60, 60}, {120, 120}},      // EntranceIcons::Owl_Pick
    {"./Resources/Common/Entrances/Owl_Drop.png", {60, 60}, {120, 120}},      // EntranceIcons::Owl_Drop
    {"./Resources/Common/Entrances/Warp_Song.png", {60, 60}, {120, 120}},      // EntranceIcons::Warp_Song
    {"./Resources/Common/Entrances/Wallmaster.png", {60, 60}, {120, 120}},      // EntranceIcons::Wallmaster
    {"./Resources/Common/Entrances/Gerudo_Catch.png", {60, 60}, {120, 120}},      // EntranceIcons::Gerudo_Catch
    {"./Resources/Common/Entrances/Guard_Catch.png", {60, 60}, {120, 120}},      // EntranceIcons::Guard_Catch
    {"./Resources/Common/Entrances/Trapdoor.png", {60, 60}, {120, 120}},      // EntranceIcons::Trapdoor
    {"./Resources/Common/Entrances/Telescope.png", {60, 60}, {120, 120}},      // EntranceIcons::Telescope
};

#pragma endregion Entrances


#pragma region GameIcons


static GameIcons* IconsRef = nullptr;

GameIcons::GameIcons()
{
    for (size_t i = 0; i < NumOfGameIcons; i++)
    {
        this->Icons[i] = nullptr;
        this->Pixmaps[i] = nullptr;
    }

    for (size_t i = 0; i < EntranceIcons::Entrance_Last; i++)
    {
        this->EntIcons[i] = new QIcon(EntranceIconsMetaInfo[i].IconPath);
        this->EntPixmaps[i] = new QPixmap(EntranceIconsMetaInfo[i].IconPath);
    }
}


GameIcons::~GameIcons()
{
    for (size_t i = 0; i < NumOfGameIcons; i++)
    {
        delete this->Icons[i];
        delete this->Pixmaps[i];
    }

    for (size_t i = 0; i < EntranceIcons::Entrance_Last; i++)
    {
        delete this->EntIcons[i];
        delete this->EntPixmaps[i];
    }
}


const IconMetaInf* GameIcons::GetIconMetaInf(EGameIcon Icon)
{
    return &IconsMetaInfo[(uint8_t)Icon];
}


QIcon* GameIcons::GetGameIcon(EGameIcon Icon)
{
    uint8_t IconID = (uint8_t)Icon;

    if (IconsRef == nullptr)
    {
        IconsRef = new GameIcons();
        IconsRef->Icons[IconID] = new QIcon(IconsMetaInfo[IconID].IconPath);
    }
    else if (IconsRef->Icons[IconID] == nullptr)
    {   // The icon doen not exit yet, we need to create it

        IconsRef->Icons[IconID] = new QIcon(IconsMetaInfo[IconID].IconPath);
    }

    return IconsRef->Icons[IconID];
}


QPixmap* GameIcons::GetGamePixmap(EGameIcon Icon)
{
    uint8_t IconID = (uint8_t)Icon;

    if (IconsRef == nullptr)
    {   
        IconsRef = new GameIcons();
        IconsRef->Pixmaps[IconID] = new QPixmap(IconsMetaInfo[IconID].IconPath);
    }
    else if (IconsRef->Pixmaps[IconID] == nullptr)
    {   // The pixmap doen not exit yet, we need to create it

        IconsRef->Pixmaps[IconID] = new QPixmap(IconsMetaInfo[IconID].IconPath);
    }

    return IconsRef->Pixmaps[IconID];
}


QIcon* GameIcons::GetEntranceIcon(EntranceIcons Icon)
{
    uint8_t IconID = (uint8_t)Icon;

    if (IconsRef == nullptr)
    {
        IconsRef = new GameIcons();
        IconsRef->EntIcons[IconID] = new QIcon(EntranceIconsMetaInfo[IconID].IconPath);
    }
    else if (IconsRef->EntIcons[IconID] == nullptr)
    {   // The icon doen not exit yet, we need to create it

        IconsRef->EntIcons[IconID] = new QIcon(EntranceIconsMetaInfo[IconID].IconPath);
    }

    return IconsRef->EntIcons[IconID];
}


QPixmap* GameIcons::GetEntrancePixmap(EntranceIcons Icon)
{
    uint8_t IconID = (uint8_t)Icon;

    if (IconsRef == nullptr)
    {
        IconsRef = new GameIcons();
        IconsRef->EntPixmaps[IconID] = new QPixmap(EntranceIconsMetaInfo[IconID].IconPath);
    }
    else if (IconsRef->EntPixmaps[IconID] == nullptr)
    {   // The pixmap doen not exit yet, we need to create it

        IconsRef->EntPixmaps[IconID] = new QPixmap(EntranceIconsMetaInfo[IconID].IconPath);
    }

    return IconsRef->EntPixmaps[IconID];
}

#pragma endregion
