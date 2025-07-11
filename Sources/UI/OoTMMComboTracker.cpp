#include "Combo/Objects.h"
#include "UI/AppConfig.h"
#include "UI/OoTMMComboTracker.h"
#include <QMessageBox>
#include <QFileDialog>

OoTMMComboTracker::OoTMMComboTracker(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    // The main tab widget
    this->TabWidget = new QTabWidget;

    // The log / lauch tab
    this->Log = new LogTab(this);

    // Create OoT and MM game tabs
    this->OoTTab = new GameTab(OOT_GAME, this);
    this->OoTTab->Owner = this;
    this->MMTab = new GameTab(MM_GAME, this);
    this->MMTab->Owner = this;
    
    // Add the tabs to the widget : Log -> OoT -> MM
    this->TabWidget->addTab(this->Log, "Launch");
    this->TabWidget->addTab(this->OoTTab, this->OoTTab->TabName);
    this->TabWidget->addTab(this->MMTab, this->MMTab->TabName);

    // Update game tabs name
    this->UpdateTabNameText(0);
    this->UpdateTabNameText(1);
    this->setCentralWidget(this->TabWidget);
    this->setWindowTitle("OoTMMCombo Auto Tracker");
    this->setWindowIcon(QIcon("./Resources/Logo.ico"));

    // Loads default value
    this->ui.actionAutoSnapView->setChecked(AppConfig::GetAutoSnapView());
    this->ui.actionAutoZoom->setChecked(AppConfig::GetAutoZoom());
    this->ui.actionAutoSaving->setChecked(AppConfig::GetAutoSave());

    connect(MultiLogger::GetLogger(), &MultiLogger::NotifyObjectFound, this, &OoTMMComboTracker::UpdateTrackedObject);
    connect(this->ui.actionSaveSession, &QAction::triggered, this->Log, &LogTab::SaveTracking);
    connect(this->ui.actionLoadSession, &QAction::triggered, this->Log, &LogTab::LoadTracking);
    connect(this->ui.actionAutoSnapView, &QAction::toggled, this, &AppConfig::SetAutoSnapView);
    connect(this->ui.actionAutoZoom, &QAction::toggled, this, &AppConfig::SetAutoZoom);
    connect(this->ui.actionAutoSaving, &QAction::toggled, this, &AppConfig::SetAutoSave);
    connect(this->ui.actionAbout, &QAction::triggered, this, &OoTMMComboTracker::ShowAboutDialog);
}

OoTMMComboTracker::~OoTMMComboTracker()
{
    delete this->Log;
    delete this->OoTTab;
    delete this->MMTab;
    delete this->TabWidget;
}

void OoTMMComboTracker::ShowAboutDialog()
{
    QMessageBox::about(this, "About", "OoTMMCombo Tracker\nVersion 1.0\n(c) 2025 Loupimo");
}

void OoTMMComboTracker::CreatePath(QString PathToCreate)
{
    QDir dir(PathToCreate);

    if (!dir.exists())
    {
        if (dir.mkpath("."))
        {
            MultiLogger::LogMessage("Created path : %s", PathToCreate.toStdString().c_str());
        }
        else
        {
            MultiLogger::LogMessage("Can't created path : %s", PathToCreate.toStdString().c_str());
        }
    }
}

#pragma region Object related

void OoTMMComboTracker::UpdateTrackedObject(int Game, ObjectInfo* ObjectFound, const ItemInfo* ItemFound)
{
    switch (Game)
    {
        case OOT_GAME:
        {
            this->OoTTab->ItemFound(ObjectFound, ItemFound);
            if (AppConfig::GetAutoSave())
            {
                this->CreatePath(AppConfig::GetAutoSavePath());
                GameTab::SaveGameScenes(AppConfig::GetAutoSaveFullPath());
            }
            break;
        }
        case MM_GAME:
        {
            this->MMTab->ItemFound(ObjectFound, ItemFound);
            if (AppConfig::GetAutoSave())
            {
                this->CreatePath(AppConfig::GetAutoSavePath());
                GameTab::SaveGameScenes(AppConfig::GetAutoSaveFullPath());
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

void OoTMMComboTracker::UpdateTabNameText(int TabID)
{
    GameTab* activeTab = nullptr;

    if (TabID == OOT_GAME)
    {   // OoT

        activeTab = this->OoTTab;
    }
    else
    {   // MM

        activeTab = this->MMTab;
    }

    const size_t max_size = 150;
    char finalName[max_size] = { 0 };
    char tmp[5] = { 0 };

    // Initialize the string with : GameName (
    size_t offset = 0;
    size_t typeLen = strlen(activeTab->TabName);
    memcpy_s(finalName, max_size, activeTab->TabName, typeLen);
    offset += typeLen;
    finalName[offset] = ' ';
    finalName[offset + 1] = '(';
    offset += 2;

    // Add the number of found object : GameName (foundObjs / 
    _itoa_s((int)activeTab->FoundObjects, tmp, 10);
    memcpy_s(finalName + offset, max_size - offset, tmp, strlen(tmp));
    offset += strlen(tmp);
    finalName[offset] = ' ';
    finalName[offset + 1] = '/';
    finalName[offset + 2] = ' ';
    offset += 3;

    // Add the total number of object : GameName (foundObjs / totObjs)
    _itoa_s(activeTab->TotalObjects, tmp, 10);
    memcpy_s(finalName + offset, max_size - offset, tmp, strlen(tmp));
    offset += strlen(tmp);
    finalName[offset] = ')';
    finalName[offset + 1] = '\0';
    this->TabWidget->setTabText(TabID + 1, finalName);
}

#pragma endregion

#pragma region Saving / Loading

void OoTMMComboTracker::LoadGameScenes(QString FilePath)
{
    QFile loadFile(FilePath);
    if (!loadFile.open(QIODevice::ReadOnly))
    {
        MultiLogger::LogMessage("Can't open file: %s\n", FilePath.toStdString().c_str());
        return;
    }

    // Load file data
    QByteArray data = loadFile.readAll();

    LoadSceneObjects(&data, 0);
    loadFile.close();

    MultiLogger::LogMessage("File loaded: %s\n", FilePath.toStdString().c_str());

    // Refresh game tabs
    this->OoTTab->RefreshGameTab();
    this->MMTab->RefreshGameTab();
}


void OoTMMComboTracker::LoadGameSpoiler()
{
    this->OoTTab->RefreshGameTab();
    this->MMTab->RefreshGameTab();
}

#pragma endregion