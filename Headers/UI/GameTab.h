#pragma once

#include <QMainWindow>
#include <QGraphicsView>
#include <QVBoxLayout>
#include "MapTab.h"
#include "UI/SceneRenderer.h"


class MainRegionTab : public QTabWidget
{
    Q_OBJECT

#pragma region Attributes

public:
    QHBoxLayout* MainLayout;

    MapTab* Overworld;   // The overworld tab
    MapTab* Temples;     // The temples tab
    MapTab* Houses;      // The houses tab
    MapTab* Grottos;     // The grottos tab

    int PrevTab = -1;

#pragma endregion

public:

    MainRegionTab(int GameID);
    ~MainRegionTab();

    /*
    *   Unloads the tab corresponding to the given index
    *
    *   @param TabIndex    The tab index to unload
    */
    void UnloadTab(int TabIndex);

public slots:

    /*
    *   Loads the tab corresponding to the given index.
    *
    *   Note: This should be called and connected to the currentChanged signal.
    *
    *   @param TabIndex    The tab index to unload
    */
    void LoadTab(int TabIndex);
};


class GameTab : public QWidget
{
    Q_OBJECT

#pragma region Attributes

public:
    QHBoxLayout* MainLayout;
    QGraphicsView* View;
    QPixmap* Map;

    int GameID;                 // The game ID this tab refers to
    const char* TabName;        // The tab name. Should correspond to the game it refers to
    MainRegionTab* MainRegion;  // This tab widget will regroup all possible main region for a game

#pragma endregion

public:
    GameTab(int GameID, QWidget* parent = nullptr);
    ~GameTab();
};