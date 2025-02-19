#include <QApplication>
#include <QGraphicsEllipseItem>
#include <QPlainTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QGraphicsProxyWidget>
#include "UI/MapTab.h"
#include "UI/GameTab.h"

void ToggleSwitch::animateSwitch(bool checked) {
    // Animation du déplacement du cercle
    moveAnimation->setStartValue(circle->pos());
    moveAnimation->setEndValue(checked ? QPoint(42, 2) : QPoint(2, 2));
    moveAnimation->start();

    // Animation de la couleur de fond
    colorAnimation->setStartValue(background->palette().color(QPalette::Window));
    colorAnimation->setEndValue(checked ? QColor("#0078D7") : QColor("#ccc"));
    colorAnimation->start();

    this->Owner->ContextSwitch(checked);
}

void MapView::UpdateContext(ObjectContext Context)
{
    this->Owner->UpdateContext(Context);
}


// Création d'un onglet avec une carte et un panneau latéral
MapTab::MapTab(GameTab * Owner, int Game, SceneInfo* Scenes, size_t NumOfScenes, QWidget* parent) : QWidget(parent)
{
    // Containers
    this->MapContainer = new QWidget();
    this->ObjectContainer = new QWidget();
    this->SwitchContainer = new QWidget();

    // Layouts
    this->LayoutSplitter = new QSplitter(Qt::Horizontal);
    this->MainLayout = new QHBoxLayout;
    this->SwitchLayout = new QHBoxLayout;
    this->MapTreeLayout = new QVBoxLayout(this->MapContainer);
    this->ObjectTreeLayout = new QVBoxLayout(this->ObjectContainer);

    // Switch context button
    this->LeftIcon = new QLabel();
    this->RightIcon = new QLabel();
    QPixmap pixmap;
    if (Game == OOT_GAME)
    {
        pixmap = pixmap.fromImage(QImage("./Resources/Common/ChildHead.png"));
        pixmap = pixmap.scaled(50, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        this->LeftIcon->setPixmap(pixmap);
        pixmap = pixmap.fromImage(QImage("./Resources/Common/AdultHead.png"));
        pixmap = pixmap.scaled(50, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        this->RightIcon->setPixmap(pixmap);
    }
    else
    {
        pixmap = pixmap.fromImage(QImage("./Resources/Common/Winter.png"));
        pixmap = pixmap.scaled(50, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        this->LeftIcon->setPixmap(pixmap);
        pixmap = pixmap.fromImage(QImage("./Resources/Common/Spring.png"));
        pixmap = pixmap.scaled(50, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        this->RightIcon->setPixmap(pixmap);
    }
    this->SwitchButton = new ToggleSwitch(this);
    this->SwitchLayout->addWidget(this->LeftIcon);
    this->SwitchLayout->addWidget(this->SwitchButton);
    this->SwitchLayout->addWidget(this->RightIcon);
    this->SwitchLayout->setContentsMargins(5, 5, 5, 5);
    this->SwitchLayout->setSpacing(10);
    this->SwitchContainer->setLayout(this->SwitchLayout);
    this->SwitchContainer->setStyleSheet("background-color: rgba(255, 255, 255, 0); border-radius: 10px;");
    this->SwitchContainer->setVisible(false);


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
    this->View = new MapView(this);

    // Panneau latéral
    QLabel* mapLabel = new QLabel("Maps");
    this->MapTreeLayout->addWidget(mapLabel);

    QLabel* objectLabel = new QLabel("Objects");
    this->ObjectTreeLayout->addWidget(objectLabel);

    for (size_t i = 0; i < NumOfScenes; i++)
    {   // Creates all the scenes that match this map category.

        //this->ScenesToRender.push_back(new SceneRenderer(&Scenes[i]));
        uint8_t sceneRegionID = Scenes[i].Info->ParentRegion;
        if (sceneRegionID != 0)
        {   // We don't want to add scene that has no object

            RegionTree* currRegion = this->FindRegionTree(sceneRegionID);//this->FindRegionTree(this->ScenesToRender[i]->GetSceneParentRegion());
            if (currRegion == nullptr)
            {   // Create a new region in the tree list

                currRegion = new RegionTree(Owner, Game, sceneRegionID, this->MapList);//new RegionTree(Game, this->ScenesToRender[i]->GetSceneParentRegion(), this->MapList);
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
    this->SwitchContainer->setParent(this->View);
    this->SwitchContainer->move(10, 10);
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

    QObject::connect(this->ObjectList, &QTreeWidget::itemSelectionChanged, this, &MapTab::ResetSelection);
}

MapTab::~MapTab()
{
    this->RenderedScene = nullptr;

    for (SceneItemTree * Scene : this->Scenes)
    {
        delete Scene;
    }

    this->Scenes.clear();
    delete this->MapList;
    delete this->ObjectList;
    delete this->SwitchButton;
    delete this->LeftIcon;
    delete this->RightIcon;
    delete this->SwitchLayout;
    delete this->ObjectTreeLayout;
    delete this->MapTreeLayout;
    delete this->SwitchContainer;
    delete this->MapContainer;
    delete this->ObjectContainer;
    delete this->View;
    delete this->MainLayout;
}

void MapTab::RenderMap()
{
    if (this->RenderedScene != nullptr)
    {   // There is a scene to render. Render it !

        this->ObjectContainer->setHidden(false);
        this->RenderedScene->RenderScene(this->ObjectList, this->SwitchButton->GetContext(), true);
        this->View->setScene(this->RenderedScene->GetScene());
        this->SwitchContainer->setVisible(this->RenderedScene->HasContext());
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
        this->SwitchContainer->setVisible(false);
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

        for (int j = 0; j < parentItem->childCount(); ++j)
        {
            QTreeWidgetItem* childItem = parentItem->child(j);

            if (childItem->childCount() > 0)
            {
                QTreeWidgetItem* childItem2 = childItem->child(0);

                bool match = childItem2->text(0).contains(SearchText, Qt::CaseInsensitive);
                childItem->setHidden(!match);
                childItem2->setHidden(!match);
                parentVisible |= match;  // Si un enfant est visible, le parent doit l'être aussi
            }
            else
            {   // The current child has no child

                bool match = childItem->text(0).contains(SearchText, Qt::CaseInsensitive);
                childItem->setHidden(!match);
                parentVisible |= match;  // Si un enfant est visible, le parent doit l'être aussi
            }
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

void MapTab::UpdateContext(ObjectContext Context)
{
    this->SwitchButton->UpdateContext(Context);
}


void MapTab::ContextSwitch(bool NewState)
{
    if (this->RenderedScene)
    {
        this->RenderedScene->RenderScene(this->ObjectList, NewState, false);
    }
}

void MapTab::ResetSelection()
{
    QList<QTreeWidgetItem*> selectedItems = this->ObjectList->selectedItems();

    if (selectedItems.isEmpty()) return;

    // Vérifier si l'élément sélectionné est un enfant
    QTreeWidgetItem* selectedItem = selectedItems.last();
    QTreeWidgetItem* parent = selectedItem->parent();

    if (parent) { // L'élément sélectionné est un enfant
        for (int i = 0; i < parent->childCount(); ++i) {
            QTreeWidgetItem* child = parent->child(i);
            if (child != selectedItem)
            {
                child->setSelected(false);
            }
            else
            {
                this->ObjectList->scrollTo(this->ObjectList->indexFromItem(selectedItem), QAbstractItemView::PositionAtCenter);
            }
        }
    }
}