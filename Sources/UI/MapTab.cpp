#include <QApplication>
#include <QGraphicsEllipseItem>
#include <QPainter>
#include <QPlainTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QHeaderView>
#include <QGraphicsProxyWidget>
#include <QStyledItemDelegate>
#include "UI/RoomRenderer.h"
#include "UI/MapTab.h"
#include "UI/GameTab.h"
#include "UI/FilterManager.h"
#include "UI/AppConfig.h"
#include "UI/ObjectRenderer.h"
#include "UI/SceneRenderer.h"

namespace {

/*
*   QTreeWidget subclass that paints a depth-based tint across the entire row,
*   covering the indent and branch (+/-) area as well as the columns. Subclassing
*   the view (rather than using a delegate) is required because a QStyledItemDelegate
*   only paints inside the item cell rect — it cannot reach the indent / branch area.
*
*   Depth 0 (regions / object categories): strong tint (alpha 64) + thin separator.
*   Depth 1 (scenes inside a region):       light tint  (alpha 16).
*   Deeper rows (rooms / object items):     no tint.
*/
class TintedTreeWidget : public QTreeWidget
{
public:
    QColor Accent;
    QColor SelectionBg;

    TintedTreeWidget(const QColor& AccentColor, const QColor& SelectionBgColor, QWidget* Parent = nullptr)
        : QTreeWidget(Parent), Accent(AccentColor), SelectionBg(SelectionBgColor)
    {
        // Sync QPalette::Highlight with the QSS selection color. Without this, the indent /
        // branch area (which the default delegate paints via the palette rather than via QSS)
        // falls back to the global highlight color (OoT blue) on the MM tab.
        QPalette pal = this->palette();
        pal.setColor(QPalette::Highlight, SelectionBgColor);
        pal.setColor(QPalette::Inactive, QPalette::Highlight, SelectionBgColor);
        this->setPalette(pal);
    }

protected:
    void drawRow(QPainter* Painter, const QStyleOptionViewItem& Options, const QModelIndex& Index) const override
    {
        int depth = 0;
        QModelIndex p = Index.parent();
        while (p.isValid()) { depth++; p = p.parent(); }

        // Depth 0 (regions / object categories): strong tint.
        // Depth >= 1 (scenes, rooms, individual objects): light tint.
        int alpha = (depth == 0) ? 64 : 16;
        bool selected = (Options.state & QStyle::State_Selected) != 0;

        QRect rowRect(0, Options.rect.top(), viewport()->width(), Options.rect.height());

        if (selected)
        {
            // Paint the selection bg across the entire row (incl. the indent area),
            // so the default delegate cannot leave a strip of palette-blue on the left.
            Painter->fillRect(rowRect, SelectionBg);
        }
        else
        {
            QColor bg = Accent;
            bg.setAlpha(alpha);
            Painter->fillRect(rowRect, bg);
        }

        QTreeWidget::drawRow(Painter, Options, Index);

        if (depth == 0)
        {
            Painter->save();
            Painter->setPen(QPen(QColor(255, 255, 255, 32), 1));
            int y = Options.rect.bottom();
            Painter->drawLine(0, y, viewport()->width(), y);
            Painter->restore();
        }
    }
};

} // namespace

#pragma region ContextSwitchButton

ContextSwitchButton::ContextSwitchButton(MapTab* Owner, QWidget* parent) : QWidget(parent)
{
    int width = 70;
    int height = 30;

    this->Owner = Owner;
    this->setFixedSize(width, height);

    // Background creation
    this->Background = new QFrame(this);
    this->Background->setFrameShape(QFrame::NoFrame);
    this->Background->setStyleSheet("background-color: #ccc; border: none; border-radius: 15px;");
    this->Background->setGeometry(0, 0, width, height);

    // Moving circle creation
    this->Circle = new QLabel(this);
    this->Circle->setFixedSize(26, 26);
    this->Circle->setStyleSheet("background-color: white; border: none; border-radius: 13px;");
    this->Circle->move(2, 2);

    // Moving animation
    this->MoveAnimation = new QPropertyAnimation(this->Circle, "pos");
    this->MoveAnimation->setDuration(200);
    this->MoveAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    // Background animation
    this->ColorAnimation = new QPropertyAnimation(this, "backgroundColor");
    this->ColorAnimation->setDuration(200);
    this->ColorAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    // Invisible button used to detects click events
    this->Button = new QPushButton(this);
    this->Button->setCheckable(true);
    this->Button->setFlat(true);
    this->Button->setFocusPolicy(Qt::NoFocus);
    this->Button->setStyleSheet("QPushButton { background: transparent; border: none; outline: none; } QPushButton:focus { border: none; outline: none; }");
    this->Button->setGeometry(0, 0, width, height);

    // Connect the button to the animation
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
    this->Background->setStyleSheet(QString("background-color: %1; border: none; border-radius: 15px;").arg(Color.name()));
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
    this->MapContainer->setObjectName("MapContainer");
    this->ObjectContainer = new QWidget();
    this->ObjectContainer->setObjectName("ObjectContainer");
    this->SwitchContainer = new QWidget();

    // Game accent color used by the tinted tree rows below (OoT blue / MM violet).
    QString accent = GameTab::GetAccentColorFor(Game);

    // Long accent stripe along the left edge of the map panel.
    this->MapContainer->setStyleSheet(QString("#MapContainer { border-left: 2px solid %1; }").arg(accent));

    // Game-specific hover / selected colors so the MM tab uses violet shades
    // instead of the global OoT blues coming from the base stylesheet.
    // Note: the MM shades are tuned so they read as violet on a dark theme
    // (the previous #3a1560 / #1e0a38 had a dominant blue channel and felt blue-ish).
    QString hoverBg    = (Game == OOT_GAME) ? "#0d2a4a" : "#2a1248";
    QString selectedBg = (Game == OOT_GAME) ? "#1a4a7a" : "#5a2580";
    // selection-background-color overrides the QPalette::Highlight inherited from the
    // global stylesheet (which is OoT-blue). Without this, the indent / branch area
    // — painted via the palette rather than via ::item rules — keeps the OoT-blue
    // selection color in the MM tab.
    QString treeHoverSelectQss = QString(
        "QTreeWidget, QTreeView { selection-background-color: %2; selection-color: #ddeeff; } "
        "QTreeWidget::item:hover:!selected, QTreeView::item:hover:!selected { background-color: %1; } "
        "QTreeWidget::item:selected, QTreeView::item:selected { background-color: %2; color: #ddeeff; } "
    ).arg(hoverBg, selectedBg);

    // Layouts
    this->LayoutSplitter = new QSplitter(Qt::Horizontal);
    this->MainLayout = new QHBoxLayout;
    this->SwitchLayout = new QHBoxLayout;
    this->ObjectBarLayout = new QHBoxLayout;
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
    this->MapList = new TintedTreeWidget(QColor(accent), QColor(selectedBg));
    this->MapList->setHeaderHidden(true);
    this->MapList->setColumnCount(2);
    this->MapList->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    this->MapList->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    this->MapList->header()->setStretchLastSection(false);
    this->MapList->setStyleSheet(treeHoverSelectQss);
    this->MapTreeToggleButton = new QPushButton("Expand All", this);
    this->MapTreeToggleButton->setCheckable(true);

    // Filter Button
    this->FilterButton = new FilterManager(Owner);

    // Object Tree — scene header (name + counter + progress bar)
    this->SceneHeaderWidget = new QWidget();
    QVBoxLayout* sceneHeaderLayout = new QVBoxLayout(this->SceneHeaderWidget);
    sceneHeaderLayout->setContentsMargins(2, 2, 2, 4);
    sceneHeaderLayout->setSpacing(4);

    QHBoxLayout* sceneHeaderRow = new QHBoxLayout();
    sceneHeaderRow->setContentsMargins(0, 0, 0, 0);
    sceneHeaderRow->setSpacing(6);
    this->SceneNameLabel = new QLabel("Objects");
    this->SceneNameLabel->setStyleSheet("font-weight: 600; font-size: 13px; color: palette(bright-text);");
    this->SceneCountLabel = new QLabel("0/0");
    this->SceneCountLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    this->SceneCountLabel->setStyleSheet("font-size: 11px; color: #7a9abf;");
    sceneHeaderRow->addWidget(this->SceneNameLabel, 1);
    sceneHeaderRow->addWidget(this->SceneCountLabel);

    this->SceneProgress = new QProgressBar();
    this->SceneProgress->setRange(0, 100);
    this->SceneProgress->setValue(0);
    this->SceneProgress->setTextVisible(false);

    sceneHeaderLayout->addLayout(sceneHeaderRow);
    sceneHeaderLayout->addWidget(this->SceneProgress);

    this->ObjectTreeLayout->addWidget(this->SceneHeaderWidget);

    this->ObjectSearchBar = new QLineEdit();
    this->ObjectSearchBar->setPlaceholderText("Find...");
    this->ObjectBarLayout->addWidget(this->ObjectSearchBar);
    this->ObjectBarLayout->addWidget(this->FilterButton);

    this->ObjectList = new TintedTreeWidget(QColor(accent), QColor(selectedBg));
    this->ObjectList->setHeaderHidden(true);
    this->ObjectList->setColumnCount(2);
    this->ObjectList->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    this->ObjectList->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    this->ObjectList->header()->setStretchLastSection(false);
    this->ObjectList->setStyleSheet(treeHoverSelectQss);
    // Two-line rendering for object rows (icon + object name + item / "???").
    this->ObjectList->setItemDelegate(new ObjectItemDelegate(this->ObjectList));
    this->ObjectTreeToggleButton = new QPushButton("Collapse All", this);
    this->ObjectTreeLayout->addLayout(this->ObjectBarLayout);
    this->ObjectTreeLayout->addWidget(this->ObjectTreeToggleButton);
    this->ObjectTreeLayout->addWidget(this->ObjectList);
    this->ObjectContainer->setHidden(true);
    this->ObjectTreeToggleButton->setCheckable(true);

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

                currRegion = new RegionTree(Owner, sceneRegionID, this->MapList);
                this->Regions.push_back(currRegion);
            }

            // Create a new scene item and add it to the region
            SceneItemTree* tmp = new SceneItemTree(&Scenes[i], this->FilterButton, currRegion);

            if (tmp->GetTotalObjects() == 0)
            {   // Scene has no item to show. Hide it

                currRegion->child(currRegion->indexOfChild(tmp))->setHidden(true);
            }
            else
            {   // Update the region object counts
            
                currRegion->AddObjectCounts(tmp->GetCollectedObjects(), tmp->GetTotalObjects());
                currRegion->RefreshObjsCountText();
            }

            this->Scenes.insert(Scenes[i].SceneID, tmp);
        }
    }

    // Sort the final map list by alphabetic oder
    this->MapList->sortItems(0, Qt::AscendingOrder);
    this->MapTreeLayout->addWidget(this->MapSearchBar);
    this->MapTreeLayout->addWidget(this->MapTreeToggleButton);
    this->MapTreeLayout->addWidget(this->MapList);
    this->MapTreeLayout->setStretch(0, 0);  // No stretch for label
    this->MapTreeLayout->setStretch(1, 0);  // No stretch for search bar
    this->MapTreeLayout->setStretch(2, 1);  // Tree list take all the remaining space


    ////// Add to main view /////

    // Put the switch button on the top left corner
    this->SwitchContainer->setParent(this->View);
    this->SwitchContainer->move(10, 10);

    // Set the maximum width for the lists
    this->MapContainer->setMaximumWidth(320);
    this->ObjectContainer->setMaximumWidth(360);

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
    QObject::connect(this->FilterButton, &FilterManager::filterChanged, this, &MapTab::UpdateObjectVisibility);

    QObject::connect(this->MapTreeToggleButton, &QPushButton::clicked, this, [&]()
    {
        this->IsMapExpanded = !this->IsMapExpanded;
        this->MapTreeToggleButton->setText(this->IsMapExpanded ? "Collapse All" : "Expand All");
        this->OnToggleExpandCollapse(this->MapList, this->IsMapExpanded);
    });

    QObject::connect(this->ObjectTreeToggleButton, &QPushButton::clicked, this, [&]()
    {
        this->IsObjectExpanded = !this->IsObjectExpanded;
        this->ObjectTreeToggleButton->setText(this->IsObjectExpanded ? "Collapse All" : "Expand All");
        this->OnToggleExpandCollapse(this->ObjectList, this->IsObjectExpanded);
    });

}


MapTab::~MapTab()
{
    this->RenderedScene = nullptr;

    // Disconnect signals in order to not trigger event on partially destroyed object
    QObject::disconnect(this->MapList, nullptr, nullptr, nullptr);
    QObject::disconnect(this->ObjectList, nullptr, nullptr, nullptr);

    for (SceneItemTree* Scene : this->Scenes)
    {
        delete Scene;
    }

    this->Scenes.clear();
    delete this->MapList;
    delete this->MapTreeToggleButton;
    delete this->ObjectList;
    delete this->ObjectTreeToggleButton;
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
    return FindRegionTreeIn(this->Regions, Region);
}


void MapTab::RefreshScenesObjectCounts()
{
    for (SceneItemTree* currScene : this->Scenes)
    {   // Browse all scenes

        uint32_t tmpCount = currScene->GetCollectedObjects();

        if (currScene->Rooms.size() > 0)
        {   // Refresh count by using room

            currScene->FoundObjects = 0;

            for (size_t i = 0; i < currScene->Rooms.size(); i++)
            {
                currScene->FoundObjects+= currScene->Rooms[i]->RefreshRoomObjectsCount();
            }
        }
        else
        {   // Use the global scene count


            if (currScene == this->RenderedScene)
            {   // Update the object list and the renderer

                this->UnloadMap();
                this->RenderedScene = currScene;
                this->RenderMap();
            }

            // Refresh the scene objects counters
            currScene->CountSceneObjects();

        }

        currScene->UpdateObjectCounts(currScene->GetCollectedObjects() - tmpCount);
    }
}


void MapTab::ResetAllObjectCounts()
{
    // Reset game tab counters
    this->FilterButton->TabOwner->FoundObjects = 0;
    this->FilterButton->TabOwner->TotalObjects = 0;

    for (size_t i = 0; i < this->Regions.size(); i++)
    {   // Reset all region counters

        this->Regions[i]->ResetRegion();
    }

    for (SceneItemTree * currScene : this->Scenes.values())
    {   // Creates all the scenes that match this map category

        uint8_t sceneRegionID = currScene->Scene->Info->ParentRegion;
        if (sceneRegionID != 0)
        {   // We don't want to add scene that has no object

            RegionTree* currRegion = this->FindRegionTree(sceneRegionID);

            // Reset scene object counters
            currScene->CountSceneObjects();
            currScene->RefreshItemName();

            if (currScene->Rooms.size() > 0)
            {   // Refresh count by using room

                for (size_t i = 0; i < currScene->Rooms.size(); i++)
                {
                    currScene->Rooms[i]->InitRoomCounters();
                    if (currScene->Rooms[i]->GetTotalObjects() > 0)
                    {   // The room has items

                        currScene->child(currScene->indexOfChild(currScene->Rooms[i]))->setHidden(false);
                        currScene->Rooms[i]->RefreshItemName();
                    }
                    else
                    {   // The room has no item, we need to hide it

                        currScene->child(currScene->indexOfChild(currScene->Rooms[i]))->setHidden(true);
                    }
                }
            }

            // Resolve the active scene root: when a room is currently rendered, its
            // SceneItem is the parent scene we have to compare against currScene.
            RoomItemTree* renderedRoom = dynamic_cast<RoomItemTree*>(this->RenderedScene);
            SceneItemTree* renderedRoot = renderedRoom != nullptr ? renderedRoom->SceneItem : this->RenderedScene;

            if (currScene->GetTotalObjects() == 0)
            {   // Scene has no item to show. Hide it

                currRegion->child(currRegion->indexOfChild(currScene))->setHidden(true);

                if (currScene == renderedRoot)
                {   // Update the object list and the renderer

                    this->UnloadMap();
                }
            }
            else
            {   // Update the region object counts

                currRegion->child(currRegion->indexOfChild(currScene))->setHidden(false);   // The scene could previously be hidden depending on the settings
                currRegion->AddObjectCounts(currScene->GetCollectedObjects(), currScene->GetTotalObjects());
                currRegion->RefreshObjsCountText();

                if (currScene == renderedRoot)
                {   // Update the object list and the renderer. When a room was active,
                    // keep the user on the same room — unless it just got emptied, in
                    // which case fall back to the parent scene view.

                    this->UnloadMap();
                    if (renderedRoom != nullptr && renderedRoom->GetTotalObjects() > 0)
                    {
                        this->RenderedScene = renderedRoom;
                    }
                    else
                    {
                        this->RenderedScene = currScene;
                    }
                    this->RenderMap();
                }
            }
        }
    }
}


void MapTab::FilterTree(QTreeWidget* TreeWidget, const QString& SearchText)
{
    FilterQTreeWidget(TreeWidget, SearchText);

    if (TreeWidget == this->ObjectList && SearchText.isEmpty())
    {
        this->UpdateObjectVisibility();
    }
}


void MapTab::OnToggleExpandCollapse(QTreeWidget* TreeWidget, bool Expand)
{
    OnToggleExpandCollapseQTreeWidget(TreeWidget, Expand);
}

#pragma endregion

#pragma region Context

void MapTab::UpdateContext(ObjectContext Context)
{
    this->SwitchButton->UpdateContext(Context);

    this->ObjectTreeToggleButton->setText("Collapse All");
    this->IsObjectExpanded = true;
}


void MapTab::ContextSwitch(bool NewState)
{
    if (this->RenderedScene)
    {
        this->RenderedScene->RenderScene(this->ObjectList, NewState, false);
    }

    this->ObjectTreeToggleButton->setText("Collapse All");
    this->IsObjectExpanded = true;
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

        // Refresh the scene header
        this->RefreshSceneHeader();
    }
}


void MapTab::RefreshSceneHeader()
{
    if (this->SceneNameLabel == nullptr)
    {
        return;
    }

    if (this->RenderedScene == nullptr)
    {
        this->SceneNameLabel->setText("Objects");
        this->SceneCountLabel->setText("0/0");
        this->SceneProgress->setValue(0);
        return;
    }

    int found = this->RenderedScene->GetCollectedObjects();
    int total = this->RenderedScene->GetTotalObjects();
    int pct = total > 0 ? (100 * found) / total : 0;

    this->SceneNameLabel->setText(this->RenderedScene->GetSceneName());
    this->SceneCountLabel->setText(QString("%1/%2").arg(found).arg(total));
    this->SceneProgress->setValue(pct);
}


void MapTab::UnloadMap()
{
    if (this->RenderedScene != nullptr)
    {   // A scene was rendered. Unload it !

        // We need to disconnect it in order to not call the perform function if an object was selected and destroyed
        QObject::disconnect(this->ObjectList, &QTreeWidget::itemSelectionChanged, this, &MapTab::UpdateObjectSelection);

        /*int numOfObjs = this->ObjectList->topLevelItemCount();
        for (int i = 0; i < numOfObjs; i++)
        {
            this->ObjectList->takeTopLevelItem(0);
        }
        this->ObjectList->clear();
        numOfObjs = this->ObjectList->topLevelItemCount();*/
        this->PrevSelected = nullptr;
        this->RenderedScene->UnloadScene();
        this->RenderedScene = nullptr;
        this->View->setScene(nullptr);
        this->SwitchContainer->setVisible(false);
        this->ObjectTreeToggleButton->setText("Collapse All");
        this->IsObjectExpanded = true;

        // Reset the scene header
        this->RefreshSceneHeader();

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

    if (this->RenderedScene)
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
    if (this->Scenes[Object->RenderScene]->Renderer == nullptr)
    {   // Force the scene to load

        if (this->RenderedScene)
        {   // The previous selected item was a scene

            this->UnloadMap();
            //this->RenderedScene->UnloadScene();
        }

        this->RenderedScene = this->Scenes[Object->RenderScene];
        if (this->RenderedScene->childCount() != 0)
        {
            this->RenderedScene->ActiveRoom = &this->RenderedScene->Rooms[Object->RoomID]->Info;
        }
        this->RenderMap();
    }
    this->Scenes[Object->RenderScene]->ItemFound(Object, ItemFound);

    // Keep the scene header in sync with the active rendered scene
    if (this->RenderedScene != nullptr && Object->RenderScene == this->RenderedScene->Scene->SceneID)
    {
        this->RefreshSceneHeader();
    }
}


void MapTab::ObjectClicked(QTreeWidgetItem* Item, int Column)
{
    // This function gets called only when an object is clicked on the list. We therefore need to update the scroll
    //QModelIndex index = this->ObjectList->indexAt(this->ObjectList->visualItemRect(Item).center());
    //this->ObjectList->scrollTo(index, QAbstractItemView::PositionAtCenter);
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

                //QModelIndex index = this->ObjectList->indexAt(this->ObjectList->visualItemRect(selectedItem).center());
                //this->ObjectList->scrollTo(index, QAbstractItemView::PositionAtCenter);
                this->ObjectList->scrollTo(this->ObjectList->indexFromItem(selectedItem), QAbstractItemView::PositionAtCenter); // Note : scrollTo seems to consume the click event and prevent the click function from being called after the selection event.
            }

            this->PrevSelected->SetCalledFromGraph(false);          // Reset the caller flag in order to proceed to rest of the object updating function
            ((CommonBaseItemTree*)selectedItem)->PerformAction();   // We can perform the object updating action
        }
    }

    // Reconnect the selection changed signal
    QObject::connect(this->ObjectList, &QTreeWidget::itemSelectionChanged, this, &MapTab::UpdateObjectSelection);
}


void MapTab::UpdateObjectVisibility()
{
    if (this->RenderedScene != nullptr)
    {
        this->RenderedScene->GetScene()->UpdateSceneObjectVisibility();
    }
}


void MapTab::FocusObject(ObjectInfo* Object)
{
    if (Object == nullptr) return;

    auto it = this->Scenes.find(Object->RenderScene);
    if (it == this->Scenes.end()) return;

    SceneItemTree* sceneItem = it.value();
    if (sceneItem == nullptr) return;

    // The map list is filtered by the layout / filter settings. The target scene
    // (and its containing region) may currently be hidden — unhide them so the
    // user can actually see the row that just got selected.
    if (QTreeWidgetItem* region = sceneItem->parent())
    {
        region->setHidden(false);
        region->setExpanded(true);
    }
    sceneItem->setHidden(false);

    // For multi-room scenes, the renderer is keyed off the active room: pick
    // the room that owns the object so RenderMap loads the right minimap.
    QTreeWidgetItem* targetItem = sceneItem;
    if (!sceneItem->Rooms.empty())
    {
        sceneItem->setExpanded(true);
        for (RoomItemTree* room : sceneItem->Rooms)
        {
            if (room != nullptr && room->Info.RoomID == Object->RoomID)
            {
                targetItem = room;
                break;
            }
        }
    }

    // setCurrentItem fires currentItemChanged which routes through ChangeActiveScene
    // -> RenderMap. When the same item is already current, the signal does not fire,
    // so we call ChangeActiveScene explicitly if no scene is rendered yet.
    this->MapList->setCurrentItem(targetItem);
    if (this->RenderedScene == nullptr)
    {
        this->ChangeActiveScene(targetItem, nullptr);
    }
    this->MapList->scrollToItem(targetItem);

    if (this->RenderedScene == nullptr || this->RenderedScene->Renderer == nullptr) return;

    // Locate the ObjectItemTree associated with the object. Direct pointer
    // identity is the cheapest hit, but cross-scene navigation (Progression
    // dashboard, paired entries living in both the home and the render
    // scene's static arrays) can hand us an ObjectInfo* that the renderer
    // never instantiated a leaf for — fall back to logical identity in that
    // case so the right leaf is still picked.
    ObjectItemTree* match = nullptr;
    for (size_t i = 0; i < ObjectType::last - 1 && match == nullptr; ++i)
    {
        ObjectRenderer* rdr = this->RenderedScene->Renderer->ObjectsRen[i];
        if (rdr == nullptr) continue;

        for (ObjectItemTree* leaf : rdr->Objects)
        {
            if (leaf == nullptr || leaf->Object == nullptr) continue;
            if (leaf->Object == Object
                || (leaf->Object->ObjectID    == Object->ObjectID
                 && leaf->Object->Type        == Object->Type
                 && leaf->Object->RenderScene == Object->RenderScene))
            {
                match = leaf;
                break;
            }
        }
    }

    if (match == nullptr) return;

    // Reveal the row inside its category, select it (so MapTab's existing
    // selection-driven highlighting kicks in) and scroll it into view.
    if (QTreeWidgetItem* cat = match->parent())
    {
        cat->setExpanded(true);
    }
    this->ObjectList->setCurrentItem(match);
    match->setSelected(true);
    this->ObjectList->scrollToItem(match, QAbstractItemView::PositionAtCenter);

    // Zoom the viewport closer than the default fit-in-view so the target
    // actually pops out of the surrounding map. The user explicitly asked
    // to be taken to it, so we ignore the AutoSnap setting that gates
    // SceneRenderer::CenterViewOn.
    if (match->GraphItem != nullptr && this->View != nullptr)
    {
        QRectF iconRect = match->GraphItem->mapToScene(match->GraphItem->boundingRect()).boundingRect();
        const qreal pad = qMax(iconRect.width(), iconRect.height()) * 6.0;
        QRectF target = iconRect.adjusted(-pad, -pad, pad, pad);

        QRectF sceneBounds = this->View->scene() != nullptr
            ? this->View->scene()->sceneRect()
            : target;
        if (!sceneBounds.isNull())
        {   // Clamp the target rect inside the scene so fitInView keeps the
            // zoom centered on the object even when it sits near a map edge.
            target = target.intersected(sceneBounds);
            if (target.isEmpty()) target = iconRect;
        }

        this->View->fitInView(target, Qt::KeepAspectRatio);
    }
}

#pragma endregion

#pragma endregion
