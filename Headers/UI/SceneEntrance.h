#pragma once

#include "Combo/OoTEntrances.h"
#include "Combo/MMEntrances.h"
#include "UI/SceneEntranceUpdate.h"
#include <map>
#include <vector>
#include <QFile>


// Forward declared so the load chain can take a version parameter without
// pulling AppConfig.h into every translation unit that includes us.
enum class TrackerVersion;


/*
*   A single inbound source (entrance + game) that leads to a given destination
*   entrance. Several of these can target the same destination (e.g. a warp
*   song zone is reachable from both the song and the matching boss room exit),
*   which is why EntranceLink stores a list of them rather than a single value.
*/
typedef struct EntranceSource
{
	uint32_t EntranceID = UINT32_MAX;	// The source entrance ID that leads to the owning entrance
	uint8_t Game = NO_GAME;				// The game the source entrance belongs to
} EntranceSource;


typedef struct EntranceLink
{
	std::vector<EntranceSource> InLinks;	// All distinct source entrances known to lead to this entrance
	uint32_t OutLink = UINT32_MAX;			// The entrance ID where going when leaving from this entrance
	uint8_t OutLinkGame = NO_GAME;			// The game the out link comes from

public:

	/*
	*   Serialize the in / out entrance IDs and their games into the save file.
	*
	*   @param SaveFile    The save file to write the link data into.
	*/
	void SaveLink(QFile* SaveFile);

	/*
	*   Deserialize the in / out entrance IDs and their games from the given byte buffer.
	*
	*   @param Data       The byte buffer containing the save data.
	*   @param Offset     The current offset in the buffer.
	*   @param Version    The tracker version of the buffer; controls the on-disk layout.
	*
	*   @return The new offset after reading the link data.
	*/
	size_t LoadLink(QByteArray* Data, size_t Offset, TrackerVersion Version);

	/*
	*   Reset the link to its default (unbound) state.
	*/
	void ResetLink();

	/*
	*   Tell whether at least one inbound source is known for this entrance.
	*
	*   @return <b>True</b> if the InLinks list has at least one entry, <b>false</b> otherwise.
	*/
	bool HasInLink() const { return !this->InLinks.empty(); }

	/*
	*   Return the most recently discovered inbound source entrance ID, or
	*   UINT32_MAX if no source has been observed yet.
	*
	*   @return The latest inbound source entrance ID.
	*/
	uint32_t GetLatestInLinkID() const { return this->InLinks.empty() ? UINT32_MAX : this->InLinks.back().EntranceID; }

	/*
	*   Return the game of the most recently discovered inbound source, or
	*   NO_GAME if no source has been observed yet.
	*
	*   @return The latest inbound source game.
	*/
	uint8_t GetLatestInLinkGame() const { return this->InLinks.empty() ? NO_GAME : this->InLinks.back().Game; }

	/*
	*   Record a new inbound source for this entrance. No-op if the exact
	*   (EntranceID, Game) pair is already present in the list, so traversing
	*   the same path twice never duplicates an entry.
	*
	*   @param EntranceID    The source entrance ID that leads to this entrance.
	*   @param Game          The game the source entrance belongs to.
	*/
	void AddInLink(uint32_t EntranceID, uint8_t Game);

} EntranceLink;


typedef struct SceneEntranceMetaInf
{
	uint32_t SceneID;								// The scene ID to render
	uint8_t RegionID;								// The region ID the scene belongs to
	std::map<uint32_t, EntranceLink> EntranceIDs;	// The entrances to display on the scene with their in and out link
	const char* MapPath;							// The mini map image path to load

public:

	/*
	*   Serialize the scene meta information and all its entrance links into the save file.
	*
	*   @param SaveFile    The save file to write the meta information into.
	*/
	void SaveMetaInf(QFile* SaveFile);

	/*
	*   Deserialize the scene meta information and all its entrance links from the given byte buffer.
	*
	*   @param Data       The byte buffer containing the save data.
	*   @param Offset     The current offset in the buffer.
	*   @param Version    The tracker version of the buffer; controls the on-disk layout.
	*
	*   @return The new offset after reading the meta information.
	*/
	size_t LoadMetaInf(QByteArray* Data, size_t Offset, TrackerVersion Version);

	/*
	*   Reset all entrance links of this scene to their default state.
	*/
	void ResetMetaInf();

} SceneEntranceMetaInf;

extern std::map<uint32_t, SceneEntranceMetaInf>OoTSceneEntranceMeta;
extern std::map<uint32_t, SceneEntranceMetaInf>MMSceneEntranceMeta;

/*
*   Retrieve the scene entrance meta info for the given game and scene ID.
*
*   @param Game       The game the scene belongs to (OOT_GAME or MM_GAME).
*   @param SceneID    The scene ID to look up.
*
*   @return The scene entrance meta info if found, nullptr otherwise.
*/
SceneEntranceMetaInf* GetSceneEntranceMetaInf(int Game, uint32_t SceneID);

/*
*   Retrieve the full scene entrance meta info map for the given game.
*
*   @param Game    The game to look up (OOT_GAME or MM_GAME).
*
*   @return A pointer to the matching game meta info map, nullptr if the game is unknown.
*/
std::map<uint32_t, SceneEntranceMetaInf>* GetSceneEntranceMetaInfForGame(int Game);

/*
*   Save all entrances status.
*
*   @param SaveFile		The save file to write the entrance status to.
*/
void SaveEntrances(QFile* SaveFile);

/*
*   Serialize all scene entrance meta info of the given array into the save file.
*
*   @param SaveFile    The save file to write the entrance data into.
*   @param Array       The scene entrance meta info map to serialize.
*/
void SaveEntrancesFor(QFile* SaveFile, std::map<uint32_t, SceneEntranceMetaInf>* Array);

/*
*   Load all entrances status from the given file starting at the given offset.
*
*   @param Data       The data that contains the entrances to load.
*   @param Offset     The starting offset.
*   @param Version    The tracker version of the buffer; controls the on-disk layout.
*
*	@return The end offset of the last loaded entrance.
*/
size_t LoadEntrances(QByteArray* Data, size_t Offset, TrackerVersion Version);

/*
*   Deserialize the scene entrance meta info of the given array from the byte buffer.
*
*   @param Data       The byte buffer containing the save data.
*   @param Offset     The current offset in the buffer.
*   @param Array      The scene entrance meta info map to populate.
*   @param Version    The tracker version of the buffer; controls the on-disk layout.
*
*   @return The new offset after reading the entrance data, or -1 if the stored count mismatches.
*/
size_t LoadEntrancesFor(QByteArray* Data, size_t Offset, std::map<uint32_t, SceneEntranceMetaInf>* Array, TrackerVersion Version);

/*
*   Reset all entrance links of every scene in the given array to their default state.
*
*   @param Array    The scene entrance meta info map to reset.
*/
void ResetEntrancesFor(std::map<uint32_t, SceneEntranceMetaInf>* Array);

/*
*   Reset all entrance links of every scene for both Ocarina of Time and Majora's Mask.
*/
void ResetAllEntrances();
