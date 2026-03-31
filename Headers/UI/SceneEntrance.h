#pragma once

#include "Combo/OoTEntrances.h"
#include "Combo/MMEntrances.h"
#include <map>

typedef struct EntranceCost
{
	uint32_t EntranceID;				// The current entrance
	std::map<uint32_t, uint32_t> Costs;	// Key = accessible entrance, Value = cost to reach the entrance from current entrance
} EntranceCost;


typedef struct EntranceLink
{
	uint32_t InLink = UINT32_MAX;		// The entrance ID that leads to this scene from the current entrance 
	uint32_t OutLink = UINT32_MAX;		// The entrance ID where going when leaving from this entrance
} EntranceLink;


typedef struct SceneEntranceMetaInf
{
	uint32_t SceneID;								// The scene ID to render
	std::map<uint32_t, EntranceLink> EntranceIDs;	// The entrances to display on the scene with their in and out link
	const char* MapPath;							// The mini map image path to load

} SceneEntranceMetaInf;


const SceneEntranceMetaInf* GetSceneEntranceMetaInf(int Game, uint32_t SceneID);
const std::map<uint32_t, SceneEntranceMetaInf>& GetSceneEntranceMetaInfForGame(int Game);