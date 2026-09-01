#pragma once

#include "Multi/Multi.h"
#include "Combo/Scenes.h"
#include "Multi/Game.h"
#include <QHash>
#include "UI/SceneRenderer.h"

/*
*   Contains all information to defines a scene room.
*/
typedef struct RoomInfo
{
	uint32_t RoomID;		// The room ID.
	const char* RoomName;	// The room Name.
	const char* ImagePath;	// The image path that corresponding to the room.
} RoomInfo;

// An hash array that contains all OoT rooms for each scene that has more than one.
const QHash<int, std::vector<RoomInfo>> OoTRooms =
{
	{
		OOT_DEKU_TREE,
		std::vector<RoomInfo>(
			{
				{ 0, "1F", "./Resources/OoT/Dungeons/Deku_Tree/DEK_1F.jpg" },
				{ 1, "2F", "./Resources/OoT/Dungeons/Deku_Tree/DEK_2F.jpg" },
				{ 2, "3F", "./Resources/OoT/Dungeons/Deku_Tree/DEK_3F.jpg" },
				{ 3, "B1", "./Resources/OoT/Dungeons/Deku_Tree/DEK_B1.jpg" },
				{ 4, "B2", "./Resources/OoT/Dungeons/Deku_Tree/DEK_B2.jpg" }
			}
		)
	},

    {
        OOT_KOKIRI_FOREST,
        std::vector<RoomInfo>(
            {
                { 0, "Kokiri Forest", "./Resources/OoT/Kokiri_Forest/Kokiri_Forest.jpg" },
                { 1, "Deku Tree", "./Resources/OoT/Kokiri_Forest/Deku_Tree.jpg" }
            }
        )
    },

	{
		OOT_DEATH_MOUNTAIN_TRAIL,
		std::vector<RoomInfo>(
			{
				{ 0, "Bottom", "./Resources/OoT/Death_Mountain_Trail/Bottom.jpg" },
				{ 1, "Middle", "./Resources/OoT/Death_Mountain_Trail/Middle.jpg" },
				{ 2, "Top", "./Resources/OoT/Death_Mountain_Trail/Top.jpg" }
			}
		)
	},

	{
		OOT_GANON_TOWER,
		std::vector<RoomInfo>(
			{
				{ 0, "Boss Key Room", "./Resources/OoT/Ganon/Boss_Key_Room.jpg" },
				{ 1, "Pots Room", "./Resources/OoT/Ganon/Pots_Room.jpg" }
			}
		)
	},

	{
		OOT_BOTTOM_OF_THE_WELL,
		std::vector<RoomInfo>(
			{
				{ 0, "B1", "./Resources/OoT/Dungeons/Well/WEL_B1.jpg" },
				{ 1, "B2", "./Resources/OoT/Dungeons/Well/WEL_B2.jpg" }
			}
		)
	},

	{
		OOT_DODONGO_CAVERN,
		std::vector<RoomInfo>(
			{
				{ 0, "1F", "./Resources/OoT/Dungeons/Cavern/CAV_1F.jpg" },
				{ 1, "2F", "./Resources/OoT/Dungeons/Cavern/CAV_2F.jpg" }
			}
		)
	},
	
	{
		OOT_INSIDE_JABU_JABU,
		std::vector<RoomInfo>(
			{
				{ 0, "1F", "./Resources/OoT/Dungeons/Jabu-Jabu/JAB_1F.jpg" },
				{ 1, "B1", "./Resources/OoT/Dungeons/Jabu-Jabu/JAB_B1.jpg" }
			}
		)
	},
	
	{
		OOT_TEMPLE_FOREST,
		std::vector<RoomInfo>(
			{
				{ 0, "1F", "./Resources/OoT/Dungeons/Forest/FOT_1F.jpg" },
				{ 1, "2F", "./Resources/OoT/Dungeons/Forest/FOT_2F.jpg" },
				{ 2, "B1", "./Resources/OoT/Dungeons/Forest/FOT_B1.jpg" },
				{ 3, "B2", "./Resources/OoT/Dungeons/Forest/FOT_B2.jpg" }
			}
		)
	},

	{
		OOT_TEMPLE_FIRE,
		std::vector<RoomInfo>(
			{
				{ 0, "1F", "./Resources/OoT/Dungeons/Fire/FIT_1F.jpg" },
				{ 1, "2F", "./Resources/OoT/Dungeons/Fire/FIT_2F.jpg" },
				{ 2, "3F", "./Resources/OoT/Dungeons/Fire/FIT_3F.jpg" },
				{ 3, "4F", "./Resources/OoT/Dungeons/Fire/FIT_4F.jpg" },
				{ 4, "5F", "./Resources/OoT/Dungeons/Fire/FIT_5F.jpg" }
			}
		)
	},

	{
		OOT_TEMPLE_WATER,
		std::vector<RoomInfo>(
			{
				{ 0, "1F", "./Resources/OoT/Dungeons/Water/WT_1F.jpg" },
				{ 1, "2F", "./Resources/OoT/Dungeons/Water/WT_2F.jpg" },
				{ 2, "3F", "./Resources/OoT/Dungeons/Water/WT_3F.jpg" },
				{ 3, "B1", "./Resources/OoT/Dungeons/Water/WT_B1.jpg" }
			}
		)
	},

	{
		OOT_TEMPLE_SHADOW,
		std::vector<RoomInfo>(
			{
				{ 0, "B1", "./Resources/OoT/Dungeons/Shadow/SHT_B1.jpg" },
				{ 1, "B2", "./Resources/OoT/Dungeons/Shadow/SHT_B2.jpg" },
				{ 2, "B3", "./Resources/OoT/Dungeons/Shadow/SHT_B3.jpg" },
				{ 3, "B4", "./Resources/OoT/Dungeons/Shadow/SHT_B4.jpg" },
				{ 4, "B5", "./Resources/OoT/Dungeons/Shadow/SHT_B5.jpg" }
			}
		)
	},

	{
		OOT_TEMPLE_SPIRIT,
		std::vector<RoomInfo>(
			{
				{ 0, "1F", "./Resources/OoT/Dungeons/Spirit/SPT_1F.jpg" },
				{ 1, "2F", "./Resources/OoT/Dungeons/Spirit/SPT_2F.jpg" },
				{ 2, "3F", "./Resources/OoT/Dungeons/Spirit/SPT_3F.jpg" },
				{ 3, "4F", "./Resources/OoT/Dungeons/Spirit/SPT_4F.jpg" }
			}
		)
	}
};

// An hash array that contains all MM rooms for each scene that has more than one.
const QHash<int, std::vector<RoomInfo>> MMRooms = 
{
	{
		MM_OBSERVATORY,
		std::vector<RoomInfo>(
			{
				{ 0, "Observatory", "./Resources/MM/Clock_Town/Observatory.jpg" },
				{ 1, "Passage", "./Resources/MM/Clock_Town/Passage.jpg" }
			}
		)
	},
	
	{
		MM_STOCK_POT_INN,
		std::vector<RoomInfo>(
			{
				{ 0, "Lobby", "./Resources/MM/Clock_Town/SPI_Lobby.jpg" },
				{ 1, "Back", "./Resources/MM/Clock_Town/SPI_Back.jpg" },
				{ 2, "Rooms", "./Resources/MM/Clock_Town/SPI_Rooms.jpg" }
			}
		)
	},
		
	{
		MM_CASTLE_IKANA,
		std::vector<RoomInfo>(
			{
				{ 0, "Outside", "./Resources/MM/Ikana_Canyon/Castle_Exterior.jpg" },
				{ 1, "Inside", "./Resources/MM/Ikana_Canyon/Castle_Inside.jpg" }
			}
		)
	},

	{
		MM_SPIDER_HOUSE_OCEAN,
		std::vector<RoomInfo>(
			{
				{ 0, "Lobby", "./Resources/MM/Great_Bay_Coast/HS_Lobby.jpg" },
				{ 1, "Main Room", "./Resources/MM/Great_Bay_Coast/HS_1.jpg" },
				{ 2, "Library Room", "./Resources/MM/Great_Bay_Coast/HS_2.jpg" },
				{ 3, "Masks Room", "./Resources/MM/Great_Bay_Coast/HS_3.jpg" },
				{ 4, "Storage Room", "./Resources/MM/Great_Bay_Coast/HS_4.jpg" }
			}
		)
	},

	{
		MM_SPIDER_HOUSE_SWAMP,
		std::vector<RoomInfo>(
			{
				{ 0, "Lobby", "./Resources/MM/Southern_Swamp/HS_Lobby.jpg" },
				{ 1, "Main Room", "./Resources/MM/Southern_Swamp/HS_1.jpg" },
				{ 2, "Gold Room", "./Resources/MM/Southern_Swamp/HS_2.jpg" },
				{ 3, "Monument Room", "./Resources/MM/Southern_Swamp/HS_3.jpg" },
				{ 4, "Pots Room", "./Resources/MM/Southern_Swamp/HS_4.jpg" },
				{ 5, "Tree Room", "./Resources/MM/Southern_Swamp/HS_5.jpg" }
			}
		)
	},

	{
		MM_TEMPLE_WOODFALL,
		std::vector<RoomInfo>(
			{
				{ 0, "Entrance", "./Resources/MM/Dungeons/Woodfall/Entrance.jpg" },
				{ 1, "Pre-Boss Room", "./Resources/MM/Dungeons/Woodfall/Pre_Boss.jpg" },
				{ 2, "Central Room", "./Resources/MM/Dungeons/Woodfall/Central.jpg" },
				{ 3, "Maze Room", "./Resources/MM/Dungeons/Woodfall/Maze.jpg" },
				{ 4, "Compass Room", "./Resources/MM/Dungeons/Woodfall/Compass.jpg" },
				{ 5, "Water Room", "./Resources/MM/Dungeons/Woodfall/Water.jpg" },
				{ 6, "Map Room", "./Resources/MM/Dungeons/Woodfall/Map.jpg" },
				{ 7, "Dinolfos Room", "./Resources/MM/Dungeons/Woodfall/Dinalfos.jpg" },
				{ 8, "Geeko Room", "./Resources/MM/Dungeons/Woodfall/Geeko.jpg" },
				{ 9, "Dark Room", "./Resources/MM/Dungeons/Woodfall/Dark.jpg" },
				{ 10, "Pits Room", "./Resources/MM/Dungeons/Woodfall/Pits.jpg" },
				{ 11, "Boss Room", "./Resources/MM/Dungeons/Woodfall/Boss.jpg" }
			}
		)
	},

	{
		MM_TEMPLE_SNOWHEAD,
		std::vector<RoomInfo>(
			{
				{ 0, "Entrance", "./Resources/MM/Dungeons/Snowhead/Entrance.jpg" },
				{ 1, "Central Room", "./Resources/MM/Dungeons/Snowhead/Central.jpg" },
				{ 2, "Bridge Room", "./Resources/MM/Dungeons/Snowhead/Bridge.jpg" },
				{ 3, "Compass Room", "./Resources/MM/Dungeons/Snowhead/Compass.jpg" },
				{ 4, "Blocks Room", "./Resources/MM/Dungeons/Snowhead/Blocks.jpg" },
				{ 5, "Pillars Room", "./Resources/MM/Dungeons/Snowhead/Pillars.jpg" },
				{ 6, "Fire Arrow Room", "./Resources/MM/Dungeons/Snowhead/Fire_Arrow.jpg" },
				{ 7, "Icicle Room", "./Resources/MM/Dungeons/Snowhead/Icicle.jpg" },
				{ 8, "Dual Switches Room", "./Resources/MM/Dungeons/Snowhead/Dual_Switches.jpg" },
				{ 9, "Map Room", "./Resources/MM/Dungeons/Snowhead/Map.jpg" },
				{ 10, "Snow Room", "./Resources/MM/Dungeons/Snowhead/Snow.jpg" },
				{ 11, "Dinolfos Room", "./Resources/MM/Dungeons/Snowhead/Dinalfos.jpg" },
				{ 12, "Wizzrobe Room", "./Resources/MM/Dungeons/Snowhead/Wizzrobe.jpg" },
				{ 13, "Boss Room", "./Resources/MM/Dungeons/Snowhead/Boss.jpg" }
			}
		)
	},

	{
		MM_TEMPLE_GREAT_BAY,
		std::vector<RoomInfo>(
			{
				{ 0, "Entrance", "./Resources/MM/Dungeons/Great_Bay/Entrance.jpg" },
				{ 1, "Water Wheel Room", "./Resources/MM/Dungeons/Great_Bay/Water_Wheel.jpg" },
				{ 2, "Central Room", "./Resources/MM/Dungeons/Great_Bay/Central.jpg" },
				{ 3, "Geeko Room", "./Resources/MM/Dungeons/Great_Bay/Geeko.jpg" },
				{ 4, "Map Room", "./Resources/MM/Dungeons/Great_Bay/Map.jpg" },
				{ 5, "Red Pipe Room", "./Resources/MM/Dungeons/Great_Bay/Red_Pipe.jpg" },
				{ 6, "Bio-Baba Room", "./Resources/MM/Dungeons/Great_Bay/Bio_baba.jpg" },
				{ 7, "Boss Key Room", "./Resources/MM/Dungeons/Great_Bay/Boss_Key.jpg" },
				{ 8, "Before Wart Room", "./Resources/MM/Dungeons/Great_Bay/Before_Wart.jpg" },
				{ 9, "Wart Room", "./Resources/MM/Dungeons/Great_Bay/Wart.jpg" },
				{ 10, "Green Pipe Water Wheel Room", "./Resources/MM/Dungeons/Great_Bay/Green_Pipe_2.jpg" },
				{ 11, "Moving Platform Room", "./Resources/MM/Dungeons/Great_Bay/Moving_Platform.jpg" },
				{ 12, "Pre-Boss Room", "./Resources/MM/Dungeons/Great_Bay/Pre_Boss.jpg" },
				{ 13, "Green Pipe Room", "./Resources/MM/Dungeons/Great_Bay/Green_Pipe.jpg" },
				{ 14, "Chuchu Before Wart Room", "./Resources/MM/Dungeons/Great_Bay/Chuchu.jpg" },
				{ 15, "Boss Room", "./Resources/MM/Dungeons/Great_Bay/Boss.jpg" }
			}
		)
	},

	{
		MM_TEMPLE_STONE_TOWER,
		std::vector<RoomInfo>(
			{
				{ 0, "Entrance", "./Resources/MM/Dungeons/Stone_Tower/Entrance.jpg" },
				{ 1, "Central Room", "./Resources/MM/Dungeons/Stone_Tower/Central.jpg" },
				{ 2, "Maze Room", "./Resources/MM/Dungeons/Stone_Tower/Maze.jpg" },
				{ 3, "Water Room", "./Resources/MM/Dungeons/Stone_Tower/Water.jpg" },
				{ 4, "Lava Room", "./Resources/MM/Dungeons/Stone_Tower/Lava.jpg" },
				{ 5, "Mirrors Room", "./Resources/MM/Dungeons/Stone_Tower/Mirrors.jpg" },
				{ 6, "Hiploop Bridge Room", "./Resources/MM/Dungeons/Stone_Tower/Hiploop.jpg" },
				{ 7, "Wind Room", "./Resources/MM/Dungeons/Stone_Tower/Wind.jpg" },
				{ 8, "Garo Master Room", "./Resources/MM/Dungeons/Stone_Tower/Garo.jpg" }
			}
		)
	},
	
	{
		MM_TEMPLE_STONE_TOWER_INVERTED,
		std::vector<RoomInfo>(
			{
				{ 0, "Entrance", "./Resources/MM/Dungeons/Stone_Tower_Inverted/Entrance.jpg" },
				{ 1, "Central Room", "./Resources/MM/Dungeons/Stone_Tower_Inverted/Central.jpg" },
				{ 2, "Maze Room", "./Resources/MM/Dungeons/Stone_Tower_Inverted/Maze.jpg" },
				{ 3, "Water Room", "./Resources/MM/Dungeons/Stone_Tower_Inverted/Water.jpg" },
				{ 4, "Lava Room", "./Resources/MM/Dungeons/Stone_Tower_Inverted/Lava.jpg" },
				{ 5, "Hiploop Bridge Room", "./Resources/MM/Dungeons/Stone_Tower_Inverted/Hiploop.jpg" },
				{ 6, "Gomess Room", "./Resources/MM/Dungeons/Stone_Tower_Inverted/Gomess.jpg" },
				{ 7, "Boss Room", "./Resources/MM/Dungeons/Stone_Tower_Inverted/Boss.jpg" }
			}
		)
	}
};


/*
*   Gets all rooms that belongs to the given scene.
*
*	@param Scene		The scene the rooms should belong to.
* 
*	@return All rooms that belongs to the given scene.
*/
const QHash<int, std::vector<RoomInfo>>* GetSceneRooms(SceneInfo* Scene);


/*
*   The class that handles the rooms information.
*/
class RoomItemTree : public SceneItemTree
{

public:

	RoomInfo Info;				// The room information.
	SceneItemTree* SceneItem;	// The parent item to attach this room to.

	/*
	*   Constructs the room based on the given information.
	*
	*   @param RInfo				The room information to use.
	*   @param ParentSceneItem		The scene this room belongs to.
	*/
	RoomItemTree(RoomInfo* RInfo, SceneItemTree* ParentSceneItem);

	/*
	*   Renders the scene (here room).
	*
	*   @param ObjectsTreeWidget   The object tree list to fill when this scene is active.
    *   @param Context             The context in which the scene should be rendered.
    *   @param CreateNew           Tells if we should create a new scene renderer or not.
    */
	void RenderScene(QTreeWidget* ObjectsTreeWidget, bool Context, bool CreateNew) override;

	/*
	*   Unload the scene associated to this item.
	*/
    void UnloadScene() override;

	/*
	*   Increase / decrease the number of found object by the given amount.
	*
	*   @param Count  The number of found object to add or remove.
	*/
	void UpdateObjectCounts(int Count) override;

	/*
	*   Tells if this scene has a context.
	*
	*   @return True means the scene has a context, false means no context.
	*/
	bool HasContext() override;

	/*
	*   Refresh the object count of the room and its scene.
	* 
	*	@return The number of collected objects.
	*/
	uint32_t RefreshRoomObjectsCount();
	
	/*
	*   Initialize the room counters
	*/
	void InitRoomCounters();

	/*
	*   Gets the scene renderer.
	*
	*   @return The scene renderer.
	*/
	SceneRenderer* GetScene() override;

	/*
	*   Gets the scene name.
	*
	*   @return The scene name.
	*/
	const char* GetSceneName() override;
};
