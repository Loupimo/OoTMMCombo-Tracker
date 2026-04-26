#pragma once

#include <QtWidgets/QMainWindow>
#include <QTabWidget>
#include <QLabel>
#include <QStringList>
#include "ui_OoTMMComboTracker.h"
#include "UI/Settings.h"
#include "UI/EntranceTable.h"
#include "UI/SceneEntrance.h"
#include "LogTab.h"
#include "GameTab.h"


/*
*   This class represent the main tracker window and is handling the core program.
*/
class OoTMMComboTracker : public QMainWindow
{
    Q_OBJECT

public:
    Settings ROMSettings;            // The settings used when the ROM was built.

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

public slots:

    /*
    *   Updates the given object status and dispatch it to the corresponding scene view.
    *
    *   @param Game           The game the object belong to.
    *   @param ObjectFound    The object in which the item has been found.
    *   @param ItemFound      The item that has been found.
    */
    void UpdateTrackedObject(int Game, ObjectInfo* ObjectFound, const ItemInfo* ItemFound);

#pragma endregion

#pragma region Entrance related


public slots:

    /*
    *   Updates the given entrance status and dispatch it to the corresponding entrance scene view.
    *
    *   @param OutEntrance      The out entrance to update.
    *   @param InEntrance       The in entrance to update.
    */
    void UpdateTrackedEntrance(SceneEntranceUpdate* OutEntrance, SceneEntranceUpdate* InEntrance);

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

#pragma endregion

};
