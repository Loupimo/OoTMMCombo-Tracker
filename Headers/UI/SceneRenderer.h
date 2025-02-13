#pragma once

#include <QObject>
#include <QGraphicsScene>
#include <QPixmap>
#include <QTreeWidget>
#include "Combo/Objects.h"
#include "Combo/Scenes.h"
#include "Combo/Items.h"
#include "Multi/Game.h"
#include "ObjectRenderer.h"

class SceneRenderer;

enum SceneType
{
    None = 0,
    Overworld = 1,
    Temple = 2,
    House = 3,
    Grotto = 4
};


class SceneInfo : public QObject
{
    Q_OBJECT

signals:

    /*
    *   Called when an object belonging to this scene is collected.
    *
    *   @param Object       The collected object.
    *   @param ItemFound    Should be removed.
    */
    void NotifyItemFound(ObjectInfo* Object, const ItemInfo* ItemFound);

public:

	uint32_t SceneID;			            // The actual OOTMM rom scene
    int GameID;                             // The game scene come from
    SceneObjects* Objects;                  // The scene objects
    SceneType Type;                         // The type of scene
    const SceneMetaInfo* Info;              // The scene name and image path

public:

    /*
    *   Construct the scene info based on the given parameters.
    *
    *   @param PSceneID     The actual scene OOTMM rom scene ID.
    *   @param PGameID      The object tree list to fill when this scene is active.
    *   @param PType        Should be removed.
    */
    SceneInfo(int PSceneID, int PGameID, SceneType PType = SceneType::None);

    /*
    *   Default destructor.
    */
    ~SceneInfo();

};


/*
*   As the scenes are not sorted in alphabetic order this class act as a wrapper between the clicked tree widget item and the actual scene it refers to.
*/
class SceneItemTree : public QTreeWidgetItem
{

#pragma region Attributes

public:

    int FoundObjects = 0;
    int TotalObjects = 0;

    SceneInfo* Scene = nullptr;
    SceneRenderer* Renderer = nullptr;   // The associated scene to load when this item is active.

#pragma endregion

public:

    /*
    *   Construct the associated scene and tree item and it to the given parent.
    *
    *   @param SceneToRender       The actual scene information used to create the scene.
    *   @param Parent              The parent tree item to attach this item to.
    */
    SceneItemTree(SceneInfo* SceneToRender, QTreeWidgetItem* Parent = nullptr);

    /*
    *   Default destructor.
    */
    ~SceneItemTree();

    /*
    *   Render the scene associated to this item.
    *
    *   @param ObjectsTreeWidget   The object tree list to fill when this scene is active.
    */
    void RenderScene(QTreeWidget* ObjectsTreeWidget);

    /*
    *   Unload the scene associated to this item.
    */
    void UnloadScene();

    const char* GetSceneName();
    int GetCollectedObjects();

    int GetTotalObjects();

    void RefreshObjectCounts(int Count);

    void ItemFound(ObjectInfo* Object, const ItemInfo* Item);
};


/*
*   The class that handles scene rendering and objects interaction.
*/
class SceneRenderer : public QGraphicsScene
{
    Q_OBJECT

#pragma region Attributes

public:

    SceneInfo* CurrScene;                                                   // Contains all info for this scene.
    bool IsRendered = false;                                                // Tells if the scene is currently being rendered by on the GUI
    QTreeWidget* ObjectsTree = nullptr;                                     // The object panel where to display the scene object list;
    SceneItemTree* ItemOwner = nullptr;                                     // The tree widget item that owns this scene

    QPixmap* SceneImage = nullptr;                                          // The scene image to render

    ObjectRenderer* ObjectsRen[ObjectType::fairy] = { nullptr };

#pragma endregion

public:

    /*
    *   Construct the scene and all its objects.
    *
    *   @param SceneToRender       The actual scene information used to create the scene.
    *   @param ObjectsTreeWidget   The object tree list to fill when this scene is active.
    *   @param Owner               The item tree list that owns this scene.
    */
    SceneRenderer(SceneInfo* SceneToRender, QTreeWidget* ObjectsTreeWidget, SceneItemTree* Owner);

    /*
    *   Default destructor.
    */
    ~SceneRenderer();

    /*
    *   Get the scene name.
    *
    *   @return The scene name;
    */
    const char* GetSceneName();

    /*
    *   Get the region ID this scene belongs to.
    *
    *   @return The region ID this scene belongs to.
    */
    uint8_t GetSceneParentRegion();

    /*
    *   Render the scene elements.
    */
    void RenderScene();

    /*
    *   Unload the scene elements.
    */
    void UnloadScene();

    void RefreshObjectCounts(int Count);

//public slots:

    /*
    *   Update the matching scene object.
    *
    *   @param Object       The object in which the item has been found.
    *   @param ItemFound    The item that has been found.
    * 
    *   @warning This should be executed by the main thread only at it can modify the GUI elements.
    */
    void ItemFound(ObjectInfo* Object, const ItemInfo* ItemFound);

    void CenterViewOn(ObjectPixmapItem* Target);

protected:

    /*
    *   Find the object renderer matching the given object.
    *
    *   @param Object       The object to find the matching renderer.
    *
    *   @return The object rendere that match the given object type.
    */
    ObjectRenderer* FindObjectRendererCategory(ObjectInfo* Object);
};