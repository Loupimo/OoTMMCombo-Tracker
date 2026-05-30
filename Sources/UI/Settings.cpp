#include "UI/FilterManager.h"
#include "UI/AppConfig.h"
#include "UI/Settings.h"
#include "Combo/Objects.h"
#include "Combo/Scenes.h"



Settings::Settings()
{
	this->Game = ROMGame::ootmm;
	this->Mode = GameMode::single;
    this->Goal = GoalMode::boss;
	this->NumOfTeams = 1;
	this->LocalWorld = 1;
    this->FilterSettings = QMap<QString, Parameter>({
        { "songs", { "Song Notes", ParamType::boolean, ShuffleSetting::vanilla } },
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
        { "skipZelda", { "Skip Zelda", ParamType::boolean, ShuffleSetting::vanilla } },
        { "restoreBrokenActors", { "Restore Broken Actors", ParamType::boolean, ShuffleSetting::all } },
        { "shuffleLotteryMm", { "Lottery Prizes", ParamType::boolean, ShuffleSetting::all } },
    });

    this->ItemSettings = QMap<QString, Parameter>({
        { "progressiveShieldsOot", { "Progressive Shields - OoT", ParamType::boolean, ShuffleSetting::vanilla } },
        { "progressiveSwordsOot", { "Progressive Swords - OoT", ParamType::boolean, ShuffleSetting::vanilla } },
        { "progressiveShieldsMm", { "Progressive Shields - Mm", ParamType::boolean, ShuffleSetting::vanilla } },
        { "progressiveGFS", { "Progressive Great Fairy Sword", ParamType::boolean, ShuffleSetting::vanilla } },
        { "progressiveClocks", { "Progressive Clock", ParamType::boolean, ShuffleSetting::all } },
        { "sticksNutsUpgradesMm", { "Deku Sticks / Nuts Upgrades - MM", ParamType::boolean, ShuffleSetting::all } },
        { "sunSongMm", { "Sun's Song - MM", ParamType::boolean, ShuffleSetting::all } },
        { "fairyOcarinaMm", { "Fairy Ocarina - MM", ParamType::boolean, ShuffleSetting::all } },
        { "shortHookshotMm", { "Short Hookshot - MM", ParamType::boolean, ShuffleSetting::all } },
        { "bronzeScale", { "Bonze Scale", ParamType::boolean, ShuffleSetting::all } },
        { "childWallets", { "Child Wallet", ParamType::boolean, ShuffleSetting::all } },
        { "colossalWallets", { "Colossal Wallet", ParamType::boolean, ShuffleSetting::all } },
        { "bottomlessWallets", { "Bottomless Wallet", ParamType::boolean, ShuffleSetting::all } },
        { "skeletonKeyOot", { "Skeleton Key - OoT", ParamType::boolean, ShuffleSetting::all } },
        { "skeletonKeyMm", { "Skeleton Key - MM", ParamType::boolean, ShuffleSetting::all } },
        { "magicalRupee", { "Magical Rupee", ParamType::boolean, ShuffleSetting::all } },
        { "transcendentFairy", { "Transcendent Fairy", ParamType::boolean, ShuffleSetting::all } },
        { "platinumTokenOot", { "Platinum Token - OoT", ParamType::boolean, ShuffleSetting::all } },
        { "platinumTokenMm", { "Platinum Token - MM", ParamType::boolean, ShuffleSetting::all } },
        { "spellFireMm", { "Din's Fire - MM", ParamType::boolean, ShuffleSetting::all } },
        { "spellWindMm", { "Farore's Wind - MM", ParamType::boolean, ShuffleSetting::all } },
        { "spellLoveMm", { "Nayru's Love - MM", ParamType::boolean, ShuffleSetting::all } },
        { "bootsIronMm", { "Iron Boots - MM", ParamType::boolean, ShuffleSetting::all } },
        { "bootsHoverMm", { "Hover Boots - MM", ParamType::boolean, ShuffleSetting::all } },
        { "tunicGoronMm", { "Goron Tunic - MM", ParamType::boolean, ShuffleSetting::all } },
        { "tunicZoraMm", { "Zora Tunic - MM", ParamType::boolean, ShuffleSetting::all } },
        { "scalesMm", { "Scales - MM", ParamType::boolean, ShuffleSetting::all } },
        { "strengthMm", { "Strength - MM", ParamType::boolean, ShuffleSetting::all } },
        { "hammerMm", { "Megaton Hammer - MM", ParamType::boolean, ShuffleSetting::all } },
        { "spinUpgradeOot", { "Spin Attack Upgrade - OoT", ParamType::boolean, ShuffleSetting::all } },
        { "dekuShieldMm", { "Deku Shield - MM", ParamType::boolean, ShuffleSetting::all } },
        { "blastMaskOot", { "Blast Mask - OoT", ParamType::boolean, ShuffleSetting::all } },
        { "stoneMaskOot", { "Stone Mask - OoT", ParamType::boolean, ShuffleSetting::all } },
        { "elegyOot", { "Elegy of Emptiness - OoT", ParamType::boolean, ShuffleSetting::all } },
        { "ocarinaButtonsShuffleOot", { "Ocarina Buttons Shuffle - OoT", ParamType::boolean, ShuffleSetting::all } },
        { "ocarinaButtonsShuffleMm", { "Ocarina Buttons Shuffle - MM", ParamType::boolean, ShuffleSetting::all } },
        { "soulsEnemyOot", { "Enemy Souls - OoT", ParamType::boolean, ShuffleSetting::all } },
        { "soulsEnemyMm", { "Enemy Souls - MM", ParamType::boolean, ShuffleSetting::all } },
        { "soulsBossOot", { "Boss Souls - OoT", ParamType::boolean, ShuffleSetting::all } },
        { "soulsBossMm", { "Boss Souls - MM", ParamType::boolean, ShuffleSetting::all } },
        { "soulsNpcOot", { "NPC Souls - OoT", ParamType::boolean, ShuffleSetting::all } },
        { "soulsNpcMm", { "NPC Souls - MM", ParamType::boolean, ShuffleSetting::all } },
        { "soulsAnimalOot", { "Animal Souls - OoT", ParamType::boolean, ShuffleSetting::all } },
        { "soulsAnimalMm", { "Animal Souls - MM", ParamType::boolean, ShuffleSetting::all } },
        { "soulsMiscOot", { "Misc Souls - OoT", ParamType::boolean, ShuffleSetting::all } },
        { "soulsMiscMm", { "Misc Souls - MM", ParamType::boolean, ShuffleSetting::all } },
        { "clocks", { "Clocks Shuffle", ParamType::boolean, ShuffleSetting::all } },
        { "coins", { "Coins", ParamType::boolean, ShuffleSetting::all } },
        { "coinsRed", { "Red Coins", ParamType::uint, ShuffleSetting::all } },
        { "coinsGreen", { "Green Coins", ParamType::uint, ShuffleSetting::all } },
        { "coinsBlue", { "Blue Coins", ParamType::uint, ShuffleSetting::all } },
        { "coinsYellow", { "Yellow Coins", ParamType::uint, ShuffleSetting::all } },
        { "trapIce", { "Ice Trap", ParamType::boolean, ShuffleSetting::all } },
        { "trapFire", { "Fire Trap", ParamType::boolean, ShuffleSetting::all } },
        { "trapShock", { "Shock Trap", ParamType::boolean, ShuffleSetting::all } },
        { "trapDrain", { "Drain Trap", ParamType::boolean, ShuffleSetting::all } },
        { "trapAntiMagic", { "Magic Trap", ParamType::boolean, ShuffleSetting::all } },
        { "trapKnockback", { "Knockback Trap", ParamType::boolean, ShuffleSetting::all } },
        { "trapRupoor", { "Rupoor", ParamType::boolean, ShuffleSetting::all } },
        { "sharedStoneAgony", { "Shared Stone of Agony", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedSpinUpgrade", { "Shared Spin of Upgrade", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedNutsSticks", { "Shared Nuts / Sticks", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedBows", { "Shared Bows", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedBombBags", { "Shared Bomb Bags", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedMagic", { "Shared Magic", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedMagicArrowFire", { "Shared Fire Arrow", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedMagicArrowIce", { "Shared Ice Arrow", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedMagicArrowLight", { "Shared Light Arrow", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedSongEpona", { "Shared Epona's Song", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedSongStorms", { "Shared Song of Storms", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedSongTime", { "Shared Song of Time", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedSongSun", { "Shared Sun's Sung", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedHookshot", { "Shared Hookshot", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedLens", { "Shared Lens of Truth", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedOcarina", { "Shared Ocarina", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedMaskGoron", { "Shared Goron Mask", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedMaskZora", { "Shared Zora Mask", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedMaskBunny", { "Shared Bunny Mask", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedMaskKeaton", { "Shared Keaton Mask", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedMaskTruth", { "Shared Truth Mask", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedMaskBlast", { "Shared Blast Mask", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedMaskStone", { "Shared Stone Mask", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedSongElegy", { "Shared Elegy of Emptiness", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedWallets", { "Shared Wallets", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedHealth", { "Shared Health", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedShields", { "Shared Shields", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedSoulsEnemy", { "Shared Souls of Enemy", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedSoulsNpc", { "Shared Souls of NPC", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedSoulsAnimal", { "Shared Souls of Animal", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedSoulsMisc", { "Shared Misc Souls", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedOcarinaButtons", { "Shared Ocarina Buttons", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedSkeletonKey", { "Shared Skeleton Key", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedPlatinumToken", { "Shared Platinum Token", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedSpellFire", { "Shared Din's Fire", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedSpellWind", { "Shared Farore's Wind", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedSpellLove", { "Shared Nayru's Love", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedBootsIron", { "Shared Iron Boots", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedBootsHover", { "Shared Hover Boots", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedTunicGoron", { "Shared Goron Tunic", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedTunicZora", { "Shared Zora Tunic", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedScales", { "Shared Scales", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedStrength", { "Shared Strength", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedHammer", { "Shared Megaton Hammer", ParamType::boolean, ShuffleSetting::vanilla } },
        { "sharedBottles", { "Shared Bottles", ParamType::boolean, ShuffleSetting::vanilla } },
        { "crossWarpOot", { "Cross-Games OoT Warp Song", ParamType::boolean, ShuffleSetting::all } },
        { "crossWarpMm", { "Cross-Games MM Song of Soaring", ParamType::boolean, ShuffleSetting::all } }
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
	qsizetype numOfParams = this->FilterSettings.size();
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

			this->ParseWorldFlags(currSection);
		}
        else if (currSection.startsWith("Starting Items"))
        {   // Starting items

            this->ParseStartingItems(currSection);
        }
	}
}

void Settings::ParseStartingItems(QString& LayoutSection)
{
    QRegularExpression reg("^  (.+: .+\n)", QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator it = reg.globalMatch(LayoutSection);

    while (it.hasNext())
    {
        QRegularExpressionMatch match = it.next();
        QStringList layoutParams = match.captured(1).split(": ");

        if (layoutParams.size() == 2)
        {
            uint32_t itemID = FindItemByName(layoutParams.at(0))->ItemID;

            if (this->StartingItemIDs.contains(itemID))
            {
                this->StartingItemIDs.find(itemID).value() += layoutParams.at(1).toInt();
            }
            else
            {
                this->StartingItemIDs.insert(itemID, layoutParams.at(1).toInt());
            }
        }
    }
}

void Settings::ParseWorldFlags(QString& LayoutSection)
{
    this->ParseKeyRings(LayoutSection);
    this->ParseSilverPouches(LayoutSection);
    this->ParsePreActivatedOwl(LayoutSection);
    this->ParseGamesLayouts(LayoutSection);
}


void Settings::ParseKeyRings(QString& LayoutSection)
{
    QRegularExpression reg("^  ((?:Small Key Ring .+):(?: \\w*|(?:\n    - .+)*))", QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator it = reg.globalMatch(LayoutSection);

    for (uint32_t i = OOT_KEY_RING_FOREST; i <= OOT_KEY_RING_GTG; i++)
    {   // Disable all OoT key rings by default

        this->DisabledItemIDs.insert(i);
    }

    this->DisabledItemIDs.insert(OOT_KEY_RING_TCG); // Don't forget the treasure chest game

    for (uint32_t i = MM_KEY_RING_WF; i <= MM_KEY_RING_ST; i++)
    {   // Disable all MM key rings by default

        this->DisabledItemIDs.insert(i);
    }

    while (it.hasNext())
    {
        QRegularExpressionMatch match = it.next();
        QStringList layoutParams = match.captured(1).split("\n");

        if (layoutParams.size() == 1)
        {	// None or all

            layoutParams = layoutParams.at(0).split(": ");
            if (layoutParams.at(0) == "Small Key Ring (OoT)")
            {	// Ocarina of Time

                if (layoutParams.at(1) == "all")
                {	// All key rings

                    for (uint32_t i = OOT_SMALL_KEY_FOREST; i <= OOT_SMALL_KEY_GTG; i++)
                    {   // Disable all OoT small keys

                        this->DisabledItemIDs.insert(i);
                    }

                    for (uint32_t i = OOT_KEY_RING_FOREST; i <= OOT_KEY_RING_GTG; i++)
                    {   // Enable all OoT key rings

                        this->DisabledItemIDs.remove(i);
                    }

                    if (this->FilterSettings["smallKeyShuffleChestGame"].Value != ShuffleSetting::vanilla)
                    {
                        this->DisabledItemIDs.insert(OOT_SMALL_KEY_TCG);
                        this->DisabledItemIDs.remove(OOT_KEY_RING_TCG);
                    }

                }
            }
            else if (layoutParams.at(0) == "Small Key Ring (MM)")
            {	// Majora's Mask

                if (layoutParams.at(1) == "all")
                {	// All key rings

                    for (uint32_t i = MM_SMALL_KEY_WF; i <= MM_SMALL_KEY_ST; i++)
                    {   // Disable all MM small keys

                        this->DisabledItemIDs.insert(i);
                    }

                    for (uint32_t i = MM_KEY_RING_WF; i <= MM_KEY_RING_ST; i++)
                    {   // Enable all MM key rings

                        this->DisabledItemIDs.remove(i);
                    }
                }
            }
        }
        else
        {
            if (layoutParams.at(0) == "Small Key Ring (OoT):")
            {	// Ocarina of Time

                QMap<QString, QPair<uint32_t, uint32_t>> possibleChoice =
                {
                    { "Forest Temple", { OOT_SMALL_KEY_FOREST, OOT_KEY_RING_FOREST } },
                    { "Fire Temple", { OOT_SMALL_KEY_FIRE, OOT_KEY_RING_FIRE } },
                    { "Water Temple", { OOT_SMALL_KEY_WATER, OOT_KEY_RING_WATER } },
                    { "Shadow Temple", { OOT_SMALL_KEY_SHADOW, OOT_KEY_RING_SHADOW } },
                    { "Spirit Temple", { OOT_SMALL_KEY_SPIRIT, OOT_KEY_RING_SPIRIT } },
                    { "Bottom of the Well", { OOT_SMALL_KEY_BOTW, OOT_KEY_RING_BOTW } },
                    { "Gerudo Training Grounds", { OOT_SMALL_KEY_GTG, OOT_KEY_RING_GTG } },
                    { "Ganon's Castle", { OOT_SMALL_KEY_GANON, OOT_KEY_RING_GANON } },
                    { "Hideout", { OOT_SMALL_KEY_GF, OOT_KEY_RING_GF } },
                    { "Chest Game", { OOT_SMALL_KEY_TCG, OOT_KEY_RING_TCG } }
                };

                for (qsizetype i = 1; i < layoutParams.size(); i++)
                {
                    QString currKey = layoutParams.at(i);
                    currKey = currKey.replace("    - ", "");

                    auto choice = possibleChoice.find(currKey);
                    if (choice != possibleChoice.end())
                    {
                        this->DisabledItemIDs.insert(choice.value().first); // Disable small keys
                        this->DisabledItemIDs.remove(choice.value().second);  // Enable key ring
                    }
                }
            }
            else if (layoutParams.at(0) == "Small Key Ring (MM):")
            {   // Majora's Mask

                QMap<QString, QPair<uint32_t, uint32_t>> possibleChoice =
                {
                    { "Woodfall Temple", { MM_SMALL_KEY_WF, MM_KEY_RING_WF } },
                    { "Snowhead Temple", { MM_SMALL_KEY_SH, MM_KEY_RING_SH } },
                    { "Great Bay Temple", { MM_SMALL_KEY_GB, MM_KEY_RING_GB } },
                    { "Stone Tower Temple", { MM_SMALL_KEY_ST, MM_KEY_RING_ST } }
                };

                for (qsizetype i = 1; i < layoutParams.size(); i++)
                {
                    QString currKey = layoutParams.at(i);
                    currKey = currKey.replace("    - ", "");

                    auto choice = possibleChoice.find(currKey);
                    if (choice != possibleChoice.end())
                    {
                        this->DisabledItemIDs.insert(choice.value().first); // Disable small keys
                        this->DisabledItemIDs.remove(choice.value().second);  // Enable key ring
                    }
                }
            }
        }
    }

    // Note: the 'removed' / 'vanilla' override for the OoT/MM/ChestGame small keys + key rings is
    // applied by ApplyItemSettings (called later from Settings::ApplySettings), so it survives the
    // SettingsTab Apply path too — ParseKeyRings only runs on spoiler load.
}


void Settings::ParseSilverPouches(QString& LayoutSection)
{
    QRegularExpression reg("^  ((?:Silver Rupee Pouches):(?: \\w*|(?:\n    - .+)*))", QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator it = reg.globalMatch(LayoutSection);

    for (uint32_t i = OOT_POUCH_SILVER_DC; i <= OOT_POUCH_SILVER_GANON_WATER; i++)
    {   // Disable all OoT silver pouches by default

        this->DisabledItemIDs.insert(i);
    }

    while (it.hasNext())
    {
        QRegularExpressionMatch match = it.next();
        QStringList layoutParams = match.captured(1).split("\n");

        if (layoutParams.size() == 1)
        {	// None or all

            layoutParams = layoutParams.at(0).split(": ");
            if (layoutParams.at(0) == "Silver Rupee Pouches")
            {	// Ocarina of Time

                if (layoutParams.at(1) == "all")
                {	// All key rings

                    for (uint32_t i = OOT_SMALL_KEY_FOREST; i <= OOT_SMALL_KEY_GTG; i++)
                    {   // Disable all OoT small keys

                        this->DisabledItemIDs.insert(i);
                    }

                    for (uint32_t i = OOT_KEY_RING_FOREST; i <= OOT_KEY_RING_GTG; i++)
                    {   // Enable all OoT key rings

                        this->DisabledItemIDs.remove(i);
                    }

                    if (this->FilterSettings["smallKeyShuffleChestGame"].Value != ShuffleSetting::vanilla)
                    {
                        this->DisabledItemIDs.insert(OOT_SMALL_KEY_TCG);
                        this->DisabledItemIDs.remove(OOT_KEY_RING_TCG);
                    }

                }
            }
        }
        else
        {
            if (layoutParams.at(0) == "Silver Rupee Pouches:")
            {	// Ocarina of Time

                QMap<QString, QPair<uint32_t, uint32_t>> possibleChoice =
                {
                    { "Dodongo's Cavern", { OOT_RUPEE_SILVER_DC, OOT_POUCH_SILVER_DC } },
                    { "Bottom of the Well", { OOT_RUPEE_SILVER_BOTW, OOT_POUCH_SILVER_BOTW } },
                    { "Spirit Temple (Child)", { OOT_RUPEE_SILVER_SPIRIT_CHILD, OOT_POUCH_SILVER_SPIRIT_CHILD } },
                    { "Spirit Temple (Sun)", { OOT_RUPEE_SILVER_SPIRIT_SUN, OOT_POUCH_SILVER_SPIRIT_SUN } },
                    { "Spirit Temple (Boulders)", { OOT_RUPEE_SILVER_SPIRIT_BOULDERS, OOT_POUCH_SILVER_SPIRIT_BOULDERS } },
                    { "Spirit Temple (Lobby)", { OOT_RUPEE_SILVER_SPIRIT_LOBBY, OOT_POUCH_SILVER_SPIRIT_LOBBY } },
                    { "Spirit Temple (Adult)", { OOT_RUPEE_SILVER_SPIRIT_ADULT, OOT_POUCH_SILVER_SPIRIT_ADULT } },
                    { "Shadow Temple (Scythe)", { OOT_RUPEE_SILVER_SHADOW_SCYTHE, OOT_POUCH_SILVER_SHADOW_SCYTHE } },
                    { "Shadow Temple (Pit)", { OOT_RUPEE_SILVER_SHADOW_PIT, OOT_POUCH_SILVER_SHADOW_PIT } },
                    { "Shadow Temple (Spikes)", { OOT_RUPEE_SILVER_SHADOW_SPIKES, OOT_POUCH_SILVER_SHADOW_SPIKES } },
                    { "Shadow Temple (Blades)", { OOT_RUPEE_SILVER_SHADOW_BLADES, OOT_POUCH_SILVER_SHADOW_BLADES } },
                    { "Ice Cavern (Scythe)", { OOT_RUPEE_SILVER_IC_SCYTHE, OOT_POUCH_SILVER_IC_SCYTHE } },
                    { "Ice Cavern (Block)", { OOT_RUPEE_SILVER_IC_BLOCK, OOT_POUCH_SILVER_IC_BLOCK } },
                    { "GTG (Slopes)", { OOT_RUPEE_SILVER_GTG_SLOPES, OOT_POUCH_SILVER_GTG_SLOPES } },
                    { "GTG (Lava)", { OOT_RUPEE_SILVER_GTG_LAVA, OOT_POUCH_SILVER_GTG_LAVA } },
                    { "GTG (Water)", { OOT_RUPEE_SILVER_GTG_WATER, OOT_POUCH_SILVER_GTG_WATER } },
                    { "Ganon's Castle (Light)", { OOT_RUPEE_SILVER_GANON_LIGHT,  OOT_POUCH_SILVER_GANON_LIGHT } },
                    { "Ganon's Castle (Forest)", { OOT_RUPEE_SILVER_GANON_FOREST, OOT_POUCH_SILVER_GANON_FOREST } },
                    { "Ganon's Castle (Fire)", { OOT_RUPEE_SILVER_GANON_FIRE, OOT_POUCH_SILVER_GANON_FIRE } },
                    { "Ganon's Castle (Water)", { OOT_RUPEE_SILVER_GANON_WATER, OOT_POUCH_SILVER_GANON_WATER } },
                    { "Ganon's Castle (Shadow)", { OOT_RUPEE_SILVER_GANON_SHADOW, OOT_POUCH_SILVER_GANON_SHADOW } },
                    { "Ganon's Castle (Spirit)", { OOT_RUPEE_SILVER_GANON_SPIRIT, OOT_POUCH_SILVER_GANON_SPIRIT } }
                };

                for (qsizetype i = 1; i < layoutParams.size(); i++)
                {
                    QString currKey = layoutParams.at(i);
                    currKey = currKey.replace("    - ", "");

                    auto choice = possibleChoice.find(currKey);
                    if (choice != possibleChoice.end())
                    {
                        this->DisabledItemIDs.insert(choice.value().first); // Disable sivler rupees
                        this->DisabledItemIDs.remove(choice.value().second);  // Enable silver pouches
                    }
                }
            }
        }
    }
}


void Settings::ParsePreActivatedOwl(QString& LayoutSection)
{
    QRegularExpression reg("^  ((?:Pre-Activated Owl Statues):(?: \\w*|(?:\n    - .+)*))", QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator it = reg.globalMatch(LayoutSection);

    for (uint32_t i = OOT_POUCH_SILVER_DC; i <= OOT_POUCH_SILVER_GANON_WATER; i++)
    {   // Disable all OoT silver pouches by default

        this->DisabledItemIDs.insert(i);
    }

    while (it.hasNext())
    {
        QRegularExpressionMatch match = it.next();
        QStringList layoutParams = match.captured(1).split("\n");

        if (layoutParams.size() == 1)
        {	// None or all

            layoutParams = layoutParams.at(0).split(": ");
            if (layoutParams.at(0) == "Pre-Activated Owl Statues")
            {	// Ocarina of Time

                if (layoutParams.at(1) == "all")
                {	// All key rings

                    for (uint32_t i = MM_OWL_GREAT_BAY; i <= MM_OWL_STONE_TOWER; i++)
                    {   // Add all owls status

                        this->StartingItemIDs.insert(i, 1);
                    }
                }
            }
        }
        else
        {
            if (layoutParams.at(0) == "Pre-Activated Owl Statues:")
            {	// Majora's Mask

                QMap<QString, uint32_t> possibleChoice =
                {
                    { "Clock Town", MM_OWL_CLOCK_TOWN },
                    { "Milk Road", MM_OWL_MILK_ROAD },
                    { "Southern Swamp", MM_OWL_SOUTHERN_SWAMP },
                    { "Woodfall", MM_OWL_WOODFALL },
                    { "Mountain Village", MM_OWL_MOUNTAIN_VILLAGE },
                    { "Snowhead", MM_OWL_SNOWHEAD },
                    { "Great Bay Coast", MM_OWL_GREAT_BAY },
                    { "Zora Cape", MM_OWL_ZORA_CAPE },
                    { "Ikana Canyon", MM_OWL_IKANA_CANYON },
                    { "Stone Tower", MM_OWL_STONE_TOWER }
                };

                for (qsizetype i = 1; i < layoutParams.size(); i++)
                {
                    QString currKey = layoutParams.at(i);
                    currKey = currKey.replace("    - ", "");

                    auto choice = possibleChoice.find(currKey);
                    if (choice != possibleChoice.end())
                    {
                        this->StartingItemIDs.insert(choice.value(), 1);
                    }
                }
            }
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
    GetSceneMetaInfo(MM_GROTTO_DEKU_PALACE_CLIMB, MM_GAME)->ActiveLayout = GameLayout::mm;

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
                    GetSceneMetaInfo(MM_GROTTO_DEKU_PALACE_CLIMB, MM_GAME)->ActiveLayout = GameLayout::mm_jp;
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
                        GetSceneMetaInfo(MM_GROTTO_DEKU_PALACE_CLIMB, MM_GAME)->ActiveLayout = GameLayout::mm_jp;
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
		else if (Value == "multi")
		{
			this->Mode = GameMode::multi;
		}
		else
		{
			this->Mode = GameMode::single;
		}
	}
    else if (Name == "goal")
    {
        if (Value == "triforce3")
        {
            this->Goal = GoalMode::triforce3;
        }
        else if (Value == "triforce")
        {
            this->Goal = GoalMode::triforce;
        }
        else
        {
            this->Goal = GoalMode::boss;
        }
    }
	else if (Name == "teams")
	{
		this->NumOfTeams = (size_t)atoi(Value.toStdString().c_str());
	}
    else if (Name == "songs")
    {
        ShuffleSetting setting;

        if (Value == "notes")
        {
            setting = ShuffleSetting::all;
        }
        else
        {
            setting = ShuffleSetting::vanilla;
        }

        this->FilterSettings[Name].Value = setting;
    }
    else if (this->FilterSettings.contains(Name))
	{
		ShuffleSetting setting;

		if (Value == "all" || Value == "true" || Value == "full" || Value == "anywhere" || Value == "ganon" || Value == "child" || Value == "cross" || Value.toInt() > 0)
		{
			setting = ShuffleSetting::all;
		}
        else if (Value == "starting")
        {
            setting = ShuffleSetting::starting;
        }
		else if (Value == "dungeons" || Value == "ownDungeon")
		{
			setting = ShuffleSetting::dungeons;
		}
		else if (Value == "overworld")
		{
			setting = ShuffleSetting::overworld;
		}
        else if (Value == "removed")
        {
            setting = ShuffleSetting::removed;
        }
		else
		{
			setting = ShuffleSetting::vanilla;
		}

		this->FilterSettings[Name].Value = setting;
	}
    else if (this->ItemSettings.contains(Name))
    {
        ShuffleSetting setting;

        if (Value == "progressive" || Value == "all" || Value == "true" || Value == "ascending" || Value.toInt() > 0)
        {
            setting = ShuffleSetting::all;
        }
        else if (Value == "goron" || Value == "descending")
        {
            setting = ShuffleSetting::overworld;
        }
        else
        {
            setting = ShuffleSetting::vanilla;
        }

        this->ItemSettings[Name].Value = setting;
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

    this->ApplyItemSettings();
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
					this->CheckObjectExclusion(currObj, this->FilterSettings["goldSkulltulaTokens"].Value, Filter);
                    if (this->FilterSettings["goldSkulltulaTokens"].Value == ShuffleSetting::vanilla)
                    {   // Set the object item to gs token

                        currObj->Item = FindItem(OOT_GS_TOKEN);
                    }
					break;
				}

				case ObjectType::map:
                {
                    this->CheckObjectExclusion(currObj, this->FilterSettings["mapCompassShuffle"].Value, Filter);
                    break;
                }

				case ObjectType::compass:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["mapCompassShuffle"].Value, Filter);
					break;
				}

				case ObjectType::small_key:
				{
					if (currObj->Scene == OOT_THIEVES_HIDEOUT)
					{	// Hideout small key

						this->CheckObjectExclusion(currObj, this->FilterSettings["smallKeyShuffleHideout"].Value, Filter);
					}
					else if (currObj->LocationType == LocType::minigame)
					{	// Treasure chest small key

						this->CheckObjectExclusion(currObj, this->FilterSettings["smallKeyShuffleChestGame"].Value, Filter);
					}
					else
					{	// Dungeon small key

						this->CheckObjectExclusion(currObj, this->FilterSettings["smallKeyShuffleOot"].Value, Filter);
					}
					break;
				}

				case ObjectType::boss_key:
				{
					if (currObj->Scene == OOT_GANON_TOWER)
					{	// Ganon boss key

						this->CheckObjectExclusion(currObj, this->FilterSettings["ganonBossKey"].Value, Filter);
					}
					else
					{	// Other boss key

						this->CheckObjectExclusion(currObj, this->FilterSettings["bossKeyShuffleOot"].Value, Filter);
					}
					break;
				}

				case ObjectType::sr:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["silverRupeeShuffle"].Value, Filter);
					break;
				}

				case ObjectType::scrub:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["scrubShuffleOot"].Value, Filter);
					break;
				}

				case ObjectType::cow:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["cowShuffleOot"].Value, Filter);
					break;
				}

				case ObjectType::shop:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shopShuffleOot"].Value, Filter);
					break;
				}

				case ObjectType::pot:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shufflePotsOot"].Value, Filter);
					break;
				}

				case ObjectType::crate:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleCratesOot"].Value, Filter);
					break;
				}

				case ObjectType::hive:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleHivesOot"].Value, Filter);
					break;
				}

				case ObjectType::grass:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleGrassOot"].Value, Filter);
					break;
				}

				case ObjectType::rock:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleRocksOot"].Value, Filter);
					break;
				}

				case ObjectType::tree:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleTreesOot"].Value, Filter);
					break;
				}

				case ObjectType::bush:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleBushOot"].Value, Filter);
					break;
				}

				case ObjectType::soil:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleSoilOot"].Value, Filter);
					break;
				}

				case ObjectType::rupee:
				{
					if (currObj->LocationType == LocType::minigame)
					{	// Diving game rupee

						this->CheckObjectExclusion(currObj, this->FilterSettings["divingGameRupeeShuffle"].Value, Filter);
					}
					else
					{	// Normal freestanding rupee

						this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleFreeRupeesOot"].Value, Filter);
					}
					break;
				}

				case ObjectType::heart:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleFreeHeartsOot"].Value, Filter);
					break;
				}

				case ObjectType::wonder:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleWonderItemsOot"].Value, Filter);
					break;
				}

				case ObjectType::butterfly:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleButterfliesOot"].Value, Filter);
					break;
				}

				case ObjectType::redboulder:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleRedBouldersOot"].Value, Filter);
					break;
				}

				case ObjectType::icicle:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleIciclesOot"].Value, Filter);
					break;
				}

				case ObjectType::redice:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleRedIceOot"].Value, Filter);
					break;
				}
				
				case ObjectType::ocarina:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleOcarinasOot"].Value, Filter);
					break;
				}

				case ObjectType::sword:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleMasterSword"].Value, Filter);
					break;
				}

				case ObjectType::mask:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleMaskTrades"].Value, Filter);
					break;
				}

				case ObjectType::merchant:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleMerchantsOot"].Value, Filter);
					break;
				}

				case ObjectType::fish:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["pondFishShuffle"].Value, Filter);
					break;
				}

				case ObjectType::fairy:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["fairyFountainFairyShuffleOot"].Value, Filter);
					break;
				}

				case ObjectType::fairy_spot:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["fairySpotShuffleOot"].Value, Filter);
					break;
				}

				case ObjectType::egg:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["eggShuffle"].Value, Filter);
					break;
				}

				case ObjectType::npc:
				{
					switch (currObj->MapIcon)
					{
						case EGameIcon::card:
						{	// Check for gerudo card

							this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleGerudoCard"].Value, Filter);
							break;
						}

						case EGameIcon::frog:
						{	// Check for frog rupees rewards

							if (currObj->LocationType == LocType::minigame)
							{
								this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleFrogsRupeesOot"].Value, Filter);
							}
							break;
						}

                        case EGameIcon::letter:
                        {

                            this->CheckObjectExclusion(currObj, this->FilterSettings["skipZelda"].Value == ShuffleSetting::vanilla ? ShuffleSetting::all : ShuffleSetting::vanilla, Filter);
                            break;
                        }
					}
				}

                case ObjectType::song:
                {
                    if (currObj->RenderScene == OOT_CASTLE_COURTYARD)
                    {   // Only filter zelda's song object

                        this->CheckObjectExclusion(currObj, this->FilterSettings["skipZelda"].Value == ShuffleSetting::vanilla ? ShuffleSetting::all : ShuffleSetting::vanilla, Filter);
                    }
                    break;
                }

				case ObjectType::none:
				default:
				{	// We don't want to overload the filter with non rendered object anyway

					break;
				}
			}
		}
	}

	if (this->FilterSettings["restoreBrokenActors"].Value != ShuffleSetting::all)
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
					this->CheckObjectExclusion(currObj, this->FilterSettings["housesSkulltulaTokens"].Value, Filter);
					break;
				}

				case ObjectType::map:
				{
					if (currObj->Type == ObjectType::npc)
					{	// Tingle maps

						this->CheckObjectExclusion(currObj, this->FilterSettings["tingleShuffle"].Value, Filter);
						break;
					}
				}
				case ObjectType::compass:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["mapCompassShuffle"].Value, Filter);
					break;
				}

				case ObjectType::small_key:
				{	// Dungeon small key

					this->CheckObjectExclusion(currObj, this->FilterSettings["smallKeyShuffleMm"].Value, Filter);
					break;
				}

				case ObjectType::boss_key:
				{	// Boss key

					this->CheckObjectExclusion(currObj, this->FilterSettings["bossKeyShuffleMm"].Value, Filter);
					break;
				}

				case ObjectType::sf:
				{
					if (currObj->ObjectID == STRAY_FAIRY_TOWN)
					{	// Stray fairy town

						this->CheckObjectExclusion(currObj, this->FilterSettings["townFairyShuffle"].Value, Filter);
					}
					else if (currObj->Type == ObjectType::chest)
					{	// Stray fairy in a chest

						this->CheckObjectExclusion(currObj, this->FilterSettings["strayFairyChestShuffle"].Value, Filter);
                        if (this->FilterSettings["strayFairyChestShuffle"].Value == ShuffleSetting::starting)
                        {
                            uint32_t itemID = MM_STRAY_FAIRY;
                            switch (currObj->Scene)
                            {
                                case MM_TEMPLE_WOODFALL:
                                {
                                    itemID = MM_STRAY_FAIRY_WF;
                                    break;
                                }

                                case MM_TEMPLE_SNOWHEAD:
                                {
                                    itemID = MM_STRAY_FAIRY_SH;
                                    break;
                                }
                                case MM_TEMPLE_GREAT_BAY:
                                {
                                    itemID = MM_STRAY_FAIRY_GB;
                                    break;
                                }
                                case MM_STONE_TOWER:
                                case MM_STONE_TOWER_INVERTED:
                                {
                                    itemID = MM_STRAY_FAIRY_ST;
                                    break;
                                }

                                default:
                                    continue;
                            }

                            if (this->StartingItemIDs.contains(itemID))
                            {
                                this->StartingItemIDs.find(itemID).value()++;
                            }
                            else
                            {
                                this->StartingItemIDs.insert(itemID, 1);
                            }
                        }
					}
					else
					{	// All other stray fairies

						this->CheckObjectExclusion(currObj, this->FilterSettings["strayFairyOtherShuffle"].Value, Filter);
                        if (this->FilterSettings["strayFairyOtherShuffle"].Value == ShuffleSetting::starting)
                        {
                            uint32_t itemID = MM_STRAY_FAIRY;
                            switch (currObj->Scene)
                            {
                                case MM_TEMPLE_WOODFALL:
                                {
                                    itemID = MM_STRAY_FAIRY_WF;
                                    break;
                                }

                                case MM_TEMPLE_SNOWHEAD:
                                {
                                    itemID = MM_STRAY_FAIRY_SH;
                                    break;
                                }
                                case MM_TEMPLE_GREAT_BAY:
                                {
                                    itemID = MM_STRAY_FAIRY_GB;
                                    break;
                                }
                                case MM_STONE_TOWER:
                                case MM_STONE_TOWER_INVERTED:
                                {
                                    itemID = MM_STRAY_FAIRY_ST;
                                    break;
                                }

                                default:
                                    continue;
                            }

                            if (this->StartingItemIDs.contains(itemID))
                            {
                                this->StartingItemIDs.find(itemID).value()++;
                            }
                            else
                            {
                                this->StartingItemIDs.insert(itemID, 1);
                            }
                        }
					}
					break;
				}

				case ObjectType::scrub:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["scrubShuffleMm"].Value, Filter);
					break;
				}

				case ObjectType::cow:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["cowShuffleMm"].Value, Filter);
					break;
				}

				case ObjectType::shop:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shopShuffleMm"].Value, Filter);
					break;
				}

				case ObjectType::owl:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["owlShuffle"].Value, Filter);
					break;
				}

				case ObjectType::pot:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shufflePotsMm"].Value, Filter);
					break;
				}

				case ObjectType::crate:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleCratesMm"].Value, Filter);
					break;
				}

				case ObjectType::barrel:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleBarrelsMm"].Value, Filter);
					break;
				}

				case ObjectType::hive:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleHivesMm"].Value, Filter);
					break;
				}

				case ObjectType::rock:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleRocksMm"].Value, Filter);
					break;
				}

				case ObjectType::grass:
				{
					if (currObj->Scene == MM_TERMINA_FIELD)
					{	// Termina field grass

						this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleTFGrassMm"].Value, Filter);
					}
					else
					{	// All other grass

						this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleGrassMm"].Value, Filter);
					}
					break;
				}

				case ObjectType::tree:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleTreesMm"].Value, Filter);
					break;
				}

				case ObjectType::bush:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleBushMm"].Value, Filter);
					break;
				}

				case ObjectType::soil:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleSoilMm"].Value, Filter);
					break;
				}

				case ObjectType::rupee:
				{	// Normal freestanding rupee

					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleFreeRupeesMm"].Value, Filter);
					break;
				}

				case ObjectType::heart:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleFreeHeartsMm"].Value, Filter);
					break;
				}

				case ObjectType::wonder:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleWonderItemsMm"].Value, Filter);
					break;
				}

				case ObjectType::snowball:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleSnowballsMm"].Value, Filter);
					break;
				}

				case ObjectType::butterfly:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleButterfliesMm"].Value, Filter);
					break;
				}

				case ObjectType::redboulder:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleRedBouldersMm"].Value, Filter);
					break;
				}

				case ObjectType::icicle:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleIciclesMm"].Value, Filter);
					break;
				}

				case ObjectType::merchant:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleMerchantsMm"].Value, Filter);
					break;
				}

				case ObjectType::fairy:
				{
					this->CheckObjectExclusion(currObj, this->FilterSettings["fairyFountainFairyShuffleMm"].Value, Filter);
					break;
				}

				case ObjectType::npc:
				{
					if (currObj->Scene == MM_LOTTERY)
					{	// Lottery

						this->CheckObjectExclusion(currObj, this->FilterSettings["shuffleLotteryMm"].Value, Filter);
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

void Settings::ApplyItemSettings()
{
    if (this->Goal == GoalMode::triforce)
    {
        this->DisabledItemIDs.insert(OOT_TRIFORCE_POWER);
        this->DisabledItemIDs.insert(OOT_TRIFORCE_COURAGE);
        this->DisabledItemIDs.insert(OOT_TRIFORCE_WISDOM);
    }
    else if (this->Goal == GoalMode::triforce3)
    {
        this->DisabledItemIDs.insert(OOT_TRIFORCE);
        this->DisabledItemIDs.insert(OOT_TRIFORCE_FULL);
    }
    else
    {
        this->DisabledItemIDs.insert(OOT_TRIFORCE);
        this->DisabledItemIDs.insert(OOT_TRIFORCE_FULL);
        this->DisabledItemIDs.insert(OOT_TRIFORCE_POWER);
        this->DisabledItemIDs.insert(OOT_TRIFORCE_COURAGE);
        this->DisabledItemIDs.insert(OOT_TRIFORCE_WISDOM);
    }

    if (this->FilterSettings["mapCompassShuffle"].Value == ShuffleSetting::removed)
    {
        for (uint32_t i = OOT_MAP_DT; i <= OOT_COMPASS_IC; i++)
        {   // Disable all OoT maps / compasses

            this->DisabledItemIDs.insert(i);
        }

        for (uint32_t i = MM_MAP_WF; i <= MM_COMPASS_ST; i++)
        {   // Disable all MM maps / compasses

            this->DisabledItemIDs.insert(i);
        }
    }

    if (this->FilterSettings["tingleShuffle"].Value == ShuffleSetting::removed)
    {
        for (uint32_t i = MM_WORLD_MAP_CLOCK_TOWN; i <= MM_WORLD_MAP_STONE_TOWER; i++)
        {   // Disable all MM tingle maps

            this->DisabledItemIDs.insert(i);
        }
    }

    if (this->FilterSettings["ganonBossKey"].Value == ShuffleSetting::removed)
    {
        this->DisabledItemIDs.insert(OOT_BOSS_KEY_GANON);
    }

    if (this->FilterSettings["bossKeyShuffleOot"].Value == ShuffleSetting::removed)
    {
        for (uint32_t i = OOT_BOSS_KEY_FOREST; i <= OOT_BOSS_KEY_SHADOW; i++)
        {
            this->DisabledItemIDs.insert(i);
        }
    }

    if (this->FilterSettings["bossKeyShuffleMm"].Value == ShuffleSetting::removed)
    {
        for (uint32_t i = MM_BOSS_KEY_WF; i <= MM_BOSS_KEY_ST; i++)
        {
            this->DisabledItemIDs.insert(i);
        }
    }

    // Small keys + key rings: 'removed' means the rolled seed contains neither the small key nor
    // its ring (Chest Game and Hideout follow their own boolean below). Mirrors the bossKey /
    // mapCompass blocks above so the spoiler-load and SettingsTab paths share one source of truth.
    if (this->FilterSettings["smallKeyShuffleOot"].Value == ShuffleSetting::removed)
    {
        for (uint32_t i = OOT_SMALL_KEY_FOREST; i <= OOT_SMALL_KEY_GTG; i++)
        {
            this->DisabledItemIDs.insert(i);
        }
        for (uint32_t i = OOT_KEY_RING_FOREST; i <= OOT_KEY_RING_GTG; i++)
        {
            this->DisabledItemIDs.insert(i);
        }
    }

    if (this->FilterSettings["smallKeyShuffleMm"].Value == ShuffleSetting::removed)
    {
        for (uint32_t i = MM_SMALL_KEY_WF; i <= MM_SMALL_KEY_ST; i++)
        {
            this->DisabledItemIDs.insert(i);
        }
        for (uint32_t i = MM_KEY_RING_WF; i <= MM_KEY_RING_ST; i++)
        {
            this->DisabledItemIDs.insert(i);
        }
    }

    // Note: Chest Game is intentionally NOT force-hidden here even when smallKeyShuffleChestGame
    // is vanilla. The "Small Key Ring (OoT)" parser (or the user's per-dungeon checkbox via the
    // World Items page) already decides whether OOT_KEY_RING_TCG / OOT_SMALL_KEY_TCG are tracked;
    // overriding it here was destroying the user's explicit choice when the ring is enabled but
    // the boolean is off.

    if (this->FilterSettings["skipZelda"].Value == ShuffleSetting::all)
    {
        this->DisabledItemIDs.insert(OOT_CHICKEN);
        this->StartingItemIDs.insert(OOT_SONG_ZELDA, 1);
        this->StartingItemIDs.insert(OOT_ZELDA_LETTER, 1);
    }

    for (auto it = this->ItemSettings.cbegin(); it != this->ItemSettings.cend(); ++it)
    {
        // Is progressive
        if (it.key().compare("progressiveShieldsOot") == 0)
        {
            this->ProgressiveItemIDs.insert(OOT_PROGRESSIVE_SHIELD_DEKU);
            this->ProgressiveItemIDs.insert(OOT_PROGRESSIVE_SHIELD_HYLIAN);
        }
        else if (it.key().compare("progressiveSwordsOot") == 0)
        {
            this->ProgressiveItemIDs.insert(OOT_SWORD_KOKIRI);
            this->ProgressiveItemIDs.insert(OOT_SWORD_MASTER);
            this->ProgressiveItemIDs.insert(OOT_SWORD_KNIFE);
            this->ProgressiveItemIDs.insert(OOT_SWORD_GORON);
            this->ProgressiveItemIDs.insert(OOT_SWORD_BIGGORON);
            this->ProgressiveItemIDs.insert(OOT_SWORD_RAZOR);
            this->ProgressiveItemIDs.insert(OOT_SWORD_GILDED);
            this->ProgressiveItemIDs.insert(OOT_SWORD);
        }
        else if (it.key().compare("progressiveShieldsMm") == 0)
        {
            this->ProgressiveItemIDs.insert(MM_PROGRESSIVE_SHIELD_DEKU);
            this->ProgressiveItemIDs.insert(MM_PROGRESSIVE_SHIELD_HERO);
        }
        else if (it.key().compare("progressiveGFS") == 0)
        {
            this->ProgressiveItemIDs.insert(MM_SWORD_KOKIRI);
            this->ProgressiveItemIDs.insert(MM_SWORD_RAZOR);
            this->ProgressiveItemIDs.insert(MM_SWORD_GILDED);
            this->ProgressiveItemIDs.insert(MM_GREAT_FAIRY_SWORD);
        }
        else if (it.key().compare("progressiveClocks") == 0)
        {
            this->ProgressiveItemIDs.insert(MM_CLOCK);
            this->ProgressiveItemIDs.insert(MM_CLOCK1);
            this->ProgressiveItemIDs.insert(MM_CLOCK2);
            this->ProgressiveItemIDs.insert(MM_CLOCK3);
            this->ProgressiveItemIDs.insert(MM_CLOCK4);
            this->ProgressiveItemIDs.insert(MM_CLOCK5);
            this->ProgressiveItemIDs.insert(MM_CLOCK6);
        }
        // Disable Items
        else if (it.key().compare("sticksNutsUpgradesMm") == 0)
        {
            if (!this->CheckItemEnabled(it->Value, MM_NUT_UPGRADE))
            {
                this->DisabledItemIDs.insert(MM_NUT_UPGRADE2);
                this->DisabledItemIDs.insert(MM_STICK_UPGRADE);
                this->DisabledItemIDs.insert(MM_STICK_UPGRADE2);
            }
        }
        else if (it.key().compare("sunSongMm") == 0)
        {
            this->CheckItemEnabled(it->Value, MM_SONG_SUN);
        }
        else if (it.key().compare("fairyOcarinaMm") == 0)
        {
            this->CheckItemEnabled(it->Value, MM_OCARINA_FAIRY);
        }
        else if (it.key().compare("shortHookshotMm") == 0)
        {
            this->CheckItemEnabled(it->Value, MM_HOOKSHOT_SHORT);
        }
        else if (it.key().compare("bronzeScale") == 0)
        {
            if (!this->CheckItemEnabled(it->Value, OOT_SCALE_BRONZE))
            {
                this->DisabledItemIDs.insert(MM_SCALE_BRONZE);
            }
        }
        else if (it.key().compare("childWallets") == 0)
        {
            if (!this->CheckItemEnabled(it->Value, OOT_WALLET))
            {
                this->DisabledItemIDs.insert(MM_WALLET);
            }
        }
        else if (it.key().compare("colossalWallets") == 0)
        {
            if (!this->CheckItemEnabled(it->Value, OOT_WALLET4))
            {
                this->DisabledItemIDs.insert(MM_WALLET4);
            }
        }
        else if (it.key().compare("bottomlessWallets") == 0)
        {
            if (!this->CheckItemEnabled(it->Value, OOT_WALLET5))
            {
                this->DisabledItemIDs.insert(MM_WALLET5);
            }
        }
        else if (it.key().compare("skeletonKeyOot") == 0)
        {
            this->CheckItemEnabled(it->Value, OOT_SKELETON_KEY);
        }
        else if (it.key().compare("skeletonKeyMm") == 0)
        {
            this->CheckItemEnabled(it->Value, MM_SKELETON_KEY);
        }
        else if (it.key().compare("magicalRupee") == 0)
        {
            this->CheckItemEnabled(it->Value, OOT_RUPEE_MAGICAL);
        }
        else if (it.key().compare("transcendentFairy") == 0)
        {
            this->CheckItemEnabled(it->Value, MM_TRANSCENDENT_FAIRY);
        }
        else if (it.key().compare("platinumTokenOot") == 0)
        {
            this->CheckItemEnabled(it->Value, OOT_PLATINUM_TOKEN);
        }
        else if (it.key().compare("platinumTokenMm") == 0)
        {
            this->CheckItemEnabled(it->Value, MM_PLATINUM_TOKEN);
        }
        else if (it.key().compare("spellFireMm") == 0)
        {
            this->CheckItemEnabled(it->Value, MM_SPELL_FIRE);
        }
        else if (it.key().compare("spellWindMm") == 0)
        {
            this->CheckItemEnabled(it->Value, MM_SPELL_WIND);
        }
        else if (it.key().compare("spellLoveMm") == 0)
        {
            this->CheckItemEnabled(it->Value, MM_SPELL_LOVE);
        }
        else if (it.key().compare("bootsIronMm") == 0)
        {
            this->CheckItemEnabled(it->Value, MM_BOOTS_IRON);
        }
        else if (it.key().compare("bootsHoverMm") == 0)
        {
            this->CheckItemEnabled(it->Value, MM_BOOTS_HOVER);
        }
        else if (it.key().compare("tunicGoronMm") == 0)
        {
            this->CheckItemEnabled(it->Value, MM_TUNIC_GORON);
        }
        else if (it.key().compare("tunicZoraMm") == 0)
        {
            this->CheckItemEnabled(it->Value, MM_TUNIC_ZORA);
        }
        else if (it.key().compare("scalesMm") == 0)
        {
            if (!this->CheckItemEnabled(it->Value, MM_SCALE_SILVER))
            {
                this->DisabledItemIDs.insert(MM_SCALE_GOLDEN);
            }
        }
        else if (it.key().compare("strengthMm") == 0)
        {
            if (!this->CheckItemEnabled(it->Value, MM_GORON_BRACELET))
            {
                this->DisabledItemIDs.insert(MM_SILVER_GAUNTLETS);
                this->DisabledItemIDs.insert(MM_GOLDEN_GAUNTLETS);
            }
        }
        else if (it.key().compare("hammerMm") == 0)
        {
            this->CheckItemEnabled(it->Value, MM_HAMMER);
        }
        else if (it.key().compare("spinUpgradeOot") == 0)
        {
            this->CheckItemEnabled(it->Value, OOT_SPIN_UPGRADE);
        }
        else if (it.key().compare("dekuShieldMm") == 0)
        {
            this->CheckItemEnabled(it->Value, MM_SHIELD_DEKU);
        }
        else if (it.key().compare(QString("blastMaskOot")) == 0)
        {
            this->CheckItemEnabled(it->Value, OOT_MASK_BLAST);
        }
        else if (it.key().compare("stoneMaskOot") == 0)
        {
            this->CheckItemEnabled(it->Value, OOT_MASK_STONE);
        }
        else if (it.key().compare("elegyOot") == 0)
        {
            this->CheckItemEnabled(it->Value, OOT_SONG_EMPTINESS);
        }
        else if (it.key().compare("ocarinaButtonsShuffleOot") == 0)
        {
            if (!this->CheckItemEnabled(it->Value, OOT_BUTTON_A))
            {
                this->DisabledItemIDs.insert(OOT_BUTTON_C_DOWN);
                this->DisabledItemIDs.insert(OOT_BUTTON_C_LEFT);
                this->DisabledItemIDs.insert(OOT_BUTTON_C_RIGHT);
                this->DisabledItemIDs.insert(OOT_BUTTON_C_UP);
            }
        }
        else if (it.key().compare("ocarinaButtonsShuffleMm") == 0)
        {
            if (!this->CheckItemEnabled(it->Value, MM_BUTTON_A))
            {
                this->DisabledItemIDs.insert(MM_BUTTON_C_DOWN);
                this->DisabledItemIDs.insert(MM_BUTTON_C_LEFT);
                this->DisabledItemIDs.insert(MM_BUTTON_C_RIGHT);
                this->DisabledItemIDs.insert(MM_BUTTON_C_UP);
            }
        }
        else if (it.key().compare("soulsEnemyOot") == 0)
        {
            if (!this->CheckItemEnabled(it->Value, OOT_SOUL_ENEMY_STALFOS))
            {
                for (uint32_t i = OOT_SOUL_ENEMY_OCTOROK; i <= OOT_SOUL_ENEMY_POE; i++)
                {
                    this->DisabledItemIDs.insert(i);
                }
            }
        }
        else if (it.key().compare("soulsEnemyMm") == 0)
        {
            if (!this->CheckItemEnabled(it->Value, MM_SOUL_ENEMY_OCTOROK))
            {
                for (uint32_t i = MM_SOUL_ENEMY_WALLMASTER; i <= MM_SOUL_ENEMY_POE; i++)
                {
                    this->DisabledItemIDs.insert(i);
                }
            }
        }
        else if (it.key().compare("soulsBossOot") == 0)
        {
            if (!this->CheckItemEnabled(it->Value, OOT_SOUL_BOSS_QUEEN_GOHMA))
            {
                for (uint32_t i = OOT_SOUL_BOSS_KING_DODONGO; i <= OOT_SOUL_BOSS_TWINROVA; i++)
                {
                    this->DisabledItemIDs.insert(i);
                }
            }
        }
        else if (it.key().compare("soulsBossMm") == 0)
        {
            if (!this->CheckItemEnabled(it->Value, MM_SOUL_BOSS_ODOLWA))
            {
                for (uint32_t i = MM_SOUL_BOSS_GOHT; i <= MM_SOUL_BOSS_IGOS; i++)
                {
                    this->DisabledItemIDs.insert(i);
                }
            }
        }
        else if (it.key().compare("soulsNpcOot") == 0)
        {
            if (!this->CheckItemEnabled(it->Value, OOT_SOUL_NPC_SARIA))
            {
                for (uint32_t i = OOT_SOUL_NPC_DARUNIA; i <= OOT_SOUL_NPC_THIEVES; i++)
                {
                    this->DisabledItemIDs.insert(i);
                }
            }
        }
        else if (it.key().compare("soulsNpcMm") == 0)
        {
            if (!this->CheckItemEnabled(it->Value, MM_SOUL_NPC_AROMA))
            {
                for (uint32_t i = MM_SOUL_NPC_SHOOTING_GALLERY_OWNER; i <= MM_SOUL_NPC_THIEVES; i++)
                {
                    this->DisabledItemIDs.insert(i);
                }
            }
        }
        else if (it.key().compare("soulsAnimalOot") == 0)
        {
            if (!this->CheckItemEnabled(it->Value, OOT_SOUL_ANIMAL_CUCCO))
            {
                for (uint32_t i = OOT_SOUL_ANIMAL_COW; i <= OOT_SOUL_ANIMAL_BUTTERFLY; i++)
                {
                    this->DisabledItemIDs.insert(i);
                }
            }
        }
        else if (it.key().compare("soulsAnimalMm") == 0)
        {
            if (!this->CheckItemEnabled(it->Value, MM_SOUL_ANIMAL_CUCCO))
            {
                for (uint32_t i = MM_SOUL_ANIMAL_COW; i <= MM_SOUL_ANIMAL_BUTTERFLY; i++)
                {
                    this->DisabledItemIDs.insert(i);
                }
            }
        }
        else if (it.key().compare("soulsMiscOot") == 0)
        {
            this->CheckItemEnabled(it->Value, OOT_SOUL_MISC_GS);
            this->CheckItemEnabled(it->Value, OOT_SOUL_MISC_BUSINESS_SCRUB);
        }
        else if (it.key().compare("soulsMiscMm") == 0)
        {
            this->CheckItemEnabled(it->Value, MM_SOUL_MISC_GS);
            this->CheckItemEnabled(it->Value, MM_SOUL_MISC_BUSINESS_SCRUB);
        }
        else if (it.key().compare("clocks") == 0)
        {
            if (!this->CheckItemEnabled(it->Value, MM_CLOCK1))
            {
                this->DisabledItemIDs.insert(MM_CLOCK2);
                this->DisabledItemIDs.insert(MM_CLOCK3);
                this->DisabledItemIDs.insert(MM_CLOCK4);
                this->DisabledItemIDs.insert(MM_CLOCK5);
                this->DisabledItemIDs.insert(MM_CLOCK6);
            }
        }
        else if (it.key().compare("coins") == 0)
        {
            if (!this->CheckItemEnabled(it->Value, OOT_COIN_RED))
            {
                this->DisabledItemIDs.insert(OOT_COIN_GREEN);
                this->DisabledItemIDs.insert(OOT_COIN_BLUE);
                this->DisabledItemIDs.insert(OOT_COIN_YELLOW);
            }
        }
        else if (it.key().compare("coinsRed") == 0)
        {
            this->CheckItemEnabled(it->Value, OOT_COIN_RED);
        }
        else if (it.key().compare("coinsGreen") == 0)
        {
            this->CheckItemEnabled(it->Value, OOT_COIN_GREEN);
        }
        else if (it.key().compare("coinsBlue") == 0)
        {
            this->CheckItemEnabled(it->Value, OOT_COIN_BLUE);
        }
        else if (it.key().compare("coinsYellow") == 0)
        {
            this->CheckItemEnabled(it->Value, OOT_COIN_YELLOW);
        }
        else if (it.key().compare("trapIce") == 0)
        {
            this->CheckItemEnabled(it->Value, OOT_TRAP_ICE);
        }
        else if (it.key().compare("trapFire") == 0)
        {
            this->CheckItemEnabled(it->Value, OOT_TRAP_FIRE);
        }
        else if (it.key().compare("trapShock") == 0)
        {
            this->CheckItemEnabled(it->Value, OOT_TRAP_SHOCK);
        }
        else if (it.key().compare("trapDrain") == 0)
        {
            this->CheckItemEnabled(it->Value, OOT_TRAP_DRAIN);
        }
        else if (it.key().compare("trapAntiMagic") == 0)
        {
            this->CheckItemEnabled(it->Value, OOT_TRAP_ANTI_MAGIC);
        }
        else if (it.key().compare("trapKnockback") == 0)
        {
            this->CheckItemEnabled(it->Value, OOT_TRAP_KNOCKBACK);
        }
        else if (it.key().compare("trapRupoor") == 0)
        {
            this->CheckItemEnabled(it->Value, OOT_TRAP_RUPOOR);
        }
        // Shared Settings
        else if (it.key().compare("sharedStoneAgony") == 0)
        {
            this->SharedItemIDs.insert(OOT_STONE_OF_AGONY);
            this->SharedItemIDs.insert(MM_STONE_OF_AGONY);
        }
        else if (it.key().compare("sharedSpinUpgrade") == 0)
        {
            this->SharedItemIDs.insert(OOT_SPIN_UPGRADE);
            this->SharedItemIDs.insert(MM_SPIN_UPGRADE);
        }
        else if (it.key().compare("sharedNutsSticks") == 0)
        {
            this->SharedItemIDs.insert(OOT_STICK);
            this->SharedItemIDs.insert(OOT_STICKS_5);
            this->SharedItemIDs.insert(OOT_STICKS_10);
            this->SharedItemIDs.insert(OOT_NUTS_5);
            this->SharedItemIDs.insert(OOT_NUTS_5_ALT);
            this->SharedItemIDs.insert(OOT_NUTS_10);
            this->SharedItemIDs.insert(OOT_STICK_UPGRADE);
            this->SharedItemIDs.insert(OOT_STICK_UPGRADE2);
            this->SharedItemIDs.insert(OOT_NUT_UPGRADE);
            this->SharedItemIDs.insert(OOT_NUT_UPGRADE2);
            this->SharedItemIDs.insert(MM_STICK);
            this->SharedItemIDs.insert(MM_UNK_2F);
            this->SharedItemIDs.insert(MM_UNK_30);
            this->SharedItemIDs.insert(MM_UNK_31);
            this->SharedItemIDs.insert(MM_NUT);
            this->SharedItemIDs.insert(MM_NUTS_5);
            this->SharedItemIDs.insert(MM_NUTS_10);
            this->SharedItemIDs.insert(MM_UNK_2B);
            this->SharedItemIDs.insert(MM_UNK_2C);
            this->SharedItemIDs.insert(MM_UNK_2D);
            this->SharedItemIDs.insert(OOT_STICK_UPGRADE);
            this->SharedItemIDs.insert(OOT_STICK_UPGRADE2);
            this->SharedItemIDs.insert(OOT_NUT_UPGRADE);
            this->SharedItemIDs.insert(OOT_NUT_UPGRADE2);
        }
        else if (it.key().compare("sharedBows") == 0)
        {
            this->SharedItemIDs.insert(OOT_BOW);
            this->SharedItemIDs.insert(OOT_ARROWS_5);
            this->SharedItemIDs.insert(OOT_ARROWS_10);
            this->SharedItemIDs.insert(OOT_ARROWS_30);
            this->SharedItemIDs.insert(OOT_QUIVER2);
            this->SharedItemIDs.insert(OOT_QUIVER3);
            this->SharedItemIDs.insert(MM_BOW);
            this->SharedItemIDs.insert(MM_ARROWS_10);
            this->SharedItemIDs.insert(MM_ARROWS_30);
            this->SharedItemIDs.insert(MM_ARROWS_40);
            this->SharedItemIDs.insert(MM_UNK_21);
            this->SharedItemIDs.insert(MM_QUIVER2);
            this->SharedItemIDs.insert(MM_QUIVER3);
        }
        else if (it.key().compare("sharedBombBags") == 0)
        {
            this->SharedItemIDs.insert(OOT_BOMBCHU_BAG);
            this->SharedItemIDs.insert(OOT_BOMBCHU_BAG2);
            this->SharedItemIDs.insert(OOT_BOMBCHU_BAG3);
            this->SharedItemIDs.insert(OOT_BOMB_BAG);
            this->SharedItemIDs.insert(OOT_BOMB_BAG2);
            this->SharedItemIDs.insert(OOT_BOMB_BAG3);
            this->SharedItemIDs.insert(OOT_BOMB);
            this->SharedItemIDs.insert(OOT_BOMBS_5);
            this->SharedItemIDs.insert(OOT_BOMBS_10);
            this->SharedItemIDs.insert(OOT_BOMBS_20);
            this->SharedItemIDs.insert(OOT_BOMBS_30);
            this->SharedItemIDs.insert(OOT_BOMBCHU_5);
            this->SharedItemIDs.insert(OOT_BOMBCHU_10);
            this->SharedItemIDs.insert(OOT_BOMBCHU_20);
            this->SharedItemIDs.insert(MM_BOMBCHU_BAG);
            this->SharedItemIDs.insert(MM_BOMBCHU_BAG2);
            this->SharedItemIDs.insert(MM_BOMBCHU_BAG3);
            this->SharedItemIDs.insert(MM_BOMB_BAG);
            this->SharedItemIDs.insert(MM_BOMB_BAG2);
            this->SharedItemIDs.insert(MM_BOMB_BAG3);
            this->SharedItemIDs.insert(MM_BOMB);
            this->SharedItemIDs.insert(MM_BOMBS_5);
            this->SharedItemIDs.insert(MM_BOMBS_10);
            this->SharedItemIDs.insert(MM_BOMBS_20);
            this->SharedItemIDs.insert(MM_BOMBS_30);
            this->SharedItemIDs.insert(MM_BOMBCHU);
            this->SharedItemIDs.insert(MM_BOMBCHU_5);
            this->SharedItemIDs.insert(MM_BOMBCHU_10);
            this->SharedItemIDs.insert(MM_BOMBCHU_20);
        }
        else if (it.key().compare("sharedMagic") == 0)
        {
            this->SharedItemIDs.insert(OOT_MAGIC_UPGRADE);
            this->SharedItemIDs.insert(OOT_MAGIC_UPGRADE2);
            this->SharedItemIDs.insert(OOT_MAGIC_JAR_SMALL);
            this->SharedItemIDs.insert(OOT_MAGIC_JAR_LARGE);
            this->SharedItemIDs.insert(MM_MAGIC_UPGRADE);
            this->SharedItemIDs.insert(MM_MAGIC_UPGRADE2);
            this->SharedItemIDs.insert(MM_MAGIC_JAR_SMALL);
            this->SharedItemIDs.insert(MM_MAGIC_JAR_LARGE);
        }
        else if (it.key().compare("sharedMagicArrowFire") == 0)
        {
            this->SharedItemIDs.insert(OOT_ARROW_FIRE);
            this->SharedItemIDs.insert(MM_ARROW_FIRE);
        }
        else if (it.key().compare("sharedMagicArrowIce") == 0)
        {
            this->SharedItemIDs.insert(OOT_ARROW_ICE);
            this->SharedItemIDs.insert(MM_ARROW_ICE);
        }
        else if (it.key().compare("sharedMagicArrowLight") == 0)
        {
            this->SharedItemIDs.insert(OOT_ARROW_LIGHT);
            this->SharedItemIDs.insert(MM_ARROW_LIGHT);
        }
        else if (it.key().compare("sharedSongEpona") == 0)
        {
            this->SharedItemIDs.insert(OOT_SONG_EPONA);
            this->SharedItemIDs.insert(OOT_SONG_NOTE_EPONA);
            this->SharedItemIDs.insert(MM_SONG_EPONA);
            this->SharedItemIDs.insert(MM_SONG_NOTE_EPONA);
        }
        else if (it.key().compare("sharedSongStorms") == 0)
        {
            this->SharedItemIDs.insert(OOT_SONG_STORMS);
            this->SharedItemIDs.insert(OOT_SONG_NOTE_STORMS);
            this->SharedItemIDs.insert(MM_SONG_STORMS);
            this->SharedItemIDs.insert(MM_SONG_NOTE_STORMS);
        }
        else if (it.key().compare("sharedSongTime") == 0)
        {
            this->SharedItemIDs.insert(OOT_SONG_TIME);
            this->SharedItemIDs.insert(OOT_SONG_NOTE_TIME);
            this->SharedItemIDs.insert(MM_SONG_TIME);
            this->SharedItemIDs.insert(MM_SONG_NOTE_TIME);
        }
        else if (it.key().compare("sharedSongSun") == 0)
        {
            this->SharedItemIDs.insert(OOT_SONG_SUN);
            this->SharedItemIDs.insert(OOT_SONG_NOTE_SUN);
            this->SharedItemIDs.insert(MM_SONG_SUN);
            this->SharedItemIDs.insert(MM_SONG_NOTE_SUN);
        }
        else if (it.key().compare("sharedHookshot") == 0)
        {
            this->SharedItemIDs.insert(OOT_HOOKSHOT);
            this->SharedItemIDs.insert(OOT_LONGSHOT);
            this->SharedItemIDs.insert(MM_HOOKSHOT);
            this->SharedItemIDs.insert(MM_HOOKSHOT_SHORT);
        }
        else if (it.key().compare("sharedLens") == 0)
        {
            this->SharedItemIDs.insert(OOT_LENS);
            this->SharedItemIDs.insert(MM_LENS);
        }
        else if (it.key().compare("sharedOcarina") == 0)
        {
            this->SharedItemIDs.insert(OOT_OCARINA_FAIRY);
            this->SharedItemIDs.insert(OOT_OCARINA_TIME);
            this->SharedItemIDs.insert(MM_OCARINA_FAIRY);
            this->SharedItemIDs.insert(MM_OCARINA_OF_TIME);
        }
        else if (it.key().compare("sharedMaskGoron") == 0)
        {
            this->SharedItemIDs.insert(OOT_MASK_GORON);
            this->SharedItemIDs.insert(MM_MASK_GORON);
        }
        else if (it.key().compare("sharedMaskZora") == 0)
        {
            this->SharedItemIDs.insert(OOT_MASK_ZORA);
            this->SharedItemIDs.insert(MM_MASK_ZORA);
        }
        else if (it.key().compare("sharedMaskBunny") == 0)
        {
            this->SharedItemIDs.insert(OOT_MASK_BUNNY);
            this->SharedItemIDs.insert(MM_MASK_BUNNY);
        }
        else if (it.key().compare("sharedMaskKeaton") == 0)
        {
            this->SharedItemIDs.insert(OOT_MASK_KEATON);
            this->SharedItemIDs.insert(MM_MASK_KEATON);
        }
        else if (it.key().compare("sharedMaskTruth") == 0)
        {
            this->SharedItemIDs.insert(OOT_MASK_TRUTH);
            this->SharedItemIDs.insert(MM_MASK_TRUTH);
        }
        else if (it.key().compare("sharedMaskBlast") == 0)
        {
            this->SharedItemIDs.insert(OOT_MASK_BLAST);
            this->SharedItemIDs.insert(MM_MASK_BLAST);
        }
        else if (it.key().compare("sharedMaskStone") == 0)
        {
            this->SharedItemIDs.insert(OOT_MASK_STONE);
            this->SharedItemIDs.insert(MM_MASK_STONE);
        }
        else if (it.key().compare("sharedSongElegy") == 0)
        {
            this->SharedItemIDs.insert(OOT_SONG_EMPTINESS);
            this->SharedItemIDs.insert(OOT_SONG_NOTE_EMPTINESS);
            this->SharedItemIDs.insert(MM_SONG_EMPTINESS);
            this->SharedItemIDs.insert(MM_SONG_NOTE_EMPTINESS);
        }
        else if (it.key().compare("sharedWallets") == 0)
        {
            this->SharedItemIDs.insert(OOT_WALLET);
            this->SharedItemIDs.insert(OOT_WALLET2);
            this->SharedItemIDs.insert(OOT_WALLET3);
            this->SharedItemIDs.insert(OOT_WALLET4);
            this->SharedItemIDs.insert(OOT_WALLET5);
            this->SharedItemIDs.insert(MM_WALLET);
            this->SharedItemIDs.insert(MM_WALLET2);
            this->SharedItemIDs.insert(MM_WALLET3);
            this->SharedItemIDs.insert(MM_WALLET4);
            this->SharedItemIDs.insert(MM_WALLET5);
        }
        else if (it.key().compare("sharedHealth") == 0)
        {
            this->SharedItemIDs.insert(OOT_RECOVERY_HEART);
            this->SharedItemIDs.insert(OOT_TC_HEART_PIECE);
            this->SharedItemIDs.insert(OOT_HEART_PIECE);
            this->SharedItemIDs.insert(OOT_HEART_CONTAINER);
            this->SharedItemIDs.insert(OOT_DEFENSE_UPGRADE);
            this->SharedItemIDs.insert(MM_RECOVERY_HEART);
            this->SharedItemIDs.insert(MM_HEART_PIECE);
            this->SharedItemIDs.insert(MM_HEART_CONTAINER);
            this->SharedItemIDs.insert(MM_DEFENSE_UPGRADE);
        }
        else if (it.key().compare("sharedShields") == 0)
        {
            this->SharedItemIDs.insert(OOT_SHIELD_DEKU);
            this->SharedItemIDs.insert(OOT_SHIELD_HYLIAN);
            this->SharedItemIDs.insert(OOT_SHIELD_MIRROR);
            this->SharedItemIDs.insert(MM_SHIELD_DEKU);
            this->SharedItemIDs.insert(MM_SHIELD_HERO);
            this->SharedItemIDs.insert(MM_SHIELD_MIRROR);
        }
        else if (it.key().compare("sharedSoulsEnemy") == 0)
        {
            this->SharedItemIDs.insert(OOT_SOUL_ENEMY_ARMOS);
            this->SharedItemIDs.insert(OOT_SOUL_ENEMY_BUBBLE);
            this->SharedItemIDs.insert(OOT_SOUL_ENEMY_BEAMOS);
            this->SharedItemIDs.insert(OOT_SOUL_ENEMY_DEKU_BABA);
            this->SharedItemIDs.insert(OOT_SOUL_ENEMY_DEKU_SCRUB);
            this->SharedItemIDs.insert(OOT_SOUL_ENEMY_DODONGO);
            this->SharedItemIDs.insert(OOT_SOUL_ENEMY_FLOORMASTER);
            this->SharedItemIDs.insert(OOT_SOUL_ENEMY_FLYING_POT);
            this->SharedItemIDs.insert(OOT_SOUL_ENEMY_FREEZARD);
            this->SharedItemIDs.insert(OOT_SOUL_ENEMY_GUAY);
            this->SharedItemIDs.insert(OOT_SOUL_ENEMY_IRON_KNUCKLE);
            this->SharedItemIDs.insert(OOT_SOUL_ENEMY_KEESE);
            this->SharedItemIDs.insert(OOT_SOUL_ENEMY_LEEVER);
            this->SharedItemIDs.insert(OOT_SOUL_ENEMY_LIKE_LIKE);
            this->SharedItemIDs.insert(OOT_SOUL_ENEMY_LIZALFOS_DINOLFOS);
            this->SharedItemIDs.insert(OOT_SOUL_ENEMY_OCTOROK);
            this->SharedItemIDs.insert(OOT_SOUL_ENEMY_PEAHAT);
            this->SharedItemIDs.insert(OOT_SOUL_ENEMY_POE);
            this->SharedItemIDs.insert(OOT_SOUL_ENEMY_REDEAD_GIBDO);
            this->SharedItemIDs.insert(OOT_SOUL_ENEMY_SHELL_BLADE);
            this->SharedItemIDs.insert(OOT_SOUL_ENEMY_SKULLTULA);
            this->SharedItemIDs.insert(OOT_SOUL_ENEMY_SKULLWALLTULA);
            this->SharedItemIDs.insert(OOT_SOUL_ENEMY_STALCHILD);
            this->SharedItemIDs.insert(OOT_SOUL_ENEMY_TEKTITE);
            this->SharedItemIDs.insert(OOT_SOUL_ENEMY_THIEVES);
            this->SharedItemIDs.insert(OOT_SOUL_ENEMY_WALLMASTER);
            this->SharedItemIDs.insert(OOT_SOUL_ENEMY_WOLFOS);
            this->SharedItemIDs.insert(MM_SOUL_ENEMY_ARMOS);
            this->SharedItemIDs.insert(MM_SOUL_ENEMY_BUBBLE);
            this->SharedItemIDs.insert(MM_SOUL_ENEMY_BEAMOS);
            this->SharedItemIDs.insert(MM_SOUL_ENEMY_DEKU_BABA);
            this->SharedItemIDs.insert(MM_SOUL_ENEMY_DEKU_SCRUB);
            this->SharedItemIDs.insert(MM_SOUL_ENEMY_DODONGO);
            this->SharedItemIDs.insert(MM_SOUL_ENEMY_FLOORMASTER);
            this->SharedItemIDs.insert(MM_SOUL_ENEMY_FLYING_POT);
            this->SharedItemIDs.insert(MM_SOUL_ENEMY_FREEZARD);
            this->SharedItemIDs.insert(MM_SOUL_ENEMY_GUAY);
            this->SharedItemIDs.insert(MM_SOUL_ENEMY_IRON_KNUCKLE);
            this->SharedItemIDs.insert(MM_SOUL_ENEMY_KEESE);
            this->SharedItemIDs.insert(MM_SOUL_ENEMY_LEEVER);
            this->SharedItemIDs.insert(MM_SOUL_ENEMY_LIKE_LIKE);
            this->SharedItemIDs.insert(MM_SOUL_ENEMY_LIZALFOS_DINOLFOS);
            this->SharedItemIDs.insert(MM_SOUL_ENEMY_OCTOROK);
            this->SharedItemIDs.insert(MM_SOUL_ENEMY_PEAHAT);
            this->SharedItemIDs.insert(MM_SOUL_ENEMY_POE);
            this->SharedItemIDs.insert(MM_SOUL_ENEMY_REDEAD_GIBDO);
            this->SharedItemIDs.insert(MM_SOUL_ENEMY_SHELL_BLADE);
            this->SharedItemIDs.insert(MM_SOUL_ENEMY_SKULLTULA);
            this->SharedItemIDs.insert(MM_SOUL_ENEMY_SKULLWALLTULA);
            this->SharedItemIDs.insert(MM_SOUL_ENEMY_STALCHILD);
            this->SharedItemIDs.insert(MM_SOUL_ENEMY_TEKTITE);
            this->SharedItemIDs.insert(MM_SOUL_ENEMY_THIEVES);
            this->SharedItemIDs.insert(MM_SOUL_ENEMY_WALLMASTER);
            this->SharedItemIDs.insert(MM_SOUL_ENEMY_WOLFOS);
        }
        else if (it.key().compare("sharedSoulsNpc") == 0)
        {
            this->SharedItemIDs.insert(OOT_SOUL_NPC_ANJU);
            this->SharedItemIDs.insert(OOT_SOUL_NPC_ASTRONOMER);
            this->SharedItemIDs.insert(OOT_SOUL_NPC_BANKER);
            this->SharedItemIDs.insert(OOT_SOUL_NPC_BAZAAR_SHOPKEEPER);
            this->SharedItemIDs.insert(OOT_SOUL_NPC_BEAN_SALESMAN);
            this->SharedItemIDs.insert(OOT_SOUL_NPC_BIGGORON);
            this->SharedItemIDs.insert(OOT_SOUL_NPC_BOMBCHU_BOWLING_LADY);
            this->SharedItemIDs.insert(OOT_SOUL_NPC_BOMBCHU_SHOPKEEPER);
            this->SharedItemIDs.insert(OOT_SOUL_NPC_BOMBERS);
            this->SharedItemIDs.insert(OOT_SOUL_NPC_CARPENTERS);
            this->SharedItemIDs.insert(OOT_SOUL_NPC_CARPET_MAN);
            this->SharedItemIDs.insert(OOT_SOUL_NPC_CHEST_GAME_OWNER);
            this->SharedItemIDs.insert(OOT_SOUL_NPC_CITIZEN);
            this->SharedItemIDs.insert(OOT_SOUL_NPC_COMPOSER_BROS);
            this->SharedItemIDs.insert(OOT_SOUL_NPC_DAMPE);
            this->SharedItemIDs.insert(OOT_SOUL_NPC_DOG_LADY);
            this->SharedItemIDs.insert(OOT_SOUL_NPC_FISHING_POND_OWNER);
            this->SharedItemIDs.insert(OOT_SOUL_NPC_GORMAN);
            this->SharedItemIDs.insert(OOT_SOUL_NPC_GORON);
            this->SharedItemIDs.insert(OOT_SOUL_NPC_GORON_CHILD);
            this->SharedItemIDs.insert(OOT_SOUL_NPC_GORON_SHOPKEEPER);
            this->SharedItemIDs.insert(OOT_SOUL_NPC_GROG);
            this->SharedItemIDs.insert(OOT_SOUL_NPC_GURU_GURU);
            this->SharedItemIDs.insert(OOT_SOUL_NPC_HONEY_DARLING);
            this->SharedItemIDs.insert(OOT_SOUL_NPC_MALON);
            this->SharedItemIDs.insert(OOT_SOUL_NPC_MEDIGORON);
            this->SharedItemIDs.insert(OOT_SOUL_NPC_OLD_HAG);
            this->SharedItemIDs.insert(OOT_SOUL_NPC_POE_COLLECTOR);
            this->SharedItemIDs.insert(OOT_SOUL_NPC_ROOFTOP_MAN);
            this->SharedItemIDs.insert(OOT_SOUL_NPC_RUTO);
            this->SharedItemIDs.insert(OOT_SOUL_NPC_SCIENTIST);
            this->SharedItemIDs.insert(OOT_SOUL_NPC_SHOOTING_GALLERY_OWNER);
            this->SharedItemIDs.insert(OOT_SOUL_NPC_TALON);
            this->SharedItemIDs.insert(OOT_SOUL_NPC_THIEVES);
            this->SharedItemIDs.insert(OOT_SOUL_NPC_ZORA);
            this->SharedItemIDs.insert(OOT_SOUL_NPC_ZORA_SHOPKEEPER);
            this->SharedItemIDs.insert(MM_SOUL_NPC_ANJU);
            this->SharedItemIDs.insert(MM_SOUL_NPC_ASTRONOMER);
            this->SharedItemIDs.insert(MM_SOUL_NPC_BANKER);
            this->SharedItemIDs.insert(MM_SOUL_NPC_BAZAAR_SHOPKEEPER);
            this->SharedItemIDs.insert(MM_SOUL_NPC_BEAN_SALESMAN);
            this->SharedItemIDs.insert(MM_SOUL_NPC_BIGGORON);
            this->SharedItemIDs.insert(MM_SOUL_NPC_BOMBCHU_BOWLING_LADY);
            this->SharedItemIDs.insert(MM_SOUL_NPC_BOMBCHU_SHOPKEEPER);
            this->SharedItemIDs.insert(MM_SOUL_NPC_BOMBERS);
            this->SharedItemIDs.insert(MM_SOUL_NPC_CARPENTERS);
            this->SharedItemIDs.insert(MM_SOUL_NPC_CARPET_MAN);
            this->SharedItemIDs.insert(MM_SOUL_NPC_CHEST_GAME_OWNER);
            this->SharedItemIDs.insert(MM_SOUL_NPC_CITIZEN);
            this->SharedItemIDs.insert(MM_SOUL_NPC_COMPOSER_BROS);
            this->SharedItemIDs.insert(MM_SOUL_NPC_DAMPE);
            this->SharedItemIDs.insert(MM_SOUL_NPC_DOG_LADY);
            this->SharedItemIDs.insert(MM_SOUL_NPC_FISHING_POND_OWNER);
            this->SharedItemIDs.insert(MM_SOUL_NPC_GORMAN);
            this->SharedItemIDs.insert(MM_SOUL_NPC_GORON);
            this->SharedItemIDs.insert(MM_SOUL_NPC_GORON_CHILD);
            this->SharedItemIDs.insert(MM_SOUL_NPC_GORON_SHOPKEEPER);
            this->SharedItemIDs.insert(MM_SOUL_NPC_GROG);
            this->SharedItemIDs.insert(MM_SOUL_NPC_GURU_GURU);
            this->SharedItemIDs.insert(MM_SOUL_NPC_HONEY_DARLING);
            this->SharedItemIDs.insert(MM_SOUL_NPC_MALON);
            this->SharedItemIDs.insert(MM_SOUL_NPC_MEDIGORON);
            this->SharedItemIDs.insert(MM_SOUL_NPC_OLD_HAG);
            this->SharedItemIDs.insert(MM_SOUL_NPC_POE_COLLECTOR);
            this->SharedItemIDs.insert(MM_SOUL_NPC_ROOFTOP_MAN);
            this->SharedItemIDs.insert(MM_SOUL_NPC_RUTO);
            this->SharedItemIDs.insert(MM_SOUL_NPC_SCIENTIST);
            this->SharedItemIDs.insert(MM_SOUL_NPC_SHOOTING_GALLERY_OWNER);
            this->SharedItemIDs.insert(MM_SOUL_NPC_TALON);
            this->SharedItemIDs.insert(MM_SOUL_NPC_THIEVES);
            this->SharedItemIDs.insert(MM_SOUL_NPC_ZORA);
            this->SharedItemIDs.insert(MM_SOUL_NPC_ZORA_SHOPKEEPER);
        }
        else if (it.key().compare("sharedSoulsAnimal") == 0)
        {
            this->SharedItemIDs.insert(OOT_SOUL_ANIMAL_CUCCO);
            this->SharedItemIDs.insert(OOT_SOUL_ANIMAL_COW);
            this->SharedItemIDs.insert(OOT_SOUL_ANIMAL_DOG);
            this->SharedItemIDs.insert(OOT_SOUL_ANIMAL_BUTTERFLY);
            this->SharedItemIDs.insert(MM_SOUL_ANIMAL_CUCCO);
            this->SharedItemIDs.insert(MM_SOUL_ANIMAL_COW);
            this->SharedItemIDs.insert(MM_SOUL_ANIMAL_DOG);
            this->SharedItemIDs.insert(MM_SOUL_ANIMAL_BUTTERFLY);
        }
        else if (it.key().compare("sharedSoulsMisc") == 0)
        {
            this->SharedItemIDs.insert(OOT_SOUL_MISC_GS);
            this->SharedItemIDs.insert(OOT_SOUL_MISC_BUSINESS_SCRUB);
            this->SharedItemIDs.insert(MM_SOUL_MISC_GS);
            this->SharedItemIDs.insert(MM_SOUL_MISC_BUSINESS_SCRUB);
        }
        else if (it.key().compare("sharedOcarinaButtons") == 0)
        {
            this->SharedItemIDs.insert(OOT_BUTTON_A);
            this->SharedItemIDs.insert(OOT_BUTTON_C_LEFT);
            this->SharedItemIDs.insert(OOT_BUTTON_C_RIGHT);
            this->SharedItemIDs.insert(OOT_BUTTON_C_UP);
            this->SharedItemIDs.insert(OOT_BUTTON_C_DOWN);
            this->SharedItemIDs.insert(MM_BUTTON_A);
            this->SharedItemIDs.insert(MM_BUTTON_C_LEFT);
            this->SharedItemIDs.insert(MM_BUTTON_C_RIGHT);
            this->SharedItemIDs.insert(MM_BUTTON_C_UP);
            this->SharedItemIDs.insert(MM_BUTTON_C_DOWN);
        }
        else if (it.key().compare("sharedSkeletonKey") == 0)
        {
            this->SharedItemIDs.insert(OOT_SKELETON_KEY);
            this->SharedItemIDs.insert(MM_SKELETON_KEY);
        }
        else if (it.key().compare("sharedPlatinumToken") == 0)
        {
            this->SharedItemIDs.insert(OOT_PLATINUM_TOKEN);
            this->SharedItemIDs.insert(MM_PLATINUM_TOKEN);
        }
        else if (it.key().compare("sharedSpellFire") == 0)
        {
            this->SharedItemIDs.insert(OOT_SPELL_FIRE);
            this->SharedItemIDs.insert(MM_SPELL_FIRE);
        }
        else if (it.key().compare("sharedSpellWind") == 0)
        {
            this->SharedItemIDs.insert(OOT_SPELL_WIND);
            this->SharedItemIDs.insert(MM_SPELL_WIND);
        }
        else if (it.key().compare("sharedSpellLove") == 0)
        {
            this->SharedItemIDs.insert(OOT_SPELL_LOVE);
            this->SharedItemIDs.insert(MM_SPELL_LOVE);
        }
        else if (it.key().compare("sharedBootsIron") == 0)
        {
            this->SharedItemIDs.insert(OOT_BOOTS_IRON);
            this->SharedItemIDs.insert(MM_BOOTS_IRON);
        }
        else if (it.key().compare("sharedBootsHover") == 0)
        {
            this->SharedItemIDs.insert(OOT_BOOTS_HOVER);
            this->SharedItemIDs.insert(MM_BOOTS_HOVER);
        }
        else if (it.key().compare("sharedTunicGoron") == 0)
        {
            this->SharedItemIDs.insert(OOT_TUNIC_GORON);
            this->SharedItemIDs.insert(MM_TUNIC_GORON);
        }
        else if (it.key().compare("sharedTunicZora") == 0)
        {
            this->SharedItemIDs.insert(OOT_TUNIC_ZORA);
            this->SharedItemIDs.insert(MM_TUNIC_ZORA);
        }
        else if (it.key().compare("sharedScales") == 0)
        {
            this->SharedItemIDs.insert(OOT_SCALE_BRONZE);
            this->SharedItemIDs.insert(OOT_SCALE_SILVER);
            this->SharedItemIDs.insert(OOT_SCALE_GOLDEN);
            this->SharedItemIDs.insert(MM_SCALE_BRONZE);
            this->SharedItemIDs.insert(MM_SCALE_SILVER);
            this->SharedItemIDs.insert(MM_SCALE_GOLDEN);
        }
        else if (it.key().compare("sharedStrength") == 0)
        {
            this->SharedItemIDs.insert(OOT_GORON_BRACELET);
            this->SharedItemIDs.insert(OOT_SILVER_GAUNTLETS);
            this->SharedItemIDs.insert(OOT_GOLDEN_GAUNTLETS);
            this->SharedItemIDs.insert(MM_GORON_BRACELET);
            this->SharedItemIDs.insert(MM_SILVER_GAUNTLETS);
            this->SharedItemIDs.insert(MM_GOLDEN_GAUNTLETS);
        }
        else if (it.key().compare("sharedHammer") == 0)
        {
            this->SharedItemIDs.insert(OOT_HAMMER);
            this->SharedItemIDs.insert(MM_HAMMER);
        }
        else if (it.key().compare("sharedBottles") == 0)
        {
            this->SharedItemIDs.insert(OOT_BOTTLE_RUTO_LETTER);
            this->SharedItemIDs.insert(OOT_BOTTLE_BIG_POE);
            this->SharedItemIDs.insert(OOT_BOTTLE_BLUE_FIRE);
            this->SharedItemIDs.insert(OOT_BOTTLE_CHATEAU);
            this->SharedItemIDs.insert(OOT_BOTTLE_EMPTY);
            this->SharedItemIDs.insert(OOT_BOTTLE_FAIRY);
            this->SharedItemIDs.insert(OOT_BOTTLE_MILK);
            this->SharedItemIDs.insert(OOT_BOTTLE_POE);
            this->SharedItemIDs.insert(OOT_BOTTLE_POTION_BLUE);
            this->SharedItemIDs.insert(OOT_BOTTLE_POTION_GREEN);
            this->SharedItemIDs.insert(OOT_BOTTLE_POTION_RED);
            this->SharedItemIDs.insert(OOT_BOTTLED_GOLD_DUST);
            this->SharedItemIDs.insert(OOT_WEIRD_MUSHROOM);
            this->SharedItemIDs.insert(OOT_SEAHORSE2);
            this->SharedItemIDs.insert(MM_BOTTLE_RUTO_LETTER);
            this->SharedItemIDs.insert(MM_BOTTLE_BIG_POE);
            this->SharedItemIDs.insert(MM_BOTTLE_BLUE_FIRE);
            this->SharedItemIDs.insert(MM_BOTTLE_CHATEAU);
            this->SharedItemIDs.insert(MM_BOTTLE_EMPTY);
            this->SharedItemIDs.insert(MM_BOTTLE_FAIRY);
            this->SharedItemIDs.insert(MM_BOTTLE_MILK);
            this->SharedItemIDs.insert(MM_BOTTLE_POE);
            this->SharedItemIDs.insert(MM_BOTTLE_POTION_BLUE);
            this->SharedItemIDs.insert(MM_BOTTLE_POTION_GREEN);
            this->SharedItemIDs.insert(MM_BOTTLE_POTION_RED);
            this->SharedItemIDs.insert(MM_BOTTLED_GOLD_DUST);
            this->SharedItemIDs.insert(MM_WEIRD_MUSHROOM);
            this->SharedItemIDs.insert(MM_SEAHORSE2);
        }
    }
}

void Settings::CheckObjectExclusion(ObjectInfo* ToCheck, ShuffleSetting SettingValue, FilterManager* Filter)
{
	switch (SettingValue)
	{
        case ShuffleSetting::removed:
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


bool Settings::CheckItemEnabled(ShuffleSetting SettingValue, uint32_t ItemID)
{
    switch (SettingValue)
    {
        case ShuffleSetting::vanilla:
        {
            this->DisabledItemIDs.insert(ItemID);
            return false;
        }
    }

    return true;
}


SceneObjects* Settings::ApplySettingsToFilter(FilterManager* Filter, uint32_t Game)
{
	Filter->ResetExcludedObject();
	return GetGameSceneObjects(Game);
}
