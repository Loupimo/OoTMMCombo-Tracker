#pragma once

#include "Combo/OoTEntrances.h"
#include "Combo/MMEntrances.h"
#include <map>
#include <QFile>

typedef struct EntranceCost
{
	uint32_t EntranceID;				// The current entrance
	std::map<uint32_t, uint32_t> Costs;	// Key = accessible entrance, Value = cost to reach the entrance from current entrance
} EntranceCost;


typedef struct EntranceLink
{
	uint32_t InLink = UINT32_MAX;		// The entrance ID that leads to this scene spawn
	uint32_t OutLink = UINT32_MAX;		// The entrance ID where going when leaving from this entrance
	uint8_t InLinkGame = NO_GAME;		// The game the out link comes from
	uint8_t OutLinkGame = NO_GAME;		// The game the out link comes from

public:

	void SaveLink(QFile* SaveFile);
	size_t LoadLink(QByteArray* Data, size_t Offset);

} EntranceLink;


typedef struct SceneEntranceMetaInf
{
	uint32_t SceneID;								// The scene ID to render
	std::map<uint32_t, EntranceLink> EntranceIDs;	// The entrances to display on the scene with their in and out link
	const char* MapPath;							// The mini map image path to load

public:

	void SaveMetaInf(QFile* SaveFile);
	size_t LoadMetaInf(QByteArray* Data, size_t Offset);

} SceneEntranceMetaInf;

typedef struct SceneEntranceUpdate
{
	uint8_t Game;
	uint32_t SceneID;
	uint32_t EntranceID;
	EntranceLink* Link;
}SceneEntranceUpdate;


extern std::map<uint32_t, SceneEntranceMetaInf>OoTSceneEntranceMeta;
extern std::map<uint32_t, SceneEntranceMetaInf>MMSceneEntranceMeta;

SceneEntranceMetaInf* GetSceneEntranceMetaInf(int Game, uint32_t SceneID);
std::map<uint32_t, SceneEntranceMetaInf>* GetSceneEntranceMetaInfForGame(int Game);

/*
*   Save all entrances status.
*
*   @param SaveFile		The save file to write the entrance status to.
*/
void SaveEntrances(QFile* SaveFile);

void SaveEntrancesFor(QFile* SaveFile, std::map<uint32_t, SceneEntranceMetaInf>* Array);

/*
*   Load all entrances status from the given file starting at the given offset.
*
*   @param Data		The data that contains the entrances to load.
*   @param Offset	The starting offset.
* 
*	@return The end offset of the last loaded entrance.
*/
size_t LoadEntrances(QByteArray* Data, size_t Offset);

size_t LoadEntrancesFor(QByteArray* Data, size_t Offset, std::map<uint32_t, SceneEntranceMetaInf>* Array);