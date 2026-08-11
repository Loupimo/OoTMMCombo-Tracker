#pragma once

#include <QMainWindow>
#include <QGraphicsView>
#include <QVBoxLayout>
#include "MapTab.h"
#include "SceneRenderer.h"

class OoTMMComboTracker;
class Settings;

/*
*   This class is responsible of holding all maps of a specific game.
*/
class GameTab : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QString AccentColor READ GetAccentColor NOTIFY AccentColorChanged)

#pragma region Attributes

public:
    QHBoxLayout* MainLayout;            // The main tab layout.

    int GameID;                         // The game ID this tab refers to.
    const char* TabName;                // The tab name. Should correspond to the game it refers to.
    MapTab* GameMaps;                   // This tab widget will regroup all possible main regions and maps for a game.

    OoTMMComboTracker* Owner = nullptr; // The main window that owns this game tab.

    int FoundObjects = 0;               // The number found objects in this game.
    int TotalObjects = 0;               // The total number of objects that can be found in this game.

#pragma endregion

#pragma region Accent color

public:

    /*
    *   Get the accent hex color matching the given game ID.
    *
    *   @param GameID    The game ID to resolve the accent color for.
    *
    *   @return The accent color hex string (with leading '#').
    */
    static QString GetAccentColorFor(int GameID);

    /*
    *   Get the accent color associated to this game tab.
    *
    *   @return The accent color hex string (with leading '#').
    */
    QString GetAccentColor() const;

signals:

    /*
    *   Signal emitted when the accent color changes.
    */
    void AccentColorChanged();

#pragma endregion

public:

    /*
    *   Construct the game tab matching the given game ID.
    *
    *   @param GameID              The game ID this tab refers to.
    *   @param Parent              The parent tree item to attach this item to.
    */
    GameTab(int GameID, QWidget* parent = nullptr);

    /*
    *   Default destructor.
    */
    ~GameTab();

#pragma region Object related

public:

    /*
    *   Update the matching scene object.
    *
    *   @param Object       The object in which the item has been found.
    *   @param ItemFound    The item that has been found.
    *
    *   @warning This should be executed by the main thread only at it can modify the GUI elements.
    */
    void ItemFound(ObjectInfo* Object, const ItemInfo* Item);

    /*
    *   Refresh the tab name.
    */
    void RefreshTabCountText();

    /*
    *   Refresh the object visibility of the tab.
    */
    void UpdateObjectVisibility();

    /*
    *   Forward a scene focus request to the underlying MapTab. Used by the auto-follow player
    *   feature: OoTMMComboTracker resolves which game the player is in, brings that GameTab to
    *   the front and calls this to line up the map on the new scene.
    *
    *   @param SceneID    The scene ID to focus on.
    */
    void FocusScene(uint32_t SceneID);

#pragma endregion

#pragma region Saving / Loading

public:

    /*
    *   Load all elements of this game tab.
    */
    void LoadGameTab();

    /*
    *   Refresh all elements of this game tab.
    */
    void RefreshGameTab();

    /*
    *   Save all objects contains in this game to the desired path.
    * 
    *   @param FilePath    The file where to save the game objects.
    */
    static void SaveGameScenes(QString FilePath, Settings* FileSettings);


    /*
    *   Reset all objects / scenes.
    */
    static void ResetScenes();

#pragma endregion
};