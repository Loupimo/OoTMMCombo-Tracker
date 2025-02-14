#include <QFileDialog>
#include <QRegularExpression>
#include "UI/LogTab.h"
#include "UI/GameTab.h"
#include "UI/OoTMMComboTracker.h"
#include "Combo/Scenes.h"

const QHash<QString, uint32_t> SpoilerMap =
{
    { "Inside Eggs", OOT_NUM_SCENES },
    { "Kokiri Forest", KOKIRI_FOREST },
    { "Hyrule Field", HYRULE_FIELD },
    { "Market", MARKET },
    { "Lon Lon Ranch", LON_LON_RANCH },
    { "Hyrule Castle", HYRULE_CASTLE },
    { "Outside Ganon", GANON_CASTLE_EXTERIOR },
    { "Lost Woods", OOT_LOST_WOODS },
    { "Sacred Forest Meadow", SACRED_FOREST_MEADOW },
    { "Kakariko", KAKARIKO_VILLAGE },
    { "Graveyard", GRAVEYARD },
    { "Death Mountain Trail", DEATH_MOUNTAIN_TRAIL },
    { "Goron City", GORON_CITY },
    { "Zora's River", ZORA_RIVER },
    { "Zora's Domain", ZORA_DOMAIN },
    { "Lake Hylia", LAKE_HYLIA },
    { "Zora's Fountain", ZORA_FOUNTAIN },
    { "Temple of Time", TEMPLE_OF_TIME },
    { "Sacred Realm", TEMPLE_OF_TIME },
    { "Death Mountain Crater", DEATH_MOUNTAIN_CRATER },
    { "Gerudo Valley", GERUDO_VALLEY },
    { "Gerudo's Fortress", GERUDO_FORTRESS },
    { "Haunted Wasteland", HAUNTED_WASTELAND },
    { "Desert Colossus", DESERT_COLOSSUS },
    { "Deku Tree", DEKU_TREE },
    { "Dodongo's Cavern", DODONGO_CAVERN },
    { "Jabu-Jabu's Belly", INSIDE_JABU_JABU },
    { "Forest Temple", TEMPLE_FOREST },
    { "Fire Temple", TEMPLE_FIRE },
    { "Water Temple", TEMPLE_WATER },
    { "Spirit Temple", TEMPLE_SPIRIT },
    { "Shadow Temple", TEMPLE_SHADOW },
    { "Bottom of the Well", BOTTOM_OF_THE_WELL },
    { "Ice Cavern", ICE_CAVERN },
    { "Thieves' Hideout", THIEVES_HIDEOUT },
    { "Gerudo's Training Ground", GERUDO_TRAINING_GROUND },
    { "Ganon's Castle", INSIDE_GANON_CASTLE },
    { "Tingle", CLOCK_TOWN_NORTH },
    { "South Clock Town", CLOCK_TOWN_SOUTH },
    { "Milk Road", MILK_ROAD },
    { "Southern Swamp", SOUTHERN_SWAMP },
    { "Woodfall", WOODFALL },
    { "Mountain Village", MOUNTAIN_VILLAGE },
    { "Snowhead", SNOWHEAD },
    { "Great Bay Coast", GREAT_BAY_COAST },
    { "Zora Cape", ZORA_CAPE },
    { "Ikana Canyon", IKANA_CANYON },
    { "Stone Tower", STONE_TOWER },
    { "Giant's Dream", TEMPLE_WOODFALL },
    { "North Clock Town", CLOCK_TOWN_NORTH },
    { "West Clock Town", CLOCK_TOWN_WEST },
    { "East Clock Town", CLOCK_TOWN_EAST },
    { "Laundry Pool", LAUNDRY_POOL },
    { "Stock Pot Inn", STOCK_POT_INN },
    { "Termina Field", TERMINA_FIELD },
    { "Road to Southern Swamp", ROAD_SOUTHERN_SWAMP },
    { "Deku Palace", DEKU_PALACE },
    { "Path to Mountain Village", PATH_MOUNTAIN_VILLAGE },
    { "Twin Islands", TWIN_ISLANDS },
    { "Goron Village", GORON_VILLAGE_WINTER },
    { "Road to Snowhead", PATH_SNOWHEAD },
    { "Romani Ranch", ROMANI_RANCH },
    { "Pinnacle Rock", PINNACLE_ROCK },
    { "Zora Hall", ZORA_HALL_ROOMS },
    { "Road to Ikana", ROAD_IKANA },
    { "Ikana Graveyard", IKANA_GRAVEYARD },
    { "Ikana Castle", CASTLE_IKANA },
    { "Woodfall Temple", TEMPLE_WOODFALL },
    { "Snowhead Temple", TEMPLE_SNOWHEAD },
    { "Great Bay Temple", TEMPLE_GREAT_BAY },
    { "Stone Tower Temple", TEMPLE_STONE_TOWER },
    { "Pirates' Fortress Interior", PIRATE_FORTRESS_INTERIOR },
    { "Pirates' Fortress Sewers", PIRATE_FORTRESS_INTERIOR },
    { "Pirates' Fortress Exterior", PIRATE_FORTRESS_ENTRANCE },
    { "Beneath The Well", BENEATH_THE_WELL },
    { "Secret Shrine", SECRET_SHRINE },
    { "Swamp Spider House", SPIDER_HOUSE_SWAMP },
    { "Ocean Spider House", SPIDER_HOUSE_OCEAN },
    { "Clock Tower Roof", CLOCK_TOWER_ROOFTOP },
    { "The Moon", MOON }
};


LogTab::LogTab(OoTMMComboTracker* Owner, QWidget* parent) : QWidget(parent)
{
    this->WinOwner = Owner;

    this->IsRunning = false;
    this->EnableMultiplayer = false;

    // Conteneur principal encadré
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

    // Bouton avec texte
    this->LaunchButton = new QPushButton("Start Tracking");
    QObject::connect(this->LaunchButton, &QPushButton::pressed, this, &LogTab::PressLaunchButton);

    // Checkbox avec texte à droite
    this->NetCheckBox = new QCheckBox("Use Multiplayer");
    this->NetCheckBox->setChecked(false);
    QObject::connect(this->NetCheckBox, &QCheckBox::checkStateChanged, this, &LogTab::ToggleNetOption);

    // Champ pour URL
    this->Host = new QLineEdit;
    this->Host->setPlaceholderText("Enter the host server address");
    this->Host->setToolTip("Example : multi.ootmm.com");
    this->Host->setText("multi.ootmm.com");
    this->Host->setEnabled(this->EnableMultiplayer);

    // Champ pour un entier
    this->Port = new QLineEdit;
    this->Port->setPlaceholderText("Enter the port to use");
    this->Port->setToolTip("Integer between 0 et 65535. Default: 13248");
    this->Port->setText("13248");
    this->Port->setEnabled(this->EnableMultiplayer);

    // Ajout de validations
    QValidator* intValidator = new QIntValidator(0, 65535); // Valide les entiers
    this->Port->setValidator(intValidator);

    // Layout pour aligner les widgets horizontalement
    this->MultiLayout = new QHBoxLayout;
    this->MultiLayout->addWidget(this->NetCheckBox);   // Ajouter la checkbox
    this->MultiLayout->addWidget(this->Host);  // Ajouter le champ URL
    this->MultiLayout->addWidget(this->Port);  // Ajouter le champ Port


    // Layout pour aligner les widgets verticalement
    this->NetLayout = new QVBoxLayout;
    this->NetLayout->addLayout(this->FileLayout);
    this->NetLayout->addLayout(this->MultiLayout);   // Ajouter la checkbox
    this->NetLayout->addWidget(this->LaunchButton);  // Ajouter le bouton


    // Assigner le layout au conteneur encadré
    this->LaunchGroup->setLayout(this->NetLayout);

    // Layout principal pour la fenêtre
    this->MainLayout = new QVBoxLayout;
    this->MainLayout->addWidget(this->LaunchGroup);

	this->LogViewer = new QPlainTextEdit;
    this->LogViewer->setReadOnly(true);
    this->MainLayout->addWidget(this->LogViewer);
	this->setLayout(this->MainLayout);

    // Init other attribute
    this->Tracker = new App();
    connect(MultiLogger::GetLogger(), &MultiLogger::LogMsgToView, this, &LogTab::LogMessage);
}


LogTab::~LogTab()
{
    if (this->Tracker && this->Tracker->IsRunning)
    {
        this->Tracker->IsRunning = false;
        this->TrackerThread.join();
        this->Tracker->~App();
    }
    this->LaunchButton->~QPushButton();
    this->NetCheckBox->~QCheckBox();
    this->Host->~QLineEdit();
    this->Port->~QLineEdit();
    this->MultiLayout->~QHBoxLayout();
    this->NetLayout->~QVBoxLayout();
    this->LaunchGroup->~QGroupBox();
    this->LogViewer->~QPlainTextEdit();
    this->MainLayout->~QVBoxLayout();
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
        if (this->Tracker->IsRunning)
        {   // Stop the auto-tracker

            this->Tracker->IsRunning = false;
            this->LaunchButton->setText("Start Tracking");
            this->TrackerThread.join();
            this->Tracker->appQuit();
        }
        else
        {   // Start the auto-tracker

            if (this->Tracker->appInit())
                return;
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
            this->LaunchButton->setText("Stop Tracking");
            this->Tracker->IsRunning = true;

            this->TrackerThread = std::thread(&App::appRun, this->Tracker, this->EnableMultiplayer, this->Host, this->Port->text().toUShort());
        }
    }
}

void LogTab::LogMessage(const QString& Message)
{
    this->LogViewer->appendPlainText(Message);
    this->LogViewer->verticalScrollBar()->setValue(this->LogViewer->verticalScrollBar()->maximum());
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
    {
        //this->WinOwner->LoadGameSpoiler(filePath);
        QFile fichier(filePath);

        if (!fichier.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "Impossible d'ouvrir le fichier:" << fichier.errorString();
            return;
        }

        QTextStream in(&fichier);
        QString contenu = in.readAll(); // Lire tout le fichier
        fichier.close();

        // Découper en sections avec "==="
        QStringList sections = contenu.split("===========================================================================", Qt::SkipEmptyParts, Qt::CaseSensitive);

        // Vérifier si on a au moins 3 sections
        if (sections.size() < 3) {
            qWarning() << "Le fichier ne contient pas assez de sections.";
            return;
        }

        QString t = "Location List (4273)\n  Inside Eggs(2) :\n    OOT Hatch Chicken : Rupoor(OoT)\n    OOT Hatch Pocket Cucco : Deku Shield\n\n  Kokiri Forest(99) :\n    OOT Kokiri Forest Cow : Green Rupee(MM)\n    OOT Link's House Pot: Recovery Heart (OoT)\n    OOT Kokiri Forest Kokiri Sword Chest : Rupoor(OoT)\n    OOT Kokiri Forest GS Soil : Rupoor(OoT)";

        QRegularExpression reg("^\\s{2}(.+:(?:\n\\s{4}.*)+)\n*", QRegularExpression::MultilineOption);
        QRegularExpressionMatchIterator it = reg.globalMatch(sections[2]);

        QStringList maps;
        while (it.hasNext())
        {
            QRegularExpressionMatch match = it.next();
            maps.append(match.captured(1)); // Capture complète du bloc
        }

        QString finald = "";

        for (QString map : maps)
        {   // Browse all sections

            QStringList objects = map.split("    ");
            
            reg = QRegularExpression("^([\\w'-]+(?:\\s[\\w'-]+)*)", QRegularExpression::MultilineOption);
            it = reg.globalMatch(objects[0]);
            QString mapName = it.next().captured(1);

            finald += "{ \"" + mapName + "\", 0 },\n";
            uint32_t zdzd = SpoilerMap[mapName];
            printf("%d", finald);
        }

    }
}