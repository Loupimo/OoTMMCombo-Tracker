#include <QFileDialog>
#include <QRegularExpression>
#include "UI/LogTab.h"
#include "UI/GameTab.h"
#include "UI/OoTMMComboTracker.h"
#include "Combo/Scenes.h"

// Contains all the association between the spoiler location and their matching scene in this program
const QHash<QString, QPair<uint32_t, uint32_t>> SpoilerMap =
{
    { "Inside Eggs", { INSIDE_EGGS, OOT_GAME } },
    { "Kokiri Forest", { KOKIRI_FOREST, OOT_GAME } },
    { "Hyrule Field", { HYRULE_FIELD, OOT_GAME } },
    { "Market", { MARKET, OOT_GAME } },
    { "Lon Lon Ranch", { LON_LON_RANCH, OOT_GAME } },
    { "Hyrule Castle", { HYRULE_CASTLE, OOT_GAME } },
    { "Outside Ganon's Castle", { GANON_CASTLE_EXTERIOR, OOT_GAME } },
    { "Lost Woods", { OOT_LOST_WOODS, OOT_GAME } },
    { "Sacred Forest Meadow", { SACRED_FOREST_MEADOW, OOT_GAME } },
    { "Kakariko", { KAKARIKO_VILLAGE, OOT_GAME } },
    { "Graveyard", { GRAVEYARD, OOT_GAME } },
    { "Death Mountain Trail", { DEATH_MOUNTAIN_TRAIL, OOT_GAME } },
    { "Goron City", { GORON_CITY, OOT_GAME } },
    { "Zora's River", { ZORA_RIVER, OOT_GAME } },
    { "Zora's Domain", { ZORA_DOMAIN, OOT_GAME } },
    { "Lake Hylia", { LAKE_HYLIA, OOT_GAME } },
    { "Zora's Fountain", { ZORA_FOUNTAIN, OOT_GAME } },
    { "Temple of Time", { TEMPLE_OF_TIME, OOT_GAME} },
    { "Sacred Realm", { TEMPLE_OF_TIME, OOT_GAME } },
    { "Death Mountain Crater", { DEATH_MOUNTAIN_CRATER, OOT_GAME } },
    { "Gerudo Valley", { GERUDO_VALLEY, OOT_GAME } },
    { "Gerudo's Fortress", { GERUDO_FORTRESS, OOT_GAME } },
    { "Haunted Wasteland", { HAUNTED_WASTELAND, OOT_GAME } },
    { "Desert Colossus", { DESERT_COLOSSUS, OOT_GAME } },
    { "Deku Tree", { DEKU_TREE, OOT_GAME } },
    { "Dodongo's Cavern", { DODONGO_CAVERN, OOT_GAME } },
    { "Jabu-Jabu's Belly", { INSIDE_JABU_JABU, OOT_GAME } },
    { "Forest Temple", { TEMPLE_FOREST, OOT_GAME } },
    { "Fire Temple", { TEMPLE_FIRE, OOT_GAME } },
    { "Water Temple", { TEMPLE_WATER, OOT_GAME } },
    { "Spirit Temple", { TEMPLE_SPIRIT, OOT_GAME } },
    { "Shadow Temple", { TEMPLE_SHADOW, OOT_GAME } },
    { "Bottom of the Well", { BOTTOM_OF_THE_WELL, OOT_GAME } },
    { "Ice Cavern", { ICE_CAVERN, OOT_GAME } },
    { "Thieves' Hideout", { THIEVES_HIDEOUT, OOT_GAME } },
    { "Gerudo's Training Ground", { GERUDO_TRAINING_GROUND, OOT_GAME } },
    { "Ganon's Castle", { INSIDE_GANON_CASTLE, OOT_GAME } },
    { "Tingle", { CLOCK_TOWN_NORTH, MM_GAME } },
    { "South Clock Town", { CLOCK_TOWN_SOUTH, MM_GAME } },
    { "Milk Road", { MILK_ROAD, MM_GAME } },
    { "Southern Swamp", { SOUTHERN_SWAMP, MM_GAME } },
    { "Woodfall", { WOODFALL, MM_GAME } },
    { "Mountain Village", { MOUNTAIN_VILLAGE, MM_GAME } },
    { "Snowhead", { SNOWHEAD, MM_GAME } },
    { "Great Bay Coast", { GREAT_BAY_COAST, MM_GAME } },
    { "Zora Cape", { ZORA_CAPE, MM_GAME } },
    { "Ikana Canyon", { IKANA_CANYON, MM_GAME } },
    { "Stone Tower", { STONE_TOWER, MM_GAME } },
    { "Giant's Dream", { TEMPLE_WOODFALL, MM_GAME } },
    { "North Clock Town", { CLOCK_TOWN_NORTH, MM_GAME } },
    { "West Clock Town", { CLOCK_TOWN_WEST, MM_GAME } },
    { "East Clock Town", { CLOCK_TOWN_EAST, MM_GAME } },
    { "Laundry Pool", { LAUNDRY_POOL, MM_GAME } },
    { "Stock Pot Inn", { STOCK_POT_INN, MM_GAME } },
    { "Termina Field", { TERMINA_FIELD, MM_GAME } },
    { "Road to Southern Swamp", { ROAD_SOUTHERN_SWAMP, MM_GAME } },
    { "Deku Palace", { DEKU_PALACE, MM_GAME } },
    { "Path to Mountain Village", { PATH_MOUNTAIN_VILLAGE, MM_GAME } },
    { "Twin Islands", { TWIN_ISLANDS, MM_GAME } },
    { "Goron Village", { GORON_VILLAGE_WINTER, MM_GAME } },
    { "Road to Snowhead", { PATH_SNOWHEAD, MM_GAME } },
    { "Romani Ranch", { ROMANI_RANCH, MM_GAME } },
    { "Pinnacle Rock", { PINNACLE_ROCK, MM_GAME } },
    { "Zora Hall", { ZORA_HALL_ROOMS, MM_GAME } },
    { "Road to Ikana", { ROAD_IKANA, MM_GAME } },
    { "Ikana Graveyard", { IKANA_GRAVEYARD, MM_GAME } },
    { "Ikana Castle", { CASTLE_IKANA, MM_GAME } },
    { "Woodfall Temple", { TEMPLE_WOODFALL, MM_GAME } },
    { "Snowhead Temple", { TEMPLE_SNOWHEAD, MM_GAME } },
    { "Great Bay Temple", { TEMPLE_GREAT_BAY, MM_GAME } },
    { "Stone Tower Temple", { TEMPLE_STONE_TOWER, MM_GAME } },
    { "Pirates' Fortress Interior", { PIRATE_FORTRESS_INTERIOR, MM_GAME } },
    { "Pirates' Fortress Sewers", { PIRATE_FORTRESS_INTERIOR, MM_GAME } },
    { "Pirates' Fortress Exterior", { PIRATE_FORTRESS_ENTRANCE, MM_GAME } },
    { "Beneath The Well", { BENEATH_THE_WELL, MM_GAME } },
    { "Secret Shrine", { SECRET_SHRINE, MM_GAME } },
    { "Swamp Spider House", { MM_SPIDER_HOUSE_SWAMP, MM_GAME } },
    { "Ocean Spider House", { MM_SPIDER_HOUSE_OCEAN, MM_GAME } },
    { "Clock Tower Roof", { CLOCK_TOWER_ROOFTOP, MM_GAME } },
    { "The Moon", { MOON, MM_GAME } }
};


LogTab::LogTab(OoTMMComboTracker* Owner, QWidget* parent) : QWidget(parent)
{
    this->WinOwner = Owner;

    this->IsRunning = false;
    this->EnableMultiplayer = false;

    // Launch container
    this->LaunchGroup = new QGroupBox("Launch Options");
    this->FileLayout = new QHBoxLayout();

    // Save tracking
    this->SaveButton = new QPushButton("Save Tracking");
    QObject::connect(this->SaveButton, &QPushButton::pressed, this, &LogTab::SaveTracking);
    this->FileLayout->addWidget(this->SaveButton);

    // Load tracking
    this->LoadButton = new QPushButton("Load Tracking");
    QObject::connect(this->LoadButton, &QPushButton::pressed, this, &LogTab::LoadTracking);
    this->FileLayout->addWidget(this->LoadButton);

    // Load spoiler
    this->LoadSpoilerButton = new QPushButton("Load Spoiler Log");
    QObject::connect(this->LoadSpoilerButton, &QPushButton::pressed, this, &LogTab::LoadSpoiler);
    this->FileLayout->addWidget(this->LoadSpoilerButton);

    // Start tracking
    this->LaunchButton = new QPushButton("Start Tracking");
    this->LaunchButton->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackStart));
    QObject::connect(this->LaunchButton, &QPushButton::pressed, this, &LogTab::PressLaunchButton);

    // Multiplayer checkbox
    this->NetCheckBox = new QCheckBox("Use Multiplayer");
    this->NetCheckBox->setChecked(false);
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

	this->LogViewer = new QPlainTextEdit;
    this->LogViewer->setReadOnly(true);
    this->MainLayout->addWidget(this->LogViewer);
	this->setLayout(this->MainLayout);

    // Init other attribute
    this->Tracker = new App();

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
    this->Host->setEnabled(this->EnableMultiplayer);
    this->Port->setEnabled(this->EnableMultiplayer);
}


void LogTab::PressLaunchButton()
{
    if (this->Tracker)
    {
        QIcon launchIcon(QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackStart));
        QString trackText = "Start Tracking";
        if (this->Tracker->IsRunning)
        {   // Stop the auto-tracker

            this->Tracker->IsRunning = false;
            this->TrackerThread.join();
            this->Tracker->appQuit();
        }
        else
        {   // Start the auto-tracker

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
            trackText = "Stop Tracking";
            launchIcon = QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackStop);
            this->Tracker->IsRunning = true;

            this->TrackerThread = std::thread(&App::appRun, this->Tracker, this->EnableMultiplayer, this->Host, this->Port->text().toUShort());
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
        GameTab::SaveGameScenes(filePath);
    }
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