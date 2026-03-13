#include "Combo/Objects.h"
#include "Combo/Items.h"
#include "Combo/OvTypes.h"
#include "Combo/OoTObjectScene.h"
#include "Combo/MMObjectScene.h"
#include "Multi/Game.h"


#pragma region SceneObjects

#pragma region OoT

// Empty Scenes
CreateEmptyScene(OOT_GANON_TOWER_COLLAPSING)
CreateEmptyScene(OOT_INSIDE_GANON_CASTLE_COLLAPSING)
CreateEmptyScene(OOT_LAIR_GANONDORF)
CreateEmptyScene(OOT_TOWER_COLLAPSE_EXTERIOR)
CreateEmptyScene(OOT_MARKET_ENTRANCE_CHILD_DAY)
CreateEmptyScene(OOT_MARKET_ENTRANCE_CHILD_NIGHT)
CreateEmptyScene(OOT_MARKET_ENTRANCE_ADULT)
CreateEmptyScene(OOT_BACK_ALLEY_DAY)
CreateEmptyScene(OOT_BACK_ALLEY_NIGHT)
CreateEmptyScene(OOT_MARKET_ADULT)
CreateEmptyScene(OOT_TEMPLE_OF_TIME_EXTERIOR_CHILD_DAY)
CreateEmptyScene(OOT_TEMPLE_OF_TIME_EXTERIOR_CHILD_NIGHT)
CreateEmptyScene(OOT_TEMPLE_OF_TIME_EXTERIOR_ADULT)
CreateEmptyScene(OOT_CARPENTER_BOSS_HOUSE)
CreateEmptyScene(OOT_HAPPY_MASK_SHOP)
CreateEmptyScene(OOT_CARPENTER_TENT)
CreateEmptyScene(OOT_GRAVEKEEPER_HUT)
CreateEmptyScene(OOT_CHAMBER_OF_THE_SAGES)
CreateEmptyScene(OOT_CASTLE_MAZE_DAY)
CreateEmptyScene(OOT_CASTLE_MAZE_NIGHT)
CreateEmptyScene(OOT_CUTSCENE_MAP)
CreateEmptyScene(OOT_GANON_BATTLE_ARENA)
CreateEmptyScene(OOT_MARKET_ENTRANCE)
CreateEmptyScene(OOT_TEMPLE_OF_TIME_ENTRYWAY)
CreateEmptyScene(OOT_DAMPE_HOUSE)

SceneObjects OoTSceneObjects[OOT_NUM_SCENES] =
{
	CreateSceneObjects(OOT_DEKU_TREE),
	CreateSceneObjects(OOT_DODONGO_CAVERN),
	CreateSceneObjects(OOT_INSIDE_JABU_JABU),
	CreateSceneObjects(OOT_TEMPLE_FOREST),
	CreateSceneObjects(OOT_TEMPLE_FIRE),
	CreateSceneObjects(OOT_TEMPLE_WATER),
	CreateSceneObjects(OOT_TEMPLE_SPIRIT),
	CreateSceneObjects(OOT_TEMPLE_SHADOW),
	CreateSceneObjects(OOT_BOTTOM_OF_THE_WELL),
	CreateSceneObjects(OOT_ICE_CAVERN),
	CreateSceneObjects(OOT_GANON_TOWER),
	CreateSceneObjects(OOT_GERUDO_TRAINING_GROUND),
	CreateSceneObjects(OOT_THIEVES_HIDEOUT),
	CreateSceneObjects(OOT_INSIDE_GANON_CASTLE),
	CreateSceneObjects(OOT_GANON_TOWER_COLLAPSING),
	CreateSceneObjects(OOT_INSIDE_GANON_CASTLE_COLLAPSING),
	CreateSceneObjects(OOT_TREASURE_SHOP),
	CreateSceneObjects(OOT_LAIR_GOHMA),
	CreateSceneObjects(OOT_LAIR_KING_DODONGO),
	CreateSceneObjects(OOT_LAIR_BARINADE),
	CreateSceneObjects(OOT_LAIR_PHANTOM_GANON),
	CreateSceneObjects(OOT_LAIR_VOLVAGIA),
	CreateSceneObjects(OOT_LAIR_MORPHA),
	CreateSceneObjects(OOT_LAIR_TWINROVA),
	CreateSceneObjects(OOT_LAIR_BONGO_BONGO),
	CreateSceneObjects(OOT_LAIR_GANONDORF),
	CreateSceneObjects(OOT_TOWER_COLLAPSE_EXTERIOR),
	CreateSceneObjects(OOT_MARKET_ENTRANCE_CHILD_DAY),
	CreateSceneObjects(OOT_MARKET_ENTRANCE_CHILD_NIGHT),
	CreateSceneObjects(OOT_MARKET_ENTRANCE_ADULT),
	CreateSceneObjects(OOT_BACK_ALLEY_DAY),
	CreateSceneObjects(OOT_BACK_ALLEY_NIGHT),
	CreateSceneObjects(OOT_MARKET_CHILD_DAY),
	CreateSceneObjects(OOT_MARKET_CHILD_NIGHT),
	CreateSceneObjects(OOT_MARKET_ADULT),
	CreateSceneObjects(OOT_TEMPLE_OF_TIME_EXTERIOR_CHILD_DAY),
	CreateSceneObjects(OOT_TEMPLE_OF_TIME_EXTERIOR_CHILD_NIGHT),
	CreateSceneObjects(OOT_TEMPLE_OF_TIME_EXTERIOR_ADULT),
	CreateSceneObjects(OOT_KOKIRI_KNOW_IT_ALL),
	CreateSceneObjects(OOT_KOKIRI_TWINS),
	CreateSceneObjects(OOT_KOKIRI_MIDO),
	CreateSceneObjects(OOT_KOKIRI_SARIA),
	CreateSceneObjects(OOT_CARPENTER_BOSS_HOUSE),
	CreateSceneObjects(OOT_BACK_ALLEY_HOUSE),
	CreateSceneObjects(OOT_BAZAAR),
	CreateSceneObjects(OOT_KOKIRI_SHOP),
	CreateSceneObjects(OOT_GORON_SHOP),
	CreateSceneObjects(OOT_ZORA_SHOP),
	CreateSceneObjects(OOT_KAKARIKO_POTION_SHOP),
	CreateSceneObjects(OOT_MARKET_POTION_SHOP),
	CreateSceneObjects(OOT_BOMBCHU_SHOP),
	CreateSceneObjects(OOT_HAPPY_MASK_SHOP),
	CreateSceneObjects(OOT_LINK_HOUSE),
	CreateSceneObjects(OOT_BACK_ALLEY_HOUSE2),
	CreateSceneObjects(OOT_STABLE),
	CreateSceneObjects(OOT_IMPA_HOUSE),
	CreateSceneObjects(OOT_LABORATORY),
	CreateSceneObjects(OOT_CARPENTER_TENT),
	CreateSceneObjects(OOT_GRAVEKEEPER_HUT),
	CreateSceneObjects(OOT_GREAT_FAIRY_FOUNTAIN_UPGRADES),
	CreateSceneObjects(OOT_FAIRY_FOUNTAIN),
	CreateSceneObjects(OOT_GREAT_FAIRY_FOUNTAIN_SPELLS),
	CreateSceneObjects(OOT_GROTTOS),
	CreateSceneObjects(OOT_TOMB_REDEAD),
	CreateSceneObjects(OOT_TOMB_FAIRY),
	CreateSceneObjects(OOT_TOMB_ROYAL),
	CreateSceneObjects(OOT_SHOOTING_GALLERY),
	CreateSceneObjects(OOT_TEMPLE_OF_TIME),
	CreateSceneObjects(OOT_CHAMBER_OF_THE_SAGES),
	CreateSceneObjects(OOT_CASTLE_MAZE_DAY),
	CreateSceneObjects(OOT_CASTLE_MAZE_NIGHT),
	CreateSceneObjects(OOT_CUTSCENE_MAP),
	CreateSceneObjects(OOT_TOMB_DAMPE_WINDMILL),
	CreateSceneObjects(OOT_FISHING_POND),
	CreateSceneObjects(OOT_CASTLE_COURTYARD),
	CreateSceneObjects(OOT_BOMBCHU_BOWLING_ALLEY),
	CreateSceneObjects(OOT_RANCH_HOUSE_SILO),
	CreateSceneObjects(OOT_GUARD_HOUSE),
	CreateSceneObjects(OOT_GRANNY_POTION_SHOP),
	CreateSceneObjects(OOT_GANON_BATTLE_ARENA),
	CreateSceneObjects(OOT_HOUSE_OF_SKULLTULA),
	CreateSceneObjects(OOT_HYRULE_FIELD),
	CreateSceneObjects(OOT_KAKARIKO_VILLAGE),
	CreateSceneObjects(OOT_GRAVEYARD),
	CreateSceneObjects(OOT_ZORA_RIVER),
	CreateSceneObjects(OOT_KOKIRI_FOREST),
	CreateSceneObjects(OOT_SACRED_FOREST_MEADOW),
	CreateSceneObjects(OOT_LAKE_HYLIA),
	CreateSceneObjects(OOT_ZORA_DOMAIN),
	CreateSceneObjects(OOT_ZORA_FOUNTAIN),
	CreateSceneObjects(OOT_GERUDO_VALLEY),
	CreateSceneObjects(OOT_LOST_WOODS),
	CreateSceneObjects(OOT_DESERT_COLOSSUS),
	CreateSceneObjects(OOT_GERUDO_FORTRESS),
	CreateSceneObjects(OOT_HAUNTED_WASTELAND),
	CreateSceneObjects(OOT_HYRULE_CASTLE),
	CreateSceneObjects(OOT_DEATH_MOUNTAIN_TRAIL),
	CreateSceneObjects(OOT_DEATH_MOUNTAIN_CRATER),
	CreateSceneObjects(OOT_GORON_CITY),
	CreateSceneObjects(OOT_LON_LON_RANCH),
	CreateSceneObjects(OOT_GANON_CASTLE_EXTERIOR),

	// OoT detailed grotto scene
	CreateSceneObjects(OOT_GROTTO_KOKIRI_FOREST_STORMS),
	CreateSceneObjects(OOT_GROTTO_LOST_WOODS_SCRUB_UPGRADE),
	CreateSceneObjects(OOT_GROTTO_LOST_WOODS_GENERIC),
	CreateSceneObjects(OOT_GROTTO_LOST_WOODS_THEATER),
	CreateSceneObjects(OOT_GROTTO_SACRED_MEADOW_WOLFOS),
	CreateSceneObjects(OOT_GROTTO_SACRED_MEADOW_STORMS),
	CreateSceneObjects(OOT_GROTTO_KAKARIKO_REDEAD),
	CreateSceneObjects(OOT_GROTTO_KAKARIKO_OPEN),
	CreateSceneObjects(OOT_GROTTO_DEATH_TRIAL_STORMS),
	CreateSceneObjects(OOT_GROTTO_DEATH_TRIAL_COW),
	CreateSceneObjects(OOT_GROTTO_GORON_CITY_SCRUBS),
	CreateSceneObjects(OOT_GROTTO_DEATH_CRATER_GENERIC),
	CreateSceneObjects(OOT_GROTTO_DEATH_CRATER_SCRUBS),
	CreateSceneObjects(OOT_GROTTO_ZORA_RIVER_STORMS),
	CreateSceneObjects(OOT_GROTTO_ZORA_RIVER_GENERIC),
	CreateSceneObjects(OOT_GROTTO_LAKE_HYLIA_SCRUBS),
	CreateSceneObjects(OOT_GROTTO_LON_LON_SCRUBS),
	CreateSceneObjects(OOT_GROTTO_HYRULE_SCRUBS),
	CreateSceneObjects(OOT_GROTTO_HYRULE_SE),
	CreateSceneObjects(OOT_GROTTO_HYRULE_OPEN),
	CreateSceneObjects(OOT_GROTTO_HYRULE_MARKET),
	CreateSceneObjects(OOT_GROTTO_HYRULE_TEKTITE),
	CreateSceneObjects(OOT_GROTTO_HYRULE_KAKARIKO),
	CreateSceneObjects(OOT_GROTTO_HYRULE_GERUDO),
	CreateSceneObjects(OOT_GROTTO_CASTLE_STORMS),
	CreateSceneObjects(OOT_GROTTO_VALLEY_STORMS),
	CreateSceneObjects(OOT_GROTTO_VALLEY_OCTOROK),
	CreateSceneObjects(OOT_GROTTO_DESERT_SCRUBS),

	// OoT detailed fairy scene
	CreateSceneObjects(OOT_FAIRY_SACRED_MEADOW),
	CreateSceneObjects(OOT_FAIRY_ZORA_RIVER),
	CreateSceneObjects(OOT_FAIRY_ZORA_DOMAIN),
	CreateSceneObjects(OOT_FAIRY_HYRULE),
	CreateSceneObjects(OOT_FAIRY_GERUDO_FORTRESS),

	// OoT detailed great fairy scene
	CreateSceneObjects(OOT_GREAT_FAIRY_CASTLE),
	CreateSceneObjects(OOT_GREAT_FAIRY_FARORE),
	CreateSceneObjects(OOT_GREAT_FAIRY_NAYRU),
	CreateSceneObjects(OOT_GREAT_FAIRY_MAGIC),
	CreateSceneObjects(OOT_GREAT_FAIRY_MAGIC2),

	// OoT detailed other scene
	CreateSceneObjects(OOT_KAKARIKO_BAZAAR),
	CreateSceneObjects(OOT_MARKET_BAZAAR),
	CreateSceneObjects(OOT_KAKARIKO_SHOOTING),
	CreateSceneObjects(OOT_MARKET_SHOOTING),
	CreateSceneObjects(OOT_SILO),
	CreateSceneObjects(OOT_WINDMILL),
	CreateSceneObjects(OOT_MARKET_ENTRANCE),
	CreateSceneObjects(OOT_TEMPLE_OF_TIME_ENTRYWAY),
	CreateSceneObjects(OOT_DAMPE_HOUSE),

	// Spoiler Log
	CreateSceneObjects(OOT_INSIDE_EGGS),
	CreateSceneObjects(OOT_MARKET)
};

#pragma endregion

#pragma region MM

// Empty scene objects
CreateEmptyScene(MM_FISHERMAN_HUT)
CreateEmptyScene(MM_BENEATH_THE_GRAVEYARD_NIGHT1)
CreateEmptyScene(MM_BENEATH_THE_GRAVEYARD_NIGHT2)
CreateEmptyScene(MM_INSIDE_CASTLE_IKANA)
CreateEmptyScene(MM_SOUTHERN_SWAMP_CLEAR)
CreateEmptyScene(MM_CUTSCENE_MAP)
CreateEmptyScene(MM_OPENING)
CreateEmptyScene(MM_GORON_VILLAGE_SPRING)
CreateEmptyScene(MM_GREAT_BAY_CUTSCENE)
CreateEmptyScene(MM_CLOCK_TOWER_INTERIOR)
CreateEmptyScene(MM_LOST_WOODS)
CreateEmptyScene(MM_GIANT_CHAMBER)
CreateEmptyScene(MM_PATH_SNOWHEAD_SPRING)
CreateEmptyScene(MM_PATH_MOUNTAIN_VILLAGE_SPRING)
CreateEmptyScene(MM_SNOWHEAD_SPRING)
CreateEmptyScene(MM_ZORA_JAPAS_ROOM)
CreateEmptyScene(MM_ZORA_TIJO_ROOM)
CreateEmptyScene(MM_ZORA_LULU_ROOM)
CreateEmptyScene(MM_ROMANI_RANCH_BARN)
CreateEmptyScene(MM_PIRATE_SEWERS)
CreateEmptyScene(MM_GROTTO_TERMINA_SWAMP_GOSSIP)
CreateEmptyScene(MM_GROTTO_TERMINA_MOUNTAIN_GOSSIP)
CreateEmptyScene(MM_GROTTO_DEKU_PALACE_CLIMB)

SceneObjects MMSceneObjects[MM_NUM_SCENES] =
{
	CreateSceneObjects(MM_SOUTHERN_SWAMP_CLEAR),
	CreateSceneObjects(MM_FAIRY_SNOWHEAD),
	CreateSceneObjects(MM_FAIRY_GREAT_BAY_COAST),
	CreateSceneObjects(MM_FAIRY_WOODFALL),
	CreateSceneObjects(MM_FAIRY_CLOCK_TOWN),
	CreateSceneObjects(MM_FAIRY_IKANA),
	CreateSceneObjects(MM_PATH_SNOWHEAD_SPRING),
	CreateSceneObjects(MM_GROTTOS),
	CreateSceneObjects(MM_CUTSCENE_MAP),
	CreateSceneObjects(MM_PATH_MOUNTAIN_VILLAGE_SPRING),
	CreateSceneObjects(MM_POTION_SHOP),
	CreateSceneObjects(MM_LAIR_MAJORA),
	CreateSceneObjects(MM_BENEATH_THE_GRAVEYARD),
	CreateSceneObjects(MM_CURIOSITY_SHOP),
	CreateSceneObjects(MM_BENEATH_THE_GRAVEYARD_NIGHT1),
	CreateSceneObjects(MM_BENEATH_THE_GRAVEYARD_NIGHT2),
	CreateSceneObjects(MM_RANCH_HOUSE_BARN),
	CreateSceneObjects(MM_HONEY_DARLING),
	CreateSceneObjects(MM_MAYOR_HOUSE),
	CreateSceneObjects(MM_IKANA_CANYON),
	CreateSceneObjects(MM_PIRATE_FORTRESS_EXTERIOR),
	CreateSceneObjects(MM_MILK_BAR),
	CreateSceneObjects(MM_TEMPLE_STONE_TOWER),
	CreateSceneObjects(MM_TREASURE_SHOP),
	CreateSceneObjects(MM_TEMPLE_STONE_TOWER_INVERTED),
	CreateSceneObjects(MM_CLOCK_TOWER_ROOFTOP),
	CreateSceneObjects(MM_OPENING),
	CreateSceneObjects(MM_TEMPLE_WOODFALL),
	CreateSceneObjects(MM_PATH_MOUNTAIN_VILLAGE),
	CreateSceneObjects(MM_CASTLE_IKANA),
	CreateSceneObjects(MM_DEKU_PLAYGROUND),
	CreateSceneObjects(MM_LAIR_ODOLWA),
	CreateSceneObjects(MM_SHOOTING_GALLERY),
	CreateSceneObjects(MM_TEMPLE_SNOWHEAD),
	CreateSceneObjects(MM_MILK_ROAD),
	CreateSceneObjects(MM_PIRATE_FORTRESS_INTERIOR),
	CreateSceneObjects(MM_SHOOTING_GALLERY_SWAMP),
	CreateSceneObjects(MM_PINNACLE_ROCK),
	CreateSceneObjects(MM_FAIRY_FOUNTAIN),
	CreateSceneObjects(MM_SPIDER_HOUSE_SWAMP),
	CreateSceneObjects(MM_SPIDER_HOUSE_OCEAN),
	CreateSceneObjects(MM_OBSERVATORY),
	CreateSceneObjects(MM_MOON_DEKU),
	CreateSceneObjects(MM_DEKU_PALACE),
	CreateSceneObjects(MM_BLACKSMITH),
	CreateSceneObjects(MM_TERMINA_FIELD),
	CreateSceneObjects(MM_POST_OFFICE),
	CreateSceneObjects(MM_LABORATORY),
	CreateSceneObjects(MM_DAMPE_HOUSE),
	CreateSceneObjects(MM_INSIDE_CASTLE_IKANA),
	CreateSceneObjects(MM_GORON_SHRINE),
	CreateSceneObjects(MM_ZORA_HALL),
	CreateSceneObjects(MM_TRADING_POST),
	CreateSceneObjects(MM_ROMANI_RANCH),
	CreateSceneObjects(MM_LAIR_TWINMOLD),
	CreateSceneObjects(MM_GREAT_BAY_COAST),
	CreateSceneObjects(MM_ZORA_CAPE),
	CreateSceneObjects(MM_LOTTERY),
	CreateSceneObjects(MM_SNOWHEAD_SPRING),
	CreateSceneObjects(MM_PIRATE_FORTRESS_ENTRANCE),
	CreateSceneObjects(MM_FISHERMAN_HUT),
	CreateSceneObjects(MM_GORON_SHOP),
	CreateSceneObjects(MM_DEKU_KING_CHAMBER),
	CreateSceneObjects(MM_MOON_GORON),
	CreateSceneObjects(MM_ROAD_SOUTHERN_SWAMP),
	CreateSceneObjects(MM_DOG_RACETRACK),
	CreateSceneObjects(MM_CUCCO_SHACK),
	CreateSceneObjects(MM_IKANA_GRAVEYARD),
	CreateSceneObjects(MM_LAIR_GOHT),
	CreateSceneObjects(MM_SOUTHERN_SWAMP),
	CreateSceneObjects(MM_WOODFALL),
	CreateSceneObjects(MM_MOON_ZORA),
	CreateSceneObjects(MM_GORON_VILLAGE_SPRING),
	CreateSceneObjects(MM_TEMPLE_GREAT_BAY),
	CreateSceneObjects(MM_WATERFALL_RAPIDS),
	CreateSceneObjects(MM_BENEATH_THE_WELL),
	CreateSceneObjects(MM_ZORA_HALL_ROOMS),
	CreateSceneObjects(MM_GORON_VILLAGE_WINTER),
	CreateSceneObjects(MM_GORON_GRAVEYARD),
	CreateSceneObjects(MM_SAKON_HIDEOUT),
	CreateSceneObjects(MM_MOUNTAIN_VILLAGE_WINTER),
	CreateSceneObjects(MM_GHOST_HUT),
	CreateSceneObjects(MM_DEKU_SHRINE),
	CreateSceneObjects(MM_ROAD_IKANA),
	CreateSceneObjects(MM_SWORDSMAN_SCHOOL),
	CreateSceneObjects(MM_MUSIC_BOX_HOUSE),
	CreateSceneObjects(MM_LAIR_IKANA),
	CreateSceneObjects(MM_TOURIST_INFORMATION),
	CreateSceneObjects(MM_STONE_TOWER),
	CreateSceneObjects(MM_STONE_TOWER_INVERTED),
	CreateSceneObjects(MM_MOUNTAIN_VILLAGE_SPRING),
	CreateSceneObjects(MM_PATH_SNOWHEAD),
	CreateSceneObjects(MM_SNOWHEAD),
	CreateSceneObjects(MM_TWIN_ISLANDS_WINTER),
	CreateSceneObjects(MM_TWIN_ISLANDS_SPRING),
	CreateSceneObjects(MM_LAIR_GYORG),
	CreateSceneObjects(MM_SECRET_SHRINE),
	CreateSceneObjects(MM_STOCK_POT_INN),
	CreateSceneObjects(MM_GREAT_BAY_CUTSCENE),
	CreateSceneObjects(MM_CLOCK_TOWER_INTERIOR),
	CreateSceneObjects(MM_WOODS_MYSTERY),
	CreateSceneObjects(MM_LOST_WOODS),
	CreateSceneObjects(MM_MOON_LINK),
	CreateSceneObjects(MM_MOON),
	CreateSceneObjects(MM_BOMB_SHOP),
	CreateSceneObjects(MM_GIANT_CHAMBER),
	CreateSceneObjects(MM_GORMAN_TRACK),
	CreateSceneObjects(MM_GORON_RACETRACK),
	CreateSceneObjects(MM_CLOCK_TOWN_EAST),
	CreateSceneObjects(MM_CLOCK_TOWN_WEST),
	CreateSceneObjects(MM_CLOCK_TOWN_NORTH),
	CreateSceneObjects(MM_CLOCK_TOWN_SOUTH),
	CreateSceneObjects(MM_LAUNDRY_POOL),
	CreateSceneObjects(MM_EXTRA),

	// MM detailed other
	CreateSceneObjects(MM_LONE_PEAK),
	CreateSceneObjects(MM_ZORA_SHOP),
	CreateSceneObjects(MM_ZORA_EVANS_ROOM),
	CreateSceneObjects(MM_ZORA_JAPAS_ROOM),
	CreateSceneObjects(MM_ZORA_TIJO_ROOM),
	CreateSceneObjects(MM_ZORA_LULU_ROOM),
	CreateSceneObjects(MM_ROMANI_RANCH_BARN),
	CreateSceneObjects(MM_PIRATE_SEWERS),

	// MM detailed grottos
	CreateSceneObjects(MM_GROTTO_TERMINA_DODONGO),
	CreateSceneObjects(MM_GROTTO_TERMINA_OCEAN_GOSSIP),
	CreateSceneObjects(MM_GROTTO_TERMINA_CANYON_GOSSIP),
	CreateSceneObjects(MM_GROTTO_TERMINA_BIO_BABA),
	CreateSceneObjects(MM_GROTTO_TERMINA_PEAHAT),
	CreateSceneObjects(MM_GROTTO_TERMINA_SCRUB),
	CreateSceneObjects(MM_GROTTO_TERMINA_TALL_GRASS),
	CreateSceneObjects(MM_GROTTO_TERMINA_COW),
	CreateSceneObjects(MM_GROTTO_TERMINA_PILLAR),
	CreateSceneObjects(MM_GROTTO_GREAT_BAY_COAST_FISHERMAN),
	CreateSceneObjects(MM_GROTTO_GREAT_BAY_COAST_COW),
	CreateSceneObjects(MM_GROTTO_ZORA_CAPE_GENERIC),
	CreateSceneObjects(MM_GROTTO_IKANA_GRAVEYARD_GENERIC),
	CreateSceneObjects(MM_GROTTO_IKANA_VALLEY_OPEN),
	CreateSceneObjects(MM_GROTTO_IKANA_ROAD_GENERIC),
	CreateSceneObjects(MM_GROTTO_TWIN_ISLANDS_FROZEN),
	CreateSceneObjects(MM_GROTTO_TWIN_ISLANDS_RAMP),
	CreateSceneObjects(MM_GROTTO_PATH_TO_SNOWHEAD_GENERIC),
	CreateSceneObjects(MM_GROTTO_MOUNTAIN_VILLAGE_GENERIC),
	CreateSceneObjects(MM_GROTTO_SOUTHERN_SWAMP_ROAD_OPEN),
	CreateSceneObjects(MM_GROTTO_SOUTHERN_SWAMP_OPEN),
	CreateSceneObjects(MM_GROTTO_WOODS_OF_MYSTERY_OPEN),
	CreateSceneObjects(MM_GROTTO_DEKU_PALACE_BEANS),
	CreateSceneObjects(MM_GROTTO_DEKU_PALACE_GENERIC),
	CreateSceneObjects(MM_GROTTO_DEKU_PALACE_CLIMB),
	

	// Spoiler Log
	CreateSceneObjects(MM_MOUNTAIN_VILLAGE),
	CreateSceneObjects(MM_TWIN_ISLANDS)
};

#pragma endregion

#pragma endregion

#pragma region ObjectInfo

void ObjectInfo::SaveObject(QFile* SaveFile)
{
	QByteArray tmp(sizeof(uint32_t), 0);

	// Save ID
	memcpy_s(tmp.data(), 4, &this->ObjectID, sizeof(this->ObjectID));
	SaveFile->write(tmp);

	// Save status
	memcpy_s(tmp.data(), 4, &this->Status, sizeof(this->Status));
	SaveFile->write(tmp);

	// Save item
	uint32_t itemID = 0;
	if (this->Item && this->Item->ItemID != -1)
	{
		itemID = this->Item->ItemID;
	}

	memcpy_s(tmp.data(), 4, &itemID, sizeof(itemID));
	SaveFile->write(tmp);
}


size_t ObjectInfo::LoadObject(QByteArray* Data, size_t Offset)
{
	// Load ID
	uint32_t objID = 0;
	memcpy_s(&objID, sizeof(objID), Data->data() + Offset, sizeof(objID));
	Offset += sizeof(objID);

	if (objID == this->ObjectID)
	{	// It is the correct object

		// Load status
		uint32_t state = 0;
		memcpy_s(&state, sizeof(state), Data->data() + Offset, sizeof(state));
		Offset += sizeof(state);
		this->Status = (ObjectState)state;

		// Load item
		uint32_t itemID = 0;
		memcpy_s(&itemID, sizeof(itemID), Data->data() + Offset, sizeof(itemID));
		Offset += sizeof(itemID);

		if (itemID != 0)
		{	// There is an item to load

			this->Item = FindItem(itemID);
		}
	}

	return Offset;
}

void ObjectInfo::ResetObject()
{
	this->Status = ObjectState::Hidden;
	this->Item = nullptr;
}


bool ObjectInfo::HasCorrectLayout(GameLayout ActiveLayout)
{
	return this->Layout == GameLayout::all || this->Layout == ActiveLayout;
}

#pragma endregion

#pragma region Object info getter

ObjectInfo* FindObject(ComboItem Item)
{
	SceneMetaInfo* arraySceneMeta = nullptr;
	ObjectInfo* arrayObjs = nullptr;
	size_t arraySize = 0;
	ObjectInfo* currObj = nullptr;

	if (Item.GameID == MM_GAME)
	{	// Majora's Mask

		arrayObjs = MMSceneObjects[Item.SceneID].Objects;
		arraySize = MMSceneObjects[Item.SceneID].NumOfObjs;
	}
	else
	{	// Ocarina of time

		arrayObjs = OoTSceneObjects[Item.SceneID].Objects;
		arraySize = OoTSceneObjects[Item.SceneID].NumOfObjs;
	}

	for (size_t i = 0; i < arraySize; i++)
	{	// Browse all objects

		currObj = &arrayObjs[i];

		if (currObj->Scene == Item.SceneID)
		{	// They have the same scene ID

			SceneMetaInfo* currSceneMeta = GetSceneMetaInfo(Item.SceneID, Item.GameID);

			if (currObj->HasCorrectLayout(currSceneMeta->ActiveLayout))
			{	// We need to check for game ID as there might be conflict between OoT and OoT_MQ and MM and MM_JP object ID

				if (Item.OvType > OV_FISH)
				{	// We can check for the object only if its type is above the fish one

					if (currObj->Type > ObjectType::fish)
					{	// We can check the object

						if (currObj->ObjectID == Item.ObjectID)
						{	// This is the correct object.

							return currObj;
						}
					}
				}
				else
				{	// The object should have the exact same type

					if (currObj->Type == Item.OvType)
					{
						if (currObj->ObjectID == Item.ObjectID)
						{	// This is the correct object

							return currObj;
						}
					}
				}
			}
		}
	}

	return currObj;
}


SceneObjects* GetGameSceneObjects(uint32_t GameID)
{
	if (GameID == OOT_GAME)
	{
		return OoTSceneObjects;
	}
	else
	{
		return MMSceneObjects;
	}
}

#pragma endregion

#pragma region Saving / Loading

void SaveSceneObjects(QFile* SaveFile)
{
	SaveSceneObjectsFor(SaveFile, OoTSceneObjects, OOT_NUM_SCENES);
	SaveSceneObjectsFor(SaveFile, MMSceneObjects, MM_NUM_SCENES);
}


void SaveSceneObjectsFor(QFile* SaveFile, SceneObjects* Array, size_t NumOfScenes)
{
	QByteArray ID(sizeof(uint32_t), 0);
	QByteArray numObj(sizeof(size_t), 0);

	for (size_t i = 0; i < NumOfScenes; i++)
	{	// Browse all scenes

		// Save scene ID
		memcpy_s(ID.data(), sizeof(Array[i].SceneID), &Array[i].SceneID, sizeof(Array[i].SceneID));
		SaveFile->write(ID);

		// Save number of objects
		memcpy_s(numObj.data(), sizeof(Array[i].NumOfObjs), &Array[i].NumOfObjs, sizeof(Array[i].NumOfObjs));
		SaveFile->write(numObj);

		for (size_t j = 0; j < Array[i].NumOfObjs; j++)
		{	// Save all objects

			Array[i].Objects[j].SaveObject(SaveFile);
		}
	}
}


void LoadSceneObjects(QByteArray * Data, size_t Offset)
{
	Offset = LoadSceneObjectsFor(Data, Offset, OoTSceneObjects, OOT_NUM_SCENES);
	Offset = LoadSceneObjectsFor(Data, Offset, MMSceneObjects, MM_NUM_SCENES);
}


size_t LoadSceneObjectsFor(QByteArray* Data, size_t Offset, SceneObjects * Array, size_t NumOfScenes)
{
	for (size_t i = 0; i < NumOfScenes; i++)
	{	// Browse all scenes

		// Load scene ID
		uint32_t sceneID = 0;
		memcpy_s(&sceneID, sizeof(sceneID), Data->data() + Offset, sizeof(sceneID));
		Offset += sizeof(sceneID);

		if (Array[i].SceneID == sceneID)
		{	// The scene is correct

			// Load number of objects
			size_t numObjs = 0;
			memcpy_s(&numObjs, sizeof(numObjs), Data->data() + Offset, sizeof(numObjs));
			Offset += sizeof(numObjs);

			if (numObjs == Array[i].NumOfObjs)
			{	// It has the same number of objects

				for (size_t j = 0; j < Array[i].NumOfObjs; j++)
				{	// Load all objects

					Offset = Array[i].Objects[j].LoadObject(Data, Offset);
				}
			}
		}
	}

	return Offset;
}


void ResetSceneObjects()
{
	ResetSceneObjectsFor(OoTSceneObjects, OOT_NUM_SCENES);
	ResetSceneObjectsFor(MMSceneObjects, MM_NUM_SCENES);
}


void ResetSceneObjectsFor(SceneObjects* Array, size_t NumOfScenes)
{
	for (size_t i = 0; i < NumOfScenes; i++)
	{	// Browse all scenes
		
		for (size_t j = 0; j < Array[i].NumOfObjs; j++)
		{	// Reset all objects

			Array[i].Objects[j].ResetObject();
		}
	}
}

#pragma endregion