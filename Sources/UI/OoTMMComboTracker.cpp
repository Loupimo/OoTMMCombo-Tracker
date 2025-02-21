#include "Combo/Objects.h"
#include "UI/OoTMMComboTracker.h"

OoTMMComboTracker::OoTMMComboTracker(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    // The main tab widget
    this->TabWidget = new QTabWidget;

    // The log / lauch tab
    this->Log = new LogTab(this);

    // Create OoT and MM game tabs
    this->OoTTab = new GameTab(OOT_GAME);
    this->OoTTab->Owner = this;
    this->MMTab = new GameTab(MM_GAME);
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

    connect(MultiLogger::GetLogger(), &MultiLogger::NotifyObjectFound, this, &OoTMMComboTracker::UpdateTrackedObject);
}

OoTMMComboTracker::~OoTMMComboTracker()
{
    delete this->Log;
    delete this->OoTTab;
    delete this->MMTab;
    delete this->TabWidget;
}

#pragma region Object related

void OoTMMComboTracker::UpdateTrackedObject(int Game, ObjectInfo* ObjectFound, const ItemInfo* ItemFound)
{
    switch (Game)
    {
        case OOT_GAME:
        {
            this->OoTTab->ItemFound(ObjectFound, ItemFound);
            break;
        }
        case MM_GAME:
        {
            this->MMTab->ItemFound(ObjectFound, ItemFound);
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