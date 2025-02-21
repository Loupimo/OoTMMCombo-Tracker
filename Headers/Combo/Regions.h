#pragma once

enum class OoTRegions
{
	None,
	Dungeons,
	Kokiri_Forest,
	Lost_Woods,
	Sacred_Forest_Meadow,
	Hyrule,
	Market,
	Castle,
	Kakariko,
	Graveyard,
	Death_Mountain_Trial,
	Death_Mountain_Crater,
	Goron_City,
	Lon_Lon_Ranch,
	Zora_River,
	Zora_Domain,
	Zora_Fountain,
	Lake_Hylia,
	Gerudo_Valley,
	Gerudo_Fortress,
	Haunted_Wasteland,
	Desert_Colossus
};

enum class MMRegions
{
	None,
	Dungeons,
	Clock_Town,
	Termina,
	Milk_Road,
	Ranch,
	Southern_Swamp_Road,
	Southern_Swamp,
	Deku_Palace,
	Woods_of_Mystery,
	Woodfall,
	Path_to_Mountain_Village,
	Mountain_Village,
	Twin_Islands,
	Goron_Village,
	Path_to_Snowhead,
	Snowhead,
	Great_Bay_Coast,
	Fortress,
	Zora_Cape,
	Zora_Hall,
	Road_to_Ikana,
	Graveyard,
	Ikana_Canyon,
	Stone_Tower,
	Moon
};

typedef struct RegionMetaInfo
{
	const char* RegionName;		// The region name
	const char* Path;			// The icon path that illustrates the region
	uint8_t Region;				// The region ID
} RegionMetaInfo;


// OoT Region meta information
const RegionMetaInfo OoTRegionsMetaInfo[(uint8_t) OoTRegions::Desert_Colossus + 1] =
{
	{ "None", "", (uint8_t)OoTRegions::None},
	{ "Dungeons", "./Resources/Regions/Dungeon.png", (uint8_t)OoTRegions::Dungeons },
	{ "Kokiri Forest", "./Resources/Regions/Kokiri_Forest.png", (uint8_t)OoTRegions::Kokiri_Forest },
	{ "Lost Woods", "./Resources/Regions/Lost_Woods.png", (uint8_t)OoTRegions::Lost_Woods },
	{ "Sacred Forest Meadow", "./Resources/Regions/Sacred_Forest.png", (uint8_t)OoTRegions::Sacred_Forest_Meadow },
	{ "Hyrule Field", "./Resources/Regions/Field.png", (uint8_t)OoTRegions::Hyrule },
	{ "Market", "./Resources/Regions/Hyrule_Market.png", (uint8_t)OoTRegions::Market },
	{ "Hyrule / Ganon Castle", "./Resources/Regions/Hyrule_Castle.png", (uint8_t)OoTRegions::Castle },
	{ "Kakariko", "./Resources/Regions/Kakariko.png", (uint8_t)OoTRegions::Kakariko },
	{ "Graveyard", "./Resources/Regions/Graveyard.png", (uint8_t)OoTRegions::Graveyard },
	{ "Death Mountain Trial", "./Resources/Regions/Mountain_Trail.png", (uint8_t)OoTRegions::Death_Mountain_Trial },
	{ "Death Mountain Crater", "./Resources/Regions/Moutain_Crater.png", (uint8_t)OoTRegions::Death_Mountain_Crater },
	{ "Goron City", "./Resources/Regions/Goron_City.png", (uint8_t)OoTRegions::Goron_City },
	{ "Lon Lon's Ranch", "./Resources/Regions/Ranch.png", (uint8_t)OoTRegions::Lon_Lon_Ranch },
	{ "Zora's River", "./Resources/Regions/Zora_River.png", (uint8_t)OoTRegions::Zora_River },
	{ "Zora's Domain", "./Resources/Regions/Zora_Domain.png", (uint8_t)OoTRegions::Zora_Domain },
	{ "Zora's Fountain", "./Resources/Regions/Zora_Fountain.png", (uint8_t)OoTRegions::Zora_Fountain },
	{ "Lake Hylia", "./Resources/Regions/Lake_Hylia.png", (uint8_t)OoTRegions::Lake_Hylia },
	{ "Gerudo Valley", "./Resources/Regions/Valley.png", (uint8_t)OoTRegions::Gerudo_Valley },
	{ "Gerudo Fortress", "./Resources/Regions/Gerudo_Fortress.png", (uint8_t)OoTRegions::Gerudo_Fortress },
	{ "Haunted Wasteland", "./Resources/Regions/Haunted.png", (uint8_t)OoTRegions::Haunted_Wasteland },
	{ "Desert Colossus", "./Resources/Regions/Colossus.png", (uint8_t)OoTRegions::Desert_Colossus }
};


// MM Region meta information
const RegionMetaInfo MMRegionsMetaInfo[(uint8_t)MMRegions::Moon + 1] =
{
	{ "None", "", (uint8_t)MMRegions::None },
	{ "Dungeons", "./Resources/Regions/Dungeon.png", (uint8_t)MMRegions::Dungeons },
	{ "Clock Town", "./Resources/Regions/Clock_Town.png", (uint8_t)MMRegions::Clock_Town },
	{ "Termina Field", "./Resources/Regions/Field.png", (uint8_t)MMRegions::Termina },
	{ "Milk Road", "./Resources/Regions/Milk_Road.png", (uint8_t)MMRegions::Milk_Road },
	{ "Romani's Ranch", "./Resources/Regions/Ranch.png", (uint8_t)MMRegions::Ranch },
	{ "Road to Southern Swamp", "./Resources/Regions/Road.png", (uint8_t)MMRegions::Southern_Swamp_Road },
	{ "Southern Swamp", "./Resources/Regions/Swamp.png", (uint8_t)MMRegions::Southern_Swamp },
	{ "Deku Palace", "./Resources/Regions/Palace.png", (uint8_t)MMRegions::Deku_Palace },
	{ "Woods of Mystery", "./Resources/Regions/Lost_Woods.png", (uint8_t)MMRegions::Woods_of_Mystery },
	{ "Woodfall", "./Resources/Regions/Woodfall.png", (uint8_t)MMRegions::Woodfall },
	{ "Path to Mountain Village", "./Resources/Regions/Path_to_Village.png", (uint8_t)MMRegions::Path_to_Mountain_Village },
	{ "Mountain Village", "./Resources/Regions/Mountain_Village.png", (uint8_t)MMRegions::Mountain_Village },
	{ "Twin Islands", "./Resources/Regions/Twin_Islands.png", (uint8_t)MMRegions::Twin_Islands },
	{ "Goron Village", "./Resources/Regions/Goron_City.png", (uint8_t)MMRegions::Goron_Village },
	{ "Path to Snowhead", "./Resources/Regions/Path_to_Snowhead.png", (uint8_t)MMRegions::Path_to_Snowhead },
	{ "Snowhead", "./Resources/Regions/Snowhead.png", (uint8_t)MMRegions::Snowhead },
	{ "Great Bay Coast", "./Resources/Regions/Bay.png", (uint8_t)MMRegions::Great_Bay_Coast },
	{ "Pirate's Fortress", "./Resources/Regions/Pirate.png", (uint8_t)MMRegions::Fortress },
	{ "Zora Cape", "./Resources/Regions/Zora_Cape.png", (uint8_t)MMRegions::Zora_Cape },
	{ "Zora Theater", "./Resources/Regions/Theater.png", (uint8_t)MMRegions::Zora_Hall },
	{ "Road to Ikana", "./Resources/Regions/Haunted.png", (uint8_t)MMRegions::Road_to_Ikana },
	{ "Ikana Graveyard", "./Resources/Regions/Graveyard.png", (uint8_t)MMRegions::Graveyard },
	{ "Ikana Canyon", "./Resources/Regions/Valley.png", (uint8_t)MMRegions::Ikana_Canyon },
	{ "Stone Tower", "./Resources/Regions/Stone_Tower.png", (uint8_t)MMRegions::Stone_Tower },
	{ "Moon", "./Resources/Regions/Moon.png", (uint8_t)MMRegions::Moon }
};