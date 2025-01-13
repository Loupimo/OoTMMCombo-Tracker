#include "Objects.h"
#include "Items.h"
#include "game.h"
#include "OvTypes.h"

ObjectInfo FindObject(ComboItem Item) 
{
	const ObjectInfo* arrayObjs = nullptr;
	size_t arraySize = 0;
	ObjectInfo currObj;

	if (Item.GameID == MM_GAME)
	{	// Majora's Mask

		arrayObjs = MMObjects;
		arraySize = NUM_MM_OBJ;
	}
	else
	{	// Ocarina of time

		arrayObjs = OoTObjects;
		arraySize = NUM_OOT_OBJ;
	}

	for (size_t i = 0; i < arraySize; i++)
	{	// Browse all objects

		currObj = arrayObjs[i];

		if (currObj.Scene == Item.SceneID)
		{	// They have the same scene ID

			if (Item.OvType > OV_FISH)
			{	// We can check for the object only if its type is above the fish one
				
				if (currObj.Type > ObjectType::fish)
				{	// We can check the object

					if (currObj.ObjectID == Item.ObjectID)
					{	// This is the correct object

						return currObj;
					}
				}
			}
			else
			{	// The object should have the exact same type

				if (currObj.Type == Item.OvType)
				{
					if (currObj.ObjectID == Item.ObjectID)
					{	// This is the correct object

						return currObj;
					}
				}
			}
		}
	}

	return currObj;
}