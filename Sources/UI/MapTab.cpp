#include <QApplication>
#include <QGraphicsEllipseItem>
#include <QPlainTextEdit>
#include <QLabel>
#include <QPushButton>
#include "UI/MapTab.h"


// Création d'un onglet avec une carte et un panneau latéral
MapTab::MapTab(int Game, SceneInfo* Scenes, size_t NumOfScenes, QWidget* parent) : QWidget(parent)
{
    // Containers
    this->MapContainer = new QWidget();
    this->ObjectContainer = new QWidget();

    // Layouts
    this->LayoutSplitter = new QSplitter(Qt::Horizontal);
    this->MainLayout = new QHBoxLayout;
    this->MapTreeLayout = new QVBoxLayout(this->MapContainer);
    this->ObjectTreeLayout = new QVBoxLayout(this->ObjectContainer);

    // Map Tree
    this->MapSearchBar = new QLineEdit();
    this->MapSearchBar->setPlaceholderText("Find...");
    this->MapList = new QTreeWidget();
    this->MapList->setHeaderHidden(true);

    // Zone graphique pour la carte
    this->View = new QGraphicsView();

    // Panneau latéral
    QLabel* mapLabel = new QLabel("Maps");
    this->MapTreeLayout->addWidget(mapLabel);

    for (size_t i = 0; i < NumOfScenes; i++)
    {   // Creates all the scenes that match this map category.

        this->ScenesToRender.push_back(new SceneRenderer(&Scenes[i]));
        RegionTree * currRegion = this->FindRegionTree(this->ScenesToRender[i]->GetSceneParentRegion());
        if (currRegion == nullptr)
        {   // Create a new region in the tree list

            currRegion = new RegionTree(Game, this->ScenesToRender[i]->GetSceneParentRegion(), this->MapList);
            this->Regions.push_back(currRegion);
        }
        QTreeWidgetItem * tmp = new QTreeWidgetItem(currRegion);
        tmp->setText(0, this->ScenesToRender[i]->GetSceneName());
        //this->MapList->addItem(this->ScenesToRender[i]->GetSceneName());
    }

    this->MapList->sortItems(0, Qt::AscendingOrder);
    this->MapTreeLayout->addWidget(this->MapSearchBar);
    this->MapTreeLayout->addWidget(this->MapList);
    this->MapTreeLayout->setStretch(0, 0);  // L'étiquette a un stretch de 0
    this->MapTreeLayout->setStretch(1, 0);  // La search bar a un stretch de 0
    this->MapTreeLayout->setStretch(2, 1);  // Le QTreeWidget prend tout l'espace restant

    // Object Tree
    this->ObjectSearchBar = new QLineEdit();
    this->ObjectSearchBar->setPlaceholderText("Find...");
    this->ObjectList = new QTreeWidget();
    this->ObjectList->setHeaderHidden(true);
    this->ObjectTreeLayout->addWidget(this->ObjectSearchBar);
    this->ObjectTreeLayout->addWidget(this->ObjectList);

    // Ajouter à la mise en page principale
    this->MapContainer->setMaximumWidth(300);
    this->LayoutSplitter->addWidget(this->MapContainer);
    this->LayoutSplitter->addWidget(this->View);
    this->LayoutSplitter->addWidget(this->ObjectContainer);

    this->MainLayout->addWidget(this->LayoutSplitter);

    this->setLayout(this->MainLayout);

    // Connections
    //QObject::connect(this->MapList, &QComboBox::currentIndexChanged, this, &MapTab::ChangeActiveScene);
    QObject::connect(this->MapSearchBar, &QLineEdit::textChanged, [&](const QString& text) {
        this->FilterTree(this->MapList, text);
    });

    QObject::connect(this->ObjectSearchBar, &QLineEdit::textChanged, [&](const QString& text) {
        this->FilterTree(this->ObjectList, text);
    });
}

MapTab::~MapTab()
{
    for (size_t i = 0; i < this->ScenesToRender.size(); i++)
    {
        this->ScenesToRender[i]->~SceneRenderer();
    }
    this->ScenesToRender.clear();
    this->RenderedScene = nullptr;
    //this->MapList->~QComboBox();
    this->MapList->~QTreeWidget();
    this->MapTreeLayout->~QVBoxLayout();
    this->View->~QGraphicsView();
    this->MainLayout->~QHBoxLayout();
}

void MapTab::RenderMap()
{
    if (this->ActiveSceneID != -1)
    {   // Update the scene to render

        this->RenderedScene = this->ScenesToRender[this->ActiveSceneID];
    }

    if (this->RenderedScene != nullptr)
    {   // There is a scene to render. Render it !

        this->RenderedScene->RenderScene();
        this->View->setScene(this->RenderedScene);
    }
}

void MapTab::UnloadMap()
{
    if (this->RenderedScene != nullptr)
    {   // A scene was rendered. Unload it !

        this->RenderedScene->UnloadScene();
        this->RenderedScene = nullptr;
        this->View->setScene(nullptr);
    }
}


void MapTab::ChangeActiveScene(int NewIndex)
{
    if (NewIndex < this->ScenesToRender.size())
    {   // Check if the given index correspond to an existing scene

        if (this->ActiveSceneID != -1)
        {   // Unload the previous rendered map

            this->UnloadMap();
        }

        this->ActiveSceneID = NewIndex;
    }
    
    this->RenderMap();
}


RegionTree* MapTab::FindRegionTree(uint8_t Region)
{
    for (size_t i = 0; i < this->Regions.size(); i++)
    {   // Browse through all the available regions

        if (this->Regions[i]->MetaInfo->Region == Region)
        {
            return this->Regions[i];
        }
    }

    return nullptr;
}


void MapTab::FilterTree(QTreeWidget* TreeWidget, const QString& SearchText)
{
    for (int i = 0; i < TreeWidget->topLevelItemCount(); ++i)
    {
        QTreeWidgetItem* parentItem = TreeWidget->topLevelItem(i);
        bool parentVisible = false;

        for (int j = 0; j < parentItem->childCount(); ++j) {
            QTreeWidgetItem* childItem = parentItem->child(j);
            bool match = childItem->text(0).contains(SearchText, Qt::CaseInsensitive);
            childItem->setHidden(!match);
            parentVisible |= match;  // Si un enfant est visible, le parent doit l'être aussi
        }

        bool matchParent = parentItem->text(0).contains(SearchText, Qt::CaseInsensitive);
        parentItem->setHidden(!parentVisible && !matchParent);
    }
}