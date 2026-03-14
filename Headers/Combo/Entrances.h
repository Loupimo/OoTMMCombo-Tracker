#pragma once

#include "Combo/Scenes.h"
#include "Multi/Game.h"
#include <map>
#include <string>

#define SPAWN_LOADING 0xfffe

typedef struct EntranceMetaInfo
{
	uint32_t EntranceID;
	uint32_t SceneID;
	const char* Name;
	const char* FromToName;
	const char* ImagePath;
	GameLayout ActiveLayout;
} EntranceMetaInfo;


class EntranceHelper
{

#pragma region Attributes

public:

	uint32_t EntranceID = 0;				// The current entrance ID.
	uint32_t LastTouchedEntranceID = 0;		// The ID of the last entrance ID
	bool IsEntranceTouched = false;			// Tells if the ID we have is from the touched entrance (true) or the loaded one (false)
	int LastGameTouchedEntrance = NO_GAME;	// Tells which game the last touched entrance came from
	std::string LastTouchedStr;
	std::string EntranceStr;

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
	*	Check the current loaded game and update the LoadedGame attribut.
	*/
	void CheckCurrentLoadedGame();

	/*
	*   Read the current entrance ID for the desired game and store the result in the EntranceID attribute.
	*
	*   @param Game       The game to read the entrance ID from.
	*/
	void ReadEntranceID(int Game, uint8_t* RAMData);

	/*
	*   Check if the current entrance ID is from a grotto.
	*
	*   @return <b>True</b> if the ID is associated to a grotto, <b>false</b> otherwise.
	*/
	bool IsGrottoEntrance(uint32_t ID);

	/*
	*   Check if the current last touched entrance ID is from a grotto.
	*
	*   @return <b>True</b> if the ID is associated to a grotto, <b>false</b> otherwise.
	*/
	bool IsGrottoExit(uint32_t ID);

	/*
	*   Get the entrance grotto associated to the current last entrance ID.
	*
	*   @return The scene grotto associated to the touched exit.
	*/
	uint32_t GetGrottoEntrance(int Game, uint8_t* RAMData, uint32_t ID);

	/*
	*   Get the exit grotto associated to the current last entrance ID.
	*
	*   @return The scene grotto associated to the touched exit.
	*/
	uint32_t GetGrottoExit(int Game, uint8_t* RAMData);

#pragma endregion
};