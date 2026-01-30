#include "UI/FilterManager.h"
#include "UI/Settings.h"



Settings::Settings()
{
	this->Game = ROMGame::ootmm;
	this->ROMSettings = QMap<QString, ShuffleSetting>();
}


Settings::~Settings()
{
	this->ROMSettings.clear();
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

		if (Value == "none" || Value == "false")
		{
			setting = ShuffleSetting::none;
		}
		else if (Value == "all" || Value == "true" || Value == "anywhere" || Value == "starting" || Value == "removed")
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


				case ObjectType::none:
				default:
				{	// We don't want to overload the filter with non rendered object anyway

					break;
				}
			}

			switch (this->ROMSettings["songs"])
			{
			case ShuffleSetting::vanilla:
			{

				break;
			}

			default:
				break;
			}
		}
	}
}

void Settings::ApplyMMSettingsToFilter(FilterManager* Filter)
{

}