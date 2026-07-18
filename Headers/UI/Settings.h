#pragma once

#include <QMap>
#include <QSet>
#include <QFile>
#include <vector>

class FilterManager;
struct ObjectInfo;
struct SceneObjects;

enum class ROMVersion
{
    stable, // 30.1
    stable_31_1,
    dev
};

enum class ROMGame
{
	oot,
	mm,
	ootmm
};

enum class GameMode
{
	single,
	coop,
	multi
};

enum class GoalMode
{
    boss,       // Ganon, Majora, Both
    triforce3,  // Triforce hunt
    triforce    // Triforce Pieces
};

enum class ParamType
{
	game,
	mode,
	uint,
	boolean,
	shuffle,
};

enum class ParamCategory
{
    standard,
    souls,
    progressive,
    shared
};

enum class ShuffleSetting
{
	vanilla,	// Objects for this parameter are vanilla (not shuffled)
    removed,    // Objects for this parameter are removed from the game (not shuffled / not in progression tab)
    starting,   // Objects for this parameter are given as starting item (still shuffled)
	all,		// All objects of this parameter are shuffled
	dungeons,	// Only dungeons objects of this parameter are shuffled
	overworld,	// Only overworld objects of this parameter are shuffled
};


/*
*   Represents a single ROM build parameter parsed from the spoiler log, pairing its name with its type and shuffle setting.
*/
typedef struct Parameter
{
public:

	/* The key name of the parameter as it appears in the spoiler log. */
	QString Name;
	/* The value type of this parameter (game, mode, boolean, shuffle, etc.). */
	ParamType Type;
    /* The parameter category use to apply the setting on the correct array using the correct function. */
    ParamCategory Cat;
	/* The shuffle setting assigned to this parameter, controlling which objects are active or excluded. */
	ShuffleSetting Value;
    /* The values affected by this parameter. */
    std::vector<uint32_t> AffectedVal;
} Parameter;


class Settings
{

#pragma region Attributes

public:

	/* The ROM game combination (OoT only, MM only, or OoTMM combo) parsed from the spoiler log. */
    ROMVersion Version;
	ROMGame Game;
	GameMode Mode;
    GoalMode Goal;
	size_t NumOfTeams;
    bool IsFireTempleOpenAsChild;
	/* The local player's world / team id (1-based) used to route items in coop / multiworld.
	   Matches the spoiler ("World 1") and the network ledger (playerFrom / playerTo). In
	   single mode it is ignored. */
	size_t LocalWorld;
	QMap<QString, Parameter> FilterSettings;
    QMap<QString, Parameter> ItemSettings;

    // ProgressionTab-facing toggles. Populated by the spoiler-log / settings
    // parser; consumed by ProgressionTab::ApplySettings.
    //   * SharedItemIDs   - item IDs whose updates must propagate to every
    //                       widget that lists them (only honored when the
    //                       matching ItemInfo::CanBeShared is true).
    //   * DisabledItemIDs - item IDs whose widgets must be hidden in the
    //                       progression dashboard (e.g. enemy-soul shuffle off).
    //   * StartingItemIDs - item IDs the player starts the run with; their
    //                       widgets are pre-marked as found and the detail
    //                       panel shows a non-clickable "Starting Item" entry.
    QSet<uint32_t> SharedItemIDs;
    QSet<uint32_t> DisabledItemIDs;
    QSet<uint32_t> ProgressiveItemIDs;
    QMap<uint32_t, uint32_t> StartingItemIDs;

    // Multiworld: starting items differ per world. In a multiworld spoiler the "Starting Items"
    // section is split into "Player N" sub-blocks; each world's items are stored here, indexed
    // 0-based (world 1 -> index 0). Empty in single / coop, where StartingItemIDs alone is used.
    std::vector<QMap<uint32_t, uint32_t>> StartingItemIDsByWorld;

#pragma endregion

#pragma region Class creation

public:

	/*
	*   Constructs the settings with default values and the full list of ROM parameters.
	*/
	Settings();

	/*
	*   Write the tracker version and the number of parameters to the given save file.
	*
	*   @param SaveFile    The save file to write the settings into.
	*/
	void SaveFileSettings(QFile* SaveFile);

	/*
	*   Read the number of parameters from the given byte buffer.
	*
	*   @param Data      The byte buffer containing the save data.
	*   @param Offset    The current offset in the buffer.
	*
	*   @return The new offset after reading the settings section.
	*/
	size_t LoadFileSettings(QByteArray* Data, size_t Offset);

	/*
	*   Parse the settings and world layout sections from the given spoiler log text.
	*
	*   @param SettingsSection    The text section containing the settings and world layouts.
	*/
	void ParseSettings(QString& SettingsSection);

    /*
    *   Parse all starting items sub-sections from the spoiler log.
    *
    *   @param LayoutSection    The text section containing the starting items.
    */
    void ParseStartingItems(QString& LayoutSection);

	/*
	*   Parse all world-flag sub-sections (key rings, silver pouches, game layouts) from the spoiler log.
	*
	*   @param LayoutSection    The text section containing the world flags.
	*/
    void ParseWorldFlags(QString& LayoutSection);

	/*
	*   Parse the small key ring layout for both games and update the disabled item IDs accordingly.
	*
	*   @param LayoutSection    The text section containing the small key ring layout.
	*/
    void ParseKeyRings(QString& LayoutSection);

	/*
	*   Parse the silver rupee pouches layout and update the disabled item IDs accordingly.
	*
	*   @param LayoutSection    The text section containing the silver rupee pouches layout.
	*/
    void ParseSilverPouches(QString& LayoutSection);

    /*
    *   Parse the opened dungeons for OoT and update the dungeons status accordingly.
    *
    *   @param LayoutSection    The text section containing the opened dungeons for OoT.
    */
    void ParseOpenDungeonsOoT(QString& LayoutSection);

    /*
    *   Parse the pre-activated owl layout and update the starting item IDs accordingly.
    *
    *   @param LayoutSection    The text section containing the pre-activated owl layout.
    */
    void ParsePreActivatedOwl(QString& LayoutSection);

	/*
	*   Parse the game layout section and apply the Master Quest / JP layout to the matching scenes.
	*
	*   @param LayoutSection    The text section containing the world layout information.
	*/
	void ParseGamesLayouts(QString& LayoutSection);

	/*
	*   Add or update a setting value in the ROM settings map.
	*
	*   @param Name     The name of the setting to add or update.
	*   @param Value    The value of the setting as a string.
	*/
	void AddSetting(QString Name, QString Value);

	/*
	*   Apply the ROM settings to the corresponding game filter manager(s).
	*
	*   @param FilterOoT    The filter manager of the Ocarina of Time game tab.
	*   @param FilterMM     The filter manager of the Majora's Mask game tab.
	*/
	void ApplySettings(FilterManager* FilterOoT, FilterManager* FilterMM);

	/*
	*   Apply the Ocarina of Time ROM settings to the given filter manager.
	*
	*   @param Filter    The filter manager to apply the settings to.
	*/
	void ApplyOoTSettingsToFilter(FilterManager* Filter);

	/*
	*   Apply the Majora's Mask ROM settings to the given filter manager.
	*
	*   @param Filter    The filter manager to apply the settings to.
	*/
	void ApplyMMSettingsToFilter(FilterManager* Filter);


	/*
	*   Apply the parsed item-related ROM settings to the disabled item IDs set used by the progression tab.
	*/
    void ApplyItemSettings();

	/*
	*   Exclude the given object from the filter based on the shuffle setting and its location type.
	*
	*   @param ToCheck         The object to check for exclusion.
	*   @param SettingValue    The shuffle setting used to decide the exclusion.
	*   @param Filter          The filter manager to update with the exclusion.
	*/
	void CheckObjectExclusion(ObjectInfo* ToCheck, ShuffleSetting SettingValue, FilterManager* Filter);


	/*
	*   Check if the given item is enabled based on the shuffle setting and disable it if vanilla.
	*
	*   @param SettingValue    The shuffle setting used to decide if the item is enabled.
	*   @param ItemID          The ID of the item to check.
	*
	*   @return True if the item is enabled, false otherwise.
	*/
    bool CheckItemEnabled(ShuffleSetting SettingValue, uint32_t ItemID);

private:

	/*
	*   Initialize filter and get scenes for the given game.
	*
	*   @param Filter           The filter manager to reset.
	*   @param Game             The game ID (OOT_GAME or MM_GAME).
	*   @return                 Pointer to the game's scene objects array.
	*/
	SceneObjects* ApplySettingsToFilter(FilterManager* Filter, uint32_t Game);

#pragma endregion

};
