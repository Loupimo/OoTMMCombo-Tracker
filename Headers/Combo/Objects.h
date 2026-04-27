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

enum ObjectIconMap
{
	// Masks
	skull,
	spooky,
    gerudo,
	keaton,
	bunny,
	truth,
	blast,
	bremen,
	captain,
	couple,
	deity,
	fairy_mask,
	garo,
	gero,
	giant,
	gibdo,
	goht,
	goron,
	gyorg,
	kafei,
	kamaro,
	night,
	odolwa,
	postman,
	romani,
	scents,
	stone,
	troupe,
	twinmold,
    deku,
	zora,

	// Swords
	kokiri,
	master,
	broken_big,
	biggoron,
    kokiri_mm,
	razor,
	gilded,
	fairy_sword,

	// Quest
	agony,
	card,
	emerald,
	ruby,
    sapphire,
	light,
	forest,
	fire,
	water,
	shadow,
	spirit,
    gs_token,
    clock_d1,
    clock_d2,
    clock_d3,
    clock_n1,
    clock_n2,
    clock_n3,
	bombers,
	spin,
    sf_green,
    sf_blue,
    sf_yellow,
    sf_orange,
    sf_transcendent,
    swamp_token,
    ocean_token,
	defense,
    magic_upgrade,
    large_magic_upgrade,
    soul_of_animal,
    soul_of_boss,
    soul_of_foe,
    soul_of_npc,
    a_button,
    c_left,
    c_right,
    c_down,
    c_up,
    song_green,
    song_red,
    song_blue,
    song_purple,
    song_orange,
    song_yellow,
    triforce_piece,
    triforce_power,
    triforce_courage,
    triforce_wisdom,


	// Equipment
	big_seed,
	biggest_seed,
	silver,
	golden,
	bracelet,
	silver_gauntlet,
	golden_gauntlet,
    stick_upgrade_1,
    stick_upgrade_2,
    nut_upgrade_1,
    nut_upgrade_2,
	goron_tunic,
	zora_tunic,
	hover,
	iron,
    deku_shield,
    hylian_shield,
	mirror_oot,
    hero_shield,
	mirror_mm,
    bronze_scale,
    bombchu_bag,
	bomb_bag,
	big_bomb,
	biggest_bomb,
	big_quiver,
	biggest_quiver,
    child_wallet,
	wallet,
	big_wallet,
    colossal_wallet,
    bottomless_wallet,
    silver_pouch,

	// Trade
	letter,
	cucco,
	cojiro,
	mushroom,
	potion,
	saw,
	prescription,
	frog,
	eye_drops,
	claim,
	land,
	swamp,
	mountain,
	ocean,
	key,
	letter_kafei,
	letter_mama,
	pendant,

	// Items
    seeds,
	fairy_ocarina,
	boomrang,
	slingshot,
	bow_oot,
	din,
	farore,
	nayru,
	hammer,
	hookshot_oot,
	longshot,
	ruto,
    blue_fire,
	bow_mm,
	chateau,
	dust,
    seahorse,
	hookshot_mm,
	moon,
	picto,
	powder,
	bean,
	bottle,
	milk,
	red_potion,
    green_potion,
    blue_potion,
    bottled_poe,
    bottled_big_poe,
    bottled_fairy,
    bottled_bugs,
    bottled_fish,
	lens,
	fire_arrow,
	ice_arrow,
	light_arrow,
	bombchu,
    bomb,
    stick,
    nut,
    arrow,
    blue_rupee,
    red_rupee,
    purple_rupee,
    gold_rupee,
    rainbow_rupee,
    magical_rupee,
    rupoor,
    magic,
    double_magic,
    skeleton_key,
    key_ring,
    red_coin,
    green_coin,
    blue_coin,
    yellow_coin,
    carp_fish,
    ice_trap,
    fire_trap,
    drain_trap,
    magic_trap,
    shock_trap,
    knockback_trap,


	// Special : must be at the end
	type,			// Tells to use the ID matching the type of the object
	render_type		// Tells to use the ID matching the render type of the obejct
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

	// Extra type used for rendering
	song = bush + 1,
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
	ObjectType::butterfly, ObjectType::rupee, ObjectType::heart, ObjectType::fairy, ObjectType::fairy_spot, ObjectType::icicle, ObjectType::redboulder, ObjectType::redice,
	ObjectType::rock, ObjectType::soil, ObjectType::tree, ObjectType::bush,
	ObjectType::song, ObjectType::heart_piece, ObjectType::heart_container, ObjectType::small_key, ObjectType::boss_key,
	ObjectType::map, ObjectType::compass, ObjectType::sword, ObjectType::ocarina,
	ObjectType::merchant, ObjectType::mask, ObjectType::egg
};

const QSet<ObjectType> MMTypes =
{
	ObjectType::chest, ObjectType::collectible, ObjectType::npc, ObjectType::gs, ObjectType::sf, ObjectType::cow, ObjectType::shop, ObjectType::scrub,
	ObjectType::wonder, ObjectType::grass, ObjectType::crate, ObjectType::pot, ObjectType::hive, ObjectType::butterfly, ObjectType::rupee, ObjectType::snowball,
	ObjectType::barrel, ObjectType::heart, ObjectType::fairy, ObjectType::icicle, ObjectType::redboulder, ObjectType::rock, ObjectType::soil, ObjectType::tree, ObjectType::bush,
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
	ObjectIconMap MapIcon;							// The icon to use to render the object on the map
	ObjectContext Context;							// The context in which the object appears
	uint32_t RoomID;								// The room in which the object should be put
	GameLayout Layout;								// The actual game layout to which this object belongs to.
	LocType LocationType;							// The type of location the object is located to.
	const char* Tooltip;							// The text to display in tooltip to give some hint on how to get the object.
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
