#pragma once

#include <QObject>
#include <QFile>
#include "Multi/API.h"
#include "Scenes.h"
#include "NPC.h"
#include "Multi/Game.h"
#include "UI/Icons.h"

class SceneInfo;
struct ItemInfo;
struct ComboItem;

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
    silverboulder = 30,
    boulder = 31,

	// Extra type used for rendering
	song = boulder + 1,
	heart_piece = song + 1,
	heart_container = heart_piece + 1,
	small_key = heart_container + 1,
	boss_key = small_key + 1,
	map = boss_key + 1,
	compass = map + 1,
	sword = compass + 1,
	ocarina = sword + 1,
	merchant = ocarina + 1,
	mask = merchant + 1,
	egg = mask + 1,
	owl = egg + 1,

	last = owl + 1
};

const QSet<ObjectType> OoTTypes =
{
	ObjectType::chest, ObjectType::collectible, ObjectType::npc, ObjectType::gs, ObjectType::cow, ObjectType::shop, ObjectType::scrub,
	ObjectType::sr, ObjectType::fish, ObjectType::wonder, ObjectType::grass, ObjectType::crate, ObjectType::pot, ObjectType::hive,
	ObjectType::butterfly, ObjectType::rupee, ObjectType::heart, ObjectType::fairy, ObjectType::fairy_spot, ObjectType::icicle, ObjectType::redice,
    ObjectType::redboulder, ObjectType::boulder, ObjectType::silverboulder,
	ObjectType::rock, ObjectType::soil, ObjectType::tree, ObjectType::bush,
	ObjectType::song, ObjectType::heart_piece, ObjectType::heart_container, ObjectType::small_key, ObjectType::boss_key,
	ObjectType::map, ObjectType::compass, ObjectType::sword, ObjectType::ocarina,
	ObjectType::merchant, ObjectType::mask, ObjectType::egg
};

const QSet<ObjectType> MMTypes =
{
	ObjectType::chest, ObjectType::collectible, ObjectType::npc, ObjectType::gs, ObjectType::sf, ObjectType::cow, ObjectType::shop, ObjectType::scrub,
	ObjectType::wonder, ObjectType::grass, ObjectType::crate, ObjectType::pot, ObjectType::hive, ObjectType::butterfly, ObjectType::rupee, ObjectType::snowball,
	ObjectType::barrel, ObjectType::heart, ObjectType::fairy, ObjectType::icicle, ObjectType::boulder, ObjectType::redboulder, ObjectType::rock, ObjectType::soil, ObjectType::tree, ObjectType::bush,
	ObjectType::song, ObjectType::heart_piece, ObjectType::heart_container, ObjectType::small_key, ObjectType::boss_key, ObjectType::map, ObjectType::compass,
	ObjectType::sword, ObjectType::ocarina, ObjectType::merchant, ObjectType::mask, ObjectType::owl
};

typedef struct ObjectInfo
{
	uint32_t ObjectID;								// The actual OOTMM rom object ID
	uint32_t Scene;									// The actual OOTMM rom scene
	const char* Name;								// The object friendly name
	const char* Location;							// The string location of the object (should be greatly similar as the spoiler log location)
	ObjectType Type;								// The actual OOTMM rom type of object
	int Position[3];								// Object position on its corresponding scene image. ID 0 = X, ID 1 = Y, ID 2 = Z
	uint32_t RenderScene;							// The scene where the object should be rendered
	ObjectType RenderType;							// The type to use to render the object
    EGameIcon MapIcon;							    // The icon to use to render the object on the map
	ObjectContext Context;							// The context in which the object appears
	uint32_t RoomID;								// The room in which the object should be put
	GameLayout Layout;								// The actual game layout to which this object belongs to.
	LocType LocationType;							// The type of location the object is located to.
	const char* Tooltip;							// The text to display in tooltip to give some hint on how to get the object.
	const ItemInfo* Item;							// The item contained in the object
	ObjectState Status = ObjectState::Hidden;		// The status object
	bool PosSet = false;							// Tells if the position has already been set.
	uint8_t TargetWorld = 1;						// Multiworld: the world this placement's item is destined to (1-based). Always 1 in single / coop.

public:

	/*
	*   Save the object in the given save file.
	*
	*   @param SaveFile           The file where to save the object to.
	*   @param IncludeTargetWorld When true, also writes the multiworld destination world
	*                             (TargetWorld) after the item id. Must match the value used
	*                             when loading.
	*/
	void SaveObject(QFile* SaveFile, bool IncludeTargetWorld = false);

	/*
	*   Load the object from the given data.
	*
	*   @param Data               The data array that contains the object to load.
	*   @param Offset             The offset at which the object start.
	*   @param IncludeTargetWorld When true, also reads the multiworld destination world
	*                             (TargetWorld) after the item id. Must match the value used
	*                             when saving.
	*
	*	@return The end offset of the object.
	*/
	size_t LoadObject(QByteArray* Data, size_t Offset, bool IncludeTargetWorld = false);

	/*
	*   Reset the object.
	*/
	void ResetObject();

	/*
	*   Tells if the object layout does match the active one.
	*
	*   @param ActiveLayout		The active layout
	*
	*	@return <b>True</b> if the object layout matches the active layout, <b>false</b> otherwise.
	*/
	bool HasCorrectLayout(GameLayout ActiveLayout);

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
	"Bush",
    "Silver Boulder",
    "Boulder",

	// Extra types
	"Song",
	"Heart Piece",
	"Heart Container",
	"Small Key",
	"Boss Key",
	"Map",
	"Compass",
	"Sword",
	"Ocarina",
	"Merchant",
	"Mask",
	"Egg",
	"Owl"
};

/*
*   Multiworld: a full set of both games' scene-object arrays for a single world.
*   World 0 (the local / world 1) aliases the static template arrays, so single-world
*   behaviour is unchanged. Worlds 1..N-1 are deep copies whose Item / Status /
*   TargetWorld are filled by the spoiler parser and the live network ledger.
*/
typedef struct WorldObjects
{
	SceneObjects* OoT;		// OOT_NUM_SCENES scenes for this world.
	SceneObjects* MM;		// MM_NUM_SCENES scenes for this world.
	bool Owned;				// True when the arrays were heap-allocated and must be freed (clones).
} WorldObjects;

#pragma region Object getters

/*
*   Get all scene objects of the desired game for the currently active world.
*   The active world is controlled by SetActiveWorld; with a single world it always
*   returns the static template arrays, so existing call sites are unaffected.
*
*   @param GameID	The game ID to get the scene objects from.
*
*	@return All scene objects of the desired game for the active world.
*/
SceneObjects* GetGameSceneObjects(uint32_t GameID);

/*
*   Get the scene objects of a specific world and game, regardless of the active world.
*   Used by the spoiler parser and the per-world network routing.
*
*   @param WorldIndex	The 0-based world index (clamped to an existing world).
*   @param GameID		The game ID to get the scene objects from.
*
*	@return The requested world's scene objects for the desired game.
*/
SceneObjects* GetWorldSceneObjects(size_t WorldIndex, uint32_t GameID);

/*
*   Allocate the requested number of worlds as deep copies of the static template arrays
*   (world 0 aliases the templates, worlds 1..N-1 are clones) and reset the active world
*   to 0. Calling it again frees any previously allocated clones first. Passing 1 (or 0)
*   leaves only the template-backed world 0.
*
*   @param NumWorlds	The number of worlds to allocate (at least 1).
*/
void InitWorlds(size_t NumWorlds);

/*
*   @return The number of worlds currently allocated (at least 1).
*/
size_t GetNumWorlds();

/*
*   Select which world GetGameSceneObjects(...) returns.
*
*   @param WorldIndex	The 0-based world index (clamped to an existing world).
*/
void SetActiveWorld(size_t WorldIndex);

/*
*   @return The 0-based index of the currently active world.
*/
size_t GetActiveWorld();

/*
*   Find the object info matching the given combo item.
*
*   @param Item		The item matching the object to find.
*
*	@return The object info matching the given combo item.
*/
ObjectInfo* FindObject(ComboItem Item);

/*
*   Multiworld: given an object resolved against world 0 (the static templates, as returned
*   by FindObject), return the equivalent object in another world. Worlds are byte-for-byte
*   clones of the templates, so the equivalent lives at the same index in the same scene.
*
*   @param WorldIndex	The 0-based destination world index (clamped to an existing world).
*   @param GameID		The game the object belongs to.
*   @param Reference	The reference object (must point inside the world-0 scene arrays).
*
*	@return The equivalent object in the requested world, or the reference itself on failure.
*/
ObjectInfo* FindObjectInWorld(size_t WorldIndex, int GameID, ObjectInfo* Reference);

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
* 
*	@return The end offset of the last loaded scene object.
*/
size_t LoadSceneObjects(QByteArray* Data, size_t Offset);

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

/*
*   Multiworld: save every allocated world's scene objects (both games), including each
*   placement's destination world (TargetWorld). The stream starts with the world count so
*   the loader can re-allocate the right number of worlds. Used by the V2_0 save format.
*
*   @param SaveFile		The save file to write the worlds to.
*/
void SaveAllWorlds(QFile* SaveFile);

/*
*   Multiworld: load every world's scene objects previously written by SaveAllWorlds.
*   Reads the world count first, re-allocates the worlds (InitWorlds) then fills each one.
*
*   @param Data		The data that contains the worlds to load.
*   @param Offset	The starting offset.
*
*	@return The end offset after the last loaded world.
*/
size_t LoadAllWorlds(QByteArray* Data, size_t Offset);

/*
*   Legacy loader for pre-V2_1 saves (world blocks without a leading scene count). Same interface as
*   LoadAllWorlds; kept so old saves still load (best-effort) instead of crashing. Re-save as V2_1 to
*   get the layout-resilient format.
*
*   @param Data		The data that contains the worlds to load.
*   @param Offset	The starting offset.
*
*	@return The end offset after the last loaded world.
*/
size_t LoadAllWorlds_V2_0(QByteArray* Data, size_t Offset);

/*
*   Reset all scene objects of both game.
*/
void ResetSceneObjects();

/*
*   Reset all scene objects of a specific game.
*
*   @param Array		The array containing the scene object to load.
*   @param NumOfScenes	The the number of scenes in the array.
*/
void ResetSceneObjectsFor(SceneObjects* Array, size_t NumOfScenes);

#pragma endregion
