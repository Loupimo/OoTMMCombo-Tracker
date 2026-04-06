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

                row.SceneName = this->formatScene(sceneID);
                row.EntranceName = this->formatEntrance(row.EntranceID);
                row.InLinkName = this->formatEntranceLink(row.InGame, row.EntranceID, row.InLink, true);
                row.OutLinkName = this->formatEntranceLink(row.OutGame, row.EntranceID, row.OutLink, false);

                m_rows.push_back(row);
        }
    }

    // Trier par SceneID puis EntranceID

    // Trier
    std::sort(
        m_rows.begin(),
        m_rows.end(),
        [](const GlobalEntranceRow& a,
            const GlobalEntranceRow& b)
        {
            if (a.SceneID != b.SceneID)
                return a.SceneID < b.SceneID;

            return a.EntranceID < b.EntranceID;
        });

        // NOUVEAU
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
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
    {
        switch (section)
        {
            case 0: return "Scene";
            case 1: return "Entrance";
            case 2: return "How to spawn here ?";
            case 3: return "Where does it lead ?";
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

QColor GlobalEntranceTableModel::rowColor(int rowIndex) const
{
    if (rowIndex < 0 ||
        rowIndex >= m_rows.size())
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

    QColor baseColor =
        toggle
        ? QColor(45, 45, 45)
        : QColor(60, 60, 60);

    const auto& row =
        m_rows[rowIndex];

    bool hasIn =
        row.InLink != UINT32_MAX;

    bool hasOut =
        row.OutLink != UINT32_MAX;

    if (!hasIn && !hasOut)
        return baseColor;

    if (hasIn && !hasOut)
        return QColor(200, 200, 0);

    if (!hasIn && hasOut)
        return QColor(0, 180, 180);

    return QColor(0, 180, 0);
}

void GlobalEntranceTableModel::sort(int column, Qt::SortOrder order)
{
    beginResetModel();

    auto compAscending =
        [this, column](
            const GlobalEntranceRow& a,
            const GlobalEntranceRow& b)
        {
            switch (column)
            {
                case 0: // Scene
                {
                    if (a.SceneID != b.SceneID)
                        return QString::localeAwareCompare(a.SceneName, b.SceneName) < 0;

                    return QString::localeAwareCompare(a.EntranceName, b.EntranceName) < 0;
                }

                case 1: // Entrance (alphabetic)
                {
                    return QString::localeAwareCompare(a.EntranceName, b.EntranceName) < 0;
                }

                case 2: // Way In
                {
                    if (a.InLink != b.InLink)
                        return a.InLink < b.InLink;

                    return a.EntranceID < b.EntranceID;
                }

                case 3: // Way Out
                {
                    if (a.OutLink != b.OutLink)
                        return a.OutLink < b.OutLink;

                    return a.EntranceID < b.EntranceID;
                }
            }

            return false;
        };

    if (order == Qt::AscendingOrder)
    {
        std::sort(
            m_rows.begin(),
            m_rows.end(),
            compAscending);
    }
    else
    {
        std::sort(
            m_rows.begin(),
            m_rows.end(),
            [compAscending](
                const GlobalEntranceRow& a,
                const GlobalEntranceRow& b)
            {
                return compAscending(b, a);
            });
    }

    rebuildRowColors();

    endResetModel();
}


void GlobalEntranceTableModel::rebuildRowColors()
{
    m_rowColors.clear();
    m_rowColors.reserve(m_rows.size());

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

        QColor base =
            computeBaseColor(toggle);

        bool hasIn =
            row.InLink != UINT32_MAX;

        bool hasOut =
            row.OutLink != UINT32_MAX;

        QColor finalColor;

        //if (!hasIn && !hasOut)
        {
            finalColor = base;
        }
       /* else if (hasIn && !hasOut)
        {
            finalColor =
                QColor(240, 220, 80);   // jaune clair
        }
        else if (!hasIn && hasOut)
        {
            finalColor =
                QColor(80, 220, 220);   // cyan clair
        }
        else
        {
            finalColor =
                QColor(80, 200, 120);   // vert
        }*/

        m_rowColors.push_back(finalColor);
    }
}

QColor GlobalEntranceTableModel::computeBaseColor(bool toggle) const
{
    return toggle ? QColor(30, 50, 70) : QColor(50, 80, 100);
}

#pragma endregion // GlobalEntranceTableModel

#pragma region // EntranceTableView

EntranceTableView::EntranceTableView(int Game, const char * Name, QWidget* parent) : QTableView(parent)
{
    this->Game = Game;
    this->TabName = Name;
    this->Model = new GlobalEntranceTableModel(Game, Name, this);
    this->Model->sort(0, Qt::AscendingOrder);

    this->setModel(this->Model);
    this->setSortingEnabled(true);
    this->setAlternatingRowColors(false);
    this->horizontalHeader()->setStretchLastSection(true);
    this->horizontalHeader()->setSortIndicatorShown(true);
    this->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    this->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    this->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    this->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    this->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    this->verticalHeader()->setDefaultSectionSize(20);
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
    //this->OoTEntranceModel = new GlobalEntranceTableModel(OOT_GAME, "OoT", this);
    //this->MMEntranceModel = new GlobalEntranceTableModel(MM_GAME, "MM", this);
    //this->OoTEntranceTab = new QTableView(this);
    //this->MMEntranceTab = new QTableView(this);

    //this->OoTEntranceTab->setModel(this->OoTEntranceModel);
    //this->MMEntranceTab->setModel(this->MMEntranceModel);
    /*

    this->OoTEntranceTab->setSortingEnabled(true);
    this->MMEntranceTab->setSortingEnabled(true);
    this->OoTEntranceTab->setAlternatingRowColors(true);
    this->MMEntranceTab->setAlternatingRowColors(true);
    this->OoTEntranceTab->horizontalHeader()->setStretchLastSection(true);
    this->MMEntranceTab->horizontalHeader()->setStretchLastSection(true);
    this->OoTEntranceModel->sort(0, Qt::AscendingOrder);
    this->MMEntranceModel->sort(0, Qt::AscendingOrder);
    this->OoTEntranceTab->horizontalHeader()->setSortIndicatorShown(true);
    this->MMEntranceTab->horizontalHeader()->setSortIndicatorShown(true);
    this->OoTEntranceTab->setAlternatingRowColors(false);
    this->MMEntranceTab->setAlternatingRowColors(false);
    this->OoTEntranceTab->horizontalHeader()->setStretchLastSection(true);
    this->MMEntranceTab->horizontalHeader()->setStretchLastSection(true);
    this->OoTEntranceTab->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    this->MMEntranceTab->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    this->OoTEntranceTab->verticalHeader()->setDefaultSectionSize(20);
    this->MMEntranceTab->verticalHeader()->setDefaultSectionSize(20);*/
    /*this->OoTEntranceTab->sortByColumn(0, Qt::AscendingOrder);
    this->MMEntranceTab->sortByColumn(0, Qt::AscendingOrder);


    this->OoTEntranceTab->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    this->OoTEntranceTab->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    this->OoTEntranceTab->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    this->OoTEntranceTab->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);

    this->MMEntranceTab->horizontalHeader()->setSectionResizeMode(
        0,
        QHeaderView::ResizeToContents);

    this->MMEntranceTab->horizontalHeader()->setSectionResizeMode(
        1,
        QHeaderView::ResizeToContents);

    this->MMEntranceTab->horizontalHeader()->setSectionResizeMode(
        2,
        QHeaderView::Stretch);

    this->MMEntranceTab->horizontalHeader()->setSectionResizeMode(
        3,
        QHeaderView::Stretch);*/

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