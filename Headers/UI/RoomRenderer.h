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
});

// An hash array that contains all MM rooms for each scene that has more than one.
const QHash<int, std::vector<RoomInfo>> MMRooms(
	{

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