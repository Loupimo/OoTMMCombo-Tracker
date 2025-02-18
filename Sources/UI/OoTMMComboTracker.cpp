#include "Combo/Objects.h"
#include "UI/OoTMMComboTracker.h"

OoTMMComboTracker::OoTMMComboTracker(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    this->TabWidget = new QTabWidget;

    this->OoTTab = new GameTab(OOT_GAME);
    this->MMTab = new GameTab(MM_GAME);
    this->Log = new LogTab(this);
    this->TabWidget->addTab(this->Log, "Launch");
    this->TabWidget->addTab(this->OoTTab, this->OoTTab->TabName);
    this->TabWidget->addTab(this->MMTab, this->MMTab->TabName);
    this->OoTTab->Owner = this;
    this->MMTab->Owner = this;
    this->UpdateTabNameText(0);
    this->UpdateTabNameText(1);
    //this->Maps = new MapTab("./Resources/OoT/Pot_House.png");
    //this->Maps = new MapTab("./Resources/OoT/Kokiri_Forest/Link_House.png");
    
    //this->TabWidget->addTab(this->Maps, "Pot House");
    //ObjectRenderer* h = new ObjectRenderer(ObjectType::pot, this);
    //this->TabWidget->addTab(h, "o");
    this->setCentralWidget(this->TabWidget);
    this->setWindowTitle("OoTMMCombo Auto Tracker");
    this->setWindowIcon(QIcon("./Resources/Logo.ico"));
    /*mainWindow.resize(800, 600);
    mainWindow.show();*/

    connect(MultiLogger::GetLogger(), &MultiLogger::NotifyObjectFound, this, &OoTMMComboTracker::UpdateTrackedObject);
}

OoTMMComboTracker::~OoTMMComboTracker()
{
    delete this->Log;
    delete this->OoTTab;
    delete this->MMTab;
}


void OoTMMComboTracker::UpdateTabNameText(int TabID)
{
    GameTab* activeTab = nullptr;

    if (TabID == 0)
    {
        activeTab = this->OoTTab;
    }
    else
    {
        activeTab = this->MMTab;
    }

    const size_t max_size = 150;
    char finalName[max_size] = { 0 };
    char tmp[5] = { 0 };

    size_t offset = 0;
    size_t typeLen = strlen(activeTab->TabName);
    memcpy_s(finalName, max_size, activeTab->TabName, typeLen);
    offset += typeLen;
    finalName[offset] = ' ';
    finalName[offset + 1] = '(';
    offset += 2;

    _itoa_s((int)activeTab->FoundObjects, tmp, 10);

    memcpy_s(finalName + offset, max_size - offset, tmp, strlen(tmp));
    offset += strlen(tmp);

    finalName[offset] = ' ';
    finalName[offset + 1] = '/';
    finalName[offset + 2] = ' ';

    offset += 3;

    _itoa_s(activeTab->TotalObjects, tmp, 10);

    memcpy_s(finalName + offset, max_size - offset, tmp, strlen(tmp));

    offset += strlen(tmp);
    finalName[offset] = ')';
    finalName[offset + 1] = '\0';
    this->TabWidget->setTabText(TabID + 1, finalName);
}

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


void OoTMMComboTracker::LoadGameScenes(QString FilePath)
{
    //GameTab::LoadGameScenes(FilePath);
    QFile loadFile(FilePath);
    if (!loadFile.open(QIODevice::ReadOnly))
    {
        MultiLogger::LogMessage("Can't open file: %s\n", FilePath.toStdString().c_str());
        return;
    }

    QByteArray data = loadFile.readAll();

    LoadSceneObjects(&data, 0);
    loadFile.close();

    MultiLogger::LogMessage("File loaded: %s\n", FilePath.toStdString().c_str());

    this->OoTTab->RefreshGameTab();
    this->MMTab->RefreshGameTab();
}


void OoTMMComboTracker::LoadGameSpoiler()
{
    //GameTab::LoadGameSpoiler(FilePath);
    this->OoTTab->RefreshGameTab();
    this->MMTab->RefreshGameTab();
}