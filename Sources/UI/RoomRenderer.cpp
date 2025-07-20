#include "UI/RoomRenderer.h"

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


RoomItemTree::RoomItemTree(RoomInfo* RInfo, SceneItemTree* ParentSceneItem)
{
	this->Info.ImagePath = RInfo->ImagePath;
	this->Info.RoomID = RInfo->RoomID;
	this->Info.RoomName = RInfo->RoomName;
	this->SceneItem = ParentSceneItem;
	this->FoundObjects = 0;
	this->TotalObjects = 0;
	for (size_t i = 0; i < this->SceneItem->Scene->Objects->NumOfObjs; i++)
	{
		if (this->SceneItem->Scene->Objects->Objects[i].Type != ObjectType::none && this->SceneItem->Scene->Objects->Objects[i].RenderScene == this->SceneItem->Scene->SceneID && this->SceneItem->Scene->Objects->Objects[i].RoomID == this->Info.RoomID)
		{
			this->TotalObjects++;
		}
	}
	this->RefreshItemName();
}


void RoomItemTree::RenderScene(QTreeWidget* ObjectsTreeWidget, bool Context, bool CreateNew)
{
	if (CreateNew)
	{
		this->SceneItem->ActiveRoom = &this->Info;
	}
	this->SceneItem->RenderScene(ObjectsTreeWidget, Context, CreateNew);
}


void RoomItemTree::UnloadScene()
{
	this->SceneItem->UnloadScene();
}


void RoomItemTree::UpdateObjectCounts(int Count)
{
	this->FoundObjects += Count;
	this->SceneItem->UpdateObjectCounts(Count);
	this->RefreshItemName();
}


uint32_t RoomItemTree::RefreshRoomObjectsCount()
{
	this->FoundObjects = 0;
	uint32_t tmpCount = 0;

	for (size_t i = 0; i < this->SceneItem->Scene->Objects->NumOfObjs; i++)
	{   // Browse each scene objects

		ObjectInfo* currObject = &this->SceneItem->Scene->Objects->Objects[i];
		if (currObject->Type != ObjectType::none && currObject->RenderScene == this->SceneItem->Scene->SceneID && currObject->RoomID == this->Info.RoomID && currObject->Status != ObjectState::Hidden)
		{   // Ignore the object if the render scene ID is different from this scene ID

			this->FoundObjects++;
		}
	}

	this->RefreshItemName();
	return this->FoundObjects;
}


bool RoomItemTree::HasContext()
{
	return this->SceneItem->HasContext();
}


SceneRenderer* RoomItemTree::GetScene()
{
	return this->SceneItem->GetScene();
}


const char* RoomItemTree::GetSceneName()
{
    return this->Info.RoomName;
}