#include <QApplication>
#include <QGraphicsEllipseItem>
#include <QPlainTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QGraphicsProxyWidget>
#include "UI/MapTab.h"
#include "UI/GameTab.h"

#pragma region ContextSwitchButton

ContextSwitchButton::ContextSwitchButton(MapTab* Owner, QWidget* parent) : QWidget(parent)
{
    this->Owner = Owner;
    setFixedSize(70, 30);

    // Création du fond
    this->Background = new QFrame(this);
    this->Background->setStyleSheet("background-color: #ccc; border-radius: 15px;");
    this->Background->setGeometry(0, 0, 70, 30);

    // Création du cercle mobile (le "point" du switch)
    this->Circle = new QLabel(this);
    this->Circle->setFixedSize(26, 26);
    this->Circle->setStyleSheet("background-color: white; border-radius: 13px;");
    this->Circle->move(2, 2);

    // Animation du déplacement
    this->MoveAnimation = new QPropertyAnimation(this->Circle, "pos");
    this->MoveAnimation->setDuration(200);
    this->MoveAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    // Animation du fond
    this->ColorAnimation = new QPropertyAnimation(this, "backgroundColor");
    this->ColorAnimation->setDuration(200);
    this->ColorAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    // Bouton invisible qui détecte les clics
    this->Button = new QPushButton(this);
    this->Button->setCheckable(true);
    this->Button->setStyleSheet("background: transparent;");
    this->Button->setGeometry(0, 0, 70, 30);

    // Connexion du bouton à l'animation
    connect(this->Button, &QPushButton::toggled, this, &ContextSwitchButton::AnimateSwitch);
}


ContextSwitchButton::~ContextSwitchButton()
{
    delete this->MoveAnimation;
    delete this->ColorAnimation;
    delete this->Button;
    delete this->Circle;
    delete this->Background;
}


bool ContextSwitchButton::GetContext()
{
    return this->Button->isChecked();
}


void ContextSwitchButton::UpdateContext(ObjectContext Context)
{
    switch (Context)
    {
        case ObjectContext::Spring:
        case ObjectContext::Adult:
        {
            this->Button->setChecked(true);
            this->AnimateSwitch(true);
            break;
        }

        case ObjectContext::All:
        {   // Here we don't want to change the context. However we still want to call the scene refresh in case the room has changed

            this->Owner->ContextSwitch(this->Button->isChecked());
            break;
        }

        case ObjectContext::Winter:
        case ObjectContext::Child:
        default:
        {
            this->Button->setChecked(false);
            this->AnimateSwitch(false);
            break;
        }
    }
}


void ContextSwitchButton::AnimateSwitch(bool Checked)
{
    // Circle moving animation
    this->MoveAnimation->setStartValue(this->Circle->pos());
    this->MoveAnimation->setEndValue(Checked ? QPoint(42, 2) : QPoint(2, 2));             // If false, goes to the left otherwise goes to the right
    this->MoveAnimation->start();

    // Background color animation
    this->ColorAnimation->setStartValue(this->Background->palette().color(QPalette::Window));
    this->ColorAnimation->setEndValue(Checked ? QColor("#0078D7") : QColor("#ccc"));      // If false, fade to grey otherwise fade to blue
    this->ColorAnimation->start();

    // Call the scene context switch function
    this->Owner->ContextSwitch(Checked);
}


void ContextSwitchButton::SetBackgroundColor(QColor Color)
{
    this->Background->setStyleSheet(QString("background-color: %1; border-radius: 15px;").arg(Color.name()));
}


QColor ContextSwitchButton::GetBackgroundColor() const
{
    return this->Background->palette().color(QPalette::Window);
}

#pragma endregion

#pragma region MapView

MapView::MapView(MapTab* Owner, QWidget* parent) : QGraphicsView(parent)
{
    this->Owner = Owner;
    this->setRenderHint(QPainter::Antialiasing);
    this->setRenderHint(QPainter::SmoothPixmapTransform);
    this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);  // Center zoom on mouse
    this->setDragMode(QGraphicsView::ScrollHandDrag);                // Allow mouse moving view
}


void MapView::UpdateContext(ObjectContext Context)
{
    this->Owner->UpdateContext(Context);
}


void MapView::wheelEvent(QWheelEvent* event)
{
    const double scaleFactor = 1.15;                        // Zoom speed
    if (event->angleDelta().y() > 0)
    {
        this->scale(scaleFactor, scaleFactor);              // Zoom in
    }
    else
    {
        this->scale(1.0 / scaleFactor, 1.0 / scaleFactor);  // Zoom out
    }
}

#pragma endregion

#pragma region MapTab

#pragma region Class creation / loading

// Création d'un onglet avec une carte et un panneau latéral
MapTab::MapTab(GameTab* Owner, int Game, SceneInfo* Scenes, size_t NumOfScenes, QWidget* parent) : QWidget(parent)
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
    this->SwitchButton = new ContextSwitchButton(this);
    this->SwitchLayout->addWidget(this->LeftIcon);
    this->SwitchLayout->addWidget(this->SwitchButton);
    this->SwitchLayout->addWidget(this->RightIcon);
    this->SwitchLayout->setContentsMargins(5, 5, 5, 5);
    this->SwitchLayout->setSpacing(10);
    this->SwitchContainer->setLayout(this->SwitchLayout);
    this->SwitchContainer->setStyleSheet("background-color: rgba(255, 255, 255, 0); border-radius: 10px;");
    this->SwitchContainer->setVisible(false);

    // Map Tree
    QLabel* mapLabel = new QLabel("Maps");
    this->MapTreeLayout->addWidget(mapLabel);
    this->MapSearchBar = new QLineEdit();
    this->MapSearchBar->setPlaceholderText("Find...");
    this->MapList = new QTreeWidget();
    this->MapList->setHeaderHidden(true);

    // Object Tree
    QLabel* objectLabel = new QLabel("Objects");
    this->ObjectTreeLayout->addWidget(objectLabel);
    this->ObjectSearchBar = new QLineEdit();
    this->ObjectSearchBar->setPlaceholderText("Find...");
    this->ObjectList = new QTreeWidget();
    this->ObjectList->setHeaderHidden(true);
    this->ObjectTreeLayout->addWidget(this->ObjectSearchBar);
    this->ObjectTreeLayout->addWidget(this->ObjectList);
    this->ObjectContainer->setHidden(true);

    // Rendering view
    this->View = new MapView(this);

    for (size_t i = 0; i < NumOfScenes; i++)
    {   // Creates all the scenes that match this map category

        uint8_t sceneRegionID = Scenes[i].Info->ParentRegion;
        if (sceneRegionID != 0)
        {   // We don't want to add scene that has no object

            RegionTree* currRegion = this->FindRegionTree(sceneRegionID);
            if (currRegion == nullptr)
            {   // Create a new region in the tree list

                currRegion = new RegionTree(Owner, Game, sceneRegionID, this->MapList);
                this->Regions.push_back(currRegion);
            }

            // Create a new scene item and add it to the region
            SceneItemTree* tmp = new SceneItemTree(&Scenes[i], currRegion);

            // Update the region object counts
            currRegion->AddObjectCounts(tmp->GetCollectedObjects(), tmp->GetTotalObjects());
            currRegion->RefreshObjsCountText();
            this->Scenes.insert(Scenes[i].SceneID, tmp);
        }
    }

    // Sort the final map list by alphabetic oder
    this->MapList->sortItems(0, Qt::AscendingOrder);
    this->MapTreeLayout->addWidget(this->MapSearchBar);
    this->MapTreeLayout->addWidget(this->MapList);
    this->MapTreeLayout->setStretch(0, 0);  // No stretch for label
    this->MapTreeLayout->setStretch(1, 0);  // No stretch for search bar
    this->MapTreeLayout->setStretch(2, 1);  // Tree list take all the remaining space


    ////// Add to main view /////

    // Put the switch button on the top left corner
    this->SwitchContainer->setParent(this->View);
    this->SwitchContainer->move(10, 10);

    // Set the maximum width for the lists
    this->MapContainer->setMaximumWidth(300);
    this->ObjectContainer->setMaximumWidth(300);

    // Add map -> view -> object to the splitter layout
    this->LayoutSplitter->addWidget(this->MapContainer);
    this->LayoutSplitter->addWidget(this->View);
    this->LayoutSplitter->addWidget(this->ObjectContainer);

    this->MainLayout->addWidget(this->LayoutSplitter);

    this->setLayout(this->MainLayout);

    // Connections
    QObject::connect(this->MapList, &QTreeWidget::currentItemChanged, this, &MapTab::ChangeActiveScene);
    QObject::connect(this->ObjectList, &QTreeWidget::itemClicked, this, &MapTab::ObjectClicked);
    QObject::connect(this->MapSearchBar, &QLineEdit::textChanged, [&](const QString& text) {
        this->FilterTree(this->MapList, text);
    });

    QObject::connect(this->ObjectSearchBar, &QLineEdit::textChanged, [&](const QString& text) {
        this->FilterTree(this->ObjectList, text);
    });

    QObject::connect(this->ObjectList, &QTreeWidget::itemSelectionChanged, this, &MapTab::UpdateObjectSelection);
}


MapTab::~MapTab()
{
    this->RenderedScene = nullptr;

    // Disconnect singals in order to not trigger event on partially destroyed object
    QObject::disconnect(this->MapList, nullptr, nullptr, nullptr);
    QObject::disconnect(this->ObjectList, nullptr, nullptr, nullptr);

    for (SceneItemTree* Scene : this->Scenes)
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


RegionTree* MapTab::FindRegionTree(uint8_t Region)
{
    for (size_t i = 0; i < this->Regions.size(); i++)
    {   // Browse through all the available regions

        if (this->Regions[i]->MetaInfo->Region == Region)
        {   // We have found the matching region

            return this->Regions[i];
        }
    }

    return nullptr;
}


void MapTab::RefreshScenesObjectCounts()
{
    for (SceneItemTree* currScene : this->Scenes)
    {   // Browse all scenes

        uint32_t tmpCount = currScene->GetCollectedObjects();

        if (currScene == this->RenderedScene)
        {   // Update the object list and the renderer

            this->UnloadMap();
            this->RenderedScene = currScene;
            this->RenderMap();
        }

        // Refresh the scene objects counters
        currScene->CountSceneObjects();
        currScene->RefreshObjectCounts(currScene->GetCollectedObjects() - tmpCount);
    }
}


void MapTab::FilterTree(QTreeWidget* TreeWidget, const QString& SearchText)
{
    for (int i = 0; i < TreeWidget->topLevelItemCount(); ++i)
    {   // Browse all tree item from the top

        QTreeWidgetItem* parentItem = TreeWidget->topLevelItem(i);
        bool parentVisible = false;

        for (int j = 0; j < parentItem->childCount(); ++j)
        {   // Browse all current item children

            QTreeWidgetItem* childItem = parentItem->child(j);

            if (childItem->childCount() > 0)
            {   // The current child has also some child

                QTreeWidgetItem* childItem2 = childItem->child(0);

                bool match = childItem2->text(0).contains(SearchText, Qt::CaseInsensitive);

                // Hide or unhide this part of the tree
                childItem->setHidden(!match);
                childItem2->setHidden(!match);
                parentVisible |= match;
            }
            else
            {   // The current child has no child

                bool match = childItem->text(0).contains(SearchText, Qt::CaseInsensitive);

                // Hide or unhide this part of the tree
                childItem->setHidden(!match);
                parentVisible |= match;
            }
        }

        bool matchParent = parentItem->text(0).contains(SearchText, Qt::CaseInsensitive);
        parentItem->setHidden(!parentVisible && !matchParent);
    }
}

#pragma endregion

#pragma region Context

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

#pragma endregion

#pragma region Map Tree / Scene Rendering

void MapTab::RenderMap()
{
    if (this->RenderedScene != nullptr)
    {   // There is a scene to render. Render it !

        // Unhide the object list
        this->ObjectContainer->setHidden(false);

        // Render the scene
        this->RenderedScene->RenderScene(this->ObjectList, this->SwitchButton->GetContext(), true);

        // Attach the scene to the view and adjust zoom and camera position
        this->View->setScene(this->RenderedScene->GetScene());
        this->View->fitInView(this->RenderedScene->GetScene()->sceneRect(), Qt::KeepAspectRatio);
        this->View->centerOn(this->RenderedScene->GetScene()->sceneRect().center());

        // Hide or unhide the context switch
        this->SwitchContainer->setVisible(this->RenderedScene->HasContext());
    }
}


void MapTab::UnloadMap()
{
    if (this->RenderedScene != nullptr)
    {   // A scene was rendered. Unload it !

        // We need to disconnect it in order to not call the perform function if an object was selected and destroyed
        QObject::disconnect(this->ObjectList, &QTreeWidget::itemSelectionChanged, this, &MapTab::UpdateObjectSelection);

        this->PrevSelected = nullptr;
        this->RenderedScene->UnloadScene();
        this->RenderedScene = nullptr;
        this->View->setScene(nullptr);
        this->ObjectList->clear();
        this->SwitchContainer->setVisible(false);

        // Don't forget to reconnect the signal
        QObject::connect(this->ObjectList, &QTreeWidget::itemSelectionChanged, this, &MapTab::UpdateObjectSelection);
    }
}


void MapTab::ChangeActiveScene(QTreeWidgetItem* Current, QTreeWidgetItem* Previous)
{
    if (Current->childCount() != 0)
    {   // The user has selected a region item tree

        return;
    }

    if (this->RenderedScene && this->RenderedScene->childCount() == 0)
    {   // The previous selected item was a scene

        this->UnloadMap();
        //this->RenderedScene->UnloadScene();
    }

    this->RenderedScene = (SceneItemTree*)Current;
    this->RenderMap();
}

#pragma endregion

#pragma region Object Tree / Object changed

void MapTab::ItemFound(ObjectInfo* Object, const ItemInfo* ItemFound)
{
    this->Scenes[Object->RenderScene]->ItemFound(Object, ItemFound);
}


void MapTab::ObjectClicked(QTreeWidgetItem* Item, int Column)
{
    // This function gets called only when an object is clicked on the list. We therefore need to update the scroll
    this->ObjectList->scrollTo(this->ObjectList->indexFromItem(Item), QAbstractItemView::PositionAtCenter);

    if (!this->SelectionUpdated)
    {   // This means that the itemSelectionChanged event has not occured before and the object can be updated

        ((CommonBaseItemTree*)Item)->PerformAction();
    }

    // We still need to reset the flag
    this->SelectionUpdated = false;
}


void MapTab::UpdateObjectSelection()
{
    QList<QTreeWidgetItem*> selectedItems = this->ObjectList->selectedItems();

    // We need to disconnect the list otherwise we will call this function multiple times
    QObject::disconnect(this->ObjectList, &QTreeWidget::itemSelectionChanged, this, &MapTab::UpdateObjectSelection);

    if (selectedItems.isEmpty())
    {   // No item is selected anymore. The scene has probably changed

        if (this->PrevSelected != nullptr)
        {   // Unselect the previously selected item

            this->PrevSelected->setSelected(false);
            this->PrevSelected->ResetObjectEffect();
            this->PrevSelected = nullptr;
        }

        // Reconnect the selection change event
        QObject::connect(this->ObjectList, &QTreeWidget::itemSelectionChanged, this, &MapTab::UpdateObjectSelection);
        return;
    }

    // Vérifier si l'élément sélectionné est un enfant
    QTreeWidgetItem* selectedItem = selectedItems.last();

    if (this->PrevSelected == selectedItem)
    {   // The previously selected is the same as the new one. This can only happen when the object was selected and is clicked from the graph item

        this->PrevSelected->ResetObjectEffect();
        this->PrevSelected = nullptr;
    }
    else if (this->PrevSelected != nullptr)
    {   // There was a previously selected item. We need to reset its effect

        this->PrevSelected->ResetObjectEffect();
    }

    this->PrevSelected = (CommonBaseItemTree*)selectedItem;

    // Multiple selection can occured when the selection event is trigger by clicking several graph items in a row
    for (QTreeWidgetItem* currItem : selectedItems)
    {   // Browse all the currently selected items

        for (int i = 0; i < currItem->childCount(); i++)
        {   // Update all children of this item

            QTreeWidgetItem* child = currItem->child(i);

            child->setSelected(false);
            ((CommonBaseItemTree*)child)->ResetObjectEffect();
        }

        if (currItem != selectedItem)
        {   // The current item is a different one. We need to unselect it

            currItem->setSelected(false);
            ((CommonBaseItemTree*)currItem)->ResetObjectEffect();
        }
        else
        {
            if (!this->PrevSelected->IsCalledFromGraph())
            {   // The selection event comes from a click in the list. We need to set up the flag in order to not double update the object

                this->SelectionUpdated = true;
            }
            else
            {   // The selection event comes from a click on a graph item. We can scroll to the object in the list without fear of consuming the click event

                this->ObjectList->scrollTo(this->ObjectList->indexFromItem(selectedItem), QAbstractItemView::PositionAtCenter); // Note : scrollTo seems to consume the click event and prevent the click function from being called after the selection event.
            }

            this->PrevSelected->SetCalledFromGraph(false);          // Reset the caller flag in order to proceed to rest of the object updating function
            ((CommonBaseItemTree*)selectedItem)->PerformAction();   // We can perform the object updating action
        }
    }

    // Reconnect the selection changed signal
    QObject::connect(this->ObjectList, &QTreeWidget::itemSelectionChanged, this, &MapTab::UpdateObjectSelection);
}

#pragma endregion

#pragma endregion