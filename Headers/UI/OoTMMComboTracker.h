#pragma once

#include <QtWidgets/QMainWindow>
#include <QTabWidget>
#include <QLabel>
#include <QStringList>
#include <QWidget>
#include <QComboBox>
#include "ui_OoTMMComboTracker.h"
#include "UI/Settings.h"
#include "UI/EntranceTable.h"
#include "UI/SceneEntrance.h"
#include "LogTab.h"
#include "GameTab.h"


/*
*   Compact clickable status indicator (colored dot + label) used inside the
*   bottom QStatusBar. Active state controls dot color (green / red) and label
*   text. A click emits Clicked() — the owner decides how to react and may
*   call SetActive() to update the visual.
*/
class StatusPill : public QWidget
{
    Q_OBJECT

public:

    /*
    *   Construct the status pill with the given active and inactive display texts.
    *
    *   @param ActiveText      The label shown when the pill is in the active state.
    *   @param InactiveText    The label shown when the pill is in the inactive state.
    *   @param Parent          The optional parent widget.
    */
    StatusPill(const QString& ActiveText, const QString& InactiveText, QWidget* Parent = nullptr);

    /* Set the active visual state. */
    void SetActive(bool NewActive);

    /* Tell whether the pill is currently displaying its active visual. */
    bool IsActive() const { return this->Active; }

    /*
    *   Return the preferred size for this pill widget.
    *
    *   @return The recommended size.
    */
    QSize sizeHint() const override;

signals:
    /* Emitted when the user clicks the pill. */
    void Clicked();

protected:

    /*
    *   Paint the colored dot and the matching text label.
    *
    *   @param Event    The paint event triggering the redraw.
    */
    void paintEvent(QPaintEvent* Event) override;

    /*
    *   Emit the Clicked() signal when the user presses the pill.
    *
    *   @param Event    The mouse press event.
    */
    void mousePressEvent(QMouseEvent* Event) override;

    /*
    *   Set the hover state to true and trigger a repaint.
    *
    *   @param Event    The enter event.
    */
    void enterEvent(QEnterEvent* Event) override;

    /*
    *   Clear the hover state and trigger a repaint.
    *
    *   @param Event    The leave event.
    */
    void leaveEvent(QEvent* Event) override;

private:
    QString ActiveText;     // Label shown when Active == true.
    QString InactiveText;   // Label shown when Active == false.
    bool Active = false;    // Current state.
    bool Hovered = false;   // Hover state, used for subtle highlight.
};


/*
*   This class represent the main tracker window and is handling the core program.
*/
class ProgressionTab;

class OoTMMComboTracker : public QMainWindow
{
    Q_OBJECT

public:
    Settings ROMSettings;            // The settings used when the ROM was built.
    ProgressionTab* ProgTab = nullptr; // Items collection dashboard.

private:

    Ui::OoTMMComboTrackerClass ui;   // The main window UI.
    QTabWidget* TabWidget;           // The main tab widget.
    LogTab* Log;                     // Log / Launch tab.
    GameTab* OoTTab;                 // Ocarina of Time game tab.
    GameTab* MMTab;                  // Majora's Mask game tab.
    EntranceTab* EntTab;             // Entrance tab.
    QStringList RecentFiles;         // The list of the last opened files.
    byte MaxRecentFiles = 5;         // The maximum number of recent files to show (auto save not counted).
    QList<QAction*> RecentActions;   // The actual recent files options.

    QLabel* OoTTabLabel = nullptr;            // Counter label displayed inside the OoT tab (above its progress bar).
    QLabel* MMTabLabel = nullptr;             // Counter label displayed inside the MM tab (above its progress bar).
    QWidget* OoTTabProgress = nullptr;        // Custom-painted progress line under the OoT tab counter (TabProgressLine).
    QWidget* MMTabProgress = nullptr;         // Custom-painted progress line under the MM tab counter (TabProgressLine).
    QLabel* GlobalCounter = nullptr;          // Aggregated "X/Y" value label displayed in the tab bar corner.
    QWidget* GlobalProgress = nullptr;        // Dual-segment (OoT blue + MM violet) progress bar next to the global counter.
    QComboBox* WorldSelector = nullptr;       // Multiworld world selector in the tab bar corner (hidden when single-world).
    StatusPill* TrackingPill = nullptr;       // Bottom status pill: tracking on/off (clickable to start/stop).
    StatusPill* AutoSavePill = nullptr;       // Bottom status pill: auto-save on/off (clickable to toggle).
    StatusPill* RevealPill = nullptr;         // Bottom status pill: reveal uncollected items on/off (clickable to toggle).
    QLabel* LastActivityLabel = nullptr;      // Bottom-right label: most recent collected item / parsed entrance.

public:

    /*
    *   Constructs the main tracker window.
    *
    *   @param Parent   The potential parent to attach this class to.
    */
    OoTMMComboTracker(QWidget *parent = nullptr);

    /*
    *   Default destructor.
    */
    ~OoTMMComboTracker();

    /*
    *   Apply the accent color theme of the given game to the main window.
    *
    *   @param GameID    The game whose accent color should be applied.
    */
    void ApplyGameTheme(int GameID);

    /*
    *   Shows the about dialog message.
    */
    void ShowAboutDialog();

    /*
    *   Creates the given path if not exist.
    * 
    *   @param PathToCreate   The path to create.
    */
    void CreatePath(QString PathToCreate);

#pragma region Object related

public:

    /*
    *   Refresh the game tab name corresponding to the given ID.
    *
    *   @param TabID       The game tab to refresh.
    */
    void UpdateTabNameText(int TabID);

    /*
    *   Refresh the collected object map visibility.
    *
    *   @param NewValue       The new visibility state.
    */
    void UpdateObjectMapVisibility(bool NewValue);

    /*
    *   Refresh the collected object list visibility.
    *
    *   @param NewValue       The new visibility state.
    */
    void UpdateObjectListVisibility(bool NewValue);

    /*
    *   Toggle whether uncollected objects display their item label or "???".
    *
    *   @param NewValue       True to show the item label, false to display "???" instead.
    */
    void UpdateRevealUncollectedItems(bool NewValue);

private:

    /*
    *   Update object visibility on both game tabs.
    */
    void UpdateObjectVisibilityForAllGames();

public:

    /*
    *   Switch to the matching game tab, open the scene that renders the object
    *   and center the viewport on it. Used by the progression dashboard to jump
    *   from an item entry to its in-game location.
    *
    *   @param Game     The game the object belongs to.
    *   @param Object   The object to focus on.
    */
    void NavigateToObject(int Game, ObjectInfo* Object);

public slots:

    /*
    *   Updates the given object status and dispatch it to the corresponding scene view.
    *   The map / progression update is gated by the game mode and the event source
    *   (see the routing table in the implementation).
    *
    *   @param Game           The game the object belong to.
    *   @param ObjectFound    The object in which the item has been found.
    *   @param ItemFound      The item that has been found.
    *   @param Source         The collection path that produced the event (hook / network).
    *   @param FromWorld      The world that collected the item, or -1 when unknown.
    *   @param ToWorld        The world the item is destined to, or -1 when unknown.
    */
    void UpdateTrackedObject(int Game, ObjectInfo* ObjectFound, const ItemInfo* ItemFound, ItemSource Source, int FromWorld, int ToWorld);

    /*
    *   Switch the active world and refresh every view (maps, entrances, progression).
    *   Connected to the multiworld world selector.
    *
    *   @param Index    The 0-based world index selected in the combo box.
    */
    void OnWorldSelected(int Index);

#pragma endregion

#pragma region Entrance related


public slots:

    /*
    *   Updates the given entrance status and dispatch it to the corresponding entrance scene view.
    *
    *   @param OutEntrance      The out entrance to update.
    *   @param InEntrance       The in entrance to update.
    */
    void UpdateTrackedEntrance(SceneEntranceUpdate OutEntrance, SceneEntranceUpdate InEntrance);

#pragma endregion

#pragma region Saving / Loading / Menu

public:

    /*
    *   Refresh the recent files entries shown in the main menu.
    */
    void UpdateRecentFiles();

    /*
    *   Add the given file path at the top of the recent files list.
    *
    *   @param filePath    The file path to add as the most recent one.
    */
    void AddRecentFile(const QString& filePath);

    /*
    *   Update the tracking menu action with the given label and icon.
    *
    *   @param NewState    The new label of the tracking action.
    *   @param NewIcon     The new icon to display next to the action.
    */
    void UpdateTrackingState(QString NewState, QIcon NewIcon);

    /*
    *   Apply the current ROM settings to the tab visibility and filter state.
    */
    void ApplySettings();

    /*
    *   Resfresh the tracker game tabs.
    */
    void RefreshTracker();

    /*
    *   Rebuild / show / hide the multiworld world selector based on the current world count.
    *   Hidden when there is a single world; populated with "World N" entries otherwise.
    */
    void RefreshWorldSelector();

    /*
    *   Loads the game scenes using the giving tracking session.
    * 
    *   @param FilePath       The file path to the tracking session to load.
    */
    void LoadGameScenes(QString FilePath);

    /*
    *   Loads the game spoiler log.
    *
    *   @param FilePath       The file path to the spoiler log to load.
    */
    void LoadGameSpoiler(QString FilePath);

    /*
    *   Parse a single world's location block and fill that world's scene objects.
    *   The block must already be dedented to the single-world layout (two leading
    *   spaces per location). In multiworld each item carries a "Player N " prefix
    *   identifying its destination world; it is stripped for the item lookup and
    *   stored in ObjectInfo::TargetWorld.
    *
    *   @param LocationBlock    The dedented location text for this world.
    *   @param WorldIndex       The 0-based world index whose arrays must be filled.
    *   @param IsMultiworld     True when the spoiler is a multiworld seed (enables the
    *                           "Player N " prefix parsing).
    */
    void ParseWorldLocations(const QString& LocationBlock, size_t WorldIndex, bool IsMultiworld);

    /*
    *   Assign the given item to the object whose Location matches ObjName inside a single scene.
    *   Mirrors the item into the object's RenderScene counterpart when it differs, and keeps
    *   scanning the scene after a placeholder (ObjectType::none) match so the real rendered
    *   object is filled too.
    *
    *   @param WorldIndex       The 0-based world index whose arrays must be filled.
    *   @param Game             The game the scene belongs to (OOT_GAME / MM_GAME).
    *   @param SceneID          The scene to search the object in.
    *   @param ObjName          The full spoiler location string of the object (globally unique).
    *   @param Item             The item to assign to the object.
    *   @param TargetWorld      The multiworld destination world (1-based).
    *
    *   @return <b>True</b> if at least one matching object was found and assigned, <b>false</b> otherwise.
    */
    bool AssignSpoilerObjectInScene(size_t WorldIndex, uint32_t Game, uint32_t SceneID, const char* ObjName, const ItemInfo* Item, uint8_t TargetWorld);

    /*
    *   Cross-scene fallback for entrance shuffle: scan every scene of the given game for an object
    *   whose Location matches ObjName. Needed when a randomized entrance lists a location under a
    *   different scene header than the one it natively belongs to (moved grottos, relocated boss
    *   lairs, ...). The Location string is globally unique so the first match is the real home.
    *
    *   @param WorldIndex       The 0-based world index whose arrays must be filled.
    *   @param Game             The game to scan (OOT_GAME / MM_GAME).
    *   @param ObjName          The full spoiler location string of the object (globally unique).
    *   @param Item             The item to assign to the object.
    *   @param TargetWorld      The multiworld destination world (1-based).
    *
    *   @return <b>True</b> if the object was found in some scene and assigned, <b>false</b> otherwise.
    */
    bool AssignSpoilerObjectAnyScene(size_t WorldIndex, uint32_t Game, const char* ObjName, const ItemInfo* Item, uint8_t TargetWorld);

#pragma endregion

};
