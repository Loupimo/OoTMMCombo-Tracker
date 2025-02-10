#pragma once

#include "Multi/API.h"
#include "Regions.h"

typedef struct SceneMetaInfo
{
	const char* Name;
	const char* ImagePath;
	uint8_t ParentRegion;
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
#define OOT_GROTTO_SACRED_MEADOW_STORMS			0x6a	// Sacred Meadow song of storms grotto
#define OOT_GROTTO_KAKARIKO_REDEAD				0x6b	// Kakariko redead grotto
#define OOT_GROTTO_KAKARIKO_OPEN				0x6c	// Kakariko open grotto
#define OOT_GROTTO_DEATH_TRIAL_STORMS			0x6d	// Death mountain trial song of storms grotto
#define OOT_GROTTO_DEATH_TRIAL_COW				0x6e	// Death mountain trial cow grotto
#define OOT_GROTTO_GORON_CITY_SCRUBS			0x6f	// Goron city scrubs grotto
#define OOT_GROTTO_DEATH_CRATER_GENERIC			0x70	// Death mountain crater generic grotto
#define OOT_GROTTO_DEATH_CRATER_SCRUBS			0x71	// Death mountain crater scrubs grotto
#define OOT_GROTTO_ZORA_RIVER_STORMS			0x72	// Zora's river song of storms grotto
#define OOT_GROTTO_ZORA_RIVER_GENERIC			0x73	// Zora's river generic grotto
#define OOT_GROTTO_LAKE_HYLIA_SCRUBS			0x74	// Lake Hylia scrubs grotto
#define OOT_GROTTO_LON_LON_SCRUBS				0x75	// Lon Lon's Ranch scrubs 
#define OOT_GROTTO_HYRULE_SCRUBS				0x76	// Hyrule field scrub grotto
#define OOT_GROTTO_HYRULE_SE					0x77	// Hyrule field southeast grotto
#define OOT_GROTTO_HYRULE_OPEN					0x78	// Hyrule field open grotto
#define OOT_GROTTO_HYRULE_MARKET				0x79	// Hyrule field market grotto
#define OOT_GROTTO_HYRULE_TEKTITE				0x7a	// Hyrule field tektite grotto
#define OOT_GROTTO_HYRULE_KAKARIKO				0x7b	// Hyrule field kakariko grotto
#define OOT_GROTTO_HYRULE_GERUDO				0x7c	// Hyrule field gerudo grotto
#define OOT_GROTTO_CASTLE_STORMS				0x7d	// Hyrule castle song of storms grotto
#define OOT_GROTTO_VALLEY_STORMS				0x7e	// Hyrule castle song of storms grotto
#define OOT_GROTTO_VALLEY_OCTOROK				0x7f	// Hyrule castle octorok grotto
#define OOT_GROTTO_DESERT_SCRUBS				0x80	// Desert colossus scrubs grotto

// OoT detailed fairy scene
#define OOT_FAIRY_SACRED_MEADOW					0x81	// Sacred meadow fairy fountain
#define OOT_FAIRY_ZORA_RIVER					0x82	// Zora's river fairy fountain
#define OOT_FAIRY_ZORA_DOMAIN					0x83	// Zora's domain fairy fountain
#define OOT_FAIRY_HYRULE						0x84	// Hyrule field fairy fountain
#define OOT_FAIRY_GERUDO_FORTRESS				0x85	// Gerudo fortress fairy fountain

// OoT detailed great fairy scene
#define OOT_GREAT_FAIRY_DIN						0x86	// Din's fire fairy fountain
#define OOT_GREAT_FAIRY_FARORE					0x87	// Farore's wind fairy fountain
#define OOT_GREAT_FAIRY_NAYRU					0x88	// Nayru's love fairy fountain
#define OOT_GREAT_FAIRY_MAGIC					0x89	// First magic upgrade fairy fountain
#define OOT_GREAT_FAIRY_MAGIC2					0x8a	// Second magic upgrade fairy fountain
#define OOT_GREAT_FAIRY_DEFENSE					0x8b	// Double defense upgrade fairy fountain

// OoT detailed other scene
#define KAKARIKO_BAZAAR							0x8c	// Kakariko's bazaar
#define MARKET_BAZAAR							0x8d	// Market bazaar
#define KAKARIKO_SHOOTING						0x8e	// Kakariko's shooting gallery
#define MARKET_SHOOTING							0x8f	// Kakariko's shooting gallery
#define SILO									0x90	// It the real Silo and not the "RANCH_HOUSE_SILO"
#define WINDMILL								0x91	// Kakariko's windmill

const SceneMetaInfo OoTScenesMetaInfo[WINDMILL + 1] =
{
	{ "Deku Tree", "./Resources/OoT/Dungeons/Deku_Tree.png", (uint8_t)OoTRegions::Dungeons },
	{ "Dodongo's Cavern", "./Resources/OoT/Dungeons/Dodongo_Cavern.png", (uint8_t)OoTRegions::Dungeons },
	{ "Inside Jabu-Jabu", "./Resources/OoT/Dungeons/Jabu_Jabu.png", (uint8_t)OoTRegions::Dungeons },
	{ "Forest Temple", "./Resources/OoT/Dungeons/Forest_Temple.png", (uint8_t)OoTRegions::Dungeons },
	{ "Fire Temple", "./Resources/OoT/Dungeons/Fire_Temple.png", (uint8_t)OoTRegions::Dungeons },
	{ "Water Temple", "./Resources/OoT/Dungeons/Water_Temple.png", (uint8_t)OoTRegions::Dungeons },
	{ "Spirit Temple", "./Resources/OoT/Dungeons/Spirit_Temple.png", (uint8_t)OoTRegions::Dungeons },
	{ "Shadow Temple", "./Resources/OoT/Dungeons/Shadow_Temple.png", (uint8_t)OoTRegions::Dungeons },
	{ "Bottom of the Well", "./Resources/OoT/Dungeons/Well.png", (uint8_t)OoTRegions::Dungeons },
	{ "Ice Cavern", "./Resources/OoT/Dungeons/Ice_Cavern.png", (uint8_t)OoTRegions::Dungeons },
	{ "Ganon Tower", "./Resources/OoT/Ganon/Tower.png", (uint8_t)OoTRegions::Dungeons },
	{ "Gerudo Training Ground", "./Resources/OoT/Gerudo_Fortress/GTG.png", (uint8_t)OoTRegions::Gerudo_Fortress },
	{ "Thieves Hideout", "./Resources/OoT/Gerudo_Fortress/Thieves_Hideout.png", (uint8_t)OoTRegions::Gerudo_Fortress },
	{ "Inside Ganon Castle", "./Resources/OoT/Ganon/Inside.png", (uint8_t)OoTRegions::Dungeons },
	{ "Ganon Tower - Collapsing", "", (uint8_t)OoTRegions::None },
	{ "Inside Ganon Castle - Collapsing", "", (uint8_t)OoTRegions::None },
	{ "Treasure Shop", "./Resources/OoT/Market/Treasure.png", (uint8_t)OoTRegions::Market },
	{ "Gohma's Lair", "./Resources/OoT/Dungeons/Gohma_Lair.png", (uint8_t)OoTRegions::None },
	{ "King Dodongo's Lair", "./Resources/OoT/Dungeons/King_Dodongo_Lair.png", (uint8_t)OoTRegions::None },
	{ "Barinade's Lair", "./Resources/OoT/Dungeons/Barinade_Lair.png", (uint8_t)OoTRegions::None },
	{ "Phantom Ganon's Lair", "./Resources/OoT/Dungeons/Phantom_Ganon_Lair.png", (uint8_t)OoTRegions::None },
	{ "Volvagia's Lair", "./Resources/OoT/Dungeons/Volvagia_Lair.png", (uint8_t)OoTRegions::None },
	{ "Morpha's Lair", "./Resources/OoT/Dungeons/Morpha_Lair.png", (uint8_t)OoTRegions::None },
	{ "Twinrova's Lair", "./Resources/OoT/Dungeons/Twinrova_Lair.png", (uint8_t)OoTRegions::None },
	{ "Bongo-Bongo's Lair", "./Resources/OoT/Dungeons/Bongo_Bongo_Lair.png", (uint8_t)OoTRegions::None },
	{ "Ganondorf's Lair", "", (uint8_t)OoTRegions::None },
	{ "Tower Collapse - Exterior", "", (uint8_t)OoTRegions::None },
	{ "Market Entrance - Child Day", "", (uint8_t)OoTRegions::None },
	{ "Market Entrance - Child Night", "", (uint8_t)OoTRegions::None },
	{ "Market Entrance - Adult", "", (uint8_t)OoTRegions::None },
	{ "Back Alley - Day", "", (uint8_t)OoTRegions::None },
	{ "Back Alley - Night", "", (uint8_t)OoTRegions::None },
	{ "Market - Child Day", "./Resources/OoT/Market/Market.png", (uint8_t)OoTRegions::Market },
	{ "Market - Child Night", "./Resources/OoT/Market/Market.png", (uint8_t)OoTRegions::Market },
	{ "Market - Adult", "", (uint8_t)OoTRegions::None },
	{ "Temple of Time Exterior Child Day", "", (uint8_t)OoTRegions::None },
	{ "Temple of Time Exterior Child Night", "", (uint8_t)OoTRegions::None },
	{ "Temple of Time Exterior Adult", "", (uint8_t)OoTRegions::None },
	{ "House - Know-It-All Brothers", "./Resources/OoT/Kokiri_Forest/Know_It_All.png", (uint8_t)OoTRegions::Kokiri_Forest },
	{ "House - Twins", "./Resources/OoT/Kokiri_Forest/Twins_House.png", (uint8_t)OoTRegions::Kokiri_Forest },
	{ "House - Mido", "./Resources/OoT/Kokiri_Forest/Mido_House.png", (uint8_t)OoTRegions::Kokiri_Forest },
	{ "House - Saria", "./Resources/OoT/Kokiri_ForestSaria_House.png", (uint8_t)OoTRegions::Kokiri_Forest },
	{ "House - Carpenter's Boss", "./Resources/OoT/Kakariko/Carpenter_Boss_House.png", (uint8_t)OoTRegions::Kakariko },
	{ "House - Back Alley", "./Resources/OoT/Market/Back_Alley_House.png", (uint8_t)OoTRegions::Market },
	{ "Bazaar", "", (uint8_t)OoTRegions::None },
	{ "Shop - Kokiri", "./Resources/OoT/Kokiri_Forest/Shop.png", (uint8_t)OoTRegions::Kokiri_Forest },
	{ "Shop - Goron", "./Resources/OoT/Goron_City/Shop.png", (uint8_t)OoTRegions::Goron_City },
	{ "Shop - Zora", "./Resources/OoT/Zora_Domain/Shop.png", (uint8_t)OoTRegions::Zora_Domain },
	{ "Potion Shop - Kakariko", "./Resources/OoT/Kakariko/Potion_Shop.png", (uint8_t)OoTRegions::Kakariko },
	{ "Potion Shop - Market", "./Resources/OoT/Market/Potion_Shop.png", (uint8_t)OoTRegions::Market },
	{ "Shop - Bombchu", "./Resources/OoT/Market/Bombchu_Shop.png", (uint8_t)OoTRegions::Market },
	{ "Shop - Happy Mask", "", (uint8_t)OoTRegions::None },
	{ "House - Link", "./Resources/OoT/Kokiri_Forest/Link_House.png", (uint8_t)OoTRegions::Kokiri_Forest },
	{ "House - Dog's Lady", "./Resources/OoT/Market/Dog_Lady_House.png", (uint8_t)OoTRegions::Market },
	{ "Stable", "./Resources/OoT/Ranch/Stable.png", (uint8_t)OoTRegions::Lon_Lon_Ranch },
	{ "House - Impa", "./Resources/OoT/Kakariko/Impa_House.png", (uint8_t)OoTRegions::Kakariko },
	{ "Laboratory", "./Resources/OoT/Lake_Hylia/Laboratory.png", (uint8_t)OoTRegions::Lake_Hylia },
	{ "Carpenter's Tent", "", (uint8_t)OoTRegions::None },
	{ "Gravekeeper's Hut", "", (uint8_t)OoTRegions::None },
	{ "Great Fairy Fountain - Upgrades", "", (uint8_t)OoTRegions::None },
	{ "Fairy Fountain", "", (uint8_t)OoTRegions::None },
	{ "Great Fairy Fountain - Spells", "", (uint8_t)OoTRegions::None },
	{ "Grottos", "", (uint8_t)OoTRegions::None },
	{ "Tomb - Redead", "./Resources/OoT/Graveyard/Redead_Tomb.png", (uint8_t)OoTRegions::Graveyard },
	{ "Tomb - Fairy's Foutain", "./Resources/OoT/Graveyard/Fairy_Foutain_Tomb.png", (uint8_t)OoTRegions::Graveyard },
	{ "Tomb - Royal's Family", "./Resources/OoT/Graveyard/Royal_Tomb.png", (uint8_t)OoTRegions::Graveyard },
	{ "Shooting Gallery", "", (uint8_t)OoTRegions::None },
	{ "Temple of Time", "./Resources/OoT/Market/Temple of Time.png", (uint8_t)OoTRegions::Market },
	{ "Chamber of the Sages", "", (uint8_t)OoTRegions::None },
	{ "Castle Maze - Day", "", (uint8_t)OoTRegions::None },
	{ "Castle Maze - Night", "", (uint8_t)OoTRegions::None },
	{ "Cutscene Map", "", (uint8_t)OoTRegions::None },
	{ "Tomb - Dampe", "./Resources/OoT/Graveyard/Dampe_Tomb.png", (uint8_t)OoTRegions::Graveyard },
	{ "Fishing Pond", "./Resources/OoT/Lake_Hylia/Fishing_Pond.png", (uint8_t)OoTRegions::Lake_Hylia },
	{ "Castle Courtyard", "./Resources/OoT/Hyrule/Castle_Courtyard.png", (uint8_t)OoTRegions::Castle },
	{ "Bombchu Bowling", "./Resources/OoT/Market/Bombchu_Bowling.png", (uint8_t)OoTRegions::Market },
	{ "House - Talon", "./Resources/OoT/Ranch/Ranch_House_Silo.png", (uint8_t)OoTRegions::Lon_Lon_Ranch },
	{ "House - Pot", "./Resources/OoT/Market/Guard_House.png", (uint8_t)OoTRegions::Market },
	{ "Potion Shop - Granny", "./Resources/OoT/Kakariko/Granny_Potion_Shop.png", (uint8_t)OoTRegions::Kakariko },
	{ "Ganon Battle Arena", "", (uint8_t)OoTRegions::None },
	{ "House of Skulltula", "./Resources/OoT/Kakariko/House_of_Skulltula.png", (uint8_t)OoTRegions::Kakariko },
	{ "Hyrule Field", "./Resources/OoT/Hyrule/Hyrule_Field.png", (uint8_t)OoTRegions::Hyrule },
	{ "Kakariko's Village", "./Resources/OoT/Kakariko/Kakariko_Village.png", (uint8_t)OoTRegions::Kakariko },
	{ "Graveyard", "./Resources/OoT/Graveyard/Graveyard.png", (uint8_t)OoTRegions::Graveyard },
	{ "Zora's River", "./Resources/OoT/Zora_River/Zora_River.png", (uint8_t)OoTRegions::Zora_River },
	{ "Kokiri Forest", "./Resources/OoT/Kokiri_Forest/Kokiri_Forest.png", (uint8_t)OoTRegions::Kokiri_Forest },
	{ "Sacred Forest Meadow", "./Resources/OoT/Sacred_Forest_Meadow/Sacred_Forest_Meadow.png", (uint8_t)OoTRegions::Sacred_Forest_Meadow },
	{ "Lake Hylia", "./Resources/OoT/Lake_Hylia/Lake_Hylia.png", (uint8_t)OoTRegions::Lake_Hylia },
	{ "Zora's Domain", "./Resources/OoT/Zora_Domain/Zora_Domain.png", (uint8_t)OoTRegions::Zora_Domain },
	{ "Zora's Fountain", "./Resources/OoT/Zora_Fountain/Zora_Fountain.png", (uint8_t)OoTRegions::Zora_Fountain },
	{ "Gerudo Valley", "./Resources/OoT/Gerudo_Valley/Valley.png", (uint8_t)OoTRegions::Gerudo_Valley },
	{ "Lost Woods", "./Resources/OoT/Lost_Woods/Lost_Woods.png", (uint8_t)OoTRegions::Lost_Woods },
	{ "Desert Colossus", "./Resources/OoT/Desert_Colossus/Desert_Colossus.png", (uint8_t)OoTRegions::Desert_Colossus },
	{ "Gerudo's Fortress", "./Resources/OoT/Gerudo_Fortress/Fortress.png", (uint8_t)OoTRegions::Gerudo_Fortress },
	{ "Haunted Wasteland", "./Resources/OoT/Haunted_Wasteland/Haunted_Wasteland.png", (uint8_t)OoTRegions::Haunted_Wasteland },
	{ "Hyrule Castle", "./Resources/OoT/Hyrule/Hyrule_Castle.png", (uint8_t)OoTRegions::Castle },
	{ "Death Mountain Trail", "./Resources/OoT/Death_Mountain_Trial/Trail.png", (uint8_t)OoTRegions::Death_Mountain_Trial },
	{ "Death Mountain Crater", "./Resources/OoT/Death_Mountain_Crater/Crater.png", (uint8_t)OoTRegions::Death_Mountain_Crater },
	{ "Goron City", "./Resources/OoT/Goron_City/Goron_City.png", (uint8_t)OoTRegions::Goron_City },
	{ "Lon Lon's Ranch", "./Resources/OoT/Ranch/Lon_Lon_Ranch.png", (uint8_t)OoTRegions::Lon_Lon_Ranch },
	{ "Ganon Castle - Exterior", "./Resources/OoT/Ganon/Exterior.png", (uint8_t)OoTRegions::Castle },
	{ "Grotto - Song of Storms", "./Resources/OoT/Kokiri_Forest/Storms.png", (uint8_t)OoTRegions::Kokiri_Forest },
	{ "Grotto - Deku Scrub Upgrade", "./Resources/OoT/Lost_Woods/Scrub_Upgrade.png", (uint8_t)OoTRegions::Lost_Woods },
	{ "Grotto - Generic", "./Resources/OoT/Lost_Woods/Generic.png", (uint8_t)OoTRegions::Lost_Woods },
	{ "Grotto - Deku's Theater", "./Resources/OoT/Lost_Woods/Theater.png", (uint8_t)OoTRegions::Lost_Woods },
	{ "Grotto - Wolfos", "./Resources/OoT/Sacred_Forest_Meadow/Wolfos.png", (uint8_t)OoTRegions::Lost_Woods },
	{ "Grotto - Song of Storms", "./Resources/OoT/Sacred_Forest_Meadow/Storms.png", (uint8_t)OoTRegions::Sacred_Forest_Meadow },
	{ "Grotto - Redead", "./Resources/OoT/Kakariko/Redead.png", (uint8_t)OoTRegions::Kakariko },
	{ "Grotto - Open", "./Resources/OoT/Kakariko/Open.png", (uint8_t)OoTRegions::Kakariko },
	{ "Grotto - Song of Storms", "./Resources/OoT/Death_Mountain_Trial/Storms.png", (uint8_t)OoTRegions::Death_Mountain_Trial },
	{ "Grotto - Cow", "./Resources/OoT/Death_Mountain_Trial/Cow.png", (uint8_t)OoTRegions::Death_Mountain_Trial },
	{ "Grotto - Deku Scrubs", "./Resources/OoT/Goron_City/Scrubs.png", (uint8_t)OoTRegions::Goron_City },
	{ "Grotto - Generic", "./Resources/OoT/Death_Mountain_Crater/Generic.png", (uint8_t)OoTRegions::Death_Mountain_Crater },
	{ "Grotto - Deku Scrubs", "./Resources/OoT/Death_Mountain_Crater/Scrubs.png", (uint8_t)OoTRegions::Death_Mountain_Crater },
	{ "Grotto - Song of Storms", "./Resources/OoT/Zora_River/Storms.png", (uint8_t)OoTRegions::Zora_River },
	{ "Grotto - Generic", "./Resources/OoT/Zora_River/Generic.png", (uint8_t)OoTRegions::Zora_River },
	{ "Grotto - Deku Scrubs", "./Resources/OoT/Lake_Hylia/Scrubs.png", (uint8_t)OoTRegions::Lake_Hylia },
	{ "Grotto - Deku Scrubs", "./Resources/OoT/Ranch/Scrubs.png", (uint8_t)OoTRegions::Lon_Lon_Ranch },
	{ "Grotto - Deku Scrubs", "./Resources/OoT/Hyrule/Field_Scrubs.png", (uint8_t)OoTRegions::Hyrule },
	{ "Grotto - Southeast", "./Resources/OoT/Hyrule/Field_SE.png", (uint8_t)OoTRegions::Hyrule },
	{ "Grotto - Open", "./Resources/OoT/Hyrule/Field_Open.png", (uint8_t)OoTRegions::Hyrule },
	{ "Grotto - Market Side", "./Resources/OoT/Hyrule/Field_Market.png", (uint8_t)OoTRegions::Hyrule },
	{ "Grotto - Tektite", "./Resources/OoT/Hyrule/Field_Tektite.png", (uint8_t)OoTRegions::Hyrule },
	{ "Grotto - Kakariko Side", "./Resources/OoT/Hyrule/Field_Kakariko.png", (uint8_t)OoTRegions::Hyrule },
	{ "Grotto - Gerudo Side", "./Resources/OoT/Hyrule/Field_Gerudo.png", (uint8_t)OoTRegions::Hyrule },
	{ "Grotto - Song of Storms", "./Resources/OoT/Hyrule/Castle_Storms.png", (uint8_t)OoTRegions::Castle },
	{ "Grotto - Song of Storms", "./Resources/OoT/Gerudo_Valley/Storms.png", (uint8_t)OoTRegions::Gerudo_Valley },
	{ "Grotto - Octorok", "./Resources/OoT/Gerudo_Valley/Octorok.png", (uint8_t)OoTRegions::Gerudo_Valley },
	{ "Grotto - Deku Scrubs", "./Resources/OoT/Desert_Colossus/Scrubs.png", (uint8_t)OoTRegions::Desert_Colossus },
	{ "Fairy Fountain - Sacred Forest Meadow", "./Resources/OoT/Sacred_Forest_Meadow/Fountain.png", (uint8_t)OoTRegions::Sacred_Forest_Meadow },
	{ "Fairy Fountain - Zora's River", "./Resources/OoT/Zora_River/Fountain.png", (uint8_t)OoTRegions::Zora_River },
	{ "Fairy Fountain - Zora's Domain", "./Resources/OoT/Zora_Domain/Fountain.png", (uint8_t)OoTRegions::Zora_Domain },
	{ "Fairy Fountain - Hyrule Field", "./Resources/OoT/Hyrule/Field_Fountain.png", (uint8_t)OoTRegions::Hyrule },
	{ "Fairy Fountain - Gerudo's Fortress", "./Resources/OoT/Gerudo_Fortress/Fountain.png", (uint8_t)OoTRegions::Gerudo_Fortress },
	{ "Great Fairy - Din", "./Resources/OoT/Hyrule/Great_Fairy.png", (uint8_t)OoTRegions::Castle },
	{ "Great Fairy - Farore", "./Resources/OoT/Zora_Fountain/Great_Fairy.png", (uint8_t)OoTRegions::Zora_Fountain },
	{ "Great Fairy - Nayru", "./Resources/OoT/Desert_Colossus/Great_Fairy.png", (uint8_t)OoTRegions::Desert_Colossus },
	{ "Great Fairy - First Magic Upgrade", "./Resources/OoT/Death_Moutain_Trail/Great_Fairy.png", (uint8_t)OoTRegions::Death_Mountain_Trial },
	{ "Great Fairy - Second Magic Upgrade", "./Resources/OoT/Death_Moutain_Crater/Great_Fairy.png", (uint8_t)OoTRegions::Death_Mountain_Crater },
	{ "Great Fairy - Double Defense", "./Resources/OoT/Ganon/Great_Fairy.png", (uint8_t)OoTRegions::Castle },
	{ "Bazaar - Kakariko", "./Resources/OoT/Kakariko/Bazaar.png", (uint8_t)OoTRegions::Kakariko },
	{ "Bazaar - Market", "./Resources/OoT/Market/Bazaar.png", (uint8_t)OoTRegions::Market },
	{ "Shooting Gallery - Kakariko", "./Resources/OoT/Kakariko/Shooting.png", (uint8_t)OoTRegions::Kakariko },
	{ "Shooting Gallery - Market", "./Resources/OoT/Market/Shooting.png", (uint8_t)OoTRegions::Market },
	{ "Silo", "./Resources/OoT/Ranch/Silo.png", (uint8_t)OoTRegions::Lon_Lon_Ranch },
	{ "Windmill", "./Resources/OoT/Kakariko/Windmill.png", (uint8_t)OoTRegions::Kakariko }
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
#define LONE_PEAK								0x72	// Lone Peak Shrine
#define MM_ZORA_SHOP							0x73
#define ZORA_EVAN_ROOM							0x74

// MM detailed grottos
#define MM_GROTTO_TERMINA_DODONGO				0x75	// Termina field dodongo's grotto
#define MM_GROTTO_TERMINA_OCEAN_GOSSIP			0x76	// Termina field ocean gossip's grotto
#define MM_GROTTO_TERMINA_CANYON_GOSSIP			0x77	// Termina field canyon gossip's grotto
#define MM_GROTTO_TERMINA_BIO_BABA				0x78	// Termina field bio baba's grotto
#define MM_GROTTO_TERMINA_PEEHAT				0x79	// Termina field peehat's grotto
#define MM_GROTTO_TERMINA_SCRUB					0x7a	// Termina field scrub grotto
#define MM_GROTTO_TERMINA_TALL_GRASS			0x7b	// Termina field tall grass grotto
#define MM_GROTTO_TERMINA_COW					0x7c	// Termina field cow grotto
#define MM_GROTTO_TERMINA_PILLAR				0x7d	// Termina field pillar grotto
#define MM_GROTTO_DEKU_PALACE_GENERIC			0x7e	// Deku palace generic grotto
#define MM_GROTTO_DEKU_PALACE_BEANS				0x7f	// Deku palace beans grotto
#define MM_GROTTO_GREAT_BAY_COAST_FISHERMAN		0x80	// Great bay coast fisherman's grotto
#define MM_GROTTO_GREAT_BAY_COAST_COW			0x81	// Great bay coast cow grotto
#define MM_GROTTO_ZORA_CAPE_GENERIC				0x82	// Zora cape generic grotto
#define MM_GROTTO_IKANA_GRAVEYARD_GENERIC		0x83	// Ikana graveyard generic grotto
#define MM_GROTTO_IKANA_VALLEY_OPEN				0x84	// Ikana valley generic open
#define MM_GROTTO_IKANA_ROAD_GENERIC			0x85	// Ikana road generic grotto
#define MM_GROTTO_TWIN_ISLANDS_FROZEN			0x86	// Twin islands frozen grotto
#define MM_GROTTO_TWIN_ISLANDS_RAMP				0x87	// Twin islands ramp grotto
#define MM_GROTTO_PATH_TO_SNOWHEAD_GENERIC		0x88	// Path to snowhead generic grotto
#define MM_GROTTO_MOUNTAIN_VILLAGE_GENERIC		0x89	// Mountain village generic grotto
#define MM_GROTTO_SOUTHERN_SWAMP_ROAD_OPEN		0x8a	// Southern swamp road generic open
#define MM_GROTTO_SOUTHERN_SWAMP_OPEN			0x8b	// Southern swamp generic open
#define MM_GROTTO_WOODS_OF_MYSTERY_OPEN			0x8c	// Woods of mystery generic open


const SceneMetaInfo MMScenesMetaInfo[MM_GROTTO_WOODS_OF_MYSTERY_OPEN + 1] =
{
	{ "Southern Swamp - Clear", "", (uint8_t)MMRegions::None },
	{ "Fairy Fountain - Snowhead", "./Resources/MM/Snowhead/Fairy_Fountain.png", (uint8_t)MMRegions::Snowhead },
	{ "Fairy Fountain - Zora Cape", "./Resources/MM/Zora_Cape/Fairy_Fountain.png", (uint8_t)MMRegions::Zora_Cape },
	{ "Fairy Fountain - Woodfall", "./Resources/MM/Woodfall/Fairy_Fountain.png", (uint8_t)MMRegions::Woodfall },
	{ "Fairy Fountain - Clock Town", "./Resources/MM/Clock_Town/Fairy Fountain.png", (uint8_t)MMRegions::Clock_Town },
	{ "Fairy Fountain - Ikana Canyon", "./Resources/MM/Ikana_Canyon/Fairy_Fountain.png", (uint8_t)MMRegions::Ikana_Canyon },
	{ "Path to Snowhead - Spring", "./Resources/MM/Path_to_Snowhead/Spring.png", (uint8_t)MMRegions::Path_to_Snowhead },
	{ "Grottos", "", (uint8_t)MMRegions::None },
	{ "Cutscene Map", "", (uint8_t)MMRegions::None },
	{ "Path to Mountain Village - Spring", "./Resources/MM/Path_to_Mountain_Village/Spring.png", (uint8_t)MMRegions::Path_to_Mountain_Village },
	{ "Potion Shop", "./Resources/MM/Southern_Swamp/Potion_Shop.png", (uint8_t)MMRegions::Southern_Swamp },
	{ "Majora's Lair", "./Resources/MM/Moon/Majora_Lair.png", (uint8_t)MMRegions::Moon },
	{ "Beneath the Graveyard", "", (uint8_t)MMRegions::None },
	{ "Curiosity Shop", "./Resources/MM/Clock_Town/Curiosity_Shop.png", (uint8_t)MMRegions::Clock_Town },
	{ "Beneath the Graveyard - Night 1", "./Resources/MM/Graveyard/Beneath_Graveyard_N1.png", (uint8_t)MMRegions::Graveyard },
	{ "Beneath the Graveyard - Night 2", "./Resources/MM/Graveyard/Beneath_Graveyard_N2.png", (uint8_t)MMRegions::Graveyard },
	{ "House - Ranch Barn", "./Resources/MM/Ranch/House_Barn.png", (uint8_t)MMRegions::Ranch },
	{ "House - Honey Darling", "./Resources/MM/Clock_Town/Honey_Darling.png", (uint8_t)MMRegions::Clock_Town },
	{ "House - Mayor", "./Resources/MM/Clock_Town/Mayor_House.png", (uint8_t)MMRegions::Clock_Town },
	{ "Ikana Canyon", "./Resources/MM/Ikana_Canyon/Ikana_Canyon.png", (uint8_t)MMRegions::Ikana_Canyon },
	{ "Pirate's Fortress - Exterior", "./Resources/MM/Fortress/Exterior.png", (uint8_t)MMRegions::Fortress },
	{ "House - Milk Bar", "./Resources/MM/Clock_Town/Milk_Bar.png", (uint8_t)MMRegions::Clock_Town },
	{ "Stone Tower Temple", "./Resources/MM/Stone_Tower/Temple.png", (uint8_t)MMRegions::Dungeons },
	{ "Treasure Shop", "./Resources/MM/Clock_Town/Treasure Shop.png", (uint8_t)MMRegions::Clock_Town },
	{ "Stone Tower Temple - Inverted", "./Resources/MM/Stone_Tower/Temple_Inverted.png", (uint8_t)MMRegions::Dungeons },
	{ "Clock Tower - Rooftop", "./Resources/MM/Clock_Town/Tower_Rooftop.png", (uint8_t)MMRegions::Clock_Town },
	{ "Opening", "", (uint8_t)MMRegions::None },
	{ "Woodfall Temple", "./Resources/MM/Woodfall/Temple.png", (uint8_t)MMRegions::Dungeons },
	{ "Path to Mountain Village - Winter", "./Resources/MM/Path_to_Mountain_Village/Winter.png", (uint8_t)MMRegions::Path_to_Mountain_Village },
	{ "Ancient Ikana Castle", "./Resources/MM/Ikana/Ancient_Ikana_Castle.png", (uint8_t)MMRegions::Dungeons },
	{ "Deku's Playground", "./Resources/MM/Clock_Town/Deku_Playground.png", (uint8_t)MMRegions::Clock_Town },
	{ "Odolwa's Lair", "./Resources/MM/Woodfall/Odolwa_Lair.png", (uint8_t)MMRegions::Dungeons },
	{ "Shooting Gallery - Clock Town", "./Resources/MM/Clock_Town/Shooting_Gallery.png", (uint8_t)MMRegions::Clock_Town },
	{ "Snowhead Temple", "./Resources/MM/Snowhead/Temple.png", (uint8_t)MMRegions::Dungeons },
	{ "Milk Road", "./Resources/MM/Milk_Road/Road.png", (uint8_t)MMRegions::Milk_Road },
	{ "Pirate's Fortress - Interior", "./Resources/MM/Fortress/Interior.png", (uint8_t)MMRegions::Fortress },
	{ "Shooting Gallery - Road to Southern Swamp", "./Resources/MM/Road_to_Southern_Swamp/Shooting_Gallery.png", (uint8_t)MMRegions::Southern_Swamp_Road },
	{ "Pinnacle Rock", "./Resources/MM/Great_Bay_Coast/Pinnacle_Rock.png", (uint8_t)MMRegions::Great_Bay_Coast },
	{ "Fairy Fountain", "", (uint8_t)MMRegions::None },
	{ "Spider's House - Southern Swamp", "./Resources/MM/Southern_Swamp/Spider_House.png", (uint8_t)MMRegions::Southern_Swamp },
	{ "Spider's House - Great Bay Coast", "./Resources/MM/Great_Bay_Coast/Spider_House.png", (uint8_t)MMRegions::Great_Bay_Coast },
	{ "Observatory", "./Resources/MM/Clock_Town/Observatory.png", (uint8_t)MMRegions::Clock_Town },
	{ "Moon - Deku's Trial", "./Resources/MM/Moon/Deku.png", (uint8_t)MMRegions::Moon },
	{ "Deku's Palace", "./Resources/MM/Southern_Swamp/Deku_Palace.png", (uint8_t)MMRegions::Deku_Palace },
	{ "Blacksmith", "./Resources/MM/Mountain_Village/Blacksmith.png", (uint8_t)MMRegions::Mountain_Village },
	{ "Termina Field", "./Resources/MM/Termina/Field.png", (uint8_t)MMRegions::Termina },
	{ "Post Office", "./Resources/MM/Clock_Town/Post_Office.png", (uint8_t)MMRegions::Clock_Town },
	{ "Laboratory", "./Resources/MM/Great_Bay_Coast/Laboratory.png", (uint8_t)MMRegions::Great_Bay_Coast },
	{ "Dampe's House", "./Resources/MM/Graveyard/Dampe_House.png", (uint8_t)MMRegions::Graveyard },
	{ "Inside Ancient Ikana Castle", "./Resources/MM/Ikana/Inside_Castle.png", (uint8_t)MMRegions::Dungeons },
	{ "Goron's Shrine", "./Resources/MM/Goron_Village/Goron_Shrine.png", (uint8_t)MMRegions::Goron_Village },
	{ "Zora's Theater", "./Resources/MM/Zora_Hall/Zora_Hall.png", (uint8_t)MMRegions::Zora_Hall },
	{ "Trading Post", "./Resources/MM/Clock_Town/Trading_Post.png", (uint8_t)MMRegions::Clock_Town },
	{ "Romani's Ranch", "./Resources/MM/Ranch/Romani_Ranch.png", (uint8_t)MMRegions::Ranch },
	{ "Twinmold Lair", "./Resources/MM/Stone_Tower/Twinmold_Lair.png", (uint8_t)MMRegions::Dungeons },
	{ "Great Bay Coast", "./Resources/MM/Great_Bay_Coast/Great_Bay.png", (uint8_t)MMRegions::Great_Bay_Coast },
	{ "Zora Cape", "./Resources/MM/Zora_Cape/Zora_Cape.png", (uint8_t)MMRegions::Zora_Cape },
	{ "Lottery", "", (uint8_t)MMRegions::None },
	{ "Snowhead - Spring", "./Resources/MM/Snowhead/Spring.png", (uint8_t)MMRegions::Snowhead },
	{ "Pirate's Fortress - Entrance", "./Resources/MM/Fortress/Entrance.png", (uint8_t)MMRegions::Fortress },
	{ "House - Fisherman", "./Resources/MM/Great_Bay_Coast/Fisherman_Hut.png", (uint8_t)MMRegions::Great_Bay_Coast },
	{ "Shop - Goron", "./Resources/MM/Goron_Village/Goron_Shop.png", (uint8_t)MMRegions::Goron_Village },
	{ "Deku King's Chamber", "./Resources/MM/Southern_Swamp/Deku_King_Chamber.png", (uint8_t)MMRegions::Deku_Palace },
	{ "Moon - Goron's Trial", "./Resources/MM/Moon/Goron.png", (uint8_t)MMRegions::Moon },
	{ "Road to Southern Swamp", "./Resources/MM/Road_to_Southern_Swamp/Road.png", (uint8_t)MMRegions::Southern_Swamp_Road },
	{ "Dog's Racetrack", "./Resources/MM/Ranch/Dog_Racetrack.png", (uint8_t)MMRegions::Ranch },
	{ "Cucco's Shack", "./Resources/MM/Ranch/Cucco_Shack.png", (uint8_t)MMRegions::Ranch },
	{ "Ikana Graveyard", "./Resources/MM/Graveyard/Graveyard.png", (uint8_t)MMRegions::Graveyard },
	{ "Goht's Lair", "./Resources/MM/Snowhead/Goht_Lair.png", (uint8_t)MMRegions::Dungeons },
	{ "Southern Swamp", "./Resources/MM/Southern_Swamp/Swamp.png", (uint8_t)MMRegions::Southern_Swamp },
	{ "Woodfall", "./Resources/MM/Woodfall/Woodfall.png", (uint8_t)MMRegions::Woodfall },
	{ "Moon - Zora's Trial", "./Resources/MM/Moon/Zora.png", (uint8_t)MMRegions::Moon },
	{ "Goron's Village - Spring", "./Resources/MM/Goron_Village/Spring.png", (uint8_t)MMRegions::Goron_Village },
	{ "Great Bay Temple", "./Resources/MM/Great_Bay_Coast/Temple.png", (uint8_t)MMRegions::Dungeons },
	{ "Waterfall Rapids", "./Resources/MM/Zora_Cape/Waterfall_Rapids.png", (uint8_t)MMRegions::Zora_Cape },
	{ "Beneath the Well", "./Resources/MM/Ikana_Canyon/Beneath_the_Well.png", (uint8_t)MMRegions::Ikana_Canyon },
	{ "Lulu's Room", "./Resources/MM/Zora_Hall/Lulu_Room.png", (uint8_t)MMRegions::Zora_Hall },
	{ "Goron's Village - Winter", "./Resources/MM/Goron_Village/Winter.png", (uint8_t)MMRegions::Goron_Village },
	{ "Goron's Graveyard", "./Resources/MM/Mountain_Village/Graveyard.png", (uint8_t)MMRegions::Mountain_Village },
	{ "Sakon's Hideout", "./Resources/MM/Ikana_Canyon/Sakon_Hideout.png", (uint8_t)MMRegions::Ikana_Canyon },
	{ "Mountain Village - Winter", "./Resources/MM/Mountain_Village/Winter.png", (uint8_t)MMRegions::Mountain_Village },
	{ "Ghost Hut", "./Resources/MM/Ikana_Canyon/Ghost_Hut.png", (uint8_t)MMRegions::Ikana_Canyon },
	{ "Deku's Shrine", "./Resources/MM/Deku_Palace/Deku_Shrine.png", (uint8_t)MMRegions::Deku_Palace },
	{ "Road to Ikana", "./Resources/MM/Road_to_Ikana/Road.png", (uint8_t)MMRegions::Road_to_Ikana },
	{ "Swordsman's School", "./Resources/MM/Clock_Town/Swordsman_School.png", (uint8_t)MMRegions::Clock_Town },
	{ "House - Music Box", "./Resources/MM/Ikana_Canyon/Music_Box_House.png", (uint8_t)MMRegions::Ikana_Canyon },
	{ "Ikana's Lair", "./Resources/MM/Dungeons/Ikana_Lair.png", (uint8_t)MMRegions::Dungeons },
	{ "Tourist Information", "./Resources/MM/Southern_Swamp/Tourist_Information.png", (uint8_t)MMRegions::Southern_Swamp },
	{ "Stone Tower", "./Resources/MM/Stone_Tower/Tower.png", (uint8_t)MMRegions::Stone_Tower },
	{ "Stone Tower - Inverted", "./Resources/MM/Stone_Tower/Tower_Inverted.png", (uint8_t)MMRegions::Stone_Tower },
	{ "Mountain Village - Spring", "./Resources/MM/Mountain_Village/Spring.png", (uint8_t)MMRegions::Mountain_Village },
	{ "Path to Snowhead - Winter", "./Resources/MM/Path_to_Snowhead/Winter.png", (uint8_t)MMRegions::Path_to_Snowhead },
	{ "Snowhead", "./Resources/MM/Snowhed/Snowhead.png", (uint8_t)MMRegions::Snowhead },
	{ "Twin Islands - Winter", "./Resources/MM/Twin_Islands/Winter.png", (uint8_t)MMRegions::Twin_Islands },
	{ "Twin Islands - Spring", "./Resources/MM/Twin_Islands/Spring.png", (uint8_t)MMRegions::Twin_Islands },
	{ "Gyorg's Lair", "./Resources/MM/Great_Bay_Coast/Gyorg_Lair.png", (uint8_t)MMRegions::Dungeons },
	{ "Secret's Shrine", "./Resources/MM/Ikana_Canyon/Secret_Shrine.png", (uint8_t)MMRegions::Ikana_Canyon },
	{ "Stock Pot Inn", "./Resources/MM/Clock_Town/Stock_Pot_Inn.png", (uint8_t)MMRegions::Clock_Town },
	{ "Great Bay Cutscene", "", (uint8_t)MMRegions::None },
	{ "Clock Tower - Interior", "", (uint8_t)MMRegions::None },
	{ "Woods of Mystery", "./Resources/MM/Woods_of_Mystery/Woods_Mystery.png", (uint8_t)MMRegions::Woods_of_Mystery },
	{ "Lost Woods", "", (uint8_t)MMRegions::None },
	{ "Moon - Link's Trial", "./Resources/MM/Moon/Link.png", (uint8_t)MMRegions::Moon },
	{ "Moon", "./Resources/MM/Moon/Moon.png", (uint8_t)MMRegions::Moon },
	{ "Bomb Shop", "./Resources/MM/Clock_Town/Bomb_Shop.png", (uint8_t)MMRegions::Clock_Town },
	{ "Giant Chamber", "", (uint8_t)MMRegions::None },
	{ "Gorman's Track", "./Resources/MM/Milk_Road/Gorman_Track.png", (uint8_t)MMRegions::Milk_Road },
	{ "Goron's Racetrack", "./Resources/MM/Twin_Islands/Goron_Racetrack.png", (uint8_t)MMRegions::Twin_Islands },
	{ "Clock Town - East", "./Resources/MM/Clock_Town/East.png", (uint8_t)MMRegions::Clock_Town },
	{ "Clock Town - West", "./Resources/MM/Clock_Town/West.png", (uint8_t)MMRegions::Clock_Town },
	{ "Clock Town - North", "./Resources/MM/Clock_Town/North.png", (uint8_t)MMRegions::Clock_Town },
	{ "Clock Town - South", "./Resources/MM/Clock_Town/South.png", (uint8_t)MMRegions::Clock_Town },
	{ "Laundry Pool", "./Resources/MM/Clock_Town/Laundry Pool.png", (uint8_t)MMRegions::Clock_Town },
	{ "Extra", "", (uint8_t)MMRegions::None },
	{ "Lone Peak Shrine", "./Resources/MM/Goron_Village/Lone_Peak_Shrine.png", (uint8_t)MMRegions::Goron_Village },
	{ "Shop - Zora", "./Resources/MM/Zora_Hall/Zora_Shop.png", (uint8_t)MMRegions::Zora_Hall },
	{ "Evan's Room", "./Resources/MM/Zora_Hall/Evan_Room.png", (uint8_t)MMRegions::Zora_Hall },
	{ "Grotto - Dodongo", "./Resources/MM/Termina/Dodongo.png", (uint8_t)MMRegions::Termina },
	{ "Grotto - Ocean Gossip", "./Resources/MM/Termina/Ocean_Gossip.png", (uint8_t)MMRegions::Termina },
	{ "Grotto - Canyon Gossip", "./Resources/MM/Termina/Canyon_Gossip.png", (uint8_t)MMRegions::Termina },
	{ "Grotto - Bio Baba", "./Resources/MM/Termina/Bio_Baba.png", (uint8_t)MMRegions::Termina },
	{ "Grotto - Peehat", "./Resources/MM/Termina/Peehat.png", (uint8_t)MMRegions::Termina },
	{ "Grotto - Deku Scrub", "./Resources/MM/Termina/Scrub.png", (uint8_t)MMRegions::Termina },
	{ "Grotto - Tall Grass", "./Resources/MM/Termina/Tall_Grass.png", (uint8_t)MMRegions::Termina },
	{ "Grotto - Cow", "./Resources/MM/Termina/Cow.png", (uint8_t)MMRegions::Termina },
	{ "Grotto - Pillar", "./Resources/MM/Termina/Pillar.png", (uint8_t)MMRegions::Termina },
	{ "Grotto - Generic", "./Resources/MM/Deku_Palace/Generic.png", (uint8_t)MMRegions::Deku_Palace },
	{ "Grotto - Beans", "./Resources/MM/Deku_Palace/Beans.png", (uint8_t)MMRegions::Deku_Palace },
	{ "Grotto - Open", "./Resources/MM/Great_Bay_Coast/Open.png", (uint8_t)MMRegions::Great_Bay_Coast },
	{ "Grotto - Cow", "./Resources/MM/Great_Bay_Coast/Cow.png", (uint8_t)MMRegions::Great_Bay_Coast },
	{ "Grotto - Generic", "./Resources/MM/Zora_Cape/Generic.png", (uint8_t)MMRegions::Zora_Cape },
	{ "Grotto - Generic", "./Resources/MM/Garveyard/Generic.png", (uint8_t)MMRegions::Graveyard },
	{ "Grotto - Open", "./Resources/MM/Ikana_Canyon/Open.png", (uint8_t)MMRegions::Ikana_Canyon },
	{ "Grotto - Generic", "./Resources/MM/Road_to_Ikana/Generic.png", (uint8_t)MMRegions::Road_to_Ikana },
	{ "Grotto - Frozen", "./Resources/MM/Twin_Islands/Frozen.png", (uint8_t)MMRegions::Twin_Islands },
	{ "Grotto - Ramp", "./Resources/MM/Twin_Islands/Ramp.png", (uint8_t)MMRegions::Twin_Islands },
	{ "Grotto - Generic", "./Resources/MM/Path_to_Snowhead/Generic.png", (uint8_t)MMRegions::Path_to_Snowhead },
	{ "Grotto - Generic", "./Resources/MM/Mountain_Village/Generic.png", (uint8_t)MMRegions::Mountain_Village },
	{ "Grotto - Open", "./Resources/MM/Road_to_Southern_Swamp/Open.png", (uint8_t)MMRegions::Southern_Swamp_Road },
	{ "Grotto - Open", "./Resources/MM/Southern_Swamp/Open.png", (uint8_t)MMRegions::Southern_Swamp },
	{ "Grotto - Open", "./Resources/MM/Woods_of_Mystery/Woods_Open", (uint8_t)MMRegions::Woods_of_Mystery }
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

const SceneMetaInfo* GetSceneMetaInfo(uint32_t SceneID, uint32_t Game);
uint32_t GetSceneShop(uint32_t ItemID, uint32_t Game);
uint32_t GetSceneNPC(uint32_t NPC, uint32_t Game);
uint32_t GetSceneSR(uint32_t SilverRupee);
uint32_t GetSceneGS(uint32_t GS);
uint32_t GetSceneScrub(uint32_t Scrub);