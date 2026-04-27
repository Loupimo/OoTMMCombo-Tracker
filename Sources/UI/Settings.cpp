#include "UI/FilterManager.h"
#include "UI/AppConfig.h"
#include "UI/Settings.h"
#include "Combo/Objects.h"
#include "Combo/Scenes.h"



Settings::Settings()
{
	this->Game = ROMGame::ootmm;
	this->Mode = GameMode::single;
	this->NumOfTeams = 1;
	this->ROMSettings = QMap<QString, Parameter>({
				{ "goldSkulltulaTokens", { "Gold Skulltula - OoT", ParamType::shuffle, ShuffleSetting::all } },
				{ "housesSkulltulaTokens", { "Gold Skulltula - MM", ParamType::boolean, ShuffleSetting::all } },
				{ "tingleShuffle", { "Tingle Maps", ParamType::boolean, ShuffleSetting::all } },
				{ "mapCompassShuffle", { "Map / Compass", ParamType::boolean, ShuffleSetting::all } },
				{ "smallKeyShuffleOot", { "Small Key - OoT", ParamType::shuffle, ShuffleSetting::all } },
				{ "smallKeyShuffleMm", { "Small Key - MM", ParamType::shuffle, ShuffleSetting::all } },
				{ "smallKeyShuffleHideout", { "Small Key - Hideout", ParamType::boolean, ShuffleSetting::all } },
				{ "smallKeyShuffleChestGame", { "Small Key - Chest Game", ParamType::boolean, ShuffleSetting::all } },
				{ "bossKeyShuffleOot", { "Boss Key - OoT", ParamType::boolean, ShuffleSetting::all } },
				{ "bossKeyShuffleMm", { "Boss Key - MM", ParamType::boolean, ShuffleSetting::all } },
				{ "silverRupeeShuffle", { "Silver Rupee", ParamType::boolean, ShuffleSetting::all } },
				{ "townFairyShuffle", { "Town Fairy", ParamType::boolean, ShuffleSetting::all } },
				{ "strayFairyChestShuffle", { "Stray Fairy - Chest", ParamType::boolean, ShuffleSetting::all } },
				{ "strayFairyOtherShuffle", { "Stray Fairy - Other", ParamType::boolean, ShuffleSetting::all } },
				{ "ganonBossKey", { "Boss Key - Ganon", ParamType::boolean, ShuffleSetting::all } },
				{ "scrubShuffleOot", { "Scrub - OoT", ParamType::boolean, ShuffleSetting::all } },
				{ "scrubShuffleMm", { "Scrub - MM", ParamType::boolean, ShuffleSetting::all } },
				{ "cowShuffleOot", { "Cow - OoT", ParamType::boolean, ShuffleSetting::all } },
				{ "cowShuffleMm", { "Cow - MM", ParamType::boolean, ShuffleSetting::all } },
				{ "shopShuffleOot", { "Shop - OoT", ParamType::boolean, ShuffleSetting::all } },
				{ "shopShuffleMm", { "Shop - MM", ParamType::boolean, ShuffleSetting::all } },
				{ "owlShuffle", { "Owl", ParamType::boolean, ShuffleSetting::all } },
				{ "shufflePotsOot", { "Pots - OoT", ParamType::shuffle, ShuffleSetting::all } },
				{ "shufflePotsMm", { "Pots - MM", ParamType::shuffle, ShuffleSetting::all } },
				{ "shuffleCratesOot", { "Crates - OoT", ParamType::shuffle, ShuffleSetting::all } },
				{ "shuffleCratesMm", { "Crates - MM", ParamType::shuffle, ShuffleSetting::all } },
				{ "shuffleBarrelsMm", { "Barrels - MM", ParamType::shuffle, ShuffleSetting::all } },
				{ "shuffleHivesOot", { "Hives - OoT", ParamType::boolean, ShuffleSetting::all } },
				{ "shuffleHivesMm", { "Hives - MM", ParamType::boolean, ShuffleSetting::all } },
				{ "shuffleGrassOot", { "Grass - OoT", ParamType::shuffle, ShuffleSetting::all } },
				{ "shuffleRocksOot", { "Rocks - OoT", ParamType::boolean, ShuffleSetting::all } },
				{ "shuffleRocksMm", { "Rocks - MM", ParamType::shuffle, ShuffleSetting::all } },
				{ "shuffleGrassMm", { "Grass - MM", ParamType::shuffle, ShuffleSetting::all } },
				{ "shuffleTFGrassMm", { "Termina Field Grass", ParamType::boolean, ShuffleSetting::all } },
				{ "shuffleTreesOot", { "Trees - OoT", ParamType::boolean, ShuffleSetting::all } },
				{ "shuffleTreesMm", { "Trees - MM", ParamType::shuffle, ShuffleSetting::all } },
				{ "shuffleBushOot", { "Bush - OoT", ParamType::boolean, ShuffleSetting::all } },
				{ "shuffleBushMm", { "Bush - MM", ParamType::shuffle, ShuffleSetting::all } },
				{ "shuffleSoilOot", { "Soil - OoT", ParamType::boolean, ShuffleSetting::all } },
				{ "shuffleSoilMm", { "Soil - MM", ParamType::shuffle, ShuffleSetting::all } },
				{ "shuffleFreeRupeesOot", { "Freestanding Rupees - OoT", ParamType::shuffle, ShuffleSetting::all } },
				{ "shuffleFreeRupeesMm", { "Freestanding Rupees - MM", ParamType::shuffle, ShuffleSetting::all } },
				{ "shuffleFreeHeartsOot", { "Freestanding Hearts - OoT", ParamType::shuffle, ShuffleSetting::all } },
				{ "shuffleFreeHeartsMm", { "Freestanding Hearts - MM", ParamType::boolean, ShuffleSetting::all } },
				{ "shuffleWonderItemsOot", { "Wonder Items - OoT", ParamType::shuffle, ShuffleSetting::all } },
				{ "shuffleWonderItemsMm", { "Wonder Items - MM", ParamType::boolean, ShuffleSetting::all } },
				{ "shuffleSnowballsMm", { "Snowballs - MM", ParamType::shuffle, ShuffleSetting::all } },
				{ "shuffleButterfliesOot", { "Butterflies - OoT", ParamType::boolean, ShuffleSetting::all } },
				{ "shuffleButterfliesMm", { "Butterflies - MM", ParamType::boolean, ShuffleSetting::all } },
				{ "shuffleRedBouldersOot", { "Red Boulders - OoT", ParamType::boolean, ShuffleSetting::all } },
				{ "shuffleRedBouldersMm", { "Red Boulders - MM", ParamType::boolean, ShuffleSetting::all } },
				{ "shuffleFrogsRupeesOot", { "Frogs Rupees", ParamType::boolean, ShuffleSetting::all } },
				{ "shuffleIciclesOot", { "Icicles - OoT", ParamType::boolean, ShuffleSetting::all } },
				{ "shuffleIciclesMm", { "Icicles - MM", ParamType::boolean, ShuffleSetting::all } },
				{ "shuffleRedIceOot", { "Red Ice", ParamType::boolean, ShuffleSetting::all } },
				{ "shuffleOcarinasOot", { "Ocarinas", ParamType::boolean, ShuffleSetting::all } },
				{ "shuffleMasterSword", { "Master Sword", ParamType::boolean, ShuffleSetting::all } },
				{ "shuffleGerudoCard", { "Gerudo Card", ParamType::boolean, ShuffleSetting::all } },
				{ "shuffleMaskTrades", { "Mask Trades", ParamType::boolean, ShuffleSetting::all } },
				{ "shuffleMerchantsOot", { "Merchants - OoT", ParamType::boolean, ShuffleSetting::all } },
				{ "shuffleMerchantsMm", { "Merchants - MM", ParamType::boolean, ShuffleSetting::all } },
				{ "pondFishShuffle", { "Pond Fish", ParamType::boolean, ShuffleSetting::all } },
				{ "divingGameRupeeShuffle", { "Diving Game Rupees", ParamType::boolean, ShuffleSetting::all } },
				{ "fairyFountainFairyShuffleOot", { "Fairy Fountain - OoT", ParamType::boolean, ShuffleSetting::all } },
				{ "fairyFountainFairyShuffleMm", { "Fairy Fountain - MM", ParamType::boolean, ShuffleSetting::all } },
				{ "fairySpotShuffleOot", { "Fairy Spot", ParamType::boolean, ShuffleSetting::all } },
				{ "eggShuffle", { "Egg Content", ParamType::boolean, ShuffleSetting::all } },
				{ "restoreBrokenActors", { "Restore Broken Actors", ParamType::boolean, ShuffleSetting::all } },
				{ "shuffleLotteryMm", { "Lottery Prizes", ParamType::boolean, ShuffleSetting::all } }
	});
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
	// Load number of parameters
	qsizetype numParams = 0;
	memcpy_s(&numParams, sizeof(numParams), Data->data() + Offset, sizeof(numParams));
	Offset += sizeof(numParams);
	return Offset;
}


void Settings::ParseSettings(QString& SettingsSection)
{	// Split with "===" to find the right section

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
	GetSceneMetaInfo(OOT_DEKU_TREE, OOT_GAME)->ActiveLayout = GameLayout::oot;
	GetSceneMetaInfo(OOT_DODONGO_CAVERN, OOT_GAME)->ActiveLayout = GameLayout::oot;
	GetSceneMetaInfo(OOT_INSIDE_JABU_JABU, OOT_GAME)->ActiveLayout = GameLayout::oot;
	GetSceneMetaInfo(OOT_TEMPLE_FOREST, OOT_GAME)->ActiveLayout = GameLayout::oot;
	GetSceneMetaInfo(OOT_TEMPLE_FIRE, OOT_GAME)->ActiveLayout = GameLayout::oot;
	GetSceneMetaInfo(OOT_TEMPLE_WATER, OOT_GAME)->ActiveLayout = GameLayout::oot;
	GetSceneMetaInfo(OOT_TEMPLE_SHADOW, OOT_GAME)->ActiveLayout = GameLayout::oot;
	GetSceneMetaInfo(OOT_TEMPLE_SPIRIT, OOT_GAME)->ActiveLayout = GameLayout::oot;
	GetSceneMetaInfo(OOT_BOTTOM_OF_THE_WELL, OOT_GAME)->ActiveLayout = GameLayout::oot;
	GetSceneMetaInfo(OOT_GERUDO_TRAINING_GROUND, OOT_GAME)->ActiveLayout = GameLayout::oot;
	GetSceneMetaInfo(OOT_ICE_CAVERN, OOT_GAME)->ActiveLayout = GameLayout::oot;
	GetSceneMetaInfo(OOT_INSIDE_GANON_CASTLE, OOT_GAME)->ActiveLayout = GameLayout::oot;

	GetSceneMetaInfo(MM_GROTTOS, MM_GAME)->ActiveLayout = GameLayout::mm;
	GetSceneMetaInfo(MM_DEKU_PALACE, MM_GAME)->ActiveLayout = GameLayout::mm;
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

					GetSceneMetaInfo(OOT_DEKU_TREE, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					GetSceneMetaInfo(OOT_DODONGO_CAVERN, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					GetSceneMetaInfo(OOT_INSIDE_JABU_JABU, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					GetSceneMetaInfo(OOT_TEMPLE_FOREST, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					GetSceneMetaInfo(OOT_TEMPLE_FIRE, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					GetSceneMetaInfo(OOT_TEMPLE_WATER, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					GetSceneMetaInfo(OOT_TEMPLE_SHADOW, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					GetSceneMetaInfo(OOT_TEMPLE_SPIRIT, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					GetSceneMetaInfo(OOT_BOTTOM_OF_THE_WELL, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					GetSceneMetaInfo(OOT_ICE_CAVERN, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					GetSceneMetaInfo(OOT_GERUDO_TRAINING_GROUND, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					GetSceneMetaInfo(OOT_INSIDE_GANON_CASTLE, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
				}
			}
			else
			{	// Majora's mask

				if (layoutParams.at(1) == "all")
				{	// All JP

					GetSceneMetaInfo(MM_GROTTOS, MM_GAME)->ActiveLayout = GameLayout::mm_jp;
					GetSceneMetaInfo(MM_DEKU_PALACE, MM_GAME)->ActiveLayout = GameLayout::mm_jp;
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
						GetSceneMetaInfo(OOT_DEKU_TREE, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					}
					else if (currScene == "Dodongo cavern")
					{
						GetSceneMetaInfo(OOT_DODONGO_CAVERN, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					}
					else if (currScene == "Jabu-Jabu")
					{
						GetSceneMetaInfo(OOT_INSIDE_JABU_JABU, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					}
					else if (currScene == "Forest Temple")
					{
						GetSceneMetaInfo(OOT_TEMPLE_FOREST, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					}
					else if (currScene == "Fire Temple")
					{
						GetSceneMetaInfo(OOT_TEMPLE_FIRE, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					}
					else if (currScene == "Water Temple")
					{
						GetSceneMetaInfo(OOT_TEMPLE_WATER, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					}
					else if (currScene == "Shadow Temple")
					{
						GetSceneMetaInfo(OOT_TEMPLE_SHADOW, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					}
					else if (currScene == "Spirit Temple")
					{
						GetSceneMetaInfo(OOT_TEMPLE_SPIRIT, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					}
					else if (currScene == "Bottom of the Well")
					{
						GetSceneMetaInfo(OOT_BOTTOM_OF_THE_WELL, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					}
					else if (currScene == "Ice Cavern")
					{
						GetSceneMetaInfo(OOT_ICE_CAVERN, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					}
					else if (currScene == "Gerudo Training Grounds")
					{
						GetSceneMetaInfo(OOT_GERUDO_TRAINING_GROUND, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
					}
					else if (currScene == "Ganon's Castle")
					{
						GetSceneMetaInfo(OOT_INSIDE_GANON_CASTLE, OOT_GAME)->ActiveLayout = GameLayout::oot_mq;
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

						GetSceneMetaInfo(MM_GROTTOS, MM_GAME)->ActiveLayout = GameLayout::mm_jp;
						GetSceneMetaInfo(MM_DEKU_PALACE, MM_GAME)->ActiveLayout = GameLayout::mm_jp;
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
	else if (Name == "mode")
	{
		if (Value == "coop")
		{
			this->Mode = GameMode::coop;
		}
		else if (Value == "mm")
		{
			this->Mode = GameMode::multi;
		}
		else
		{
			this->Mode = GameMode::single;
		}
	}
	else if (Name == "teams")
	{
		this->NumOfTeams = (size_t)atoi(Value.toStdString().c_str());
	}
	else if (this->ROMSettings.contains(Name))
	{
		ShuffleSetting setting;

		if (Value == "all" || Value == "true" || Value == "full" || Value == "anywhere" || Value == "starting" || Value == "removed" || Value == "cross")
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

		this->ROMSettings[Name].Value = setting;
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
	SceneObjects* scenes = this->ApplySettingsToFilter(Filter, OOT_GAME);

	for (size_t i = 0; i < OOT_NUM_SCENES; i++)
	{	// Browse all OoT scenes

		SceneObjects* currScene = &scenes[i];

		for (size_t j = 0; j < currScene->NumOfObjs; j++)
		{
			ObjectInfo* currObj = &currScene->Objects[j];

			switch (currObj->RenderType)
			{	
				case ObjectType::gs:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["goldSkulltulaTokens"].Value, Filter);
					break;
				}

				case ObjectType::map:
				case ObjectType::compass:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["mapCompassShuffle"].Value, Filter);
					break;
				}

				case ObjectType::small_key:
				{
					if (currObj->Scene == OOT_THIEVES_HIDEOUT)
					{	// Hideout small key

						this->CheckObjectExclusion(currObj, this->ROMSettings["smallKeyShuffleHideout"].Value, Filter);
					}
					else if (currObj->LocationType == LocType::minigame)
					{	// Treasure chest small key

						this->CheckObjectExclusion(currObj, this->ROMSettings["smallKeyShuffleChestGame"].Value, Filter);
					}
					else
					{	// Dungeon small key

						this->CheckObjectExclusion(currObj, this->ROMSettings["smallKeyShuffleOot"].Value, Filter);
					}
					break;
				}

				case ObjectType::boss_key:
				{
					if (currObj->Scene == OOT_GANON_TOWER)
					{	// Ganon boss key

						this->CheckObjectExclusion(currObj, this->ROMSettings["ganonBossKey"].Value, Filter);
					}
					else
					{	// Other boss key

						this->CheckObjectExclusion(currObj, this->ROMSettings["bossKeyShuffleOot"].Value, Filter);
					}
					break;
				}

				case ObjectType::sr:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["silverRupeeShuffle"].Value, Filter);
					break;
				}

				case ObjectType::scrub:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["scrubShuffleOot"].Value, Filter);
					break;
				}

				case ObjectType::cow:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["cowShuffleOot"].Value, Filter);
					break;
				}

				case ObjectType::shop:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["shopShuffleOot"].Value, Filter);
					break;
				}

				case ObjectType::pot:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["shufflePotsOot"].Value, Filter);
					break;
				}

				case ObjectType::crate:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["shuffleCratesOot"].Value, Filter);
					break;
				}

				case ObjectType::hive:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["shuffleHivesOot"].Value, Filter);
					break;
				}

				case ObjectType::grass:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["shuffleGrassOot"].Value, Filter);
					break;
				}

				case ObjectType::rock:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["shuffleRocksOot"].Value, Filter);
					break;
				}

				case ObjectType::tree:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["shuffleTreesOot"].Value, Filter);
					break;
				}

				case ObjectType::bush:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["shuffleBushOot"].Value, Filter);
					break;
				}

				case ObjectType::soil:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["shuffleSoilOot"].Value, Filter);
					break;
				}

				case ObjectType::rupee:
				{
					if (currObj->LocationType == LocType::minigame)
					{	// Diving game rupee

						this->CheckObjectExclusion(currObj, this->ROMSettings["divingGameRupeeShuffle"].Value, Filter);
					}
					else
					{	// Normal freestanding rupee

						this->CheckObjectExclusion(currObj, this->ROMSettings["shuffleFreeRupeesOot"].Value, Filter);
					}
					break;
				}

				case ObjectType::heart:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["shuffleFreeHeartsOot"].Value, Filter);
					break;
				}

				case ObjectType::wonder:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["shuffleWonderItemsOot"].Value, Filter);
					break;
				}

				case ObjectType::butterfly:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["shuffleButterfliesOot"].Value, Filter);
					break;
				}

				case ObjectType::redboulder:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["shuffleRedBouldersOot"].Value, Filter);
					break;
				}

				case ObjectType::icicle:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["shuffleIciclesOot"].Value, Filter);
					break;
				}

				case ObjectType::redice:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["shuffleRedIceOot"].Value, Filter);
					break;
				}
				
				case ObjectType::ocarina:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["shuffleOcarinasOot"].Value, Filter);
					break;
				}

				case ObjectType::sword:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["shuffleMasterSword"].Value, Filter);
					break;
				}

				case ObjectType::mask:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["shuffleMaskTrades"].Value, Filter);
					break;
				}

				case ObjectType::merchant:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["shuffleMerchantsOot"].Value, Filter);
					break;
				}

				case ObjectType::fish:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["pondFishShuffle"].Value, Filter);
					break;
				}

				case ObjectType::fairy:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["fairyFountainFairyShuffleOot"].Value, Filter);
					break;
				}

				case ObjectType::fairy_spot:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["fairySpotShuffleOot"].Value, Filter);
					break;
				}

				case ObjectType::egg:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["eggShuffle"].Value, Filter);
					break;
				}

				case ObjectType::npc:
				{
					switch (currObj->MapIcon)
					{
						case EGameIcon::card:
						{	// Check for gerudo card

							this->CheckObjectExclusion(currObj, this->ROMSettings["shuffleGerudoCard"].Value, Filter);
							break;
						}

						case EGameIcon::frog:
						{	// Check for frog rupees rewards

							if (currObj->LocationType == LocType::minigame)
							{
								this->CheckObjectExclusion(currObj, this->ROMSettings["shuffleFrogsRupeesOot"].Value, Filter);
							}
							break;
						}
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

	if (this->ROMSettings["restoreBrokenActors"].Value != ShuffleSetting::all)
	{	// Restore broken actor is not active. Hide broken actors

		uint32_t sceneToCheck[3] = { OOT_DODONGO_CAVERN, OOT_HYRULE_CASTLE, OOT_LAKE_HYLIA };

		for (uint32_t i = 0; i < 3; i++)
		{	// Browse all scenes that contains broken actors

			for (uint32_t j = 0; j < scenes[sceneToCheck[i]].NumOfObjs; j++)
			{	// Find all broken actors

				ObjectInfo* currObj = &scenes[sceneToCheck[i]].Objects[j];

				if (currObj->LocationType == LocType::broken)
				{	// The actor is broken

					Filter->ExcludeNewObject(currObj);
				}
			}
		}
	}
}

void Settings::ApplyMMSettingsToFilter(FilterManager* Filter)
{
	SceneObjects* scenes = this->ApplySettingsToFilter(Filter, MM_GAME);

	for (size_t i = 0; i < MM_NUM_SCENES; i++)
	{	// Browse all OoT scenes

		SceneObjects* currScene = &scenes[i];

		for (size_t j = 0; j < currScene->NumOfObjs; j++)
		{
			ObjectInfo* currObj = &currScene->Objects[j];

			switch (currObj->RenderType)
			{
				case ObjectType::gs:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["housesSkulltulaTokens"].Value, Filter);
					break;
				}

				case ObjectType::map:
				{
					if (currObj->Type == ObjectType::map)
					{	// Tingle maps

						this->CheckObjectExclusion(currObj, this->ROMSettings["tingleShuffle"].Value, Filter);
						break;
					}
				}
				case ObjectType::compass:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["mapCompassShuffle"].Value, Filter);
					break;
				}

				case ObjectType::small_key:
				{	// Dungeon small key

					this->CheckObjectExclusion(currObj, this->ROMSettings["smallKeyShuffleMm"].Value, Filter);
					break;
				}

				case ObjectType::boss_key:
				{	// Boss key

					this->CheckObjectExclusion(currObj, this->ROMSettings["bossKeyShuffleMm"].Value, Filter);
					break;
				}

				case ObjectType::sf:
				{
					if (currObj->ObjectID == STRAY_FAIRY_TOWN)
					{	// Stray fairy town

						this->CheckObjectExclusion(currObj, this->ROMSettings["townFairyShuffle"].Value, Filter);
					}
					else if (currObj->Type == ObjectType::chest)
					{	// Stray fairy in a chest

						this->CheckObjectExclusion(currObj, this->ROMSettings["strayFairyChestShuffle"].Value, Filter);
					}
					else
					{	// All other stray fairies

						this->CheckObjectExclusion(currObj, this->ROMSettings["strayFairyOtherShuffle"].Value, Filter);
					}
					break;
				}

				case ObjectType::scrub:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["scrubShuffleMm"].Value, Filter);
					break;
				}

				case ObjectType::cow:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["cowShuffleMm"].Value, Filter);
					break;
				}

				case ObjectType::shop:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["shopShuffleMm"].Value, Filter);
					break;
				}

				case ObjectType::owl:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["owlShuffle"].Value, Filter);
					break;
				}

				case ObjectType::pot:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["shufflePotsMm"].Value, Filter);
					break;
				}

				case ObjectType::crate:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["shuffleCratesMm"].Value, Filter);
					break;
				}

				case ObjectType::barrel:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["shuffleBarrelsMm"].Value, Filter);
					break;
				}

				case ObjectType::hive:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["shuffleHivesMm"].Value, Filter);
					break;
				}

				case ObjectType::rock:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["shuffleRocksMm"].Value, Filter);
					break;
				}

				case ObjectType::grass:
				{
					if (currObj->Scene == MM_TERMINA_FIELD)
					{	// Termina field grass

						this->CheckObjectExclusion(currObj, this->ROMSettings["shuffleTFGrassMm"].Value, Filter);
					}
					else
					{	// All other grass

						this->CheckObjectExclusion(currObj, this->ROMSettings["shuffleGrassMm"].Value, Filter);
					}
					break;
				}

				case ObjectType::tree:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["shuffleTreesMm"].Value, Filter);
					break;
				}

				case ObjectType::bush:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["shuffleBushMm"].Value, Filter);
					break;
				}

				case ObjectType::soil:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["shuffleSoilMm"].Value, Filter);
					break;
				}

				case ObjectType::rupee:
				{	// Normal freestanding rupee

					this->CheckObjectExclusion(currObj, this->ROMSettings["shuffleFreeRupeesMm"].Value, Filter);
					break;
				}

				case ObjectType::heart:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["shuffleFreeHeartsMm"].Value, Filter);
					break;
				}

				case ObjectType::wonder:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["shuffleWonderItemsMm"].Value, Filter);
					break;
				}

				case ObjectType::snowball:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["shuffleSnowballsMm"].Value, Filter);
					break;
				}

				case ObjectType::butterfly:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["shuffleButterfliesMm"].Value, Filter);
					break;
				}

				case ObjectType::redboulder:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["shuffleRedBouldersMm"].Value, Filter);
					break;
				}

				case ObjectType::icicle:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["shuffleIciclesMm"].Value, Filter);
					break;
				}

				case ObjectType::merchant:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["shuffleMerchantsMm"].Value, Filter);
					break;
				}

				case ObjectType::fairy:
				{
					this->CheckObjectExclusion(currObj, this->ROMSettings["fairyFountainFairyShuffleMm"].Value, Filter);
					break;
				}

				case ObjectType::npc:
				{
					if (currObj->Scene == MM_LOTTERY)
					{	// Lottery

						this->CheckObjectExclusion(currObj, this->ROMSettings["shuffleLotteryMm"].Value, Filter);
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

void Settings::CheckObjectExclusion(ObjectInfo* ToCheck, ShuffleSetting SettingValue, FilterManager* Filter)
{
	switch (SettingValue)
	{
		case ShuffleSetting::vanilla:
		{
			Filter->ExcludeNewObject(ToCheck);
			break;
		}

		case ShuffleSetting::overworld:
		{
			if (ToCheck->LocationType != LocType::overworld)
			{
				Filter->ExcludeNewObject(ToCheck);
			}
			break;
		}

		case ShuffleSetting::dungeons:
		{
			if (ToCheck->LocationType != LocType::dungeon)
			{
				Filter->ExcludeNewObject(ToCheck);
			}
			break;
		}

		default:
			break;
	}
}


SceneObjects* Settings::ApplySettingsToFilter(FilterManager* Filter, uint32_t Game)
{
	Filter->ResetExcludedObject();
	return GetGameSceneObjects(Game);
}
