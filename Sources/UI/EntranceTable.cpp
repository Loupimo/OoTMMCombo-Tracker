#include "UI/EntranceTable.h"
#include "Combo/Entrances.h"
#include "Combo/Scenes.h"

#include <QVBoxLayout>
#include <QHeaderView>


EntranceTableWidget::EntranceTableWidget(QWidget* parent) : QWidget(parent)
{
    tableView = new QTableView(this);

    model = new QStandardItemModel(this);

    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);

    tableView->setModel(proxyModel);

    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    tableView->horizontalHeader()->setStretchLastSection(true);

    tableView->setAlternatingRowColors(true);

    tableView->setStyleSheet(R"(
QTableView {
    background-color: #232629;
    alternate-background-color: #2b2b2b;
    color: #DFE1E2;
    gridline-color: #3A3F44;
}

QTableView::item {
    background-color: #2b2b2b;
    color: #DFE1E2;
}

QTableView::item:selected {
    background-color: #346792;
    color: white;
}

QHeaderView::section {
    background-color: #31363B;
    color: #DFE1E2;
    padding: 4px;
    border: 1px solid #3A3F44;
}
)");

    setupModel();

    QVBoxLayout* layout = new QVBoxLayout(this);

    layout->addWidget(tableView);

    connect(tableView, &QTableView::doubleClicked, this, &EntranceTableWidget::onRowDoubleClicked);

    this->TabName = "Entrances";
    this->setScenes(GetSceneEntranceMetaInfForGame(OOT_GAME));
}

// ==============================
// Model Setup
// ==============================

void EntranceTableWidget::setupModel()
{
    model->setColumnCount(5);

    model->setHorizontalHeaderLabels({"Scene", "Entrance", "Way In", "Way Out", "Cost" });
}

// ==============================
// Public API
// ==============================

void EntranceTableWidget::setScenes(const std::map<uint32_t, SceneEntranceMetaInf>& scenes)
{
    scenesData = scenes;
    populateTable();
}

// ==============================
// Populate Table
// ==============================

void EntranceTableWidget::populateTable()
{
    model->removeRows(0, model->rowCount());

    int row = 0;

    for (auto& [sceneID, scene] : scenesData)
    {
        for (auto& [entranceID, link] : scene.EntranceIDs)
        {
            const EntranceMetaInfo* entrance = EntranceHelper::GetEntranceMetaInf(OOT_GAME, entranceID);

            if (entrance->Type == EntranceType::None)
                continue;

            model->insertRow(row);

            // Scene
            model->setItem(row, 0, new QStandardItem(formatScene(sceneID)));

            // Entrance
            auto entranceItem = new QStandardItem(formatEntrance(entranceID));

            // store ID internally
            entranceItem->setData(entranceID, Qt::UserRole);

            model->setItem(row, 1, entranceItem);

            // Way In
            model->setItem(row, 2, new QStandardItem(formatEntranceLink(entranceID, &link, true)));

            // Way Out
            model->setItem(row, 3, new QStandardItem(formatEntranceLink(entranceID, &link, false)));

            // Cost placeholder
            model->setItem(row, 4, new QStandardItem("1"));

            applyRowColor(row, link);

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
    return EntranceHelper::GetEntranceFromName(OOT_GAME, entranceID);
}


QString EntranceTableWidget::formatEntranceLink(uint32_t EntranceID, EntranceLink * EntranceLink, bool IsWayIn)
{
    const EntranceMetaInfo * entrance = EntranceHelper::GetEntranceMetaInf(OOT_GAME, EntranceID);

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
        return QString::fromStdString(EntranceHelper::GetEntranceFromToString(OOT_GAME, EntranceLink->InLink));
    }
    else
    {
        if (EntranceLink->OutLink == UINT32_MAX)
        {
            return "?";
        }
        return QString::fromStdString(EntranceHelper::GetEntranceFromToString(OOT_GAME, EntranceLink->OutLink));
    }
}


QString EntranceTableWidget::formatScene(uint32_t sceneID)
{
    // TODO: remplacer par vrai nom
    return GetSceneName(OOT_GAME, sceneID);
}

// ==============================
// Row Coloring
// ==============================

void EntranceTableWidget::applyRowColor(int row, const EntranceLink& link)
{
    QBrush brush;

    bool hasIn = link.InLink != UINT32_MAX;

    bool hasOut = link.OutLink != UINT32_MAX;

    if (!hasIn && !hasOut)
        brush = QBrush(Qt::lightGray);

    else if (hasIn && !hasOut)
        brush = QBrush(Qt::yellow);

    else if (!hasIn && hasOut)
        brush = QBrush(Qt::cyan);

    else
        brush = QBrush(Qt::green);

    for (int col = 0; col < model->columnCount(); col++)
    {
        auto item = model->item(row, col);

        if (item)
            item->setBackground(brush);
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