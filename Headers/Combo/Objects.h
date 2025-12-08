#pragma once

#include <QObject>
#include <QFile>
#include "Multi/API.h"
#include "Scenes.h"
#include "NPC.h"
#include "Items.h"
#include "Multi/Game.h"

class SceneInfo;

enum ObjectState
{
	Hidden,		// The object has not been collected yet
	Collected,	// The object has been collected in the game
	Forced		// The user has decided to show the object item
};

enum ObjectContext
{
	All,		// The object is present in all context
	Adult,		// The object is only present when Link is adult
	Child,		// The object is only present when Link is child
	Winter,		// The object is only present when season is winter
	Spring		// The object is only present when season is spring
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
	icicle = 23,
	redboulder = 24,
	redice = 25,
	rock = 26,
	soil = 27,
	tree = 28,
	bush = 29,

	last = bush + 1
};


typedef struct ObjectInfo
{
	uint32_t ObjectID;								// The actual OOTMM rom object ID
	uint32_t Scene;									// The actual OOTMM rom scene
	const char* Name;								// The object friendly name
	const char* Location;							// The string location of the object (should be greatly similar as the spoiler log location)
	ObjectType Type;								// The actual OOTMM rom type of object
	int Position[2];								// Object position on its corresponding scene image. ID 0 = X, ID 1 = Y
	uint32_t RenderScene;							// The scene where the object should be rendered
	ObjectType RenderType;							// The type to use to render the object
	ObjectContext Context;							// The context in which the object appears
	uint32_t RoomID;								// The room in which the object should be put
	ZGame GameID;									// The actual game to which this object belongs to.
	const ItemInfo* Item;							// The item contained in the object
	ObjectState Status = ObjectState::Hidden;		// The status object
	bool PosSet = false;							// Tells if the position has already been set.

public:

	/*
	*   Save the object in the given save file.
	*
	*   @param SaveFile  The file where to save the object to.
	*/
	void SaveObject(QFile* SaveFile);

	/*
	*   Load the object from the given data.
	*
	*   @param Data		The data array that contains the object to load.
	*   @param Offset	The offset at which the object start.
	* 
	*	@return The end offset of the object.
	*/
	size_t LoadObject(QByteArray* Data, size_t Offset);

} ObjectInfo;


typedef struct SceneObjects
{
	uint32_t SceneID;			// The scene ID to which the objects belongs to
	const size_t NumOfObjs;		// The number of objects in this scene
	ObjectInfo* Objects;		// The actual objects

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
#define CreateSceneObjects(SceneID) { SceneID, SceneID##NumOfObjs, SceneID##SceneObjects }

// The strings for all object types
const char* const ObjTypeName[ObjectType::last] =
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
	"Fairy",
	"Icicle",
	"Red Boulder",
	"Red Ice",
	"Rock",
	"Soil",
	"Tree",
	"Bush"
};

#pragma region Object getters

/*
*   Get all scene objects of the desired game.
*
*   @param GameID	The game ID to get the scene objects from.
*
*	@return All scene objects of the desired game.
*/
SceneObjects* GetGameSceneObjects(uint32_t GameID);

/*
*   Find the object info matching the given combo item.
*
*   @param Item		The item matching the object to find.
*
*	@return The object info matching the given combo item.
*/
ObjectInfo* FindObject(ComboItem Item);

#pragma endregion

#pragma region Saving / Loading

/*
*   Save all scene objects of both game to the given file.
*
*   @param SaveFile		The save file to write the scene objects to.
*/
void SaveSceneObjects(QFile* SaveFile);

/*
*   Save all scene objects of a specific game to the given file.
*
*   @param SaveFile		The save file to write the scene objects to.
*   @param Array		The array containing the scene object to save.
*   @param NumOfScenes	The the number of scenes in the array.
*/
void SaveSceneObjectsFor(QFile* SaveFile, SceneObjects* Array, size_t NumOfScenes);

/*
*   Load all scene objects of both game from the given file starting at the given offset.
*
*   @param Data		The data that contains the scene objects to load.
*   @param Offset	The starting offset.
*/
void LoadSceneObjects(QByteArray* Data, size_t Offset);

/*
*   Load all scene objects of a specific game to the given file.
*
*   @param Data			The data that contains the scene objects to load.
*   @param Offset		The starting offset.
*   @param Array		The array containing the scene object to load.
*   @param NumOfScenes	The the number of scenes in the array.
* 
*	@return The end offset of the last loaded scene object.
*/
size_t LoadSceneObjectsFor(QByteArray* Data, size_t Offset, SceneObjects* Array, size_t NumOfScenes);

#pragma endregion