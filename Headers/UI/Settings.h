#pragma once

#include <QMap>
#include <QFile>

class FilterManager;
struct ObjectInfo;
struct SceneObjects;

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

enum class ParamType
{
	game,
	mode,
	uint,
	boolean,
	shuffle,
};

enum class ShuffleSetting
{
	vanilla,	// Objects for this parameter are vanilla (not shuffled)
	all,		// All objects of this parameter are shuffled
	dungeons,	// Only dungeons objects of this parameter are shuffled
	overworld,	// Only overworld objects of this parameter are shuffled
};


typedef struct Parameter
{
public:

	QString Name;
	ParamType Type;
	ShuffleSetting Value;
} Parameter;


class Settings
{

#pragma region Attributes

public:

	ROMGame Game;
	GameMode Mode;
	size_t NumOfTeams;
	QMap<QString, Parameter> ROMSettings;

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
	*   Exclude the given object from the filter based on the shuffle setting and its location type.
	*
	*   @param ToCheck         The object to check for exclusion.
	*   @param SettingValue    The shuffle setting used to decide the exclusion.
	*   @param Filter          The filter manager to update with the exclusion.
	*/
	void CheckObjectExclusion(ObjectInfo* ToCheck, ShuffleSetting SettingValue, FilterManager* Filter);

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
