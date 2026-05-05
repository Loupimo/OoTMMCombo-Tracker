#pragma once

#include <QWidget>
#include <QLabel>
#include <QHash>
#include <QSet>
#include <QList>
#include <QPair>
#include <QTabBar>
#include <QStackedWidget>
#include <QScrollArea>
#include <QTreeWidget>
#include <QPushButton>
#include "Combo/Items.h"
#include "Combo/Objects.h"
#include "UI/Icons.h"

class OoTMMComboTracker;
class ProgressionTab;
class Settings;
class QGridLayout;
struct ProgSection;

/*
*   Custom widget that represents a single tracked item entry inside the progression dashboard.
*   Displays a 64x64 icon (greyscale when not found, color + glow when found), the item name
*   below the icon, and an optional counter badge in the bottom-right corner for stackable items.
*/
class ItemIconWidget : public QWidget
{
    Q_OBJECT

public:

    QString DisplayName;                // Human-readable name shown under the icon.
    QSet<uint32_t> LookupKeys;
    EGameIcon Icon = EGameIcon::none;   // The EGameIcon associated with this widget (display + primary lookup key).
    bool IsCounter = false;             // Whether the widget shows a counter badge.
    int Count = 0;                      // Current counter value when IsCounter is true.
    bool Found = false;                 // True once at least one matching item has been collected.

    QStringList LocationsFound;         // Scenes where the item was tracked, used for the tooltip.
    int Game = -1;                      // OOT_GAME / MM_GAME associated with the widget's page (used as a hint for the detail panel).
    bool IsStartingItem = false;        // Set by ProgressionTab::ApplySettings when the player starts the run with this item.

private:

    QLabel* IconLabel = nullptr;        // QLabel that hosts the QPixmap.
    QLabel* NameLabel = nullptr;        // Label showing DisplayName.
    QLabel* CountBadge = nullptr;       // Overlay label used for the counter badge.
    bool VisualLoaded = false;          // Lazy-loading guard: gates RefreshVisual until the widget is first shown.

public:

    /*
    *   Constructs the icon widget for the given EGameIcon. The pixmap is NOT
    *   loaded here — RefreshVisual defers its work until showEvent fires the
    *   first time the widget is actually mapped to the screen, so widgets on
    *   non-active pages do not pay the disk-load cost at startup.
    *
    *   @param IconValue       The EGameIcon used to fetch the pixmap from the GameIcons singleton.
    *   @param DisplayName     The human-readable item name shown under the icon.
    *   @param IsCounter       Whether to show a counter badge for stackable items.
    *   @param Parent          The Qt parent.
    */
    ItemIconWidget(EGameIcon IconValue, const QString& DisplayName, bool IsCounter = false, QWidget* Parent = nullptr);

    /*
    *   Mark the item as found, optionally update its counter and refresh the visual state.
    *
    *   @param Game      The game the reported object belongs to (used to resolve the scene name).
    *   @param Object    The reported object (used to record the scene location). Can be nullptr.
    */
    void MarkFound(int Game, const ObjectInfo* Object = nullptr);

    /*
    *   Decrements the counter, mark the item as not found if count reaches 0 and refresh the visual state.
    *
    *   @param Game      The game the reported object belongs to (used to resolve the scene name).
    *   @param Object    The reported object (used to record the scene location). Can be nullptr.
    */
    void MarkNotFound(int Game, const ObjectInfo* Object = nullptr);

    /*
    *   Reset the widget back to its initial unfound state.
    */
    void ResetFound();

    /*
    *   Refresh the icon visuals (greyscale / color, glow effect, count badge text and tooltip).
    */
    void RefreshVisual();

signals:

    /*
    *   Emitted when the user clicks on the icon, used by the detail panel.
    */
    void Selected(ItemIconWidget* Widget);

protected:

    /*
    *   Qt override. Emits the Selected signal so the detail panel can update
    *   when the user left-clicks the icon, then forwards the event to the base class.
    *
    *   @param Event    The Qt mouse event.
    */
    void mousePressEvent(QMouseEvent* Event) override;

    /*
    *   Qt override. Forwards the resize event to the base class so the icon and
    *   labels stay laid out correctly when the widget is resized.
    *
    *   @param Event    The Qt resize event.
    */
    void resizeEvent(QResizeEvent* Event) override;

    /*
    *   Qt override. Lazy-loads the icon pixmap the first time the widget is
    *   actually mapped to the screen, so widgets on inactive pages do not pay
    *   the disk-load cost at startup. Subsequent state updates (Found / Count)
    *   that ran while VisualLoaded was false are caught up by RefreshVisual.
    *
    *   @param Event    The Qt show event.
    */
    void showEvent(QShowEvent* Event) override;

};


/*
*   Dashboard tab that visually tracks the items collected so far across both games.
*   Provides three sub-tabs (OoT, MM, Souls) and a right-side detail panel that shows
*   information about the currently selected item.
*/
class ProgressionTab : public QWidget
{
    Q_OBJECT

public:

    OoTMMComboTracker* WinOwner = nullptr;     // The owning main window.

public:

    /*
    *   Per-game registry: maps an EGameIcon to every widget that displays it. Multiple widgets
    *   may share the same icon (e.g. all 12 ocarina songs use EGameIcon::song); the matching
    *   entry is then resolved against the item name using the widget's LookupKey. The flat
    *   list is used as a fallback when Item->RenderType cannot be located in the hash (e.g.
    *   items with RenderType=none).
    */
    typedef struct GameProgData
    {
        QHash<EGameIcon, QList<ItemIconWidget*>> ByIcon;   // EGameIcon -> widgets sharing it.
        QList<ItemIconWidget*> All;                        // Flat list of every widget on the page.
    } GameProgData;

    /*
    *   Per-section layout bookkeeping used to repack the grid after a settings
    *   change disables widgets. The widgets are stored in declaration order so
    *   the repack can place visible ones sequentially without leaving holes.
    *   Header and GridHost are tracked so the whole section can be hidden when
    *   every one of its widgets is disabled. PageIndex maps the section back
    *   to its hosting QStackedWidget page so the parent tab can also be hidden
    *   when all of its sections are empty.
    */
    typedef struct GridSection
    {
        QGridLayout* Grid;
        QList<ItemIconWidget*> Widgets;
        QLabel* Header;
        QWidget* GridHost;
        int PageIndex;
    } GridSection;

private:

    QTabBar* SubTabBar = nullptr;              // Top sub-tab selector (OoT / MM / Souls).
    QStackedWidget* PageStack = nullptr;       // Stack of per-game pages.

    GameProgData OoTData;                      // Widgets registered on the OoT page.
    GameProgData MMData;                       // Widgets registered on the MM page.
    GameProgData SoulsData;                    // Widgets registered on the Souls page (spans both games).
    GameProgData CollectiblesData;             // Widgets registered on the Collectibles page (spans both games).
    QList<GridSection> Sections;               // Per-section grid bookkeeping (repacking after settings).

    static ProgressionTab* sInstance;          // Single living dashboard, used by callers that cannot reach the tracker.

    const Settings* RomSettings = nullptr;     // Borrowed pointer to the active spoiler-derived settings; drives shared / disabled / starting-item handling.

    // Detail panel widgets.
    QLabel* DetailIcon = nullptr;
    QLabel* DetailName = nullptr;
    QLabel* DetailStatus = nullptr;
    QLabel* DetailLocationsHeader = nullptr;
    QPushButton* DetailExpandToggle = nullptr;     // Expand-all / Collapse-all switch above the location tree.
    QTreeWidget* DetailLocations = nullptr;
    QLabel* DetailCount = nullptr;
    ItemIconWidget* CurrentDetailWidget = nullptr; // Widget currently displayed by ShowDetailFor.
    bool DetailLocationsExpanded = true;           // Tracks the current state of the expand/collapse toggle.

public:

    /*
    *   Constructs the progression dashboard.
    *
    *   @param Owner    The owning main window.
    *   @param Parent   The Qt parent widget.
    */
    explicit ProgressionTab(OoTMMComboTracker* Owner, QWidget* Parent = nullptr);

    /*
    *   Default destructor. Clears the static instance pointer if it points to this tab.
    */
    ~ProgressionTab();

    /*
    *   Access the active ProgressionTab from anywhere in the codebase. Returns nullptr
    *   when the dashboard has not been instantiated yet (e.g. early startup) so callers
    *   must check before dereferencing.
    *
    *   @return The single living ProgressionTab, or nullptr.
    */
    static ProgressionTab* GetInstance();

    /*
    *   Notify the dashboard that an item has been collected. Looks up the matching ItemIconWidget
    *   and marks / unmark it as found (or increments / decrements its counter if it is a stackable item).
    *
    *   @param Game         The game the object belongs to (OOT_GAME or MM_GAME).
    *   @param Object       The object in which the item was found.
    *   @param Item         The item that was found.
    *   @param IsAddOp      The marking operation to execute.
    */
    void OnItemFound(int Game, ObjectInfo* Object, const ItemInfo* Item, bool IsAddOp);

    /*
    *   Notify the dashboard that the user manually toggled the force state of an object.
    *   Forwards to OnItemFound as an add or remove depending on the new ObjectState
    *   (Forced -> add, Hidden -> remove, Collected -> ignored), then refreshes the
    *   currently shown detail panel so the location tree reflects the change.
    *
    *   @param Game      The game the object belongs to (OOT_GAME or MM_GAME).
    *   @param Object    The object whose force state changed.
    */
    void OnObjectForceStateChanged(int Game, ObjectInfo* Object);

    /*
    *   Walk every scene of both games and replay OnItemFound for every Object whose
    *   Status is not Hidden and whose Item is set. The dashboard is reset first so
    *   removals (manual unforce, fresh save load) are reflected too. Used both after
    *   loading a save file and after the user toggles an object's force state.
    */
    void RebuildFromSceneObjects();

    /*
    *   Reset the dashboard back to its empty state. Called when the tracker is reset.
    */
    void ResetProgress();

    /*
    *   Refresh the detail panel currently shown (if any). Called when the
    *   "Reveal Uncollected Items" option is toggled so the location tree
    *   reflects the new visibility.
    */
    void RefreshCurrentDetail();

    /*
    *   Sync the dashboard with the spoiler-derived settings: hide widgets whose
    *   LookupKeys land in DisabledItemIDs and store the reference for later
    *   shared / starting-item logic. Starting-item pre-marking is performed by
    *   RebuildFromSceneObjects so it survives the reset-and-replay cycle.
    *
    *   @param NewRomSettings   Borrowed pointer to the active settings (must outlive the call).
    */
    void ApplySettings(const Settings* NewRomSettings);

private:

    /*
    *   Build the OoT, MM and Souls pages and populate their item registries.
    */
    void BuildPages();

    /*
    *   Build a single page from a static array of sections.
    *
    *   @param Sections     Pointer to the sections array.
    *   @param SectionCount Number of sections in the array.
    *   @param Target       The progression registry to populate with the created widgets.
    *   @param Game         The game these items belong to (for the detail panel).
    *   @param PageIndex    Index of the page in the QStackedWidget / QTabBar. Stored
    *                       on every GridSection so empty pages can hide their tab.
    *
    *   @return The built page widget.
    */
    QWidget* BuildPage(const ProgSection* Sections, size_t SectionCount, GameProgData& Target, int Game, int PageIndex);

    /*
    *   Build the right-side detail panel.
    *
    *   @return The built detail panel widget.
    */
    QWidget* BuildDetailPanel();

    /*
    *   Update the detail panel with the information of the given item widget.
    *
    *   @param Widget    The item widget that was selected.
    */
    void ShowDetailFor(ItemIconWidget* Widget);

    /*
    *   Build the location tree of the detail panel for the given widget. Top level
    *   items are scenes and leaves are individual objects, mirroring the visual style
    *   of MapTab's object list. When the "Reveal Uncollected Items" option is on,
    *   uncollected matching objects are also listed (greyed out).
    *
    *   @param Widget    The widget the tree describes.
    */
    void BuildLocationTree(ItemIconWidget* Widget);

    /*
    *   Forward a click on a location leaf to the main window so it switches to the
    *   matching game tab and centers the view on the underlying object.
    *
    *   @param Item      The clicked tree item.
    *   @param Column    Unused; kept for the QTreeWidget::itemClicked signature.
    */
    void OnLocationClicked(QTreeWidgetItem* Item, int Column);

    /*
    *   Toggle the location tree between fully expanded and fully collapsed and
    *   update the toggle button label accordingly.
    */
    void OnToggleLocationsExpansion();

    /*
    *   Detach every widget from its section's grid layout and re-add only the
    *   non-explicitly-hidden ones, packing them sequentially. Called by
    *   ApplySettings so disabled items do not leave a gap in the grid.
    */
    void RepackVisibleWidgets();

    /*
    *   Hide the section header + grid host of every section that has no visible
    *   widget left, and hide the SubTabBar tab of every page whose sections are
    *   all empty. If the active tab is hidden, the selection is moved to the
    *   first still-visible tab so the dashboard never shows a blank page.
    */
    void UpdateSectionAndTabVisibility();

    /*
    *   Test whether the given item would be matched to the given widget by the
    *   same logic OnItemFound uses (icon hash + LookupKey disambiguation, with a
    *   name-only fallback). Souls widgets accept items from any game.
    *
    *   @param Widget    The widget to test against.
    *   @param Game      The game the item was reported in.
    *   @param Item      The item to test.
    *
    *   @return True if the item would be associated to the widget.
    */
    static bool ItemMatchesWidget(const ItemIconWidget* Widget, int Game, const ItemInfo* Item);

    /*
    *   Normalize the given item name by stripping the trailing "(OoT)" / "(MM)" suffix and
    *   collapsing it to lowercase so LookupKey substring tests are tolerant to variations.
    *
    *   @param Name    The raw item name from Items.cpp.
    *
    *   @return The normalized item name.
    */
    static QString NormalizeItemName(const QString& Name);

    /*
    *   Locate the widget that should be marked when an item is collected, by matching
    *   Item->RenderType against the registry's icon hash and then disambiguating by LookupKey.
    *
    *   @param Data         The game registry to search in.
    *   @param Icon         The EGameIcon coming from Item->RenderType.
    *   @param Normalized   The normalized item name (used for LookupKey disambiguation).
    *
    *   @return The matching widget, or nullptr when no candidate matches.
    */
    //static ItemIconWidget* FindByIcon(const GameProgData& Data, EGameIcon Icon, const QString& Normalized);
    static ItemIconWidget* FindByIcon(const GameProgData& Data, const ItemInfo* Item);

    /*
    *   Fallback lookup that scans every widget of a page and matches by LookupKey only.
    *   Used when Item->RenderType does not yield a hash hit (e.g. RenderType=none).
    *
    *   @param Data         The game registry to search in.
    *   @param Normalized   The normalized item name to test against each widget's LookupKey.
    *
    *   @return The matching widget, or nullptr when no candidate matches.
    */
    //static ItemIconWidget* FindByLookupKey(const GameProgData& Data, const QString& Normalized);
    static ItemIconWidget* FindByLookupKey(const GameProgData& Data, const ItemInfo* Item);

    /*
    *   Collect every ItemIconWidget across the four registries whose LookupKeys
    *   contain Item->ItemID. The order mirrors BuildPage's iteration of the
    *   static ProgSection arrays, which is the declaration order — required by
    *   the progressive-item logic (sword stages, shield stages, ...) to advance
    *   to the next stage when an earlier one is already found.
    *
    *   @param Item     The item to look up.
    *
    *   @return Every matching widget in declaration order. Empty when none.
    */
    QList<ItemIconWidget*> FindAllMatchingWidgets(const ItemInfo* Item) const;
};
