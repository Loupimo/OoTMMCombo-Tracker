#include "Combo/Objects.h"
#include "Combo/Items.h"
#include "Combo/OvTypes.h"
#include "Combo/OoTObjectScene.h"
#include "Combo/MMObjectScene.h"
#include "Multi/Game.h"

#pragma region ObjectInfo

void ObjectInfo::SaveObject(QFile* SaveFile)
{
	QByteArray tmp(sizeof(uint32_t), 0);

	// Save ID
	memcpy_s(tmp.data(), 4, &this->ObjectID, sizeof(this->ObjectID));
	SaveFile->write(tmp);

	// Save status
	memcpy_s(tmp.data(), 4, &this->Status, sizeof(this->Status));
	SaveFile->write(tmp);

	// Save item
	uint32_t itemID = 0;
	if (this->Item && this->Item->ItemID != -1)
	{
		itemID = this->Item->ItemID;
	}

	memcpy_s(tmp.data(), 4, &itemID, sizeof(itemID));
	SaveFile->write(tmp);
}


size_t ObjectInfo::LoadObject(QByteArray* Data, size_t Offset)
{
	// Load ID
	uint32_t objID = 0;
	memcpy_s(&objID, sizeof(objID), Data->data() + Offset, sizeof(objID));
	Offset += sizeof(objID);

	if (objID == this->ObjectID)
	{	// It is the correct object

		// Load status
		uint32_t state = 0;
		memcpy_s(&state, sizeof(state), Data->data() + Offset, sizeof(state));
		Offset += sizeof(state);
		this->Status = (ObjectState)state;

		// Load item
		uint32_t itemID = 0;
		memcpy_s(&itemID, sizeof(itemID), Data->data() + Offset, sizeof(itemID));
		Offset += sizeof(itemID);

		if (itemID != 0)
		{	// There is an item to load

			this->Item = FindItem(itemID);
		}
	}

	return Offset;
}

#pragma endregion

#pragma region Object info getter

ObjectInfo* FindObject(ComboItem Item)
{
	ObjectInfo* arrayObjs = nullptr;
	size_t arraySize = 0;
	ObjectInfo* currObj = nullptr;

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

#pragma endregion

#pragma region Saving / Loading

void SaveSceneObjects(QFile* SaveFile)
{
	SaveSceneObjectsFor(SaveFile, OoTSceneObjects, OOT_NUM_SCENES);
	SaveSceneObjectsFor(SaveFile, MMSceneObjects, MM_NUM_SCENES);
}


void SaveSceneObjectsFor(QFile* SaveFile, SceneObjects* Array, size_t NumOfScenes)
{
	QByteArray ID(sizeof(uint32_t), 0);
	QByteArray numObj(sizeof(size_t), 0);

	for (size_t i = 0; i < NumOfScenes; i++)
	{	// Browse all scenes

		// Save scene ID
		memcpy_s(ID.data(), sizeof(Array[i].SceneID), &Array[i].SceneID, sizeof(Array[i].SceneID));
		SaveFile->write(ID);

		// Save number of objects
		memcpy_s(numObj.data(), sizeof(Array[i].NumOfObjs), &Array[i].NumOfObjs, sizeof(Array[i].NumOfObjs));
		SaveFile->write(numObj);

		for (size_t j = 0; j < Array[i].NumOfObjs; j++)
		{	// Save all objects

			Array[i].Objects[j].SaveObject(SaveFile);
		}
	}
}


void LoadSceneObjects(QByteArray * Data, size_t Offset)
{
	Offset = LoadSceneObjectsFor(Data, Offset, OoTSceneObjects, OOT_NUM_SCENES);
	Offset = LoadSceneObjectsFor(Data, Offset, MMSceneObjects, MM_NUM_SCENES);
}


size_t LoadSceneObjectsFor(QByteArray* Data, size_t Offset, SceneObjects * Array, size_t NumOfScenes)
{
	for (size_t i = 0; i < NumOfScenes; i++)
	{	// Browse all scenes

		// Load scene ID
		uint32_t sceneID = 0;
		memcpy_s(&sceneID, sizeof(sceneID), Data->data() + Offset, sizeof(sceneID));
		Offset += sizeof(sceneID);

		if (Array[i].SceneID == sceneID)
		{	// The scene is correct

			// Load number of objects
			size_t numObjs = 0;
			memcpy_s(&numObjs, sizeof(numObjs), Data->data() + Offset, sizeof(numObjs));
			Offset += sizeof(numObjs);

			if (numObjs == Array[i].NumOfObjs)
			{	// It has the same number of objects

				for (size_t j = 0; j < Array[i].NumOfObjs; j++)
				{	// Load all objects

					Offset = Array[i].Objects[j].LoadObject(Data, Offset);
				}
			}
		}
	}

	return Offset;
}

#pragma endregion