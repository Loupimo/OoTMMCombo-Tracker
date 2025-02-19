#pragma once

#include "Multi/Multi.h"
#include "Combo/Scenes.h"
#include "Multi/Game.h"
#include <QHash>
#include "UI/SceneRenderer.h"

typedef struct RoomInfo
{
	uint32_t RoomID;
	const char* RoomName;
	const char* ImagePath;
} RoomInfo;

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


const QHash<int, std::vector<RoomInfo>> MMRooms(
	{

	});


const QHash<int, std::vector<RoomInfo>>* GetSceneRooms(SceneInfo* Scene)
{
	if (Scene->GameID == OOT_GAME)
	{
		return &OoTRooms;
	}
	else
	{
		return &MMRooms;
	}
}


class RoomItemTree : public SceneItemTree
{

public:
	RoomInfo Info;
	SceneItemTree* SceneItem;

	RoomItemTree(RoomInfo* RInfo, SceneItemTree* ParentSceneItem)
	{
		this->Info.ImagePath = RInfo->ImagePath;
		this->Info.RoomID = RInfo->RoomID;
		this->Info.RoomName = RInfo->RoomName;
		this->SceneItem = ParentSceneItem;
	}

	void RenderScene(QTreeWidget* ObjectsTreeWidget, bool Context, bool CreateNew) override
	{
		if (CreateNew)
		{
			this->SceneItem->ActiveRoom = &this->Info;
		}
		this->SceneItem->RenderScene(ObjectsTreeWidget, Context, CreateNew);
		/*if (CreateNew)
		{
			this->Renderer = new SceneRenderer(this->SceneItem->Scene, ObjectsTreeWidget, this);
		}
		this->Renderer->RenderScene(Context, this->Info);*/
	}

	void UnloadScene() override
	{
		this->SceneItem->UnloadScene();
	}

	void RefreshObjectCounts(int Count) override
	{
		this->SceneItem->RefreshObjectCounts(Count);
	}

	bool HasContext() override
	{
		return this->SceneItem->HasContext();
	}

	SceneRenderer* GetScene() override
	{
		return this->SceneItem->GetScene();
	}
};