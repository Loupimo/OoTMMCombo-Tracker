#pragma once

#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include "UI/SceneEntrance.h"

#include <map>
#include <cstdint>


// ===== Ligne interne =====

typedef struct GlobalEntranceRow
{
    uint32_t SceneID = UINT32_MAX;
    uint32_t EntranceID = UINT32_MAX;
    uint32_t InLink = UINT32_MAX;
    uint32_t OutLink = UINT32_MAX;
    uint8_t InGame = NO_GAME;
    uint8_t OutGame = NO_GAME;
    uint8_t RegionID = UINT8_MAX;

    QString SceneName;
    QString EntranceName;
    QString InLinkName;
    QString OutLinkName;
} GlobalEntranceRow;


// ===== Model =====

class GlobalEntranceTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    int Game;
    const char* TabName;                // The tab name. Should correspond to the game it refers to.

    explicit GlobalEntranceTableModel(int Game, const char* Name, QObject* parent = nullptr);

    // Chargement complet

    void setScenes(const std::map<uint32_t, SceneEntranceMetaInf>& scenes);

    // Mise à jour d'une entrée

    void updateEntrance(uint32_t sceneID, uint32_t entranceID, const EntranceLink* link);

    // Qt overrides

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;


    QString formatEntrance(uint32_t entranceID) const;
    QString formatEntranceLink(uint8_t GameLink, uint32_t EntranceID, uint32_t EntranceLink, bool IsWayIn) const;
    QString formatScene(uint32_t sceneID) const;
    
    void sort(int column, Qt::SortOrder order) override;
    QColor rowStatusColor(GlobalEntranceRow row) const;

private:

    QString formatLink(uint32_t id) const;

    QColor rowColor(int row) const;

    void rebuildRowColors();
    QColor computeBaseColor(bool toggle) const;
public:

    std::vector<GlobalEntranceRow> m_rows;
    std::vector<QColor> m_rowColors;
    std::vector<QColor> m_rowStatusColors;

};


// ==============================
// Widget Class
// ==============================

class EntranceTableView : public QWidget
{
    Q_OBJECT

public:

    int Game;
    QTableView* Table;
    GlobalEntranceTableModel * Model;
    QSortFilterProxyModel* Proxy;
    const char* TabName;                // The tab name. Should correspond to the game it refers to.


    EntranceTableView(int Game, const char * Name, QWidget* parent = nullptr);

    void RefreshContent();
};


class EntranceTab : public QTabWidget
{
    Q_OBJECT

public:

    EntranceTableView* OoTEntranceTab;
    EntranceTableView* MMEntranceTab;
    //QTableView* OoTEntranceTab;
    //QTableView* MMEntranceTab;
    //GlobalEntranceTableModel* OoTEntranceModel;
    //GlobalEntranceTableModel* MMEntranceModel;
    const char* TabName;                // The tab name. Should correspond to the game it refers to.


    explicit EntranceTab(QTabWidget* parent = nullptr);

    /*
    *   Update the entrance status with the given information.
    *
    *	@param OutGame		    The game the out entrance ID belongs to.
    *	@param InGame		    The game the in entrance ID belongs to.
    *	@param OutEntranceID	The entrance ID you come from.
    *	@param InEntranceID		The entrance ID you arrived to.
    *
    */
    void UpdateEntranceWay(int Game, uint32_t SceneID, uint32_t EntranceID, const EntranceLink* Link);


    /*
    *   Refresh all elements of this entrance tab.
    */
    void RefreshEntranceTab();
};