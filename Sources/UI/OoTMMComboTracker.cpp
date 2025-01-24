#include "UI/OoTMMComboTracker.h"

OoTMMComboTracker::OoTMMComboTracker(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    this->TabWidget = new QTabWidget;

    this->Log = new LogTab();
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
    /*mainWindow.resize(800, 600);
    mainWindow.show();*/

    connect(MultiLogger::GetLogger(), &MultiLogger::NotifyObjectFound, this, &OoTMMComboTracker::UpdateTrackedObject);
}

OoTMMComboTracker::~OoTMMComboTracker()
{
    //this->Maps->~MapTab();
    this->Log->~LogTab();
}


void OoTMMComboTracker::UpdateTrackedObject(int Game, ObjectInfo* ObjectFound, ItemInfo ItemFound)
{
    switch (Game)
    {
        case OOT_GAME:
        {
            break;
        }
        case MM_GAME:
        {
            break;
        }
        default:
        {
            break;
        }
    }
}