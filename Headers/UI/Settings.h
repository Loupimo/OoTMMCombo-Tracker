#pragma once

#include <QMap>
#include <QFile>

class FilterManager;
struct ObjectInfo;

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

	Settings();

	void SaveFileSettings(QFile* SaveFile);
	size_t LoadFileSettings(QByteArray* Data, size_t Offset);

	void ParseSettings(QString& SettingsSection);
	void ParseGamesLayouts(QString& LayoutSection);
	void AddSetting(QString Name, QString Value);

	void ApplySettings(FilterManager* FilterOoT, FilterManager* FilterMM);
	void ApplyOoTSettingsToFilter(FilterManager* Filter);
	void ApplyMMSettingsToFilter(FilterManager* Filter);

	void CheckObjectExclusion(ObjectInfo* ToCheck, ShuffleSetting SettingValue, FilterManager* Filter);

#pragma endregion

};