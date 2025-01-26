#include <QApplication>
#include <QGraphicsEllipseItem>
#include <QPlainTextEdit>
#include <QLabel>
#include <QPushButton>
#include "UI/MapTab.h"


// Création d'un onglet avec une carte et un panneau latéral
MapTab::MapTab(SceneInfo* Scenes, size_t NumOfScenes, QWidget* parent) : QWidget(parent)
{
    // Layout principal
    this->MainLayout = new QHBoxLayout;

    //this->SceneToRender = new SceneRenderer(&OoTScenes[0]);
    //this->SceneToRender->RenderScene();

    // Zone graphique pour la carte
    //this->View = new QGraphicsView(this->SceneToRender);
    this->View = new QGraphicsView();

    // Panneau latéral
    QVBoxLayout* sidePanelLayout = new QVBoxLayout;
    QLabel* infoLabel = new QLabel("Informations sur les éléments");
    QPushButton* button = new QPushButton("Action");
    sidePanelLayout->addWidget(infoLabel);
    sidePanelLayout->addWidget(button);

    for (size_t i = 0; i < NumOfScenes; i++)
    {
        this->ScenesToRender.push_back(new SceneRenderer(&Scenes[i]));
        QPushButton* button = new QPushButton(this->ScenesToRender[i]->Name);
    }

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
    for (size_t i = 0; this->ScenesToRender.size(); i++)
    {
        this->ScenesToRender[i]->~SceneRenderer();
    }
    this->ScenesToRender.clear();
    this->RenderedScene = nullptr;
    this->View->~QGraphicsView();
    this->MainLayout->~QHBoxLayout();
}

void MapTab::RenderTab()
{


    //this->SceneToRender->RenderScene();

    // Zone graphique pour la carte
    //this->View->setScene(this->SceneToRender);
}

void MapTab::UnloadTab()
{
    if (this->RenderedScene != nullptr)
    {
        this->RenderedScene->UnloadScene();
        this->RenderedScene = nullptr;
        this->View->setScene(nullptr);
    }
}