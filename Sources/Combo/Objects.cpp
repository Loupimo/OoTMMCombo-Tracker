#include "Combo/Objects.h"
#include "Combo/Items.h"
#include "Combo/OvTypes.h"
#include "Combo/OoTObjectScene.h"
#include "Combo/MMObjectScene.h"
#include "Multi/Game.h"




SceneObjects* GetGameSceneObjects(uint32_t GameID)
{
	if (GameID == OOT_GAME)
	{
		return OoTSceneObjects;
	}
	else
	{
		return MMSceneObjects;
	}
}


ObjectInfo* FindObject(ComboItem Item) 
{
	ObjectInfo* arrayObjs = nullptr;
	size_t arraySize = 0;
	ObjectInfo* currObj = nullptr;// { MAXUINT32, MAXUINT32, "Unknown Object", ObjectType::none, { 0, 0 }, MAXUINT32 };

	if (Item.GameID == MM_GAME)
	{	// Majora's Mask

		arrayObjs = MMSceneObjects[Item.SceneID].Objects;
		arraySize = MMSceneObjects[Item.SceneID].NumOfObjs;
	}
	else
	{	// Ocarina of time

		arrayObjs = OoTSceneObjects[Item.SceneID].Objects;
		arraySize = OoTSceneObjects[Item.SceneID].NumOfObjs;
	}

	for (size_t i = 0; i < arraySize; i++)
	{	// Browse all objects

		currObj = &arrayObjs[i];

		if (currObj->Scene == Item.SceneID)
		{	// They have the same scene ID

			if (Item.OvType > OV_FISH)
			{	// We can check for the object only if its type is above the fish one
				
				if (currObj->Type > ObjectType::fish)
				{	// We can check the object

					if (currObj->ObjectID == Item.ObjectID)
					{	// This is the correct object

						return currObj;
					}
				}
			}
			else
			{	// The object should have the exact same type

				if (currObj->Type == Item.OvType)
				{
					if (currObj->ObjectID == Item.ObjectID)
					{	// This is the correct object

						return currObj;
					}
				}
			}
		}
	}

	return currObj;
}