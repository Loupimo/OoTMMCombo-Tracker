#include "UI/RoomRenderer.h"

#pragma once

#include "Multi/Multi.h"
#include "Combo/Scenes.h"
#include "Multi/Game.h"
#include <QHash>
#include "UI/SceneRenderer.h"

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
	this->SceneItem->UpdateObjectCounts(Count);
}


bool RoomItemTree::HasContext()
{
	return this->SceneItem->HasContext();
}


SceneRenderer* RoomItemTree::GetScene()
{
	return this->SceneItem->GetScene();
}