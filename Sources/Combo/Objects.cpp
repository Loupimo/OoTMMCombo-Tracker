#include "Combo/Objects.h"
#include "Combo/Items.h"
#include "Combo/OvTypes.h"
#include "Combo/OoTObjectScene.h"
#include "Combo/MMObjectScene.h"
#include "Multi/Game.h"


#pragma region SceneObjects

#pragma region OoT

// Empty Scenes
CreateEmptyScene(GANON_TOWER_COLLAPSING)
CreateEmptyScene(INSIDE_GANON_CASTLE_COLLAPSING)
CreateEmptyScene(LAIR_GANONDORF)
CreateEmptyScene(TOWER_COLLAPSE_EXTERIOR)
CreateEmptyScene(MARKET_ENTRANCE_CHILD_DAY)
CreateEmptyScene(MARKET_ENTRANCE_CHILD_NIGHT)
CreateEmptyScene(MARKET_ENTRANCE_ADULT)
CreateEmptyScene(BACK_ALLEY_DAY)
CreateEmptyScene(BACK_ALLEY_NIGHT)
CreateEmptyScene(MARKET_ADULT)
CreateEmptyScene(TEMPLE_OF_TIME_EXTERIOR_CHILD_DAY)
CreateEmptyScene(TEMPLE_OF_TIME_EXTERIOR_CHILD_NIGHT)
CreateEmptyScene(TEMPLE_OF_TIME_EXTERIOR_ADULT)
CreateEmptyScene(CARPENTER_BOSS_HOUSE)
CreateEmptyScene(HAPPY_MASK_SHOP)
CreateEmptyScene(CARPENTER_TENT)
CreateEmptyScene(GRAVEKEEPER_HUT)
CreateEmptyScene(CHAMBER_OF_THE_SAGES)
CreateEmptyScene(CASTLE_MAZE_DAY)
CreateEmptyScene(CASTLE_MAZE_NIGHT)
CreateEmptyScene(OOT_CUTSCENE_MAP)
CreateEmptyScene(GANON_BATTLE_ARENA)

SceneObjects OoTSceneObjects[OOT_NUM_SCENES] =
{
	CreateSceneObjects(DEKU_TREE),
	CreateSceneObjects(DODONGO_CAVERN),
	CreateSceneObjects(INSIDE_JABU_JABU),
	CreateSceneObjects(TEMPLE_FOREST),
	CreateSceneObjects(TEMPLE_FIRE),
	CreateSceneObjects(TEMPLE_WATER),
	CreateSceneObjects(TEMPLE_SPIRIT),
	CreateSceneObjects(TEMPLE_SHADOW),
	CreateSceneObjects(BOTTOM_OF_THE_WELL),
	CreateSceneObjects(ICE_CAVERN),
	CreateSceneObjects(GANON_TOWER),
	CreateSceneObjects(GERUDO_TRAINING_GROUND),
	CreateSceneObjects(THIEVES_HIDEOUT),
	CreateSceneObjects(INSIDE_GANON_CASTLE),
	CreateSceneObjects(GANON_TOWER_COLLAPSING),
	CreateSceneObjects(INSIDE_GANON_CASTLE_COLLAPSING),
	CreateSceneObjects(OOT_TREASURE_SHOP),
	CreateSceneObjects(LAIR_GOHMA),
	CreateSceneObjects(LAIR_KING_DODONGO),
	CreateSceneObjects(LAIR_BARINADE),
	CreateSceneObjects(LAIR_PHANTOM_GANON),
	CreateSceneObjects(LAIR_VOLVAGIA),
	CreateSceneObjects(LAIR_MORPHA),
	CreateSceneObjects(LAIR_TWINROVA),
	CreateSceneObjects(LAIR_BONGO_BONGO),
	CreateSceneObjects(LAIR_GANONDORF),
	CreateSceneObjects(TOWER_COLLAPSE_EXTERIOR),
	CreateSceneObjects(MARKET_ENTRANCE_CHILD_DAY),
	CreateSceneObjects(MARKET_ENTRANCE_CHILD_NIGHT),
	CreateSceneObjects(MARKET_ENTRANCE_ADULT),
	CreateSceneObjects(BACK_ALLEY_DAY),
	CreateSceneObjects(BACK_ALLEY_NIGHT),
	CreateSceneObjects(MARKET_CHILD_DAY),
	CreateSceneObjects(MARKET_CHILD_NIGHT),
	CreateSceneObjects(MARKET_ADULT),
	CreateSceneObjects(TEMPLE_OF_TIME_EXTERIOR_CHILD_DAY),
	CreateSceneObjects(TEMPLE_OF_TIME_EXTERIOR_CHILD_NIGHT),
	CreateSceneObjects(TEMPLE_OF_TIME_EXTERIOR_ADULT),
	CreateSceneObjects(KOKIRI_KNOW_IT_ALL),
	CreateSceneObjects(KOKIRI_TWINS),
	CreateSceneObjects(KOKIRI_MIDO),
	CreateSceneObjects(KOKIRI_SARIA),
	CreateSceneObjects(CARPENTER_BOSS_HOUSE),
	CreateSceneObjects(BACK_ALLEY_HOUSE),
	CreateSceneObjects(BAZAAR),
	CreateSceneObjects(KOKIRI_SHOP),
	CreateSceneObjects(OOT_GORON_SHOP),
	CreateSceneObjects(ZORA_SHOP),
	CreateSceneObjects(KAKARIKO_POTION_SHOP),
	CreateSceneObjects(MARKET_POTION_SHOP),
	CreateSceneObjects(BOMBCHU_SHOP),
	CreateSceneObjects(HAPPY_MASK_SHOP),
	CreateSceneObjects(LINK_HOUSE),
	CreateSceneObjects(BACK_ALLEY_HOUSE2),
	CreateSceneObjects(STABLE),
	CreateSceneObjects(IMPA_HOUSE),
	CreateSceneObjects(OOT_LABORATORY),
	CreateSceneObjects(CARPENTER_TENT),
	CreateSceneObjects(GRAVEKEEPER_HUT),
	CreateSceneObjects(GREAT_FAIRY_FOUNTAIN_UPGRADES),
	CreateSceneObjects(OOT_FAIRY_FOUNTAIN),
	CreateSceneObjects(GREAT_FAIRY_FOUNTAIN_SPELLS),
	CreateSceneObjects(OOT_GROTTOS),
	CreateSceneObjects(TOMB_REDEAD),
	CreateSceneObjects(TOMB_FAIRY),
	CreateSceneObjects(TOMB_ROYAL),
	CreateSceneObjects(OOT_SHOOTING_GALLERY),
	CreateSceneObjects(TEMPLE_OF_TIME),
	CreateSceneObjects(CHAMBER_OF_THE_SAGES),
	CreateSceneObjects(CASTLE_MAZE_DAY),
	CreateSceneObjects(CASTLE_MAZE_NIGHT),
	CreateSceneObjects(OOT_CUTSCENE_MAP),
	CreateSceneObjects(TOMB_DAMPE_WINDMILL),
	CreateSceneObjects(FISHING_POND),
	CreateSceneObjects(CASTLE_COURTYARD),
	CreateSceneObjects(BOMBCHU_BOWLING_ALLEY),
	CreateSceneObjects(RANCH_HOUSE_SILO),
	CreateSceneObjects(GUARD_HOUSE),
	CreateSceneObjects(GRANNY_POTION_SHOP),
	CreateSceneObjects(GANON_BATTLE_ARENA),
	CreateSceneObjects(HOUSE_OF_SKULLTULA),
	CreateSceneObjects(HYRULE_FIELD),
	CreateSceneObjects(KAKARIKO_VILLAGE),
	CreateSceneObjects(GRAVEYARD),
	CreateSceneObjects(ZORA_RIVER),
	CreateSceneObjects(KOKIRI_FOREST),
	CreateSceneObjects(SACRED_FOREST_MEADOW),
	CreateSceneObjects(LAKE_HYLIA),
	CreateSceneObjects(ZORA_DOMAIN),
	CreateSceneObjects(ZORA_FOUNTAIN),
	CreateSceneObjects(GERUDO_VALLEY),
	CreateSceneObjects(OOT_LOST_WOODS),
	CreateSceneObjects(DESERT_COLOSSUS),
	CreateSceneObjects(GERUDO_FORTRESS),
	CreateSceneObjects(HAUNTED_WASTELAND),
	CreateSceneObjects(HYRULE_CASTLE),
	CreateSceneObjects(DEATH_MOUNTAIN_TRAIL),
	CreateSceneObjects(DEATH_MOUNTAIN_CRATER),
	CreateSceneObjects(GORON_CITY),
	CreateSceneObjects(LON_LON_RANCH),
	CreateSceneObjects(GANON_CASTLE_EXTERIOR),

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
	CreateSceneObjects(KAKARIKO_BAZAAR),
	CreateSceneObjects(MARKET_BAZAAR),
	CreateSceneObjects(KAKARIKO_SHOOTING),
	CreateSceneObjects(MARKET_SHOOTING),
	CreateSceneObjects(SILO),
	CreateSceneObjects(WINDMILL),

	// Spoiler Log
	CreateSceneObjects(INSIDE_EGGS),
	CreateSceneObjects(MARKET)
};

#pragma endregion

#pragma region MM

// Empty scene objects
CreateEmptyScene(FISHERMAN_HUT)
CreateEmptyScene(BENEATH_THE_GRAVEYARD_NIGHT1)
CreateEmptyScene(BENEATH_THE_GRAVEYARD_NIGHT2)
CreateEmptyScene(INSIDE_CASTLE_IKANA)
CreateEmptyScene(SOUTHERN_SWAMP_CLEAR)
CreateEmptyScene(MM_CUTSCENE_MAP)
CreateEmptyScene(OPENING)
CreateEmptyScene(GORON_VILLAGE_SPRING)
CreateEmptyScene(GREAT_BAY_CUTSCENE)
CreateEmptyScene(CLOCK_TOWER_INTERIOR)
CreateEmptyScene(MM_LOST_WOODS)
CreateEmptyScene(GIANT_CHAMBER)
CreateEmptyScene(PATH_SNOWHEAD_SPRING)
CreateEmptyScene(PATH_MOUNTAIN_VILLAGE_SPRING)
CreateEmptyScene(SNOWHEAD_SPRING)
CreateEmptyScene(ROMANI_RANCH_BARN)

SceneObjects MMSceneObjects[MM_NUM_SCENES] =
{
	CreateSceneObjects(SOUTHERN_SWAMP_CLEAR),
	CreateSceneObjects(MM_FAIRY_SNOWHEAD),
	CreateSceneObjects(MM_FAIRY_GREAT_BAY_COAST),
	CreateSceneObjects(MM_FAIRY_WOODFALL),
	CreateSceneObjects(MM_FAIRY_CLOCK_TOWN),
	CreateSceneObjects(MM_FAIRY_IKANA),
	CreateSceneObjects(PATH_SNOWHEAD_SPRING),
	CreateSceneObjects(MM_GROTTOS),
	CreateSceneObjects(MM_CUTSCENE_MAP),
	CreateSceneObjects(PATH_MOUNTAIN_VILLAGE_SPRING),
	CreateSceneObjects(POTION_SHOP),
	CreateSceneObjects(LAIR_MAJORA),
	CreateSceneObjects(BENEATH_THE_GRAVEYARD),
	CreateSceneObjects(CURIOSITY_SHOP),
	CreateSceneObjects(BENEATH_THE_GRAVEYARD_NIGHT1),
	CreateSceneObjects(BENEATH_THE_GRAVEYARD_NIGHT2),
	CreateSceneObjects(RANCH_HOUSE_BARN),
	CreateSceneObjects(HONEY_DARLING),
	CreateSceneObjects(MAYOR_HOUSE),
	CreateSceneObjects(IKANA_CANYON),
	CreateSceneObjects(PIRATE_FORTRESS_EXTERIOR),
	CreateSceneObjects(MILK_BAR),
	CreateSceneObjects(TEMPLE_STONE_TOWER),
	CreateSceneObjects(MM_TREASURE_SHOP),
	CreateSceneObjects(TEMPLE_STONE_TOWER_INVERTED),
	CreateSceneObjects(CLOCK_TOWER_ROOFTOP),
	CreateSceneObjects(OPENING),
	CreateSceneObjects(TEMPLE_WOODFALL),
	CreateSceneObjects(PATH_MOUNTAIN_VILLAGE),
	CreateSceneObjects(CASTLE_IKANA),
	CreateSceneObjects(DEKU_PLAYGROUND),
	CreateSceneObjects(LAIR_ODOLWA),
	CreateSceneObjects(MM_SHOOTING_GALLERY),
	CreateSceneObjects(TEMPLE_SNOWHEAD),
	CreateSceneObjects(MILK_ROAD),
	CreateSceneObjects(PIRATE_FORTRESS_INTERIOR),
	CreateSceneObjects(SHOOTING_GALLERY_SWAMP),
	CreateSceneObjects(PINNACLE_ROCK),
	CreateSceneObjects(MM_FAIRY_FOUNTAIN),
	CreateSceneObjects(MM_SPIDER_HOUSE_SWAMP),
	CreateSceneObjects(MM_SPIDER_HOUSE_OCEAN),
	CreateSceneObjects(OBSERVATORY),
	CreateSceneObjects(MOON_DEKU),
	CreateSceneObjects(DEKU_PALACE),
	CreateSceneObjects(BLACKSMITH),
	CreateSceneObjects(TERMINA_FIELD),
	CreateSceneObjects(POST_OFFICE),
	CreateSceneObjects(MM_LABORATORY),
	CreateSceneObjects(DAMPE_HOUSE),
	CreateSceneObjects(INSIDE_CASTLE_IKANA),
	CreateSceneObjects(GORON_SHRINE),
	CreateSceneObjects(ZORA_HALL),
	CreateSceneObjects(TRADING_POST),
	CreateSceneObjects(ROMANI_RANCH),
	CreateSceneObjects(LAIR_TWINMOLD),
	CreateSceneObjects(GREAT_BAY_COAST),
	CreateSceneObjects(ZORA_CAPE),
	CreateSceneObjects(LOTTERY),
	CreateSceneObjects(SNOWHEAD_SPRING),
	CreateSceneObjects(PIRATE_FORTRESS_ENTRANCE),
	CreateSceneObjects(FISHERMAN_HUT),
	CreateSceneObjects(MM_GORON_SHOP),
	CreateSceneObjects(DEKU_KING_CHAMBER),
	CreateSceneObjects(MOON_GORON),
	CreateSceneObjects(ROAD_SOUTHERN_SWAMP),
	CreateSceneObjects(DOG_RACETRACK),
	CreateSceneObjects(CUCCO_SHACK),
	CreateSceneObjects(IKANA_GRAVEYARD),
	CreateSceneObjects(LAIR_GOHT),
	CreateSceneObjects(SOUTHERN_SWAMP),
	CreateSceneObjects(WOODFALL),
	CreateSceneObjects(MOON_ZORA),
	CreateSceneObjects(GORON_VILLAGE_SPRING),
	CreateSceneObjects(TEMPLE_GREAT_BAY),
	CreateSceneObjects(WATERFALL_RAPIDS),
	CreateSceneObjects(BENEATH_THE_WELL),
	CreateSceneObjects(ZORA_HALL_ROOMS),
	CreateSceneObjects(GORON_VILLAGE_WINTER),
	CreateSceneObjects(GORON_GRAVEYARD),
	CreateSceneObjects(SAKON_HIDEOUT),
	CreateSceneObjects(MOUNTAIN_VILLAGE_WINTER),
	CreateSceneObjects(GHOST_HUT),
	CreateSceneObjects(DEKU_SHRINE),
	CreateSceneObjects(ROAD_IKANA),
	CreateSceneObjects(SWORDSMAN_SCHOOL),
	CreateSceneObjects(MUSIC_BOX_HOUSE),
	CreateSceneObjects(LAIR_IKANA),
	CreateSceneObjects(TOURIST_INFORMATION),
	CreateSceneObjects(STONE_TOWER),
	CreateSceneObjects(STONE_TOWER_INVERTED),
	CreateSceneObjects(MOUNTAIN_VILLAGE_SPRING),
	CreateSceneObjects(PATH_SNOWHEAD),
	CreateSceneObjects(SNOWHEAD),
	CreateSceneObjects(TWIN_ISLANDS_WINTER),
	CreateSceneObjects(TWIN_ISLANDS_SPRING),
	CreateSceneObjects(LAIR_GYORG),
	CreateSceneObjects(SECRET_SHRINE),
	CreateSceneObjects(STOCK_POT_INN),
	CreateSceneObjects(GREAT_BAY_CUTSCENE),
	CreateSceneObjects(CLOCK_TOWER_INTERIOR),
	CreateSceneObjects(WOODS_MYSTERY),
	CreateSceneObjects(MM_LOST_WOODS),
	CreateSceneObjects(MOON_LINK),
	CreateSceneObjects(MOON),
	CreateSceneObjects(BOMB_SHOP),
	CreateSceneObjects(GIANT_CHAMBER),
	CreateSceneObjects(GORMAN_TRACK),
	CreateSceneObjects(GORON_RACETRACK),
	CreateSceneObjects(CLOCK_TOWN_EAST),
	CreateSceneObjects(CLOCK_TOWN_WEST),
	CreateSceneObjects(CLOCK_TOWN_NORTH),
	CreateSceneObjects(CLOCK_TOWN_SOUTH),
	CreateSceneObjects(LAUNDRY_POOL),
	CreateSceneObjects(EXTRA),

	CreateSceneObjects(LONE_PEAK),
	CreateSceneObjects(MM_ZORA_SHOP),
	CreateSceneObjects(ZORA_EVAN_ROOM),
	CreateSceneObjects(ROMANI_RANCH_BARN),

	CreateSceneObjects(MM_GROTTO_TERMINA_DODONGO),
	CreateSceneObjects(MM_GROTTO_TERMINA_OCEAN_GOSSIP),
	CreateSceneObjects(MM_GROTTO_TERMINA_CANYON_GOSSIP),
	CreateSceneObjects(MM_GROTTO_TERMINA_BIO_BABA),
	CreateSceneObjects(MM_GROTTO_TERMINA_PEEHAT),
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

	// Spoiler Log
	CreateSceneObjects(MOUNTAIN_VILLAGE),
	CreateSceneObjects(TWIN_ISLANDS)
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