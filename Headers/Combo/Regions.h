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
	const char* RegionName;
	uint8_t Region;
}RegionMetaInfo;

const RegionMetaInfo OoTRegionsMetaInfo[(uint8_t) OoTRegions::Desert_Colossus + 1] =
{
	{ "None", (uint8_t)OoTRegions::None },
	{ "Dungeons", (uint8_t)OoTRegions::Dungeons },
	{ "Kokiri Forest", (uint8_t)OoTRegions::Kokiri_Forest },
	{ "Lost Woods", (uint8_t)OoTRegions::Lost_Woods },
	{ "Sacred Forest Meadow", (uint8_t)OoTRegions::Sacred_Forest_Meadow },
	{ "Hyrule Field", (uint8_t)OoTRegions::Hyrule },
	{ "Market", (uint8_t)OoTRegions::Market },
	{ "Hyrule / Ganon Castle", (uint8_t)OoTRegions::Castle },
	{ "Kakariko", (uint8_t)OoTRegions::Kakariko },
	{ "Graveyard", (uint8_t)OoTRegions::Graveyard },
	{ "Death Mountain Trial", (uint8_t)OoTRegions::Death_Mountain_Trial },
	{ "Death Mountain Crater", (uint8_t)OoTRegions::Death_Mountain_Crater },
	{ "Goron City", (uint8_t)OoTRegions::Goron_City },
	{ "Lon Lon's Ranch", (uint8_t)OoTRegions::Lon_Lon_Ranch },
	{ "Zora's River", (uint8_t)OoTRegions::Zora_River },
	{ "Zora's Domain", (uint8_t)OoTRegions::Zora_Domain },
	{ "Zora's Fountain", (uint8_t)OoTRegions::Zora_Fountain },
	{ "Lake Hylia", (uint8_t)OoTRegions::Lake_Hylia },
	{ "Gerudo Valley", (uint8_t)OoTRegions::Gerudo_Valley },
	{ "Gerudo Fortress", (uint8_t)OoTRegions::Gerudo_Fortress },
	{ "Haunted Wasteland", (uint8_t)OoTRegions::Haunted_Wasteland },
	{ "Desert Colossus", (uint8_t)OoTRegions::Desert_Colossus }
};

const RegionMetaInfo MMRegionsMetaInfo[(uint8_t)MMRegions::Moon + 1] =
{
	{ "None", (uint8_t)MMRegions::None },
	{ "Dungeons", (uint8_t)MMRegions::Dungeons },
	{ "Clock Town", (uint8_t)MMRegions::Clock_Town },
	{ "Termina Field", (uint8_t)MMRegions::Termina },
	{ "Milk Road", (uint8_t)MMRegions::Milk_Road },
	{ "Romani's Ranch", (uint8_t)MMRegions::Ranch },
	{ "Road to Southern Swamp", (uint8_t)MMRegions::Southern_Swamp_Road },
	{ "Southern Swamp", (uint8_t)MMRegions::Southern_Swamp },
	{ "Deku Palace", (uint8_t)MMRegions::Deku_Palace },
	{ "Woods of Mystery", (uint8_t)MMRegions::Woods_of_Mystery },
	{ "Woodfall", (uint8_t)MMRegions::Woodfall },
	{ "Path to Mountain Village", (uint8_t)MMRegions::Path_to_Mountain_Village },
	{ "Mountain Village", (uint8_t)MMRegions::Mountain_Village },
	{ "Twin Islands", (uint8_t)MMRegions::Twin_Islands },
	{ "Goron Village", (uint8_t)MMRegions::Goron_Village },
	{ "Path to Snowhead", (uint8_t)MMRegions::Path_to_Snowhead },
	{ "Snowhead", (uint8_t)MMRegions::Snowhead },
	{ "Great Bay Coast", (uint8_t)MMRegions::Great_Bay_Coast },
	{ "Pirate's Fortress", (uint8_t)MMRegions::Fortress },
	{ "Zora Cape", (uint8_t)MMRegions::Zora_Cape },
	{ "Zora Theater", (uint8_t)MMRegions::Zora_Hall },
	{ "Road to Ikana", (uint8_t)MMRegions::Road_to_Ikana },
	{ "Ikana Graveyard", (uint8_t)MMRegions::Graveyard },
	{ "Ikana Canyon", (uint8_t)MMRegions::Ikana_Canyon },
	{ "Stone Tower", (uint8_t)MMRegions::Stone_Tower },
	{ "Moon", (uint8_t)MMRegions::Moon }
};