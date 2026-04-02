#pragma once

#include "Combo/Scenes.h"
#include "Multi/Game.h"
#include <map>
#include <string>

#define WARP_LOADING 0xfffe
#define IN_MAGIC	0xFFFFAAAA		// Used to determine if the message is an incoming entrance
#define OUT_MAGIC	0xFFFFBBBB		// Used to determine if the message is an outgoing entrance
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
	Minuet_of_Forest = 0,
	Bolero_of_Fire = 1,
	Serenade_of_Water = 2,
	Requiem_of_Spirit = 3,
	Nocturne_of_Shadow = 4,
	Prelude_of_Light = 5,
	Song_of_Soaring = 9
};


typedef struct EntranceMetaInfo
{
	uint32_t FromEntranceID;
	uint32_t ToEntranceID;
	uint32_t FromSceneID;
	uint32_t ToSceneID;
	const char* FromName;
	const char* ToName;
	EntranceType Type;
	int InPosition[3];				// Entrance incoming position on its corresponding scene image. ID 0 = X, ID 1 = Y, ID 2 = Z
	int OutPosition[3];				// Entrance outgoing position on its corresponding scene image. ID 0 = X, ID 1 = Y, ID 2 = Z
	float ArrowRot;
	GameLayout ActiveLayout;
} EntranceMetaInfo;


typedef struct GrottoEntrance
{
	uint32_t EntranceID;
	float SpawnPos[3];
} GrottoEntrance;

class EntranceHelper
{

#pragma region Attributes

public:

	uint8_t OutGame = NO_GAME;				// The game the last entrance ID comes from
	uint32_t OutEntrance = 0;				// The last outgoing entrance ID
	uint32_t OutScene = 0;					// The last outgoing scene ID
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
	*   Get the warp song matching the entrance ID.
	*
	*	@param Game			The game the data come from. The game can be modified if the warp song is called from MM and leads to OoT and vice versa.
	*	@param ID			The entrance ID.
	*	@param SongIndex	The song index of the last played song.
	*
	*   @return The warp song entrance ID.
	*/
	uint32_t GetWarpSong(uint8_t * Game, uint32_t ID, uint8_t SongIndex);

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
	*	@param Buffer		The entrance message to parse.
	*/
	void ParseOutgoingMessage(uint32_t Buffer[6]);


	static const char* GetEntranceFromName(int Game, uint32_t EntranceID);
	static const char* GetEntranceToName(int Game, uint32_t EntranceID);
	static std::string GetOneWayInName(int Game, uint32_t EntranceID);
	static std::string GetOneWayOutName(int Game, uint32_t EntranceID);
	static std::string GetEntranceSpawnsString(int Game, uint32_t EntranceID);
	static std::string GetEntranceLeadsString(int Game, uint32_t EntranceID);
	static const EntranceMetaInfo* GetEntranceMetaInf(int Game, uint32_t EntranceID);

#pragma endregion

};