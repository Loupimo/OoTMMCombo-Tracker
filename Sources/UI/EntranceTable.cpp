#include "UI/EntranceTable.h"
#include "Combo/Entrances.h"
#include "Combo/Scenes.h"
#include "UI/ObjectRenderer.h"

#include <QVBoxLayout>
#include <QHeaderView>


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
    this->Owner->FoundEntrances = 0;
    this->Owner->TotalEntrances = 0;

    beginResetModel();

    m_rows.clear();

    for (auto& [sceneID, scene] : scenes)
    {
        for (auto& [entranceID, link] : scene.EntranceIDs)
        {
            const EntranceMetaInfo* entrance = EntranceHelper::GetEntranceMetaInf(this->Owner->GameID, entranceID);

            switch (entrance->Type)
            {
                case EntranceType::One_Way_In:
                case EntranceType::One_Way_Out:
                {
                    this->Owner->TotalEntrances++;
                    break;
                }

                case EntranceType::Normal:
                {
                    this->Owner->TotalEntrances += 2;
                    break;
                }

                default:
                {
                    continue;
                }
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

            if (row.InLink != UINT32_MAX)
            {
                this->Owner->FoundEntrances++;
            }
            if (row.OutLink != UINT32_MAX)
            {
                this->Owner->FoundEntrances++;
            }
        }
    }

    // Trier par SceneID puis EntranceID
    this->sort(0, Qt::AscendingOrder);
    rebuildRowColors();

    endResetModel();

    this->Owner->RefreshName();
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

            if (row.InLink != UINT32_MAX)
            {
                this->Owner->FoundEntrances++;
            }
            if (row.OutLink != UINT32_MAX)
            {
                this->Owner->FoundEntrances++;
            }

            this->Owner->RefreshName();

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
            // 1️ Toujours grouper par Region

            if (a.RegionID != b.RegionID)
                return a.RegionID < b.RegionID;

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

                // 3️⃣ Asc / Desc correct

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

    this->Proxy = new QSortFilterProxyModel(this);
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

            // Disable this setting in order to be able to get rid of artifacts in case the number of rows does not cover the entire window.
            bool filtering = this->Proxy->rowCount() > 40;
            this->Table->viewport()->setAttribute(Qt::WA_OpaquePaintEvent, filtering);
        });

    connect(this->Table->horizontalHeader(), &QHeaderView::sectionClicked,
        this, [=](int column)
        {
            this->Model->sort(column, this->Table->horizontalHeader()->sortIndicatorOrder());
        });

    this->MainLayout->addWidget(searchBar);
    this->MainLayout->addWidget(this->Table);
    this->setLayout(this->MainLayout);
}

void AllEntranceView::RefreshContent()
{
    this->Model->setScenes(*GetSceneEntranceMetaInfForGame(this->Owner->GameID));
}

#pragma endregion // AllEntranceView

#pragma region // EntranceGameTabView

EntranceGameTabView::EntranceGameTabView(int Game, const char * Name, EntranceTab* parent) : QWidget(parent)
{
    this->Owner = parent;
    this->GameID = Game;
    this->TabName = Name;
    this->MainLayout = new QHBoxLayout();
    this->LayoutSplitter = new QSplitter(Qt::Horizontal);

    // All View
    this->AllView = new AllEntranceView(this);

    // Map tree
    this->MapList = new CustomTreeWidget("Maps", 300, this);

    // Entrance tree
    this->EntranceList = new CustomTreeWidget("Entrances", 300, this);
    this->EntranceList->setVisible(false);

    // Layout
    this->LayoutSplitter->addWidget(this->MapList);
    this->LayoutSplitter->addWidget(this->AllView);
    this->LayoutSplitter->addWidget(this->EntranceList);

    this->MainLayout->addWidget(this->LayoutSplitter);

    this->setLayout(this->MainLayout);

    auto scenes = GetSceneEntranceMetaInfForGame(Game);

    for (auto& [sceneID, MetaInf] : *scenes)
    {
        RegionTree* currRegion = this->FindRegionTree(MetaInf.RegionID);
        if (currRegion == nullptr)
        {   // Create a new region in the tree list

            currRegion = new RegionTree((GameTab*)this, MetaInf.RegionID, this->MapList->List);
            this->Regions.push_back(currRegion);
        }

        QTreeWidgetItem* comm = new QTreeWidgetItem(currRegion);
        comm->setText(0, GetSceneName(this->GameID, sceneID));
        //this->MapList->List->addChild(GetSceneName(this->Owner->Game, sceneID));
    }
}

RegionTree* EntranceGameTabView::FindRegionTree(uint8_t Region)
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

#pragma endregion // EntranceTab