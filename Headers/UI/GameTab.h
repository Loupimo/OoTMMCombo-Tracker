#pragma once

#include <QMainWindow>
#include <QGraphicsView>
#include <QVBoxLayout>
#include <QFile>
#include "MapTab.h"
#include "UI/SceneRenderer.h"

class OoTMMComboTracker;

class GameTab : public QWidget
{
    Q_OBJECT

#pragma region Attributes

public:
    QHBoxLayout* MainLayout;

    int GameID;                 // The game ID this tab refers to
    const char* TabName;        // The tab name. Should correspond to the game it refers to
    MapTab* GameMaps;           // This tab widget will regroup all possible main regions and maps for a game
    
    OoTMMComboTracker* Owner = nullptr;

    int FoundObjects = 0;
    int TotalObjects = 0;

#pragma endregion

public:
    GameTab(int GameID, QWidget* parent = nullptr);
    ~GameTab();

    void RefreshTabCountText();
    void ItemFound(ObjectInfo* Object, const ItemInfo* Item);
    void LoadGameTab();
    void RefreshGameTab();

    static void SaveGameScenes(QString FilePath);
    static void LoadGameScenes(QString FilePath);
    static void LoadGameSpoiler(QString FilePath);

protected:

    static void SaveOoTScenes(QFile* SaveFile);
    static void SaveMMScenes(QFile* SaveFile);
};