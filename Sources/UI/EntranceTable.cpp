#include "UI/EntranceTable.h"
#include "Combo/Entrances.h"
#include "Combo/Scenes.h"

#include <QVBoxLayout>
#include <QHeaderView>


#include <algorithm>

#pragma region // GlobalEntranceTableModel

// ============================================
// Constructor
// ============================================

GlobalEntranceTableModel::GlobalEntranceTableModel(int Game, const char* Name, QObject* parent) : QAbstractTableModel(parent)
{
    this->Game = Game;
    this->TabName = Name;
    this->setScenes(*GetSceneEntranceMetaInfForGame(this->Game));
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
            const EntranceMetaInfo* entrance = EntranceHelper::GetEntranceMetaInf(this->Game, entranceID);

            if (entrance->Type == EntranceType::None)
                continue;


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
    return EntranceHelper::GetEntranceFromName(this->Game, entranceID);
}


QString GlobalEntranceTableModel::formatEntranceLink(uint8_t GameLink, uint32_t EntranceID, uint32_t EntranceLink, bool IsWayIn) const
{
    const EntranceMetaInfo* entrance = EntranceHelper::GetEntranceMetaInf(this->Game, EntranceID);

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
    return GetSceneName(this->Game, sceneID);
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

#pragma region // EntranceTableView

EntranceTableView::EntranceTableView(int Game, const char * Name, QWidget* parent) : QWidget(parent)
{
    this->Game = Game;
    this->TabName = Name;
    this->Model = new GlobalEntranceTableModel(Game, Name, this);
    this->Model->sort(0, Qt::AscendingOrder); 

    QVBoxLayout* layout = new QVBoxLayout(this);
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
            bool filtering = !text.isEmpty();

            this->Table->viewport()->setAttribute(Qt::WA_OpaquePaintEvent, !filtering);
            this->Proxy->setFilterFixedString(text);
        });

    connect(this->Table->horizontalHeader(), &QHeaderView::sectionClicked,
        this, [=](int column)
        {
            this->Model->sort(column, this->Table->horizontalHeader()->sortIndicatorOrder());
        });

    layout->addWidget(searchBar);
    layout->addWidget(this->Table);
    this->setLayout(layout);
}


void EntranceTableView::RefreshContent()
{
    this->Model->setScenes(*GetSceneEntranceMetaInfForGame(this->Game));
}

#pragma endregion // EntranceTableView

#pragma region // EntranceTab

EntranceTab::EntranceTab(QTabWidget* parent) : QTabWidget(parent)
{
    this->TabName = "Entrances";
    this->OoTEntranceTab = new EntranceTableView(OOT_GAME, "OoT", this);
    this->MMEntranceTab = new EntranceTableView(MM_GAME, "MM", this);

    this->addTab(this->OoTEntranceTab, this->OoTEntranceTab->TabName);
    this->addTab(this->MMEntranceTab, this->MMEntranceTab->TabName);
}


void EntranceTab::UpdateEntranceWay(int Game, uint32_t SceneID, uint32_t EntranceID, const EntranceLink* Link)
{
    if (Game == OOT_GAME)
    {
        this->OoTEntranceTab->Model->updateEntrance(SceneID, EntranceID, Link);
    }
    else
    {
        this->MMEntranceTab->Model->updateEntrance(SceneID, EntranceID, Link);
    }
}


void EntranceTab::RefreshEntranceTab()
{
    this->OoTEntranceTab->RefreshContent();
    this->MMEntranceTab->RefreshContent();
}

#pragma endregion // EntranceTab