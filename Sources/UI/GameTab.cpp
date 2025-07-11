#include "UI/GameTab.h"
#include "UI/OoTMMComboTracker.h"
#include "Multi/Game.h"
#include "UI/SceneRenderer.h"
#include "Combo/Regions.h"

#define CreateOverworldScene(SceneID, GameID) SceneInfo (SceneID, GameID, SceneType::Overworld)
#define CreateTempleScene(SceneID, GameID) SceneInfo (SceneID,  GameID, SceneType::Temple)
#define CreateHouseScene(SceneID, GameID) SceneInfo (SceneID, GameID, SceneType::House)
#define CreateGrottoScene(SceneID, GameID) SceneInfo (SceneID, GameID, SceneType::Grotto)

#define CreateInfoScenes(NumOfScenes, Prefix, ...) \
const size_t Prefix##Size = NumOfScenes;              \
SceneInfo Prefix##Scenes[Prefix##Size] = { __VA_ARGS__ };

CreateInfoScenes(OOT_NUM_SCENES, OoTOverworld,
    CreateOverworldScene(DEKU_TREE, OOT_GAME),
    CreateOverworldScene(DODONGO_CAVERN, OOT_GAME),
    CreateOverworldScene(INSIDE_JABU_JABU, OOT_GAME),
    CreateOverworldScene(TEMPLE_FOREST, OOT_GAME),
    CreateOverworldScene(TEMPLE_FIRE, OOT_GAME),
    CreateOverworldScene(TEMPLE_WATER, OOT_GAME),
    CreateOverworldScene(TEMPLE_SPIRIT, OOT_GAME),
    CreateOverworldScene(TEMPLE_SHADOW, OOT_GAME),
    CreateOverworldScene(BOTTOM_OF_THE_WELL, OOT_GAME),
    CreateOverworldScene(ICE_CAVERN, OOT_GAME),
    CreateOverworldScene(GANON_TOWER, OOT_GAME),
    CreateOverworldScene(GERUDO_TRAINING_GROUND, OOT_GAME),
    CreateOverworldScene(THIEVES_HIDEOUT, OOT_GAME),
    CreateOverworldScene(INSIDE_GANON_CASTLE, OOT_GAME),
    CreateOverworldScene(GANON_TOWER_COLLAPSING, OOT_GAME),
    CreateOverworldScene(INSIDE_GANON_CASTLE_COLLAPSING, OOT_GAME),
    CreateOverworldScene(OOT_TREASURE_SHOP, OOT_GAME),
    CreateOverworldScene(LAIR_GOHMA, OOT_GAME),
    CreateOverworldScene(LAIR_KING_DODONGO, OOT_GAME),
    CreateOverworldScene(LAIR_BARINADE, OOT_GAME),
    CreateOverworldScene(LAIR_PHANTOM_GANON, OOT_GAME),
    CreateOverworldScene(LAIR_VOLVAGIA, OOT_GAME),
    CreateOverworldScene(LAIR_MORPHA, OOT_GAME),
    CreateOverworldScene(LAIR_TWINROVA, OOT_GAME),
    CreateOverworldScene(LAIR_BONGO_BONGO, OOT_GAME),
    CreateOverworldScene(LAIR_GANONDORF, OOT_GAME),
    CreateOverworldScene(TOWER_COLLAPSE_EXTERIOR, OOT_GAME),
    CreateOverworldScene(MARKET_ENTRANCE_CHILD_DAY, OOT_GAME),
    CreateOverworldScene(MARKET_ENTRANCE_CHILD_NIGHT, OOT_GAME),
    CreateOverworldScene(MARKET_ENTRANCE_ADULT, OOT_GAME),
    CreateOverworldScene(BACK_ALLEY_DAY, OOT_GAME),
    CreateOverworldScene(BACK_ALLEY_NIGHT, OOT_GAME),
    CreateOverworldScene(MARKET_CHILD_DAY, OOT_GAME),
    CreateOverworldScene(MARKET_CHILD_NIGHT, OOT_GAME),
    CreateOverworldScene(MARKET_ADULT, OOT_GAME),
    CreateOverworldScene(TEMPLE_OF_TIME_EXTERIOR_CHILD_DAY, OOT_GAME),
    CreateOverworldScene(TEMPLE_OF_TIME_EXTERIOR_CHILD_NIGHT, OOT_GAME),
    CreateOverworldScene(TEMPLE_OF_TIME_EXTERIOR_ADULT, OOT_GAME),
    CreateOverworldScene(KOKIRI_KNOW_IT_ALL, OOT_GAME),
    CreateOverworldScene(KOKIRI_TWINS, OOT_GAME),
    CreateOverworldScene(KOKIRI_MIDO, OOT_GAME),
    CreateOverworldScene(KOKIRI_SARIA, OOT_GAME),
    CreateOverworldScene(CARPENTER_BOSS_HOUSE, OOT_GAME),
    CreateOverworldScene(BACK_ALLEY_HOUSE, OOT_GAME),
    CreateOverworldScene(BAZAAR, OOT_GAME),
    CreateOverworldScene(KOKIRI_SHOP, OOT_GAME),
    CreateOverworldScene(OOT_GORON_SHOP, OOT_GAME),
    CreateOverworldScene(ZORA_SHOP, OOT_GAME),
    CreateOverworldScene(KAKARIKO_POTION_SHOP, OOT_GAME),
    CreateOverworldScene(MARKET_POTION_SHOP, OOT_GAME),
    CreateOverworldScene(BOMBCHU_SHOP, OOT_GAME),
    CreateOverworldScene(HAPPY_MASK_SHOP, OOT_GAME),
    CreateOverworldScene(LINK_HOUSE, OOT_GAME),
    CreateOverworldScene(BACK_ALLEY_HOUSE2, OOT_GAME),
    CreateOverworldScene(STABLE, OOT_GAME),
    CreateOverworldScene(IMPA_HOUSE, OOT_GAME),
    CreateOverworldScene(OOT_LABORATORY, OOT_GAME),
    CreateOverworldScene(CARPENTER_TENT, OOT_GAME),
    CreateOverworldScene(GRAVEKEEPER_HUT, OOT_GAME),
    CreateOverworldScene(GREAT_FAIRY_FOUNTAIN_UPGRADES, OOT_GAME),
    CreateOverworldScene(OOT_FAIRY_FOUNTAIN, OOT_GAME),
    CreateOverworldScene(GREAT_FAIRY_FOUNTAIN_SPELLS, OOT_GAME),
    CreateOverworldScene(OOT_GROTTOS, OOT_GAME),
    CreateOverworldScene(TOMB_REDEAD, OOT_GAME),
    CreateOverworldScene(TOMB_FAIRY, OOT_GAME),
    CreateOverworldScene(TOMB_ROYAL, OOT_GAME),
    CreateOverworldScene(OOT_SHOOTING_GALLERY, OOT_GAME),
    CreateOverworldScene(TEMPLE_OF_TIME, OOT_GAME),
    CreateOverworldScene(CHAMBER_OF_THE_SAGES, OOT_GAME),
    CreateOverworldScene(CASTLE_MAZE_DAY, OOT_GAME),
    CreateOverworldScene(CASTLE_MAZE_NIGHT, OOT_GAME),
    CreateOverworldScene(OOT_CUTSCENE_MAP, OOT_GAME),
    CreateOverworldScene(TOMB_DAMPE_WINDMILL, OOT_GAME),
    CreateOverworldScene(FISHING_POND, OOT_GAME),
    CreateOverworldScene(CASTLE_COURTYARD, OOT_GAME),
    CreateOverworldScene(BOMBCHU_BOWLING_ALLEY, OOT_GAME),
    CreateOverworldScene(RANCH_HOUSE_SILO, OOT_GAME),
    CreateOverworldScene(GUARD_HOUSE, OOT_GAME),
    CreateOverworldScene(GRANNY_POTION_SHOP, OOT_GAME),
    CreateOverworldScene(GANON_BATTLE_ARENA, OOT_GAME),
    CreateOverworldScene(HOUSE_OF_SKULLTULA, OOT_GAME),
    CreateOverworldScene(HYRULE_FIELD, OOT_GAME),
    CreateOverworldScene(KAKARIKO_VILLAGE, OOT_GAME),
    CreateOverworldScene(GRAVEYARD, OOT_GAME),
    CreateOverworldScene(ZORA_RIVER, OOT_GAME),
    CreateOverworldScene(KOKIRI_FOREST, OOT_GAME),
    CreateOverworldScene(SACRED_FOREST_MEADOW, OOT_GAME),
    CreateOverworldScene(LAKE_HYLIA, OOT_GAME),
    CreateOverworldScene(ZORA_DOMAIN, OOT_GAME),
    CreateOverworldScene(ZORA_FOUNTAIN, OOT_GAME),
    CreateOverworldScene(GERUDO_VALLEY, OOT_GAME),
    CreateOverworldScene(OOT_LOST_WOODS, OOT_GAME),
    CreateOverworldScene(DESERT_COLOSSUS, OOT_GAME),
    CreateOverworldScene(GERUDO_FORTRESS, OOT_GAME),
    CreateOverworldScene(HAUNTED_WASTELAND, OOT_GAME),
    CreateOverworldScene(HYRULE_CASTLE, OOT_GAME),
    CreateOverworldScene(DEATH_MOUNTAIN_TRAIL, OOT_GAME),
    CreateOverworldScene(DEATH_MOUNTAIN_CRATER, OOT_GAME),
    CreateOverworldScene(GORON_CITY, OOT_GAME),
    CreateOverworldScene(LON_LON_RANCH, OOT_GAME),
    CreateOverworldScene(GANON_CASTLE_EXTERIOR, OOT_GAME),
    CreateOverworldScene(OOT_GROTTO_KOKIRI_FOREST_STORMS, OOT_GAME),
    CreateOverworldScene(OOT_GROTTO_LOST_WOODS_SCRUB_UPGRADE, OOT_GAME),
    CreateOverworldScene(OOT_GROTTO_LOST_WOODS_GENERIC, OOT_GAME),
    CreateOverworldScene(OOT_GROTTO_LOST_WOODS_THEATER, OOT_GAME),
    CreateOverworldScene(OOT_GROTTO_SACRED_MEADOW_WOLFOS, OOT_GAME),
    CreateOverworldScene(OOT_GROTTO_SACRED_MEADOW_STORMS, OOT_GAME),
    CreateOverworldScene(OOT_GROTTO_KAKARIKO_REDEAD, OOT_GAME),
    CreateOverworldScene(OOT_GROTTO_KAKARIKO_OPEN, OOT_GAME),
    CreateOverworldScene(OOT_GROTTO_DEATH_TRIAL_STORMS, OOT_GAME),
    CreateOverworldScene(OOT_GROTTO_DEATH_TRIAL_COW, OOT_GAME),
    CreateOverworldScene(OOT_GROTTO_GORON_CITY_SCRUBS, OOT_GAME),
    CreateOverworldScene(OOT_GROTTO_DEATH_CRATER_GENERIC, OOT_GAME),
    CreateOverworldScene(OOT_GROTTO_DEATH_CRATER_SCRUBS, OOT_GAME),
    CreateOverworldScene(OOT_GROTTO_ZORA_RIVER_STORMS, OOT_GAME),
    CreateOverworldScene(OOT_GROTTO_ZORA_RIVER_GENERIC, OOT_GAME),
    CreateOverworldScene(OOT_GROTTO_LAKE_HYLIA_SCRUBS, OOT_GAME),
    CreateOverworldScene(OOT_GROTTO_LON_LON_SCRUBS, OOT_GAME),
    CreateOverworldScene(OOT_GROTTO_HYRULE_SCRUBS, OOT_GAME),
    CreateOverworldScene(OOT_GROTTO_HYRULE_SE, OOT_GAME),
    CreateOverworldScene(OOT_GROTTO_HYRULE_OPEN, OOT_GAME),
    CreateOverworldScene(OOT_GROTTO_HYRULE_MARKET, OOT_GAME),
    CreateOverworldScene(OOT_GROTTO_HYRULE_TEKTITE, OOT_GAME),
    CreateOverworldScene(OOT_GROTTO_HYRULE_KAKARIKO, OOT_GAME),
    CreateOverworldScene(OOT_GROTTO_HYRULE_GERUDO, OOT_GAME),
    CreateOverworldScene(OOT_GROTTO_CASTLE_STORMS, OOT_GAME),
    CreateOverworldScene(OOT_GROTTO_VALLEY_STORMS, OOT_GAME),
    CreateOverworldScene(OOT_GROTTO_VALLEY_OCTOROK, OOT_GAME),
    CreateOverworldScene(OOT_GROTTO_DESERT_SCRUBS, OOT_GAME),
    CreateOverworldScene(OOT_FAIRY_SACRED_MEADOW, OOT_GAME),
    CreateOverworldScene(OOT_FAIRY_ZORA_RIVER, OOT_GAME),
    CreateOverworldScene(OOT_FAIRY_ZORA_DOMAIN, OOT_GAME),
    CreateOverworldScene(OOT_FAIRY_HYRULE, OOT_GAME),
    CreateOverworldScene(OOT_FAIRY_GERUDO_FORTRESS, OOT_GAME),
    CreateOverworldScene(OOT_GREAT_FAIRY_CASTLE, OOT_GAME),
    CreateOverworldScene(OOT_GREAT_FAIRY_FARORE, OOT_GAME),
    CreateOverworldScene(OOT_GREAT_FAIRY_NAYRU, OOT_GAME),
    CreateOverworldScene(OOT_GREAT_FAIRY_MAGIC, OOT_GAME),
    CreateOverworldScene(OOT_GREAT_FAIRY_MAGIC2, OOT_GAME),
    CreateOverworldScene(KAKARIKO_BAZAAR, OOT_GAME),
    CreateOverworldScene(MARKET_BAZAAR, OOT_GAME),
    CreateOverworldScene(KAKARIKO_SHOOTING, OOT_GAME),
    CreateOverworldScene(MARKET_SHOOTING, OOT_GAME),
    CreateOverworldScene(SILO, OOT_GAME),
    CreateOverworldScene(WINDMILL, OOT_GAME),
    CreateOverworldScene(INSIDE_EGGS, OOT_GAME),
    CreateOverworldScene(MARKET, OOT_GAME)
)



CreateInfoScenes(MM_NUM_SCENES, MMOverworld,
    CreateOverworldScene(SOUTHERN_SWAMP_CLEAR, MM_GAME),
    CreateOverworldScene(MM_FAIRY_SNOWHEAD, MM_GAME),
    CreateOverworldScene(MM_FAIRY_GREAT_BAY_COAST, MM_GAME),
    CreateOverworldScene(MM_FAIRY_WOODFALL, MM_GAME),
    CreateOverworldScene(MM_FAIRY_CLOCK_TOWN, MM_GAME),
    CreateOverworldScene(MM_FAIRY_IKANA, MM_GAME),
    CreateOverworldScene(PATH_SNOWHEAD_SPRING, MM_GAME),
    CreateOverworldScene(MM_GROTTOS, MM_GAME),
    CreateOverworldScene(MM_CUTSCENE_MAP, MM_GAME),
    CreateOverworldScene(PATH_MOUNTAIN_VILLAGE_SPRING, MM_GAME),
    CreateOverworldScene(POTION_SHOP, MM_GAME),
    CreateOverworldScene(LAIR_MAJORA, MM_GAME),
    CreateOverworldScene(BENEATH_THE_GRAVEYARD, MM_GAME),
    CreateOverworldScene(CURIOSITY_SHOP, MM_GAME),
    CreateOverworldScene(BENEATH_THE_GRAVEYARD_NIGHT1, MM_GAME),
    CreateOverworldScene(BENEATH_THE_GRAVEYARD_NIGHT2, MM_GAME),
    CreateOverworldScene(RANCH_HOUSE_BARN, MM_GAME),
    CreateOverworldScene(HONEY_DARLING, MM_GAME),
    CreateOverworldScene(MAYOR_HOUSE, MM_GAME),
    CreateOverworldScene(IKANA_CANYON, MM_GAME),
    CreateOverworldScene(PIRATE_FORTRESS_EXTERIOR, MM_GAME),
    CreateOverworldScene(MILK_BAR, MM_GAME),
    CreateOverworldScene(TEMPLE_STONE_TOWER, MM_GAME),
    CreateOverworldScene(MM_TREASURE_SHOP, MM_GAME),
    CreateOverworldScene(TEMPLE_STONE_TOWER_INVERTED, MM_GAME),
    CreateOverworldScene(CLOCK_TOWER_ROOFTOP, MM_GAME),
    CreateOverworldScene(OPENING, MM_GAME),
    CreateOverworldScene(TEMPLE_WOODFALL, MM_GAME),
    CreateOverworldScene(PATH_MOUNTAIN_VILLAGE, MM_GAME),
    CreateOverworldScene(CASTLE_IKANA, MM_GAME),
    CreateOverworldScene(DEKU_PLAYGROUND, MM_GAME),
    CreateOverworldScene(LAIR_ODOLWA, MM_GAME),
    CreateOverworldScene(MM_SHOOTING_GALLERY, MM_GAME),
    CreateOverworldScene(TEMPLE_SNOWHEAD, MM_GAME),
    CreateOverworldScene(MILK_ROAD, MM_GAME),
    CreateOverworldScene(PIRATE_FORTRESS_INTERIOR, MM_GAME),
    CreateOverworldScene(SHOOTING_GALLERY_SWAMP, MM_GAME),
    CreateOverworldScene(PINNACLE_ROCK, MM_GAME),
    CreateOverworldScene(MM_FAIRY_FOUNTAIN, MM_GAME),
    CreateOverworldScene(MM_SPIDER_HOUSE_SWAMP, MM_GAME),
    CreateOverworldScene(MM_SPIDER_HOUSE_OCEAN, MM_GAME),
    CreateOverworldScene(OBSERVATORY, MM_GAME),
    CreateOverworldScene(MOON_DEKU, MM_GAME),
    CreateOverworldScene(DEKU_PALACE, MM_GAME),
    CreateOverworldScene(BLACKSMITH, MM_GAME),
    CreateOverworldScene(TERMINA_FIELD, MM_GAME),
    CreateOverworldScene(POST_OFFICE, MM_GAME),
    CreateOverworldScene(MM_LABORATORY, MM_GAME),
    CreateOverworldScene(DAMPE_HOUSE, MM_GAME),
    CreateOverworldScene(INSIDE_CASTLE_IKANA, MM_GAME),
    CreateOverworldScene(GORON_SHRINE, MM_GAME),
    CreateOverworldScene(ZORA_HALL, MM_GAME),
    CreateOverworldScene(TRADING_POST, MM_GAME),
    CreateOverworldScene(ROMANI_RANCH, MM_GAME),
    CreateOverworldScene(LAIR_TWINMOLD, MM_GAME),
    CreateOverworldScene(GREAT_BAY_COAST, MM_GAME),
    CreateOverworldScene(ZORA_CAPE, MM_GAME),
    CreateOverworldScene(LOTTERY, MM_GAME),
    CreateOverworldScene(SNOWHEAD_SPRING, MM_GAME),
    CreateOverworldScene(PIRATE_FORTRESS_ENTRANCE, MM_GAME),
    CreateOverworldScene(FISHERMAN_HUT, MM_GAME),
    CreateOverworldScene(MM_GORON_SHOP, MM_GAME),
    CreateOverworldScene(DEKU_KING_CHAMBER, MM_GAME),
    CreateOverworldScene(MOON_GORON, MM_GAME),
    CreateOverworldScene(ROAD_SOUTHERN_SWAMP, MM_GAME),
    CreateOverworldScene(DOG_RACETRACK, MM_GAME),
    CreateOverworldScene(CUCCO_SHACK, MM_GAME),
    CreateOverworldScene(IKANA_GRAVEYARD, MM_GAME),
    CreateOverworldScene(LAIR_GOHT, MM_GAME),
    CreateOverworldScene(SOUTHERN_SWAMP, MM_GAME),
    CreateOverworldScene(WOODFALL, MM_GAME),
    CreateOverworldScene(MOON_ZORA, MM_GAME),
    CreateOverworldScene(GORON_VILLAGE_SPRING, MM_GAME),
    CreateOverworldScene(TEMPLE_GREAT_BAY, MM_GAME),
    CreateOverworldScene(WATERFALL_RAPIDS, MM_GAME),
    CreateOverworldScene(BENEATH_THE_WELL, MM_GAME),
    CreateOverworldScene(ZORA_HALL_ROOMS, MM_GAME),
    CreateOverworldScene(GORON_VILLAGE_WINTER, MM_GAME),
    CreateOverworldScene(GORON_GRAVEYARD, MM_GAME),
    CreateOverworldScene(SAKON_HIDEOUT, MM_GAME),
    CreateOverworldScene(MOUNTAIN_VILLAGE_WINTER, MM_GAME),
    CreateOverworldScene(GHOST_HUT, MM_GAME),
    CreateOverworldScene(DEKU_SHRINE, MM_GAME),
    CreateOverworldScene(ROAD_IKANA, MM_GAME),
    CreateOverworldScene(SWORDSMAN_SCHOOL, MM_GAME),
    CreateOverworldScene(MUSIC_BOX_HOUSE, MM_GAME),
    CreateOverworldScene(LAIR_IKANA, MM_GAME),
    CreateOverworldScene(TOURIST_INFORMATION, MM_GAME),
    CreateOverworldScene(STONE_TOWER, MM_GAME),
    CreateOverworldScene(STONE_TOWER_INVERTED, MM_GAME),
    CreateOverworldScene(MOUNTAIN_VILLAGE_SPRING, MM_GAME),
    CreateOverworldScene(PATH_SNOWHEAD, MM_GAME),
    CreateOverworldScene(SNOWHEAD, MM_GAME),
    CreateOverworldScene(TWIN_ISLANDS_WINTER, MM_GAME),
    CreateOverworldScene(TWIN_ISLANDS_SPRING, MM_GAME),
    CreateOverworldScene(LAIR_GYORG, MM_GAME),
    CreateOverworldScene(SECRET_SHRINE, MM_GAME),
    CreateOverworldScene(STOCK_POT_INN, MM_GAME),
    CreateOverworldScene(GREAT_BAY_CUTSCENE, MM_GAME),
    CreateOverworldScene(CLOCK_TOWER_INTERIOR, MM_GAME),
    CreateOverworldScene(WOODS_MYSTERY, MM_GAME),
    CreateOverworldScene(MM_LOST_WOODS, MM_GAME),
    CreateOverworldScene(MOON_LINK, MM_GAME),
    CreateOverworldScene(MOON, MM_GAME),
    CreateOverworldScene(BOMB_SHOP, MM_GAME),
    CreateOverworldScene(GIANT_CHAMBER, MM_GAME),
    CreateOverworldScene(GORMAN_TRACK, MM_GAME),
    CreateOverworldScene(GORON_RACETRACK, MM_GAME),
    CreateOverworldScene(CLOCK_TOWN_EAST, MM_GAME),
    CreateOverworldScene(CLOCK_TOWN_WEST, MM_GAME),
    CreateOverworldScene(CLOCK_TOWN_NORTH, MM_GAME),
    CreateOverworldScene(CLOCK_TOWN_SOUTH, MM_GAME),
    CreateOverworldScene(LAUNDRY_POOL, MM_GAME),
    CreateOverworldScene(EXTRA, MM_GAME),
    CreateOverworldScene(LONE_PEAK, MM_GAME),
    CreateOverworldScene(MM_ZORA_SHOP, MM_GAME),
    CreateOverworldScene(ZORA_EVAN_ROOM, MM_GAME),
    CreateOverworldScene(ROMANI_RANCH_BARN, MM_GAME),
    CreateOverworldScene(MM_GROTTO_TERMINA_DODONGO, MM_GAME),
    CreateOverworldScene(MM_GROTTO_TERMINA_OCEAN_GOSSIP, MM_GAME),
    CreateOverworldScene(MM_GROTTO_TERMINA_CANYON_GOSSIP, MM_GAME),
    CreateOverworldScene(MM_GROTTO_TERMINA_BIO_BABA, MM_GAME),
    CreateOverworldScene(MM_GROTTO_TERMINA_PEEHAT, MM_GAME),
    CreateOverworldScene(MM_GROTTO_TERMINA_SCRUB, MM_GAME),
    CreateOverworldScene(MM_GROTTO_TERMINA_TALL_GRASS, MM_GAME),
    CreateOverworldScene(MM_GROTTO_TERMINA_COW, MM_GAME),
    CreateOverworldScene(MM_GROTTO_TERMINA_PILLAR, MM_GAME),
    CreateOverworldScene(MM_GROTTO_GREAT_BAY_COAST_FISHERMAN, MM_GAME),
    CreateOverworldScene(MM_GROTTO_GREAT_BAY_COAST_COW, MM_GAME),
    CreateOverworldScene(MM_GROTTO_ZORA_CAPE_GENERIC, MM_GAME),
    CreateOverworldScene(MM_GROTTO_IKANA_GRAVEYARD_GENERIC, MM_GAME),
    CreateOverworldScene(MM_GROTTO_IKANA_VALLEY_OPEN, MM_GAME),
    CreateOverworldScene(MM_GROTTO_IKANA_ROAD_GENERIC, MM_GAME),
    CreateOverworldScene(MM_GROTTO_TWIN_ISLANDS_FROZEN, MM_GAME),
    CreateOverworldScene(MM_GROTTO_TWIN_ISLANDS_RAMP, MM_GAME),
    CreateOverworldScene(MM_GROTTO_PATH_TO_SNOWHEAD_GENERIC, MM_GAME),
    CreateOverworldScene(MM_GROTTO_MOUNTAIN_VILLAGE_GENERIC, MM_GAME),
    CreateOverworldScene(MM_GROTTO_SOUTHERN_SWAMP_ROAD_OPEN, MM_GAME),
    CreateOverworldScene(MM_GROTTO_SOUTHERN_SWAMP_OPEN, MM_GAME),
    CreateOverworldScene(MM_GROTTO_WOODS_OF_MYSTERY_OPEN, MM_GAME),
    CreateOverworldScene(MM_GROTTO_DEKU_PALACE_BEANS, MM_GAME),
    //CreateOverworldScene(MM_GROTTO_DEKU_PALACE_GENERIC, MM_GAME),
    CreateOverworldScene(MOUNTAIN_VILLAGE, MM_GAME),
    CreateOverworldScene(TWIN_ISLANDS, MM_GAME)
)


GameTab::GameTab(int GameID, QWidget* parent) : QWidget(parent)
{
	this->GameID = GameID;

    // Main layout
    this->MainLayout = new QHBoxLayout;

    this->LoadGameTab();

    this->setLayout(this->MainLayout);
}


GameTab::~GameTab()
{
    delete this->GameMaps;
    delete this->MainLayout;
}

#pragma region Objects related

void GameTab::ItemFound(ObjectInfo* Object, const ItemInfo* Item)
{
    this->GameMaps->ItemFound(Object, Item);
}


void GameTab::RefreshTabCountText()
{
    if (this->Owner != nullptr)
    {
        this->Owner->UpdateTabNameText(this->GameID);
    }
}

#pragma endregion

#pragma region Saving / Loading

void GameTab::RefreshGameTab()
{
    this->GameMaps->RefreshScenesObjectCounts();
}

void GameTab::LoadGameTab()
{
    if (this->GameMaps)
    {   // The maps already existed

        this->MainLayout->removeWidget(this->GameMaps);
        delete this->GameMaps;
    }

    if (this->GameID == OOT_GAME)
    {	// Ocarina of time

        this->TabName = "OoT";
        this->GameMaps = new MapTab(this, OOT_GAME, OoTOverworldScenes, OoTOverworldSize);
    }
    else
    {	// Majora's mask

        this->TabName = "MM";
        this->GameMaps = new MapTab(this, MM_GAME, MMOverworldScenes, MMOverworldSize);
    }

    this->MainLayout->addWidget(this->GameMaps);
}


void GameTab::SaveGameScenes(QString FilePath)
{
    QFile saveFile(FilePath);
    if (!saveFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        MultiLogger::LogMessage("Can't open file: %s\n", FilePath.toStdString().c_str());
        return;
    }
    
    SaveSceneObjects(&saveFile);
    saveFile.close();

    MultiLogger::LogMessage("File saved: %s\n", FilePath.toStdString().c_str());
}

#pragma endregion