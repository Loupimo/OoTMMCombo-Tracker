#pragma once

#include <QMainWindow>
#include <QGraphicsView>
#include <QVBoxLayout>
#include "MapTab.h"
#include "Combo/Objects.h
#include "UI/SceneRenderer.h"


class MainRegionTab : public QTabWidget
{
    Q_OBJECT

#pragma region Attributes

public:
    QHBoxLayout* MainLayout;

    std::vector<MapTab> Overworld;   // The overworld tab
    std::vector<MapTab> Temples;     // The temples tab
    std::vector<MapTab> Houses;      // The houses tab
    std::vector<MapTab> Grottos;     // The grottos tab

    int PrevTab = -1;

#pragma endregion

public:

    MainRegionTab();
    ~MainRegionTab();

    /*
    *   Fills the desired map tab with the given scenes.
    *
    *   @param TabIndex    The tab index to unload
    */
    void PrepareMaps(std::vector<MapTab>* DestMap, size_t NumOfMaps, SceneInfo * Scenes);

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

    int GameID;                // The game ID this tab refers to
    const char* TabName;       // The tab name. Should correspond to the game it refers to
    MainRegionTab MainRegion;  // This tab widget will regroup all possible main region for a game

#pragma endregion

public:
    GameTab(int GameID, QWidget* parent = nullptr);
    ~GameTab();
};