#pragma once

#include "Multi/API.h"

typedef struct SceneMetaInfo
{
	const char* Name;
	const char* ImagePath;
} SceneMetaInfo;

#define DEKU_TREE								0x00
#define DODONGO_CAVERN							0x01
#define INSIDE_JABU_JABU						0x02
#define TEMPLE_FOREST							0x03
#define TEMPLE_FIRE								0x04
#define TEMPLE_WATER							0x05
#define TEMPLE_SPIRIT							0x06
#define TEMPLE_SHADOW							0x07
#define BOTTOM_OF_THE_WELL						0x08
#define ICE_CAVERN								0x09
#define GANON_TOWER								0x0a
#define GERUDO_TRAINING_GROUND					0x0b
#define THIEVES_HIDEOUT							0x0c
#define INSIDE_GANON_CASTLE						0x0d
#define GANON_TOWER_COLLAPSING					0x0e
#define INSIDE_GANON_CASTLE_COLLAPSING			0x0f
#define OOT_TREASURE_SHOP						0x10
#define LAIR_GOHMA								0x11
#define LAIR_KING_DODONGO						0x12
#define LAIR_BARINADE							0x13
#define LAIR_PHANTOM_GANON						0x14
#define LAIR_VOLVAGIA							0x15
#define LAIR_MORPHA								0x16
#define LAIR_TWINROVA							0x17
#define LAIR_BONGO_BONGO						0x18
#define LAIR_GANONDORF							0x19
#define TOWER_COLLAPSE_EXTERIOR					0x1a
#define MARKET_ENTRANCE_CHILD_DAY				0x1b
#define MARKET_ENTRANCE_CHILD_NIGHT				0x1c
#define MARKET_ENTRANCE_ADULT					0x1d
#define BACK_ALLEY_DAY							0x1e
#define BACK_ALLEY_NIGHT						0x1f
#define MARKET_CHILD_DAY						0x20
#define MARKET_CHILD_NIGHT						0x21
#define MARKET_ADULT							0x22
#define TEMPLE_OF_TIME_EXTERIOR_CHILD_DAY		0x23
#define TEMPLE_OF_TIME_EXTERIOR_CHILD_NIGHT		0x24
#define TEMPLE_OF_TIME_EXTERIOR_ADULT			0x25
#define KOKIRI_KNOW_IT_ALL						0x26
#define KOKIRI_TWINS							0x27
#define KOKIRI_MIDO								0x28
#define KOKIRI_SARIA							0x29
#define CARPENTER_BOSS_HOUSE					0x2a
#define BACK_ALLEY_HOUSE						0x2b
#define BAZAAR									0x2c
#define KOKIRI_SHOP								0x2d
#define OOT_GORON_SHOP							0x2e
#define ZORA_SHOP								0x2f
#define KAKARIKO_POTION_SHOP					0x30
#define MARKET_POTION_SHOP						0x31
#define BOMBCHU_SHOP							0x32
#define HAPPY_MASK_SHOP							0x33
#define LINK_HOUSE								0x34
#define BACK_ALLEY_HOUSE2						0x35
#define STABLE									0x36
#define IMPA_HOUSE								0x37
#define OOT_LABORATORY							0x38
#define CARPENTER_TENT							0x39
#define GRAVEKEEPER_HUT							0x3a
#define GREAT_FAIRY_FOUNTAIN_UPGRADES			0x3b
#define OOT_FAIRY_FOUNTAIN						0x3c
#define GREAT_FAIRY_FOUNTAIN_SPELLS				0x3d
#define OOT_GROTTOS								0x3e
#define TOMB_REDEAD								0x3f
#define TOMB_FAIRY								0x40
#define TOMB_ROYAL								0x41
#define OOT_SHOOTING_GALLERY					0x42
#define TEMPLE_OF_TIME							0x43
#define CHAMBER_OF_THE_SAGES					0x44
#define CASTLE_MAZE_DAY							0x45
#define CASTLE_MAZE_NIGHT						0x46
#define OOT_CUTSCENE_MAP						0x47
#define TOMB_DAMPE_WINDMILL						0x48
#define FISHING_POND							0x49
#define CASTLE_COURTYARD						0x4a
#define BOMBCHU_BOWLING_ALLEY					0x4b
#define RANCH_HOUSE_SILO						0x4c
#define GUARD_HOUSE								0x4d
#define GRANNY_POTION_SHOP						0x4e
#define GANON_BATTLE_ARENA						0x4f
#define HOUSE_OF_SKULLTULA						0x50
#define HYRULE_FIELD							0x51
#define KAKARIKO_VILLAGE						0x52
#define GRAVEYARD								0x53
#define ZORA_RIVER								0x54
#define KOKIRI_FOREST							0x55
#define SACRED_FOREST_MEADOW					0x56
#define LAKE_HYLIA								0x57
#define ZORA_DOMAIN								0x58
#define ZORA_FOUNTAIN							0x59
#define GERUDO_VALLEY							0x5a
#define OOT_LOST_WOODS							0x5b
#define DESERT_COLOSSUS							0x5c
#define GERUDO_FORTRESS							0x5d
#define HAUNTED_WASTELAND						0x5e
#define HYRULE_CASTLE							0x5f
#define DEATH_MOUNTAIN_TRAIL					0x60
#define DEATH_MOUNTAIN_CRATER					0x61
#define GORON_CITY								0x62
#define LON_LON_RANCH							0x63
#define GANON_CASTLE_EXTERIOR					0x64

// OoT detailed grotto scene
#define OOT_GROTTO_KOKIRI_FOREST_STORMS			0x65	// Kokiri forest song of storms grotto
#define OOT_GROTTO_LOST_WOODS_SCRUB_UPGRADE		0x66	// Los woods nut upgrade grotto
#define OOT_GROTTO_LOST_WOODS_GENERIC			0x67	// Lost woods generic grotto
#define OOT_GROTTO_LOST_WOODS_THEATER			0x68	// Lost woods theater grotto
#define OOT_GROTTO_SACRED_MEADOW_WOLFOS			0x69	// Sacred Meadow wolfos grotto
#define OOT_GROTTO_SACRED_MEADOW_STORMS			0x70	// Sacred Meadow song of storms grotto
#define OOT_GROTTO_KAKARIKO_REDEAD				0x71	// Kakariko redead grotto
#define OOT_GROTTO_KAKARIKO_OPEN				0x72	// Kakariko open grotto
#define OOT_GROTTO_DEATH_TRIAL_STORMS			0x73	// Death mountain trial song of storms grotto
#define OOT_GROTTO_DEATH_TRIAL_COW				0x74	// Death mountain trial cow grotto
#define OOT_GROTTO_GORON_CITY_SCRUBS			0x75	// Goron city scrubs grotto
#define OOT_GROTTO_DEATH_CRATER_GENERIC			0x76	// Death mountain crater generic grotto
#define OOT_GROTTO_DEATH_CRATER_SCRUBS			0x77	// Death mountain crater scrubs grotto
#define OOT_GROTTO_ZORA_RIVER_STORMS			0x78	// Zora's river song of storms grotto
#define OOT_GROTTO_ZORA_RIVER_GENERIC			0x79	// Zora's river generic grotto
#define OOT_GROTTO_LAKE_HYLIA_SCRUBS			0x7a	// Lake Hylia scrubs grotto
#define OOT_GROTTO_LON_LON_SCRUBS				0x7b	// Lon Lon's Ranch scrubs 
#define OOT_GROTTO_HYRULE_SCRUBS				0x7c	// Hyrule field scrub grotto
#define OOT_GROTTO_HYRULE_SE					0x7d	// Hyrule field southeast grotto
#define OOT_GROTTO_HYRULE_OPEN					0x7e	// Hyrule field open grotto
#define OOT_GROTTO_HYRULE_MARKET				0x7f	// Hyrule field market grotto
#define OOT_GROTTO_HYRULE_TEKTITE				0x80	// Hyrule field tektite grotto
#define OOT_GROTTO_HYRULE_KAKARIKO				0x81	// Hyrule field kakariko grotto
#define OOT_GROTTO_HYRULE_GERUDO				0x82	// Hyrule field gerudo grotto
#define OOT_GROTTO_CASTLE_STORMS				0x83	// Hyrule castle song of storms grotto
#define OOT_GROTTO_VALLEY_STORMS				0x84	// Hyrule castle song of storms grotto
#define OOT_GROTTO_VALLEY_OCTOROK				0x85	// Hyrule castle octorok grotto
#define OOT_GROTTO_DESERT_SCRUBS				0x86	// Desert colossus scrubs grotto

// OoT detailed fairy scene
#define OOT_FAIRY_SACRED_MEADOW					0x87	// Sacred meadow fairy fountain
#define OOT_FAIRY_ZORA_RIVER					0x88	// Zora's river fairy fountain
#define OOT_FAIRY_ZORA_DOMAIN					0x89	// Zora's domain fairy fountain
#define OOT_FAIRY_HYRULE						0x8a	// Hyrule field fairy fountain
#define OOT_FAIRY_GERUDO_FORTRESS				0x8b	// Gerudo fortress fairy fountain

// OoT detailed great fairy scene
#define OOT_GREAT_FAIRY_DIN						0x8c	// Din's fire fairy fountain
#define OOT_GREAT_FAIRY_FARORE					0x8d	// Farore's wind fairy fountain
#define OOT_GREAT_FAIRY_NAYRU					0x8e	// Nayru's love fairy fountain
#define OOT_GREAT_FAIRY_MAGIC					0x8f	// First magic upgrade fairy fountain
#define OOT_GREAT_FAIRY_MAGIC2					0x90	// Second magic upgrade fairy fountain
#define OOT_GREAT_FAIRY_DEFENSE					0x91	// Double defense upgrade fairy fountain

// OoT detailed other scene
#define KAKARIKO_BAZAAR							0x92	// Kakariko's bazaar
#define MARKET_BAZAAR							0x93	// Market bazaar
#define KAKARIKO_SHOOTING						0x94	// Kakariko's shooting gallery
#define MARKET_SHOOTING							0x95	// Kakariko's shooting gallery
#define SILO									0x96	// It the real Silo and not the "RANCH_HOUSE_SILO"
#define WINDMILL								0x97	// Kakariko's windmill

const SceneMetaInfo OoTScenesName[WINDMILL + 1] =
{
	{ "Deku Tree", "/OoT/Kokiri_Forest/Deku_Tree.png" },
	{ "Dodongo's Cavern", "/OoT/Death_Mountain/Dodongo_Cavern.png" },
	{ "Inside Jabu-Jabu", "/OoT/Zora_Foutain/Jabu_Jabu.png" },
	{ "Forest Temple", "/OoT/Kokiri_Forest/Temple.png" },
	{ "Fire Temple", "/OoT/Death_Mountain/Temple.png" },
	{ "Water Temple", "/OoT/Lake_Hylia/Temple.png" },
	{ "Spirit Temple", "/OoT/Desert/Temple.png" },
	{ "Shadow Temple", "/OoT/Kakariko/Temple.png" },
	{ "Bottom of the Well", "/OoT/Kakariko/Well.png" },
	{ "Ice Cavern", "/OoT/Zora_Foutain/Ice_Cavern.png" },
	{ "Ganon Tower", "/OoT/Ganon/Tower.png" },
	{ "Gerudo Training Ground", "/OoT/Gerudo/GTG.png" },
	{ "Thieves Hideout", "/OoT/Gerudo/Thieves_Hideout.png" },
	{ "Inside Ganon Castle", "/OoT/Ganon/Inside.png" },
	{ "Ganon Tower - Collapsing", "" },
	{ "Inside Ganon Castle - Collapsing", "" },
	{ "Gohma's Lair", "/OoT/Kokiri_Forest/Gohma_Lair.png" },
	{ "King Dodongo's Lair", "/OoT/Death_Moutain/King_Dodongo_Lair.png" },
	{ "Barinade's Lair", "/OoT/Zora_Fountain/Barinade_Lair.png" },
	{ "Phantom Ganon's Lair", "/OoT/Kokiri_Forest/Phantom_Ganon_Lair.png" },
	{ "Volvagia's Lair", "/OoT/Death_Moutain/Volvagia_Lair.png" },
	{ "Morpha's Lair", "/OoT/Lake_Hylia/Morpha_Lair.png" },
	{ "Twinrova's Lair", "/OoT/Desert/Twinrova_Lair.png" },
	{ "Bongo-Bongo's Lair", "/OoT/Kakariko/Bongo_Bongo_Lair.png" },
	{ "Ganondorf's Lair", "" },
	{ "Tower Collapse - Exterior", "" },
	{ "Market Entrance - Child Day", "" },
	{ "Market Entrance - Child Night", "" },
	{ "Market Entrance - Adult", "" },
	{ "Back Alley - Day", "" },
	{ "Back Alley - Night", "" },
	{ "Market - Child Day", "/OoT/Market/Market.png" },
	{ "Market - Child Night", "/OoT/Market/Market.png" },
	{ "Market - Adult", "" },
	{ "Temple of Time Exterior Child Day", "" },
	{ "Temple of Time Exterior Child Night", "" },
	{ "Temple of Time Exterior Adult", "" },
	{ "Know-It-All Brothers's House", "/OoT/Kokiri_Forest/Know_It_All.png" },
	{ "Twins's House", "/OoT/Kokiri_Forest/Twins_House.png" },
	{ "Mido's House", "/OoT/Kokiri_Forest/Mido_House.png" },
	{ "Saria's House", "/OoT/Kokiri_ForestSaria_House.png" },
	{ "Carpenter's Boss House", "/OoT/Kakariko/Carpenter_Boss_House.png" },
	{ "Back Alley House", "" },
	{ "Bazaar", "/OoT/Market/Bazaar.png" },
	{ "Kokiri's Shop", "/OoT/Kokiri_Forest/Shop.png" },
	{ "Goron's Shop", "/OoT/Goron_City/Shop.png" },
	{ "Zora's Shop", "/OoT/Zora_Domain/Shop.png" },
	{ "Kakariko Potion Shop", "/OoT/Kakariko/Potion_Shop.png" },
	{ "Market Potion Shop", "/OoT/Market/Potion_Shop.png" },
	{ "Bombchu Shop", "/OoT/Market/Bombchu_Shop.png" },
	{ "Happy Mask Shop", "" },
	{ "Link's House", "/OoT/Kokiri_Forest/Link_House.png" },
	{ "Dog's Lady House", "/OoT/Market/Dog_Lady_House.png" },
	{ "Stable", "/OoT/Ranch/Stable.png" },
	{ "Impa's House", "/OoT/Kakariko/Impa_House.png" },
	{ "Laboratory", "/OoT/Lake_Hylia/Laboratory.png" },
	{ "Carpenter's Tent", "" },
	{ "Gravekeeper's Hut", "" },
	{ "Great Fairy Fountain - Upgrades", "" },
	{ "Fairy Fountain", "" },
	{ "Great Fairy Fountain - Spells", "" },
	{ "Grottos", "" },
	{ "Redead's Tomb", "/OoT/Kakariko/Redead_Tomb.png" },
	{ "Fairy's Foutain Tomb", "/OoT/Kakariko/Fairy_Foutain_Tomb.png" },
	{ "Royal's Tomb", "/OoT/Kakariko/Royal_Tomb.png" },
	{ "Shooting Gallery", "" },
	{ "Temple of Time", "/OoT/Market/Temple of Time.png" },
	{ "Chamber of the Sages", "" },
	{ "Castle Maze - Day", "" },
	{ "Castle Maze - Night", "" },
	{ "Cutscene Map", "" },
	{ "Dampe's Tomb", "/OoT/Kakariko/Dampe_Tomb.png" },
	{ "Fishing Pond", "/OoT/Lake_Hylia/Fishing_Pond.png" },
	{ "Castle Courtyard", "/OoT/Hyrule/Castle_Courtyard.png" },
	{ "Bombchu Bowling", "/OoT/Market/Bombchu_Bowling.png" },
	{ "Ranch House Silo", "/OoT/Ranch/Ranch_House_Silo.png" },
	{ "Guard House", "/OoT/Market/Guard_House.png" },
	{ "Granny Potion Shop", "/OoT/Kakariko/Granny_Potion_Shop.png" },
	{ "Ganon Battle Arena", "" },
	{ "House of Skulltula", "/OoT/Kakariko/House_of_Skulltula.png" },
	{ "Hyrule Field", "/OoT/Hyrule/Hyrule_Field.png" },
	{ "Kakariko's Village", "/OoT/Kakariko/Kakariko_Village.png" },
	{ "Graveyard", "/OoT/Kakariko/Graveyard.png" },
	{ "Zora's River", "/OoT/Zora_River/Zora_River.png" },
	{ "Kokiri Forest", "/OoT/Kokiri_Forest/Kokiri_Forest.png" },
	{ "Sacred Forest Meadow", "/OoT/Kokiri_Forest/Sacred_Forest_Meadow.png" },
	{ "Lake Hylia", "/OoT/Lake_Hylia/Lake_Hylia.png" },
	{ "Zora's Domain", "/OoT/Zora_Domain/Zora_Domain.png" },
	{ "Zora's Fountain", "/OoT/Zora_Fountain/Zora_Fountain.png" },
	{ "Gerudo Valley", "/OoT/Gerudo/Gerudo_Valley.png" },
	{ "Lost Woods", "/OoT/Kokiri_Forest/Lost_Woods.png" },
	{ "Desert Colossus", "/OoT/Desert/Desert_Colossus.png" },
	{ "Gerudo's Fortress", "/OoT/Gerudo/Gerudo_Fortress.png" },
	{ "Haunted Wasteland", "/OoT/Gerudo/Haunted_Wasteland.png" },
	{ "Hyrule Castle", "/OoT/Hyrule/Hyrule_Castle.png" },
	{ "Death Mountain Trail", "/OoT/Death_Mountain/Trail.png" },
	{ "Death Mountain Crater", "/OoT/Death_Mountain/Crater.png" },
	{ "Goron City", "/OoT/Death_Mountain/Goron_City.png" },
	{ "Lon Lon's Ranch", "/OoT/Ranch/Lon_Lon_Ranch.png" },
	{ "Ganon Castle - Exterior", "/OoT/Ganon/Exterior.png" },

	{ "Kokiri Forest - Song of Storms", "/OoT/Kokiri_Forest/Storms.png" },
	{ "Lost Woods - Deku Scrub Upgrade", "/OoT/Kokiri_Forest/LW_Scrub_Upgrade.png" },
	{ "Lost Woods - Generic", "/OoT/Kokiri_Forest/LW_Generic.png" },
	{ "Lost Woods - Deku's Theater", "/OoT/Kokiri_Forest/LW_Theater.png" },
	{ "Sacred Meadow - Wolfos", "/OoT/Kokiri_Forest/Wolfos.png" },
	{ "Sacred Meadow - Song of Storms", "/OoT/Kokiri_Forest/SM_Storms.png" },
	{ "Kakariko - Redead", "/OoT/Kakariko/Redead.png" },
	{ "Kakariko - Open", "/OoT/Kakariko/Open.png" },
	{ "Death Mountain Trial - Song of Storms", "/OoT/Death_Mountain/Storms.png" },
	{ "Death Mountain Trial - Cow", "/OoT/Death_Mountain/Cow.png" },
	{ "Goron City - Deku Scrubs", "/OoT/Death_Mountain/GC_Scrubs.png" },
	{ "Death Mountain Crater - Generic", "/OoT/Death_Mountain/Crater_Generic.png" },
	{ "Death Mountain Crater Deku Scrubs", "/OoT/Death_Mountain/Crater_Scrubs.png" },
	{ "Zora's River - Song of Storms", "/OoT/Zora_River/Storms.png" },
	{ "Zora's River - Generic", "/OoT/Zora_River/Generic.png" },
	{ "Lake Hylia - Deku Scrubs", "/OoT/Lake_Hylia/Scrubs.png" },
	{ "Lon Lon's Ranch - Deku Scrubs", "/OoT/Ranch/Scrubs.png" },
	{ "Hyrule Field - Deku Scrubs", "/OoT/Hyrule/Field_Scrubs.png" },
	{ "Hyrule Field - Southeast", "/OoT/Hyrule/Field_SE.png" },
	{ "Hyrule Field - Open", "/OoT/Hyrule/Field_Open.png" },
	{ "Hyrule Field - Market Side", "/OoT/Hyrule/Field_Market.png" },
	{ "Hyrule Field - Tektite", "/OoT/Hyrule/Field_Tektite.png" },
	{ "Hyrule Field - Kakariko Side", "/OoT/Hyrule/Field_Kakariko.png" },
	{ "Hyrule Field - Gerudo Side", "/OoT/Hyrule/Field_Gerudo.png" },
	{ "Hyrule Castle - Song of Storms", "/OoT/Hyrule/Castle_Storms.png" },
	{ "Gerudo Valley - Song of Storms", "/OoT/Gerudo/Valley_Storms.png" },
	{ "Gerudo Valley - Octorok", "/OoT/Gerudo/Valley_Octorok.png" },
	{ "Desert Colossus - Deku Scrubs", "/OoT/Desert/Scrubs.png" },

	{ "Sacred Meadow Fairy Fountain", "/OoT/Kokiri_Forest/SM_Fountain.png" },
	{ "Zora's River Fairy Fountain", "/OoT/Zora_River/Fountain.png" },
	{ "Zora's Domain Fairy Fountain", "/OoT/Zora_Domain/Fountain.png" },
	{ "Hyrule Field Fairy Fountain", "/OoT/Hyrule/Field_Fountain.png" },
	{ "Gerudo's Fortress Fairy Fountain", "/OoT/Gerudo/Fountain.png" },

	{ "Din's Great Fairy", "/OoT/Hyrule/Great_Fairy.png" },
	{ "Farore's Great Fairy", "/OoT/Zora_Fountain/Great_Fairy.png" },
	{ "Nayru's Great Fairy", "/OoT/Desert/Great_Fairy.png" },
	{ "First Magic Upgrade Great Fairy", "/OoT/Death_Moutain/Trail_Great_Fairy.png" },
	{ "Second Magic Upgrade Great Fairy", "/OoT/Death_Moutain/Crater_Great_Fairy.png" },
	{ "Double Defense Great Fairy", "/OoT/Ganon/Great_Fairy.png" },

	{ "Kakariko's Bazaar", "/OoT/Kakariko/Bazaar.png" },
	{ "Market's Bazaar", "/OoT/Market/Bazaar.png" },
	{ "Kakariko's Shooting", "/OoT/Kakariko/Shooting.png" },
	{ "Market's Shooting", "/OoT/Market/Shooting.png" },
	{ "Silo", "/OoT/Ranch/Silo.png" },
	{ "Windmill", "/OoT/Kakariko/Windmill.png" }
};

#define SOUTHERN_SWAMP_CLEAR					0x00
#define MM_FAIRY_SNOWHEAD						0x01
#define MM_FAIRY_GREAT_BAY_COAST				0x02
#define MM_FAIRY_WOODFALL						0x03
#define MM_FAIRY_CLOCK_TOWN						0x04
#define MM_FAIRY_IKANA							0x05
#define PATH_SNOWHEAD_SPRING					0x06
#define MM_GROTTOS								0x07
#define MM_CUTSCENE_MAP							0x08
#define PATH_MOUNTAIN_VILLAGE_SPRING			0x09
#define POTION_SHOP								0x0a
#define LAIR_MAJORA								0x0b
#define BENEATH_THE_GRAVEYARD					0x0c
#define CURIOSITY_SHOP							0x0d
#define BENEATH_THE_GRAVEYARD_NIGHT1			0x0e
#define BENEATH_THE_GRAVEYARD_NIGHT2			0x0f
#define RANCH_HOUSE_BARN						0x10
#define HONEY_DARLING							0x11
#define MAYOR_HOUSE								0x12
#define IKANA_CANYON							0x13
#define PIRATE_FORTRESS_EXTERIOR				0x14
#define MILK_BAR								0x15
#define TEMPLE_STONE_TOWER						0x16
#define MM_TREASURE_SHOP						0x17
#define TEMPLE_STONE_TOWER_INVERTED				0x18
#define CLOCK_TOWER_ROOFTOP						0x19
#define OPENING									0x1a
#define TEMPLE_WOODFALL							0x1b
#define PATH_MOUNTAIN_VILLAGE					0x1c
#define CASTLE_IKANA							0x1d
#define DEKU_PLAYGROUND							0x1e
#define LAIR_ODOLWA								0x1f
#define MM_SHOOTING_GALLERY						0x20
#define TEMPLE_SNOWHEAD							0x21
#define MILK_ROAD								0x22
#define PIRATE_FORTRESS_INTERIOR				0x23
#define SHOOTING_GALLERY_SWAMP					0x24
#define PINNACLE_ROCK							0x25
#define MM_FAIRY_FOUNTAIN						0x26
#define MM_SPIDER_HOUSE_SWAMP					0x27
#define MM_SPIDER_HOUSE_OCEAN					0x28
#define OBSERVATORY								0x29
#define MOON_DEKU								0x2a
#define DEKU_PALACE								0x2b
#define BLACKSMITH								0x2c
#define TERMINA_FIELD							0x2d
#define POST_OFFICE								0x2e
#define MM_LABORATORY							0x2f
#define DAMPE_HOUSE								0x30
#define INSIDE_CASTLE_IKANA						0x31
#define GORON_SHRINE							0x32
#define ZORA_HALL								0x33
#define TRADING_POST							0x34
#define ROMANI_RANCH							0x35
#define LAIR_TWINMOLD							0x36
#define GREAT_BAY_COAST							0x37
#define ZORA_CAPE								0x38
#define LOTTERY									0x39
#define SNOWHEAD_SPRING							0x3a
#define PIRATE_FORTRESS_ENTRANCE				0x3b
#define FISHERMAN_HUT							0x3c
#define MM_GORON_SHOP							0x3d
#define DEKU_KING_CHAMBER						0x3e
#define MOON_GORON								0x3f
#define ROAD_SOUTHERN_SWAMP						0x40
#define DOG_RACETRACK							0x41
#define CUCCO_SHACK								0x42
#define IKANA_GRAVEYARD							0x43
#define LAIR_GOHT								0x44
#define SOUTHERN_SWAMP							0x45
#define WOODFALL								0x46
#define MOON_ZORA								0x47
#define GORON_VILLAGE_SPRING					0x48
#define TEMPLE_GREAT_BAY						0x49
#define WATERFALL_RAPIDS						0x4a
#define BENEATH_THE_WELL						0x4b
#define ZORA_HALL_ROOMS							0x4c
#define GORON_VILLAGE_WINTER					0x4d
#define GORON_GRAVEYARD							0x4e
#define SAKON_HIDEOUT							0x4f
#define MOUNTAIN_VILLAGE_WINTER					0x50
#define GHOST_HUT								0x51
#define DEKU_SHRINE								0x52
#define ROAD_IKANA								0x53
#define SWORDSMAN_SCHOOL						0x54
#define MUSIC_BOX_HOUSE							0x55
#define LAIR_IKANA								0x56
#define TOURIST_INFORMATION						0x57
#define STONE_TOWER								0x58
#define STONE_TOWER_INVERTED					0x59
#define MOUNTAIN_VILLAGE_SPRING					0x5a
#define PATH_SNOWHEAD							0x5b
#define SNOWHEAD								0x5c
#define TWIN_ISLANDS_WINTER						0x5d
#define TWIN_ISLANDS_SPRING						0x5e
#define LAIR_GYORG								0x5f
#define SECRET_SHRINE							0x60
#define STOCK_POT_INN							0x61
#define GREAT_BAY_CUTSCENE						0x62
#define CLOCK_TOWER_INTERIOR					0x63
#define WOODS_MYSTERY							0x64
#define MM_LOST_WOODS							0x65
#define MOON_LINK								0x66
#define MOON									0x67
#define BOMB_SHOP								0x68
#define GIANT_CHAMBER							0x69
#define GORMAN_TRACK							0x6a
#define GORON_RACETRACK							0x6b
#define CLOCK_TOWN_EAST							0x6c
#define CLOCK_TOWN_WEST							0x6d
#define CLOCK_TOWN_NORTH						0x6e
#define CLOCK_TOWN_SOUTH						0x6f
#define LAUNDRY_POOL							0x70

#define EXTRA									0x71

// MM detailed other
#define MM_ZORA_SHOP							0x72
#define ZORA_EVAN_ROOM							0x73

// MM detailed grottos
#define MM_GROTTO_TERMINA_DODONGO				0x74	// Termina field dodongo's grotto
#define MM_GROTTO_TERMINA_OCEAN_GOSSIP			0x75	// Termina field ocean gossip's grotto
#define MM_GROTTO_TERMINA_CANYON_GOSSIP			0x76	// Termina field canyon gossip's grotto
#define MM_GROTTO_TERMINA_BIO_BABA				0x77	// Termina field bio baba's grotto
#define MM_GROTTO_TERMINA_PEEHAT				0x78	// Termina field peehat's grotto
#define MM_GROTTO_TERMINA_SCRUB					0x79	// Termina field scrub grotto
#define MM_GROTTO_TERMINA_TALL_GRASS			0x7a	// Termina field tall grass grotto
#define MM_GROTTO_TERMINA_COW					0x7b	// Termina field cow grotto
#define MM_GROTTO_TERMINA_PILLAR				0x7c	// Termina field pillar grotto
#define MM_GROTTO_DEKU_PALACE_GENERIC			0x7d	// Deku palace generic grotto
#define MM_GROTTO_DEKU_PALACE_BEANS				0x7e	// Deku palace beans grotto
#define MM_GROTTO_GREAT_BAY_COAST_FISHERMAN		0x7f	// Great bay coast fisherman's grotto
#define MM_GROTTO_GREAT_BAY_COAST_COW			0x80	// Great bay coast cow grotto
#define MM_GROTTO_ZORA_CAPE_GENERIC				0x81	// Zora cape generic grotto
#define MM_GROTTO_IKANA_GRAVEYARD_GENERIC		0x82	// Ikana graveyard generic grotto
#define MM_GROTTO_IKANA_VALLEY_GENERIC			0x83	// Ikana valley generic grotto
#define MM_GROTTO_IKANA_ROAD_GENERIC			0x84	// Ikana road generic grotto
#define MM_GROTTO_LONE_PEAK_GENERIC				0x85	// Lone Peak generic grotto
#define MM_GROTTO_TWIN_ISLANDS_FROZEN			0x86	// Twin islands frozen grotto
#define MM_GROTTO_TWIN_ISLANDS_RAMP				0x87	// Twin islands ramp grotto
#define MM_GROTTO_PATH_TO_SNOWHEAD_GENERIC		0x88	// Path to snowhead generic grotto
#define MM_GROTTO_MOUNTAIN_VILLAGE_GENERIC		0x89	// Mountain village generic grotto
#define MM_GROTTO_SOUTHERN_SWAMP_ROAD_GENERIC	0x8a	// Southern swamp road generic grotto
#define MM_GROTTO_SOUTHERN_SWAMP_GENERIC		0x8b	// Southern swamp generic grotto
#define MM_GROTTO_WOODS_OF_MYSTERY_GENERIC		0x8c	// Woods of mystery generic grotto


const SceneMetaInfo MMScenesName[MM_GROTTO_WOODS_OF_MYSTERY_GENERIC + 1] =
{
	{ "Southern Swamp - Clear", "" },
	{ "Snowhead - Fairy Fountain", "/MM/Snowhead/Fairy_Fountain.png" },
	{ "Great Bay Coast - Fairy Fountain", "/MM/Great_Bay_Coast/Fairy_Fountain.png" },
	{ "Woodfall - Fairy Fountain", "/MM/Woodfall/Fairy_Fountain.png" },
	{ "Clock Town - Fairy Fountain", "/MM/Clock_Town/Fairy Fountain.png" },
	{ "Ikana - Fairy Fountain", "/MM/Ikana/Fairy_Fountain.png" },
	{ "Path to Snowhead - Spring", "/MM/Path_to_Snowhead/Spring.png" },
	{ "Grottos", "" },
	{ "Cutscene Map", "" },
	{ "Path to Mountain Village - Spring", "/MM/Path_to_Mountain_Village/Spring.png" },
	{ "Potion Shop", "/MM/Southern_Swamp/Potion_Shop.png" },
	{ "Majora's Lair", "/MM/Moon/Majora_Lair.png" },
	{ "Beneath the Graveyard", "/MM/Ikana/Beneath_Graveyard.png" },
	{ "Curiosity Shop", "/MM/Clock_Town/Curiosity_Shop.png" },
	{ "Beneath the Graveyard - Night 1", "/MM/Road_to_Ikana/Beneath_Graveyard_N1.png" },
	{ "Beneath the Graveyard - Night 2", "/MM/Road_to_Ikana/Beneath_Graveyard_N2.png" },
	{ "Ranch House Barn", "/MM/Ranch/House_Barn.png" },
	{ "Honey Darling", "/MM/Clock_Town/Honey_Darling.png" },
	{ "Mayor House", "/MM/Clock_Town/Mayor_House.png" },
	{ "Ikana Canyon", "/MM/Ikana/Ikana_Canyon.png" },
	{ "Pirate's Fortress - Exterior", "/MM/Fortress/Exterior.png" },
	{ "Milk Bar", "/MM/Clock_Town/Milk_Bar.png" },
	{ "Stone Tower Temple", "/MM/Stone_Tower/Temple.png" },
	{ "Treasure Shop", "/MM/Clock_Town/Treasure Shop.png" },
	{ "Stone Tower Temple - Inverted", "/MM/Stone_Tower/Temple_Inverted.png" },
	{ "Clock Tower - Rooftop", "/MM/Clock_Town/Tower_Rooftop.png" },
	{ "Opening", "" },
	{ "Woodfall Temple", "/MM/Woodfall/Temple.png" },
	{ "Path to Mountain Village", "/MM/Path_to_Mountain_Village/Winter.png" },
	{ "Ancient Ikana Castle", "/MM/Ikana/Ancient_Ikana_Castle.png" },
	{ "Deku's Playground", "/MM/Clock_Town/Deku_Playground.png" },
	{ "Odolwa's Lair", "/MM/Woodfall/Odolwa_Lair.png" },
	{ "Shooting Gallery - Clock Town", "/MM/Clock_Town/Shooting_Gallery.png" },
	{ "Snowhead Temple", "/MM/Snowhead/Temple.png" },
	{ "Milk Road", "/MM/Milk_Road/Road.png" },
	{ "Pirate's Fortress - Interior", "/MM/Fortress/Interior.png" },
	{ "Shooting Gallery - Southern Swamp", "/MM/Southern_Swamp/Shooting_Gallery.png" },
	{ "Pinnacle Rock", "/MM/Great_Bay_Coast/Pinnacle_Rock.png" },
	{ "Fairy Fountain", "" },
	{ "Spider's House - Southern Swamp", "/MM/Southern_Swamp/Spider_House.png" },
	{ "Spider's House - Great Bay Coast", "/MM/Great_Bay_Coast/Spider_House.png" },
	{ "Observatory", "/MM/Clock_Town/Observatory.png" },
	{ "Moon - Deku's Trial", "/MM/Moon/Deku.png" },
	{ "Deku's Palace", "/MM/Southern_Swamp/Deku_Palace.png" },
	{ "Blacksmith", "/MM/Mountain_Village/Blacksmith.png" },
	{ "Termina Field", "/MM/Termina/Field.png" },
	{ "Post Office", "/MM/Clock_Town/Post_Office.png" },
	{ "Laboratory", "/MM/Great_Bay_Coast/Laboratory.png" },
	{ "Dampe's House", "/MM/Road_to_Ikana/Dampe_House.png" },
	{ "Inside Ancient Ikana Castle", "/MM/Ikana/Inside_Castle.png" },
	{ "Goron's Shrine", "/MM/Goron_Village/Goron_Shrine.png" },
	{ "Zora's Theater", "/MM/Zora_Cape/Zora_Hall.png" },
	{ "Trading Post", "/MM/Clock_Town/Trading_Post.png" },
	{ "Romani's Ranch", "/MM/Ranch/Romani_Ranch.png" },
	{ "Twinmold Lair", "/MM/Stone_Tower/Twinmold_Lair.png" },
	{ "Great Bay Coast", "/MM/Great_Bay_Coast/Great_Bay.png" },
	{ "Zora Cape", "/MM/Zora_Cape/Zora_Cape.png" },
	{ "Lottery", "" },
	{ "Snowhead - Spring", "/MM/Snowhead/Spring.png" },
	{ "Pirate's Fortress Entrance", "/MM/Fortress/Entrance.png" },
	{ "Fisherman Hut", "/MM/Great_Bay_Coast/Fisherman_Hut.png" },
	{ "Goron's Shop", "/MM/Goron_Village/Goron_Shop.png" },
	{ "Deku King's Chamber", "/MM/Southern_Swamp/Deku_King_Chamber.png" },
	{ "Moon - Goron's Trial", "/MM/Moon/Goron.png" },
	{ "Road to Southern Swamp", "/MM/Road_to_Southern_Swamp/Road.png" },
	{ "Dog's Racetrack", "/MM/Ranch/Dog_Racetrack.png" },
	{ "Cucco's Shack", "/MM/Ranch/Cucco_Shack.png" },
	{ "Ikana Graveyard", "/MM/Road_to_Ikana/Graveyard.png" },
	{ "Goht's Lair", "/MM/Snowhead/Goht_Lair.png" },
	{ "Southern Swamp", "/MM/Southern_Swamp/Swamp.png" },
	{ "Woodfall", "/MM/Woodfall/Woodfall.png" },
	{ "Moon - Zora's Trial", "/MM/Moon/Zora.png" },
	{ "Goron's Village - Spring", "/MM/Goron_Village/Spring.png" },
	{ "Great Bay Temple", "/MM/Great_Bay_Coast/Temple.png" },
	{ "Waterfall Rapids", "/MM/Zora_Cape/Waterfall_Rapids.png" },
	{ "Beneath the Well", "/MM/Ikana/Beneath_the_Well.png" },
	{ "Zora's Theater", "/MM/Zora_Cape/Theater.png" },
	{ "Goron's Village - Winter", "/MM/Goron_Village/Winter.png" },
	{ "Goron's Graveyard", "/MM/Mountain_Village/Graveyard.png" },
	{ "Sakon's Hideout", "/MM/Ikana/Sakon_Hideout.png" },
	{ "Mountain Village - Winter", "/MM/Mountain_Village/Winter.png" },
	{ "Ghost Hut", "/MM/Ikana/Ghost_Hut.png" },
	{ "Deku's Shrine", "/MM/Southern_Swamp/Deku_Shrine.png" },
	{ "Road to Ikana", "/MM/Road_to_Ikana/Road.png" },
	{ "Swordsman's School", "/MM/Clock_Town/Swordsman_School.png" },
	{ "Music Box House", "/MM/Ikana/Music_Box_House.png" },
	{ "Ikana's Lair", "/MM/Ikana/Ikana_Lair.png" },
	{ "Tourist Information", "/MM/Southern_Swamp/Tourist_Information.png" },
	{ "Stone Tower", "/MM/Stone_Tower/Tower.png" },
	{ "Stone Tower - Inverted", "/MM/Stone_Tower/Tower_Inverted.png" },
	{ "Mountain Village - Spring", "/MM/Mountain_Village/Spring.png" },
	{ "Path to Snowhead", "/MM/Path_to_Snowhead/Winter.png" },
	{ "Snowhead", "/MM/Snowhed/Snowhead.png" },
	{ "Twin Islands - Winter", "/MM/Twin_Islands/Winter.png" },
	{ "Twin Islands - Spring", "/MM/Twin_Islands/Spring.png" },
	{ "Gyorg's Lair", "/MM/Great_Bay_Coast/Gyorg_Lair.png" },
	{ "Secret's Shrine", "/MM/Ikana/Secret_Shrine.png" },
	{ "Stock Pot Inn", "/MM/Clock_Town/Stock_Pot_Inn.png" },
	{ "Great Bay Cutscene", "" },
	{ "Clock Tower - Interior", "/MM/Clock_Town/Interior.png" },
	{ "Woods of Mystery", "/MM/Southtern_Swamp/Woods_Mystery.png" },
	{ "Lost Woods", "" },
	{ "Moon - Link's Trial", "/MM/Moon/Link.png" },
	{ "Moon", "/MM/Moon/Moon.png" },
	{ "Bomb Shop", "/MM/Clock_Town/Bomb_Shop.png" },
	{ "Giant Chamber", "" },
	{ "Gorman's Track", "/MM/Milk_Road/Gorman_Track.png" },
	{ "Goron's Racetrack", "/MM/Twin_Islands/Goron_Racetrack.png" },
	{ "Clock Town - East", "/MM/Clock_Town/East.png" },
	{ "Clock Town - West", "/MM/Clock_Town/West.png" },
	{ "Clock Town - North", "/MM/Clock_Town/North.png" },
	{ "Clock Town - South", "/MM/Clock_Town/South.png" },
	{ "Laundry Pool", "/MM/Clock_Town/Laundry Pool.png" },

	{ "Extra", "" },

	{ "Zora's Shop", "/MM/Zora's Shop.png" },
	{ "Evan's Room", "/MM/Evan's Room.png" },

	{ "Termina - Dodongo", "/MM/Termina/Dodongo.png" },
	{ "Termina - Ocean Gossip", "/MM/Termina/Ocean_Gossip.png" },
	{ "Termina - Canyon Gossip", "/MM/Termina/Canyon_Gossip.png" },
	{ "Termina - Bio Baba", "/MM/Termina Bio Baba.png" },
	{ "Termina - Peehat", "/MM/Termina Peehat.png" },
	{ "Termina - Deku Scrub", "/MM/Termina Scrub.png" },
	{ "Termina - Tall Grass", "/MM/Termina Tall Grass.png" },
	{ "Termina - Cow", "/MM/Termina Cow.png" },
	{ "Termina - Pillar", "/MM/Termina Pillar.png" },
	{ "Deku Palace - Generic", "/MM/Deku Palace Generic.png" },
	{ "Deku Palace - Beans", "/MM/Deku Palace Beans.png" },
	{ "Great Bay Coast - Open", "/MM/Great Bay Coast - Open.png" },
	{ "Great Bay Coast - Cow", "/MM/Great Bay Coast - Cow.png" },
	{ "Zora Cape - Generic", "/MM/Zora Cape - Generic.png" },
	{ "Ikana Graveyard - Generic", "/MM/Road_To_Ikana/Graveyard_Generic.png" },
	{ "Ikana Valley - Generic", "/MM/Ikana/Generic.png" },
	{ "Road To Ikana - Generic", "/MM/Road_To_Ikana/Generic.png" },
	{ "Lone Peak Shrine", "/MM/Goron_Village/Lone_Peak_Shrine.png" },
	{ "Twin Islands - Frozen", "/MM/Twin_Islands/Frozen.png" },
	{ "Twin Islands - Ramp", "/MM/Twin_Islands/Ramp.png" },
	{ "Path to Snowhead - Generic", "/MM/Path_to_Snowhead/Generic.png" },
	{ "Mountain Village - Generic", "/MM/Mountain_Village/Generic.png" },
	{ "Road to Southern Swamp - Open", "/MM/Road_to_Southern/Open.png" },
	{ "Southern Swamp - Generic", "/MM/Southern_Swamp/Generic.png" },
	{ "Woods of Mystery - Open", "/MM/Southern_Swamp/Woods_Open"}
};


enum OoT_Shops
{
	OoT_Kokiri_Shop = 0x00,
	OoT_Bombchu_Shop = 0x01,
	OoT_Zora_Shop = 0x02,
	OoT_Goron_Shop = 0x03,
	OoT_Market_Bazaar = 0x04,
	OoT_Market_Potion_Shop = 0x05,
	OoT_Kakariko_Bazaar = 0x06,
	OoT_Kakariko_Potion_Shop = 0x07
};

enum MM_Shops
{
	MM_Clock_Town_Shop = 0x00,
	MM_Swamp_Potion_Shop = 0x01,
	MM_Goron_Zora_Shop = 0x02,
};

uint32_t GetSceneShop(uint32_t ItemID, uint32_t Game);
uint32_t GetSceneNPC(uint32_t NPC, uint32_t Game);
uint32_t GetSceneSR(uint32_t SilverRupee);
uint32_t GetSceneGS(uint32_t GS);
uint32_t GetSceneScrub(uint32_t Scrub);