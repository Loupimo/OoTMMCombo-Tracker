#include <QFileDialog>
#include <QRegularExpression>
#include <QFontDatabase>
#include "UI/LogTab.h"
#include "UI/GameTab.h"
#include "UI/AppConfig.h"
#include "UI/OoTMMComboTracker.h"
#include "Combo/Scenes.h"
#include "UI/Settings.h"

// Contains all the association between the spoiler location and their matching scene in this program
const QHash<QString, QPair<uint32_t, uint32_t>> SpoilerMap =
{
    { "Inside Eggs", { OOT_INSIDE_EGGS, OOT_GAME } },
    { "Kokiri Forest", { OOT_KOKIRI_FOREST, OOT_GAME } },
    { "Hyrule Field", { OOT_HYRULE_FIELD, OOT_GAME } },
    { "Market", { OOT_MARKET, OOT_GAME } },
    { "Lon Lon Ranch", { OOT_LON_LON_RANCH, OOT_GAME } },
    { "Hyrule Castle", { OOT_HYRULE_CASTLE, OOT_GAME } },
    { "Outside Ganon's Castle", { OOT_GANON_CASTLE_EXTERIOR, OOT_GAME } },
    { "Lost Woods", { OOT_LOST_WOODS, OOT_GAME } },
    { "Sacred Forest Meadow", { OOT_SACRED_FOREST_MEADOW, OOT_GAME } },
    { "Kakariko", { OOT_KAKARIKO_VILLAGE, OOT_GAME } },
    { "Graveyard", { OOT_GRAVEYARD, OOT_GAME } },
    { "Death Mountain Trail", { OOT_DEATH_MOUNTAIN_TRAIL, OOT_GAME } },
    { "Goron City", { OOT_GORON_CITY, OOT_GAME } },
    { "Zora's River", { OOT_ZORA_RIVER, OOT_GAME } },
    { "Zora's Domain", { OOT_ZORA_DOMAIN, OOT_GAME } },
    { "Lake Hylia", { OOT_LAKE_HYLIA, OOT_GAME } },
    { "Zora's Fountain", { OOT_ZORA_FOUNTAIN, OOT_GAME } },
    { "Temple of Time", { OOT_TEMPLE_OF_TIME, OOT_GAME} },
    { "Sacred Realm", { OOT_TEMPLE_OF_TIME, OOT_GAME } },
    { "Death Mountain Crater", { OOT_DEATH_MOUNTAIN_CRATER, OOT_GAME } },
    { "Gerudo Valley", { OOT_GERUDO_VALLEY, OOT_GAME } },
    { "Gerudo's Fortress", { OOT_GERUDO_FORTRESS, OOT_GAME } },
    { "Haunted Wasteland", { OOT_HAUNTED_WASTELAND, OOT_GAME } },
    { "Desert Colossus", { OOT_DESERT_COLOSSUS, OOT_GAME } },
    { "Deku Tree", { OOT_DEKU_TREE, OOT_GAME } },
    { "Dodongo's Cavern", { OOT_DODONGO_CAVERN, OOT_GAME } },
    { "Jabu-Jabu's Belly", { OOT_INSIDE_JABU_JABU, OOT_GAME } },
    { "Forest Temple", { OOT_TEMPLE_FOREST, OOT_GAME } },
    { "Fire Temple", { OOT_TEMPLE_FIRE, OOT_GAME } },
    { "Water Temple", { OOT_TEMPLE_WATER, OOT_GAME } },
    { "Spirit Temple", { OOT_TEMPLE_SPIRIT, OOT_GAME } },
    { "Shadow Temple", { OOT_TEMPLE_SHADOW, OOT_GAME } },
    { "Bottom of the Well", { OOT_BOTTOM_OF_THE_WELL, OOT_GAME } },
    { "Ice Cavern", { OOT_ICE_CAVERN, OOT_GAME } },
    { "Thieves' Hideout", { OOT_THIEVES_HIDEOUT, OOT_GAME } },
    { "Gerudo's Training Ground", { OOT_GERUDO_TRAINING_GROUND, OOT_GAME } },
    { "Ganon's Castle", { OOT_INSIDE_GANON_CASTLE, OOT_GAME } },
    { "Tingle", { MM_CLOCK_TOWN_NORTH, MM_GAME } },
    { "South Clock Town", { MM_CLOCK_TOWN_SOUTH, MM_GAME } },
    { "Milk Road", { MM_MILK_ROAD, MM_GAME } },
    { "Southern Swamp", { MM_SOUTHERN_SWAMP, MM_GAME } },
    { "Woodfall", { MM_WOODFALL, MM_GAME } },
    { "Mountain Village", { MM_MOUNTAIN_VILLAGE, MM_GAME } },
    { "Snowhead", { MM_SNOWHEAD, MM_GAME } },
    { "Great Bay Coast", { MM_GREAT_BAY_COAST, MM_GAME } },
    { "Zora Cape", { MM_ZORA_CAPE, MM_GAME } },
    { "Ikana Canyon", { MM_IKANA_CANYON, MM_GAME } },
    { "Stone Tower", { MM_STONE_TOWER, MM_GAME } },
    { "Giant's Dream", { MM_TEMPLE_WOODFALL, MM_GAME } },
    { "North Clock Town", { MM_CLOCK_TOWN_NORTH, MM_GAME } },
    { "West Clock Town", { MM_CLOCK_TOWN_WEST, MM_GAME } },
    { "East Clock Town", { MM_CLOCK_TOWN_EAST, MM_GAME } },
    { "Laundry Pool", { MM_LAUNDRY_POOL, MM_GAME } },
    { "Stock Pot Inn", { MM_STOCK_POT_INN, MM_GAME } },
    { "Termina Field", { MM_TERMINA_FIELD, MM_GAME } },
    { "Road to Southern Swamp", { MM_ROAD_SOUTHERN_SWAMP, MM_GAME } },
    { "Deku Palace", { MM_DEKU_PALACE, MM_GAME } },
    { "Path to Mountain Village", { MM_PATH_MOUNTAIN_VILLAGE, MM_GAME } },
    { "Twin Islands", { MM_TWIN_ISLANDS, MM_GAME } },
    { "Goron Village", { MM_GORON_VILLAGE_WINTER, MM_GAME } },
    { "Road to Snowhead", { MM_PATH_SNOWHEAD, MM_GAME } },
    { "Romani Ranch", { MM_ROMANI_RANCH, MM_GAME } },
    { "Pinnacle Rock", { MM_PINNACLE_ROCK, MM_GAME } },
    { "Zora Hall", { MM_ZORA_HALL_ROOMS, MM_GAME } },
    { "Road to Ikana", { MM_ROAD_IKANA, MM_GAME } },
    { "Ikana Graveyard", { MM_IKANA_GRAVEYARD, MM_GAME } },
    { "Ikana Castle", { MM_CASTLE_IKANA, MM_GAME } },
    { "Woodfall Temple", { MM_TEMPLE_WOODFALL, MM_GAME } },
    { "Snowhead Temple", { MM_TEMPLE_SNOWHEAD, MM_GAME } },
    { "Great Bay Temple", { MM_TEMPLE_GREAT_BAY, MM_GAME } },
    { "Stone Tower Temple", { MM_TEMPLE_STONE_TOWER, MM_GAME } },
    { "Pirates' Fortress Interior", { MM_PIRATE_FORTRESS_INTERIOR, MM_GAME } },
    { "Pirates' Fortress Sewers", { MM_PIRATE_FORTRESS_INTERIOR, MM_GAME } },
    { "Pirates' Fortress Exterior", { MM_PIRATE_FORTRESS_ENTRANCE, MM_GAME } },
    { "Beneath The Well", { MM_BENEATH_THE_WELL, MM_GAME } },
    { "Secret Shrine", { MM_SECRET_SHRINE, MM_GAME } },
    { "Swamp Spider House", { MM_SPIDER_HOUSE_SWAMP, MM_GAME } },
    { "Ocean Spider House", { MM_SPIDER_HOUSE_OCEAN, MM_GAME } },
    { "Clock Tower Roof", { MM_CLOCK_TOWER_ROOFTOP, MM_GAME } },
    { "The Moon", { MM_MOON, MM_GAME } }
};


LogTab::LogTab(OoTMMComboTracker* Owner, QWidget* parent) : QWidget(parent)
{
    this->WinOwner = Owner;

    this->IsRunning = false;
    this->EnableMultiplayer = false;

    // Launch container
    this->LaunchGroup = new QGroupBox("Launch Options");
    this->FileLayout = new QHBoxLayout();
    this->TrackLayout = new QGridLayout();

    // Save tracking
    this->SaveButton = new QPushButton("Save Tracking");
    this->SaveButton->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::DocumentSave));
    QObject::connect(this->SaveButton, &QPushButton::pressed, this, &LogTab::SaveTracking);
    this->FileLayout->addWidget(this->SaveButton);

    // Load tracking
    this->LoadButton = new QPushButton("Load Tracking");
    this->LoadButton->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::DocumentOpen));
    QObject::connect(this->LoadButton, &QPushButton::pressed, this, &LogTab::LoadTracking);
    this->FileLayout->addWidget(this->LoadButton);

    // Load spoiler
    this->LoadSpoilerButton = new QPushButton("Load Spoiler Log");
    this->LoadSpoilerButton->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::SystemLogOut));
    QObject::connect(this->LoadSpoilerButton, &QPushButton::pressed, this, &LogTab::LoadSpoiler);
    this->FileLayout->addWidget(this->LoadSpoilerButton);

    // Reset tracking
    this->ResetButton = new QPushButton("Reset Tracking");
    this->ResetButton->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::SystemReboot));
    QObject::connect(this->ResetButton, &QPushButton::pressed, this, &LogTab::ResetTracking);
    this->FileLayout->addWidget(this->ResetButton);

    // Start tracking
    this->LaunchButton = new QPushButton("Start Tracking");
    this->LaunchButton->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackStart));
    QObject::connect(this->LaunchButton, &QPushButton::pressed, this, &LogTab::PressLaunchButton);

    // Multiplayer checkbox
    this->EnableMultiplayer = AppConfig::GetUseMultiplayer();
    this->NetCheckBox = new QCheckBox("Use Multiplayer");
    this->NetCheckBox->setChecked(this->EnableMultiplayer);
    QObject::connect(this->NetCheckBox, &QCheckBox::checkStateChanged, this, &LogTab::ToggleNetOption);

    // Host field
    this->Host = new QLineEdit;
    this->Host->setPlaceholderText("Enter the host server address");
    this->Host->setToolTip("Example : multi.ootmm.com");
    this->Host->setText("multi.ootmm.com");
    this->Host->setEnabled(this->EnableMultiplayer);

    // Port field
    this->Port = new QLineEdit;
    this->Port->setPlaceholderText("Enter the port to use");
    this->Port->setToolTip("Integer between 0 et 65535. Default: 13248");
    this->Port->setText("13248");
    this->Port->setEnabled(this->EnableMultiplayer);

    // Port validator
    this->PortValidator = new QIntValidator(0, 65535);
    this->Port->setValidator(this->PortValidator);

    // Network layout : checkbox -> host -> port
    this->MultiLayout = new QHBoxLayout;
    this->MultiLayout->addWidget(this->NetCheckBox);
    this->MultiLayout->addWidget(this->Host);
    this->MultiLayout->addWidget(this->Port);


    // Launch layout
    this->NetLayout = new QVBoxLayout;
    this->NetLayout->addLayout(this->FileLayout);
    this->NetLayout->addLayout(this->MultiLayout);   // Ajouter la checkbox
    this->NetLayout->addWidget(this->LaunchButton);  // Ajouter le bouton
    this->LaunchGroup->setLayout(this->NetLayout);

    // Main layout
    this->MainLayout = new QVBoxLayout;
    this->MainLayout->addWidget(this->LaunchGroup);

    QFont font("Consolas");
    font.setStyleHint(QFont::Monospace);
    font.setFixedPitch(true);
	this->LogViewer = new QPlainTextEdit;
    this->LogViewer->setReadOnly(true);
    this->LogViewer->setFont(font);
    this->MainLayout->addWidget(this->LogViewer);
	this->setLayout(this->MainLayout);

    // Init other attribute
    this->Tracker = new App();
    this->MemRead = new MemoryReader();

    // Used to catch messages that come from non GUI thread
    QObject::connect(MultiLogger::GetLogger(), &MultiLogger::LogMsgToView, this, &LogTab::LogMessage);
}


LogTab::~LogTab()
{
    if (this->Tracker && this->Tracker->IsRunning)
    {
        this->Tracker->IsRunning = false;
        this->TrackerThread.join();
    }

    if (this->MemRead && this->MemRead->IsRunning)
    {
        this->MemRead->IsRunning = false;
        this->MemReaderThread.join();
    }

    delete this->MemRead;
    delete this->Tracker;
    delete this->LaunchButton;
    delete this->NetCheckBox;
    delete this->Host;
    delete this->PortValidator;
    delete this->Port;
    delete this->MultiLayout;
    delete this->NetLayout;
    delete this->LaunchGroup;
    delete this->LogViewer;
    delete this->MainLayout;
}


const QHash<QString, QPair<uint32_t, uint32_t>> LogTab::GetSpoilerMap()
{
    return SpoilerMap;
}


void LogTab::ToggleNetOption(int state)
{
    if (state == Qt::Checked)
    {
        this->EnableMultiplayer = true;
    }
    else
    {
        this->EnableMultiplayer = false;
    }
    AppConfig::SetUseMultiplayer(this->EnableMultiplayer);
    this->Host->setEnabled(this->EnableMultiplayer);
    this->Port->setEnabled(this->EnableMultiplayer);
}


void LogTab::PressLaunchButton()
{
    if (this->Tracker)
    {
        QIcon launchIcon(QIcon::fromTheme(QString::fromUtf8("QIcon::ThemeIcon::MediaPlaybackStart")));
        QString trackText = "Start Tracking";
        if (this->Tracker->IsRunning || this->MemRead->IsRunning)
        {   // Stop the auto-tracker

            if (this->Tracker->IsRunning)
            {
                this->Tracker->IsRunning = false;
                this->TrackerThread.join();
                this->Tracker->appQuit();
            }
            this->MemRead->IsRunning = false;
            this->MemReaderThread.join();
            this->MemRead->ResetMemoryReader();
        }
        else
        {   // Start the auto-tracker


            if (this->EnableMultiplayer)
            {   // Use multiplayer

                // Default host / ports from Nax's multi client app
                if (this->Tracker->appInit())
                {
                    return;
                }
                if (this->Tracker->appStartPj64("localhost", 13249))
                {
                    this->Tracker->appQuit();
                    return;
                }
                if (this->Tracker->appStartAres("localhost", 9123))
                {
                    this->Tracker->appQuit();
                    return;
                }
                this->Tracker->IsRunning = true;
                this->TrackerThread = std::thread(&App::appRun, this->Tracker, this->EnableMultiplayer, this->Host, this->Port->text().toUShort());
            }

            this->MemReaderThread = std::thread(&MemoryReader::StartMemoryReader, this->MemRead);

            trackText = "Stop Tracking";
            launchIcon = QIcon::fromTheme(QString::fromUtf8("QIcon::ThemeIcon::MediaPlaybackStop"));
        }

        this->LaunchButton->setIcon(launchIcon);
        this->LaunchButton->setText(trackText);
        this->WinOwner->UpdateTrackingState(trackText, launchIcon);
    }
}


void LogTab::SaveTracking()
{
    QString filePath = QFileDialog::getSaveFileName(this, "Choose a file name", "", "Tracking Files (*.trck)");

    if (!filePath.isEmpty())
    {
        GameTab::SaveGameScenes(filePath, &this->WinOwner->ROMSettings);
    }
}


void LogTab::ResetTracking()
{
    if (AppConfig::GetAutoSave())
    {
        this->WinOwner->CreatePath(AppConfig::GetAutoSavePath());
        GameTab::SaveGameScenes(AppConfig::GetAutoSaveFullPath(), &this->WinOwner->ROMSettings);
        LogTab::LogMessage("Closing auto save file.\n");
        LogTab::LogMessage("Creating a new auto save file.\n");
        AppConfig::SetAutoSavePath("AutoSave-" + QDateTime::currentDateTime().toString("dd_MM_yyyy_hh_mm_ss") + ".trck");
    }
    LogTab::LogMessage("Resetting tracked items.");
    GameTab::ResetScenes();
    this->WinOwner->RefreshTracker();
}


void LogTab::LoadTracking()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Choose a tracking file", "", "Tracking Files (*.trck)");

    if (!filePath.isEmpty())
    {
        this->WinOwner->LoadGameScenes(filePath);
    }
}


void LogTab::LoadSpoiler()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Choose a spoiler log", "", "Text Files (*.txt)");

    if (!filePath.isEmpty())
    {   // We have found a file

        this->WinOwner->LoadGameSpoiler(filePath);
    }
}


void LogTab::LogMessage(const QString& Message)
{
    this->LogViewer->appendPlainText(Message);
    this->LogViewer->verticalScrollBar()->setValue(this->LogViewer->verticalScrollBar()->maximum());
}