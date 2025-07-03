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
const QHash<int, std::vector<RoomInfo>> OoTRooms(
{
	QPair<int, std::vector<RoomInfo>>(DEKU_TREE,
		std::vector<RoomInfo>(
			{
				{ 0, "Lobby", "./Resources/OoT/Dungeons/Deku_Tree/Lobby.png" },
				{ 1, "Compass Room", "./Resources/OoT/Dungeons/Deku_Tree/Compass_Room.png" },
				{ 2, "Slingshot Room", "./Resources/OoT/Dungeons/Deku_Tree/Sling_Room.png" },
				{ 3, "Basement", "./Resources/OoT/Dungeons/Deku_Tree/Basement.png" },
				{ 4, "Back Room", "./Resources/OoT/Dungeons/Deku_Tree/Basement_Back_Room.png" },
				{ 5, "Eye Switch Room", "./Resources/OoT/Dungeons/Deku_Tree/Eye_Switch_Room.png" },
				{ 6, "Water Room", "./Resources/OoT/Dungeons/Deku_Tree/Water_Room.png" },
				{ 7, "Torch Room", "./Resources/OoT/Dungeons/Deku_Tree/Torch_Room.png" },
				{ 8, "Larva Room", "./Resources/OoT/Dungeons/Deku_Tree/Larva_Room.png" },
				{ 9, "Pre-Boss Room", "./Resources/OoT/Dungeons/Deku_Tree/Pre-Boss_Room.png" },
				{ 10, "Boss Room", "./Resources/OoT/Dungeons/Deku_Tree/Boss_Room.png" }
			}
		)
	),

	QPair<int, std::vector<RoomInfo>>(DEATH_MOUNTAIN_TRAIL,
		std::vector<RoomInfo>(
			{
				{ 0, "Bottom", "./Resources/OoT/Death_Mountain_Trial/Bottom.png" },
				{ 1, "Middle", "./Resources/OoT/Death_Mountain_Trial/Middle.png" },
				{ 2, "Top", "./Resources/OoT/Death_Mountain_Trial/Top.png" }
			}
		)
	),

	QPair<int, std::vector<RoomInfo>>(GANON_TOWER,
		std::vector<RoomInfo>(
			{
				{ 0, "Boss Key Room", "./Resources/OoT/Ganon/Boss_Key_Room.png" },
				{ 1, "Pots Room", "./Resources/OoT/Ganon/Pots_Room.png" }
			}
		)
	),

	QPair<int, std::vector<RoomInfo>>(INSIDE_GANON_CASTLE,
		std::vector<RoomInfo>(
			{
				{ 0, "Lobby", "./Resources/OoT/Ganon/Lobby.png" },
				{ 1, "Water Trial", "./Resources/OoT/Ganon/Water.png" },
				{ 2, "Forest Trial", "./Resources/OoT/Ganon/Forest.png" },
				{ 3, "Light Trial", "./Resources/OoT/Ganon/Light.png" },
				{ 4, "Shadow Trial", "./Resources/OoT/Ganon/Shadow.png" },
				{ 5, "Spirit Trial", "./Resources/OoT/Ganon/Spirit.png" },
				{ 6, "Fire Trial", "./Resources/OoT/Ganon/Fire.png" }
			}
		)
	),

	QPair<int, std::vector<RoomInfo>>(BOTTOM_OF_THE_WELL,
		std::vector<RoomInfo>(
			{
				{ 0, "Main", "./Resources/OoT/Dungeons/Well/Main.png" },
				{ 1, "Underground", "./Resources/OoT/Dungeons/Well/Underground.png" }
			}
		)
	),

	QPair<int, std::vector<RoomInfo>>(DODONGO_CAVERN,
		std::vector<RoomInfo>(
			{
				{ 0, "Lobby", "./Resources/OoT/Dungeons/Cavern/Lobby.png" },
				{ 1, "Right Corridor", "./Resources/OoT/Dungeons/Cavern/Right_Corridor.png" },
				{ 2, "Bombable Stairs", "./Resources/OoT/Dungeons/Cavern/Stairs.png" },
				{ 3, "Lizalfos", "./Resources/OoT/Dungeons/Cavern/Lizalfos.png" },
				{ 4, "Blue Room", "./Resources/OoT/Dungeons/Cavern/Blue_Room.png" },
				{ 5, "Bomb Bag Room", "./Resources/OoT/Dungeons/Cavern/Bomb_Bag_Room.png" },
				{ 6, "Pre-Boss", "./Resources/OoT/Dungeons/Cavern/Pre-Boss.png" },
				{ 7, "Side Mini-Boss", "./Resources/OoT/Dungeons/Cavern/Side_Mini-Boss.png" },
				{ 8, "Boss Room", "./Resources/OoT/Dungeons/Cavern/Boss_Room.png" }
			}
		)
	),
	
	QPair<int, std::vector<RoomInfo>>(INSIDE_JABU_JABU,
		std::vector<RoomInfo>(
			{
				{ 0, "First Room", "./Resources/OoT/Dungeons/Jabu-Jabu/First_Room.png" },
				{ 1, "Main Corridor", "./Resources/OoT/Dungeons/Jabu-Jabu/Main_Corridor.png" },
				{ 2, "Holes Room", "./Resources/OoT/Dungeons/Jabu-Jabu/Holes_Room.png" },
				{ 3, "Water Room", "./Resources/OoT/Dungeons/Jabu-Jabu/Water_Room.png" },
				{ 4, "Mini-Boss", "./Resources/OoT/Dungeons/Jabu-Jabu/Mini-Boss.png" },
				{ 5, "Muscle Block Room", "./Resources/OoT/Dungeons/Jabu-Jabu/Muscle_Block.png" },
				{ 6, "Pre Boss Room", "./Resources/OoT/Dungeons/Jabu-Jabu/Pre-Boss.png" },
				{ 7, "Boss Room", "./Resources/OoT/Dungeons/Jabu-Jabu/Boss_Room.png" }
			}
		)
	),

	QPair<int, std::vector<RoomInfo>>(ICE_CAVERN,
		std::vector<RoomInfo>(
			{
				{ 0, "First Room", "./Resources/OoT/Dungeons/Ice/First_Room.png" },
				{ 1, "Central Room", "./Resources/OoT/Dungeons/Ice/Central_Room.png" },
				{ 2, "Right Room", "./Resources/OoT/Dungeons/Ice/Right_Room.png" },
				{ 3, "Back Room", "./Resources/OoT/Dungeons/Ice/Back_Room.png" },
				{ 4, "Pre Mini-Boss", "./Resources/OoT/Dungeons/Ice/Pre-Mini-Boss.png" },
				{ 5, "Mini-Boss", "./Resources/OoT/Dungeons/Ice/Mini-Boss.png" }
			}
		)
	),
	
	QPair<int, std::vector<RoomInfo>>(TEMPLE_FOREST,
		std::vector<RoomInfo>(
			{
				{ 0, "Entrance", "./Resources/OoT/Dungeons/Forest/Entrance.png" },
				{ 1, "Lobby", "./Resources/OoT/Dungeons/Forest/Lobby.png" },
				{ 2, "Garden West", "./Resources/OoT/Dungeons/Forest/Garden_West.png" },
				{ 3, "Garden East", "./Resources/OoT/Dungeons/Forest/Garden_East.png" },
				{ 4, "Well", "./Resources/OoT/Dungeons/Forest/Underwater.png" },
				{ 5, "Mini-Boss", "./Resources/OoT/Dungeons/Forest/Mini-Boss.png" },
				{ 6, "Maze Room", "./Resources/OoT/Dungeons/Forest/Blocks_Room.png" },
				{ 7, "Boss Key Room", "./Resources/OoT/Dungeons/Forest/Boss_Key.png" },
				{ 8, "Map Room", "./Resources/OoT/Dungeons/Forest/Map.png" },
				{ 9, "Red Poe", "./Resources/OoT/Dungeons/Forest/Red_Poe.png" },
				{ 10, "Blue Poe", "./Resources/OoT/Dungeons/Forest/Blue_Poe.png" },
				{ 11, "Green Poe", "./Resources/OoT/Dungeons/Forest/Green_Poe.png" },
				{ 12, "Spinning Room", "./Resources/OoT/Dungeons/Forest/Spinning_Room.png" },
				{ 13, "Check Room", "./Resources/OoT/Dungeons/Forest/Check_Room.png" },
				{ 14, "Floormaster Room", "./Resources/OoT/Dungeons/Forest/Wallmaster.png" },
				{ 15, "Pre-Boss", "./Resources/OoT/Dungeons/Forest/Pre-Boss.png" },
				{ 16, "Boss Room", "./Resources/OoT/Dungeons/Forest/Boss.png" }
			}
		)
	),

	QPair<int, std::vector<RoomInfo>>(TEMPLE_FIRE,
		std::vector<RoomInfo>(
			{
				{ 0, "Lava Room", "./Resources/OoT/Dungeons/Fire/Lava_Room.png" },
				{ 1, "Above Lava Room", "./Resources/OoT/Dungeons/Fire/Above_Lava.png" },
				{ 2, "Lava Side Room", "./Resources/OoT/Dungeons/Fire/Lava_Side.png" },
				{ 3, "Boulder Maze", "./Resources/OoT/Dungeons/Fire/Boulder_Maze.png" },
				{ 4, "Fire Column", "./Resources/OoT/Dungeons/Fire/Fire_Column.png" },
				{ 5, "Fire Wall", "./Resources/OoT/Dungeons/Fire/Fire_Wall.png" },
				{ 6, "Fire Maze", "./Resources/OoT/Dungeons/Fire/Fire_Maze.png" },
				{ 7, "Hammer", "./Resources/OoT/Dungeons/Fire/Hammer.png" },
				{ 8, "Boss Key Room", "./Resources/OoT/Dungeons/Fire/Boss_Key.png" },
				{ 9, "Darunia Room", "./Resources/OoT/Dungeons/Fire/Darunia.png" },
				{ 10, "Boss Room", "./Resources/OoT/Dungeons/Fire/Boss.png" }
			}
		)
	),

	QPair<int, std::vector<RoomInfo>>(TEMPLE_WATER,
		std::vector<RoomInfo>(
			{
				{ 0, "Lobby", "./Resources/OoT/Dungeons/Water/Lobby.png" },
				{ 1, "Waterfalls Room", "./Resources/OoT/Dungeons/Water/Waterfalls.png" },
				{ 2, "Ruto Room", "./Resources/OoT/Dungeons/Water/Ruto.png" },
				{ 3, "Boulder Room", "./Resources/OoT/Dungeons/Water/Boulder.png" },
				{ 4, "Block Room", "./Resources/OoT/Dungeons/Water/Block.png" },
				{ 5, "Dragon Room", "./Resources/OoT/Dungeons/Water/Dragon.png" },
				{ 6, "Pre-Mini-Boss Room", "./Resources/OoT/Dungeons/Water/Pre-Mini-Boss.png" },
				{ 7, "Compass Room", "./Resources/OoT/Dungeons/Water/Compass.png" },
				{ 8, "Central Room", "./Resources/OoT/Dungeons/Water/Central_Room.png" },
				{ 9, "Boss Room", "./Resources/OoT/Dungeons/Water/Boss.png" }
			}
		)
	),

	QPair<int, std::vector<RoomInfo>>(TEMPLE_SHADOW,
		std::vector<RoomInfo>(
			{
				{ 0, "Lobby", "./Resources/OoT/Dungeons/Shadow/Lobby.png" },
				{ 1, "Scythe Room", "./Resources/OoT/Dungeons/Shadow/Scythe.png" },
				{ 2, "Stalfos Room", "./Resources/OoT/Dungeons/Shadow/Stalfos.png" },
				{ 3, "Invisible Scythe Room", "./Resources/OoT/Dungeons/Shadow/Invisible_Scythe.png" },
				{ 4, "Falling Spikes Room", "./Resources/OoT/Dungeons/Shadow/Falling_Spikes.png" },
				{ 5, "Boat Room", "./Resources/OoT/Dungeons/Shadow/Boat.png" },
				{ 6, "Maze Room", "./Resources/OoT/Dungeons/Shadow/Maze.png" },
				{ 7, "Pre-Boss Room", "./Resources/OoT/Dungeons/Shadow/Pre-Boss.png" },
				{ 8, "Boss Room", "./Resources/OoT/Dungeons/Shadow/Boss.png" }
			}
		)
	),

	QPair<int, std::vector<RoomInfo>>(GERUDO_TRAINING_GROUND,
		std::vector<RoomInfo>(
			{
				{ 0, "Entrance", "./Resources/OoT/Dungeons/GTG/Entrance.png" },
				{ 1, "Stalfos Room", "./Resources/OoT/Dungeons/GTG/Stalfos.png" },
				{ 2, "Boulders Room", "./Resources/OoT/Dungeons/GTG/Boulders.png" },
				{ 3, "Wolfos Room", "./Resources/OoT/Dungeons/GTG/Wolfos.png" },
				{ 4, "Like Like Room", "./Resources/OoT/Dungeons/GTG/Like_Like.png" },
				{ 5, "Statue Room", "./Resources/OoT/Dungeons/GTG/Statue.png" },
				{ 6, "Flamming Room", "./Resources/OoT/Dungeons/GTG/Flamming.png" },
				{ 7, "Lava Room", "./Resources/OoT/Dungeons/GTG/Lava.png" },
				{ 8, "Water Room", "./Resources/OoT/Dungeons/GTG/Water.png" },
				{ 9, "Dinalfos Room", "./Resources/OoT/Dungeons/GTG/Dinalfos.png" },
				{ 10, "Maze", "./Resources/OoT/Dungeons/GTG/Maze.png" }
			}
		)
	),
	
	QPair<int, std::vector<RoomInfo>>(TEMPLE_SPIRIT,
		std::vector<RoomInfo>(
			{
				{ 0, "Entrance", "./Resources/OoT/Dungeons/Spirit/Entrance.png" },
				{ 1, "Child Lobby", "./Resources/OoT/Dungeons/Spirit/Lobby_Child.png" },
				{ 2, "Child Climb", "./Resources/OoT/Dungeons/Spirit/Climb_Child.png" },
				{ 3, "Sun Block", "./Resources/OoT/Dungeons/Spirit/Sun_Block.png" },
				{ 4, "Adult Lobby", "./Resources/OoT/Dungeons/Spirit/Lobby_Adult.png" },
				{ 5, "Adult Climb", "./Resources/OoT/Dungeons/Spirit/Climb_Adult.png" },
				{ 6, "Main Room", "./Resources/OoT/Dungeons/Spirit/Main.png" },
				{ 7, "Mirror Shield", "./Resources/OoT/Dungeons/Spirit/Mirror_Shield.png" },
				{ 8, "Boss Key Room", "./Resources/OoT/Dungeons/Spirit/Boss_Key.png" },
				{ 9, "Elevator", "./Resources/OoT/Dungeons/Spirit/Elevator.png" },
				{ 10, "Boss Room", "./Resources/OoT/Dungeons/Spirit/Boss.png" }
			}
		)
	)
});

// An hash array that contains all MM rooms for each scene that has more than one.
const QHash<int, std::vector<RoomInfo>> MMRooms(
{
	QPair<int, std::vector<RoomInfo>>(OBSERVATORY,
		std::vector<RoomInfo>(
			{
				{ 0, "Observatory", "./Resources/MM/Clock_Town/Observatory.png" },
				{ 1, "Passage", "./Resources/MM/Clock_Town/Passage.png" }
			}
		)
	),
	
	QPair<int, std::vector<RoomInfo>>(STOCK_POT_INN,
		std::vector<RoomInfo>(
			{
				{ 0, "Lobby", "./Resources/MM/Clock_Town/SPI_Lobby.png" },
				{ 1, "Back", "./Resources/MM/Clock_Town/SPI_Back.png" },
				{ 2, "Rooms", "./Resources/MM/Clock_Town/SPI_Rooms.png" }
			}
		)
	),
		
	QPair<int, std::vector<RoomInfo>>(CASTLE_IKANA,
		std::vector<RoomInfo>(
			{
				{ 0, "Outside", "./Resources/MM/Ikana_Canyon/Castle_Exterior.png" },
				{ 1, "Inside", "./Resources/MM/Ikana_Canyon/Castle_Inside.png" }
			}
		)
	),

	QPair<int, std::vector<RoomInfo>>(MM_SPIDER_HOUSE_OCEAN,
		std::vector<RoomInfo>(
			{
				{ 0, "Lobby", "./Resources/MM/Great_Bay_Coast/HS_Lobby.png" },
				{ 1, "Main Room", "./Resources/MM/Great_Bay_Coast/HS_1.png" },
				{ 2, "Library Room", "./Resources/MM/Great_Bay_Coast/HS_2.png" },
				{ 3, "Masks Room", "./Resources/MM/Great_Bay_Coast/HS_3.png" },
				{ 4, "Storage Room", "./Resources/MM/Great_Bay_Coast/HS_4.png" }
			}
		)
	),

	QPair<int, std::vector<RoomInfo>>(MM_SPIDER_HOUSE_SWAMP,
		std::vector<RoomInfo>(
			{
				{ 0, "Lobby", "./Resources/MM/Southern_Swamp/HS_Lobby.png" },
				{ 1, "Main Room", "./Resources/MM/Southern_Swamp/HS_1.png" },
				{ 2, "Gold Room", "./Resources/MM/Southern_Swamp/HS_2.png" },
				{ 3, "Monument Room", "./Resources/MM/Southern_Swamp/HS_3.png" },
				{ 4, "Pots Room", "./Resources/MM/Southern_Swamp/HS_4.png" },
				{ 5, "Tree Room", "./Resources/MM/Southern_Swamp/HS_5.png" }
			}
		)
	),

	QPair<int, std::vector<RoomInfo>>(TEMPLE_WOODFALL,
		std::vector<RoomInfo>(
			{
				{ 0, "Entrance", "./Resources/MM/Dungeons/Woodfall/Entrance.png" },
				{ 1, "Pre-Boss Room", "./Resources/MM/Dungeons/Woodfall/Pre_Boss.png" },
				{ 2, "Central Room", "./Resources/MM/Dungeons/Woodfall/Central.png" },
				{ 3, "Maze Room", "./Resources/MM/Dungeons/Woodfall/Maze.png" },
				{ 4, "Compass Room", "./Resources/MM/Dungeons/Woodfall/Compass.png" },
				{ 5, "Water Room", "./Resources/MM/Dungeons/Woodfall/Water.png" },
				{ 6, "Map Room", "./Resources/MM/Dungeons/Woodfall/Map.png" },
				{ 7, "Dinalfos Room", "./Resources/MM/Dungeons/Woodfall/Dinalfos.png" },
				{ 8, "Geeko Room", "./Resources/MM/Dungeons/Woodfall/Geeko.png" },
				{ 9, "Dark Room", "./Resources/MM/Dungeons/Woodfall/Dark.png" },
				{ 10, "Pits Room", "./Resources/MM/Dungeons/Woodfall/Pits.png" },
				{ 11, "Boss Room", "./Resources/MM/Dungeons/Woodfall/Boss.png" }
			}
		)
	),

	QPair<int, std::vector<RoomInfo>>(TEMPLE_SNOWHEAD,
		std::vector<RoomInfo>(
			{
				{ 0, "Entrance", "./Resources/MM/Dungeons/Snowhead/Entrance.png" },
				{ 1, "Central Room", "./Resources/MM/Dungeons/Snowhead/Central.png" },
				{ 2, "Bridge Room", "./Resources/MM/Dungeons/Snowhead/Bridge.png" },
				{ 3, "Compass Room", "./Resources/MM/Dungeons/Snowhead/Compass.png" },
				{ 4, "Blocks Room", "./Resources/MM/Dungeons/Snowhead/Blocks.png" },
				{ 5, "Pillars Room", "./Resources/MM/Dungeons/Snowhead/Pillars.png" },
				{ 6, "Fire Arrow Room", "./Resources/MM/Dungeons/Snowhead/Fire_Arrow.png" },
				{ 7, "Icicle Room", "./Resources/MM/Dungeons/Snowhead/Icicle.png" },
				{ 8, "Dual Switches Room", "./Resources/MM/Dungeons/Snowhead/Dual_Switches.png" },
				{ 9, "Map Room", "./Resources/MM/Dungeons/Snowhead/Map.png" },
				{ 10, "Snow Room", "./Resources/MM/Dungeons/Snowhead/Snow.png" },
				{ 11, "Dinalfos Room", "./Resources/MM/Dungeons/Snowhead/Dinalfos.png" },
				{ 12, "Wizzrobe Room", "./Resources/MM/Dungeons/Snowhead/Wizzrobe.png" },
				{ 13, "Boss Room", "./Resources/MM/Dungeons/Snowhead/Boss.png" }
			}
		)
	),

	QPair<int, std::vector<RoomInfo>>(TEMPLE_GREAT_BAY,
		std::vector<RoomInfo>(
			{
				{ 0, "Entrance", "./Resources/MM/Dungeons/Great_Bay/Entrance.png" },
				{ 1, "Water Wheel Room", "./Resources/MM/Dungeons/Great_Bay/Water_Wheel.png" },
				{ 2, "Central Room", "./Resources/MM/Dungeons/Great_Bay/Central.png" },
				{ 3, "Geeko Room", "./Resources/MM/Dungeons/Great_Bay/Geeko.png" },
				{ 4, "Map Room", "./Resources/MM/Dungeons/Great_Bay/Map.png" },
				{ 5, "Red Pipe Room", "./Resources/MM/Dungeons/Great_Bay/Red_Pipe.png" },
				{ 6, "Bio-Baba Room", "./Resources/MM/Dungeons/Great_Bay/Bio_baba.png" },
				{ 7, "Boss Key Room", "./Resources/MM/Dungeons/Great_Bay/Boss_Key.png" },
				{ 8, "Before Wart Room", "./Resources/MM/Dungeons/Great_Bay/Before_Wart.png" },
				{ 9, "Wart Room", "./Resources/MM/Dungeons/Great_Bay/Wart.png" },
				{ 10, "Green Pipe Water Wheel Room", "./Resources/MM/Dungeons/Great_Bay/Green_Pipe_2.png" },
				{ 11, "Moving Platform Room", "./Resources/MM/Dungeons/Great_Bay/Moving_Platform.png" },
				{ 12, "Pre-Boss Room", "./Resources/MM/Dungeons/Great_Bay/Pre_Boss.png" },
				{ 13, "Green Pipe Room", "./Resources/MM/Dungeons/Great_Bay/Green_Pipe.png" },
				{ 14, "Chuchu Before Wart Room", "./Resources/MM/Dungeons/Great_Bay/Chuchu.png" },
				{ 15, "Boss Room", "./Resources/MM/Dungeons/Great_Bay/Boss.png" }
			}
		)
	),

	QPair<int, std::vector<RoomInfo>>(TEMPLE_STONE_TOWER,
		std::vector<RoomInfo>(
			{
				{ 0, "Entrance", "./Resources/MM/Dungeons/Stone_Tower/Entrance.png" },
				{ 1, "Central Room", "./Resources/MM/Dungeons/Stone_Tower/Central.png" },
				{ 2, "Maze Room", "./Resources/MM/Dungeons/Stone_Tower/Maze.png" },
				{ 3, "Water Room", "./Resources/MM/Dungeons/Stone_Tower/Water.png" },
				{ 4, "Lava Room", "./Resources/MM/Dungeons/Stone_Tower/Lava.png" },
				{ 5, "Mirrors Room", "./Resources/MM/Dungeons/Stone_Tower/Mirrors.png" },
				{ 6, "Hiploop Bridge Room", "./Resources/MM/Dungeons/Stone_Tower/Hiploop.png" },
				{ 7, "Wind Room", "./Resources/MM/Dungeons/Stone_Tower/Wind.png" },
				{ 8, "Garo Master Room", "./Resources/MM/Dungeons/Stone_Tower/Garo.png" }
			}
		)
	),
	
	QPair<int, std::vector<RoomInfo>>(TEMPLE_STONE_TOWER_INVERTED,
		std::vector<RoomInfo>(
			{
				{ 0, "Entrance", "./Resources/MM/Dungeons/Stone_Tower_Inverted/Entrance.png" },
				{ 1, "Central Room", "./Resources/MM/Dungeons/Stone_Tower_Inverted/Central.png" },
				{ 2, "Maze Room", "./Resources/MM/Dungeons/Stone_Tower_Inverted/Maze.png" },
				{ 3, "Water Room", "./Resources/MM/Dungeons/Stone_Tower_Inverted/Water.png" },
				{ 4, "Lava Room", "./Resources/MM/Dungeons/Stone_Tower_Inverted/Lava.png" },
				{ 5, "Hiploop Bridge Room", "./Resources/MM/Dungeons/Stone_Tower_Inverted/Hiploop.png" },
				{ 6, "Gomess Room", "./Resources/MM/Dungeons/Stone_Tower_Inverted/Gomess.png" },
				{ 7, "Boss Room", "./Resources/MM/Dungeons/Stone_Tower_Inverted/Boss.png" }
			}
		)
	)
});


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

	RoomInfo Info;				// The room inromation.
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
	*   Gets the scene renderer.
	*
	*   @return The scene renderer.
	*/
	SceneRenderer* GetScene() override;
};