#pragma once

#include <QtWidgets/QMainWindow>
#include <QTabWidget>
#include <QStringList>
#include "ui_OoTMMComboTracker.h"
#include "LogTab.h"
#include "GameTab.h"


/*
*   This class represent the main tracker window and is handling the core program.
*/
class OoTMMComboTracker : public QMainWindow
{
    Q_OBJECT

private:

    Ui::OoTMMComboTrackerClass ui;   // The main window UI.
    QTabWidget* TabWidget;           // The main tab widget.
    LogTab* Log;                     // Log / Launch tab.
    GameTab* OoTTab;                 // Ocarina of Time game tab.
    GameTab* MMTab;                  // Majora's Mask game tab.
    QStringList RecentFiles;         // The list of the last opened files.
    byte MaxRecentFiles = 5;         // The maximum number of recent files to show (auto save not counted).
    QList<QAction*> RecentActions;   // The actual recent files options.


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

#pragma region Saving / Loading / Menu

public:

    void UpdateRecentFiles();
    void AddRecentFile(const QString& filePath);
    void UpdateTrackingState(QString NewState, QIcon NewIcon);

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
