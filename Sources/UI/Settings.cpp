#include "UI/FilterManager.h"
#include "UI/AppConfig.h"
#include "UI/Settings.h"
#include "Combo/Objects.h"
#include "Combo/Scenes.h"

Settings::Settings()
{
    this->Version = ROMVersion::stable;
	this->Game = ROMGame::ootmm;
	this->Mode = GameMode::single;
    this->Goal = GoalMode::boss;
	this->NumOfTeams = 1;
	this->LocalWorld = 1;
    this->IsFireTempleOpenAsChild = false;
    this->FilterSettings = QMap<QString, Parameter>({
	{ "songs", { "Song Notes", ParamType::boolean, ParamCategory::standard, ShuffleSetting::vanilla, {  } } },
	{ "goldSkulltulaTokens", { "Gold Skulltula - OoT", ParamType::shuffle, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "housesSkulltulaTokens", { "Gold Skulltula - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "tingleShuffle", { "Tingle Maps", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "mapCompassShuffle", { "Map / Compass", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "smallKeyShuffleOot", { "Small Key - OoT", ParamType::shuffle, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "smallKeyShuffleMm", { "Small Key - MM", ParamType::shuffle, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "smallKeyShuffleHideout", { "Small Key - Hideout", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "smallKeyShuffleChestGame", { "Small Key - Chest Game", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "bossKeyShuffleOot", { "Boss Key - OoT", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "bossKeyShuffleMm", { "Boss Key - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "silverRupeeShuffle", { "Silver Rupee", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "townFairyShuffle", { "Town Fairy", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "strayFairyChestShuffle", { "Stray Fairy - Chest", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "strayFairyOtherShuffle", { "Stray Fairy - Other", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "ganonBossKey", { "Boss Key - Ganon", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "scrubShuffleOot", { "Scrub - OoT", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "scrubShuffleMm", { "Scrub - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "cowShuffleOot", { "Cow - OoT", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "cowShuffleMm", { "Cow - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shopShuffleOot", { "Shop - OoT", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shopShuffleMm", { "Shop - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "owlShuffle", { "Owl", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
    { "shuffleSkulltulaFinalReward", { "100 Gold Skulltula Reward", ParamType::boolean, ParamCategory::standard, ShuffleSetting::vanilla, {  } } },
	{ "shufflePotsOot", { "Pots - OoT", ParamType::shuffle, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shufflePotsMm", { "Pots - MM", ParamType::shuffle, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shuffleCratesOot", { "Crates - OoT", ParamType::shuffle, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shuffleCratesMm", { "Crates - MM", ParamType::shuffle, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shuffleBarrelsMm", { "Barrels - MM", ParamType::shuffle, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shuffleHivesOot", { "Hives - OoT", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shuffleHivesMm", { "Hives - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shuffleGrassOot", { "Grass - OoT", ParamType::shuffle, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shuffleRocksOot", { "Rocks - OoT", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shuffleRocksMm", { "Rocks - MM", ParamType::shuffle, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shuffleGrassMm", { "Grass - MM", ParamType::shuffle, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shuffleTFGrassMm", { "Termina Field Grass", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shuffleTreesOot", { "Trees - OoT", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shuffleTreesMm", { "Trees - MM", ParamType::shuffle, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shuffleBushOot", { "Bush - OoT", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shuffleBushMm", { "Bush - MM", ParamType::shuffle, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shuffleSoilOot", { "Soil - OoT", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shuffleSoilMm", { "Soil - MM", ParamType::shuffle, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shuffleFreeRupeesOot", { "Freestanding Rupees - OoT", ParamType::shuffle, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shuffleFreeRupeesMm", { "Freestanding Rupees - MM", ParamType::shuffle, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shuffleFreeHeartsOot", { "Freestanding Hearts - OoT", ParamType::shuffle, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shuffleFreeHeartsMm", { "Freestanding Hearts - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shuffleWonderItemsOot", { "Wonder Items - OoT", ParamType::shuffle, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shuffleWonderItemsMm", { "Wonder Items - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shuffleSnowballsMm", { "Snowballs - MM", ParamType::shuffle, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shuffleButterfliesOot", { "Butterflies - OoT", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shuffleButterfliesMm", { "Butterflies - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
    { "shuffleBouldersOot", { "Boulders - OoT", ParamType::shuffle, ParamCategory::standard, ShuffleSetting::vanilla, {  } } },
    { "shuffleBouldersMm", { "Boulders - MM", ParamType::shuffle, ParamCategory::standard, ShuffleSetting::vanilla, {  } } },
    { "shuffleSilverBouldersOot", { "Silver Boulders - OoT", ParamType::boolean, ParamCategory::standard, ShuffleSetting::vanilla, {  } } },
	{ "shuffleRedBouldersOot", { "Red Boulders - OoT", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shuffleRedBouldersMm", { "Red Boulders - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shuffleFrogsRupeesOot", { "Frogs Rupees", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shuffleIciclesOot", { "Icicles - OoT", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shuffleIciclesMm", { "Icicles - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shuffleRedIceOot", { "Red Ice", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shuffleOcarinasOot", { "Ocarinas", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shuffleMasterSword", { "Master Sword", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shuffleGerudoCard", { "Gerudo Card", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shuffleMaskTrades", { "Mask Trades", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shuffleMerchantsOot", { "Merchants - OoT", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shuffleMerchantsMm", { "Merchants - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "pondFishShuffle", { "Pond Fish", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "divingGameRupeeShuffle", { "Diving Game Rupees", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "fairyFountainFairyShuffleOot", { "Fairy Fountain - OoT", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "fairyFountainFairyShuffleMm", { "Fairy Fountain - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "fairySpotShuffleOot", { "Fairy Spot", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "eggShuffle", { "Egg Content", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "skipZelda", { "Skip Zelda", ParamType::boolean, ParamCategory::standard, ShuffleSetting::vanilla, {  } } },
    { "agelessStrength", { "Use Silver / Golden Gauntlets as Child", ParamType::boolean, ParamCategory::standard, ShuffleSetting::vanilla, {  } } },
	{ "restoreBrokenActors", { "Restore Broken Actors", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
	{ "shuffleLotteryMm", { "Lottery Prizes", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, {  } } },
    { "crossWarpOot", { "Cross-Games OoT Warp Song", ParamType::boolean, ParamCategory::standard, ShuffleSetting::vanilla, {  } } },
    { "crossWarpMm", { "Cross-Games MM Song of Soaring", ParamType::boolean, ParamCategory::standard, ShuffleSetting::vanilla, {  } } },
    });

    this->ItemSettings = QMap<QString, Parameter>({
    { "progressiveShieldsOot", { "Progressive Shields - OoT", ParamType::boolean, ParamCategory::progressive, ShuffleSetting::vanilla, { OOT_PROGRESSIVE_SHIELD_DEKU, OOT_PROGRESSIVE_SHIELD_HYLIAN } } },
    { "progressiveSwordsOot", { "Progressive Swords - OoT", ParamType::boolean, ParamCategory::progressive, ShuffleSetting::vanilla, { OOT_SWORD_KOKIRI, OOT_SWORD_MASTER, OOT_SWORD_KNIFE, OOT_SWORD_GORON, OOT_SWORD_BIGGORON, OOT_SWORD_RAZOR, OOT_SWORD_GILDED, OOT_SWORD } } },
    { "progressiveShieldsMm", { "Progressive Shields - Mm", ParamType::boolean, ParamCategory::progressive, ShuffleSetting::vanilla, { MM_PROGRESSIVE_SHIELD_DEKU, MM_PROGRESSIVE_SHIELD_HERO } } },
    { "progressiveGFS", { "Progressive Great Fairy Sword", ParamType::boolean, ParamCategory::progressive, ShuffleSetting::vanilla, { MM_SWORD_KOKIRI, MM_SWORD_RAZOR, MM_SWORD_GILDED, MM_GREAT_FAIRY_SWORD, MM_SWORD } } },
    { "progressiveClocks", { "Progressive Clock", ParamType::boolean, ParamCategory::progressive, ShuffleSetting::all, { MM_CLOCK, MM_CLOCK1, MM_CLOCK2, MM_CLOCK3, MM_CLOCK4, MM_CLOCK5, MM_CLOCK6 } } },
    { "sticksNutsUpgradesMm", { "Deku Sticks / Nuts Upgrades - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { MM_NUT_UPGRADE, MM_NUT_UPGRADE2, MM_STICK_UPGRADE, MM_STICK_UPGRADE2 } } },
    { "sunSongMm", { "Sun's Song - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { MM_SONG_SUN } } },
    { "fairyOcarinaMm", { "Fairy Ocarina - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { MM_OCARINA_FAIRY } } },
    { "shortHookshotMm", { "Short Hookshot - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { MM_HOOKSHOT_SHORT } } },
    { "bronzeScale", { "Bonze Scale", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { OOT_SCALE_BRONZE, MM_SCALE_BRONZE } } },
    { "childWallets", { "Child Wallet", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { OOT_WALLET, MM_WALLET } } },
    { "colossalWallets", { "Colossal Wallet", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { OOT_WALLET4, MM_WALLET4 } } },
    { "bottomlessWallets", { "Bottomless Wallet", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { OOT_WALLET5, MM_WALLET5 } } },
    { "skeletonKeyOot", { "Skeleton Key - OoT", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { OOT_SKELETON_KEY } } },
    { "skeletonKeyMm", { "Skeleton Key - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { MM_SKELETON_KEY } } },
    { "magicalRupee", { "Magical Rupee", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { OOT_RUPEE_MAGICAL } } },
    { "transcendentFairy", { "Transcendent Fairy", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { MM_TRANSCENDENT_FAIRY } } },
    { "platinumTokenOot", { "Platinum Token - OoT", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { OOT_PLATINUM_TOKEN } } },
    { "platinumTokenMm", { "Platinum Token - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { MM_PLATINUM_TOKEN } } },
    { "spellFireMm", { "Din's Fire - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { MM_SPELL_FIRE } } },
    { "spellWindMm", { "Farore's Wind - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { MM_SPELL_WIND } } },
    { "spellLoveMm", { "Nayru's Love - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { MM_SPELL_LOVE } } },
    { "bootsIronMm", { "Iron Boots - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { MM_BOOTS_IRON } } },
    { "bootsHoverMm", { "Hover Boots - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { MM_BOOTS_HOVER } } },
    { "tunicGoronMm", { "Goron Tunic - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { MM_TUNIC_GORON } } },
    { "tunicZoraMm", { "Zora Tunic - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { MM_TUNIC_ZORA } } },
    { "scalesMm", { "Scales - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { MM_SCALE_SILVER, MM_SCALE_GOLDEN } } },
    { "strengthMm", { "Strength - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { MM_GORON_BRACELET, MM_SILVER_GAUNTLETS, MM_GOLDEN_GAUNTLETS } } },
    { "hammerMm", { "Megaton Hammer - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { MM_HAMMER } } },
    { "spinUpgradeOot", { "Spin Attack Upgrade - OoT", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { OOT_SPIN_UPGRADE } } },
    { "dekuShieldMm", { "Deku Shield - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { MM_SHIELD_DEKU } } },
    { "blastMaskOot", { "Blast Mask - OoT", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { OOT_MASK_BLAST } } },
    { "stoneMaskOot", { "Stone Mask - OoT", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { OOT_MASK_STONE } } },
    { "elegyOot", { "Elegy of Emptiness - OoT", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { OOT_SONG_EMPTINESS, OOT_SONG_NOTE_EMPTINESS } } },
    { "ocarinaButtonsShuffleOot", { "Ocarina Buttons Shuffle - OoT", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { OOT_BUTTON_A, OOT_BUTTON_C_DOWN, OOT_BUTTON_C_LEFT, OOT_BUTTON_C_RIGHT, OOT_BUTTON_C_UP } } },
    { "ocarinaButtonsShuffleMm", { "Ocarina Buttons Shuffle - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { MM_BUTTON_A, MM_BUTTON_C_DOWN, MM_BUTTON_C_LEFT, MM_BUTTON_C_RIGHT, MM_BUTTON_C_UP } } },
    { "soulsEnemyOot", { "Enemy Souls - OoT", ParamType::boolean, ParamCategory::souls, ShuffleSetting::all, { OOT_SOUL_ENEMY_STALFOS, OOT_SOUL_ENEMY_POE } } },
    { "soulsEnemyMm", { "Enemy Souls - MM", ParamType::boolean, ParamCategory::souls, ShuffleSetting::all, { MM_SOUL_ENEMY_OCTOROK, MM_SOUL_ENEMY_POE } } },
    { "soulsBossOot", { "Boss Souls - OoT", ParamType::boolean, ParamCategory::souls, ShuffleSetting::all, { OOT_SOUL_BOSS_QUEEN_GOHMA, OOT_SOUL_BOSS_TWINROVA } } },
    { "soulsBossMm", { "Boss Souls - MM", ParamType::boolean, ParamCategory::souls, ShuffleSetting::all, { MM_SOUL_BOSS_ODOLWA, MM_SOUL_BOSS_IGOS } } },
    { "soulsNpcOot", { "NPC Souls - OoT", ParamType::boolean, ParamCategory::souls, ShuffleSetting::all, { OOT_SOUL_NPC_SARIA, OOT_SOUL_NPC_THIEVES } } },
    { "soulsNpcMm", { "NPC Souls - MM", ParamType::boolean, ParamCategory::souls, ShuffleSetting::all, { MM_SOUL_NPC_AROMA, MM_SOUL_NPC_THIEVES } } },
    { "soulsAnimalOot", { "Animal Souls - OoT", ParamType::boolean, ParamCategory::souls, ShuffleSetting::all, { OOT_SOUL_ANIMAL_CUCCO, OOT_SOUL_ANIMAL_BUTTERFLY } } },
    { "soulsAnimalMm", { "Animal Souls - MM", ParamType::boolean, ParamCategory::souls, ShuffleSetting::all, { MM_SOUL_ANIMAL_CUCCO, MM_SOUL_ANIMAL_BUTTERFLY } } },
    { "soulsMiscOot", { "Misc Souls - OoT", ParamType::boolean, ParamCategory::souls, ShuffleSetting::all, { OOT_SOUL_MISC_GS, OOT_SOUL_MISC_BUSINESS_SCRUB } } },
    { "soulsMiscMm", { "Misc Souls - MM", ParamType::boolean, ParamCategory::souls, ShuffleSetting::all, { MM_SOUL_MISC_GS, MM_SOUL_MISC_BUSINESS_SCRUB } } },
    { "clocks", { "Clocks Shuffle", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { MM_CLOCK1, MM_CLOCK2, MM_CLOCK3, MM_CLOCK4, MM_CLOCK5, MM_CLOCK6 } } },
    { "coins", { "Coins", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { OOT_COIN_RED, OOT_COIN_GREEN, OOT_COIN_BLUE, OOT_COIN_YELLOW } } },
    { "coinsRed", { "Red Coins", ParamType::uint, ParamCategory::standard, ShuffleSetting::all, { OOT_COIN_RED } } },
    { "coinsGreen", { "Green Coins", ParamType::uint, ParamCategory::standard, ShuffleSetting::all, { OOT_COIN_GREEN } } },
    { "coinsBlue", { "Blue Coins", ParamType::uint, ParamCategory::standard, ShuffleSetting::all, { OOT_COIN_BLUE } } },
    { "coinsYellow", { "Yellow Coins", ParamType::uint, ParamCategory::standard, ShuffleSetting::all, { OOT_COIN_YELLOW } } },
    { "trapIce", { "Ice Trap", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { OOT_TRAP_ICE } } },
    { "trapFire", { "Fire Trap", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { OOT_TRAP_FIRE } } },
    { "trapShock", { "Shock Trap", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { OOT_TRAP_SHOCK } } },
    { "trapDrain", { "Drain Trap", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { OOT_TRAP_DRAIN } } },
    { "trapAntiMagic", { "Magic Trap", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { OOT_TRAP_ANTI_MAGIC } } },
    { "trapKnockback", { "Knockback Trap", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { OOT_TRAP_KNOCKBACK } } },
    { "trapRupoor", { "Rupoor", ParamType::boolean, ParamCategory::standard, ShuffleSetting::all, { OOT_TRAP_RUPOOR } } },
    { "sharedStoneAgony", { "Shared Stone of Agony", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_STONE_OF_AGONY, MM_STONE_OF_AGONY } } },
    { "sharedSpinUpgrade", { "Shared Spin of Upgrade", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_SPIN_UPGRADE, MM_SPIN_UPGRADE } } },
    { "sharedNutsSticks", { "Shared Nuts / Sticks", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_STICK, OOT_STICKS_5, OOT_STICKS_10, OOT_NUTS_5, OOT_NUTS_5_ALT, OOT_NUTS_10, OOT_STICK_UPGRADE, OOT_STICK_UPGRADE2, OOT_NUT_UPGRADE, OOT_NUT_UPGRADE2, MM_STICK, MM_UNK_2F, MM_UNK_30, MM_UNK_31, MM_NUT, MM_NUTS_5, MM_NUTS_10, MM_UNK_2B, MM_UNK_2C, MM_UNK_2D, OOT_STICK_UPGRADE, OOT_STICK_UPGRADE2, OOT_NUT_UPGRADE, OOT_NUT_UPGRADE2 } } },
    { "sharedBows", { "Shared Bows", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_BOW, OOT_ARROWS_5, OOT_ARROWS_10, OOT_ARROWS_30, OOT_QUIVER2, OOT_QUIVER3, MM_BOW, MM_ARROWS_10, MM_ARROWS_30, MM_ARROWS_40, MM_UNK_21, MM_QUIVER2, MM_QUIVER3 } } },
    { "sharedBombBags", { "Shared Bomb Bags", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_BOMBCHU_BAG, OOT_BOMBCHU_BAG2, OOT_BOMBCHU_BAG3, OOT_BOMB_BAG, OOT_BOMB_BAG2, OOT_BOMB_BAG3, OOT_BOMB, OOT_BOMBS_5, OOT_BOMBS_10, OOT_BOMBS_20, OOT_BOMBS_30, OOT_BOMBCHU_5, OOT_BOMBCHU_10, OOT_BOMBCHU_20, MM_BOMBCHU_BAG, MM_BOMBCHU_BAG2, MM_BOMBCHU_BAG3, MM_BOMB_BAG, MM_BOMB_BAG2, MM_BOMB_BAG3, MM_BOMB, MM_BOMBS_5, MM_BOMBS_10, MM_BOMBS_20, MM_BOMBS_30, MM_BOMBCHU, MM_BOMBCHU_5, MM_BOMBCHU_10, MM_BOMBCHU_20 } } },
    { "sharedMagic", { "Shared Magic", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_MAGIC_UPGRADE, OOT_MAGIC_UPGRADE2, OOT_MAGIC_JAR_SMALL, OOT_MAGIC_JAR_LARGE, MM_MAGIC_UPGRADE, MM_MAGIC_UPGRADE2, MM_MAGIC_JAR_SMALL, MM_MAGIC_JAR_LARGE } } },
    { "sharedMagicArrowFire", { "Shared Fire Arrow", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_ARROW_FIRE, MM_ARROW_FIRE } } },
    { "sharedMagicArrowIce", { "Shared Ice Arrow", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_ARROW_ICE, MM_ARROW_ICE } } },
    { "sharedMagicArrowLight", { "Shared Light Arrow", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_ARROW_LIGHT, MM_ARROW_LIGHT } } },
    { "sharedSongEpona", { "Shared Epona's Song", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_SONG_EPONA, OOT_SONG_NOTE_EPONA, MM_SONG_EPONA, MM_SONG_NOTE_EPONA } } },
    { "sharedSongStorms", { "Shared Song of Storms", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_SONG_STORMS, OOT_SONG_NOTE_STORMS, MM_SONG_STORMS, MM_SONG_NOTE_STORMS } } },
    { "sharedSongTime", { "Shared Song of Time", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_SONG_TIME, OOT_SONG_NOTE_TIME, MM_SONG_TIME, MM_SONG_NOTE_TIME } } },
    { "sharedSongSun", { "Shared Sun's Sung", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_SONG_SUN, OOT_SONG_NOTE_SUN, MM_SONG_SUN, MM_SONG_NOTE_SUN } } },
    { "sharedHookshot", { "Shared Hookshot", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_HOOKSHOT, OOT_LONGSHOT, MM_HOOKSHOT, MM_HOOKSHOT_SHORT } } },
    { "sharedLens", { "Shared Lens of Truth", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_LENS, MM_LENS } } },
    { "sharedOcarina", { "Shared Ocarina", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_OCARINA_FAIRY, OOT_OCARINA_TIME, MM_OCARINA_FAIRY, MM_OCARINA_OF_TIME } } },
    { "sharedMaskGoron", { "Shared Goron Mask", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_MASK_GORON, MM_MASK_GORON } } },
    { "sharedMaskZora", { "Shared Zora Mask", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_MASK_ZORA, MM_MASK_ZORA } } },
    { "sharedMaskBunny", { "Shared Bunny Mask", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_MASK_BUNNY, MM_MASK_BUNNY } } },
    { "sharedMaskKeaton", { "Shared Keaton Mask", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_MASK_KEATON, MM_MASK_KEATON } } },
    { "sharedMaskTruth", { "Shared Truth Mask", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_MASK_TRUTH, MM_MASK_TRUTH } } },
    { "sharedMaskBlast", { "Shared Blast Mask", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_MASK_BLAST, MM_MASK_BLAST } } },
    { "sharedMaskStone", { "Shared Stone Mask", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_MASK_STONE, MM_MASK_STONE } } },
    { "sharedSongElegy", { "Shared Elegy of Emptiness", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_SONG_EMPTINESS, OOT_SONG_NOTE_EMPTINESS, MM_SONG_EMPTINESS, MM_SONG_NOTE_EMPTINESS } } },
    { "sharedWallets", { "Shared Wallets", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_WALLET, OOT_WALLET2, OOT_WALLET3, OOT_WALLET4, OOT_WALLET5, MM_WALLET, MM_WALLET2, MM_WALLET3, MM_WALLET4, MM_WALLET5 } } },
    { "sharedHealth", { "Shared Health", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_RECOVERY_HEART, OOT_TC_HEART_PIECE, OOT_HEART_PIECE, OOT_HEART_CONTAINER, OOT_DEFENSE_UPGRADE, MM_RECOVERY_HEART, MM_HEART_PIECE, MM_HEART_CONTAINER, MM_DEFENSE_UPGRADE } } },
    { "sharedShields", { "Shared Shields", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_SHIELD_DEKU, OOT_SHIELD_HYLIAN, OOT_SHIELD_MIRROR, MM_SHIELD_DEKU, MM_SHIELD_HERO, MM_SHIELD_MIRROR } } },
    { "sharedSoulsEnemy", { "Shared Souls of Enemy", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_SOUL_ENEMY_ARMOS, OOT_SOUL_ENEMY_BUBBLE, OOT_SOUL_ENEMY_BEAMOS, OOT_SOUL_ENEMY_DEKU_BABA, OOT_SOUL_ENEMY_DEKU_SCRUB, OOT_SOUL_ENEMY_DODONGO, OOT_SOUL_ENEMY_FLOORMASTER, OOT_SOUL_ENEMY_FLYING_POT, OOT_SOUL_ENEMY_FREEZARD, OOT_SOUL_ENEMY_GUAY, OOT_SOUL_ENEMY_IRON_KNUCKLE, OOT_SOUL_ENEMY_KEESE, OOT_SOUL_ENEMY_LEEVER, OOT_SOUL_ENEMY_LIKE_LIKE, OOT_SOUL_ENEMY_LIZALFOS_DINOLFOS, OOT_SOUL_ENEMY_OCTOROK, OOT_SOUL_ENEMY_PEAHAT, OOT_SOUL_ENEMY_POE, OOT_SOUL_ENEMY_REDEAD_GIBDO, OOT_SOUL_ENEMY_SHELL_BLADE, OOT_SOUL_ENEMY_SKULLTULA, OOT_SOUL_ENEMY_SKULLWALLTULA, OOT_SOUL_ENEMY_STALCHILD, OOT_SOUL_ENEMY_TEKTITE, OOT_SOUL_ENEMY_THIEVES, OOT_SOUL_ENEMY_WALLMASTER, OOT_SOUL_ENEMY_WOLFOS, MM_SOUL_ENEMY_ARMOS, MM_SOUL_ENEMY_BUBBLE, MM_SOUL_ENEMY_BEAMOS, MM_SOUL_ENEMY_DEKU_BABA, MM_SOUL_ENEMY_DEKU_SCRUB, MM_SOUL_ENEMY_DODONGO, MM_SOUL_ENEMY_FLOORMASTER, MM_SOUL_ENEMY_FLYING_POT, MM_SOUL_ENEMY_FREEZARD, MM_SOUL_ENEMY_GUAY, MM_SOUL_ENEMY_IRON_KNUCKLE, MM_SOUL_ENEMY_KEESE, MM_SOUL_ENEMY_LEEVER, MM_SOUL_ENEMY_LIKE_LIKE, MM_SOUL_ENEMY_LIZALFOS_DINOLFOS, MM_SOUL_ENEMY_OCTOROK, MM_SOUL_ENEMY_PEAHAT, MM_SOUL_ENEMY_POE, MM_SOUL_ENEMY_REDEAD_GIBDO, MM_SOUL_ENEMY_SHELL_BLADE, MM_SOUL_ENEMY_SKULLTULA, MM_SOUL_ENEMY_SKULLWALLTULA, MM_SOUL_ENEMY_STALCHILD, MM_SOUL_ENEMY_TEKTITE, MM_SOUL_ENEMY_THIEVES, MM_SOUL_ENEMY_WALLMASTER, MM_SOUL_ENEMY_WOLFOS } } },
    { "sharedSoulsNpc", { "Shared Souls of NPC", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_SOUL_NPC_ANJU, OOT_SOUL_NPC_ASTRONOMER, OOT_SOUL_NPC_BANKER, OOT_SOUL_NPC_BAZAAR_SHOPKEEPER, OOT_SOUL_NPC_BEAN_SALESMAN, OOT_SOUL_NPC_BIGGORON, OOT_SOUL_NPC_BOMBCHU_BOWLING_LADY, OOT_SOUL_NPC_BOMBCHU_SHOPKEEPER, OOT_SOUL_NPC_BOMBERS, OOT_SOUL_NPC_CARPENTERS, OOT_SOUL_NPC_CARPET_MAN, OOT_SOUL_NPC_CHEST_GAME_OWNER, OOT_SOUL_NPC_CITIZEN, OOT_SOUL_NPC_COMPOSER_BROS, OOT_SOUL_NPC_DAMPE, OOT_SOUL_NPC_DOG_LADY, OOT_SOUL_NPC_FISHING_POND_OWNER, OOT_SOUL_NPC_GORMAN, OOT_SOUL_NPC_GORON, OOT_SOUL_NPC_GORON_CHILD, OOT_SOUL_NPC_GORON_SHOPKEEPER, OOT_SOUL_NPC_GROG, OOT_SOUL_NPC_GURU_GURU, OOT_SOUL_NPC_HONEY_DARLING, OOT_SOUL_NPC_MALON, OOT_SOUL_NPC_MEDIGORON, OOT_SOUL_NPC_OLD_HAG, OOT_SOUL_NPC_POE_COLLECTOR, OOT_SOUL_NPC_ROOFTOP_MAN, OOT_SOUL_NPC_RUTO, OOT_SOUL_NPC_SCIENTIST, OOT_SOUL_NPC_SHOOTING_GALLERY_OWNER, OOT_SOUL_NPC_TALON, OOT_SOUL_NPC_THIEVES, OOT_SOUL_NPC_ZORA, OOT_SOUL_NPC_ZORA_SHOPKEEPER, MM_SOUL_NPC_ANJU, MM_SOUL_NPC_ASTRONOMER, MM_SOUL_NPC_BANKER, MM_SOUL_NPC_BAZAAR_SHOPKEEPER, MM_SOUL_NPC_BEAN_SALESMAN, MM_SOUL_NPC_BIGGORON, MM_SOUL_NPC_BOMBCHU_BOWLING_LADY, MM_SOUL_NPC_BOMBCHU_SHOPKEEPER, MM_SOUL_NPC_BOMBERS, MM_SOUL_NPC_CARPENTERS, MM_SOUL_NPC_CARPET_MAN, MM_SOUL_NPC_CHEST_GAME_OWNER, MM_SOUL_NPC_CITIZEN, MM_SOUL_NPC_COMPOSER_BROS, MM_SOUL_NPC_DAMPE, MM_SOUL_NPC_DOG_LADY, MM_SOUL_NPC_FISHING_POND_OWNER, MM_SOUL_NPC_GORMAN, MM_SOUL_NPC_GORON, MM_SOUL_NPC_GORON_CHILD, MM_SOUL_NPC_GORON_SHOPKEEPER, MM_SOUL_NPC_GROG, MM_SOUL_NPC_GURU_GURU, MM_SOUL_NPC_HONEY_DARLING, MM_SOUL_NPC_MALON, MM_SOUL_NPC_MEDIGORON, MM_SOUL_NPC_OLD_HAG, MM_SOUL_NPC_POE_COLLECTOR, MM_SOUL_NPC_ROOFTOP_MAN, MM_SOUL_NPC_RUTO, MM_SOUL_NPC_SCIENTIST, MM_SOUL_NPC_SHOOTING_GALLERY_OWNER, MM_SOUL_NPC_TALON, MM_SOUL_NPC_THIEVES, MM_SOUL_NPC_ZORA, MM_SOUL_NPC_ZORA_SHOPKEEPER } } },
    { "sharedSoulsAnimal", { "Shared Souls of Animal", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_SOUL_ANIMAL_CUCCO, OOT_SOUL_ANIMAL_COW, OOT_SOUL_ANIMAL_DOG, OOT_SOUL_ANIMAL_BUTTERFLY, MM_SOUL_ANIMAL_CUCCO, MM_SOUL_ANIMAL_COW, MM_SOUL_ANIMAL_DOG, MM_SOUL_ANIMAL_BUTTERFLY } } },
    { "sharedSoulsMisc", { "Shared Misc Souls", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_SOUL_MISC_GS, OOT_SOUL_MISC_BUSINESS_SCRUB, MM_SOUL_MISC_GS, MM_SOUL_MISC_BUSINESS_SCRUB } } },
    { "sharedOcarinaButtons", { "Shared Ocarina Buttons", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_BUTTON_A, OOT_BUTTON_C_LEFT, OOT_BUTTON_C_RIGHT, OOT_BUTTON_C_UP, OOT_BUTTON_C_DOWN, MM_BUTTON_A, MM_BUTTON_C_LEFT, MM_BUTTON_C_RIGHT, MM_BUTTON_C_UP, MM_BUTTON_C_DOWN } } },
    { "sharedSkeletonKey", { "Shared Skeleton Key", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_SKELETON_KEY, MM_SKELETON_KEY } } },
    { "sharedPlatinumToken", { "Shared Platinum Token", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_PLATINUM_TOKEN, MM_PLATINUM_TOKEN } } },
    { "sharedSpellFire", { "Shared Din's Fire", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_SPELL_FIRE, MM_SPELL_FIRE } } },
    { "sharedSpellWind", { "Shared Farore's Wind", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_SPELL_WIND, MM_SPELL_WIND } } },
    { "sharedSpellLove", { "Shared Nayru's Love", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_SPELL_LOVE, MM_SPELL_LOVE } } },
    { "sharedBootsIron", { "Shared Iron Boots", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_BOOTS_IRON, MM_BOOTS_IRON } } },
    { "sharedBootsHover", { "Shared Hover Boots", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_BOOTS_HOVER, MM_BOOTS_HOVER } } },
    { "sharedTunicGoron", { "Shared Goron Tunic", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_TUNIC_GORON, MM_TUNIC_GORON } } },
    { "sharedTunicZora", { "Shared Zora Tunic", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_TUNIC_ZORA, MM_TUNIC_ZORA } } },
    { "sharedScales", { "Shared Scales", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_SCALE_BRONZE, OOT_SCALE_SILVER, OOT_SCALE_GOLDEN, MM_SCALE_BRONZE, MM_SCALE_SILVER, MM_SCALE_GOLDEN } } },
    { "sharedStrength", { "Shared Strength", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_GORON_BRACELET, OOT_SILVER_GAUNTLETS, OOT_GOLDEN_GAUNTLETS, MM_GORON_BRACELET, MM_SILVER_GAUNTLETS, MM_GOLDEN_GAUNTLETS } } },
    { "sharedHammer", { "Shared Megaton Hammer", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_HAMMER, MM_HAMMER } } },
    { "sharedBottles", { "Shared Bottles", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_BOTTLE_RUTO_LETTER, OOT_BOTTLE_BIG_POE, OOT_BOTTLE_BLUE_FIRE, OOT_BOTTLE_CHATEAU, OOT_BOTTLE_EMPTY, OOT_BOTTLE_FAIRY, OOT_BOTTLE_MILK, OOT_BOTTLE_POE, OOT_BOTTLE_POTION_BLUE, OOT_BOTTLE_POTION_GREEN, OOT_BOTTLE_POTION_RED, OOT_BOTTLED_GOLD_DUST, OOT_WEIRD_MUSHROOM, OOT_SEAHORSE2, MM_BOTTLE_RUTO_LETTER, MM_BOTTLE_BIG_POE, MM_BOTTLE_BLUE_FIRE, MM_BOTTLE_CHATEAU, MM_BOTTLE_EMPTY, MM_BOTTLE_FAIRY, MM_BOTTLE_MILK, MM_BOTTLE_POE, MM_BOTTLE_POTION_BLUE, MM_BOTTLE_POTION_GREEN, MM_BOTTLE_POTION_RED, MM_BOTTLED_GOLD_DUST, MM_WEIRD_MUSHROOM, MM_SEAHORSE2 } } },
    { "songSoaringOot", { "Song of Soaring - OoT", ParamType::boolean, ParamCategory::standard, ShuffleSetting::vanilla, { OOT_SONG_SOARING, OOT_SONG_NOTE_SOARING } } },
    { "songHealingOot", { "Song of Healing - OoT", ParamType::boolean, ParamCategory::standard, ShuffleSetting::vanilla, { OOT_SONG_HEALING, OOT_SONG_NOTE_HEALING } } },
    { "songAwakeningOot", { "Song of Awakening - OoT", ParamType::boolean, ParamCategory::standard, ShuffleSetting::vanilla, { OOT_SONG_AWAKENING, OOT_SONG_NOTE_AWAKENING } } },
    { "songGoronOot", { "Song of Goron - OoT", ParamType::boolean, ParamCategory::standard, ShuffleSetting::vanilla, { OOT_SONG_GORON, OOT_SONG_GORON_HALF, OOT_SONG_NOTE_GORON } } },
    { "songZoraOot", { "Song of Zora - OoT", ParamType::boolean, ParamCategory::standard, ShuffleSetting::vanilla, { OOT_SONG_ZORA, OOT_SONG_NOTE_ZORA } } },
    { "songOrderOot", { "Song of Order - OoT", ParamType::boolean, ParamCategory::standard, ShuffleSetting::vanilla, { OOT_SONG_ORDER, OOT_SONG_NOTE_ORDER } } },
    { "songSunMm", { "Sun's Song - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::vanilla, { MM_SONG_SUN, MM_SONG_NOTE_SUN } } },
    { "songZeldaLullabyMm", { "Zelda's Lullaby - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::vanilla, { MM_SONG_ZELDA, MM_SONG_NOTE_ZELDA } } },
    { "songSariasMm", { "Saria's Song - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::vanilla, { MM_SONG_SARIA, MM_SONG_NOTE_SARIA } } },
    { "songMinuetMm", { "Minuet of Forest - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::vanilla, { MM_SONG_TP_FOREST, MM_SONG_NOTE_TP_FOREST } } },
    { "songBoleroMm", { "Bolero of Fire - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::vanilla, { MM_SONG_TP_FIRE, MM_SONG_NOTE_TP_FIRE } } },
    { "songSerenadeMm", { "Serenade of Water - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::vanilla, { MM_SONG_TP_WATER, MM_SONG_NOTE_TP_WATER } } },
    { "songRequiemMm", { "Requiem of Spirit - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::vanilla, { MM_SONG_TP_SPIRIT, MM_SONG_NOTE_TP_SPIRIT } } },
    { "songNocturneMm", { "Nocturne of Shadow - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::vanilla, { MM_SONG_TP_SHADOW, MM_SONG_NOTE_TP_SHADOW } } },
    { "songPreludeMm", { "Prelude of Light - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::vanilla, { MM_SONG_TP_LIGHT, MM_SONG_NOTE_TP_LIGHT } } },
    { "kamaroMaskOot", { "Kamaro Mask (OoT)", ParamType::boolean, ParamCategory::standard, ShuffleSetting::vanilla, { OOT_MASK_KAMARO } } },
    { "boomerangMm", { "Boomerang (MM)", ParamType::boolean, ParamCategory::standard, ShuffleSetting::vanilla, { MM_BOOMERANG } } },
    { "sharedMaskKamaro", { "Shared Kamaro Mask", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_MASK_KAMARO, MM_MASK_KAMARO } } },
    { "sharedBoomrang", { "Shared Boomerang", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_BOOMERANG, MM_BOOMERANG } } },
    { "sharedSongHealing", { "Shared Song of Healing", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_SONG_HEALING, OOT_SONG_NOTE_HEALING, MM_SONG_HEALING, MM_SONG_NOTE_HEALING } } },
    { "sharedSongSoaring", { "Shared Song of Soaring", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_SONG_SOARING, OOT_SONG_NOTE_SOARING, MM_SONG_SOARING, MM_SONG_NOTE_SOARING } } },
    { "sharedSongAwakening", { "Shared Sonata of Awakening", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_SONG_AWAKENING, OOT_SONG_NOTE_AWAKENING, MM_SONG_AWAKENING, MM_SONG_NOTE_AWAKENING } } },
    { "sharedSongGoron", { "Shared Goron Lullaby", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_SONG_GORON, OOT_SONG_GORON_HALF, OOT_SONG_NOTE_GORON, MM_SONG_GORON, MM_SONG_GORON_HALF, MM_SONG_NOTE_GORON } } },
    { "sharedSongZora", { "Shared New Wave Bossa Nova", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_SONG_ZORA, OOT_SONG_NOTE_ZORA, MM_SONG_ZORA, MM_SONG_NOTE_ZORA } } },
    { "sharedSongOrder", { "Shared Oath to Order", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_SONG_ORDER, OOT_SONG_NOTE_ORDER, MM_SONG_ORDER, MM_SONG_NOTE_ORDER } } },
    { "sharedSongZeldaLullaby", { "Shared Zelda's Lullaby", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_SONG_ZELDA, OOT_SONG_NOTE_ZELDA, MM_SONG_ZELDA, MM_SONG_NOTE_ZELDA } } },
    { "sharedSongSarias", { "Shared Saria's Song", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_SONG_SARIA, OOT_SONG_NOTE_SARIA, MM_SONG_SARIA, MM_SONG_NOTE_SARIA } } },
    { "sharedSongMinuet", { "Shared Minuet of Forest", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_SONG_TP_FOREST, OOT_SONG_NOTE_TP_FOREST, MM_SONG_TP_FOREST, MM_SONG_NOTE_TP_FOREST } } },
    { "sharedSongBolero", { "Shared Bolero of Fire", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_SONG_TP_FIRE, OOT_SONG_NOTE_TP_FIRE, MM_SONG_TP_FIRE, MM_SONG_NOTE_TP_FIRE } } },
    { "sharedSongSerenade", { "Shared Serenade of Water", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_SONG_TP_WATER, OOT_SONG_NOTE_TP_WATER, MM_SONG_TP_WATER, MM_SONG_NOTE_TP_WATER } } },
    { "sharedSongRequiem", { "Shared Requiem of Spirit", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_SONG_TP_SPIRIT, OOT_SONG_NOTE_TP_SPIRIT, MM_SONG_TP_SPIRIT, MM_SONG_NOTE_TP_SPIRIT } } },
    { "sharedSongNocturne", { "Shared Nocturne of Shadow", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_SONG_TP_SHADOW, OOT_SONG_NOTE_TP_SHADOW, MM_SONG_TP_SHADOW, MM_SONG_NOTE_TP_SHADOW } } },
    { "sharedSongPrelude", { "Shared Prelude of Light", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_SONG_TP_LIGHT, OOT_SONG_NOTE_TP_LIGHT, MM_SONG_TP_LIGHT, MM_SONG_NOTE_TP_LIGHT } } },
    { "powderKegOot", { "Powder Keg (OoT)", ParamType::boolean, ParamCategory::standard, ShuffleSetting::vanilla, { OOT_POWDER_KEG } } },
    { "sharedPowderKeg", { "Shared Powder Keg", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_POWDER_KEG, MM_POWDER_KEG } } },
    { "gfsOot", { "Great Fairy's Sword - OoT", ParamType::boolean, ParamCategory::standard, ShuffleSetting::vanilla, { OOT_GREAT_FAIRY_SWORD } } },
    { "sharedGFS", { "Shared Great Fairy's Sword", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_GREAT_FAIRY_SWORD, MM_GREAT_FAIRY_SWORD } } },
    { "slingshotMm", { "Slingshot - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::vanilla, { MM_SLINGSHOT, MM_DEKU_SEEDS_5, MM_DEKU_SEEDS_30, MM_BULLET_BAG2, MM_BULLET_BAG3 } } },
    { "sharedSlingshot", { "Shared Slingshot", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_SLINGSHOT, OOT_DEKU_SEEDS_5, OOT_DEKU_SEEDS_30, OOT_BULLET_BAG2, OOT_BULLET_BAG3, MM_SLINGSHOT, MM_DEKU_SEEDS_5, MM_DEKU_SEEDS_30, MM_BULLET_BAG2, MM_BULLET_BAG3 } } },
    { "gerudoMaskMm", { "Gerudo Mask - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::vanilla, { MM_MASK_GERUDO } } },
    { "sharedMaskGerudo", { "Shared Gerudo Mask", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_MASK_GERUDO, MM_MASK_GERUDO } } },
    { "skullMaskMm", { "Skull Mask - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::vanilla, { MM_MASK_SKULL } } },
    { "sharedMaskSkull", { "Shared Skull Mask", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_MASK_SKULL, MM_MASK_SKULL } } },
    { "spookyMaskMm", { "Spooky Mask - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::vanilla, { MM_MASK_SPOOKY } } },
    { "sharedMaskSpooky", { "Shared Spooky Mask", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_MASK_SPOOKY, MM_MASK_SPOOKY } } },
    { "rustyKeysOot", { "Rusty Keys - OoT", ParamType::boolean, ParamCategory::standard, ShuffleSetting::vanilla, { OOT_RUSTY_KEY_TREASURE_CHEST_GAME, OOT_RUSTY_KEY_GUARD_HOUSE, OOT_RUSTY_KEY_HYRULE_CASTLE, OOT_RUSTY_KEY_DOG_LADY_HOUSE, OOT_RUSTY_KEY_BACK_ALLEY_HOUSE, OOT_RUSTY_KEY_BOMBCHU_SHOP, OOT_RUSTY_KEY_MASK_SHOP, OOT_RUSTY_KEY_CHILD_BAZAAR, OOT_RUSTY_KEY_CHILD_POTION_SHOP, OOT_RUSTY_KEY_CHILD_SHOOTING_GALLERY, OOT_RUSTY_KEY_BOMBCHU_BOWLING, OOT_RUSTY_KEY_LABORATORY, OOT_RUSTY_KEY_FISHING_POND, OOT_RUSTY_KEY_SILO, OOT_RUSTY_KEY_RANCH_STABLE, OOT_RUSTY_KEY_RANCH_HOUSE, OOT_RUSTY_KEY_RANCH_HOUSE_ROOM, OOT_RUSTY_KEY_GRAVEYARD, OOT_RUSTY_KEY_WINDMILL, OOT_RUSTY_KEY_IMPA_HOUSE, OOT_RUSTY_KEY_CARPENTER_HOUSE, OOT_RUSTY_KEY_GRANNY_POTION_SHOP, OOT_RUSTY_KEY_ADULT_SHOOTING_GALLERY, OOT_RUSTY_KEY_SKULLTULA_HOUSE, OOT_RUSTY_KEY_ADULT_BAZAAR, OOT_RUSTY_KEY_ADULT_POTION_SHOP, OOT_RUSTY_KEY_ADULT_POTION_SHOP_BACK } } },
    { "rustyKeysMm", { "Rusty Keys - MM", ParamType::boolean, ParamCategory::standard, ShuffleSetting::vanilla, { MM_RUSTY_KEY_TOURIST_INFORMATION, MM_RUSTY_KEY_POTION_SHOP, MM_RUSTY_KEY_POST_OFFICE, MM_RUSTY_KEY_SWORDSMAN_SCHOOL, MM_RUSTY_KEY_LOTTERY, MM_RUSTY_KEY_BOMB_SHOP, MM_RUSTY_KEY_TRADING_POST, MM_RUSTY_KEY_CURIOSITY_SHOP, MM_RUSTY_KEY_KAFEI_HIDEOUT, MM_RUSTY_KEY_TOWN_ARCHERY, MM_RUSTY_KEY_SWAMP_ARCHERY, MM_RUSTY_KEY_OBSERVATORY, MM_RUSTY_KEY_BLACKSMITH, MM_RUSTY_KEY_MUSIC_HOUSE, MM_RUSTY_KEY_LABORATORY, MM_RUSTY_KEY_BENEATH_GRAVEYARD, MM_RUSTY_KEY_DAMPE_HOUSE, MM_RUSTY_KEY_MAYOR_RESIDENCE, MM_RUSTY_KEY_MAYOR_RESIDENCE_OFFICE, MM_RUSTY_KEY_MAYOR_RESIDENCE_SALON, MM_RUSTY_KEY_MAYOR_RESIDENCE_KAFEI, MM_RUSTY_KEY_TREASURE_CHEST_GAME, MM_RUSTY_KEY_HONEY_DARLING, MM_RUSTY_KEY_MILK_BAR, MM_RUSTY_KEY_DOG_RACETRACK, MM_RUSTY_KEY_CUCCO_SHACK, MM_RUSTY_KEY_RANCH_HOUSE, MM_RUSTY_KEY_RANCH_BARN, MM_RUSTY_KEY_RANCH_HOUSE_ROOM, MM_RUSTY_KEY_ZORA_SHOP, MM_RUSTY_KEY_ZORA_JAPAS_ROOM, MM_RUSTY_KEY_ZORA_TIJO_ROOM, MM_RUSTY_KEY_ZORA_LULU_ROOM, MM_RUSTY_KEY_ZORA_EVAN_ROOM, MM_RUSTY_KEY_STOCK_POT_INN, MM_RUSTY_KEY_STOCK_POT_INN_ROOF, MM_RUSTY_KEY_GRANDMA_ROOM, MM_RUSTY_KEY_STOCK_POT_INN_STAFF_ROOM, MM_RUSTY_KEY_STOCK_POT_INN_DORMITORY } } },
    { "sharedShovel", { "Shared Shovel", ParamType::boolean, ParamCategory::shared, ShuffleSetting::vanilla, { OOT_SHOVEL, MM_SHOVEL } } },
    { "shovelOot", { "Shovel - OoT", ParamType::boolean, ParamCategory::standard, ShuffleSetting::vanilla, { OOT_SHOVEL } } },
    { "shovelMm", { "Shovel - OoT", ParamType::boolean, ParamCategory::standard, ShuffleSetting::vanilla, { MM_SHOVEL } } },
    { "clocksOot", { "Clocks - OoT", ParamType::boolean, ParamCategory::standard, ShuffleSetting::vanilla, { OOT_CLOCK } } }
	});
}


void Settings::SaveFileSettings(QFile* SaveFile)
{
	QByteArray tmpVersion(sizeof(uint32_t), 0);
	QByteArray tmpParams(sizeof(qsizetype), 0);
	QByteArray tmpSetting(sizeof(uint8_t), 0);


	uint32_t currVersion = (uint32_t)AppConfig::GetTrackerVersion();

	// Save tracker version
	memcpy_s(tmpVersion.data(), 4, &currVersion, sizeof(currVersion));
	SaveFile->write(tmpVersion);

	// Save num of parameters
	qsizetype numOfParams = this->FilterSettings.size();
	memcpy_s(tmpParams.data(), sizeof(qsizetype), &numOfParams, sizeof(qsizetype));
	SaveFile->write(tmpParams);
}


size_t Settings::LoadFileSettings(QByteArray* Data, size_t Offset)
{
	// Load number of parameters
	qsizetype numParams = 0;
	memcpy_s(&numParams, sizeof(numParams), Data->data() + Offset, sizeof(numParams));
	Offset += sizeof(numParams);
	return Offset;
}


void Settings::ParseSettings(QString& SettingsSection)
{	// Split with "===" to find the right section

	QRegularExpression reg("^([^\\s].*(?:\n(?![^\\s]).*)*)", QRegularExpression::MultilineOption);
	QRegularExpressionMatchIterator it = reg.globalMatch(SettingsSection);

	QStringList sections;
	while (it.hasNext())
	{   // Fill the section array with all the gathered matches

		QRegularExpressionMatch match = it.next();
		sections.append(match.captured(1));
	}

	for (qsizetype i = 0; i < sections.size(); i++)
	{
		QString currSection = sections.at(i);

		if (currSection.startsWith("Settings"))
		{	// Settings

			// Regex to split strings by location
			QRegularExpression reg("^\\s{2}(.+)\n", QRegularExpression::MultilineOption);
			QRegularExpressionMatchIterator it = reg.globalMatch(currSection);

			QStringList settings;

			while (it.hasNext())
			{   // Fill the maps array with all the gathered matches

				QRegularExpressionMatch match = it.next();
				settings.append(match.captured(1));
				reg = QRegularExpression("^(.+):\\s(.+)", QRegularExpression::MultilineOption);
				match = reg.globalMatch(match.captured(1)).next();
				this->AddSetting(match.captured(1), match.captured(2));
			}

		}
		else if (currSection.startsWith("World"))
		{	// World flags

			this->ParseWorldFlags(currSection);
		}
        else if (currSection.startsWith("Starting Items"))
        {   // Starting items

            this->ParseStartingItems(currSection);
        }
	}
}

void Settings::ParseStartingItems(QString& LayoutSection)
{
    // Multiworld spoilers split this section per player:
    //     Starting Items
    //       Player 1
    //         Bombers' Notebook: 1
    //         ...
    //       Player 2
    //         ...
    // Single / coop keep the flat layout (items directly at two-space indent). We detect the
    // multiworld form by the presence of a "  Player N" header, route each item to its world,
    // and also keep the combined StartingItemIDs (used by the single-world code paths and as a
    // fallback when no world is active).
    bool isMultiworld = QRegularExpression("^  Player \\d", QRegularExpression::MultilineOption)
                            .match(LayoutSection).hasMatch();

    // Helper: add (itemName, count) to the combined map and, when given, to a per-world map.
    auto addItem = [this](const QString& Name, int Count, QMap<uint32_t, uint32_t>* WorldMap)
    {
        const ItemInfo* info = FindItemByName(Name);
        if (info == nullptr) return;   // Unknown item name: skip rather than crash.
        uint32_t itemID = info->ItemID;

        // Combined map (kept for single-world consumers / fallback).
        if (this->StartingItemIDs.contains(itemID)) this->StartingItemIDs.find(itemID).value() += Count;
        else                                        this->StartingItemIDs.insert(itemID, Count);

        if (WorldMap != nullptr)
        {
            if (WorldMap->contains(itemID)) WorldMap->find(itemID).value() += Count;
            else                            WorldMap->insert(itemID, Count);
        }
    };

    if (!isMultiworld)
    {   // Flat layout: items at a two-space indent.

        QRegularExpression reg("^  (.+: .+)\n", QRegularExpression::MultilineOption);
        QRegularExpressionMatchIterator it = reg.globalMatch(LayoutSection);
        while (it.hasNext())
        {
            QStringList layoutParams = it.next().captured(1).split(": ");
            if (layoutParams.size() == 2)
            {
                addItem(layoutParams.at(0), layoutParams.at(1).toInt(), nullptr);
            }
        }
        return;
    }

    // Multiworld: walk line by line, tracking the current "Player N" block. Items are indented
    // one extra level (four spaces). Store each into the matching world's map (0-based).
    QRegularExpression playerHeader("^  Player (\\d+)", QRegularExpression::MultilineOption);
    QRegularExpression itemLine("^    (.+: .+)$", QRegularExpression::MultilineOption);

    int currentWorld = -1;   // 1-based world from the header
    for (const QString& line : LayoutSection.split('\n'))
    {
        QRegularExpressionMatch hm = playerHeader.match(line);
        if (hm.hasMatch())
        {
            currentWorld = hm.captured(1).toInt();
            if (currentWorld > 0 && (size_t)currentWorld > this->StartingItemIDsByWorld.size())
            {
                this->StartingItemIDsByWorld.resize((size_t)currentWorld);
            }
            continue;
        }

        if (currentWorld <= 0) continue;

        QRegularExpressionMatch im = itemLine.match(line);
        if (!im.hasMatch()) continue;

        QStringList layoutParams = im.captured(1).split(": ");
        if (layoutParams.size() == 2)
        {
            addItem(layoutParams.at(0), layoutParams.at(1).toInt(),
                    &this->StartingItemIDsByWorld[(size_t)(currentWorld - 1)]);
        }
    }
}

void Settings::ParseWorldFlags(QString& LayoutSection)
{
    this->ParseKeyRings(LayoutSection);
    this->ParseGamesLayouts(LayoutSection);
    this->ParseSilverPouches(LayoutSection);
    this->ParseOpenDungeonsOoT(LayoutSection);
    this->ParsePreActivatedOwl(LayoutSection);
}


void Settings::ParseKeyRings(QString& LayoutSection)
{
    QRegularExpression reg("^  ((?:Small Key Ring .+):(?: \\w*|(?:\n    - .+)*))", QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator it = reg.globalMatch(LayoutSection);

    for (uint32_t i = OOT_KEY_RING_FOREST; i <= OOT_KEY_RING_GTG; i++)
    {   // Disable all OoT key rings by default

        this->DisabledItemIDs.insert(i);
    }

    this->DisabledItemIDs.insert(OOT_KEY_RING_TCG); // Don't forget the treasure chest game

    for (uint32_t i = MM_KEY_RING_WF; i <= MM_KEY_RING_ST; i++)
    {   // Disable all MM key rings by default

        this->DisabledItemIDs.insert(i);
    }

    while (it.hasNext())
    {
        QRegularExpressionMatch match = it.next();
        QStringList layoutParams = match.captured(1).split("\n");

        if (layoutParams.size() == 1)
        {	// None or all

            layoutParams = layoutParams.at(0).split(": ");
            if (layoutParams.at(0) == "Small Key Ring (OoT)")
            {	// Ocarina of Time

                if (layoutParams.at(1) == "all")
                {	// All key rings

                    for (uint32_t i = OOT_SMALL_KEY_FOREST; i <= OOT_SMALL_KEY_GTG; i++)
                    {   // Disable all OoT small keys

                        this->DisabledItemIDs.insert(i);
                    }

                    for (uint32_t i = OOT_KEY_RING_FOREST; i <= OOT_KEY_RING_GTG; i++)
                    {   // Enable all OoT key rings

                        this->DisabledItemIDs.remove(i);
                    }

                    if (this->FilterSettings["smallKeyShuffleChestGame"].Value != ShuffleSetting::vanilla)
                    {
                        this->DisabledItemIDs.insert(OOT_SMALL_KEY_TCG);
                        this->DisabledItemIDs.remove(OOT_KEY_RING_TCG);
                    }

                }
            }
            else if (layoutParams.at(0) == "Small Key Ring (MM)")
            {	// Majora's Mask

                if (layoutParams.at(1) == "all")
                {	// All key rings

                    for (uint32_t i = MM_SMALL_KEY_WF; i <= MM_SMALL_KEY_ST; i++)
                    {   // Disable all MM small keys

                        this->DisabledItemIDs.insert(i);
                    }

                    for (uint32_t i = MM_KEY_RING_WF; i <= MM_KEY_RING_ST; i++)
                    {   // Enable all MM key rings

                        this->DisabledItemIDs.remove(i);
                    }
                }
            }
        }
        else
        {
            if (layoutParams.at(0) == "Small Key Ring (OoT):")
            {	// Ocarina of Time

                QMap<QString, QPair<uint32_t, uint32_t>> possibleChoice =
                {
                    { "Forest Temple", { OOT_SMALL_KEY_FOREST, OOT_KEY_RING_FOREST } },
                    { "Fire Temple", { OOT_SMALL_KEY_FIRE, OOT_KEY_RING_FIRE } },
                    { "Water Temple", { OOT_SMALL_KEY_WATER, OOT_KEY_RING_WATER } },
                    { "Shadow Temple", { OOT_SMALL_KEY_SHADOW, OOT_KEY_RING_SHADOW } },
                    { "Spirit Temple", { OOT_SMALL_KEY_SPIRIT, OOT_KEY_RING_SPIRIT } },
                    { "Bottom of the Well", { OOT_SMALL_KEY_BOTW, OOT_KEY_RING_BOTW } },
                    { "Gerudo Training Grounds", { OOT_SMALL_KEY_GTG, OOT_KEY_RING_GTG } },
                    { "Ganon's Castle", { OOT_SMALL_KEY_GANON, OOT_KEY_RING_GANON } },
                    { "Hideout", { OOT_SMALL_KEY_GF, OOT_KEY_RING_GF } },
                    { "Chest Game", { OOT_SMALL_KEY_TCG, OOT_KEY_RING_TCG } }
                };

                for (qsizetype i = 1; i < layoutParams.size(); i++)
                {
                    QString currKey = layoutParams.at(i);
                    currKey = currKey.replace("    - ", "");

                    auto choice = possibleChoice.find(currKey);
                    if (choice != possibleChoice.end())
                    {
                        this->DisabledItemIDs.insert(choice.value().first); // Disable small keys
                        this->DisabledItemIDs.remove(choice.value().second);  // Enable key ring
                    }
                }
            }
            else if (layoutParams.at(0) == "Small Key Ring (MM):")
            {   // Majora's Mask

                QMap<QString, QPair<uint32_t, uint32_t>> possibleChoice =
                {
                    { "Woodfall Temple", { MM_SMALL_KEY_WF, MM_KEY_RING_WF } },
                    { "Snowhead Temple", { MM_SMALL_KEY_SH, MM_KEY_RING_SH } },
                    { "Great Bay Temple", { MM_SMALL_KEY_GB, MM_KEY_RING_GB } },
                    { "Stone Tower Temple", { MM_SMALL_KEY_ST, MM_KEY_RING_ST } }
                };

                for (qsizetype i = 1; i < layoutParams.size(); i++)
                {
                    QString currKey = layoutParams.at(i);
                    currKey = currKey.replace("    - ", "");

                    auto choice = possibleChoice.find(currKey);
                    if (choice != possibleChoice.end())
                    {
                        this->DisabledItemIDs.insert(choice.value().first); // Disable small keys
                        this->DisabledItemIDs.remove(choice.value().second);  // Enable key ring
                    }
                }
            }
        }
    }

    // Note: the 'removed' / 'vanilla' override for the OoT/MM/ChestGame small keys + key rings is
    // applied by ApplyItemSettings (called later from Settings::ApplySettings), so it survives the
    // SettingsTab Apply path too — ParseKeyRings only runs on spoiler load.
}


void Settings::ParseSilverPouches(QString& LayoutSection)
{
    // Which dungeon layout a silver rupee cluster belongs to. A cluster only exists
    // when its scene's active layout matches (Both = present in Vanilla and MQ alike).
    enum class SilverLayout { Vanilla, MasterQuest, Both };

    struct SilverArea
    {
        const char* Key;        // Spoiler label, matching the "Silver Rupee Pouches" per-area list.
        uint32_t RupeeId;       // Individual silver rupee item id (enabled when the area keeps its rupees).
        uint32_t PouchId;       // Silver pouch item id (enabled when the area delivers a pouch instead).
        uint32_t SceneId;       // Scene owning the cluster, used to read its active Vanilla/MQ layout.
        SilverLayout Layout;    // Layout(s) the cluster exists in.
    };

    // Per-area table: which scene each cluster lives in and in which layout it exists.
    // Derived from the OoT Vanilla / OoT MQ silver rupee location lists.
    static const SilverArea areas[] =
    {
        { "Dodongo's Cavern",         OOT_RUPEE_SILVER_DC,              OOT_POUCH_SILVER_DC,              OOT_DODONGO_CAVERN,         SilverLayout::MasterQuest },
        { "Bottom of the Well",       OOT_RUPEE_SILVER_BOTW,            OOT_POUCH_SILVER_BOTW,            OOT_BOTTOM_OF_THE_WELL,     SilverLayout::Vanilla     },
        { "Spirit Temple (Child)",    OOT_RUPEE_SILVER_SPIRIT_CHILD,    OOT_POUCH_SILVER_SPIRIT_CHILD,    OOT_TEMPLE_SPIRIT,          SilverLayout::Vanilla     },
        { "Spirit Temple (Sun)",      OOT_RUPEE_SILVER_SPIRIT_SUN,      OOT_POUCH_SILVER_SPIRIT_SUN,      OOT_TEMPLE_SPIRIT,          SilverLayout::Vanilla     },
        { "Spirit Temple (Boulders)", OOT_RUPEE_SILVER_SPIRIT_BOULDERS, OOT_POUCH_SILVER_SPIRIT_BOULDERS, OOT_TEMPLE_SPIRIT,          SilverLayout::Vanilla     },
        { "Spirit Temple (Lobby)",    OOT_RUPEE_SILVER_SPIRIT_LOBBY,    OOT_POUCH_SILVER_SPIRIT_LOBBY,    OOT_TEMPLE_SPIRIT,          SilverLayout::MasterQuest },
        { "Spirit Temple (Adult)",    OOT_RUPEE_SILVER_SPIRIT_ADULT,    OOT_POUCH_SILVER_SPIRIT_ADULT,    OOT_TEMPLE_SPIRIT,          SilverLayout::MasterQuest },
        { "Shadow Temple (Scythe)",   OOT_RUPEE_SILVER_SHADOW_SCYTHE,   OOT_POUCH_SILVER_SHADOW_SCYTHE,   OOT_TEMPLE_SHADOW,          SilverLayout::Both        },
        { "Shadow Temple (Pit)",      OOT_RUPEE_SILVER_SHADOW_PIT,      OOT_POUCH_SILVER_SHADOW_PIT,      OOT_TEMPLE_SHADOW,          SilverLayout::Both        },
        { "Shadow Temple (Spikes)",   OOT_RUPEE_SILVER_SHADOW_SPIKES,   OOT_POUCH_SILVER_SHADOW_SPIKES,   OOT_TEMPLE_SHADOW,          SilverLayout::Both        },
        { "Shadow Temple (Blades)",   OOT_RUPEE_SILVER_SHADOW_BLADES,   OOT_POUCH_SILVER_SHADOW_BLADES,   OOT_TEMPLE_SHADOW,          SilverLayout::MasterQuest },
        { "Ice Cavern (Scythe)",      OOT_RUPEE_SILVER_IC_SCYTHE,       OOT_POUCH_SILVER_IC_SCYTHE,       OOT_ICE_CAVERN,             SilverLayout::Vanilla     },
        { "Ice Cavern (Block)",       OOT_RUPEE_SILVER_IC_BLOCK,        OOT_POUCH_SILVER_IC_BLOCK,        OOT_ICE_CAVERN,             SilverLayout::Vanilla     },
        { "GTG (Slopes)",             OOT_RUPEE_SILVER_GTG_SLOPES,      OOT_POUCH_SILVER_GTG_SLOPES,      OOT_GERUDO_TRAINING_GROUND, SilverLayout::Both        },
        { "GTG (Lava)",               OOT_RUPEE_SILVER_GTG_LAVA,        OOT_POUCH_SILVER_GTG_LAVA,        OOT_GERUDO_TRAINING_GROUND, SilverLayout::Both        },
        { "GTG (Water)",              OOT_RUPEE_SILVER_GTG_WATER,       OOT_POUCH_SILVER_GTG_WATER,       OOT_GERUDO_TRAINING_GROUND, SilverLayout::Both        },
        { "Ganon's Castle (Light)",   OOT_RUPEE_SILVER_GANON_LIGHT,     OOT_POUCH_SILVER_GANON_LIGHT,     OOT_INSIDE_GANON_CASTLE,    SilverLayout::Vanilla     },
        { "Ganon's Castle (Forest)",  OOT_RUPEE_SILVER_GANON_FOREST,    OOT_POUCH_SILVER_GANON_FOREST,    OOT_INSIDE_GANON_CASTLE,    SilverLayout::Vanilla     },
        { "Ganon's Castle (Fire)",    OOT_RUPEE_SILVER_GANON_FIRE,      OOT_POUCH_SILVER_GANON_FIRE,      OOT_INSIDE_GANON_CASTLE,    SilverLayout::Both        },
        { "Ganon's Castle (Water)",   OOT_RUPEE_SILVER_GANON_WATER,     OOT_POUCH_SILVER_GANON_WATER,     OOT_INSIDE_GANON_CASTLE,    SilverLayout::MasterQuest },
        { "Ganon's Castle (Shadow)",  OOT_RUPEE_SILVER_GANON_SHADOW,    OOT_POUCH_SILVER_GANON_SHADOW,    OOT_INSIDE_GANON_CASTLE,    SilverLayout::MasterQuest },
        { "Ganon's Castle (Spirit)",  OOT_RUPEE_SILVER_GANON_SPIRIT,    OOT_POUCH_SILVER_GANON_SPIRIT,    OOT_INSIDE_GANON_CASTLE,    SilverLayout::Vanilla     },
    };

    // Step 1: disable every silver rupee and every pouch. Only the clusters that exist
    // in the active layout are re-enabled below, each in the form the seed selected.
    for (uint32_t i = OOT_RUPEE_SILVER_DC; i <= OOT_RUPEE_SILVER_GANON_WATER; i++) this->DisabledItemIDs.insert(i);
    for (uint32_t i = OOT_POUCH_SILVER_DC; i <= OOT_POUCH_SILVER_GANON_WATER; i++) this->DisabledItemIDs.insert(i);

    // Step 2: read the "Silver Rupee Pouches" setting into the set of areas delivering a
    // pouch. "all" flags every area; a per-area list flags the named ones; "none" / absent
    // leaves the set empty (every existing cluster keeps its individual rupees).
    bool allPouches = false;
    QSet<QString> pouchAreas;

    QRegularExpression reg("^  ((?:Silver Rupee Pouches):(?: \\w*|(?:\n    - .+)*))", QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator it = reg.globalMatch(LayoutSection);
    while (it.hasNext())
    {
        QRegularExpressionMatch match = it.next();
        QStringList layoutParams = match.captured(1).split("\n");

        if (layoutParams.size() == 1)
        {	// "Silver Rupee Pouches: none" / ": all"

            QStringList kv = layoutParams.at(0).split(": ");
            if (kv.size() == 2 && kv.at(0) == "Silver Rupee Pouches" && kv.at(1) == "all")
            {
                allPouches = true;
            }
        }
        else if (layoutParams.at(0) == "Silver Rupee Pouches:")
        {	// Per-area list

            for (qsizetype i = 1; i < layoutParams.size(); i++)
            {
                QString tmp = layoutParams.at(i);
                pouchAreas.insert(tmp.replace("    - ", ""));
            }
        }
    }

    // Step 3: re-enable each cluster that exists in its scene's active layout, as a pouch
    // when the setting selected it, otherwise as individual silver rupees.
    for (const SilverArea& area : areas)
    {
        SceneMetaInfo* info = GetSceneMetaInfo(area.SceneId, OOT_GAME);
        bool isMQ = (info != nullptr && info->ActiveLayout == GameLayout::oot_mq);

        bool exists = (area.Layout == SilverLayout::Both)
            || (area.Layout == SilverLayout::MasterQuest && isMQ)
            || (area.Layout == SilverLayout::Vanilla && !isMQ);
        if (!exists) continue;

        if (allPouches || pouchAreas.contains(QString::fromUtf8(area.Key)))
        {
            this->DisabledItemIDs.remove(area.PouchId);   // deliver a pouch here
        }
        else
        {
            this->DisabledItemIDs.remove(area.RupeeId);   // deliver individual silver rupees
        }
    }
}


void Settings::ParseOpenDungeonsOoT(QString& LayoutSection)
{
    QRegularExpression reg("^  ((?:Open Dungeons .+):(?: \\w*|(?:\n    - .+)*))", QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator it = reg.globalMatch(LayoutSection);

    while (it.hasNext())
    {
        QRegularExpressionMatch match = it.next();
        QStringList layoutParams = match.captured(1).split("\n");

        if (layoutParams.size() == 1)
        {	// None or all

            layoutParams = layoutParams.at(0).split(": ");
            if (layoutParams.at(0) == "Open Dungeons (OoT)")
            {	// Ocarina of Time

                if (layoutParams.at(1) == "all")
                {	// All dungeons are opened

                    this->IsFireTempleOpenAsChild = true;
                }
            }
        }
        else
        {
            if (layoutParams.at(0) == "Open Dungeons (OoT):")
            {	// Specific choice

                /*QVector<QString> possibleChoice =
                {
                    "Bottom of the Well",
                    "Bottom of the Well as Adult",
                    "Deku Tree as Adult",
                    "Dodongo's Cavern",
                    "Fire Temple as Child",
                    "Jabu-Jabu",
                    "Shadow Temple",
                    "Water Temple"
                };
                */
                for (qsizetype i = 1; i < layoutParams.size(); i++)
                {
                    QString currKey = layoutParams.at(i);
                    currKey = currKey.replace("    - ", "");

                    if (currKey == "Fire Temple as Child")
                    {
                        this->IsFireTempleOpenAsChild = true;
                        return;
                    }
                }
            }
        }
    }
}


void Settings::ParsePreActivatedOwl(QString& LayoutSection)
{
    QRegularExpression reg("^  ((?:Pre-Activated Owl Statues):(?: \\w*|(?:\n    - .+)*))", QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator it = reg.globalMatch(LayoutSection);

    while (it.hasNext())
    {
        QRegularExpressionMatch match = it.next();
        QStringList layoutParams = match.captured(1).split("\n");

        if (layoutParams.size() == 1)
        {	// None or all

            layoutParams = layoutParams.at(0).split(": ");
            if (layoutParams.at(0) == "Pre-Activated Owl Statues")
            {	// Ocarina of Time

                if (layoutParams.at(1) == "all")
                {	// All owl statues

                    for (uint32_t i = MM_OWL_GREAT_BAY; i <= MM_OWL_STONE_TOWER; i++)
                    {   // Add all owls status

                        this->StartingItemIDs.insert(i, 1);
                    }
                }
            }
        }
        else
        {
            if (layoutParams.at(0) == "Pre-Activated Owl Statues:")
            {	// Majora's Mask

                QMap<QString, uint32_t> possibleChoice =
                {
                    { "Clock Town", MM_OWL_CLOCK_TOWN },
                    { "Milk Road", MM_OWL_MILK_ROAD },
                    { "Southern Swamp", MM_OWL_SOUTHERN_SWAMP },
                    { "Woodfall", MM_OWL_WOODFALL },
                    { "Mountain Village", MM_OWL_MOUNTAIN_VILLAGE },
                    { "Snowhead", MM_OWL_SNOWHEAD },
                    { "Great Bay Coast", MM_OWL_GREAT_BAY },
                    { "Zora Cape", MM_OWL_ZORA_CAPE },
                    { "Ikana Canyon", MM_OWL_IKANA_CANYON },
                    { "Stone Tower", MM_OWL_STONE_TOWER }
                };

                for (qsizetype i = 1; i < layoutParams.size(); i++)
                {
                    QString currKey = layoutParams.at(i);
                    currKey = currKey.replace("    - ", "");

                    auto choice = possibleChoice.find(currKey);
                    if (choice != possibleChoice.end())
                    {
                        this->StartingItemIDs.insert(choice.value(), 1);
                    }
                }
            }
        }
    }
}

void Settings::ParseGamesLayouts(QString& LayoutSection)
{
	// Reset layouts
	GetSceneMetaInfo(OOT_DEKU_TREE, OOT_GAME)->ActiveLayout = GameLayout::oot;
	GetSceneMetaInfo(OOT_DODONGO_CAVERN, OOT_GAME)->ActiveLayout = GameLayout::oot;
	GetSceneMetaInfo(OOT_INSIDE_JABU_JABU, OOT_GAME)->ActiveLayout = GameLayout::oot;
	GetSceneMetaInfo(OOT_TEMPLE_FOREST, OOT_GAME)->ActiveLayout = GameLayout::oot;
	GetSceneMetaInfo(OOT_TEMPLE_FIRE, OOT_GAME)->ActiveLayout = GameLayout::oot;
	GetSceneMetaInfo(OOT_TEMPLE_WATER, OOT_GAME)->ActiveLayout = GameLayout::oot;
	GetSceneMetaInfo(OOT_TEMPLE_SHADOW, OOT_GAME)->ActiveLayout = GameLayout::oot;
	GetSceneMetaInfo(OOT_TEMPLE_SPIRIT, OOT_GAME)->ActiveLayout = GameLayout::oot;
	GetSceneMetaInfo(OOT_BOTTOM_OF_THE_WELL, OOT_GAME)->ActiveLayout = GameLayout::oot;
	GetSceneMetaInfo(OOT_GERUDO_TRAINING_GROUND, OOT_GAME)->ActiveLayout = GameLayout::oot;
	GetSceneMetaInfo(OOT_ICE_CAVERN, OOT_GAME)->ActiveLayout = GameLayout::oot;
	GetSceneMetaInfo(OOT_INSIDE_GANON_CASTLE, OOT_GAME)->ActiveLayout = GameLayout::oot;

	GetSceneMetaInfo(MM_GROTTOS, MM_GAME)->ActiveLayout = GameLayout::mm;
	GetSceneMetaInfo(MM_DEKU_PALACE, MM_GAME)->ActiveLayout = GameLayout::mm;
	GetSceneMetaInfo(MM_GROTTO_DEKU_PALACE_GENERIC, MM_GAME)->ActiveLayout = GameLayout::mm;
    GetSceneMetaInfo(MM_GROTTO_DEKU_PALACE_CLIMB, MM_GAME)->ActiveLayout = GameLayout::mm;

	QRegularExpression reg("^  ((?:Master Quest Dungeons|Majora's Mask JP Layouts):(?: \\w*|(?:\n    - .+)*))", QRegularExpression::MultilineOption);
	QRegularExpressionMatchIterator it = reg.globalMatch(LayoutSection);

	while (it.hasNext())
	{
		QRegularExpressionMatch match = it.next();
		QStringList layoutParams = match.captured(1).split("\n");

		if (layoutParams.size() == 1)
		{	// None or all

			layoutParams = layoutParams.at(0).split(": ");
			if (layoutParams.at(0) == "Master Quest Dungeons")
			{	// Ocarina of Time

				if (layoutParams.at(1) == "all")
				{	// All MQ

					GetSceneMetaInfo(OOT_DEKU_TREE, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					GetSceneMetaInfo(OOT_DODONGO_CAVERN, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					GetSceneMetaInfo(OOT_INSIDE_JABU_JABU, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					GetSceneMetaInfo(OOT_TEMPLE_FOREST, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					GetSceneMetaInfo(OOT_TEMPLE_FIRE, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					GetSceneMetaInfo(OOT_TEMPLE_WATER, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					GetSceneMetaInfo(OOT_TEMPLE_SHADOW, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					GetSceneMetaInfo(OOT_TEMPLE_SPIRIT, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					GetSceneMetaInfo(OOT_BOTTOM_OF_THE_WELL, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					GetSceneMetaInfo(OOT_ICE_CAVERN, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					GetSceneMetaInfo(OOT_GERUDO_TRAINING_GROUND, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					GetSceneMetaInfo(OOT_INSIDE_GANON_CASTLE, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
				}
			}
			else
			{	// Majora's mask

				if (layoutParams.at(1) == "all")
				{	// All JP

					GetSceneMetaInfo(MM_GROTTOS, MM_GAME)->ActiveLayout = GameLayout::mm_jp;
					GetSceneMetaInfo(MM_DEKU_PALACE, MM_GAME)->ActiveLayout = GameLayout::mm_jp;
					GetSceneMetaInfo(MM_GROTTO_DEKU_PALACE_GENERIC, MM_GAME)->ActiveLayout = GameLayout::mm_jp;
                    GetSceneMetaInfo(MM_GROTTO_DEKU_PALACE_CLIMB, MM_GAME)->ActiveLayout = GameLayout::mm_jp;
				}
			}
		}
        else
        {
            if (layoutParams.at(0) == "Master Quest Dungeons:")
            {	// Ocarina of Time

                for (qsizetype i = 1; i < layoutParams.size(); i++)
                {
                    QString currScene = layoutParams.at(i);
                    currScene = currScene.replace("    - ", "");

                    if (currScene == "Deku Tree")
                    {
						GetSceneMetaInfo(OOT_DEKU_TREE, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					}
					else if (currScene == "Dodongo cavern")
					{
						GetSceneMetaInfo(OOT_DODONGO_CAVERN, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					}
					else if (currScene == "Jabu-Jabu")
					{
						GetSceneMetaInfo(OOT_INSIDE_JABU_JABU, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					}
					else if (currScene == "Forest Temple")
					{
						GetSceneMetaInfo(OOT_TEMPLE_FOREST, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					}
					else if (currScene == "Fire Temple")
					{
						GetSceneMetaInfo(OOT_TEMPLE_FIRE, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					}
					else if (currScene == "Water Temple")
					{
						GetSceneMetaInfo(OOT_TEMPLE_WATER, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					}
					else if (currScene == "Shadow Temple")
					{
						GetSceneMetaInfo(OOT_TEMPLE_SHADOW, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					}
					else if (currScene == "Spirit Temple")
					{
						GetSceneMetaInfo(OOT_TEMPLE_SPIRIT, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					}
					else if (currScene == "Bottom of the Well")
					{
						GetSceneMetaInfo(OOT_BOTTOM_OF_THE_WELL, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					}
					else if (currScene == "Ice Cavern")
					{
						GetSceneMetaInfo(OOT_ICE_CAVERN, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					}
					else if (currScene == "Gerudo Training Grounds")
					{
						GetSceneMetaInfo(OOT_GERUDO_TRAINING_GROUND, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					}
					else if (currScene == "Ganon's Castle")
					{
						GetSceneMetaInfo(OOT_INSIDE_GANON_CASTLE, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					}
				}
			}
			else
			{	// Majora's mask

				for (qsizetype i = 1; i < layoutParams.size(); i++)
				{
					QString currScene = layoutParams.at(i);
					currScene = currScene.replace("    - ", "");

					if (currScene == "Deku Palace")
					{	// Deku Palace layout

						GetSceneMetaInfo(MM_GROTTOS, MM_GAME)->ActiveLayout = GameLayout::mm_jp;
						GetSceneMetaInfo(MM_DEKU_PALACE, MM_GAME)->ActiveLayout = GameLayout::mm_jp;
                        GetSceneMetaInfo(MM_GROTTO_DEKU_PALACE_GENERIC, MM_GAME)->ActiveLayout = GameLayout::mm_jp;
                        GetSceneMetaInfo(MM_GROTTO_DEKU_PALACE_CLIMB, MM_GAME)->ActiveLayout = GameLayout::mm_jp;
					}
				}
			}
		}
	}
}


void Settings::AddSetting(QString Name, QString Value)
{
	if (Name == "games")
	{
		if (Value == "oot")
		{
			this->Game = ROMGame::oot;
		}
		else if (Value == "mm")
		{
			this->Game = ROMGame::mm;
		}
		else
		{
			this->Game = ROMGame::ootmm;
		}
	}
	else if (Name == "mode")
	{
		if (Value == "coop")
		{
			this->Mode = GameMode::coop;
		}
		else if (Value == "multi")
		{
			this->Mode = GameMode::multi;
		}
		else
		{
			this->Mode = GameMode::single;
		}
	}
    else if (Name == "goal")
    {
        if (Value == "triforce3")
        {
            this->Goal = GoalMode::triforce3;
        }
        else if (Value == "triforce")
        {
            this->Goal = GoalMode::triforce;
        }
        else
        {
            this->Goal = GoalMode::boss;
        }
    }
	else if (Name == "teams")
	{
		this->NumOfTeams = (size_t)atoi(Value.toStdString().c_str());
	}
    else if (Name == "songs")
    {
        ShuffleSetting setting;

        if (Value == "notes")
        {
            setting = ShuffleSetting::all;
        }
        else
        {
            setting = ShuffleSetting::vanilla;
        }

        this->FilterSettings[Name].Value = setting;
    }
    else if (this->FilterSettings.contains(Name))
	{
		ShuffleSetting setting;

        if (Value == "all" || Value == "true" || Value == "full" || Value == "anywhere" || Value == "ganon" || Value == "child" || Value == "cross" || Value == "day" || Value == "night" || Value.toInt() > 0)
		{
			setting = ShuffleSetting::all;
		}
        else if (Value == "starting")
        {
            setting = ShuffleSetting::starting;
        }
		else if (Value == "dungeons" || Value == "ownDungeon")
		{
			setting = ShuffleSetting::dungeons;
		}
		else if (Value == "overworld")
		{
			setting = ShuffleSetting::overworld;
		}
        else if (Value == "removed")
        {
            setting = ShuffleSetting::removed;
        }
		else
		{
			setting = ShuffleSetting::vanilla;
		}

		this->FilterSettings[Name].Value = setting;
	}
    else if (this->ItemSettings.contains(Name))
    {
        ShuffleSetting setting;

        if (Value == "progressive" || Value == "all" || Value == "true" || Value == "ascending" || Value.toInt() > 0)
        {
            setting = ShuffleSetting::all;
        }
        else if (Value == "goron" || Value == "descending")
        {
            setting = ShuffleSetting::overworld;
        }
        else
        {
            setting = ShuffleSetting::vanilla;
        }

        this->ItemSettings[Name].Value = setting;
    }
}


void Settings::ApplySettings(FilterManager* FilterOoT, FilterManager* FilterMM)
{
	switch (this->Game)
	{
		case ROMGame::oot:
		{
			this->ApplyOoTSettingsToFilter(FilterOoT);
			break;
		}

		case ROMGame::mm:
		{
			this->ApplyMMSettingsToFilter(FilterMM);
			break;
		}

		case ROMGame::ootmm:
		{
			this->ApplyOoTSettingsToFilter(FilterOoT);
			this->ApplyMMSettingsToFilter(FilterMM);
			break;
		}

		default:
		{
			break;
		}
	}

    this->ApplyItemSettings();
}

void Settings::ApplyOoTSettingsToFilter(FilterManager* Filter)
{
	SceneObjects* scenes = this->ApplySettingsToFilter(Filter, OOT_GAME);

	for (size_t i = 0; i < OOT_NUM_SCENES; i++)
	{	// Browse all OoT scenes

		SceneObjects* currScene = &scenes[i];

		for (size_t j = 0; j < currScene->NumOfObjs; j++)
		{
			ObjectInfo* currObj = &currScene->Objects[j];

			switch (currObj->RenderType)
			{	
				case ObjectType::gs:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["goldSkulltulaTokens"].Value, Filter);
                    if (this->FilterSettings["goldSkulltulaTokens"].Value == ShuffleSetting::vanilla)
                    {   // Set the object item to gs token

                        currObj->Item = FindItem(OOT_GS_TOKEN);
                    }
					break;
				}

				case ObjectType::map:
                {
                    this->CheckObjectExclusion(currObj, this->FilterSettings["mapCompassShuffle"].Value, Filter);
                    break;
                }

				case ObjectType::compass:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["mapCompassShuffle"].Value, Filter);
					break;
				}

				case ObjectType::small_key:
				{
					if (currObj->Scene == OOT_THIEVES_HIDEOUT)
					{	// Hideout small key

						this->CheckObjectExclusion(currObj, this->FilterSettings["smallKeyShuffleHideout"].Value, Filter);
					}
					else if (currObj->LocationType == LocType::minigame)
					{	// Treasure chest small key

						this->CheckObjectExclusion(currObj, this->FilterSettings["smallKeyShuffleChestGame"].Value, Filter);
					}
					else
					{	// Dungeon small key

						this->CheckObjectExclusion(currObj, this->FilterSettings["smallKeyShuffleOot"].Value, Filter);
					}
					break;
				}

				case ObjectType::boss_key:
				{
					if (currObj->Scene == OOT_GANON_TOWER)
					{	// Ganon boss key

						this->CheckObjectExclusion(currObj, this->FilterSettings["ganonBossKey"].Value, Filter);
					}
					else
					{	// Other boss key

						this->CheckObjectExclusion(currObj, this->FilterSettings["bossKeyShuffleOot"].Value, Filter);
					}
					break;
				}

				case ObjectType::sr:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["silverRupeeShuffle"].Value, Filter);
					break;
				}

				case ObjectType::scrub:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["scrubShuffleOot"].Value, Filter);
					break;
				}

				case ObjectType::cow:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["cowShuffleOot"].Value, Filter);
					break;
				}

				case ObjectType::shop:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shopShuffleOot"].Value, Filter);
					break;
				}

				case ObjectType::pot:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shufflePotsOot"].Value, Filter);
					break;
				}

				case ObjectType::crate:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleCratesOot"].Value, Filter);
					break;
				}

				case ObjectType::hive:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleHivesOot"].Value, Filter);
					break;
				}

				case ObjectType::grass:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleGrassOot"].Value, Filter);
					break;
				}

				case ObjectType::rock:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleRocksOot"].Value, Filter);
					break;
				}

				case ObjectType::tree:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleTreesOot"].Value, Filter);
					break;
				}

				case ObjectType::bush:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleBushOot"].Value, Filter);
					break;
				}

				case ObjectType::soil:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleSoilOot"].Value, Filter);
					break;
				}

				case ObjectType::rupee:
				{
					if (currObj->LocationType == LocType::minigame)
					{	// Diving game rupee

						this->CheckObjectExclusion(currObj, this->FilterSettings["divingGameRupeeShuffle"].Value, Filter);
					}
					else
					{	// Normal freestanding rupee

						this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleFreeRupeesOot"].Value, Filter);
					}
					break;
				}

				case ObjectType::heart:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleFreeHeartsOot"].Value, Filter);
					break;
				}

				case ObjectType::wonder:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleWonderItemsOot"].Value, Filter);
					break;
				}

				case ObjectType::butterfly:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleButterfliesOot"].Value, Filter);
					break;
				}

                case ObjectType::boulder:
                {
                    this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleBouldersOot"].Value, Filter);
                    break;
                }

                case ObjectType::silverboulder:
                {
                    if (currObj->Scene == OOT_DEATH_MOUNTAIN_CRATER && (this->FilterSettings["agelessStrength"].Value != ShuffleSetting::all || this->IsFireTempleOpenAsChild))
                    {   // Special case for death mountain crater silver boulder that only appear when child. Silver / Golden Gauntlets required

                        this->CheckObjectExclusion(currObj, ShuffleSetting::vanilla, Filter);
                    }
                    else
                    {
                        this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleSilverBouldersOot"].Value, Filter);
                    }
                    break;
                }

				case ObjectType::redboulder:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleRedBouldersOot"].Value, Filter);
					break;
				}

				case ObjectType::icicle:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleIciclesOot"].Value, Filter);
					break;
				}

				case ObjectType::redice:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleRedIceOot"].Value, Filter);
					break;
				}
				
				case ObjectType::ocarina:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleOcarinasOot"].Value, Filter);
					break;
				}

				case ObjectType::sword:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleMasterSword"].Value, Filter);
					break;
				}

				case ObjectType::mask:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleMaskTrades"].Value, Filter);
					break;
				}

				case ObjectType::merchant:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleMerchantsOot"].Value, Filter);
					break;
				}

				case ObjectType::fish:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["pondFishShuffle"].Value, Filter);
					break;
				}

				case ObjectType::fairy:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["fairyFountainFairyShuffleOot"].Value, Filter);
					break;
				}

				case ObjectType::fairy_spot:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["fairySpotShuffleOot"].Value, Filter);
					break;
				}

				case ObjectType::egg:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["eggShuffle"].Value, Filter);
					break;
				}

				case ObjectType::npc:
				{
					switch (currObj->MapIcon)
					{
						case EGameIcon::card:
						{	// Check for gerudo card

							this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleGerudoCard"].Value, Filter);
							break;
						}

						case EGameIcon::frog:
						{	// Check for frog rupees rewards

							if (currObj->LocationType == LocType::minigame)
							{
								this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleFrogsRupeesOot"].Value, Filter);
							}
							break;
						}

                        case EGameIcon::letter:
                        {

                            this->CheckObjectExclusion(currObj, this->FilterSettings["skipZelda"].Value == ShuffleSetting::vanilla ? ShuffleSetting::all : ShuffleSetting::vanilla, Filter);
                            break;
                        }

                        case EGameIcon::gold_rupee:
                        {   // 100 GS reward
                            
                            this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleSkulltulaFinalReward"].Value, Filter);
                            break;
                        }
					}

                    break;
				}

                case ObjectType::song:
                {
                    if (currObj->RenderScene == OOT_CASTLE_COURTYARD)
                    {   // Only filter zelda's song object

                        this->CheckObjectExclusion(currObj, this->FilterSettings["skipZelda"].Value == ShuffleSetting::vanilla ? ShuffleSetting::all : ShuffleSetting::vanilla, Filter);
                    }
                    break;
                }

				case ObjectType::none:
				default:
				{	// We don't want to overload the filter with non rendered object anyway

					break;
				}
			}
		}
	}

	if (this->FilterSettings["restoreBrokenActors"].Value != ShuffleSetting::all)
	{	// Restore broken actor is not active. Hide broken actors

		uint32_t sceneToCheck[3] = { OOT_DODONGO_CAVERN, OOT_HYRULE_CASTLE, OOT_LAKE_HYLIA };

		for (uint32_t i = 0; i < 3; i++)
		{	// Browse all scenes that contains broken actors

			for (uint32_t j = 0; j < scenes[sceneToCheck[i]].NumOfObjs; j++)
			{	// Find all broken actors

				ObjectInfo* currObj = &scenes[sceneToCheck[i]].Objects[j];

				if (currObj->LocationType == LocType::broken)
				{	// The actor is broken

					Filter->ExcludeNewObject(currObj);
				}
			}
		}
	}
}

void Settings::ApplyMMSettingsToFilter(FilterManager* Filter)
{
	SceneObjects* scenes = this->ApplySettingsToFilter(Filter, MM_GAME);

	for (size_t i = 0; i < MM_NUM_SCENES; i++)
	{	// Browse all OoT scenes

		SceneObjects* currScene = &scenes[i];

		for (size_t j = 0; j < currScene->NumOfObjs; j++)
		{
			ObjectInfo* currObj = &currScene->Objects[j];

			switch (currObj->RenderType)
			{
				case ObjectType::gs:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["housesSkulltulaTokens"].Value, Filter);
					break;
				}

				case ObjectType::map:
				{
					if (currObj->Type == ObjectType::npc)
					{	// Tingle maps

						this->CheckObjectExclusion(currObj, this->FilterSettings["tingleShuffle"].Value, Filter);
						break;
					}
				}
				case ObjectType::compass:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["mapCompassShuffle"].Value, Filter);
					break;
				}

				case ObjectType::small_key:
				{	// Dungeon small key

					this->CheckObjectExclusion(currObj, this->FilterSettings["smallKeyShuffleMm"].Value, Filter);
					break;
				}

				case ObjectType::boss_key:
				{	// Boss key

					this->CheckObjectExclusion(currObj, this->FilterSettings["bossKeyShuffleMm"].Value, Filter);
					break;
				}

				case ObjectType::sf:
				{
					if (currObj->ObjectID == STRAY_FAIRY_TOWN)
					{	// Stray fairy town

						this->CheckObjectExclusion(currObj, this->FilterSettings["townFairyShuffle"].Value, Filter);
					}
					else if (currObj->Type == ObjectType::chest)
					{	// Stray fairy in a chest

						this->CheckObjectExclusion(currObj, this->FilterSettings["strayFairyChestShuffle"].Value, Filter);
                        if (this->FilterSettings["strayFairyChestShuffle"].Value == ShuffleSetting::starting)
                        {
                            uint32_t itemID = MM_STRAY_FAIRY;
                            switch (currObj->Scene)
                            {
                                case MM_TEMPLE_WOODFALL:
                                {
                                    itemID = MM_STRAY_FAIRY_WF;
                                    break;
                                }

                                case MM_TEMPLE_SNOWHEAD:
                                {
                                    itemID = MM_STRAY_FAIRY_SH;
                                    break;
                                }
                                case MM_TEMPLE_GREAT_BAY:
                                {
                                    itemID = MM_STRAY_FAIRY_GB;
                                    break;
                                }
                                case MM_STONE_TOWER:
                                case MM_STONE_TOWER_INVERTED:
                                {
                                    itemID = MM_STRAY_FAIRY_ST;
                                    break;
                                }

                                default:
                                    continue;
                            }

                            if (this->StartingItemIDs.contains(itemID))
                            {
                                this->StartingItemIDs.find(itemID).value()++;
                            }
                            else
                            {
                                this->StartingItemIDs.insert(itemID, 1);
                            }
                        }
					}
					else
					{	// All other stray fairies

						this->CheckObjectExclusion(currObj, this->FilterSettings["strayFairyOtherShuffle"].Value, Filter);
                        if (this->FilterSettings["strayFairyOtherShuffle"].Value == ShuffleSetting::starting)
                        {
                            uint32_t itemID = MM_STRAY_FAIRY;
                            switch (currObj->Scene)
                            {
                                case MM_TEMPLE_WOODFALL:
                                {
                                    itemID = MM_STRAY_FAIRY_WF;
                                    break;
                                }

                                case MM_TEMPLE_SNOWHEAD:
                                {
                                    itemID = MM_STRAY_FAIRY_SH;
                                    break;
                                }
                                case MM_TEMPLE_GREAT_BAY:
                                {
                                    itemID = MM_STRAY_FAIRY_GB;
                                    break;
                                }
                                case MM_STONE_TOWER:
                                case MM_STONE_TOWER_INVERTED:
                                {
                                    itemID = MM_STRAY_FAIRY_ST;
                                    break;
                                }

                                default:
                                    continue;
                            }

                            if (this->StartingItemIDs.contains(itemID))
                            {
                                this->StartingItemIDs.find(itemID).value()++;
                            }
                            else
                            {
                                this->StartingItemIDs.insert(itemID, 1);
                            }
                        }
					}
					break;
				}

				case ObjectType::scrub:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["scrubShuffleMm"].Value, Filter);
					break;
				}

				case ObjectType::cow:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["cowShuffleMm"].Value, Filter);
					break;
				}

				case ObjectType::shop:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shopShuffleMm"].Value, Filter);
					break;
				}

				case ObjectType::owl:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["owlShuffle"].Value, Filter);
					break;
				}

				case ObjectType::pot:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shufflePotsMm"].Value, Filter);
					break;
				}

				case ObjectType::crate:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleCratesMm"].Value, Filter);
					break;
				}

				case ObjectType::barrel:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleBarrelsMm"].Value, Filter);
					break;
				}

				case ObjectType::hive:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleHivesMm"].Value, Filter);
					break;
				}

				case ObjectType::rock:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleRocksMm"].Value, Filter);
					break;
				}

				case ObjectType::grass:
				{
					if (currObj->Scene == MM_TERMINA_FIELD)
					{	// Termina field grass

						this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleTFGrassMm"].Value, Filter);
					}
					else
					{	// All other grass

						this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleGrassMm"].Value, Filter);
					}
					break;
				}

				case ObjectType::tree:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleTreesMm"].Value, Filter);
					break;
				}

				case ObjectType::bush:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleBushMm"].Value, Filter);
					break;
				}

				case ObjectType::soil:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleSoilMm"].Value, Filter);
					break;
				}

				case ObjectType::rupee:
				{	// Normal freestanding rupee

					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleFreeRupeesMm"].Value, Filter);
					break;
				}

				case ObjectType::heart:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleFreeHeartsMm"].Value, Filter);
					break;
				}

				case ObjectType::wonder:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleWonderItemsMm"].Value, Filter);
					break;
				}

				case ObjectType::snowball:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleSnowballsMm"].Value, Filter);
					break;
				}

				case ObjectType::butterfly:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleButterfliesMm"].Value, Filter);
					break;
				}

                case ObjectType::boulder:
                {
                    this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleBouldersMm"].Value, Filter);
                    break;
                }

				case ObjectType::redboulder:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleRedBouldersMm"].Value, Filter);
					break;
				}

				case ObjectType::icicle:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleIciclesMm"].Value, Filter);
					break;
				}

				case ObjectType::merchant:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleMerchantsMm"].Value, Filter);
					break;
				}

				case ObjectType::fairy:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["fairyFountainFairyShuffleMm"].Value, Filter);
					break;
				}

				case ObjectType::npc:
				{
					if (currObj->Scene == MM_LOTTERY)
					{	// Lottery

						this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleLotteryMm"].Value, Filter);
					}
                    break;
				}

				case ObjectType::none:
				default:
				{	// We don't want to overload the filter with non rendered object anyway

					break;
				}
			}
		}
	}
}

void Settings::ApplyItemSettings()
{
    if (this->Goal == GoalMode::triforce)
    {
        this->DisabledItemIDs.insert(OOT_TRIFORCE_POWER);
        this->DisabledItemIDs.insert(OOT_TRIFORCE_COURAGE);
        this->DisabledItemIDs.insert(OOT_TRIFORCE_WISDOM);
    }
    else if (this->Goal == GoalMode::triforce3)
    {
        this->DisabledItemIDs.insert(OOT_TRIFORCE);
        this->DisabledItemIDs.insert(OOT_TRIFORCE_FULL);
    }
    else
    {
        this->DisabledItemIDs.insert(OOT_TRIFORCE);
        this->DisabledItemIDs.insert(OOT_TRIFORCE_FULL);
        this->DisabledItemIDs.insert(OOT_TRIFORCE_POWER);
        this->DisabledItemIDs.insert(OOT_TRIFORCE_COURAGE);
        this->DisabledItemIDs.insert(OOT_TRIFORCE_WISDOM);
    }

    if (this->FilterSettings["mapCompassShuffle"].Value == ShuffleSetting::removed)
    {
        for (uint32_t i = OOT_MAP_DT; i <= OOT_COMPASS_IC; i++)
        {   // Disable all OoT maps / compasses

            this->DisabledItemIDs.insert(i);
        }

        for (uint32_t i = MM_MAP_WF; i <= MM_COMPASS_ST; i++)
        {   // Disable all MM maps / compasses

            this->DisabledItemIDs.insert(i);
        }
    }

    if (this->FilterSettings["tingleShuffle"].Value == ShuffleSetting::removed)
    {
        for (uint32_t i = MM_WORLD_MAP_CLOCK_TOWN; i <= MM_WORLD_MAP_STONE_TOWER; i++)
        {   // Disable all MM tingle maps

            this->DisabledItemIDs.insert(i);
        }
    }

    if (this->FilterSettings["ganonBossKey"].Value == ShuffleSetting::removed)
    {
        this->DisabledItemIDs.insert(OOT_BOSS_KEY_GANON);
    }

    if (this->FilterSettings["bossKeyShuffleOot"].Value == ShuffleSetting::removed)
    {
        for (uint32_t i = OOT_BOSS_KEY_FOREST; i <= OOT_BOSS_KEY_SHADOW; i++)
        {
            this->DisabledItemIDs.insert(i);
        }
    }

    if (this->FilterSettings["bossKeyShuffleMm"].Value == ShuffleSetting::removed)
    {
        for (uint32_t i = MM_BOSS_KEY_WF; i <= MM_BOSS_KEY_ST; i++)
        {
            this->DisabledItemIDs.insert(i);
        }
    }

    // Small keys + key rings: 'removed' means the rolled seed contains neither the small key nor
    // its ring (Chest Game and Hideout follow their own boolean below). Mirrors the bossKey /
    // mapCompass blocks above so the spoiler-load and SettingsTab paths share one source of truth.
    if (this->FilterSettings["smallKeyShuffleOot"].Value == ShuffleSetting::removed)
    {
        for (uint32_t i = OOT_SMALL_KEY_FOREST; i <= OOT_SMALL_KEY_GTG; i++)
        {
            this->DisabledItemIDs.insert(i);
        }
        for (uint32_t i = OOT_KEY_RING_FOREST; i <= OOT_KEY_RING_GTG; i++)
        {
            this->DisabledItemIDs.insert(i);
        }
    }

    if (this->FilterSettings["smallKeyShuffleMm"].Value == ShuffleSetting::removed)
    {
        for (uint32_t i = MM_SMALL_KEY_WF; i <= MM_SMALL_KEY_ST; i++)
        {
            this->DisabledItemIDs.insert(i);
        }
        for (uint32_t i = MM_KEY_RING_WF; i <= MM_KEY_RING_ST; i++)
        {
            this->DisabledItemIDs.insert(i);
        }
    }

    // Note: Chest Game is intentionally NOT force-hidden here even when smallKeyShuffleChestGame
    // is vanilla. The "Small Key Ring (OoT)" parser (or the user's per-dungeon checkbox via the
    // World Items page) already decides whether OOT_KEY_RING_TCG / OOT_SMALL_KEY_TCG are tracked;
    // overriding it here was destroying the user's explicit choice when the ring is enabled but
    // the boolean is off.

    if (this->FilterSettings["skipZelda"].Value == ShuffleSetting::all)
    {
        this->DisabledItemIDs.insert(OOT_CHICKEN);
        this->StartingItemIDs.insert(OOT_SONG_ZELDA, 1);
        this->StartingItemIDs.insert(OOT_ZELDA_LETTER, 1);
    }

    for (auto it = this->ItemSettings.cbegin(); it != this->ItemSettings.cend(); ++it)
    {
        switch (it->Cat)
        {
            case ParamCategory::standard:
            {
                if (it->AffectedVal.size() > 0 && !this->CheckItemEnabled(it->Value, it->AffectedVal.at(0)))
                {
                    for (uint32_t i = 1; i < it->AffectedVal.size(); i++)
                    {
                        this->DisabledItemIDs.insert(it->AffectedVal.at(i));
                    }
                }
                break;
            }

            case ParamCategory::progressive:
            {
                for (uint32_t i = 0; i < it->AffectedVal.size(); i++)
                {
                    this->ProgressiveItemIDs.insert(it->AffectedVal.at(i));
                }
                break;
            }

            case ParamCategory::souls:
            {
                if (it->AffectedVal.size() == 2 && !this->CheckItemEnabled(it->Value, it->AffectedVal.at(0)))
                {
                    for (uint32_t i = it->AffectedVal.at(0) + 1; i <= it->AffectedVal.at(1); i++)
                    {
                        this->DisabledItemIDs.insert(i);
                    }
                }
                break;
            }

            case ParamCategory::shared:
            {
                for (uint32_t i = 0; i < it->AffectedVal.size(); i++)
                {
                    this->SharedItemIDs.insert(it->AffectedVal.at(i));
                }
                break;
            }
        }
    }
}

void Settings::CheckObjectExclusion(ObjectInfo* ToCheck, ShuffleSetting SettingValue, FilterManager* Filter)
{
	switch (SettingValue)
	{
        case ShuffleSetting::removed:
		case ShuffleSetting::vanilla:
		{
			Filter->ExcludeNewObject(ToCheck);
			break;
		}

		case ShuffleSetting::overworld:
		{
			if (ToCheck->LocationType != LocType::overworld)
			{
				Filter->ExcludeNewObject(ToCheck);
			}
			break;
		}

		case ShuffleSetting::dungeons:
		{
			if (ToCheck->LocationType != LocType::dungeon)
			{
				Filter->ExcludeNewObject(ToCheck);
			}
			break;
		}

		default:
			break;
	}
}


bool Settings::CheckItemEnabled(ShuffleSetting SettingValue, uint32_t ItemID)
{
    switch (SettingValue)
    {
        case ShuffleSetting::vanilla:
        {
            this->DisabledItemIDs.insert(ItemID);
            return false;
        }
    }

    return true;
}


SceneObjects* Settings::ApplySettingsToFilter(FilterManager* Filter, uint32_t Game)
{
	Filter->ResetExcludedObject();
	return GetGameSceneObjects(Game);
}
