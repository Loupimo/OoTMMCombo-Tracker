#include "UI/EntranceTable.h"
#include "Combo/Entrances.h"
#include "Combo/Scenes.h"

#include <QVBoxLayout>
#include <QHeaderView>


EntranceTableWidget::EntranceTableWidget(int Game, const char * Name, QWidget* parent) : QWidget(parent)
{
    this->tableView = new QTableView(this);

    this->model = new QStandardItemModel(this);

    this->proxyModel = new QSortFilterProxyModel(this);
    this->proxyModel->setSourceModel(this->model);

    this->tableView->setModel(this->proxyModel);

    this->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    this->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    this->tableView->horizontalHeader()->setStretchLastSection(true);

    tableView->setShowGrid(true);

    tableView->horizontalHeader()->setStretchLastSection(true);

    tableView->verticalHeader()->setVisible(false);
    tableView->setSortingEnabled(true);
    tableView->sortByColumn(0, Qt::AscendingOrder);
    this->setupModel();

    QVBoxLayout* layout = new QVBoxLayout(this);

    layout->addWidget(tableView);

    connect(this->tableView, &QTableView::doubleClicked, this, &EntranceTableWidget::onRowDoubleClicked);
    this->Game = Game;
    this->TabName = Name;
    this->setScenes(GetSceneEntranceMetaInfForGame(this->Game));
}

// ==============================
// Model Setup
// ==============================

void EntranceTableWidget::setupModel()
{
    this->model->setColumnCount(5);

    this->model->setHorizontalHeaderLabels({"Scene", "Entrance", "Way In", "Way Out", "Cost" });
}

// ==============================
// Public API
// ==============================

void EntranceTableWidget::setScenes(const std::map<uint32_t, SceneEntranceMetaInf>& scenes)
{
    this->scenesData = scenes;
    this->populateTable();
}

// ==============================
// Populate Table
// ==============================

void EntranceTableWidget::populateTable()
{
    this->model->removeRows(0, this->model->rowCount());

    int row = 0;

    for (auto& [sceneID, scene] : this->scenesData)
    {
        for (auto& [entranceID, link] : scene.EntranceIDs)
        {
            const EntranceMetaInfo* entrance = EntranceHelper::GetEntranceMetaInf(this->Game, entranceID);

            if (entrance->Type == EntranceType::None)
                continue;

            this->model->insertRow(row);

            // Scene
            this->model->setItem(row, 0, new QStandardItem(formatScene(sceneID)));

            // Entrance
            auto entranceItem = new QStandardItem(formatEntrance(entranceID));

            // store ID internally
            entranceItem->setData(entranceID, Qt::UserRole);

            this->model->setItem(row, 1, entranceItem);

            // Way In
            this->model->setItem(row, 2, new QStandardItem(formatEntranceLink(entranceID, &link, true)));

            // Way Out
            this->model->setItem(row, 3, new QStandardItem(formatEntranceLink(entranceID, &link, false)));

            // Cost placeholder
            this->model->setItem(row, 4, new QStandardItem("1"));

            //this->applyRowColor(row, link);

            row++;
        }
    }

    tableView->resizeColumnsToContents();
}

// ==============================
// Formatting Helpers
// ==============================

QString EntranceTableWidget::formatEntrance(uint32_t entranceID)
{
    if (entranceID == UINT32_MAX)
        return "?";

    // TODO: remplacer par vrai nom
    return EntranceHelper::GetEntranceFromName(this->Game, entranceID);
}


QString EntranceTableWidget::formatEntranceLink(uint32_t EntranceID, EntranceLink * EntranceLink, bool IsWayIn)
{
    const EntranceMetaInfo * entrance = EntranceHelper::GetEntranceMetaInf(this->Game, EntranceID);

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
            break;
        }

        case EntranceType::One_Way_Out:
        {
            if (IsWayIn)
            {
                return "N/A";
            }
            break;
        }

        default:
        {
            return QString();
        }
    }

    if (IsWayIn)
    {
        if (EntranceLink->InLink == UINT32_MAX)
        {
            return "?";
        }
        return QString::fromStdString(EntranceHelper::GetEntranceFromToString(this->Game, EntranceLink->InLink));
    }
    else
    {
        if (EntranceLink->OutLink == UINT32_MAX)
        {
            return "?";
        }
        return QString::fromStdString(EntranceHelper::GetEntranceFromToString(this->Game, EntranceLink->OutLink));
    }
}


QString EntranceTableWidget::formatScene(uint32_t sceneID)
{
    // TODO: remplacer par vrai nom
    return GetSceneName(this->Game, sceneID);
}

// ==============================
// Row Coloring
// ==============================

void EntranceTableWidget::applyRowColor(
    int row,
    const EntranceLink& link)
{
    QBrush background;
    QBrush foreground(Qt::black);

    bool hasIn = link.InLink != UINT32_MAX;
    bool hasOut = link.OutLink != UINT32_MAX;

    if (!hasIn && !hasOut)
        background = QBrush(Qt::lightGray);

    else if (hasIn && !hasOut)
        background = QBrush(Qt::yellow);

    else if (!hasIn && hasOut)
        background = QBrush(Qt::cyan);

    else
        background = QBrush(Qt::green);

    for (int col = 0; col < model->columnCount(); col++)
    {
        auto item = model->item(row, col);

        if (item)
        {
            item->setBackground(background);
            item->setForeground(foreground);
        }
    }
}
// ==============================
// Double Click Handling
// ==============================

void EntranceTableWidget::onRowDoubleClicked(const QModelIndex& index)
{
    QModelIndex sourceIndex = proxyModel->mapToSource(index);

    int row = sourceIndex.row();

    auto item = model->item(row, 1);

    if (!item)
        return;

    uint32_t entranceID = item->data(Qt::UserRole).toUInt();

    emit entranceActivated(entranceID);
}





EntranceTab::EntranceTab(QTabWidget* parent) : QTabWidget(parent)
{
    this->TabName = "Entrances";
    this->OoTEntranceTab = new EntranceTableWidget(OOT_GAME, "OoT", this);
    this->MMEntranceTab = new EntranceTableWidget(MM_GAME, "MM", this);

    this->addTab(this->OoTEntranceTab, this->OoTEntranceTab->TabName);
    this->addTab(this->MMEntranceTab, this->MMEntranceTab->TabName);
}