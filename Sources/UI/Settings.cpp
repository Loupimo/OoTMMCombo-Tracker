#include "UI/FilterManager.h"
#include "UI/AppConfig.h"
#include "UI/Settings.h"
#include "Combo/Scenes.h"



Settings::Settings()
{
	this->Game = ROMGame::ootmm;
	this->ROMSettings = QMap<QString, ShuffleSetting>();
}


Settings::~Settings()
{
	this->ROMSettings.clear();
}


void Settings::SaveFileSettings(QFile* SaveFile)
{
	QByteArray tmpVersion(sizeof(uint32_t), 0);
	QByteArray tmpParams(sizeof(qsizetype), 0);
	QByteArray tmpSetting(sizeof(uint8_t), 0);


	uint32_t currVersion = (uint32_t)AppConfig::GetTrackerVersion();

	// Save tracker version
	memcpy_s(tmpVersion.data(), 4, &currVersion, sizeof(currVersion));
	SaveFile->write(tmpVersion);

	// Save num of parameters
	qsizetype numOfParams = this->ROMSettings.size();
	memcpy_s(tmpParams.data(), sizeof(qsizetype), &numOfParams, sizeof(qsizetype));
	SaveFile->write(tmpParams);
}


size_t Settings::LoadFileSettings(QByteArray* Data, size_t Offset)
{
	return 0;
}


void Settings::ParseSettings(QString& SettingsSection)
{	// Split with "===" to find the right section

	this->ROMSettings.clear();

	QRegularExpression reg("^([^\\s].*(?:\n(?![^\\s]).*)*)", QRegularExpression::MultilineOption);
	QRegularExpressionMatchIterator it = reg.globalMatch(SettingsSection);

	QStringList sections;
	while (it.hasNext())
	{   // Fill the section array with all the gathered matches

		QRegularExpressionMatch match = it.next();
		sections.append(match.captured(1));
	}

	for (qsizetype i = 0; i < sections.size(); i++)
	{
		QString currSection = sections.at(i);

		if (currSection.startsWith("Settings"))
		{	// Settings

			// Regex to split strings by location
			QRegularExpression reg("^\\s{2}(.+)\n", QRegularExpression::MultilineOption);
			QRegularExpressionMatchIterator it = reg.globalMatch(currSection);

			QStringList settings;

			while (it.hasNext())
			{   // Fill the maps array with all the gathered matches

				QRegularExpressionMatch match = it.next();
				settings.append(match.captured(1));
				reg = QRegularExpression("^(.+):\\s(.+)", QRegularExpression::MultilineOption);
				match = reg.globalMatch(match.captured(1)).next();
				this->AddSetting(match.captured(1), match.captured(2));
			}

		}
		else if (currSection.startsWith("World"))
		{	// World flags

			this->ParseGamesLayouts(currSection);
		}
	}
}


void Settings::ParseGamesLayouts(QString& LayoutSection)
{
	// Reset layouts
	GetSceneMetaInfo(DEKU_TREE, OOT_GAME)->ActiveLayout = GameLayout::oot;
	GetSceneMetaInfo(DODONGO_CAVERN, OOT_GAME)->ActiveLayout = GameLayout::oot;
	GetSceneMetaInfo(INSIDE_JABU_JABU, OOT_GAME)->ActiveLayout = GameLayout::oot;
	GetSceneMetaInfo(TEMPLE_FOREST, OOT_GAME)->ActiveLayout = GameLayout::oot;
	GetSceneMetaInfo(TEMPLE_FIRE, OOT_GAME)->ActiveLayout = GameLayout::oot;
	GetSceneMetaInfo(TEMPLE_WATER, OOT_GAME)->ActiveLayout = GameLayout::oot;
	GetSceneMetaInfo(TEMPLE_SHADOW, OOT_GAME)->ActiveLayout = GameLayout::oot;
	GetSceneMetaInfo(TEMPLE_SPIRIT, OOT_GAME)->ActiveLayout = GameLayout::oot;
	GetSceneMetaInfo(BOTTOM_OF_THE_WELL, OOT_GAME)->ActiveLayout = GameLayout::oot;
	GetSceneMetaInfo(GERUDO_TRAINING_GROUND, OOT_GAME)->ActiveLayout = GameLayout::oot;
	GetSceneMetaInfo(ICE_CAVERN, OOT_GAME)->ActiveLayout = GameLayout::oot;
	GetSceneMetaInfo(INSIDE_GANON_CASTLE, OOT_GAME)->ActiveLayout = GameLayout::oot;
	GetSceneMetaInfo(MM_GROTTO_DEKU_PALACE_GENERIC, MM_GAME)->ActiveLayout = GameLayout::mm;

	QRegularExpression reg("^  ((?:Master Quest Dungeons|Majora's Mask JP Layouts):(?: \\w*|(?:\n    - .+)*))", QRegularExpression::MultilineOption);
	QRegularExpressionMatchIterator it = reg.globalMatch(LayoutSection);

	while (it.hasNext())
	{
		QRegularExpressionMatch match = it.next();
		QStringList layoutParams = match.captured(1).split("\n");

		if (layoutParams.size() == 1)
		{	// None or all

			layoutParams = layoutParams.at(0).split(": ");
			if (layoutParams.at(0) == "Master Quest Dungeons")
			{	// Ocarina of Time

				if (layoutParams.at(1) == "all")
				{	// All MQ

					GetSceneMetaInfo(DEKU_TREE, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					GetSceneMetaInfo(DODONGO_CAVERN, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					GetSceneMetaInfo(INSIDE_JABU_JABU, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					GetSceneMetaInfo(TEMPLE_FOREST, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					GetSceneMetaInfo(TEMPLE_FIRE, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					GetSceneMetaInfo(TEMPLE_WATER, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					GetSceneMetaInfo(TEMPLE_SHADOW, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					GetSceneMetaInfo(TEMPLE_SPIRIT, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					GetSceneMetaInfo(BOTTOM_OF_THE_WELL, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					GetSceneMetaInfo(ICE_CAVERN, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					GetSceneMetaInfo(GERUDO_TRAINING_GROUND, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					GetSceneMetaInfo(INSIDE_GANON_CASTLE, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
				}
			}
			else
			{	// Majora's mask

				if (layoutParams.at(1) == "all")
				{	// All JP

					GetSceneMetaInfo(MM_GROTTO_DEKU_PALACE_GENERIC, MM_GAME)->ActiveLayout = GameLayout::mm_jp;
				}
			}
		}
		else
		{
			if (layoutParams.at(0) == "Master Quest Dungeons:")
			{	// Ocarina of Time

				for (qsizetype i = 1; i < layoutParams.size(); i++)
				{
					QString currScene = layoutParams.at(i);
					currScene = currScene.replace("    - ", "");

					if (currScene == "Deku Tree")
					{
						GetSceneMetaInfo(DEKU_TREE, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					}
					else if (currScene == "Dodongo cavern")
					{
						GetSceneMetaInfo(DODONGO_CAVERN, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					}
					else if (currScene == "Jabu-Jabu")
					{
						GetSceneMetaInfo(INSIDE_JABU_JABU, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					}
					else if (currScene == "Forest Temple")
					{
						GetSceneMetaInfo(TEMPLE_FOREST, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					}
					else if (currScene == "Fire Temple")
					{
						GetSceneMetaInfo(TEMPLE_FIRE, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					}
					else if (currScene == "Water Temple")
					{
						GetSceneMetaInfo(TEMPLE_WATER, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					}
					else if (currScene == "Shadow Temple")
					{
						GetSceneMetaInfo(TEMPLE_SHADOW, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					}
					else if (currScene == "Spirit Temple")
					{
						GetSceneMetaInfo(TEMPLE_SPIRIT, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					}
					else if (currScene == "Bottom of the Well")
					{
						GetSceneMetaInfo(BOTTOM_OF_THE_WELL, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					}
					else if (currScene == "Ice Cavern")
					{
						GetSceneMetaInfo(ICE_CAVERN, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					}
					else if (currScene == "Gerudo Training Grounds")
					{
						GetSceneMetaInfo(GERUDO_TRAINING_GROUND, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					}
					else if (currScene == "Ganon's Castle")
					{
						GetSceneMetaInfo(INSIDE_GANON_CASTLE, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					}
				}
			}
			else
			{	// Majora's mask

				for (qsizetype i = 1; i < layoutParams.size(); i++)
				{
					QString currScene = layoutParams.at(i);
					currScene = currScene.replace("    - ", "");

					if (currScene == "Deku Palace")
					{	// Deku Palace layout

						GetSceneMetaInfo(MM_GROTTO_DEKU_PALACE_GENERIC, MM_GAME)->ActiveLayout = GameLayout::mm_jp;
					}
				}
			}
		}
	}
}


void Settings::AddSetting(QString Name, QString Value)
{
	if (Name == "games")
	{
		if (Value == "oot")
		{
			this->Game = ROMGame::oot;
		}
		else if (Value == "mm")
		{
			this->Game = ROMGame::mm;
		}
		else
		{
			this->Game = ROMGame::ootmm;
		}
	}
	else
	{
		ShuffleSetting setting;

		if (Value == "all" || Value == "true" || Value == "anywhere" || Value == "starting" || Value == "removed" || "cross")
		{
			setting = ShuffleSetting::all;
		}
		else if (Value == "dungeons" || Value == "ownDungeon")
		{
			setting = ShuffleSetting::dungeons;
		}
		else if (Value == "overworld")
		{
			setting = ShuffleSetting::overworld;
		}
		else
		{
			setting = ShuffleSetting::vanilla;
		}

		this->ROMSettings.insert(Name, setting);
	}
}


void Settings::ApplySettings(FilterManager* FilterOoT, FilterManager* FilterMM)
{
	switch (this->Game)
	{
		case ROMGame::oot:
		{
			this->ApplyOoTSettingsToFilter(FilterOoT);
			break;
		}

		case ROMGame::mm:
		{
			this->ApplyMMSettingsToFilter(FilterMM);
			break;
		}

		case ROMGame::ootmm:
		{
			this->ApplyOoTSettingsToFilter(FilterOoT);
			this->ApplyMMSettingsToFilter(FilterMM);
			break;
		}

		default:
		{
			break;
		}
	}
}

void Settings::ApplyOoTSettingsToFilter(FilterManager* Filter)
{
	SceneObjects* scenes = GetGameSceneObjects(OOT_GAME);

	for (size_t i = 0; i < OOT_NUM_SCENES; i++)
	{	// Browse all OoT scenes

		SceneObjects* currScene = &scenes[i];

		for (size_t j = 0; j < currScene->NumOfObjs; j++)
		{
			ObjectInfo* currObj = &currScene->Objects[j];

			switch (currObj->RenderType)
			{	
				case ObjectType::gs:
				{	// OoT gold skulltula shuffling

					switch (this->ROMSettings["goldSkulltulaTokens"])
					{
						case ShuffleSetting::vanilla:
						{
							Filter->ExcludeNewObject(currObj);
							break;
						}

						case ShuffleSetting::overworld:
						{
							if (currObj->LocationType != LocType::overworld)
							{
								Filter->ExcludeNewObject(currObj);
							}
							break;
						}

						case ShuffleSetting::dungeons:
						{
							if (currObj->LocationType != LocType::dungeon)
							{
								Filter->ExcludeNewObject(currObj);
							}
							break;
						}

						default:
							break;
					}
				}

				case ObjectType::none:
				default:
				{	// We don't want to overload the filter with non rendered object anyway

					break;
				}
			}

			
		}
	}
}

void Settings::ApplyMMSettingsToFilter(FilterManager* Filter)
{

}