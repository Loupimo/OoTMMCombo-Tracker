#pragma once

#include "Multi/Game.h"
#include "Multi/API.h"
#include "Regions.h"

enum LocType
{
	overworld,
	dungeon,
	minigame,
	broken
};

typedef struct SceneMetaInfo
{
	const char* Name;
	const char* ImagePath;
	uint8_t ParentRegion;
	bool HasContext;
	GameLayout ActiveLayout;	// The current active layout for this scene

} SceneMetaInfo;

#pragma region OoT

#define OOT_DEKU_TREE								0x00
#define OOT_DODONGO_CAVERN							0x01
#define OOT_INSIDE_JABU_JABU						0x02
#define OOT_TEMPLE_FOREST							0x03
#define OOT_TEMPLE_FIRE								0x04
#define OOT_TEMPLE_WATER							0x05
#define OOT_TEMPLE_SPIRIT							0x06
#define OOT_TEMPLE_SHADOW							0x07
#define OOT_BOTTOM_OF_THE_WELL						0x08
#define OOT_ICE_CAVERN								0x09
#define OOT_GANON_TOWER								0x0a
#define OOT_GERUDO_TRAINING_GROUND					0x0b
#define OOT_THIEVES_HIDEOUT							0x0c
#define OOT_INSIDE_GANON_CASTLE						0x0d
#define OOT_GANON_TOWER_COLLAPSING					0x0e
#define OOT_INSIDE_GANON_CASTLE_COLLAPSING			0x0f
#define OOT_TREASURE_SHOP						    0x10
#define OOT_LAIR_GOHMA								0x11
#define OOT_LAIR_KING_DODONGO						0x12
#define OOT_LAIR_BARINADE							0x13
#define OOT_LAIR_PHANTOM_GANON						0x14
#define OOT_LAIR_VOLVAGIA							0x15
#define OOT_LAIR_MORPHA								0x16
#define OOT_LAIR_TWINROVA							0x17
#define OOT_LAIR_BONGO_BONGO						0x18
#define OOT_LAIR_GANONDORF							0x19
#define OOT_TOWER_COLLAPSE_EXTERIOR					0x1a
#define OOT_MARKET_ENTRANCE_CHILD_DAY				0x1b
#define OOT_MARKET_ENTRANCE_CHILD_NIGHT				0x1c
#define OOT_MARKET_ENTRANCE_ADULT					0x1d
#define OOT_BACK_ALLEY_DAY							0x1e
#define OOT_BACK_ALLEY_NIGHT						0x1f
#define OOT_MARKET_CHILD_DAY						0x20
#define OOT_MARKET_CHILD_NIGHT						0x21
#define OOT_MARKET_ADULT							0x22
#define OOT_TEMPLE_OF_TIME_EXTERIOR_CHILD_DAY		0x23
#define OOT_TEMPLE_OF_TIME_EXTERIOR_CHILD_NIGHT		0x24
#define OOT_TEMPLE_OF_TIME_EXTERIOR_ADULT			0x25
#define OOT_KOKIRI_KNOW_IT_ALL						0x26
#define OOT_KOKIRI_TWINS							0x27
#define OOT_KOKIRI_MIDO								0x28
#define OOT_KOKIRI_SARIA							0x29
#define OOT_CARPENTER_BOSS_HOUSE					0x2a
#define OOT_BACK_ALLEY_HOUSE						0x2b
#define OOT_BAZAAR									0x2c
#define OOT_KOKIRI_SHOP								0x2d
#define OOT_GORON_SHOP							    0x2e
#define OOT_ZORA_SHOP								0x2f
#define OOT_KAKARIKO_POTION_SHOP					0x30
#define OOT_MARKET_POTION_SHOP						0x31
#define OOT_BOMBCHU_SHOP							0x32
#define OOT_HAPPY_MASK_SHOP							0x33
#define OOT_LINK_HOUSE								0x34
#define OOT_BACK_ALLEY_HOUSE2						0x35
#define OOT_STABLE									0x36
#define OOT_IMPA_HOUSE								0x37
#define OOT_LABORATORY							    0x38
#define OOT_CARPENTER_TENT							0x39
#define OOT_GRAVEKEEPER_HUT							0x3a
#define OOT_GREAT_FAIRY_FOUNTAIN_UPGRADES			0x3b
#define OOT_FAIRY_FOUNTAIN						    0x3c
#define OOT_GREAT_FAIRY_FOUNTAIN_SPELLS				0x3d
#define OOT_GROTTOS								    0x3e
#define OOT_TOMB_REDEAD								0x3f
#define OOT_TOMB_FAIRY								0x40
#define OOT_TOMB_ROYAL								0x41
#define OOT_SHOOTING_GALLERY					    0x42
#define OOT_TEMPLE_OF_TIME							0x43
#define OOT_CHAMBER_OF_THE_SAGES					0x44
#define OOT_CASTLE_MAZE_DAY							0x45
#define OOT_CASTLE_MAZE_NIGHT						0x46
#define OOT_CUTSCENE_MAP						    0x47
#define OOT_TOMB_DAMPE_WINDMILL						0x48
#define OOT_FISHING_POND							0x49
#define OOT_CASTLE_COURTYARD						0x4a
#define OOT_BOMBCHU_BOWLING_ALLEY					0x4b
#define OOT_RANCH_HOUSE_SILO						0x4c
#define OOT_GUARD_HOUSE								0x4d
#define OOT_GRANNY_POTION_SHOP						0x4e
#define OOT_GANON_BATTLE_ARENA						0x4f
#define OOT_HOUSE_OF_SKULLTULA						0x50
#define OOT_HYRULE_FIELD							0x51
#define OOT_KAKARIKO_VILLAGE						0x52
#define OOT_GRAVEYARD								0x53
#define OOT_ZORA_RIVER								0x54
#define OOT_KOKIRI_FOREST							0x55
#define OOT_SACRED_FOREST_MEADOW					0x56
#define OOT_LAKE_HYLIA								0x57
#define OOT_ZORA_DOMAIN								0x58
#define OOT_ZORA_FOUNTAIN							0x59
#define OOT_GERUDO_VALLEY							0x5a
#define OOT_LOST_WOODS							    0x5b
#define OOT_DESERT_COLOSSUS							0x5c
#define OOT_GERUDO_FORTRESS							0x5d
#define OOT_HAUNTED_WASTELAND						0x5e
#define OOT_HYRULE_CASTLE							0x5f
#define OOT_DEATH_MOUNTAIN_TRAIL					0x60
#define OOT_DEATH_MOUNTAIN_CRATER					0x61
#define OOT_GORON_CITY								0x62
#define OOT_LON_LON_RANCH							0x63
#define OOT_GANON_CASTLE_EXTERIOR					0x64

// OoT detailed grotto scene
#define OOT_GROTTO_KOKIRI_FOREST_STORMS				OOT_GANON_CASTLE_EXTERIOR + 1			// Kokiri forest song of storms grotto
#define OOT_GROTTO_LOST_WOODS_SCRUB_UPGRADE			OOT_GROTTO_KOKIRI_FOREST_STORMS + 1		// Lost woods nut upgrade grotto
#define OOT_GROTTO_LOST_WOODS_GENERIC				OOT_GROTTO_LOST_WOODS_SCRUB_UPGRADE + 1	// Lost woods generic grotto
#define OOT_GROTTO_LOST_WOODS_THEATER				OOT_GROTTO_LOST_WOODS_GENERIC + 1		// Lost woods theater grotto
#define OOT_GROTTO_SACRED_MEADOW_WOLFOS				OOT_GROTTO_LOST_WOODS_THEATER + 1		// Sacred Meadow wolfos grotto
#define OOT_GROTTO_SACRED_MEADOW_STORMS				OOT_GROTTO_SACRED_MEADOW_WOLFOS + 1		// Sacred Meadow song of storms grotto
#define OOT_GROTTO_KAKARIKO_REDEAD					OOT_GROTTO_SACRED_MEADOW_STORMS + 1		// Kakariko redead grotto
#define OOT_GROTTO_KAKARIKO_OPEN					OOT_GROTTO_KAKARIKO_REDEAD + 1			// Kakariko open grotto
#define OOT_GROTTO_DEATH_TRIAL_STORMS				OOT_GROTTO_KAKARIKO_OPEN + 1			// Death mountain trial song of storms grotto
#define OOT_GROTTO_DEATH_TRIAL_COW					OOT_GROTTO_DEATH_TRIAL_STORMS + 1		// Death mountain trial cow grotto
#define OOT_GROTTO_GORON_CITY_SCRUBS				OOT_GROTTO_DEATH_TRIAL_COW + 1			// Goron city scrubs grotto
#define OOT_GROTTO_DEATH_CRATER_GENERIC				OOT_GROTTO_GORON_CITY_SCRUBS + 1		// Death mountain crater generic grotto
#define OOT_GROTTO_DEATH_CRATER_SCRUBS				OOT_GROTTO_DEATH_CRATER_GENERIC + 1		// Death mountain crater scrubs grotto
#define OOT_GROTTO_ZORA_RIVER_STORMS				OOT_GROTTO_DEATH_CRATER_SCRUBS + 1		// Zora's river song of storms grotto
#define OOT_GROTTO_ZORA_RIVER_GENERIC				OOT_GROTTO_ZORA_RIVER_STORMS + 1		// Zora's river generic grotto
#define OOT_GROTTO_LAKE_HYLIA_SCRUBS				OOT_GROTTO_ZORA_RIVER_GENERIC + 1		// Lake Hylia scrubs grotto
#define OOT_GROTTO_LON_LON_SCRUBS					OOT_GROTTO_LAKE_HYLIA_SCRUBS + 1		// Lon Lon's Ranch scrubs 
#define OOT_GROTTO_HYRULE_SCRUBS					OOT_GROTTO_LON_LON_SCRUBS + 1			// Hyrule field scrub grotto
#define OOT_GROTTO_HYRULE_SE						OOT_GROTTO_HYRULE_SCRUBS + 1			// Hyrule field southeast grotto
#define OOT_GROTTO_HYRULE_OPEN						OOT_GROTTO_HYRULE_SE + 1				// Hyrule field open grotto
#define OOT_GROTTO_HYRULE_MARKET					OOT_GROTTO_HYRULE_OPEN + 1				// Hyrule field market grotto
#define OOT_GROTTO_HYRULE_TEKTITE					OOT_GROTTO_HYRULE_MARKET + 1			// Hyrule field tektite grotto
#define OOT_GROTTO_HYRULE_KAKARIKO					OOT_GROTTO_HYRULE_TEKTITE + 1			// Hyrule field kakariko grotto
#define OOT_GROTTO_HYRULE_GERUDO					OOT_GROTTO_HYRULE_KAKARIKO + 1			// Hyrule field gerudo grotto
#define OOT_GROTTO_CASTLE_STORMS					OOT_GROTTO_HYRULE_GERUDO + 1			// Hyrule castle song of storms grotto
#define OOT_GROTTO_VALLEY_STORMS					OOT_GROTTO_CASTLE_STORMS + 1			// Hyrule castle song of storms grotto
#define OOT_GROTTO_VALLEY_OCTOROK					OOT_GROTTO_VALLEY_STORMS + 1			// Hyrule castle octorok grotto
#define OOT_GROTTO_DESERT_SCRUBS					OOT_GROTTO_VALLEY_OCTOROK + 1			// Desert colossus scrubs grotto

// OoT detailed fairy scene
#define OOT_FAIRY_SACRED_MEADOW						OOT_GROTTO_DESERT_SCRUBS + 1			// Sacred meadow fairy fountain
#define OOT_FAIRY_ZORA_RIVER						OOT_FAIRY_SACRED_MEADOW + 1				// Zora's river fairy fountain
#define OOT_FAIRY_ZORA_DOMAIN						OOT_FAIRY_ZORA_RIVER + 1				// Zora's domain fairy fountain
#define OOT_FAIRY_HYRULE							OOT_FAIRY_ZORA_DOMAIN + 1				// Hyrule field fairy fountain
#define OOT_FAIRY_GERUDO_FORTRESS					OOT_FAIRY_HYRULE + 1					// Gerudo fortress fairy fountain

// OoT detailed great fairy scene
#define OOT_GREAT_FAIRY_CASTLE						OOT_FAIRY_GERUDO_FORTRESS + 1			// Din's fire fairy fountain / Double defense upgrade fairy fountain
#define OOT_GREAT_FAIRY_DEFENSE						OOT_GREAT_FAIRY_CASTLE + 1				// Double defense upgrade fairy fountain. Used only by the entrance tracker
#define OOT_GREAT_FAIRY_FARORE						OOT_GREAT_FAIRY_DEFENSE + 1				// Farore's wind fairy fountain
#define OOT_GREAT_FAIRY_NAYRU						OOT_GREAT_FAIRY_FARORE + 1				// Nayru's love fairy fountain
#define OOT_GREAT_FAIRY_MAGIC						OOT_GREAT_FAIRY_NAYRU + 1				// First magic upgrade fairy fountain
#define OOT_GREAT_FAIRY_MAGIC2						OOT_GREAT_FAIRY_MAGIC + 1				// Second magic upgrade fairy fountain

// OoT detailed other scene
#define OOT_KAKARIKO_BAZAAR							OOT_GREAT_FAIRY_MAGIC2 + 1				// Kakariko's bazaar
#define OOT_MARKET_BAZAAR							OOT_KAKARIKO_BAZAAR + 1					// Market bazaar
#define OOT_KAKARIKO_SHOOTING						OOT_MARKET_BAZAAR + 1					// Kakariko's shooting gallery
#define OOT_MARKET_SHOOTING							OOT_KAKARIKO_SHOOTING + 1				// Kakariko's shooting gallery
#define OOT_SILO									OOT_MARKET_SHOOTING + 1					// It the real Silo and not the "RANCH_HOUSE_SILO"
#define OOT_WINDMILL								OOT_SILO + 1							// Kakariko's windmill
#define OOT_MARKET_ENTRANCE							OOT_WINDMILL + 1						// Market entrance (all age / time)
#define OOT_TEMPLE_OF_TIME_ENTRYWAY					OOT_MARKET_ENTRANCE + 1					// Temple of Time entryway
#define OOT_DAMPE_HOUSE								OOT_TEMPLE_OF_TIME_ENTRYWAY + 1			// Dampe's house
#define OOT_BACK_ALLEY								OOT_DAMPE_HOUSE + 1						// Back Market Alley
#define OOT_SONGS									OOT_BACK_ALLEY + 1						// Warp Songs

// Spoiler Log only
#define OOT_INSIDE_EGGS								OOT_SONGS + 1
#define OOT_MARKET									OOT_INSIDE_EGGS + 1

#define OOT_NUM_SCENES								OOT_MARKET + 1							// OOT and OOT MQ have the same number of scenes

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

#pragma endregion

#pragma region MM

#define MM_SOUTHERN_SWAMP_CLEAR					0x00
#define MM_FAIRY_SNOWHEAD						0x01
#define MM_FAIRY_GREAT_BAY_COAST				0x02
#define MM_FAIRY_WOODFALL						0x03
#define MM_FAIRY_CLOCK_TOWN						0x04
#define MM_FAIRY_IKANA							0x05
#define MM_PATH_SNOWHEAD_SPRING					0x06
#define MM_GROTTOS								0x07
#define MM_CUTSCENE_MAP							0x08
#define MM_PATH_MOUNTAIN_VILLAGE_SPRING			0x09
#define MM_POTION_SHOP							0x0a
#define MM_LAIR_MAJORA							0x0b
#define MM_BENEATH_THE_GRAVEYARD				0x0c
#define MM_CURIOSITY_SHOP						0x0d
#define MM_BENEATH_THE_GRAVEYARD_NIGHT1			0x0e
#define MM_BENEATH_THE_GRAVEYARD_NIGHT2			0x0f
#define MM_RANCH_HOUSE_BARN						0x10
#define MM_HONEY_DARLING						0x11
#define MM_MAYOR_HOUSE							0x12
#define MM_IKANA_CANYON							0x13
#define MM_PIRATE_FORTRESS_EXTERIOR				0x14
#define MM_MILK_BAR								0x15
#define MM_TEMPLE_STONE_TOWER					0x16
#define MM_TREASURE_SHOP						0x17
#define MM_TEMPLE_STONE_TOWER_INVERTED			0x18
#define MM_CLOCK_TOWER_ROOFTOP					0x19
#define MM_OPENING								0x1a
#define MM_TEMPLE_WOODFALL						0x1b
#define MM_PATH_MOUNTAIN_VILLAGE				0x1c
#define MM_CASTLE_IKANA							0x1d
#define MM_DEKU_PLAYGROUND						0x1e
#define MM_LAIR_ODOLWA							0x1f
#define MM_SHOOTING_GALLERY						0x20
#define MM_TEMPLE_SNOWHEAD						0x21
#define MM_MILK_ROAD							0x22
#define MM_PIRATE_FORTRESS_INTERIOR				0x23
#define MM_SHOOTING_GALLERY_SWAMP				0x24
#define MM_PINNACLE_ROCK						0x25
#define MM_FAIRY_FOUNTAIN						0x26
#define MM_SPIDER_HOUSE_SWAMP					0x27
#define MM_SPIDER_HOUSE_OCEAN					0x28
#define MM_OBSERVATORY							0x29
#define MM_MOON_DEKU							0x2a
#define MM_DEKU_PALACE							0x2b
#define MM_BLACKSMITH							0x2c
#define MM_TERMINA_FIELD						0x2d
#define MM_POST_OFFICE							0x2e
#define MM_LABORATORY							0x2f
#define MM_DAMPE_HOUSE							0x30
#define MM_INSIDE_CASTLE_IKANA					0x31
#define MM_GORON_SHRINE							0x32
#define MM_ZORA_HALL							0x33
#define MM_TRADING_POST							0x34
#define MM_ROMANI_RANCH							0x35
#define MM_LAIR_TWINMOLD						0x36
#define MM_GREAT_BAY_COAST						0x37
#define MM_ZORA_CAPE							0x38
#define MM_LOTTERY								0x39
#define MM_SNOWHEAD_SPRING						0x3a
#define MM_PIRATE_FORTRESS_ENTRANCE				0x3b
#define MM_FISHERMAN_HUT						0x3c
#define MM_GORON_SHOP							0x3d
#define MM_DEKU_KING_CHAMBER					0x3e
#define MM_MOON_GORON							0x3f
#define MM_ROAD_SOUTHERN_SWAMP					0x40
#define MM_DOG_RACETRACK						0x41
#define MM_CUCCO_SHACK							0x42
#define MM_IKANA_GRAVEYARD						0x43
#define MM_LAIR_GOHT							0x44
#define MM_SOUTHERN_SWAMP						0x45
#define MM_WOODFALL								0x46
#define MM_MOON_ZORA							0x47
#define MM_GORON_VILLAGE_SPRING					0x48
#define MM_TEMPLE_GREAT_BAY						0x49
#define MM_WATERFALL_RAPIDS						0x4a
#define MM_BENEATH_THE_WELL						0x4b
#define MM_ZORA_HALL_ROOMS						0x4c
#define MM_GORON_VILLAGE_WINTER					0x4d
#define MM_GORON_GRAVEYARD						0x4e
#define MM_SAKON_HIDEOUT						0x4f
#define MM_MOUNTAIN_VILLAGE_WINTER				0x50
#define MM_GHOST_HUT							0x51
#define MM_DEKU_SHRINE							0x52
#define MM_ROAD_IKANA							0x53
#define MM_SWORDSMAN_SCHOOL						0x54
#define MM_MUSIC_BOX_HOUSE						0x55
#define MM_LAIR_IKANA							0x56
#define MM_TOURIST_INFORMATION					0x57
#define MM_STONE_TOWER							0x58
#define MM_STONE_TOWER_INVERTED					0x59
#define MM_MOUNTAIN_VILLAGE_SPRING				0x5a
#define MM_PATH_SNOWHEAD						0x5b
#define MM_SNOWHEAD								0x5c
#define MM_TWIN_ISLANDS_WINTER					0x5d
#define MM_TWIN_ISLANDS_SPRING					0x5e
#define MM_LAIR_GYORG							0x5f
#define MM_SECRET_SHRINE						0x60
#define MM_STOCK_POT_INN						0x61
#define MM_GREAT_BAY_CUTSCENE					0x62
#define MM_CLOCK_TOWER_INTERIOR					0x63
#define MM_WOODS_MYSTERY						0x64
#define MM_LOST_WOODS							0x65
#define MM_MOON_LINK							0x66
#define MM_MOON									0x67
#define MM_BOMB_SHOP							0x68
#define MM_GIANT_CHAMBER						0x69
#define MM_GORMAN_TRACK							0x6a
#define MM_GORON_RACETRACK						0x6b
#define MM_CLOCK_TOWN_EAST						0x6c
#define MM_CLOCK_TOWN_WEST						0x6d
#define MM_CLOCK_TOWN_NORTH						0x6e
#define MM_CLOCK_TOWN_SOUTH						0x6f
#define MM_LAUNDRY_POOL							0x70

#define MM_EXTRA								0x71

// MM detailed other
#define MM_LONE_PEAK							MM_EXTRA + 1							// Lone Peak Shrine
#define MM_ZORA_SHOP							MM_LONE_PEAK + 1						// Zora's shop
#define MM_ZORA_EVANS_ROOM						MM_ZORA_SHOP + 1						// Evans' room
#define MM_ZORA_JAPAS_ROOM						MM_ZORA_EVANS_ROOM + 1					// Japas' room
#define MM_ZORA_TIJO_ROOM						MM_ZORA_JAPAS_ROOM + 1					// Tijo's room
#define MM_ZORA_LULU_ROOM						MM_ZORA_TIJO_ROOM + 1					// Lulu's room
#define MM_ROMANI_RANCH_BARN					MM_ZORA_LULU_ROOM + 1					// Ranch Barn
#define MM_PIRATE_SEWERS						MM_ROMANI_RANCH_BARN + 1				// Pirate sewers
#define MM_OWLS									MM_PIRATE_SEWERS + 1					// Owls choices

// MM detailed grottos
#define MM_GROTTO_TERMINA_DODONGO				MM_OWLS + 1								// Termina field dodongo's grotto
#define MM_GROTTO_TERMINA_OCEAN_GOSSIP			MM_GROTTO_TERMINA_DODONGO + 1			// Termina field ocean gossip's grotto
#define MM_GROTTO_TERMINA_CANYON_GOSSIP			MM_GROTTO_TERMINA_OCEAN_GOSSIP + 1		// Termina field canyon gossip's grotto
#define MM_GROTTO_TERMINA_SWAMP_GOSSIP			MM_GROTTO_TERMINA_CANYON_GOSSIP + 1		// Termina field swamp gossip's grotto
#define MM_GROTTO_TERMINA_MOUNTAIN_GOSSIP		MM_GROTTO_TERMINA_SWAMP_GOSSIP + 1		// Termina field mountain gossip's grotto
#define MM_GROTTO_TERMINA_BIO_BABA				MM_GROTTO_TERMINA_MOUNTAIN_GOSSIP + 1	// Termina field bio baba's grotto
#define MM_GROTTO_TERMINA_PEAHAT				MM_GROTTO_TERMINA_BIO_BABA + 1			// Termina field peehat's grotto
#define MM_GROTTO_TERMINA_SCRUB					MM_GROTTO_TERMINA_PEAHAT + 1			// Termina field scrub grotto
#define MM_GROTTO_TERMINA_TALL_GRASS			MM_GROTTO_TERMINA_SCRUB + 1				// Termina field tall grass grotto
#define MM_GROTTO_TERMINA_COW					MM_GROTTO_TERMINA_TALL_GRASS + 1		// Termina field cow grotto
#define MM_GROTTO_TERMINA_PILLAR				MM_GROTTO_TERMINA_COW + 1				// Termina field pillar grotto
#define MM_GROTTO_GREAT_BAY_COAST_FISHERMAN		MM_GROTTO_TERMINA_PILLAR + 1			// Great bay coast fisherman's grotto
#define MM_GROTTO_GREAT_BAY_COAST_COW			MM_GROTTO_GREAT_BAY_COAST_FISHERMAN + 1	// Great bay coast cow grotto
#define MM_GROTTO_ZORA_CAPE_GENERIC				MM_GROTTO_GREAT_BAY_COAST_COW + 1		// Zora cape generic grotto
#define MM_GROTTO_IKANA_GRAVEYARD_GENERIC		MM_GROTTO_ZORA_CAPE_GENERIC + 1			// Ikana graveyard generic grotto
#define MM_GROTTO_IKANA_VALLEY_OPEN				MM_GROTTO_IKANA_GRAVEYARD_GENERIC + 1	// Ikana valley generic open
#define MM_GROTTO_IKANA_ROAD_GENERIC			MM_GROTTO_IKANA_VALLEY_OPEN + 1			// Ikana road generic grotto
#define MM_GROTTO_TWIN_ISLANDS_FROZEN			MM_GROTTO_IKANA_ROAD_GENERIC + 1		// Twin islands frozen grotto
#define MM_GROTTO_TWIN_ISLANDS_RAMP				MM_GROTTO_TWIN_ISLANDS_FROZEN + 1		// Twin islands ramp grotto
#define MM_GROTTO_PATH_TO_SNOWHEAD_GENERIC		MM_GROTTO_TWIN_ISLANDS_RAMP + 1			// Path to snowhead generic grotto
#define MM_GROTTO_MOUNTAIN_VILLAGE_GENERIC		MM_GROTTO_PATH_TO_SNOWHEAD_GENERIC + 1	// Mountain village generic grotto
#define MM_GROTTO_SOUTHERN_SWAMP_ROAD_OPEN		MM_GROTTO_MOUNTAIN_VILLAGE_GENERIC + 1	// Southern swamp road generic open
#define MM_GROTTO_SOUTHERN_SWAMP_OPEN			MM_GROTTO_SOUTHERN_SWAMP_ROAD_OPEN + 1	// Southern swamp generic open
#define MM_GROTTO_WOODS_OF_MYSTERY_OPEN			MM_GROTTO_SOUTHERN_SWAMP_OPEN + 1		// Woods of mystery generic open
#define MM_GROTTO_DEKU_PALACE_BEANS				MM_GROTTO_WOODS_OF_MYSTERY_OPEN + 1		// Deku palace beans grotto
#define MM_GROTTO_DEKU_PALACE_GENERIC			MM_GROTTO_DEKU_PALACE_BEANS + 1			// Deku palace JP Line generic grotto
#define MM_GROTTO_DEKU_PALACE_CLIMB				MM_GROTTO_DEKU_PALACE_GENERIC + 1		// Deku palace JP Climb generic grotto

// Spoiler Log only
#define MM_MOUNTAIN_VILLAGE						MM_GROTTO_DEKU_PALACE_CLIMB + 1
#define MM_TWIN_ISLANDS							MM_MOUNTAIN_VILLAGE + 1

#define MM_NUM_SCENES							MM_TWIN_ISLANDS + 1	// Here I'm ignoring the JP Line Generic grotto

enum MM_Shops
{
	MM_Clock_Town_Shop = 0x00,
	MM_Swamp_Potion_Shop = 0x01,
	MM_Goron_Zora_Shop = 0x02,
};

#pragma endregion

/*
*   Get the scene meta information that match the desired game and ID.
*
*   @param SceneID		The scene ID to retreive.
*   @param Game			The game in which to get the scene from.
* 
*	@return The matching scene meta information.
*/
SceneMetaInfo* GetSceneMetaInfo(uint32_t SceneID, uint32_t Game);

/*
*   Extract the cow scene ID from given item ID.
*
*   @param ItemID		The item ID to extract the scene from.
*   @param Game			The game in which to get the scene from.
*
*	@return The matching cow scene ID.
*/
uint32_t GetSceneCow(uint32_t ItemID, uint32_t Game);

/*
*   Extract the shop scene ID from given item ID.
*
*   @param ItemID		The item ID to extract the scene from.
*   @param Game			The game in which to get the scene from.
*
*	@return The matching shop scene ID.
*/
uint32_t GetSceneShop(uint32_t ItemID, uint32_t Game);

/*
*   Extract the scene ID from given NPC.
*
*   @param NPC			The NPC to extract the scene from.
*   @param Game			The game in which to get the scene from.
*
*	@return The matching NPC scene ID.
*/
uint32_t GetSceneNPC(uint32_t NPC, uint32_t Game);

/*
*   Extract the scene ID from given silver rupee.
*
*   @param SilverRupee	The silver rupee ID to extract the scene from.
*
*	@return The matching silver rupee scene ID.
*/
uint32_t GetSceneSR(uint32_t SilverRupee);

/*
*   Extract the scene ID from given NPC.
*
*   @param GS			The gold skulltula ID to extract the scene from.
*
*	@return The matching gold skulltula scene ID.
*/
uint32_t GetSceneGS(uint32_t GS);

/*
*   Extract the scene ID from given scrub ID.
*
*   @param Scrub		The scrub ID to extract the scene from.
*
*	@return The matching scrub scene ID.
*/
uint32_t GetSceneScrub(uint32_t Scrub);

/*
*   Get the scene name matching the desired scene ID.
*
*   @param SceneID		The scene ID to get the name of.
*
*	@return The matching scene ID name.
*/
const char* GetSceneName(int Game, uint32_t SceneID);