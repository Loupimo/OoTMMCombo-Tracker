#include "UI/SceneEntrance.h"

SceneEntranceMetaInf * GetSceneEntranceMetaInf(int Game, uint32_t SceneID)
{
	if (Game == OOT_GAME)
	{
		if (OoTSceneEntranceMeta.contains(SceneID))
		{
			return &OoTSceneEntranceMeta.at(SceneID);
		}
	}
	else if (Game == MM_GAME)
	{
		if (MMSceneEntranceMeta.contains(SceneID))
		{
			return &MMSceneEntranceMeta.at(SceneID);
		}
	}

	return NULL;
}


std::map<uint32_t, SceneEntranceMetaInf>* GetSceneEntranceMetaInfForGame(int Game)
{
	if (Game == OOT_GAME)
	{
		return &OoTSceneEntranceMeta;
	}
	else if (Game == MM_GAME)
	{
		return &MMSceneEntranceMeta;
	}

	return NULL;
}