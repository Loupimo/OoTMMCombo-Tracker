#pragma once

#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QHBoxLayout>
#include <QSplitter>
#include <QStackedWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QHash>
#include "UI/SceneEntrance.h"
#include "UI/RegionTab.h"
#include "UI/ObjectRenderer.h"
#include "UI/EntranceRenderer.h"
#include "Common.h"

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

class EntranceGameTabView;
class EntranceTab;

class GlobalEntranceTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    EntranceGameTabView* Owner = nullptr;

    /*
    *   Constructs the global entrance table model with the given parent game tab.
    *
    *   @param Parent    The owning entrance game tab view, used to resolve the game and counters.
    */
    explicit GlobalEntranceTableModel(EntranceGameTabView* parent = nullptr);

    /*
    *   Populate the model with the given scene entrance meta info and refresh the row counters.
    *
    *   @param Scenes    The scene entrance meta info map to load.
    */
    void setScenes(const std::map<uint32_t, SceneEntranceMetaInf>& scenes);

    /*
    *   Update the row matching the given scene and entrance with the new link data.
    *
    *   @param SceneID       The scene ID of the row to update.
    *   @param EntranceID    The entrance ID of the row to update.
    *   @param Link          The new link data to apply.
    */
    void updateEntrance(uint32_t sceneID, uint32_t entranceID, const EntranceLink* link);

    /*
    *   Get the number of rows currently stored in the model.
    *
    *   @param Parent    Unused parent index (Qt override).
    *
    *   @return The number of rows in the model.
    */
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    /*
    *   Get the number of columns exposed by the model.
    *
    *   @param Parent    Unused parent index (Qt override).
    *
    *   @return The number of columns in the model.
    */
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    /*
    *   Return the data displayed or styled for the given cell index.
    *
    *   @param Index    The cell index to fetch data for.
    *   @param Role     The Qt role describing what kind of data is requested.
    *
    *   @return The cell text for DisplayRole, the cell background for BackgroundRole, or an empty QVariant.
    */
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    /*
    *   Return the header text or background color for the given section.
    *
    *   @param Section        The header section index.
    *   @param Orientation    The header orientation (horizontal or vertical).
    *   @param Role           The Qt role describing what kind of data is requested.
    *
    *   @return The header label for horizontal sections, the row status color for vertical sections, or an empty QVariant.
    */
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;


    /*
    *   Build the display name of the given entrance ID.
    *
    *   @param EntranceID    The entrance ID to format.
    *
    *   @return The resolved entrance name, or "?" if the ID is invalid.
    */
    QString formatEntrance(uint32_t entranceID) const;

    /*
    *   Build the display name of an entrance link based on its game, entrance and direction.
    *
    *   @param GameLink       The game ID of the linked entrance.
    *   @param EntranceID     The entrance ID of the current row.
    *   @param EntranceLink   The linked entrance ID to format.
    *   @param IsWayIn        True if the link is the "way in" (spawn) side, false if it is the "way out" (lead) side.
    *
    *   @return The formatted link name, "N/A" for unapplicable directions, "?" if unset, or an empty string on unknown type.
    */
    QString formatEntranceLink(uint8_t GameLink, uint32_t EntranceID, uint32_t EntranceLink, bool IsWayIn) const;

    /*
    *   Build the display name of the given scene ID.
    *
    *   @param SceneID    The scene ID to format.
    *
    *   @return The resolved scene name.
    */
    QString formatScene(uint32_t sceneID) const;

    /*
    *   Sort the model rows by the given column, always grouping rows by region first.
    *
    *   @param Column    The column to sort by.
    *   @param Order     The sort order (ascending or descending).
    */
    void sort(int column, Qt::SortOrder order) override;

    /*
    *   Compute the status color of the given row based on whether its in / out links are set.
    *
    *   @param Row    The row to evaluate.
    *
    *   @return Red if both links are missing, yellow if only one is set, green if both are set.
    */
    QColor rowStatusColor(GlobalEntranceRow row) const;

private:

    /*
    *   Build the display name of an entrance or scene link from its raw ID.
    *
    *   @param Id    The raw ID to format.
    *
    *   @return The formatted link string.
    */
    QString formatLink(uint32_t id) const;

    /*
    *   Get the background color for the given row index.
    *
    *   @param Row    The row index to query.
    *
    *   @return The background color for that row.
    */
    QColor rowColor(int row) const;

    /*
    *   Recompute and cache the alternating background colors for all rows.
    */
    void rebuildRowColors();

    /*
    *   Compute the base alternating row color for the given toggle state.
    *
    *   @param Toggle    True for the first alternating shade, false for the second.
    *
    *   @return The base background color for that shade.
    */
    QColor computeBaseColor(bool toggle) const;
public:

    std::vector<GlobalEntranceRow> m_rows;
    std::vector<QColor> m_rowColors;
    std::vector<QColor> m_rowStatusColors;

};


// ==============================
// Widget Class
// ==============================

class AllEntranceView : public QWidget
{

public:

    QVBoxLayout* MainLayout;
    QTableView* Table;
    GlobalEntranceTableModel* Model;
    QSortFilterProxyModel* Proxy;
    EntranceGameTabView* Owner;

    /*
    *   Constructs the all-entrance view with a search bar and a sortable table of entrances.
    *
    *   @param Parent    The owning entrance game tab view.
    */
    AllEntranceView(EntranceGameTabView* Parent = nullptr);

    /*
    *   Rebuild the table content from the current scene entrance meta info of the owning game.
    */
    void RefreshContent();
};


class EntranceGameTabView;

/*
*   The tree item that wraps a scene of the entrance tab.
*   Holds counters of valid (non-None) entrances and refreshes its label as "Scene (found / total)".
*/
class SceneEntranceItemTree : public CommonBaseItemTree
{
public:

    EntranceGameTabView* Owner;             // The owning entrance game tab view.
    SceneEntranceMetaInf* SceneInf;         // The wrapped scene meta info.
    uint32_t FoundEntrances = 0;            // The number of bound entrance links of this scene.
    uint32_t TotalEntrances = 0;            // The total number of valid entrances of this scene.

    /*
    *   Construct the scene entrance item tree and compute its initial counters.
    *
    *   @param Inf       The scene entrance meta info to wrap.
    *   @param PaOwner   The owning entrance game tab view.
    *   @param Parent    The parent tree item to attach this item to.
    */
    SceneEntranceItemTree(SceneEntranceMetaInf* Inf, EntranceGameTabView* PaOwner, QTreeWidgetItem* Parent = nullptr);

    /*
    *   Recompute the found and total valid entrance counters from the wrapped scene info.
    */
    void CountValidEntrances();

    /*
    *   Refresh the displayed scene name with the current counters.
    */
    void RefreshItemName();

    /*
    *   Return the total number of valid entrances of this scene, used by the tree filter.
    *
    *   @return The total number of valid entrances of this scene.
    */
    int GetTotalObjectAvailable() override;
};


class EntranceGameTabView : public QWidget, public ICommonFunc
{
    Q_OBJECT

public:

    int GameID;
    QHBoxLayout* MainLayout;
    QSplitter* LayoutSplitter;
    CustomTreeWidget* MapList;
    CustomTreeWidget* EntranceList;

    AllEntranceView* AllView;
    QStackedWidget* CenterStack;                // Switches between AllView and the per-scene map view.
    QGraphicsView* SceneMapView;                // The graphical view used to render the selected scene's mini-map.
    QGraphicsScene* SceneMapScene;              // The graphics scene that holds the mini-map.
    QPixmap* SceneMapImage = nullptr;           // The currently rendered mini-map image.
    QGraphicsPixmapItem* SceneMapItem = nullptr;// The graphical item attached to the scene.

    EntranceTab* Owner;
    const char* TabName;                // The tab name. Should correspond to the game it refers to.
    uint32_t FoundEntrances = 0;
    uint32_t TotalEntrances = 0;
    std::vector<RegionTree*> Regions;           // The list of all available regions
    EntranceRenderer* Renderer = nullptr;       // The entrance renderer for the per-scene tree and future graphical overlay.

    /*
    *   Constructs the entrance game tab view for the given game.
    *
    *   @param Game      The game ID this tab belongs to (OOT_GAME or MM_GAME).
    *   @param Name      The tab display name.
    *   @param Parent    The owning entrance tab, used as the Qt parent.
    */
    EntranceGameTabView(int Game, const char * Name, EntranceTab* parent = nullptr);

    /*
    *   Refresh the content of the all-entrance view.
    */
    void RefreshContent();

    /*
    *   Refresh the tab name to reflect the current found / total entrance counters.
    */
    void RefreshName() override;

    /*
    *   Find the region tree matching the given region ID.
    *
    *   @param Region    The region ID to match.
    *
    *   @return The matching region tree if found, nullptr otherwise.
    */
    RegionTree* FindRegionTree(uint8_t Region);

    /*
    *   React to a selection change in the map tree:
    *       - if the selected item is a scene, render its map and populate the entrance list,
    *       - otherwise restore the all-entrance view.
    *
    *   @param Current    The newly selected tree item.
    */
    void OnSceneSelected(QTreeWidgetItem* Current);

    /*
    *   Render the mini-map of the given scene in the center view.
    *   Clears the previous image first; does not render anything if the scene has no map path.
    *
    *   @param Scene    The scene whose mini-map should be rendered.
    */
    void RenderSceneMap(SceneEntranceMetaInf* Scene);

    /*
    *   Populate the right entrance tree with the entrances of the given scene grouped by EntranceType.
    *
    *   @param Scene    The scene whose entrances should be displayed.
    */
    void PopulateEntranceList(SceneEntranceMetaInf* Scene);

    /*
    *   Refresh every region label with the aggregated found / total entrance counters of its scenes.
    */
    void RefreshRegionCounters();

    /*
    *   Refresh every right-tree category label with the aggregated found / total entrance counters of its children.
    */
    void RefreshCategoryCounters();

    /*
    *   Recompute every counter (owner totals, scene items, region labels, right-tree categories) from the
    *   model's m_rows (single source of truth) and refresh the tab name.
    */
    void SyncCounters();

    /*
    *   React to a model update for the given entrance: refresh the matching link items then trigger a full counter sync.
    *
    *   @param SceneID       The scene ID of the updated entrance.
    *   @param EntranceID    The entrance ID of the updated entrance.
    */
    void OnEntranceUpdated(uint32_t SceneID, uint32_t EntranceID);

    /*
    *   Programmatically select the scene matching the given ID in the left map tree so that the
    *   currentItemChanged signal fires and OnSceneSelected re-renders the scene view and entrance
    *   list. No-op if no matching scene tree item exists (e.g. scene has no valid entrance).
    *
    *   @param SceneID    The scene ID to focus in the left tree.
    */
    void FocusSceneInGame(uint32_t SceneID);
};


class EntranceTab : public QTabWidget, public ICommonFunc
{
    Q_OBJECT

public:

    EntranceGameTabView* OoTEntranceTab;
    EntranceGameTabView* MMEntranceTab;
    //QTableView* OoTEntranceTab;
    //QTableView* MMEntranceTab;
    //GlobalEntranceTableModel* OoTEntranceModel;
    //GlobalEntranceTableModel* MMEntranceModel;
    const char* TabName;                // The tab name. Should correspond to the game it refers to.
    QTabWidget* Owner;
    int TabIndex;
    uint32_t FoundEntrances = 0;
    uint32_t TotalEntrances = 0;

    /*
    *   Constructs the entrance tab and its per-game sub-tabs (OoT and MM).
    *
    *   @param TabIndex    The index of this tab in the owning tab widget.
    *   @param Parent      The owning tab widget, used as the Qt parent.
    */
    explicit EntranceTab(int TabIndex, QTabWidget* parent = nullptr);

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


    /*
    *   Refresh the tab name to reflect the aggregated found / total entrance counters of both games.
    */
    void RefreshName() override;

    /*
    *   Switch the visible game sub-tab to the one matching the given game and focus the given scene
    *   inside its map tree. Used by arrow-driven navigation on the map so an arrow click can lead to
    *   the other game's scene when the discovered entrance crosses OoT <-> MM.
    *
    *   @param Game       The destination game (OOT_GAME or MM_GAME).
    *   @param SceneID    The scene ID to focus in that game's map tree.
    */
    void FocusSceneInGame(int Game, uint32_t SceneID);
};
