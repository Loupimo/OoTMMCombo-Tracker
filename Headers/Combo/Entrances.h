#pragma once

#include "Combo/Scenes.h"
#include "Multi/Game.h"
#include "UI/ObjectRenderer.h"
#include <map>
#include <string>

#define WARP_LOADING 0xfffe			// Used to determine if we are in a warping zone.
#define IN_MAGIC	0xFA000000		// Used to determine if the message is an incoming entrance
#define OUT_MAGIC	0xFB000000		// Used to determine if the message is an outgoing entrance
#define WARP_SCENE	0xFF3FFF3F		// Used to know if the scene ID is from a new clock cycle

/*
*   Layout-specific anchor override for the only entrance whose on-image position differs
*   between MM layouts (Bean Grotto exit in Deku Palace, mm vs mm_jp). Kept as a stand-alone
*   constant rather than polluting EntranceMetaInfo with an "alt" field, since this is the
*   sole case in the entire entrance table. Consumed by GetEntranceAnchorPos in Entrances.cpp.
*/
extern const int MM_BEAN_GROTTO_JP_ANCHOR[3];

enum class EntranceType
{
	None,
	Normal,
	One_Way_In,
	One_Way_Out
};


enum class OoTSongs
{
	Minuet_of_Forest = 0x00,
	Bolero_of_Fire = 0x01,
	Serenade_of_Water = 0x02,
	Requiem_of_Spirit = 0x03,
	Nocturne_of_Shadow = 0x04,
	Prelude_of_Light = 0x05,
	Saria_Song = 0x06,
	Epona_Song = 0x07,
	Zelda_Lullaby = 0x08,
	Sun_Song = 0x09,
	Song_of_Time = 0x0A,
	Song_of_Storms = 0x0B,
	Scarecrow_Spawn = 0x0C,
	Memory_Game = 0x0D,
	Song_of_Soaring = 0xFE,
};


enum class MMSongs
{
	Sonata_of_Awakening = 0x00,
	Goron_Lullaby = 0x01,
	New_Wave_Bossa_Nova = 0x02,
	Elegy_of_Emptiness = 0x03,
	Oath_to_Order = 0x04,
	Saria_Song = 0x05,
	Song_of_Time = 0x06,
	Song_of_Healing = 0x07,
	Epona_Song = 0x08,
	Song_of_Soaring = 0x09,
	Song_of_Storms = 0x0A,
	Sun_Song = 0x0B,
	Inverted_Song_of_Time = 0x0C,
	Song_of_Double_Time = 0x0D,
	Goron_Lullaby_Intro = 0x0E,
	Ballad_of_the_Wind_Fish_Human = 0x0F,
	Ballad_of_the_Wind_Fish_Goron = 0x10,
	Ballad_of_the_Wind_Fish_Zora = 0x11,
	Ballad_of_the_Wind_Fish_Deku = 0x12,
	Evan_Song_Part_1 = 0x13,
	Evan_Song_Part_2 = 0x14,
	Zelda_Lullaby = 0x15,
	Scarecrow_Spawn = 0x16,
	Termina_Wall = 0x17,
	Scarecrow_Long = 0x018,
	Minuet_of_Forest = 0x80,
	Bolero_of_Fire = 0x81,
	Serenade_of_Water = 0x82,
	Requiem_of_Spirit = 0x83,
	Nocturne_of_Shadow = 0x84,
	Prelude_of_Light = 0x85
};


enum OwlScene
{
	Great_Bay_Coast = 0x1F,
	Zora_Cape = 0x20,
	Snowhead = 0x21,
	Mountain_Village = 0x22,
	Clock_Town = 0x23,
	Milk_Road = 0x24,
	Woodfall = 0x25,
	Southern_Swamp = 0x26,
	Ikana_Canyon = 0x27,
	Stone_Tower = 0x28
};

enum class LinkAge : uint8_t
{
	Adult = 0,
	Child = 1
};


/*
*   Per-entrance travel-cost table used by the GPS pathfinder. Costs[E] gives
*   the cost (travel time) to reach entrance E from EntranceID inside the same
*   scene. A cost of UINT32_MAX is treated as "unreachable" (placeholder for the
*   future -1 marker once real travel times are filled in).
*/
typedef struct EntranceCost
{
	uint32_t EntranceID;				// The current entrance (matches the owning EntranceMetaInfo.FromEntranceID).
	std::map<uint32_t, uint32_t> Costs;	// Key = reachable entrance in the same scene, Value = cost to reach it.
} EntranceCost;


typedef struct EntranceMetaInfo
{
	uint32_t FromEntranceID;		// The entrance ID.
	uint32_t ToEntranceID;			// The entrance ID this entrance leads to.
	uint32_t FromSceneID;			// The scene ID of this entrance leads to.
	uint32_t ToSceneID;				// The scene ID of the entrance this entrance leads to.
	const char* FromName;			// The name of this entrance.
	const char* ToName;				// The name of the entrance this entrance leads to.
	EntranceType Type;				// The type entrance.
	int AnchorPos[3];				// Entrance anchor position on its corresponding scene image. ID 0 = X, ID 1 = Y, ID 2 = Z
	int TextPos[3];				    // Entrance text box position on its corresponding scene image. ID 0 = X, ID 1 = Y, ID 2 = Z*$
    EntranceIcons RenderIcon;       // The icon to rendrer for this entrance.
	GameLayout ActiveLayout;		// The layout in which this entrance is active.
	EntranceCost Cost;				// Intra-scene travel costs from FromEntranceID. Populated by InitializeEntranceCosts().

	/*
	*   Tells if the entrance layout does match the active one.
	*
	*   @param Layout		The active layout
	*
	*	@return <b>True</b> if the entrance layout matches the active layout, <b>false</b> otherwise.
	*/
	bool HasCorrectLayout(GameLayout Layout) const;
} EntranceMetaInfo;


/*
*   Populate the EntranceCost field of every entry in OoTEntrances and MMEntrances
*   with default intra-scene travel costs. Every entrance in a scene is considered
*   reachable from every other entrance in the same scene at cost 1, then the
*   per-game InitializeOoTMeasuredCosts / InitializeMMMeasuredCosts override the
*   defaults with the real measured travel times shipped in OoT/MMEntranceCosts.cpp.
*   Entries left at the default cost are still unmeasured; UINT32_MAX is reserved
*   for "unreachable" (no walkable path between the two entrances in that scene).
*/
void InitializeEntranceCosts();


/*
*   One measured intra-scene trip imported from entrance_costs.csv: the player
*   arrived in Scene through the From entrance, walked to the From entrance of
*   the To entry (resolved at runtime via the entrance map), and triggered the
*   exit in CostSeconds seconds. Generated by Pool Transform.py.
*/
typedef struct EntranceCostMeasurement
{
    uint32_t Scene;             // The scene the trip took place in.
    uint32_t From;              // The IN entrance ID emitted by the engine when the player arrived (= CSV "FromEntranceID").
    uint32_t To;                // The OUT entrance KEY emitted by the engine when leaving (= CSV "ToEntranceID").
    uint32_t CostSeconds;       // Rounded measured travel time in seconds.
} EntranceCostMeasurement;


/*
*   Apply the measured OoT travel times shipped in OoTEntranceCosts.cpp on top
*   of the default EntranceCost tables. Looks up each measurement's To key in
*   the entrance map to translate it into the actual intra-scene walk target
*   (the OUT entry's FromEntranceID), then writes the cost into the matching
*   source entry's Cost.Costs map.
*
*   @param Map    The OoT entrance map (typically <b>OoTEntrances</b>).
*/
void InitializeOoTMeasuredCosts(std::map<int, EntranceMetaInfo>& Map);


/*
*   MM counterpart of InitializeOoTMeasuredCosts.
*
*   @param Map    The MM entrance map (typically <b>MMEntrances</b>).
*/
void InitializeMMMeasuredCosts(std::map<int, EntranceMetaInfo>& Map);


typedef struct GrottoEntrance
{
	uint32_t EntranceID;			// The entrance ID.
	float SpawnPos[3];				// The spawn corrdinates of the entrance.
} GrottoEntrance;


typedef struct EntranceMessage
{
	const uint32_t* Buffer;		// The orignal message data.
	uint32_t Direction;			// The message direction.
	uint8_t GameID;				// The final message game ID. 
	union
	{
		OoTSongs OoTSongID;		// The message OoT song ID.
		MMSongs MMSongID;		// The message MM song ID.
	};
	LinkAge Age;				// The message link's age. 
	uint8_t FaroreWind;			// The state of the farore's wind.
	uint8_t OwlID;				// The message owl ID.
	uint8_t CurrRoom;			// The message current room.
	uint8_t GrottoData;			// The message grotto data.
	uint16_t CurrSceneID;		// The current scene ID.
	uint32_t SceneID;			// The final message scene ID.
	uint32_t EntranceID;		// The final message entrance ID.
	float X;					// The X respawning player coordinate.
	float Y;					// The Y respawning player coordinate.
	float Z;					// The Z respawning player coordinate.
	EntranceMetaInfo* MetaInf;	// The matching entrance meta information.
	std::string EntranceStr;	// The string matching the direction of the current entrance.

	/*
	*   Populate the message fields using the raw buffer data.
	*
	*   @param MsgDirection   The direction of the message (in or out).
	*   @param OwlID          The combined owl / farore's wind / link age information.
	*   @param Buffer         The raw 6-word message buffer.
	*/
	void SetMessage(uint32_t MsgDirection, uint32_t OwlID, uint32_t Buffer[6]);

	/*
	*   Reset all the fields of the message to their default state.
	*/
	void ResetMessage();

} EntranceMessage;


class EntranceHelper
{

#pragma region Attributes

public:

	EntranceMessage OutMessage;
	EntranceMessage InMessage;

	bool IsEntranceTouched = false;			// Tells if the ID we have is from the touched entrance (true) or the loaded one (false)

#pragma endregion

#pragma region Class creation

public:


	/*
	*   Default entrance helper constructor.
	*/
	EntranceHelper();

	/*
	*   Default entrance helper destructor.
	*/
	~EntranceHelper();

	/*
	*   Reset all the helper to its default state.
	*/
	void ResetEntranceHelper();

#pragma endregion

#pragma region Methods

public:

	/*
	*   Check if the given data are from a new cycle.
	*
	*	@param Message		The entrance message to parse.
	*
	*   @return <b>True</b> if the comes from from a new cycle, <b>false</b> otherwise.
	*/
	bool IsNewCycle(EntranceMessage& PrevMessage, EntranceMessage& CurrMessage);

	/*
	*   Check if the given data are from a moon crash or MM song of time.
	*
	*	@param Message		The entrance message to parse.
	*
	*   @return <b>True</b> if the message comes from an extra scene, <b>false</b> otherwise.
	*/
	bool IsMMExtra(EntranceMessage& Message);

	/*
	*   Check if the given data are from a death warp.
	*
	*	@param Message		The entrance message to parse.
	*
	*   @return <b>True</b> if the message comes from a death warp, <b>false</b> otherwise.
	*/
	bool IsDeath(EntranceMessage& PrevMessage, EntranceMessage& CurrMessage);

	/*
	*   Check if the given data come from a farore's wind action.
	*
	*	@param Message		The entrance message to parse.
	*
	*   @return <b>True</b> if the message comes from a farore's wind action, <b>false</b> otherwise.
	*/
	bool IsFaroreWind(EntranceMessage& Message);

	/*
	*   Check if the given data come from a sonata of awakening to open the woodfall temple.
	*
	*	@param Message		The entrance message to parse.
	*
	*   @return <b>True</b> if the message comes from a sonata of awakening to open the woodfall temple, <b>false</b> otherwise.
	*/
	bool IsSonataWoodfall(EntranceMessage& PrevMessage, EntranceMessage& CurrMessage);
	
	/*
	*   Check if the given data come from a spawn entrance.
	*
	*	@param Message		The entrance message to parse.
	*
	*   @return <b>True</b> if the message comes from a spawn entrance, <b>false</b> otherwise.
	*/
	bool IsSpawn(EntranceMessage& CurrMessage);

	/*
	*   Check if the given data come from a sun's song.
	*
	*	@param Message		The entrance message to parse.
	*
	*   @return <b>True</b> if the message comes from a sun's song, <b>false</b> otherwise.
	*/
	bool IsSunSong(EntranceMessage& PrevMessage, EntranceMessage& CurrMessage);

	/*
	*   Check if the given data come from a song of time.
	*
	*	@param Scene		The current scene ID.
	*	@param EntranceID	The current entrance ID.
	*
	*   @return <b>True</b> if the data come from a song of time, <b>false</b> otherwise.
	*/
	bool IsSongOfTime(EntranceMessage& PrevMessage, EntranceMessage& CurrMessage);

	/*
	*   Check if the given data come from a song of double time.
	*
	*	@param Scene		The current scene ID.
	*	@param EntranceID	The current entrance ID.
	*
	*   @return <b>True</b> if the data come from a song of double time, <b>false</b> otherwise.
	*/
	bool IsSongOfDoubleTime(EntranceMessage& PrevMessage, EntranceMessage& CurrMessage);

	/*
	*   Check if the given entrance ID is from a grotto entrance.
	*
	*	@param ID		The entrance ID to test.
	* 
	*   @return <b>True</b> if the ID is associated to a grotto entrance, <b>false</b> otherwise.
	*/
	bool IsGrottoEntrance(EntranceMessage& Message);

	/*
	*   Check if the given entrance ID is from a grotto exit.
	*
	*	@param ID		The entrance ID to test.
	* 
	*   @return <b>True</b> if the ID is associated to a grotto exit, <b>false</b> otherwise.
	*/
	bool IsGrottoExit(EntranceMessage& Message);

	/*
	*   Check if the given entrance ID is from a warp entrance.
	*
	*	@param ID		The entrance ID to test.
	*
	*   @return <b>True</b> if the ID is associated to a warp entrance, <b>false</b> otherwise.
	*/
	bool IsWarpEntrance(EntranceMessage& Message);

	/*
	*   Get the entrance grotto associated to the current last entrance ID.
	*
	*	@param Game			The game the data come from.
	*	@param GrottoData	The data related to the grotto entrance.
	*	@param ID			The associated type of grotto entrance ID.
	*	@param LastScene	The last scene ID before entering the grotto.
	* 
	*   @return The scene grotto associated to the touched exit.
	*/
	uint32_t GetGrottoEntrance(EntranceMessage& Message, uint32_t LastScene);

	/*
	*   Get the exit grotto associated to the current last entrance ID.
	*
	*	@param Game			The game the data come from.
	*	@param CurrRoom		The current room index.
	*	@param GrottoData	The data related to the grotto entrance.
	* 
	*   @return The scene grotto associated to the touched exit.
	*/
	uint32_t GetGrottoExit(EntranceMessage& Message);
	
	/*
	*   Get the exit grotto associated to the current last entrance ID.
	*
	*	@param Game			The game the data come from.
	*	@param ID			The grotto entrance ID to get the scene from.
	*
	*   @return The scene grotto associated to the given grotto ID.
	*/
	uint32_t CorrectGrottoScene(EntranceMessage& Message);

	/*
	*   Get the cumulative distance between the current player position and the given grotto entrance.
	*
	*	@param Grotto	The grotto entrance to use.
	*	@param X		The X player position.
	*	@param Y		The Y player position.
	*	@param Z		The Z player position.
	*
	*   @return The cumulative distance between the current player position and the given grotto entrance.
	*/
	float GetDistanceGrottoEntrance(GrottoEntrance Grotto, float X, float Y, float Z);

	/*
	*   Get the warp song matching the entrance ID.
	*
	*	@param Game			The game the data come from. The game can be modified if the warp song is called from MM and leads to OoT and vice versa.
	*	@param ID			The entrance ID.
	*	@param SongIndex	The song index of the last played song.
	*	@param OwlID		The Owl ID that has been selected by the player.
	*	@param IsWarpSong	A flag set to know if the returned ID is from a warp song or not.
	*
	*   @return The warp song entrance ID.
	*/
	uint32_t GetWarpSong(EntranceMessage& Message, bool * IsWarpSong);



	/*
	*   Check if the given entrance may match a grotto entrance.
	*
	*	@param ID		The entrance ID that may match.
	*	@param Buffer	The entrance message data.
	* 
	*   @return The matching grotto entrance if the player is close enough, the given entrance otherwise.
	*/
	uint32_t CheckGrottoSpawn(EntranceMessage& Message);

	/*
	*   Check if the given entrance is a special case (like Hyrule Castle <-> Castle Courtyard).
	*
	*	@param Game		The game the data come from.
	*	@param ID		The entrance ID that may match.
	*	@param SceneID	The scene ID the entrance belongs to. Here it can be modified by the function if it is a special case (like Market Child Day / Night).
	*
	*   @return The corrected entrance if it is a special case, the given entrance otherwise.
	*/
	uint32_t CheckSpecialCase(EntranceMessage& Message);

	/*
	*   Check if the given entrance is a warp zone.
	*
	*	@param Game		The game the data come from.
	*	@param ID		The entrance ID that may match.
	*	@param SceneID	The scene ID the entrance belongs to. Here it can be modified by the function if it is a special case (like Market Child Day / Night).
	*	@param X		The X respawn player position.
	*	@param Y		The Y respawn player position.
	*	@param Z		The Z respawn player position.
	*
	*   @return The corrected entrance if it is a special case, the given entrance otherwise.
	*/
	uint32_t CheckWrapScene(EntranceMessage& Message);

	/*
	*   Dispatch the given message to the correct parsing function.
	*
	*	@param EntranceFlag		The entrance flag used to know if it is a in or out message.
	*	@param Buffer			The entrance message to parse.
	*/
	void ParseEntranceMessage(uint32_t EntranceFlag, uint32_t Buffer[6]);

	/*
	*   Parse the given message as an incoming entrance message.
	*
	*	@param Buffer		The entrance message to parse.
	*/
	void ParseIncomingMessage(EntranceMessage& Message);

	/*
	*   Parse the given message as an outgoing entrance message.
	*
	*	@param OwlID		The Owl ID that could be associated with the song ID.
	*	@param Buffer		The entrance message to parse.
	*/
	void ParseOutgoingMessage(EntranceMessage& Message);

	/*
	*   Get the From name of the desired entrance.
	*
	*	@param Game			The game the given entrance belongs to.
	*	@param EntranceID	The entrance to get the name of.
	*
	*	@return The From name of the desired entrance.
	*/
	static const char* GetEntranceFromName(int Game, uint32_t EntranceID);

	/*
	*   Get the To name of the desired entrance.
	*
	*	@param Game			The game the given entrance belongs to.
	*	@param EntranceID	The entrance to get the name of.
	* 
	*	@return The To name of the desired entrance.
	*/
	static const char* GetEntranceToName(int Game, uint32_t EntranceID);
	
	/*
	*   Get the correct name for a one way in type entrance.
	*
	*	@param Game			The game the given entrance belongs to.
	*	@param EntranceID	The entrance to get the name of.
	*
	*	@return The correct name for a one way in type entrance.
	*/
	static std::string GetOneWayInName(int Game, uint32_t EntranceID);

	/*
	*   Get the correct name for a one way out type entrance.
	*
	*	@param Game			The game the given entrance belongs to.
	*	@param EntranceID	The entrance to get the name of.
	*
	*	@return The correct name for a one way out type entrance.
	*/
	static std::string GetOneWayOutName(int Game, uint32_t EntranceID);

	/*
	*   Get the spawns name to display for the desired entrance.
	*
	*	@param Game			The game the given entrance belongs to.
	*	@param EntranceID	The entrance to get the name of.
	*
	*	@return The spawn name for the given entrance.
	*/
	static std::string GetEntranceSpawnsString(int Game, uint32_t EntranceID);

	/*
	*   Get the leads name to display for the desired entrance.
	*
	*	@param Game			The game the given entrance belongs to.
	*	@param EntranceID	The entrance to get the name of.
	*
	*	@return The leads name for the given entrance.
	*/
	static std::string GetEntranceLeadsString(int Game, uint32_t EntranceID);

	/*
	*   Get the meta information matching the desired entrance ID. Entrance IDs are unique within
	*   their game (OoT and MM are independent namespaces), so a single lookup suffices.
	*
	*	@param Game			The game the given entrance belongs to.
	*	@param EntranceID	The entrance to get the meta info of.
	*
	*	@return The meta information of the given entrance, or <b>nullptr</b> if not found.
	*/
	static const EntranceMetaInfo* GetEntranceMetaInf(int Game, uint32_t EntranceID);


#pragma endregion

};


/*
*   Return the on-image anchor coordinates for the given entrance under the given active layout.
*   Normally this just returns <b>Meta.AnchorPos</b>; the function exists to handle the rare cases
*   where the same entrance is rendered at a different position depending on the active layout
*   (currently only Bean Grotto in Deku Palace, mm vs mm_jp).
*
*   @param Meta			The entrance meta information.
*   @param Game			The game the entrance belongs to.
*   @param SceneID		The scene the entrance is being rendered in.
*   @param Layout		The active layout of the scene.
*
*   @return Pointer to a 3-int anchor (X, Y, Z). Never null when <b>Meta.AnchorPos</b> is valid.
*/
const int* GetEntranceAnchorPos(const EntranceMetaInfo& Meta, int Game, uint32_t SceneID, GameLayout Layout);
