#pragma once

#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
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
#include "UI/GPSRouteWidget.h"
#include "UI/EntranceCostTab.h"
#include "Common.h"

#include <map>
#include <cstdint>


// ===== Ligne interne =====

typedef struct GlobalEntranceRow
{
    uint32_t SceneID = UINT32_MAX;
    uint32_t EntranceID = UINT32_MAX;
    uint32_t InLink = UINT32_MAX;       // Latest known inbound source entrance ID (UINT32_MAX if none).
    uint32_t OutLink = UINT32_MAX;
    uint8_t InGame = NO_GAME;            // Game of the latest known inbound source.
    uint8_t OutGame = NO_GAME;
    uint8_t RegionID = UINT8_MAX;

    QString SceneName;
    QString EntranceName;
    QString InLinkName;                  // Formatted name of the latest inbound source (table column display).
    QStringList InLinkNames;             // Formatted names of every known inbound source, in discovery order.
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
// Filter proxy
// ==============================

/*
*   Proxy on top of the global entrance table that combines the standard text-search filter with an
*   optional region filter. Used by the left map tree so that selecting a region restricts the
*   visible rows to that region while still letting the search bar narrow the results further.
*/
class EntranceFilterProxy : public QSortFilterProxyModel
{
    Q_OBJECT

public:

    /*
    *   Construct the proxy with no active region filter.
    *
    *   @param Parent    The Qt parent.
    */
    explicit EntranceFilterProxy(QObject* Parent = nullptr);

    /*
    *   Restrict the visible rows to the given region ID. Pass UINT8_MAX to disable the filter.
    *
    *   @param Region    The region ID to filter by, or UINT8_MAX to show every row.
    */
    void SetRegionFilter(uint8_t Region);

protected:

    /*
    *   Keep a row when both the inherited text search and the optional region filter accept it.
    *
    *   @param SourceRow       The row index in the source model.
    *   @param SourceParent    The parent index in the source model.
    *
    *   @return True if the row should remain visible.
    */
    bool filterAcceptsRow(int SourceRow, const QModelIndex& SourceParent) const override;

private:

    uint8_t RegionFilter = UINT8_MAX;       // The region ID currently filtered, or UINT8_MAX when no region filter is active.
};


// ==============================
// Multi-source InLink delegate
// ==============================

/*
*   Custom item delegate for the "How to spawn here?" column (column 2 of the entrance table).
*   When an entrance has several inbound sources (warp song zones, boss-room returns, etc.) the
*   delegate paints one stacked sub-row per source inside the same cell — visually a single cell
*   with multiple lines, but each line responds independently to the user's clicks and routes the
*   navigation to the matching source via EntranceTab::FocusEntranceInGame.
*/
class EntranceInLinkDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:

    /*
    *   Construct the delegate bound to its owning game tab view, which provides the access path
    *   to the model, the proxy and the EntranceTab used to route the per-source navigation.
    *
    *   @param Owner     The owning entrance game tab view.
    *   @param Parent    Optional Qt parent.
    */
    explicit EntranceInLinkDelegate(EntranceGameTabView* Owner, QObject* Parent = nullptr);

    /*
    *   Paint one sub-row per known inbound source. Falls back to the default delegate rendering
    *   when the row has zero or one source (preserves the legacy single-line look).
    *
    *   @param Painter    The painter to draw with.
    *   @param Option     The Qt style option carrying the cell rect and palette.
    *   @param Index      The proxy index of the cell being painted.
    */
    void paint(QPainter* Painter, const QStyleOptionViewItem& Option, const QModelIndex& Index) const override;

    /*
    *   Return a size hint that grows with the source count so every sub-row gets a full line of
    *   vertical space.
    *
    *   @param Option    The Qt style option carrying the cell rect.
    *   @param Index     The proxy index of the cell being measured.
    *
    *   @return The recommended size for the cell.
    */
    QSize sizeHint(const QStyleOptionViewItem& Option, const QModelIndex& Index) const override;

    /*
    *   Intercept mouse press events on the cell to figure out which sub-row was clicked and
    *   navigate to the matching source. Returns true when the click was handled so the default
    *   selection logic does not duplicate the action.
    *
    *   @param Event     The event being delivered to the cell.
    *   @param Model     The model the cell belongs to.
    *   @param Option    The Qt style option carrying the cell rect.
    *   @param Index     The proxy index of the cell receiving the event.
    *
    *   @return True if the event was consumed by the delegate, false otherwise.
    */
    bool editorEvent(QEvent* Event, QAbstractItemModel* Model, const QStyleOptionViewItem& Option, const QModelIndex& Index) override;

    /*
    *   Filter mouse-move / leave events on the table viewport so we can keep track of the exact
    *   sub-row currently under the cursor inside a multi-source InLink cell. Updates the
    *   per-sub-row hover state used by paint() and triggers a viewport repaint when it changes.
    *
    *   @param Watched    The object being filtered (expected to be the table viewport).
    *   @param Event      The event delivered to the watched object.
    *
    *   @return False — we always let the event continue to the original handler.
    */
    bool eventFilter(QObject* Watched, QEvent* Event) override;

private:

    /*
    *   Resolve the GlobalEntranceRow backing the given proxy index. Returns nullptr if the index
    *   is invalid or the source row is out of bounds.
    *
    *   @param ProxyIndex    The proxy index of the cell being inspected.
    *
    *   @return The matching row pointer, or nullptr if not found.
    */
    const struct GlobalEntranceRow* ResolveRow(const QModelIndex& ProxyIndex) const;

    EntranceGameTabView* OwnerView = nullptr;

    // Per-sub-row hover state. QPersistentModelIndex keeps the cell reference valid across
    // sort / filter passes so the hover does not point at a stale row after the proxy reshuffles.
    // HoveredSubRow == -1 means "no multi-source sub-row is currently under the cursor".
    mutable QPersistentModelIndex HoveredCell;
    mutable int HoveredSubRow = -1;
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
    EntranceFilterProxy* Proxy;
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

    /*
    *   Restrict the visible rows to the given region ID. Pass UINT8_MAX to disable the region filter.
    *
    *   @param Region    The region ID to filter by, or UINT8_MAX to show every row.
    */
    void SetRegionFilter(uint8_t Region);

    /*
    *   Toggle the table viewport's WA_OpaquePaintEvent attribute based on the current visible row
    *   count. The attribute is kept enabled for large row counts (>40) to keep scrolling smooth, and
    *   disabled when few rows remain so Qt erases the background and we don't see stale rows from
    *   the previous filter state. Must be called every time the filter (text or region) changes.
    */
    void RefreshViewportPaintMode();

    /*
    *   Re-apply per-row heights for every visible row so multi-source entries are tall enough to
    *   show all their sub-lines. Heights are positional in Qt (indexed by view row, not source
    *   row) so this must run after any mutation that reshuffles the proxy: initial build, sort,
    *   filter changes, and live entrance updates.
    */
    void RefreshRowHeights();
};


class EntranceGameTabView;


/*
*   A QGraphicsView specialized for the entrance scene map. Adds mouse-wheel zoom on top of the
*   pan-by-drag behavior already provided by ScrollHandDrag, mirroring the MapView used by the
*   item tracker so both views feel identical to the user.
*/
class EntranceSceneView : public QGraphicsView
{
    Q_OBJECT

public:

    /*
    *   Construct the view with the given scene.
    *
    *   @param Scene    The graphics scene this view renders.
    *   @param Parent   The parent widget.
    */
    explicit EntranceSceneView(QGraphicsScene* Scene, QWidget* Parent = nullptr);

protected:

    /*
    *   Scale the view in or out around the mouse cursor on each wheel tick. Vertical scroll only;
    *   horizontal scroll is ignored so trackpad horizontal gestures do not surprise-zoom.
    *
    *   @param Event    The wheel event carrying the scroll delta.
    */
    void wheelEvent(QWheelEvent* Event) override;
};


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
    EntranceSceneView* SceneMapView;            // The graphical view used to render the selected scene's mini-map.
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
    *   Rebuild the left map tree from scratch based on each scene's current ActiveLayout. Necessary
    *   because some scenes only have entrances valid under specific layouts (e.g. the JP grottos in
    *   MM_DEKU_PALACE under GameLayout::mm_jp); without this rebuild they stay invisible after the
    *   layout is switched via Settings::ApplySettings. Preserves the current scene selection when the
    *   scene still has at least one valid entrance under the new layout.
    */
    void RebuildSceneTree();

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

    /*
    *   Focus the given scene in the left map tree, then zoom the scene view on the given entrance.
    *   The zoom is deferred to the next event-loop tick because RenderSceneMap fits the view via a
    *   QTimer::singleShot(0); our zoom must run after that fit so it actually overrides the transform.
    *
    *   @param SceneID       The scene ID to focus in the left tree.
    *   @param EntranceID    The entrance ID to zoom on, in the focused scene.
    */
    void FocusEntranceInGame(uint32_t SceneID, uint32_t EntranceID);
};


class EntranceTab : public QTabWidget, public ICommonFunc
{
    Q_OBJECT

public:

    EntranceGameTabView* OoTEntranceTab;
    EntranceGameTabView* MMEntranceTab;
    GPSRouteWidget* GPSTab;
    EntranceCostTab* CostTab;
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

    /*
    *   Switch the visible game sub-tab, focus the given scene and zoom on the given entrance. Used
    *   by the AllEntranceView when the user clicks an Entrance / In Link / Out Link cell so the same
    *   "navigate-and-zoom" flow available from the entrance tree is reachable from the global table.
    *
    *   @param Game          The destination game (OOT_GAME or MM_GAME).
    *   @param SceneID       The scene ID to focus in that game's map tree.
    *   @param EntranceID    The entrance ID to zoom on inside the focused scene.
    */
    void FocusEntranceInGame(int Game, uint32_t SceneID, uint32_t EntranceID);
};
