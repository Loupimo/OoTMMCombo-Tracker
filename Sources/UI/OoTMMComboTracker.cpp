#include "UI/OoTMMComboTracker.h"

OoTMMComboTracker::OoTMMComboTracker(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    this->TabWidget = new QTabWidget;

    this->Log = new LogTab();
    this->TabWidget->addTab(this->Log, "Launch");
    this->Maps = new MapTab("./Resources/OoT/Pot_House.png");
    this->TabWidget->addTab(this->Maps, "Pot House");
    //ObjectRenderer* h = new ObjectRenderer(ObjectType::pot, this);
    //this->TabWidget->addTab(h, "o");
    this->setCentralWidget(this->TabWidget);
    this->setWindowTitle("OoTMMCombo Auto Tracker");
    /*mainWindow.resize(800, 600);
    mainWindow.show();*/
}

OoTMMComboTracker::~OoTMMComboTracker()
{
    this->Maps->~MapTab();
    this->Log->~LogTab();
}


// Onglet pour les logs
/*QWidget* logTab = new QWidget;
QVBoxLayout* logLayout = new QVBoxLayout;
QPlainTextEdit* logViewer = new QPlainTextEdit;
logViewer->setReadOnly(true);
logViewer->appendPlainText("Bienvenue dans l'application");
logLayout->addWidget(logViewer);
logTab->setLayout(logLayout);

tabWidget->addTab(logTab, "Logs");

// Configurer la fenêtre principale
mainWindow.setCentralWidget(tabWidget);
mainWindow.setWindowTitle("Application avec onglets");
mainWindow.resize(800, 600);
mainWindow.show();

return app.exec();*/