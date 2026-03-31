#pragma once

#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include "UI/SceneEntrance.h"

#include <map>
#include <cstdint>


// ==============================
// Widget Class
// ==============================

class EntranceTableWidget : public QWidget
{
    Q_OBJECT

public:


    const char* TabName;                // The tab name. Should correspond to the game it refers to.


    explicit EntranceTableWidget(QWidget* parent = nullptr);

    // Remplir le tableau
    void setScenes(
        const std::map<uint32_t, SceneEntranceMetaInf>& scenes
    );

signals:

    // Double clic → navigation
    void entranceActivated(uint32_t entranceID);

private slots:

    void onRowDoubleClicked(const QModelIndex& index);

private:

    // UI
    QTableView* tableView;

    // Models
    QStandardItemModel* model;
    QSortFilterProxyModel* proxyModel;

    // Data
    std::map<uint32_t, SceneEntranceMetaInf> scenesData;

    // Helpers
    void setupModel();
    void populateTable();

    QString formatEntrance(uint32_t entranceID);
    QString formatEntranceLink(uint32_t EntranceID, EntranceLink* EntranceLink, bool IsWayIn);
    QString formatScene(uint32_t sceneID);

    void applyRowColor(
        int row,
        const EntranceLink& link
    );
};