#include "UI/EntranceTable.h"
#include "UI/EntranceRenderer.h"
#include "Combo/Entrances.h"
#include "Combo/Scenes.h"
#include "UI/ObjectRenderer.h"

#include <QVBoxLayout>
#include <QHeaderView>
#include <QPainter>
#include <QPointer>
#include <QTimer>
#include <QWheelEvent>


#include <algorithm>

#pragma region // GlobalEntranceTableModel

// ============================================
// Constructor
// ============================================

GlobalEntranceTableModel::GlobalEntranceTableModel(EntranceGameTabView* parent) : QAbstractTableModel(parent)
{
    this->Owner = parent;
    this->setScenes(*GetSceneEntranceMetaInfForGame(this->Owner->GameID));
}

// ============================================
// Load scenes
// ============================================

void GlobalEntranceTableModel::setScenes(const std::map<uint32_t, SceneEntranceMetaInf>& scenes)
{
    beginResetModel();

    m_rows.clear();

    for (auto& [sceneID, scene] : scenes)
    {
        for (auto& [entranceID, link] : scene.EntranceIDs)
        {
            const EntranceMetaInfo* entrance = EntranceHelper::GetEntranceMetaInf(this->Owner->GameID, entranceID);

            if (entrance == nullptr || entrance->Type == EntranceType::None)
            {
                continue;
            }

            GlobalEntranceRow row;

            row.SceneID = sceneID;
            row.EntranceID = entranceID;
            row.InLink = link.InLink;
            row.OutLink = link.OutLink;
            row.InGame = link.InLinkGame;
            row.OutGame = link.OutLinkGame;
            row.RegionID = scene.RegionID;

            row.SceneName = this->formatScene(sceneID);
            row.EntranceName = this->formatEntrance(row.EntranceID);
            row.InLinkName = this->formatEntranceLink(row.InGame, row.EntranceID, row.InLink, true);
            row.OutLinkName = this->formatEntranceLink(row.OutGame, row.EntranceID, row.OutLink, false);

            m_rows.push_back(row);
        }
    }

    // Trier par SceneID puis EntranceID
    this->sort(0, Qt::AscendingOrder);
    rebuildRowColors();

    endResetModel();

    this->Owner->SyncCounters();
}

// ============================================
// Update one entrance
// ============================================

void GlobalEntranceTableModel::updateEntrance(uint32_t sceneID, uint32_t entranceID, const EntranceLink* link)
{
    for (size_t i = 0; i < m_rows.size(); i++)
    {
        auto& row = m_rows[i];

        if (row.SceneID == sceneID && row.EntranceID == entranceID)
        {
            row.InLink = link->InLink;
            row.OutLink = link->OutLink;
            row.InGame = link->InLinkGame;
            row.OutGame = link->OutLinkGame;

            row.InLinkName = this->formatEntranceLink(row.InGame, row.EntranceID, row.InLink, true);
            row.OutLinkName = this->formatEntranceLink(row.OutGame, row.EntranceID, row.OutLink, false);

            QModelIndex top = index((int)i, 0);

            QModelIndex bottom = index((int)i, columnCount() - 1);

            this->m_rowStatusColors[i] = this->rowStatusColor(row);

            this->Owner->OnEntranceUpdated(sceneID, entranceID);

            emit dataChanged(top, bottom);
            emit headerDataChanged(Qt::Vertical, (int)i, (int)i);
            return;
        }
    }
}

// ============================================
// Row count
// ============================================

int GlobalEntranceTableModel::rowCount(const QModelIndex&) const
{
    return static_cast<int>(m_rows.size());
}

// ============================================
// Column count
// ============================================

int GlobalEntranceTableModel::columnCount(const QModelIndex&) const
{
    return 4;
}

// ============================================
// Data
// ============================================

QVariant GlobalEntranceTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const auto& row = m_rows[index.row()];

    // =========================
    // TEXTE
    // =========================

    if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
            case 0:
                return row.SceneName;

            case 1:
                return row.EntranceName;

            case 2:
                return row.InLinkName;

            case 3:
                return row.OutLinkName;
        }
    }

    // =========================
    // COULEUR DE FOND
    // =========================

    if (role == Qt::BackgroundRole)
    {
        if (index.row() < m_rowColors.size())
            return m_rowColors[index.row()];
    }

    return QVariant();
}

// ============================================
// Headers
// ============================================

QVariant GlobalEntranceTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal)
    {
        if (role != Qt::DisplayRole)
            return QVariant();

        switch (section)
        {
            case 0: return "Scene";
            case 1: return "Entrance";
            case 2: return "How to spawn here ?";
            case 3: return "Where does it lead ?";
        }
    }
    else if (orientation == Qt::Vertical)
    {
        if (role == Qt::BackgroundRole)
        {
            return m_rowStatusColors[section];
        }
    }

    return QVariant();
}

// ============================================
// Format Link
// ============================================

QString GlobalEntranceTableModel::formatLink(uint32_t id) const
{
    if (id == UINT32_MAX)
        return "????";

    return QString("0x%1")
        .arg(id, 4, 16, QChar('0'))
        .toUpper();
}


QString GlobalEntranceTableModel::formatEntrance(uint32_t entranceID) const
{
    if (entranceID == UINT32_MAX)
        return "?";

    // TODO: remplacer par vrai nom
    return EntranceHelper::GetEntranceFromName(this->Owner->GameID, entranceID);
}


QString GlobalEntranceTableModel::formatEntranceLink(uint8_t GameLink, uint32_t EntranceID, uint32_t EntranceLink, bool IsWayIn) const
{
    const EntranceMetaInfo* entrance = EntranceHelper::GetEntranceMetaInf(this->Owner->GameID, EntranceID);

    switch (entrance->Type)
    {
        case EntranceType::Normal:
        {
            break;
        }

        case EntranceType::One_Way_In:
        {
            if (!IsWayIn)
            {
                return "N/A";
            }
            if (EntranceLink == UINT32_MAX)
            {
                return "?";
            }
            return QString::fromStdString(EntranceHelper::GetOneWayInName(GameLink, EntranceLink));
        }

        case EntranceType::One_Way_Out:
        {
            if (IsWayIn)
            {
                return "N/A";
            }
            if (EntranceLink == UINT32_MAX)
            {
                return "?";
            }
            return QString::fromStdString(EntranceHelper::GetOneWayOutName(GameLink, EntranceLink));
        }

        default:
        {
            return QString();
        }
    }

    if (EntranceLink == UINT32_MAX)
    {
        return "?";
    }

    if (IsWayIn)
    {
        return QString::fromStdString(EntranceHelper::GetEntranceSpawnsString(GameLink, EntranceLink));
    }
    else
    {
        return QString::fromStdString(EntranceHelper::GetEntranceLeadsString(GameLink, EntranceLink));
    }
}


QString GlobalEntranceTableModel::formatScene(uint32_t sceneID) const
{
    return GetSceneName(this->Owner->GameID, sceneID);
}


// ============================================
// Row color logic
// ============================================


QColor GlobalEntranceTableModel::rowStatusColor(GlobalEntranceRow row) const
{
    bool hasIn = row.InLink != UINT32_MAX;
    bool hasOut = row.OutLink != UINT32_MAX;

    if (!hasIn && !hasOut)
        return QColor(200, 60, 60);      // rouge

    hasIn = hasIn || row.InLinkName == "N/A";
    hasOut = hasOut || row.OutLinkName == "N/A";

    if (hasIn ^ hasOut)
        return QColor(220, 180, 40);     // jaune

    return QColor(60, 180, 80);          // vert
}

QColor GlobalEntranceTableModel::rowColor(int rowIndex) const
{
    if (rowIndex < 0 || rowIndex >= m_rows.size())
        return QColor(45, 45, 45);

    uint32_t sceneID =
        m_rows[rowIndex].SceneID;

    bool toggle = false;

    for (int i = 0; i < rowIndex; i++)
    {
        if (m_rows[i].SceneID !=
            m_rows[i + 1].SceneID)
        {
            toggle = !toggle;
        }
    }

    QColor baseColor = toggle ? QColor(45, 45, 45) : QColor(60, 60, 60);

    const auto& row =
        m_rows[rowIndex];

    return baseColor;
}

void GlobalEntranceTableModel::sort(int column, Qt::SortOrder order)
{
    emit layoutAboutToBeChanged();

    std::stable_sort(
        m_rows.begin(),
        m_rows.end(),
        [column, order](
            const GlobalEntranceRow& a,
            const GlobalEntranceRow& b)
        {
            /*// 1️ Toujours grouper par Region

            if (a.RegionID != b.RegionID)
                return a.RegionID < b.RegionID;*/

            // 2️ Comparaison colonne

            auto compareColumn =
                [&](const GlobalEntranceRow& x,
                    const GlobalEntranceRow& y)
                {
                    switch (column)
                    {
                        case 0:
                            if (x.SceneID != y.SceneID)
                                return QString::localeAwareCompare(x.SceneName, y.SceneName) < 0;

                            return QString::localeAwareCompare(x.EntranceName, y.EntranceName) < 0;

                        case 1:
                            return QString::localeAwareCompare(x.EntranceName, y.EntranceName) < 0;

                        case 2:
                            if (x.InLink != y.InLink)
                                return x.InLink < y.InLink;

                            return x.EntranceID < y.EntranceID;

                        case 3:
                            if (x.OutLink != y.OutLink)
                                return x.OutLink < y.OutLink;

                            return x.EntranceID < y.EntranceID;

                        default:
                            return x.EntranceID < y.EntranceID;
                    }
                };

                // 3️ Asc / Desc correct

            if (order == Qt::AscendingOrder)
                return compareColumn(a, b);
            else
                return compareColumn(b, a);
        });

    rebuildRowColors();

    emit layoutChanged();
}


void GlobalEntranceTableModel::rebuildRowColors()
{
    m_rowColors.clear();
    m_rowColors.reserve(m_rows.size());

    m_rowStatusColors.clear();
    m_rowStatusColors.reserve(m_rows.size());

    uint32_t lastScene = UINT32_MAX;
    bool toggle = false;

    for (const auto& row : m_rows)
    {
        // Changement de scène → alterne couleur

        if (row.SceneID != lastScene)
        {
            toggle = !toggle;
            lastScene = row.SceneID;
        }

        QColor base = computeBaseColor(toggle);

        m_rowColors.push_back(base);
        m_rowStatusColors.push_back(this->rowStatusColor(row));
    }
}

QColor GlobalEntranceTableModel::computeBaseColor(bool toggle) const
{
    return toggle ? QColor(30, 50, 70) : QColor(50, 80, 100);
}

#pragma endregion // GlobalEntranceTableModel

#pragma region // EntranceFilterProxy

EntranceFilterProxy::EntranceFilterProxy(QObject* Parent) : QSortFilterProxyModel(Parent)
{
}


void EntranceFilterProxy::SetRegionFilter(uint8_t Region)
{
    if (this->RegionFilter == Region)
    {
        return;
    }
    this->RegionFilter = Region;
    this->invalidateFilter();
}


bool EntranceFilterProxy::filterAcceptsRow(int SourceRow, const QModelIndex& SourceParent) const
{
    if (this->RegionFilter != UINT8_MAX)
    {   // Reject rows that do not belong to the currently selected region.

        const GlobalEntranceTableModel* model = dynamic_cast<const GlobalEntranceTableModel*>(this->sourceModel());
        if (model != nullptr && SourceRow >= 0 && SourceRow < static_cast<int>(model->m_rows.size()))
        {
            if (model->m_rows[SourceRow].RegionID != this->RegionFilter)
            {
                return false;
            }
        }
    }
    return QSortFilterProxyModel::filterAcceptsRow(SourceRow, SourceParent);
}

#pragma endregion // EntranceFilterProxy

#pragma region AllEntranceView

AllEntranceView::AllEntranceView(EntranceGameTabView* Parent) : QWidget(Parent)
{
    this->Owner = Parent;
    this->Model = new GlobalEntranceTableModel(this->Owner);
    this->Model->sort(0, Qt::AscendingOrder);

    this->MainLayout = new QVBoxLayout(this);
    this->Table = new QTableView(this);

    QLineEdit* searchBar = new QLineEdit(this);
    searchBar->setPlaceholderText("Search...");

    this->Proxy = new EntranceFilterProxy(this);
    this->Proxy->setSourceModel(this->Model);
    this->Proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    this->Proxy->setFilterKeyColumn(-1);
    this->Proxy->setSortRole(Qt::UserRole);
    this->Proxy->setDynamicSortFilter(false);


    this->Table->setModel(this->Proxy);
    //this->Table->setModel(this->Model);
    this->Table->setSortingEnabled(true);
    this->Table->setAlternatingRowColors(false);
    this->Table->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->Table->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->Table->setShowGrid(false);
    this->Table->horizontalHeader()->setStretchLastSection(true);
    this->Table->horizontalHeader()->setSortIndicatorShown(true);
    this->Table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    this->Table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    this->Table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    this->Table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    this->Table->horizontalHeader()->setSortIndicator(0, Qt::AscendingOrder);
    this->Table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    this->Table->verticalHeader()->setDefaultSectionSize(20);
    this->Table->verticalHeader()->setSectionsClickable(false);
    this->Table->viewport()->setAttribute(Qt::WA_OpaquePaintEvent, true);
    this->Table->viewport()->setAttribute(Qt::WA_NoSystemBackground, true);
    this->Table->viewport()->setAttribute(Qt::WA_StaticContents, true);

    connect(searchBar, &QLineEdit::textChanged,
        this, [=](const QString& text)
        {
            this->Proxy->setFilterFixedString(text);
            this->RefreshViewportPaintMode();
        });

    connect(this->Table->horizontalHeader(), &QHeaderView::sectionClicked,
        this, [=](int column)
        {
            this->Model->sort(column, this->Table->horizontalHeader()->sortIndicatorOrder());
        });

    // Cell click navigation: route the user to the matching scene / entrance in the same way the
    // left map tree and the right entrance tree do, so the global table is a real shortcut and not
    // just a read-only view. Column dispatch:
    //   0 (Scene)   -> focus the scene
    //   1 (Entr.)   -> focus the scene + zoom on the row's entrance
    //   2 (InLink)  -> jump to the linked entrance's scene + zoom on it (skip if unknown)
    //   3 (OutLink) -> same as column 2 for the OutLink side
    connect(this->Table, &QTableView::clicked, this,
        [this](const QModelIndex& ProxyIndex)
        {
            if (!ProxyIndex.isValid() || this->Owner == nullptr || this->Owner->Owner == nullptr)
            {
                return;
            }
            QModelIndex sourceIndex = this->Proxy->mapToSource(ProxyIndex);
            int sourceRow = sourceIndex.row();
            if (sourceRow < 0 || sourceRow >= static_cast<int>(this->Model->m_rows.size()))
            {
                return;
            }
            const GlobalEntranceRow& row = this->Model->m_rows[sourceRow];
            EntranceTab* tab = this->Owner->Owner;
            int currGame = this->Owner->GameID;

            switch (ProxyIndex.column())
            {
                case 0:
                {
                    tab->FocusSceneInGame(currGame, row.SceneID);
                    break;
                }
                case 1:
                {
                    tab->FocusEntranceInGame(currGame, row.SceneID, row.EntranceID);
                    break;
                }
                case 2:
                {
                    if (row.InLink == UINT32_MAX || row.InGame == NO_GAME)
                    {
                        break;
                    }
                    const EntranceMetaInfo* linkMeta = EntranceHelper::GetEntranceMetaInf(row.InGame, row.InLink);
                    if (linkMeta == nullptr)
                    {
                        break;
                    }
                    tab->FocusEntranceInGame(row.InGame, linkMeta->ToSceneID, row.InLink);
                    break;
                }
                case 3:
                {
                    if (row.OutLink == UINT32_MAX || row.OutGame == NO_GAME)
                    {
                        break;
                    }
                    const EntranceMetaInfo* linkMeta = EntranceHelper::GetEntranceMetaInf(row.OutGame, row.OutLink);
                    if (linkMeta == nullptr)
                    {
                        break;
                    }
                    tab->FocusEntranceInGame(row.OutGame, linkMeta->ToSceneID, row.OutLink);
                    break;
                }
                default:
                {
                    break;
                }
            }
        });

    this->MainLayout->addWidget(searchBar);
    this->MainLayout->addWidget(this->Table);
    this->setLayout(this->MainLayout);
}

void AllEntranceView::RefreshContent()
{
    this->Model->setScenes(*GetSceneEntranceMetaInfForGame(this->Owner->GameID));
}


void AllEntranceView::SetRegionFilter(uint8_t Region)
{
    if (this->Proxy != nullptr)
    {
        this->Proxy->SetRegionFilter(Region);
        this->RefreshViewportPaintMode();
    }
}


void AllEntranceView::RefreshViewportPaintMode()
{
    if (this->Proxy == nullptr || this->Table == nullptr)
    {
        return;
    }

    // Disable WA_OpaquePaintEvent when the visible rows no longer cover the viewport so Qt clears
    // the background; otherwise stale rows from the previous filter state remain painted underneath.
    bool filtering = this->Proxy->rowCount() > 40;
    this->Table->viewport()->setAttribute(Qt::WA_OpaquePaintEvent, filtering);
    this->Table->viewport()->update();
}

#pragma endregion // AllEntranceView

#pragma region // EntranceSceneView

EntranceSceneView::EntranceSceneView(QGraphicsScene* Scene, QWidget* Parent) : QGraphicsView(Scene, Parent)
{
    this->setRenderHint(QPainter::Antialiasing);
    this->setRenderHint(QPainter::SmoothPixmapTransform);
    this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);  // Zoom centered under the cursor
    this->setDragMode(QGraphicsView::ScrollHandDrag);                // Pan by click-and-drag
}


void EntranceSceneView::wheelEvent(QWheelEvent* Event)
{
    // Mirror MapView's behavior so the entrance map feels identical to the item tracker map.
    const double scaleFactor = 1.15;
    if (Event->angleDelta().y() > 0)
    {
        this->scale(scaleFactor, scaleFactor);
    }
    else if (Event->angleDelta().y() < 0)
    {
        this->scale(1.0 / scaleFactor, 1.0 / scaleFactor);
    }
}

#pragma endregion // EntranceSceneView


#pragma region // EntranceGameTabView

SceneEntranceItemTree::SceneEntranceItemTree(SceneEntranceMetaInf* Inf, EntranceGameTabView* PaOwner, QTreeWidgetItem* Parent) : CommonBaseItemTree(Parent)
{
    this->Owner = PaOwner;
    this->SceneInf = Inf;
    this->CountValidEntrances();
    this->RefreshItemName();
}


int SceneEntranceItemTree::GetTotalObjectAvailable()
{
    return this->TotalEntrances;
}


void SceneEntranceItemTree::CountValidEntrances()
{
    this->FoundEntrances = 0;
    this->TotalEntrances = 0;

    GlobalEntranceTableModel* model = (this->Owner != nullptr && this->Owner->AllView != nullptr) ? this->Owner->AllView->Model : nullptr;
    if (model == nullptr)
    {
        return;
    }

    for (auto& row : model->m_rows)
    {
        if (row.SceneID != this->SceneInf->SceneID)
        {
            continue;
        }

        const EntranceMetaInfo* entrance = EntranceHelper::GetEntranceMetaInf(this->Owner->GameID, row.EntranceID);
        if (entrance == nullptr)
        {
            continue;
        }

        switch (entrance->Type)
        {
            case EntranceType::Normal:
            {
                this->TotalEntrances += 2;
                if (row.InLink != UINT32_MAX) this->FoundEntrances++;
                if (row.OutLink != UINT32_MAX) this->FoundEntrances++;
                break;
            }
            case EntranceType::One_Way_In:
            {
                this->TotalEntrances++;
                if (row.InLink != UINT32_MAX) this->FoundEntrances++;
                break;
            }
            case EntranceType::One_Way_Out:
            {
                this->TotalEntrances++;
                if (row.OutLink != UINT32_MAX) this->FoundEntrances++;
                break;
            }
            default:
            {
                break;
            }
        }
    }
}


void SceneEntranceItemTree::RefreshItemName()
{
    QString finalName = BuildCountLabel(GetSceneName(this->Owner->GameID, this->SceneInf->SceneID), this->FoundEntrances, this->TotalEntrances);
    this->setText(0, finalName);
}


EntranceGameTabView::EntranceGameTabView(int Game, const char * Name, EntranceTab* parent) : QWidget(parent)
{
    this->Owner = parent;
    this->GameID = Game;
    this->TabName = Name;
    this->MainLayout = new QHBoxLayout();
    this->LayoutSplitter = new QSplitter(Qt::Horizontal);

    // All View
    this->AllView = new AllEntranceView(this);

    // Scene Map view: EntranceSceneView configures antialiasing, drag-pan and wheel zoom itself.
    this->SceneMapScene = new QGraphicsScene(this);
    this->SceneMapView = new EntranceSceneView(this->SceneMapScene, this);

    // Center stack: AllView when no scene is selected, SceneMapView otherwise
    this->CenterStack = new QStackedWidget(this);
    this->CenterStack->addWidget(this->AllView);
    this->CenterStack->addWidget(this->SceneMapView);
    this->CenterStack->setCurrentIndex(0);

    // Map tree
    this->MapList = new CustomTreeWidget("Maps", 300, this);

    // Entrance tree
    this->EntranceList = new CustomTreeWidget("Entrances", 500, this);
    this->EntranceList->setVisible(false);

    // Entrance renderer: wraps the graphics scene/view and owns the per-scene entrance tree items.
    this->Renderer = new EntranceRenderer(this);
    this->Renderer->SetTarget(this->SceneMapScene, this->SceneMapView);

    // Layout
    this->LayoutSplitter->addWidget(this->MapList);
    this->LayoutSplitter->addWidget(this->CenterStack);
    this->LayoutSplitter->addWidget(this->EntranceList);
    this->LayoutSplitter->setSizes({ 300, 1320, 300 });

    this->MainLayout->addWidget(this->LayoutSplitter);

    this->setLayout(this->MainLayout);

    auto scenes = GetSceneEntranceMetaInfForGame(Game);

    for (auto& [sceneID, MetaInf] : *scenes)
    {   // Only keep scenes that have at least one valid entrance

        bool hasValid = false;
        for (auto& [entranceID, link] : MetaInf.EntranceIDs)
        {
            const EntranceMetaInfo* entrance = EntranceHelper::GetEntranceMetaInf(Game, entranceID);
            if (entrance != nullptr && entrance->Type != EntranceType::None)
            {
                hasValid = true;
                break;
            }
        }
        if (!hasValid)
        {
            continue;
        }

        RegionTree* currRegion = this->FindRegionTree(MetaInf.RegionID);
        if (currRegion == nullptr)
        {   // Create a new region in the tree list

            currRegion = new RegionTree((GameTab*)this, MetaInf.RegionID, this->MapList->List);
            this->Regions.push_back(currRegion);
        }

        MetaInf.MapPath = GetSceneMiniMap(Game, MetaInf.SceneID);
        new SceneEntranceItemTree(&MetaInf, this, currRegion);
    }

    this->SyncCounters();
    this->MapList->List->sortItems(0, Qt::AscendingOrder);

    // "All" pseudo-region pinned at the top of the map tree: selecting it shows the global entrance
    // view without any region filter. Inserted after sortItems so it stays at index 0 regardless of
    // the alphabetical order of the real regions.
    QTreeWidgetItem* allItem = new QTreeWidgetItem();
    allItem->setText(0, "All");
    QFont allFont = allItem->font(0);
    allFont.setBold(true);
    allItem->setFont(0, allFont);
    this->MapList->List->insertTopLevelItem(0, allItem);

    QObject::connect(this->MapList->List, &QTreeWidget::currentItemChanged, this,
        [this](QTreeWidgetItem* current, QTreeWidgetItem* /*previous*/)
        {
            this->OnSceneSelected(current);
        });

    QObject::connect(this->EntranceList->List, &QTreeWidget::itemClicked, this,
        [](QTreeWidgetItem* item, int /*column*/)
        {
            if (item == nullptr)
            {
                return;
            }
            CommonBaseItemTree* action = dynamic_cast<CommonBaseItemTree*>(item);
            if (action != nullptr)
            {
                action->PerformAction();
            }
        });

    // A single click re-centers the map on the link, a double click reuses the same PerformAction
    // path in "from-graph" mode so the user can navigate to the destination scene directly from the
    // tree (same effect as clicking the arrow or the label on the map).
    QObject::connect(this->EntranceList->List, &QTreeWidget::itemDoubleClicked, this,
        [](QTreeWidgetItem* item, int /*column*/)
        {
            EntranceLinkItemTree* link = dynamic_cast<EntranceLinkItemTree*>(item);
            if (link == nullptr)
            {
                return;
            }
            link->SetCalledFromGraph(true);
            link->PerformAction();
            link->SetCalledFromGraph(false);
        });
}


void EntranceGameTabView::OnSceneSelected(QTreeWidgetItem* Current)
{
    SceneEntranceItemTree* sceneItem = dynamic_cast<SceneEntranceItemTree*>(Current);
    if (sceneItem != nullptr)
    {   // Scene selected: render its map and populate the right entrance tree.

        // Populate the right tree first so the overlay renderer can look up the link tree items it needs
        // to wire map-arrow clicks onto the same PerformAction pipeline as tree clicks.
        this->PopulateEntranceList(sceneItem->SceneInf);
        this->RenderSceneMap(sceneItem->SceneInf);
        this->CenterStack->setCurrentIndex(1);
        this->EntranceList->setVisible(true);
        return;
    }

    // Region or "All" pseudo-item: show the global entrance view, optionally restricted to one region.
    RegionTree* regionItem = dynamic_cast<RegionTree*>(Current);
    uint8_t regionFilter = (regionItem != nullptr && regionItem->MetaInfo != nullptr)
                            ? regionItem->MetaInfo->Region
                            : UINT8_MAX;
    if (this->AllView != nullptr)
    {
        this->AllView->SetRegionFilter(regionFilter);
    }
    this->CenterStack->setCurrentIndex(0);
    this->EntranceList->setVisible(false);
}


void EntranceGameTabView::RenderSceneMap(SceneEntranceMetaInf* Scene)
{
    if (this->SceneMapItem != nullptr)
    {   // Remove and destroy the previously rendered image

        this->SceneMapScene->removeItem(this->SceneMapItem);
        delete this->SceneMapItem;
        this->SceneMapItem = nullptr;
    }
    if (this->SceneMapImage != nullptr)
    {
        delete this->SceneMapImage;
        this->SceneMapImage = nullptr;
    }

    if (Scene->MapPath != NULL)
    {   // Load the new map image into the graphics scene

        this->SceneMapImage = new QPixmap(Scene->MapPath);
        this->SceneMapItem = this->SceneMapScene->addPixmap(*this->SceneMapImage);

        // Defer the fit to the next event-loop tick: on the very first scene selection the
        // SceneMapView is still hidden inside the QStackedWidget (index 0 shows the all-view), so
        // its viewport size is the default one and fitInView would scale against stale geometry.
        // By the time the timer fires, setCurrentIndex(1) has run and the view has been resized
        // to its real on-screen size, so the map actually fits the visible area.
        QPointer<QGraphicsView> view = this->SceneMapView;
        QPointer<QGraphicsScene> scene = this->SceneMapScene;
        QTimer::singleShot(0, [view, scene]()
        {
            if (!view.isNull() && !scene.isNull())
            {
                view->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
            }
        });
    }

    // Paint name labels + clickable arrows on top of the freshly loaded map. Done here (and not in
    // PopulateEntranceList) so the overlay cleanly attaches to the current scene's pixmap and so
    // RenderSceneMap remains the single method that knows how the map is drawn.
    if (this->Renderer != nullptr)
    {
        this->Renderer->RenderSceneOverlay(Scene);
    }
}


void EntranceGameTabView::PopulateEntranceList(SceneEntranceMetaInf* Scene)
{
    this->EntranceList->List->clear();
    if (this->Renderer != nullptr)
    {
        this->Renderer->Clear();
    }

    CommonBaseItemTree* normalCat = new CommonBaseItemTree();
    CommonBaseItemTree* oneWayInCat = new CommonBaseItemTree();
    CommonBaseItemTree* oneWayOutCat = new CommonBaseItemTree();

    QFont font = normalCat->font(0);
    font.setBold(true);
    normalCat->setFont(0, font);
    oneWayInCat->setFont(0, font);
    oneWayOutCat->setFont(0, font);

    ObjectIcons * refIcons = ObjectIcons::GetIconsRef();
    normalCat->setIcon(0, refIcons->EntranceIcons[0]);
    oneWayInCat->setIcon(0, refIcons->EntranceIcons[1]);
    oneWayOutCat->setIcon(0, refIcons->EntranceIcons[2]);

    GlobalEntranceTableModel* model = this->AllView != nullptr ? this->AllView->Model : nullptr;

    for (auto& [entranceID, link] : Scene->EntranceIDs)
    {
        const EntranceMetaInfo* entrance = EntranceHelper::GetEntranceMetaInf(this->GameID, entranceID);
        if (entrance == nullptr || entrance->Type == EntranceType::None)
        {
            continue;
        }

        GlobalEntranceRow* row = nullptr;
        if (model != nullptr)
        {
            for (auto& r : model->m_rows)
            {
                if (r.SceneID == Scene->SceneID && r.EntranceID == entranceID)
                {
                    row = &r;
                    break;
                }
            }
        }
        if (row == nullptr)
        {
            continue;
        }

        CommonBaseItemTree* category = nullptr;
        switch (entrance->Type)
        {
            case EntranceType::Normal:       category = normalCat; break;
            case EntranceType::One_Way_In:   category = oneWayInCat; break;
            case EntranceType::One_Way_Out:  category = oneWayOutCat; break;
            default: break;
        }

        if (category != nullptr)
        {
            new EntranceItemTree(row, this->Renderer, category);
        }
    }

    auto addOrDelete = [this](CommonBaseItemTree* cat, const char* name)
    {
        if (cat->childCount() > 0)
        {   // Store the base name in UserRole so RefreshCategoryCounters can rebuild the count label later.
            cat->setData(0, Qt::UserRole, QString::fromUtf8(name));
            this->EntranceList->List->addTopLevelItem(cat);
            cat->setExpanded(true);
        }
        else
        {
            delete cat;
        }
    };

    addOrDelete(normalCat, "Normal");
    addOrDelete(oneWayInCat, "One Way In");
    addOrDelete(oneWayOutCat, "One Way Out");

    this->RefreshCategoryCounters();
}


void EntranceGameTabView::RefreshRegionCounters()
{
    for (RegionTree* region : this->Regions)
    {
        uint32_t found = 0;
        uint32_t total = 0;
        for (int i = 0; i < region->childCount(); i++)
        {
            SceneEntranceItemTree* scene = dynamic_cast<SceneEntranceItemTree*>(region->child(i));
            if (scene != nullptr)
            {
                found += scene->FoundEntrances;
                total += scene->TotalEntrances;
            }
        }
        region->setText(0, BuildCountLabel(region->MetaInfo->RegionName, found, total));
    }
}


void EntranceGameTabView::OnEntranceUpdated(uint32_t SceneID, uint32_t EntranceID)
{
    if (this->Renderer != nullptr)
    {
        EntranceItemTree* entrance = this->Renderer->FindEntrance(SceneID, EntranceID);
        if (entrance != nullptr)
        {
            entrance->RefreshText();
        }
    }

    this->SyncCounters();
}


void EntranceGameTabView::SyncCounters()
{
    this->FoundEntrances = 0;
    this->TotalEntrances = 0;

    for (RegionTree* region : this->Regions)
    {
        for (int i = 0; i < region->childCount(); i++)
        {
            SceneEntranceItemTree* scene = dynamic_cast<SceneEntranceItemTree*>(region->child(i));
            if (scene == nullptr)
            {
                continue;
            }
            scene->CountValidEntrances();
            scene->RefreshItemName();
            this->FoundEntrances += scene->FoundEntrances;
            this->TotalEntrances += scene->TotalEntrances;
        }
    }

    this->RefreshRegionCounters();
    this->RefreshCategoryCounters();
    this->RefreshName();
}


void EntranceGameTabView::RefreshCategoryCounters()
{
    if (this->EntranceList == nullptr || this->EntranceList->List == nullptr)
    {
        return;
    }

    QTreeWidget* tree = this->EntranceList->List;
    for (int i = 0; i < tree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* cat = tree->topLevelItem(i);
        if (cat == nullptr)
        {
            continue;
        }
        QVariant baseName = cat->data(0, Qt::UserRole);
        if (!baseName.isValid())
        {
            continue;
        }

        uint32_t found = 0;
        uint32_t total = 0;
        for (int j = 0; j < cat->childCount(); j++)
        {
            EntranceItemTree* entrance = dynamic_cast<EntranceItemTree*>(cat->child(j));
            if (entrance == nullptr)
            {
                continue;
            }
            GlobalEntranceRow* row = entrance->GetRow();
            const EntranceMetaInfo* meta = entrance->GetMetaInfo();
            if (row == nullptr || meta == nullptr)
            {
                continue;
            }
            switch (meta->Type)
            {
                case EntranceType::Normal:
                {
                    total += 2;
                    if (row->InLink != UINT32_MAX) found++;
                    if (row->OutLink != UINT32_MAX) found++;
                    break;
                }
                case EntranceType::One_Way_In:
                {
                    total++;
                    if (row->InLink != UINT32_MAX) found++;
                    break;
                }
                case EntranceType::One_Way_Out:
                {
                    total++;
                    if (row->OutLink != UINT32_MAX) found++;
                    break;
                }
                default:
                {
                    break;
                }
            }
        }

        QByteArray bytes = baseName.toString().toUtf8();
        cat->setText(0, BuildCountLabel(bytes.constData(), found, total));
    }
}

void EntranceGameTabView::RefreshContent()
{
    this->AllView->RefreshContent();
    //this->Model->setScenes(*GetSceneEntranceMetaInfForGame(this->Game));
}


void EntranceGameTabView::RefreshName()
{
    this->Owner->setTabText(this->GameID, this->GetRefreshedName(this->TabName, FoundEntrances, TotalEntrances));
    this->Owner->RefreshName();
}

RegionTree* EntranceGameTabView::FindRegionTree(uint8_t Region)
{
    return FindRegionTreeIn(this->Regions, Region);
}


void EntranceGameTabView::FocusEntranceInGame(uint32_t SceneID, uint32_t EntranceID)
{
    this->FocusSceneInGame(SceneID);

    // Defer the zoom to the next event-loop tick: RenderSceneMap fits the freshly loaded pixmap to
    // the viewport via QTimer::singleShot(0). Both timers fire in scheduling order, so our zoom runs
    // right after the fit and ends up as the final transform on the view.
    QPointer<EntranceGameTabView> self = this;
    QTimer::singleShot(0, [self, EntranceID]()
    {
        if (self.isNull() || self->Renderer == nullptr)
        {
            return;
        }
        const EntranceMetaInfo* meta = EntranceHelper::GetEntranceMetaInf(self->GameID, EntranceID);
        if (meta == nullptr)
        {
            return;
        }
        const int* pos = nullptr;
        if (meta->Type == EntranceType::Normal || meta->Type == EntranceType::One_Way_In)
        {
            pos = meta->InPosition;
        }
        else if (meta->Type == EntranceType::One_Way_Out)
        {
            pos = meta->OutPosition;
        }
        self->Renderer->CenterAndZoomViewOn(pos);
    });
}


void EntranceGameTabView::FocusSceneInGame(uint32_t SceneID)
{
    if (this->MapList == nullptr || this->MapList->List == nullptr)
    {
        return;
    }

    for (RegionTree* region : this->Regions)
    {
        for (int i = 0; i < region->childCount(); i++)
        {
            SceneEntranceItemTree* scene = dynamic_cast<SceneEntranceItemTree*>(region->child(i));
            if (scene != nullptr && scene->SceneInf != nullptr && scene->SceneInf->SceneID == SceneID)
            {
                // Expand the containing region so the user actually sees the newly selected scene in
                // the tree. setCurrentItem fires currentItemChanged, which re-enters OnSceneSelected
                // and takes care of re-rendering the map + entrance list.
                region->setExpanded(true);
                this->MapList->List->setCurrentItem(scene);
                this->MapList->List->scrollToItem(scene);
                return;
            }
        }
    }
}

#pragma endregion // EntranceGameTabView

#pragma region // EntranceTab

EntranceTab::EntranceTab(int TabIndex, QTabWidget* parent) : QTabWidget(parent)
{
    this->TabIndex = TabIndex;
    this->Owner = parent;
    this->TabName = "Entrances";
    this->OoTEntranceTab = new EntranceGameTabView(OOT_GAME, "OoT", this);
    this->MMEntranceTab = new EntranceGameTabView(MM_GAME, "MM", this);

    this->addTab(this->OoTEntranceTab, this->OoTEntranceTab->TabName);
    this->addTab(this->MMEntranceTab, this->MMEntranceTab->TabName);

    this->OoTEntranceTab->RefreshName();
    this->MMEntranceTab->RefreshName();
}


void EntranceTab::UpdateEntranceWay(int Game, uint32_t SceneID, uint32_t EntranceID, const EntranceLink* Link)
{
    if (Game == OOT_GAME)
    {
        this->OoTEntranceTab->AllView->Model->updateEntrance(SceneID, EntranceID, Link);
    }
    else
    {
        this->MMEntranceTab->AllView->Model->updateEntrance(SceneID, EntranceID, Link);
    }
}


void EntranceTab::RefreshEntranceTab()
{
    this->OoTEntranceTab->RefreshContent();
    this->MMEntranceTab->RefreshContent();
}


void EntranceTab::RefreshName()
{
    if (this->OoTEntranceTab && this->MMEntranceTab)
    {
        this->FoundEntrances = this->OoTEntranceTab->FoundEntrances + this->MMEntranceTab->FoundEntrances;
        this->TotalEntrances = this->OoTEntranceTab->TotalEntrances + this->MMEntranceTab->TotalEntrances;
        this->Owner->setTabText(this->TabIndex, this->GetRefreshedName(this->TabName, this->FoundEntrances, this->TotalEntrances));
    }
}


void EntranceTab::FocusSceneInGame(int Game, uint32_t SceneID)
{
    EntranceGameTabView* target = (Game == OOT_GAME) ? this->OoTEntranceTab : this->MMEntranceTab;
    if (target == nullptr)
    {
        return;
    }

    // Ensure the destination game's sub-tab is visible before focusing the scene, otherwise a cross
    // game arrow click would silently update the hidden tab and leave the user staring at the wrong
    // map. setCurrentWidget is a no-op when the target is already active.
    this->setCurrentWidget(target);
    target->FocusSceneInGame(SceneID);
}


void EntranceTab::FocusEntranceInGame(int Game, uint32_t SceneID, uint32_t EntranceID)
{
    EntranceGameTabView* target = (Game == OOT_GAME) ? this->OoTEntranceTab : this->MMEntranceTab;
    if (target == nullptr)
    {
        return;
    }

    this->setCurrentWidget(target);
    target->FocusEntranceInGame(SceneID, EntranceID);
}

#pragma endregion // EntranceTab
