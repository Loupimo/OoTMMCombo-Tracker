#include "UI/SettingsTab.h"
#include "UI/OoTMMComboTracker.h"
#include "UI/AppConfig.h"
#include "Combo/Items.h"
#include "Combo/Objects.h"
#include "Combo/Scenes.h"
#include "Multi/Game.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QScrollArea>
#include <QFrame>
#include <QFileDialog>
#include <QIcon>


namespace
{
    /*
    *   Per-dungeon Ocarina of Time small key / key ring item id pairs, mirroring the
    *   "Small Key Ring (OoT)" parser table in Settings::ParseKeyRings.
    */
    struct DungeonItemPair
    {
        const char* Label;
        uint32_t SmallId;
        uint32_t RingId;
    };

    static const DungeonItemPair OoTKeyRings[] =
    {
        { "Forest Temple",            OOT_SMALL_KEY_FOREST, OOT_KEY_RING_FOREST },
        { "Fire Temple",              OOT_SMALL_KEY_FIRE,   OOT_KEY_RING_FIRE   },
        { "Water Temple",             OOT_SMALL_KEY_WATER,  OOT_KEY_RING_WATER  },
        { "Shadow Temple",            OOT_SMALL_KEY_SHADOW, OOT_KEY_RING_SHADOW },
        { "Spirit Temple",            OOT_SMALL_KEY_SPIRIT, OOT_KEY_RING_SPIRIT },
        { "Bottom of the Well",       OOT_SMALL_KEY_BOTW,   OOT_KEY_RING_BOTW   },
        { "Gerudo Training Grounds",  OOT_SMALL_KEY_GTG,    OOT_KEY_RING_GTG    },
        { "Ganon's Castle",           OOT_SMALL_KEY_GANON,  OOT_KEY_RING_GANON  },
        { "Hideout",                  OOT_SMALL_KEY_GF,     OOT_KEY_RING_GF     },
        { "Chest Game",               OOT_SMALL_KEY_TCG,    OOT_KEY_RING_TCG    }
    };

    /*
    *   Per-dungeon Majora's Mask small key / key ring item id pairs, mirroring the
    *   "Small Key Ring (MM)" parser table in Settings::ParseKeyRings.
    */
    static const DungeonItemPair MMKeyRings[] =
    {
        { "Woodfall Temple",     MM_SMALL_KEY_WF, MM_KEY_RING_WF },
        { "Snowhead Temple",     MM_SMALL_KEY_SH, MM_KEY_RING_SH },
        { "Great Bay Temple",    MM_SMALL_KEY_GB, MM_KEY_RING_GB },
        { "Stone Tower Temple",  MM_SMALL_KEY_ST, MM_KEY_RING_ST }
    };

    /*
    *   Per-area Ocarina of Time silver rupee / silver pouch item id pairs, mirroring the
    *   "Silver Rupee Pouches" parser table in Settings::ParseSilverPouches.
    */
    static const DungeonItemPair OoTSilverPouches[] =
    {
        { "Dodongo's Cavern",          OOT_RUPEE_SILVER_DC,              OOT_POUCH_SILVER_DC              },
        { "Bottom of the Well",        OOT_RUPEE_SILVER_BOTW,            OOT_POUCH_SILVER_BOTW            },
        { "Spirit Temple (Child)",     OOT_RUPEE_SILVER_SPIRIT_CHILD,    OOT_POUCH_SILVER_SPIRIT_CHILD    },
        { "Spirit Temple (Sun)",       OOT_RUPEE_SILVER_SPIRIT_SUN,      OOT_POUCH_SILVER_SPIRIT_SUN      },
        { "Spirit Temple (Boulders)",  OOT_RUPEE_SILVER_SPIRIT_BOULDERS, OOT_POUCH_SILVER_SPIRIT_BOULDERS },
        { "Spirit Temple (Lobby)",     OOT_RUPEE_SILVER_SPIRIT_LOBBY,    OOT_POUCH_SILVER_SPIRIT_LOBBY    },
        { "Spirit Temple (Adult)",     OOT_RUPEE_SILVER_SPIRIT_ADULT,    OOT_POUCH_SILVER_SPIRIT_ADULT    },
        { "Shadow Temple (Scythe)",    OOT_RUPEE_SILVER_SHADOW_SCYTHE,   OOT_POUCH_SILVER_SHADOW_SCYTHE   },
        { "Shadow Temple (Pit)",       OOT_RUPEE_SILVER_SHADOW_PIT,      OOT_POUCH_SILVER_SHADOW_PIT      },
        { "Shadow Temple (Spikes)",    OOT_RUPEE_SILVER_SHADOW_SPIKES,   OOT_POUCH_SILVER_SHADOW_SPIKES   },
        { "Shadow Temple (Blades)",    OOT_RUPEE_SILVER_SHADOW_BLADES,   OOT_POUCH_SILVER_SHADOW_BLADES   },
        { "Ice Cavern (Scythe)",       OOT_RUPEE_SILVER_IC_SCYTHE,       OOT_POUCH_SILVER_IC_SCYTHE       },
        { "Ice Cavern (Block)",        OOT_RUPEE_SILVER_IC_BLOCK,        OOT_POUCH_SILVER_IC_BLOCK        },
        { "GTG (Slopes)",              OOT_RUPEE_SILVER_GTG_SLOPES,      OOT_POUCH_SILVER_GTG_SLOPES      },
        { "GTG (Lava)",                OOT_RUPEE_SILVER_GTG_LAVA,        OOT_POUCH_SILVER_GTG_LAVA        },
        { "GTG (Water)",               OOT_RUPEE_SILVER_GTG_WATER,       OOT_POUCH_SILVER_GTG_WATER       },
        { "Ganon's Castle (Light)",    OOT_RUPEE_SILVER_GANON_LIGHT,     OOT_POUCH_SILVER_GANON_LIGHT     },
        { "Ganon's Castle (Forest)",   OOT_RUPEE_SILVER_GANON_FOREST,    OOT_POUCH_SILVER_GANON_FOREST    },
        { "Ganon's Castle (Fire)",     OOT_RUPEE_SILVER_GANON_FIRE,      OOT_POUCH_SILVER_GANON_FIRE      },
        { "Ganon's Castle (Water)",    OOT_RUPEE_SILVER_GANON_WATER,     OOT_POUCH_SILVER_GANON_WATER     },
        { "Ganon's Castle (Shadow)",   OOT_RUPEE_SILVER_GANON_SHADOW,    OOT_POUCH_SILVER_GANON_SHADOW    },
        { "Ganon's Castle (Spirit)",   OOT_RUPEE_SILVER_GANON_SPIRIT,    OOT_POUCH_SILVER_GANON_SPIRIT    }
    };

    struct OwlChoice
    {
        const char* Label;
        uint32_t ItemId;
    };

    /*
    *   Pre-activated owl statues, mirroring the table in Settings::ParsePreActivatedOwl.
    */
    static const OwlChoice MMOwlStatues[] =
    {
        { "Clock Town",        MM_OWL_CLOCK_TOWN       },
        { "Milk Road",         MM_OWL_MILK_ROAD        },
        { "Southern Swamp",    MM_OWL_SOUTHERN_SWAMP   },
        { "Woodfall",          MM_OWL_WOODFALL         },
        { "Mountain Village",  MM_OWL_MOUNTAIN_VILLAGE },
        { "Snowhead",          MM_OWL_SNOWHEAD         },
        { "Great Bay Coast",   MM_OWL_GREAT_BAY        },
        { "Zora Cape",         MM_OWL_ZORA_CAPE        },
        { "Ikana Canyon",      MM_OWL_IKANA_CANYON     },
        { "Stone Tower",       MM_OWL_STONE_TOWER      }
    };

    struct MQScene
    {
        const char* Label;
        uint32_t SceneId;
    };

    /*
    *   Scenes that own a Master Quest layout, mirroring the cases in Settings::ParseGamesLayouts.
    */
    static const MQScene OoTMQScenes[] =
    {
        { "Deku Tree",               OOT_DEKU_TREE              },
        { "Dodongo's Cavern",        OOT_DODONGO_CAVERN         },
        { "Jabu-Jabu",               OOT_INSIDE_JABU_JABU       },
        { "Forest Temple",           OOT_TEMPLE_FOREST          },
        { "Fire Temple",             OOT_TEMPLE_FIRE            },
        { "Water Temple",            OOT_TEMPLE_WATER           },
        { "Shadow Temple",           OOT_TEMPLE_SHADOW          },
        { "Spirit Temple",           OOT_TEMPLE_SPIRIT          },
        { "Bottom of the Well",      OOT_BOTTOM_OF_THE_WELL     },
        { "Ice Cavern",              OOT_ICE_CAVERN             },
        { "Gerudo Training Grounds", OOT_GERUDO_TRAINING_GROUND },
        { "Ganon's Castle",          OOT_INSIDE_GANON_CASTLE    }
    };
}


#pragma region Class creation

SettingsTab::SettingsTab(OoTMMComboTracker* Owner, QWidget* Parent)
    : QDialog(Parent ? Parent : Owner), WinOwner(Owner)
{
    this->setWindowTitle("ROM Settings");
    this->setModal(true);
    this->resize(900, 620);

    // Main layout: navigation list on the left (fixed 200px) + page stack on the right.
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Navigation column ---------------------------------------------------
    QWidget* navColumn = new QWidget(this);
    navColumn->setFixedWidth(200);
    QVBoxLayout* navLayout = new QVBoxLayout(navColumn);
    navLayout->setContentsMargins(8, 8, 8, 8);
    navLayout->setSpacing(8);

    this->NavList = new QListWidget(navColumn);
    this->NavList->addItem("General");
    this->NavList->addItem("Keys & Dungeons");
    this->NavList->addItem("NPC & Shops");
    this->NavList->addItem("Breakables");
    this->NavList->addItem("Special");
    this->NavList->addItem("Progressive Items");
    this->NavList->addItem("Shared Items");
    this->NavList->addItem("Songs");
    this->NavList->addItem("World Items");
    this->NavList->addItem("MQ / JP Layouts");
    this->NavList->setCurrentRow(0);
    navLayout->addWidget(this->NavList, 1);

    this->LoadSpoilerButton = new QPushButton("Load Spoiler Log", navColumn);
    this->LoadSpoilerButton->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::SystemLogOut));
    this->ApplyButton = new QPushButton("Apply", navColumn);
    this->CancelButton = new QPushButton("Cancel", navColumn);
    navLayout->addWidget(this->LoadSpoilerButton);
    navLayout->addWidget(this->ApplyButton);
    navLayout->addWidget(this->CancelButton);

    // Right side stack ----------------------------------------------------
    this->Pages = new QStackedWidget(this);
    this->Pages->addWidget(this->BuildGeneralPage());
    this->Pages->addWidget(this->BuildKeysPage());
    this->Pages->addWidget(this->BuildNpcPage());
    this->Pages->addWidget(this->BuildBreakablePage());
    this->Pages->addWidget(this->BuildSpecialPage());
    this->Pages->addWidget(this->BuildProgressiveItemsPage());
    this->Pages->addWidget(this->BuildSharedItemsPage());
    this->Pages->addWidget(this->BuildSongsPage());
    this->Pages->addWidget(this->BuildWorldItemsPage());
    this->Pages->addWidget(this->BuildLayoutsPage());

    // Vertical separator between the nav and the stack so the nav reads as a side panel.
    QFrame* sep = new QFrame(this);
    sep->setFrameShape(QFrame::VLine);
    sep->setStyleSheet("color: #1a3050;");

    mainLayout->addWidget(navColumn);
    mainLayout->addWidget(sep);
    mainLayout->addWidget(this->Pages, 1);

    // Wiring --------------------------------------------------------------
    connect(this->NavList, &QListWidget::currentRowChanged, this->Pages, &QStackedWidget::setCurrentIndex);
    connect(this->ApplyButton, &QPushButton::clicked, this, [this]()
    {
        this->OnApply();
        this->accept();
    });
    connect(this->CancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(this->LoadSpoilerButton, &QPushButton::clicked, this, &SettingsTab::OnLoadSpoiler);

    // Initial values populated from the owner ROMSettings.
    this->LoadFromSettings();
}

#pragma endregion

#pragma region Pages building

QLabel* SettingsTab::MakeGameBadge(const QString& GameLabel)
{
    if (GameLabel.isEmpty()) return nullptr;

    QLabel* badge = new QLabel(GameLabel);
    badge->setFixedWidth(28);
    badge->setAlignment(Qt::AlignCenter);

    if (GameLabel == "OoT")
    {
        badge->setStyleSheet(
            "background-color: rgba(74, 158, 219, 24);"
            "color: #4a9edb;"
            "border: 1px solid #4a9edb;"
            "border-radius: 3px;"
            "font-size: 9px;"
            "font-weight: 700;"
            "padding: 1px 2px;");
    }
    else
    {
        badge->setStyleSheet(
            "background-color: rgba(155, 93, 229, 24);"
            "color: #9b5de5;"
            "border: 1px solid #9b5de5;"
            "border-radius: 3px;"
            "font-size: 9px;"
            "font-weight: 700;"
            "padding: 1px 2px;");
    }

    return badge;
}


Parameter* SettingsTab::FindParameter(const QString& Key)
{
    if (this->WinOwner == nullptr) return nullptr;

    auto& filterSettings = this->WinOwner->ROMSettings.FilterSettings;
    auto fIt = filterSettings.find(Key);
    if (fIt != filterSettings.end()) return &fIt.value();

    auto& itemSettings = this->WinOwner->ROMSettings.ItemSettings;
    auto iIt = itemSettings.find(Key);
    if (iIt != itemSettings.end()) return &iIt.value();

    return nullptr;
}


void SettingsTab::AddParamRow(QGridLayout* Layout, const QString& Key, const QString& GameLabel)
{
    Parameter* param = this->FindParameter(Key);
    if (param == nullptr) return;

    int row = Layout->rowCount();

    // Game badge (column 0).
    QLabel* badge = this->MakeGameBadge(GameLabel);
    if (badge != nullptr)
    {
        Layout->addWidget(badge, row, 0, Qt::AlignVCenter | Qt::AlignLeft);
    }

    // Setting display name (column 1).
    QLabel* nameLabel = new QLabel(param->Name);
    nameLabel->setStyleSheet("color: #ddeeff;");
    Layout->addWidget(nameLabel, row, 1);

    // Editor widget (column 2).
    QWidget* editor = nullptr;
    switch (param->Type)
    {
        case ParamType::shuffle:
        {
            // Every value of the ShuffleSetting enum must be reachable from the combo, otherwise
            // findData() returns -1 when re-opening the dialog after a spoiler-driven 'removed' or
            // 'starting' was set, the combo silently falls back to index 0 (Vanilla), and Apply
            // then clobbers the original value with Vanilla.
            QComboBox* combo = new QComboBox();
            combo->addItem("Vanilla", static_cast<int>(ShuffleSetting::vanilla));
            combo->addItem("Removed", static_cast<int>(ShuffleSetting::removed));
            combo->addItem("Starting", static_cast<int>(ShuffleSetting::starting));
            combo->addItem("Dungeons", static_cast<int>(ShuffleSetting::dungeons));
            combo->addItem("Overworld", static_cast<int>(ShuffleSetting::overworld));
            combo->addItem("Anywhere", static_cast<int>(ShuffleSetting::all));
            int idx = combo->findData(static_cast<int>(param->Value));
            if (idx >= 0) combo->setCurrentIndex(idx);
            editor = combo;
            break;
        }

        case ParamType::boolean:
        case ParamType::uint:
        {
            QCheckBox* check = new QCheckBox();
            check->setChecked(param->Value == ShuffleSetting::all);
            editor = check;
            break;
        }

        default:
        {
            // Unsupported types fall back to a disabled label so the row keeps its layout.
            QLabel* placeholder = new QLabel("-");
            placeholder->setStyleSheet("color: #4a6a9a;");
            editor = placeholder;
            break;
        }
    }

    Layout->addWidget(editor, row, 2);
    this->ParamWidgets.insert(Key, editor);
}


QWidget* SettingsTab::BuildGeneralPage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* vbox = new QVBoxLayout(page);
    vbox->setContentsMargins(16, 16, 16, 16);
    vbox->setSpacing(14);

    // Game group ---------------------------------------------------------
    QGroupBox* gameBox = new QGroupBox("Game", page);
    QHBoxLayout* gameLayout = new QHBoxLayout(gameBox);
    this->GameGroup = new QButtonGroup(gameBox);

    QRadioButton* gameOot = new QRadioButton("Ocarina of Time", gameBox);
    QRadioButton* gameMm = new QRadioButton("Majora's Mask", gameBox);
    QRadioButton* gameBoth = new QRadioButton("OoT + MM", gameBox);

    this->GameGroup->addButton(gameOot, static_cast<int>(ROMGame::oot));
    this->GameGroup->addButton(gameMm, static_cast<int>(ROMGame::mm));
    this->GameGroup->addButton(gameBoth, static_cast<int>(ROMGame::ootmm));

    gameLayout->addWidget(gameOot);
    gameLayout->addWidget(gameMm);
    gameLayout->addWidget(gameBoth);
    gameLayout->addStretch(1);
    vbox->addWidget(gameBox);

    // Mode group ---------------------------------------------------------
    QGroupBox* modeBox = new QGroupBox("Mode", page);
    QHBoxLayout* modeLayout = new QHBoxLayout(modeBox);
    this->ModeGroup = new QButtonGroup(modeBox);

    QRadioButton* modeSingle = new QRadioButton("Single", modeBox);
    QRadioButton* modeCoop = new QRadioButton("Coop", modeBox);
    QRadioButton* modeMulti = new QRadioButton("Multi", modeBox);

    this->ModeGroup->addButton(modeSingle, static_cast<int>(GameMode::single));
    this->ModeGroup->addButton(modeCoop, static_cast<int>(GameMode::coop));
    this->ModeGroup->addButton(modeMulti, static_cast<int>(GameMode::multi));

    modeLayout->addWidget(modeSingle);
    modeLayout->addWidget(modeCoop);
    modeLayout->addWidget(modeMulti);
    modeLayout->addStretch(1);
    vbox->addWidget(modeBox);

    // Teams --------------------------------------------------------------
    QGroupBox* teamsBox = new QGroupBox("Teams", page);
    QHBoxLayout* teamsLayout = new QHBoxLayout(teamsBox);
    QLabel* teamsLabel = new QLabel("Number of teams", teamsBox);
    this->TeamsSpin = new QSpinBox(teamsBox);
    this->TeamsSpin->setRange(1, 64);
    teamsLayout->addWidget(teamsLabel);
    teamsLayout->addWidget(this->TeamsSpin);
    teamsLayout->addStretch(1);
    vbox->addWidget(teamsBox);

    // Local world / player -----------------------------------------------
    // Note: there is no "my world" selector. With every world tracked, the local world is
    // auto-detected from the network stream (the from/to ids the game provides), so items are
    // routed to the right world's map / progression without any manual setup.

    vbox->addStretch(1);
    return page;
}


QGroupBox* SettingsTab::MakeParamGroup(QWidget* Parent, const QString& Title, std::initializer_list<QPair<QString, QString>> Params)
{
    // Common factory for "bubble" param groups: a titled group box hosting a 3-column grid
    // (badge | label | editor). Kept central so every page shares the same paddings, spacings
    // and column stretch — without this each page slowly drifts.
    QGroupBox* box = new QGroupBox(Title, Parent);
    QGridLayout* grid = new QGridLayout(box);
    grid->setColumnStretch(1, 1);
    grid->setHorizontalSpacing(10);
    grid->setVerticalSpacing(6);

    for (const QPair<QString, QString>& entry : Params)
    {
        this->AddParamRow(grid, entry.first, entry.second);
    }
    return box;
}


QWidget* SettingsTab::BuildKeysPage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* vbox = new QVBoxLayout(page);
    vbox->setContentsMargins(16, 16, 16, 16);
    vbox->setSpacing(14);

    QScrollArea* scroll = new QScrollArea(page);
    scroll->setWidgetResizable(true);
    QWidget* content = new QWidget();
    QVBoxLayout* contentLayout = new QVBoxLayout(content);
    contentLayout->setSpacing(14);

    contentLayout->addWidget(this->MakeParamGroup(content, "Small Keys", {
        { "smallKeyShuffleOot",       "OoT" },
        { "smallKeyShuffleMm",        "MM"  },
        { "smallKeyShuffleHideout",   "OoT" },
        { "smallKeyShuffleChestGame", "OoT" },
    }));
    contentLayout->addWidget(this->MakeParamGroup(content, "Boss Keys", {
        { "bossKeyShuffleOot", "OoT" },
        { "bossKeyShuffleMm",  "MM"  },
        { "ganonBossKey",      "OoT" },
    }));
    contentLayout->addWidget(this->MakeParamGroup(content, "Dungeon Aids", {
        { "silverRupeeShuffle", "OoT" },
        { "mapCompassShuffle",  ""    },
        { "tingleShuffle",      "MM"  },
    }));
    contentLayout->addWidget(this->MakeParamGroup(content, "Rusty Keys", {
        { "rustyKeysOot", "OoT" },
        { "rustyKeysMm",  "MM"  },
    }));

    contentLayout->addStretch(1);
    scroll->setWidget(content);
    vbox->addWidget(scroll);
    return page;
}


QWidget* SettingsTab::BuildNpcPage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* vbox = new QVBoxLayout(page);
    vbox->setContentsMargins(16, 16, 16, 16);
    vbox->setSpacing(14);

    QScrollArea* scroll = new QScrollArea(page);
    scroll->setWidgetResizable(true);
    QWidget* content = new QWidget();
    QVBoxLayout* contentLayout = new QVBoxLayout(content);
    contentLayout->setSpacing(14);

    contentLayout->addWidget(this->MakeParamGroup(content, "Scrubs & Cows", {
        { "scrubShuffleOot", "OoT" },
        { "scrubShuffleMm",  "MM"  },
        { "cowShuffleOot",   "OoT" },
        { "cowShuffleMm",    "MM"  },
    }));
    contentLayout->addWidget(this->MakeParamGroup(content, "Shops & Trading", {
        { "shopShuffleOot",       "OoT" },
        { "shopShuffleMm",        "MM"  },
        { "shuffleMerchantsOot",  "OoT" },
        { "shuffleMerchantsMm",   "MM"  },
        { "shuffleMaskTrades",    ""    },
    }));
    contentLayout->addWidget(this->MakeParamGroup(content, "Mini-games & Rewards", {
        { "owlShuffle",             "MM"  },
        { "pondFishShuffle",        "OoT" },
        { "eggShuffle",             ""    },
        { "shuffleLotteryMm",       "MM"  },
        { "shuffleFrogsRupeesOot",  "OoT" },
        { "shuffleSkulltulaFinalReward", "OoT" },
        { "divingGameRupeeShuffle", "OoT" },
    }));

    contentLayout->addStretch(1);
    scroll->setWidget(content);
    vbox->addWidget(scroll);
    return page;
}


QWidget* SettingsTab::BuildBreakablePage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* vbox = new QVBoxLayout(page);
    vbox->setContentsMargins(16, 16, 16, 16);
    vbox->setSpacing(14);

    QScrollArea* scroll = new QScrollArea(page);
    scroll->setWidgetResizable(true);
    QWidget* content = new QWidget();
    QVBoxLayout* contentLayout = new QVBoxLayout(content);
    contentLayout->setSpacing(14);

    contentLayout->addWidget(this->MakeParamGroup(content, "Containers", {
        { "shufflePotsOot",   "OoT" },
        { "shufflePotsMm",    "MM"  },
        { "shuffleCratesOot", "OoT" },
        { "shuffleCratesMm",  "MM"  },
        { "shuffleBarrelsMm", "MM"  },
    }));
    contentLayout->addWidget(this->MakeParamGroup(content, "Vegetation", {
        { "shuffleGrassOot",   "OoT" },
        { "shuffleGrassMm",    "MM"  },
        { "shuffleTFGrassMm",  "MM"  },
        { "shuffleTreesOot",   "OoT" },
        { "shuffleTreesMm",    "MM"  },
        { "shuffleBushOot",    "OoT" },
        { "shuffleBushMm",     "MM"  },
        { "shuffleSoilOot",    "OoT" },
        { "shuffleSoilMm",     "MM"  },
        { "shuffleHivesOot",   "OoT" },
        { "shuffleHivesMm",    "MM"  },
    }));
    contentLayout->addWidget(this->MakeParamGroup(content, "Rocks & Ice", {
        { "shuffleRocksOot",        "OoT" },
        { "shuffleRocksMm",         "MM"  },
        { "shuffleRedBouldersOot",  "OoT" },
        { "shuffleRedBouldersMm",   "MM"  },
        { "shuffleIciclesOot",      "OoT" },
        { "shuffleIciclesMm",       "MM"  },
        { "shuffleRedIceOot",       "OoT" },
        { "shuffleSnowballsMm",     "MM"  },
    }));
    contentLayout->addWidget(this->MakeParamGroup(content, "Misc", {
        { "shuffleWonderItemsOot",  "OoT" },
        { "shuffleWonderItemsMm",   "MM"  },
        { "shuffleButterfliesOot",  "OoT" },
        { "shuffleButterfliesMm",   "MM"  },
    }));

    contentLayout->addStretch(1);
    scroll->setWidget(content);
    vbox->addWidget(scroll);
    return page;
}


QWidget* SettingsTab::BuildSpecialPage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* vbox = new QVBoxLayout(page);
    vbox->setContentsMargins(16, 16, 16, 16);
    vbox->setSpacing(14);

    QScrollArea* scroll = new QScrollArea(page);
    scroll->setWidgetResizable(true);
    QWidget* content = new QWidget();
    QVBoxLayout* contentLayout = new QVBoxLayout(content);
    contentLayout->setSpacing(14);

    contentLayout->addWidget(this->MakeParamGroup(content, "Tokens & Souls", {
        { "goldSkulltulaTokens",   "OoT" },
        { "housesSkulltulaTokens", "MM"  },
    }));
    contentLayout->addWidget(this->MakeParamGroup(content, "Fairies", {
        { "fairyFountainFairyShuffleOot", "OoT" },
        { "fairyFountainFairyShuffleMm",  "MM"  },
        { "fairySpotShuffleOot",          "OoT" },
        { "townFairyShuffle",             "MM"  },
        { "strayFairyChestShuffle",       "MM"  },
        { "strayFairyOtherShuffle",       "MM"  },
    }));
    contentLayout->addWidget(this->MakeParamGroup(content, "Freestanding", {
        { "shuffleFreeRupeesOot", "OoT" },
        { "shuffleFreeRupeesMm",  "MM"  },
        { "shuffleFreeHeartsOot", "OoT" },
        { "shuffleFreeHeartsMm",  "MM"  },
    }));
    contentLayout->addWidget(this->MakeParamGroup(content, "Unique Items", {
        { "songs",                ""    },
        { "shuffleOcarinasOot",   "OoT" },
        { "shuffleMasterSword",   "OoT" },
        { "shuffleGerudoCard",    "OoT" },
    }));
    contentLayout->addWidget(this->MakeParamGroup(content, "Misc", {
        { "skipZelda",          "OoT" },
        { "restoreBrokenActors", ""   },
    }));
    // Cross-game warp toggles: control whether the GPS pathfinder considers that the
    // OoT player owns the MM Song of Soaring (crossWarpOot) and / or that the MM player
    // owns the OoT warp songs (crossWarpMm). Both are read in GPSRouteWidget when a
    // route is recomputed.
    contentLayout->addWidget(this->MakeParamGroup(content, "Cross-Games Warps", {
        { "crossWarpOot", "OoT" },
        { "crossWarpMm", "MM"  },
    }));

    contentLayout->addStretch(1);
    scroll->setWidget(content);
    vbox->addWidget(scroll);
    return page;
}


QWidget* SettingsTab::BuildProgressiveItemsPage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* vbox = new QVBoxLayout(page);
    vbox->setContentsMargins(16, 16, 16, 16);
    vbox->setSpacing(14);

    QScrollArea* scroll = new QScrollArea(page);
    scroll->setWidgetResizable(true);
    QWidget* content = new QWidget();
    QVBoxLayout* contentLayout = new QVBoxLayout(content);
    contentLayout->setSpacing(14);

    QGroupBox* box = new QGroupBox("Progressive Items", content);
    QGridLayout* grid = new QGridLayout(box);
    grid->setColumnStretch(1, 1);
    grid->setHorizontalSpacing(10);
    grid->setVerticalSpacing(6);

    if (this->WinOwner != nullptr)
    {
        // Iterate ItemSettings in declaration order so the rows mirror Settings.cpp.
        for (auto it = this->WinOwner->ROMSettings.ItemSettings.cbegin();
             it != this->WinOwner->ROMSettings.ItemSettings.cend(); ++it)
        {
            const QString& key = it.key();
            if (key.startsWith("shared")) continue;
            if (key.startsWith("song")) continue;                 // Songs live in their own page.
            if (key == "kamaroMaskOot" || key == "boomerangMm") continue;   // -> World Items page.
            if (key == "powderKegOot" || key == "gfsOot") continue;         // -> World Items page.
            if (key == "slingshotMm" || key == "gerudoMaskMm"
                || key == "skullMaskMm" || key == "spookyMaskMm") continue;  // -> World Items page.
            if (key == "rustyKeysOot" || key == "rustyKeysMm") continue;     // -> Keys & Dungeons page.

            QString badge;
            if (key.endsWith("Oot")) badge = "OoT";
            else if (key.endsWith("Mm")) badge = "MM";
            this->AddParamRow(grid, key, badge);
        }
    }

    contentLayout->addWidget(box);
    contentLayout->addStretch(1);
    scroll->setWidget(content);
    vbox->addWidget(scroll);
    return page;
}


QWidget* SettingsTab::BuildSharedItemsPage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* vbox = new QVBoxLayout(page);
    vbox->setContentsMargins(16, 16, 16, 16);
    vbox->setSpacing(14);

    QScrollArea* scroll = new QScrollArea(page);
    scroll->setWidgetResizable(true);
    QWidget* content = new QWidget();
    QVBoxLayout* contentLayout = new QVBoxLayout(content);
    contentLayout->setSpacing(14);

    QGroupBox* box = new QGroupBox("Shared Items", content);
    QGridLayout* grid = new QGridLayout(box);
    grid->setColumnStretch(1, 1);
    grid->setHorizontalSpacing(10);
    grid->setVerticalSpacing(6);

    if (this->WinOwner != nullptr)
    {
        for (auto it = this->WinOwner->ROMSettings.ItemSettings.cbegin();
             it != this->WinOwner->ROMSettings.ItemSettings.cend(); ++it)
        {
            const QString& key = it.key();
            if (!key.startsWith("shared")) continue;
            if (key.startsWith("sharedSong")) continue;   // Shared songs live in the Songs page.

            // Shared toggles cross both games so leave the badge empty.
            this->AddParamRow(grid, key, "");
        }
    }

    contentLayout->addWidget(box);
    contentLayout->addStretch(1);
    scroll->setWidget(content);
    vbox->addWidget(scroll);
    return page;
}


QWidget* SettingsTab::BuildSongsPage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* vbox = new QVBoxLayout(page);
    vbox->setContentsMargins(16, 16, 16, 16);
    vbox->setSpacing(14);

    QScrollArea* scroll = new QScrollArea(page);
    scroll->setWidgetResizable(true);
    QWidget* content = new QWidget();
    QVBoxLayout* contentLayout = new QVBoxLayout(content);
    contentLayout->setSpacing(14);

    // Individual songs (per game) and shared songs live on the same page, split into two groups
    // so the cross-game "Shared ..." toggles are visually separated from the per-game ones.
    QGroupBox* indivBox = new QGroupBox("Songs", content);
    QGridLayout* indivGrid = new QGridLayout(indivBox);
    indivGrid->setColumnStretch(1, 1);
    indivGrid->setHorizontalSpacing(10);
    indivGrid->setVerticalSpacing(6);

    QGroupBox* sharedBox = new QGroupBox("Shared Songs", content);
    QGridLayout* sharedGrid = new QGridLayout(sharedBox);
    sharedGrid->setColumnStretch(1, 1);
    sharedGrid->setHorizontalSpacing(10);
    sharedGrid->setVerticalSpacing(6);

    if (this->WinOwner != nullptr)
    {
        for (auto it = this->WinOwner->ROMSettings.ItemSettings.cbegin();
             it != this->WinOwner->ROMSettings.ItemSettings.cend(); ++it)
        {
            const QString& key = it.key();

            if (key.startsWith("sharedSong"))
            {   // Shared songs cross both games, no badge.

                this->AddParamRow(sharedGrid, key, "");
            }
            else if (key.startsWith("song"))
            {   // Per-game songs: badge from the key suffix.

                QString badge;
                if (key.endsWith("Oot")) badge = "OoT";
                else if (key.endsWith("Mm")) badge = "MM";
                this->AddParamRow(indivGrid, key, badge);
            }
        }
    }

    contentLayout->addWidget(indivBox);
    contentLayout->addWidget(sharedBox);
    contentLayout->addStretch(1);
    scroll->setWidget(content);
    vbox->addWidget(scroll);
    return page;
}


QWidget* SettingsTab::BuildWorldItemsPage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* vbox = new QVBoxLayout(page);
    vbox->setContentsMargins(16, 16, 16, 16);
    vbox->setSpacing(14);

    QScrollArea* scroll = new QScrollArea(page);
    scroll->setWidgetResizable(true);
    QWidget* content = new QWidget();
    QVBoxLayout* contentLayout = new QVBoxLayout(content);
    contentLayout->setSpacing(14);

    // Per-world unique items ---------------------------------------------
    contentLayout->addWidget(this->MakeParamGroup(content, "Unique World Items", {
        { "kamaroMaskOot", "OoT" },
        { "powderKegOot", "OoT" },
        { "gfsOot", "OoT" },
        { "boomerangMm",   "MM"  },
        { "slingshotMm",   "MM"  },
    }));

    // Masks --------------------------------------------------------------
    contentLayout->addWidget(this->MakeParamGroup(content, "Masks", {
        { "gerudoMaskMm", "MM" },
        { "skullMaskMm",  "MM" },
        { "spookyMaskMm", "MM" },
    }));

    // Key Rings group ----------------------------------------------------
    QGroupBox* ringBox = new QGroupBox("Small Key Rings", content);
    QGridLayout* ringGrid = new QGridLayout(ringBox);
    ringGrid->setColumnStretch(1, 1);
    ringGrid->setColumnStretch(3, 1);
    ringGrid->setHorizontalSpacing(10);
    ringGrid->setVerticalSpacing(6);

    int ringRow = 0;
    for (const DungeonItemPair& pair : OoTKeyRings)
    {
        QLabel* badge = this->MakeGameBadge("OoT");
        QLabel* nameLabel = new QLabel(pair.Label);
        nameLabel->setStyleSheet("color: #ddeeff;");
        QCheckBox* check = new QCheckBox();
        ringGrid->addWidget(badge, ringRow, 0, Qt::AlignVCenter | Qt::AlignLeft);
        ringGrid->addWidget(nameLabel, ringRow, 1);
        ringGrid->addWidget(check, ringRow, 2);
        this->KeyRingChecksOoT.insert(QString::fromUtf8(pair.Label), check);
        ringRow++;
    }
    for (const DungeonItemPair& pair : MMKeyRings)
    {
        QLabel* badge = this->MakeGameBadge("MM");
        QLabel* nameLabel = new QLabel(pair.Label);
        nameLabel->setStyleSheet("color: #ddeeff;");
        QCheckBox* check = new QCheckBox();
        ringGrid->addWidget(badge, ringRow, 0, Qt::AlignVCenter | Qt::AlignLeft);
        ringGrid->addWidget(nameLabel, ringRow, 1);
        ringGrid->addWidget(check, ringRow, 2);
        this->KeyRingChecksMM.insert(QString::fromUtf8(pair.Label), check);
        ringRow++;
    }
    contentLayout->addWidget(ringBox);

    // Silver Pouches group ----------------------------------------------
    QGroupBox* pouchBox = new QGroupBox("Silver Rupee Pouches", content);
    QGridLayout* pouchGrid = new QGridLayout(pouchBox);
    pouchGrid->setColumnStretch(1, 1);
    pouchGrid->setHorizontalSpacing(10);
    pouchGrid->setVerticalSpacing(6);

    int pouchRow = 0;
    for (const DungeonItemPair& pair : OoTSilverPouches)
    {
        QLabel* badge = this->MakeGameBadge("OoT");
        QLabel* nameLabel = new QLabel(pair.Label);
        nameLabel->setStyleSheet("color: #ddeeff;");
        QCheckBox* check = new QCheckBox();
        pouchGrid->addWidget(badge, pouchRow, 0, Qt::AlignVCenter | Qt::AlignLeft);
        pouchGrid->addWidget(nameLabel, pouchRow, 1);
        pouchGrid->addWidget(check, pouchRow, 2);
        this->SilverPouchChecks.insert(QString::fromUtf8(pair.Label), check);
        pouchRow++;
    }
    contentLayout->addWidget(pouchBox);

    // Owl Statues group --------------------------------------------------
    QGroupBox* owlBox = new QGroupBox("Pre-Activated Owl Statues", content);
    QGridLayout* owlGrid = new QGridLayout(owlBox);
    owlGrid->setColumnStretch(1, 1);
    owlGrid->setHorizontalSpacing(10);
    owlGrid->setVerticalSpacing(6);

    int owlRow = 0;
    for (const OwlChoice& owl : MMOwlStatues)
    {
        QLabel* badge = this->MakeGameBadge("MM");
        QLabel* nameLabel = new QLabel(owl.Label);
        nameLabel->setStyleSheet("color: #ddeeff;");
        QCheckBox* check = new QCheckBox();
        owlGrid->addWidget(badge, owlRow, 0, Qt::AlignVCenter | Qt::AlignLeft);
        owlGrid->addWidget(nameLabel, owlRow, 1);
        owlGrid->addWidget(check, owlRow, 2);
        this->OwlStatueChecks.insert(QString::fromUtf8(owl.Label), check);
        owlRow++;
    }
    contentLayout->addWidget(owlBox);

    contentLayout->addStretch(1);
    scroll->setWidget(content);
    vbox->addWidget(scroll);
    return page;
}


QWidget* SettingsTab::BuildLayoutsPage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* vbox = new QVBoxLayout(page);
    vbox->setContentsMargins(16, 16, 16, 16);
    vbox->setSpacing(14);

    QScrollArea* scroll = new QScrollArea(page);
    scroll->setWidgetResizable(true);
    QWidget* content = new QWidget();
    QVBoxLayout* contentLayout = new QVBoxLayout(content);
    contentLayout->setSpacing(14);

    // Master Quest group -------------------------------------------------
    QGroupBox* mqBox = new QGroupBox("Master Quest Dungeons", content);
    QGridLayout* mqGrid = new QGridLayout(mqBox);
    mqGrid->setColumnStretch(1, 1);
    mqGrid->setHorizontalSpacing(10);
    mqGrid->setVerticalSpacing(6);

    int mqRow = 0;
    for (const MQScene& scene : OoTMQScenes)
    {
        QLabel* badge = this->MakeGameBadge("OoT");
        QLabel* nameLabel = new QLabel(scene.Label);
        nameLabel->setStyleSheet("color: #ddeeff;");
        QCheckBox* check = new QCheckBox();
        mqGrid->addWidget(badge, mqRow, 0, Qt::AlignVCenter | Qt::AlignLeft);
        mqGrid->addWidget(nameLabel, mqRow, 1);
        mqGrid->addWidget(check, mqRow, 2);
        this->MQLayoutChecks.insert(scene.SceneId, check);
        mqRow++;
    }
    contentLayout->addWidget(mqBox);

    // Majora's Mask JP group --------------------------------------------
    QGroupBox* jpBox = new QGroupBox("Majora's Mask JP Layouts", content);
    QGridLayout* jpGrid = new QGridLayout(jpBox);
    jpGrid->setColumnStretch(1, 1);
    jpGrid->setHorizontalSpacing(10);
    jpGrid->setVerticalSpacing(6);

    QLabel* jpBadge = this->MakeGameBadge("MM");
    QLabel* jpName = new QLabel("Deku Palace");
    jpName->setStyleSheet("color: #ddeeff;");
    this->JPLayoutDekuPalace = new QCheckBox();
    jpGrid->addWidget(jpBadge, 0, 0, Qt::AlignVCenter | Qt::AlignLeft);
    jpGrid->addWidget(jpName, 0, 1);
    jpGrid->addWidget(this->JPLayoutDekuPalace, 0, 2);
    contentLayout->addWidget(jpBox);

    contentLayout->addStretch(1);
    scroll->setWidget(content);
    vbox->addWidget(scroll);
    return page;
}

#pragma endregion

#pragma region Apply / Cancel

void SettingsTab::LoadFromSettings()
{
    if (this->WinOwner == nullptr) return;

    Settings& s = this->WinOwner->ROMSettings;

    if (this->GameGroup != nullptr)
    {
        QAbstractButton* btn = this->GameGroup->button(static_cast<int>(s.Game));
        if (btn != nullptr) btn->setChecked(true);
    }
    if (this->ModeGroup != nullptr)
    {
        QAbstractButton* btn = this->ModeGroup->button(static_cast<int>(s.Mode));
        if (btn != nullptr) btn->setChecked(true);
    }
    if (this->TeamsSpin != nullptr)
    {
        this->TeamsSpin->setValue(static_cast<int>(s.NumOfTeams));
    }

    for (auto it = this->ParamWidgets.cbegin(); it != this->ParamWidgets.cend(); ++it)
    {
        Parameter* p = this->FindParameter(it.key());
        if (p == nullptr) continue;

        if (auto* check = qobject_cast<QCheckBox*>(it.value()))
        {
            check->setChecked(p->Value == ShuffleSetting::all);
        }
        else if (auto* combo = qobject_cast<QComboBox*>(it.value()))
        {
            int idx = combo->findData(static_cast<int>(p->Value));
            if (idx >= 0) combo->setCurrentIndex(idx);
        }
    }

    // Key rings: a ring is "enabled" when its item id is NOT in DisabledItemIDs.
    for (const DungeonItemPair& pair : OoTKeyRings)
    {
        auto it = this->KeyRingChecksOoT.find(QString::fromUtf8(pair.Label));
        if (it != this->KeyRingChecksOoT.end())
        {
            it.value()->setChecked(!s.DisabledItemIDs.contains(pair.RingId));
        }
    }
    for (const DungeonItemPair& pair : MMKeyRings)
    {
        auto it = this->KeyRingChecksMM.find(QString::fromUtf8(pair.Label));
        if (it != this->KeyRingChecksMM.end())
        {
            it.value()->setChecked(!s.DisabledItemIDs.contains(pair.RingId));
        }
    }
    for (const DungeonItemPair& pair : OoTSilverPouches)
    {
        auto it = this->SilverPouchChecks.find(QString::fromUtf8(pair.Label));
        if (it != this->SilverPouchChecks.end())
        {
            it.value()->setChecked(!s.DisabledItemIDs.contains(pair.RingId));
        }
    }
    for (const OwlChoice& owl : MMOwlStatues)
    {
        auto it = this->OwlStatueChecks.find(QString::fromUtf8(owl.Label));
        if (it != this->OwlStatueChecks.end())
        {
            it.value()->setChecked(s.StartingItemIDs.contains(owl.ItemId));
        }
    }

    // Layouts: scene's ActiveLayout reflects the spoiler-driven choice.
    for (const MQScene& scene : OoTMQScenes)
    {
        auto it = this->MQLayoutChecks.find(scene.SceneId);
        if (it == this->MQLayoutChecks.end()) continue;

        SceneMetaInfo* info = GetSceneMetaInfo(scene.SceneId, OOT_GAME);
        if (info != nullptr)
        {
            it.value()->setChecked(info->ActiveLayout == GameLayout::oot_mq);
        }
    }
    if (this->JPLayoutDekuPalace != nullptr)
    {
        SceneMetaInfo* info = GetSceneMetaInfo(MM_DEKU_PALACE, MM_GAME);
        this->JPLayoutDekuPalace->setChecked(info != nullptr && info->ActiveLayout == GameLayout::mm_jp);
    }
}


void SettingsTab::OnApply()
{
    if (this->WinOwner == nullptr) return;

    Settings& s = this->WinOwner->ROMSettings;
    // Clear only the sets that are fully rebuilt from the UI / ApplyItemSettings. StartingItemIDs
    // is populated by the spoiler's "Starting Items" section and only the owl-statue subset is
    // editable from this dialog, so clearing the whole set here would silently drop every other
    // starting item the player rolled with (Light Arrow, Hookshot, ...). The owl statue loop in
    // ApplyWorldItemSelections inserts/removes its own ids individually, which is enough to keep
    // them in sync with the checkboxes without touching the rest.
    s.DisabledItemIDs.clear();
    s.SharedItemIDs.clear();

    if (this->GameGroup != nullptr && this->GameGroup->checkedId() >= 0)
    {
        s.Game = static_cast<ROMGame>(this->GameGroup->checkedId());
    }
    if (this->ModeGroup != nullptr && this->ModeGroup->checkedId() >= 0)
    {
        s.Mode = static_cast<GameMode>(this->ModeGroup->checkedId());
    }
    if (this->TeamsSpin != nullptr)
    {
        s.NumOfTeams = static_cast<size_t>(this->TeamsSpin->value());
    }

    // Allocate worlds from Mode / NumOfTeams so the world selector appears even without a
    // spoiler (e.g. switching single -> multi with teams > 1 to test). Multiworld uses one
    // world per team; single / coop collapse to a single world. We only re-init when the count
    // actually changes, otherwise a freshly parsed multiworld spoiler would be wiped on Apply.
    size_t desiredWorlds = (s.Mode == GameMode::multi) ? (s.NumOfTeams > 0 ? s.NumOfTeams : 1) : 1;
    if (desiredWorlds != GetNumWorlds())
    {
        InitWorlds(desiredWorlds);
        SetActiveWorld(0);
    }

    for (auto it = this->ParamWidgets.cbegin(); it != this->ParamWidgets.cend(); ++it)
    {
        Parameter* p = this->FindParameter(it.key());
        if (p == nullptr) continue;

        if (auto* check = qobject_cast<QCheckBox*>(it.value()))
        {
            p->Value = check->isChecked() ? ShuffleSetting::all : ShuffleSetting::vanilla;
        }
        else if (auto* combo = qobject_cast<QComboBox*>(it.value()))
        {
            p->Value = static_cast<ShuffleSetting>(combo->currentData().toInt());
        }
    }

    // Order matters: ApplyWorldItemSelections is what actually populates DisabledItemIDs from
    // the per-dungeon checkboxes (key rings / silver pouches / owls). If we propagated to the
    // tracker first, ProgressionTab::ApplySettings would read an empty DisabledItemIDs and leave
    // every widget visible regardless of the user's selection.
    this->ApplyLayoutSelections();
    this->ApplyWorldItemSelections();
    this->WinOwner->ApplySettings();
    this->WinOwner->RefreshTracker();
}


void SettingsTab::ApplyWorldItemSelections()
{
    if (this->WinOwner == nullptr) return;

    Settings& s = this->WinOwner->ROMSettings;

    // Key rings: checked => the small key id is disabled and the ring id is enabled.
    for (const DungeonItemPair& pair : OoTKeyRings)
    {
        auto it = this->KeyRingChecksOoT.find(QString::fromUtf8(pair.Label));
        if (it == this->KeyRingChecksOoT.end()) continue;

        if (it.value()->isChecked())
        {
            s.DisabledItemIDs.insert(pair.SmallId);
            s.DisabledItemIDs.remove(pair.RingId);
        }
        else
        {
            s.DisabledItemIDs.remove(pair.SmallId);
            s.DisabledItemIDs.insert(pair.RingId);
        }
    }
    for (const DungeonItemPair& pair : MMKeyRings)
    {
        auto it = this->KeyRingChecksMM.find(QString::fromUtf8(pair.Label));
        if (it == this->KeyRingChecksMM.end()) continue;

        if (it.value()->isChecked())
        {
            s.DisabledItemIDs.insert(pair.SmallId);
            s.DisabledItemIDs.remove(pair.RingId);
        }
        else
        {
            s.DisabledItemIDs.remove(pair.SmallId);
            s.DisabledItemIDs.insert(pair.RingId);
        }
    }

    // Silver pouches: checked => silver rupee id is disabled and pouch id is enabled.
    for (const DungeonItemPair& pair : OoTSilverPouches)
    {
        auto it = this->SilverPouchChecks.find(QString::fromUtf8(pair.Label));
        if (it == this->SilverPouchChecks.end()) continue;

        if (it.value()->isChecked())
        {
            s.DisabledItemIDs.insert(pair.SmallId);
            s.DisabledItemIDs.remove(pair.RingId);
        }
        else
        {
            s.DisabledItemIDs.remove(pair.SmallId);
            s.DisabledItemIDs.insert(pair.RingId);
        }
    }

    // Owl statues: checked => the owl id is in StartingItemIDs with count 1.
    for (const OwlChoice& owl : MMOwlStatues)
    {
        auto it = this->OwlStatueChecks.find(QString::fromUtf8(owl.Label));
        if (it == this->OwlStatueChecks.end()) continue;

        if (it.value()->isChecked())
        {
            s.StartingItemIDs.insert(owl.ItemId, 1);
        }
        else
        {
            s.StartingItemIDs.remove(owl.ItemId);
        }
    }

    // Note: the 'removed' / 'vanilla' override for small keys + key rings is applied by
    // Settings::ApplyItemSettings (next call from OnApply), so it runs uniformly across both
    // the spoiler-load and the SettingsTab-Apply paths.
}


void SettingsTab::ApplyLayoutSelections()
{
    // Master Quest: scene per scene.
    for (const MQScene& scene : OoTMQScenes)
    {
        auto it = this->MQLayoutChecks.find(scene.SceneId);
        if (it == this->MQLayoutChecks.end()) continue;

        SceneMetaInfo* info = GetSceneMetaInfo(scene.SceneId, OOT_GAME);
        if (info == nullptr) continue;

        info->ActiveLayout = it.value()->isChecked() ? GameLayout::oot_mq : GameLayout::oot;
    }

    // Majora's Mask JP: a single Deku Palace toggle covers three scenes.
    if (this->JPLayoutDekuPalace != nullptr)
    {
        GameLayout target = this->JPLayoutDekuPalace->isChecked() ? GameLayout::mm_jp : GameLayout::mm;
        if (SceneMetaInfo* info = GetSceneMetaInfo(MM_GROTTOS, MM_GAME))                   info->ActiveLayout = target;
        if (SceneMetaInfo* info = GetSceneMetaInfo(MM_DEKU_PALACE, MM_GAME))               info->ActiveLayout = target;
        if (SceneMetaInfo* info = GetSceneMetaInfo(MM_GROTTO_DEKU_PALACE_GENERIC, MM_GAME)) info->ActiveLayout = target;
        if (SceneMetaInfo* info = GetSceneMetaInfo(MM_GROTTO_DEKU_PALACE_CLIMB, MM_GAME)) info->ActiveLayout = target;
    }
}


void SettingsTab::OnLoadSpoiler()
{
    if (this->WinOwner == nullptr) return;

    QString filePath = QFileDialog::getOpenFileName(this, "Choose a spoiler log", "", "Text Files (*.txt)");
    if (filePath.isEmpty()) return;

    this->WinOwner->LoadGameSpoiler(filePath);

    // The spoiler parser overwrites every entry in ROMSettings, so we have to repopulate
    // the editor widgets to mirror the new state.
    this->LoadFromSettings();
}

#pragma endregion
