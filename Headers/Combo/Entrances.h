#pragma once

#include "Combo/Scenes.h"
#include "Multi/Game.h"
#include <map>
#include <string>

#define WARP_LOADING 0xfffe			// Used to determine if we are in a warping zone.
#define IN_MAGIC	0xFFFFAA00		// Used to determine if the message is an incoming entrance
#define OUT_MAGIC	0xFFFFBB00		// Used to determine if the message is an outgoing entrance
#define WARP_SCENE	0xFF3FFF3F		// Used to know if the scene ID is from a new clock cycle


enum class EntranceType
{
	None,
	Normal,
	One_Way_In,
	One_Way_Out
};


enum WarpSong
{
	Minuet_of_Forest = 0,		// OoT Value
	Bolero_of_Fire = 1,			// OoT Value
	Serenade_of_Water = 2,		// OoT Value
	Requiem_of_Spirit = 3,		// OoT Value
	Nocturne_of_Shadow = 4,		// OoT Value
	Prelude_of_Light = 5,		// OoT Value
	Song_of_Time = 0x0A,		// OoT Value
	Sun_Song = 0x0900,			// OoT value, real value is 9 but as it is also the value for MM soaring I prefere to set it to 0x0900
	Song_of_Soaring = 9,		// MM value
	Song_of_Double_Time = 0x0D	// MM value
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


typedef struct EntranceMetaInfo
{
	uint32_t FromEntranceID;		// The entrance ID.
	uint32_t ToEntranceID;			// The entrance ID this entrance leads to.
	uint32_t FromSceneID;			// The scene ID of this entrance leads to.
	uint32_t ToSceneID;				// The scene ID of the entrance this entrance leads to.
	const char* FromName;			// The name of this entrance.
	const char* ToName;				// The name of the entrance this entrance leads to.
	EntranceType Type;				// The type entrance.
	int InPosition[3];				// Entrance incoming position on its corresponding scene image. ID 0 = X, ID 1 = Y, ID 2 = Z
	int OutPosition[3];				// Entrance outgoing position on its corresponding scene image. ID 0 = X, ID 1 = Y, ID 2 = Z
	float ArrowRot;					// The rotation to apply to the arrow image.
	GameLayout ActiveLayout;		// The layout in which this entrance is active.
} EntranceMetaInfo;


typedef struct GrottoEntrance
{
	uint32_t EntranceID;			// The entrance ID.
	float SpawnPos[3];				// The spawn corrdinates of the entrance.
} GrottoEntrance;

class EntranceHelper
{

#pragma region Attributes

public:

	uint8_t OutGame = NO_GAME;				// The game the last entrance ID comes from
	uint32_t OutEntrance = 0;				// The last outgoing entrance ID
	uint32_t OutScene = 0;					// The last outgoing scene ID
	uint32_t * OutBuffer;					// The last original outgoing message buffer
	EntranceMetaInfo* OutMetaInf = NULL;	// The last outgoing entrance meta information
	bool IsEntranceTouched = false;			// Tells if the ID we have is from the touched entrance (true) or the loaded one (false)
	std::string LastTouchedStr;				// The string matching the direction of the last touched entrance
	std::string EntranceStr;				// The string matching the direction of the current entrance

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
	*	@param Buffer		The entrance message to parse.
	*
	*   @return <b>True</b> if the data are from a new cycle, <b>false</b> otherwise.
	*/
	bool IsNewCycle(uint32_t Buffer[6]);

	/*
	*   Check if the given data are from a moon crash or MM song of time.
	*
	*	@param Buffer		The entrance message to parse.
	*
	*   @return <b>True</b> if the data are from an extra scene, <b>false</b> otherwise.
	*/
	bool IsMMExtra(uint32_t Buffer[6]);

	/*
	*   Check if the given data are from a death warp.
	*
	*	@param Buffer		The entrance message to parse.
	*
	*   @return <b>True</b> if the data are from a death warp, <b>false</b> otherwise.
	*/
	bool IsDeath(uint32_t Buffer[6]);

	/*
	*   Check if the given data come from a sun's song.
	*
	*	@param Scene		The current scene ID.
	*	@param EntranceID	The current entrance ID.
	*
	*   @return <b>True</b> if the data come from a sun's song, <b>false</b> otherwise.
	*/
	bool IsSunSong(uint32_t Scene, uint32_t EntranceID);

	/*
	*   Check if the given data come from a song of time.
	*
	*	@param Scene		The current scene ID.
	*	@param EntranceID	The current entrance ID.
	*
	*   @return <b>True</b> if the data come from a song of time, <b>false</b> otherwise.
	*/
	bool IsSongOfTime(uint32_t Buffer[6]);

	/*
	*   Check if the given data come from a song of double time.
	*
	*	@param Scene		The current scene ID.
	*	@param EntranceID	The current entrance ID.
	*
	*   @return <b>True</b> if the data come from a song of double time, <b>false</b> otherwise.
	*/
	bool IsSongOfDoubleTime(uint32_t Scene, uint32_t EntranceID);

	/*
	*   Check if the given entrance ID is from a grotto entrance.
	*
	*	@param ID		The entrance ID to test.
	* 
	*   @return <b>True</b> if the ID is associated to a grotto entrance, <b>false</b> otherwise.
	*/
	bool IsGrottoEntrance(uint32_t ID);

	/*
	*   Check if the given entrance ID is from a grotto exit.
	*
	*	@param ID		The entrance ID to test.
	* 
	*   @return <b>True</b> if the ID is associated to a grotto exit, <b>false</b> otherwise.
	*/
	bool IsGrottoExit(uint32_t ID);

	/*
	*   Check if the given entrance ID is from a warp entrance.
	*
	*	@param ID		The entrance ID to test.
	*
	*   @return <b>True</b> if the ID is associated to a warp entrance, <b>false</b> otherwise.
	*/
	bool IsWarpEntrance(uint32_t ID);

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
	uint32_t GetGrottoEntrance(uint8_t Game, uint8_t GrottoData, uint32_t ID, uint32_t LastScene);

	/*
	*   Get the exit grotto associated to the current last entrance ID.
	*
	*	@param Game			The game the data come from.
	*	@param CurrRoom		The current room index.
	*	@param GrottoData	The data related to the grotto entrance.
	*	@param LastScene	The last scene ID before exiting the grotto.
	* 
	*   @return The scene grotto associated to the touched exit.
	*/
	uint32_t GetGrottoExit(uint8_t Game, uint8_t CurrRoom, uint8_t GrottoData, uint32_t LastScene);
	
	/*
	*   Get the exit grotto associated to the current last entrance ID.
	*
	*	@param Game			The game the data come from.
	*	@param ID			The grotto entrance ID to get the scene from.
	*
	*   @return The scene grotto associated to the given grotto ID.
	*/
	uint32_t CorrectGrottoScene(uint8_t Game, uint32_t ID);

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
	uint32_t GetWarpSong(uint8_t * Game, uint32_t ID, uint8_t SongIndex, uint8_t OwlID, bool * IsWarpSong);



	/*
	*   Check if the given entrance may match a grotto entrance.
	*
	*	@param ID		The entrance ID that may match.
	*	@param Buffer	The entrance message data.
	* 
	*   @return The matching grotto entrance if the player is close enough, the given entrance otherwise.
	*/
	uint32_t CheckGrottoSpawn(uint32_t ID, uint32_t Buffer[6]);

	/*
	*   Check if the given entrance is a special case (like Hyrule Castle <-> Castle Courtyard).
	*
	*	@param Game		The game the data come from.
	*	@param ID		The entrance ID that may match.
	*	@param SceneID	The scene ID the entrance belongs to. Here it can be modified by the function if it is a special case (like Market Child Day / Night).
	*
	*   @return The corrected entrance if it is a special case, the given entrance otherwise.
	*/
	uint32_t CheckSpecialCase(uint8_t Game, uint32_t ID, uint32_t * SceneID);

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
	uint32_t CheckWrapScene(uint8_t Game, uint32_t ID, uint32_t* SceneID, uint32_t X, uint32_t Y, uint32_t Z);

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
	void ParseIncomingMessage(uint32_t Buffer[6]);

	/*
	*   Parse the given message as an outgoing entrance message.
	*
	*	@param OwlID		The Owl ID that could be associated with the song ID.
	*	@param Buffer		The entrance message to parse.
	*/
	void ParseOutgoingMessage(uint8_t OwlID, uint32_t Buffer[6]);

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
	*   Get the meta information macthing the desired entrance ID.
	*
	*	@param Game			The game the given entrance belongs to.
	*	@param EntranceID	The entrance to get the meta info of.
	*
	*	@return The meta information of the given entrance.
	*/
	static const EntranceMetaInfo* GetEntranceMetaInf(int Game, uint32_t EntranceID);



#pragma endregion

};