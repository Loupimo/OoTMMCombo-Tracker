#pragma once

#include <QtWidgets/QMainWindow>
#include <QTabWidget>
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

    Ui::OoTMMComboTrackerClass ui;  // The main window UI.
    QTabWidget* TabWidget;          // The main tab widget.
    LogTab* Log;                    // Log / Launch tab.
    GameTab* OoTTab;                // Ocarina of Time game tab.
    GameTab* MMTab;                 // Majora's Mask game tab.

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

#pragma region Object related

public:

    /*
    *   Refresh the game tab name corresponding to the given ID.
    *
    *   @param TabID       The game tab to refresh.
   */
    void UpdateTabNameText(int TabID);

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

#pragma region Saving / Loading

public:

    /*
    *   Loads the game scenes using the giving tracking session.
    * 
    *   @param FilePath       The file path to the tracking session to load.
    */
    void LoadGameScenes(QString FilePath);

    /*
    *   Loads the game spoiler log.
    */
    void LoadGameSpoiler();

#pragma endregion

};
