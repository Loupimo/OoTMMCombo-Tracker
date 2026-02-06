#pragma once

#include <QMap>
#include <QFile>

class FilterManager;

enum class ROMGame
{
	oot,
	mm,
	ootmm
};

enum class ShuffleSetting
{
	vanilla,	// Objects for this parameter are vanilla (not shuffled)
	all,		// All objects of this parameter are shuffled
	dungeons,	// Only dungeons objects of this parameter are shuffled
	overworld,	// Only overworld objects of this parameter are shuffled
};

class Settings
{

#pragma region Attributes

public:

	ROMGame Game;
	QMap<QString, ShuffleSetting> ROMSettings;

#pragma endregion

#pragma region Class creation

public:

	Settings();
	~Settings();

	void SaveFileSettings(QFile* SaveFile);
	size_t LoadFileSettings(QByteArray* Data, size_t Offset);

	void ParseSettings(QString& SettingsSection);
	void ParseGamesLayouts(QString& LayoutSection);
	void AddSetting(QString Name, QString Value);

	void ApplySettings(FilterManager* FilterOoT, FilterManager* FilterMM);
	void ApplyOoTSettingsToFilter(FilterManager* Filter);
	void ApplyMMSettingsToFilter(FilterManager* Filter);

#pragma endregion

};