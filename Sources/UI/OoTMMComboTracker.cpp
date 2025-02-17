#include "Combo/Objects.h"
#include "UI/OoTMMComboTracker.h"

OoTMMComboTracker::OoTMMComboTracker(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    this->TabWidget = new QTabWidget;

    this->Log = new LogTab(this);
    this->TabWidget->addTab(this->Log, "Launch");
    this->TabWidget->addTab(&this->OoTTab, this->OoTTab.TabName);
    this->TabWidget->addTab(&this->MMTab, this->MMTab.TabName);
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
    //this->Maps->~MapTab();
    this->Log->~LogTab();
}


void OoTMMComboTracker::UpdateTrackedObject(int Game, ObjectInfo* ObjectFound, const ItemInfo* ItemFound)
{
    switch (Game)
    {
        case OOT_GAME:
        {
            this->OoTTab.ItemFound(ObjectFound, ItemFound);
            break;
        }
        case MM_GAME:
        {
            this->MMTab.ItemFound(ObjectFound, ItemFound);
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

    this->OoTTab.RefreshGameTab();
    this->MMTab.RefreshGameTab();
}


void OoTMMComboTracker::LoadGameSpoiler()
{
    //GameTab::LoadGameSpoiler(FilePath);
    this->OoTTab.RefreshGameTab();
    this->MMTab.RefreshGameTab();
}