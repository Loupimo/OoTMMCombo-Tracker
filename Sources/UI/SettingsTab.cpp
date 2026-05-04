#include "UI/SettingsTab.h"
#include "UI/OoTMMComboTracker.h"
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
    this->NavList->setCurrentRow(0);
    navLayout->addWidget(this->NavList, 1);

    this->ApplyButton = new QPushButton("Apply", navColumn);
    this->CancelButton = new QPushButton("Cancel", navColumn);
    navLayout->addWidget(this->ApplyButton);
    navLayout->addWidget(this->CancelButton);

    // Right side stack ----------------------------------------------------
    this->Pages = new QStackedWidget(this);
    this->Pages->addWidget(this->BuildGeneralPage());
    this->Pages->addWidget(this->BuildKeysPage());
    this->Pages->addWidget(this->BuildNpcPage());
    this->Pages->addWidget(this->BuildBreakablePage());
    this->Pages->addWidget(this->BuildSpecialPage());

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


void SettingsTab::AddParamRow(QGridLayout* Layout, const QString& Key, const QString& GameLabel)
{
    if (this->WinOwner == nullptr) return;

    auto& romSettings = this->WinOwner->ROMSettings.FilterSettings;
    if (!romSettings.contains(Key)) return;

    const Parameter& param = romSettings[Key];
    int row = Layout->rowCount();

    // Game badge (column 0).
    QLabel* badge = this->MakeGameBadge(GameLabel);
    if (badge != nullptr)
    {
        Layout->addWidget(badge, row, 0, Qt::AlignVCenter | Qt::AlignLeft);
    }

    // Setting display name (column 1).
    QLabel* nameLabel = new QLabel(param.Name);
    nameLabel->setStyleSheet("color: #ddeeff;");
    Layout->addWidget(nameLabel, row, 1);

    // Editor widget (column 2).
    QWidget* editor = nullptr;
    switch (param.Type)
    {
        case ParamType::boolean:
        {
            QCheckBox* check = new QCheckBox();
            check->setChecked(param.Value == ShuffleSetting::all);
            editor = check;
            break;
        }

        case ParamType::shuffle:
        {
            QComboBox* combo = new QComboBox();
            combo->addItem("Vanilla", static_cast<int>(ShuffleSetting::vanilla));
            combo->addItem("Dungeons", static_cast<int>(ShuffleSetting::dungeons));
            combo->addItem("Overworld", static_cast<int>(ShuffleSetting::overworld));
            combo->addItem("Anywhere", static_cast<int>(ShuffleSetting::all));
            int idx = combo->findData(static_cast<int>(param.Value));
            if (idx >= 0) combo->setCurrentIndex(idx);
            editor = combo;
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
    this->TeamsSpin->setRange(1, 16);
    teamsLayout->addWidget(teamsLabel);
    teamsLayout->addWidget(this->TeamsSpin);
    teamsLayout->addStretch(1);
    vbox->addWidget(teamsBox);

    vbox->addStretch(1);
    return page;
}


QWidget* SettingsTab::BuildKeysPage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* vbox = new QVBoxLayout(page);
    vbox->setContentsMargins(16, 16, 16, 16);

    QScrollArea* scroll = new QScrollArea(page);
    scroll->setWidgetResizable(true);
    QWidget* content = new QWidget();
    QGridLayout* grid = new QGridLayout(content);
    grid->setColumnStretch(1, 1);
    grid->setHorizontalSpacing(10);
    grid->setVerticalSpacing(6);

    this->AddParamRow(grid, "smallKeyShuffleOot", "OoT");
    this->AddParamRow(grid, "smallKeyShuffleMm", "MM");
    this->AddParamRow(grid, "smallKeyShuffleHideout", "OoT");
    this->AddParamRow(grid, "smallKeyShuffleChestGame", "OoT");
    this->AddParamRow(grid, "bossKeyShuffleOot", "OoT");
    this->AddParamRow(grid, "bossKeyShuffleMm", "MM");
    this->AddParamRow(grid, "ganonBossKey", "OoT");
    this->AddParamRow(grid, "silverRupeeShuffle", "OoT");
    this->AddParamRow(grid, "mapCompassShuffle", "");
    this->AddParamRow(grid, "tingleShuffle", "MM");

    grid->setRowStretch(grid->rowCount(), 1);
    scroll->setWidget(content);
    vbox->addWidget(scroll);
    return page;
}


QWidget* SettingsTab::BuildNpcPage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* vbox = new QVBoxLayout(page);
    vbox->setContentsMargins(16, 16, 16, 16);

    QScrollArea* scroll = new QScrollArea(page);
    scroll->setWidgetResizable(true);
    QWidget* content = new QWidget();
    QGridLayout* grid = new QGridLayout(content);
    grid->setColumnStretch(1, 1);
    grid->setHorizontalSpacing(10);
    grid->setVerticalSpacing(6);

    this->AddParamRow(grid, "scrubShuffleOot", "OoT");
    this->AddParamRow(grid, "scrubShuffleMm", "MM");
    this->AddParamRow(grid, "cowShuffleOot", "OoT");
    this->AddParamRow(grid, "cowShuffleMm", "MM");
    this->AddParamRow(grid, "shopShuffleOot", "OoT");
    this->AddParamRow(grid, "shopShuffleMm", "MM");
    this->AddParamRow(grid, "owlShuffle", "MM");
    this->AddParamRow(grid, "shuffleMerchantsOot", "OoT");
    this->AddParamRow(grid, "shuffleMerchantsMm", "MM");
    this->AddParamRow(grid, "shuffleMaskTrades", "");
    this->AddParamRow(grid, "pondFishShuffle", "OoT");
    this->AddParamRow(grid, "eggShuffle", "");
    this->AddParamRow(grid, "shuffleLotteryMm", "MM");
    this->AddParamRow(grid, "shuffleFrogsRupeesOot", "OoT");
    this->AddParamRow(grid, "divingGameRupeeShuffle", "OoT");

    grid->setRowStretch(grid->rowCount(), 1);
    scroll->setWidget(content);
    vbox->addWidget(scroll);
    return page;
}


QWidget* SettingsTab::BuildBreakablePage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* vbox = new QVBoxLayout(page);
    vbox->setContentsMargins(16, 16, 16, 16);

    QScrollArea* scroll = new QScrollArea(page);
    scroll->setWidgetResizable(true);
    QWidget* content = new QWidget();
    QGridLayout* grid = new QGridLayout(content);
    grid->setColumnStretch(1, 1);
    grid->setHorizontalSpacing(10);
    grid->setVerticalSpacing(6);

    this->AddParamRow(grid, "shufflePotsOot", "OoT");
    this->AddParamRow(grid, "shufflePotsMm", "MM");
    this->AddParamRow(grid, "shuffleCratesOot", "OoT");
    this->AddParamRow(grid, "shuffleCratesMm", "MM");
    this->AddParamRow(grid, "shuffleBarrelsMm", "MM");
    this->AddParamRow(grid, "shuffleGrassOot", "OoT");
    this->AddParamRow(grid, "shuffleGrassMm", "MM");
    this->AddParamRow(grid, "shuffleTFGrassMm", "MM");
    this->AddParamRow(grid, "shuffleRocksOot", "OoT");
    this->AddParamRow(grid, "shuffleRocksMm", "MM");
    this->AddParamRow(grid, "shuffleTreesOot", "OoT");
    this->AddParamRow(grid, "shuffleTreesMm", "MM");
    this->AddParamRow(grid, "shuffleBushOot", "OoT");
    this->AddParamRow(grid, "shuffleBushMm", "MM");
    this->AddParamRow(grid, "shuffleSoilOot", "OoT");
    this->AddParamRow(grid, "shuffleSoilMm", "MM");
    this->AddParamRow(grid, "shuffleSnowballsMm", "MM");
    this->AddParamRow(grid, "shuffleHivesOot", "OoT");
    this->AddParamRow(grid, "shuffleHivesMm", "MM");
    this->AddParamRow(grid, "shuffleRedBouldersOot", "OoT");
    this->AddParamRow(grid, "shuffleRedBouldersMm", "MM");
    this->AddParamRow(grid, "shuffleIciclesOot", "OoT");
    this->AddParamRow(grid, "shuffleIciclesMm", "MM");
    this->AddParamRow(grid, "shuffleRedIceOot", "OoT");
    this->AddParamRow(grid, "shuffleWonderItemsOot", "OoT");
    this->AddParamRow(grid, "shuffleWonderItemsMm", "MM");
    this->AddParamRow(grid, "shuffleButterfliesOot", "OoT");
    this->AddParamRow(grid, "shuffleButterfliesMm", "MM");

    grid->setRowStretch(grid->rowCount(), 1);
    scroll->setWidget(content);
    vbox->addWidget(scroll);
    return page;
}


QWidget* SettingsTab::BuildSpecialPage()
{
    QWidget* page = new QWidget(this);
    QVBoxLayout* vbox = new QVBoxLayout(page);
    vbox->setContentsMargins(16, 16, 16, 16);

    QScrollArea* scroll = new QScrollArea(page);
    scroll->setWidgetResizable(true);
    QWidget* content = new QWidget();
    QGridLayout* grid = new QGridLayout(content);
    grid->setColumnStretch(1, 1);
    grid->setHorizontalSpacing(10);
    grid->setVerticalSpacing(6);

    this->AddParamRow(grid, "goldSkulltulaTokens", "OoT");
    this->AddParamRow(grid, "housesSkulltulaTokens", "MM");
    this->AddParamRow(grid, "fairyFountainFairyShuffleOot", "OoT");
    this->AddParamRow(grid, "fairyFountainFairyShuffleMm", "MM");
    this->AddParamRow(grid, "fairySpotShuffleOot", "OoT");
    this->AddParamRow(grid, "townFairyShuffle", "MM");
    this->AddParamRow(grid, "strayFairyChestShuffle", "MM");
    this->AddParamRow(grid, "strayFairyOtherShuffle", "MM");
    this->AddParamRow(grid, "shuffleFreeRupeesOot", "OoT");
    this->AddParamRow(grid, "shuffleFreeRupeesMm", "MM");
    this->AddParamRow(grid, "shuffleFreeHeartsOot", "OoT");
    this->AddParamRow(grid, "shuffleFreeHeartsMm", "MM");
    this->AddParamRow(grid, "shuffleOcarinasOot", "OoT");
    this->AddParamRow(grid, "shuffleMasterSword", "OoT");
    this->AddParamRow(grid, "shuffleGerudoCard", "OoT");
    this->AddParamRow(grid, "restoreBrokenActors", "");

    grid->setRowStretch(grid->rowCount(), 1);
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
        if (!s.FilterSettings.contains(it.key())) continue;
        const Parameter& p = s.FilterSettings[it.key()];

        if (auto* check = qobject_cast<QCheckBox*>(it.value()))
        {
            check->setChecked(p.Value == ShuffleSetting::all);
        }
        else if (auto* combo = qobject_cast<QComboBox*>(it.value()))
        {
            int idx = combo->findData(static_cast<int>(p.Value));
            if (idx >= 0) combo->setCurrentIndex(idx);
        }
    }
}


void SettingsTab::OnApply()
{
    if (this->WinOwner == nullptr) return;

    Settings& s = this->WinOwner->ROMSettings;
    s.DisabledItemIDs.clear();
    s.SharedItemIDs.clear();
    s.StartingItemIDs.clear();

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

    for (auto it = this->ParamWidgets.cbegin(); it != this->ParamWidgets.cend(); ++it)
    {
        if (!s.FilterSettings.contains(it.key())) continue;
        Parameter& p = s.FilterSettings[it.key()];

        if (auto* check = qobject_cast<QCheckBox*>(it.value()))
        {
            p.Value = check->isChecked() ? ShuffleSetting::all : ShuffleSetting::vanilla;
        }
        else if (auto* combo = qobject_cast<QComboBox*>(it.value()))
        {
            p.Value = static_cast<ShuffleSetting>(combo->currentData().toInt());
        }
    }

    this->WinOwner->ApplySettings();
    this->WinOwner->RefreshTracker();
}

#pragma endregion
