#include <QApplication>
#include <QGraphicsEllipseItem>
#include <QPlainTextEdit>
#include <QLabel>
#include <QPushButton>
#include "UI/MapTab.h"

// Création d'un onglet avec une carte et un panneau latéral
MapTab::MapTab(QWidget* parent) : QWidget(parent)
{
    // Layout principal
    this->MainLayout = new QHBoxLayout;

    //this->SceneToRender = new SceneRenderer(&OoTScenes[0]);
    //this->SceneToRender->RenderScene();

    // Zone graphique pour la carte
    //this->View = new QGraphicsView(this->SceneToRender);
    this->View = new QGraphicsView();
    this->SceneToRender = new SceneRenderer(&OoTScenes[0]);

    // Panneau latéral
    QVBoxLayout* sidePanelLayout = new QVBoxLayout;
    QLabel* infoLabel = new QLabel("Informations sur les éléments");
    QPushButton* button = new QPushButton("Action");
    sidePanelLayout->addWidget(infoLabel);
    sidePanelLayout->addWidget(button);
    sidePanelLayout->addStretch();

    QWidget* sidePanel = new QWidget;
    sidePanel->setLayout(sidePanelLayout);

    // Ajouter à la mise en page principale
    this->MainLayout->addWidget(this->View);
    this->MainLayout->addWidget(sidePanel);

    this->setLayout(this->MainLayout);
}

MapTab::~MapTab()
{
    this->SceneToRender->~SceneRenderer();
    this->SceneToRender = nullptr;
    this->View->~QGraphicsView();
    this->MainLayout->~QHBoxLayout();
}

void MapTab::RenderTab()
{


    this->SceneToRender->RenderScene();

    // Zone graphique pour la carte
    this->View->setScene(this->SceneToRender);
}

void MapTab::UnloadTab()
{
    if (this->SceneToRender != nullptr)
    {
        this->SceneToRender->UnloadScene();
        this->View->setScene(nullptr);
    }
}