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
	)

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