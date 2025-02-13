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

    // Object Tree
    this->ObjectSearchBar = new QLineEdit();
    this->ObjectSearchBar->setPlaceholderText("Find...");
    this->ObjectList = new QTreeWidget();
    this->ObjectList->setHeaderHidden(true);
    this->ObjectTreeLayout->addWidget(this->ObjectSearchBar);
    this->ObjectTreeLayout->addWidget(this->ObjectList);
    this->ObjectContainer->setHidden(true);

    // Zone graphique pour la carte
    this->View = new MapView();

    // Panneau latéral
    QLabel* mapLabel = new QLabel("Maps");
    this->MapTreeLayout->addWidget(mapLabel);

    for (size_t i = 0; i < NumOfScenes; i++)
    {   // Creates all the scenes that match this map category.

        //this->ScenesToRender.push_back(new SceneRenderer(&Scenes[i]));
        uint8_t sceneRegionID = Scenes[i].Info->ParentRegion;
        if (sceneRegionID != 0)
        {   // We don't want to add scene that has no object

            RegionTree* currRegion = this->FindRegionTree(sceneRegionID);//this->FindRegionTree(this->ScenesToRender[i]->GetSceneParentRegion());
            if (currRegion == nullptr)
            {   // Create a new region in the tree list

                currRegion = new RegionTree(Game, sceneRegionID, this->MapList);//new RegionTree(Game, this->ScenesToRender[i]->GetSceneParentRegion(), this->MapList);
                this->Regions.push_back(currRegion);
            }
            SceneItemTree * tmp = new SceneItemTree(&Scenes[i], currRegion);
            //currRegion->setExpanded(true);
            currRegion->AddObjectCounts(tmp->GetCollectedObjects(), tmp->GetTotalObjects());
            currRegion->RefreshObjsCountText();
            this->Scenes.insert(Scenes[i].SceneID, tmp);
        }
    }

    this->MapList->sortItems(0, Qt::AscendingOrder);
    this->MapTreeLayout->addWidget(this->MapSearchBar);
    this->MapTreeLayout->addWidget(this->MapList);
    this->MapTreeLayout->setStretch(0, 0);  // L'étiquette a un stretch de 0
    this->MapTreeLayout->setStretch(1, 0);  // La search bar a un stretch de 0
    this->MapTreeLayout->setStretch(2, 1);  // Le QTreeWidget prend tout l'espace restant


    // Ajouter à la mise en page principale
    this->MapContainer->setMaximumWidth(300);
    this->ObjectContainer->setMaximumWidth(300);
    this->LayoutSplitter->addWidget(this->MapContainer);
    this->LayoutSplitter->addWidget(this->View);
    this->LayoutSplitter->addWidget(this->ObjectContainer);

    this->MainLayout->addWidget(this->LayoutSplitter);

    this->setLayout(this->MainLayout);

    // Connections
    QObject::connect(this->MapList, &QTreeWidget::currentItemChanged, this, &MapTab::ChangeActiveScene);
    QObject::connect(this->ObjectList, &QTreeWidget::itemClicked, this, &MapTab::ChangeObjectState);
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
    this->RenderedScene = nullptr;
    delete this->MapList;
    delete this->ObjectList;
    delete this->View;
    delete this->ObjectTreeLayout;
    delete this->MapTreeLayout;
    delete this->MapContainer;
    delete this->ObjectContainer;
    delete this->MainLayout;
}

void MapTab::RenderMap()
{
    /*if (this->ActiveSceneID != -1)
    {   // Update the scene to render

        this->RenderedScene = this->ScenesToRender[this->ActiveSceneID];
    }*/

    if (this->RenderedScene != nullptr)
    {   // There is a scene to render. Render it !

        this->ObjectContainer->setHidden(false);
        this->RenderedScene->RenderScene(this->ObjectList);
        this->View->setScene(this->RenderedScene->Renderer);
    }
}

void MapTab::UnloadMap()
{
    if (this->RenderedScene != nullptr)
    {   // A scene was rendered. Unload it !

        this->RenderedScene->UnloadScene();
        this->RenderedScene = nullptr;
        this->View->setScene(nullptr);
        this->ObjectList->clear();
    }
}


void MapTab::ChangeActiveScene(QTreeWidgetItem * Current, QTreeWidgetItem * Previous)
{
    if (Current->childCount() != 0)
    {   // The user has selected a region item tree

        return;
    }

    if (this->RenderedScene && this->RenderedScene->childCount() == 0)
    {   // The previous selected item was a scene

        this->RenderedScene->UnloadScene();
    }

    this->RenderedScene = (SceneItemTree*)Current;
    this->RenderMap();
}


void MapTab::ChangeObjectState(QTreeWidgetItem* Item, int Column)
{
    ((CommonBaseItemTree*)Item)->PerformAction();
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


void MapTab::ItemFound(ObjectInfo* Object, const ItemInfo* ItemFound)
{
    this->Scenes[Object->RenderScene]->ItemFound(Object, ItemFound);
}

void MapTab::RefreshScenesObjectCounts()
{
    for (SceneItemTree * currScene : this->Scenes)
    {   // Browse all scenes

        uint32_t tmpCount = currScene->GetCollectedObjects();

        if (currScene == this->RenderedScene)
        {   // Update the object list and the renderer

            this->UnloadMap();
            this->RenderedScene = currScene;
            this->RenderMap();
        }

        currScene->CountSceneObjects();
        currScene->RefreshObjectCounts(currScene->GetCollectedObjects() - tmpCount);
    }
}