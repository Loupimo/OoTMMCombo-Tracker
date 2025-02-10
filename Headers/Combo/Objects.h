#pragma once

#include <QObject>
#include "Multi/API.h"
#include "Scenes.h"
#include "NPC.h"
#include "Items.h"

class SceneInfo;

enum ObjectState
{
	Hidden,		// The object has not been collected yet
	Collected,	// The object has been collected in the game
	Forced		// The user has decided to show the object item
};

enum ObjectType
{
	none = 0,
	chest = 1,
	collectible = 2,
	npc = 3,
	gs = 4,			// Gold skulltula
	sf = 5,			// Stray fairy
	cow = 6,
	shop = 7,
	scrub = 8,
	sr = 9,			// Silver Rupee
	fish = 10,
	wonder = 11,
	grass = 12,
	crate = 13,
	pot = 14,
	hive = 15,
	butterfly = 16,
	rupee = 17,
	snowball = 18,
	barrel = 19,
	heart = 20,
	fairy_spot = 21,
	fairy = 22,
};


typedef struct ObjectInfo
{
	uint32_t ObjectID;		// The actual OOTMM rom object ID
	uint32_t Scene;			// The actual OOTMM rom scene
	const char* Name;		// The object friendly name
	const char* Location;	// The string location of the object (should be greatly similar as the spoiler log location)
	ObjectType Type;		// The type of object
	int Position[2];		// Object position on its corresponding scene image. ID 0 = X, ID 1 = Y
	uint32_t RenderScene;	// The scene where the object should be rendered
	const ItemInfo* Item;	// The item contained in the object
	ObjectState Status;		// The status object
} ObjectInfo;


typedef struct SceneObjects
{
	uint32_t SceneID;
	const size_t NumOfObjs;
	ObjectInfo* Objects;
	class SceneInfo* Owner;
} SceneObjects;

/* This macro define static arrays and size arrays that contains all objects of a specific scene */
#define CreateObjectsForScene(SceneID, NumOfObjects, ...) \
const size_t SceneID##NumOfObjs = NumOfObjects; \
ObjectInfo SceneID##SceneObjects [SceneID##NumOfObjs] = { __VA_ARGS__ };

/* This macro define static arrays and size arrays for an empty scene */
#define CreateEmptyScene(SceneID) \
const size_t SceneID##NumOfObjs = 0; \
ObjectInfo * SceneID##SceneObjects = nullptr;

/* This macro create a SceneObjects structure based on the given SceneID */
#define CreateSceneObjects(SceneID) { SceneID, SceneID##NumOfObjs, SceneID##SceneObjects, nullptr }


const char* const ObjTypeName[ObjectType::fairy + 1] =
{
	"",
	"Chest",
	"Collectible Item",
	"NPC / Event",
	"Gold skulltula",
	"Stray Fairy",
	"Cow",
	"Shop",
	"Scrub",
	"Silver Rupee",
	"Fish",
	"Wonder Item",
	"Grass",
	"Crate",
	"Pot",
	"Hive",
	"Butterfly",
	"Rupee",
	"Snowball",
	"Barrel",
	"Heart",
	"Big Fairy",
	"Fairy"
};

SceneObjects* GetGameSceneObjects(uint32_t GameID);
ObjectInfo* FindObject(ComboItem Item);