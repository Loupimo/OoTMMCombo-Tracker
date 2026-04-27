#pragma once

#include <QWidget>
#include <QLabel>
#include <QHash>
#include <QList>
#include <QTabBar>
#include <QStackedWidget>
#include <QScrollArea>
#include "Combo/Items.h"
#include "Combo/Objects.h"

class OoTMMComboTracker;
class ProgressionTab;

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
    QString IconPath;                   // Resolved icon path (falls back to Grass.png when missing).
    bool IsCounter = false;             // Whether the widget shows a counter badge.
    int Count = 0;                      // Current counter value when IsCounter is true.
    bool Found = false;                 // True once at least one matching item has been collected.

    QStringList LocationsFound;         // Scenes where the item was tracked, used for the tooltip.
    int Game = -1;                      // OOT_GAME / MM_GAME associated with this widget (for the detail panel).
    const ObjectInfo* LastObject = nullptr; // Last reported object (if any) so the detail panel can show its location.

private:

    QLabel* IconLabel = nullptr;        // QLabel that hosts the QPixmap.
    QLabel* NameLabel = nullptr;        // Label showing DisplayName.
    QLabel* CountBadge = nullptr;       // Overlay label used for the counter badge.

public:

    /*
    *   Constructs the icon widget with the given icon path and display name.
    *
    *   @param IconPath        The path to the icon image (relative to the working directory).
    *   @param DisplayName     The human-readable item name shown under the icon.
    *   @param IsCounter       Whether to show a counter badge for stackable items.
    *   @param Parent          The Qt parent.
    */
    ItemIconWidget(const QString& IconPath, const QString& DisplayName, bool IsCounter = false, QWidget* Parent = nullptr);

    /*
    *   Mark the item as found, optionally update its counter and refresh the visual state.
    *
    *   @param Object    The reported object (used to record the scene location). Can be nullptr.
    */
    void MarkFound(const ObjectInfo* Object = nullptr);

    /*
    *   Reset the widget back to its initial unfound state.
    */
    void ResetFound();

signals:

    /*
    *   Emitted when the user clicks on the icon, used by the detail panel.
    */
    void Selected(ItemIconWidget* Widget);

protected:

    void mousePressEvent(QMouseEvent* Event) override;
    void resizeEvent(QResizeEvent* Event) override;

private:

    /*
    *   Refresh the icon visuals (greyscale / color, glow effect, count badge text and tooltip).
    */
    void RefreshVisual();
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

private:

    QTabBar* SubTabBar = nullptr;              // Top sub-tab selector (OoT / MM / Souls).
    QStackedWidget* PageStack = nullptr;       // Stack of per-game pages.

    // Per-game item registry: maps the lowercased item base name (without the "(OoT)" / "(MM)" suffix)
    // to its ItemIconWidget. Multiple Items.cpp entries can map to the same widget (e.g. progressive
    // hookshot, multiple rupee tiers).
    QHash<QString, ItemIconWidget*> OoTItemMap;
    QHash<QString, ItemIconWidget*> MMItemMap;
    QHash<QString, ItemIconWidget*> SoulsItemMap;

    // Detail panel widgets.
    QLabel* DetailIcon = nullptr;
    QLabel* DetailName = nullptr;
    QLabel* DetailStatus = nullptr;
    QLabel* DetailLocation = nullptr;
    QLabel* DetailCount = nullptr;

public:

    /*
    *   Constructs the progression dashboard.
    *
    *   @param Owner    The owning main window.
    *   @param Parent   The Qt parent widget.
    */
    explicit ProgressionTab(OoTMMComboTracker* Owner, QWidget* Parent = nullptr);

    /*
    *   Notify the dashboard that an item has been collected. Looks up the matching ItemIconWidget
    *   and marks it as found (or increments its counter if it is a stackable item).
    *
    *   @param Game      The game the object belongs to (OOT_GAME or MM_GAME).
    *   @param Object    The object in which the item was found.
    *   @param Item      The item that was found.
    */
    void OnItemFound(int Game, ObjectInfo* Object, const ItemInfo* Item);

    /*
    *   Reset the dashboard back to its empty state. Called when the tracker is reset.
    */
    void ResetProgress();

private:

    /*
    *   Build the OoT, MM and Souls pages and populate their item registries.
    */
    void BuildPages();

    /*
    *   Build a single page composed of multiple sections.
    *
    *   @param Sections     Vector of (sectionTitle, items[]) pairs.
    *   @param TargetMap    The hash map to populate with the created ItemIconWidgets.
    *   @param Game         The game these items belong to (for the detail panel).
    *
    *   @return The built page widget.
    */
    QWidget* BuildPage(const QList<QPair<QString, QList<QPair<QString, QString>>>>& Sections,
                       QHash<QString, ItemIconWidget*>& TargetMap, int Game);

    /*
    *   Build the souls page using a flat list of soul item names mapped to their game.
    *
    *   @return The built souls page widget.
    */
    QWidget* BuildSoulsPage();

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
    *   Resolve the icon path for the given item name. Falls back to Grass.png when no match exists.
    *
    *   @param Name    The item display name to resolve.
    *
    *   @return The resolved icon path.
    */
    static QString ResolveIconPath(const QString& Name);

    /*
    *   Normalize the given item name by stripping the trailing "(OoT)" / "(MM)" suffix and
    *   collapsing it to lowercase so lookups are tolerant to variations.
    *
    *   @param Name    The raw item name from Items.cpp.
    *
    *   @return The normalized lookup key.
    */
    static QString NormalizeItemName(const QString& Name);
};
