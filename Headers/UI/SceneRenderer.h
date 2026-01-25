#pragma once

#include <QObject>
#include <QGraphicsScene>
#include <QPixmap>
#include <QTreeWidget>
#include <QFile>
#include "Combo/Objects.h"
#include "Combo/Scenes.h"
#include "Combo/Items.h"
#include "Multi/Game.h"
#include "ObjectRenderer.h"

class SceneRenderer;
class RoomItemTree;
class FilterManager;
struct RoomInfo;

enum SceneType
{
    None = 0,
    Overworld = 1,
    Temple = 2,
    House = 3,
    Grotto = 4
};

/*
*   The major scene information used by the renderer.
*/
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
    const SceneMetaInfo* Info;              // The scene meta information

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
    ~SceneInfo() {}
};


/*
*   As the scenes are not sorted in alphabetic order this class act as a wrapper between the clicked tree widget item and the actual scene it refers to.
*/
class SceneItemTree : public QTreeWidgetItem
{

#pragma region Attributes

public:

    int FoundObjects = 0;                   // The number forced / collected object in this scene.
    int TotalObjects = 0;                   // The total number of object that can be found in this scene

    SceneInfo* Scene = nullptr;             // The scene information.
    SceneRenderer* Renderer = nullptr;      // The associated scene to load when this item is active.
    RoomInfo* ActiveRoom = nullptr;         // The current active room.
    FilterManager* Filter = nullptr;        // A reference to the filter manager.

    std::vector<RoomItemTree*> Rooms;       // The available scene's rooms. If empty this means that the scene has only one room.

#pragma endregion

public:

    /*
    *   Default constructor.
    * 
    *   @param Parent              The parent tree item to attach this item to.
    */
    SceneItemTree(QTreeWidgetItem* Parent = nullptr) {}

    /*
    *   Construct the associated scene and tree item and it to the given parent.
    *
    *   @param SceneToRender       The actual scene information used to create the scene.
    *   @param Parent              The parent tree item to attach this item to.
    *   @param Filter              A reference to the filter manager to use.
    */
    SceneItemTree(SceneInfo* SceneToRender, FilterManager* Filter, QTreeWidgetItem* Parent = nullptr);

    /*
    *   Default destructor.
    */
    ~SceneItemTree();

#pragma region Rendering

public:

    /*
    *   Gets the scene renderer.
    *
    *   @return The scene renderer.
    */
    virtual SceneRenderer* GetScene();

    /*
    *   Render the scene associated to this item.
    *
    *   @param ObjectsTreeWidget   The object tree list to fill when this scene is active.
    *   @param Context             The context in which the scene should be rendered.
    *   @param CreateNew           Tells if we should create a new scene renderer or not.
    *   @param Filter              The reference to the filter manager to use.
    */
    virtual void RenderScene(QTreeWidget* ObjectsTreeWidget, bool Context, bool CreateNew);

    /*
    *   Unload the scene associated to this item.
    */
    virtual void UnloadScene();

#pragma endregion

#pragma region Scene context / Info

public:

    /*
    *   Gets the scene name.
    *
    *   @return The scene name.
    */
    virtual const char* GetSceneName();

    /*
    *   Updates the current active room with the one matching the given ID.
    *
    *   @param RoomID   Update the current active room.
    */
    void UpdateRoom(uint32_t RoomID);

    /*
    *   Tells if this scene has a context.
    *
    *   @return True means the scene has a context, false means no context.
    */
    virtual bool HasContext();

#pragma endregion

#pragma region Objects related

public:

    /*
    *   Gets the number of collected item in this scene.
    *
    *   @return The number of collected items.
    */
    int GetCollectedObjects();

    /*
    *   Gets the total number of object that can be rendered by this scene.
    *
    *   @return The total number of object that can be rendered by this scene.
    */
    int GetTotalObjects();

    /*
    *   Update the scene with the given item and updated object.
    *
    *   @param Object   The object that has been updated.
    *   @param Item     The item found in the object.
    */
    void ItemFound(ObjectInfo* Object, const ItemInfo* Item);

    /*
    *   Counts the found and total number of objects of this scenes.
    */
    void CountSceneObjects();

    /*
    *   Increase / decrease the number of found object by the given amount.
    *
    *   @param Count  The number of found object to add or remove.
    */
    virtual void UpdateObjectCounts(int Count);

    /*
    *   Refresh the item name.
    */
    void RefreshItemName();


#pragma endregion

};


/*
*   The class that handles scene rendering and objects interaction.
*/
class SceneRenderer : public QGraphicsScene
{
    Q_OBJECT

#pragma region Attributes

public:

    uint32_t ActiveRoom;                                                    // The current active room of the scene
    SceneInfo* CurrScene;                                                   // Contains all info for this scene.
    QTreeWidget* ObjectsTree = nullptr;                                     // The object panel where to display the scene object list.
    SceneItemTree* ItemOwner = nullptr;                                     // The tree widget item that owns this scene
    ObjectContext CurrContext = ObjectContext::All;                         // The current context of the scene
    FilterManager* Filter = nullptr;                                        // A reference to the filter manager.

    QPixmap* SceneImage = nullptr;                                          // The scene image to render

    ObjectRenderer* ObjectsRen[ObjectType::last - 1] = { nullptr };

#pragma endregion

public:

    /*
    *   Construct the scene and all its objects.
    *
    *   @param SceneToRender       The actual scene information used to create the scene.
    *   @param ObjectsTreeWidget   The object tree list to fill when this scene is active.
    *   @param Owner               The item tree list that owns this scene.
    *   @param Filter              A reference to the filter manager.
    */
    SceneRenderer(SceneInfo* SceneToRender, QTreeWidget* ObjectsTreeWidget, SceneItemTree* Owner, FilterManager* Filter);

    /*
    *   Default destructor.
    */
    ~SceneRenderer();

#pragma region Rendering

public:

    /*
    *   Render the scene elements.
    */
    void RenderScene(bool Context, RoomInfo* Room);

    /*
    *   Unload the scene elements.
    */
    void UnloadScene();

    /*
    *   Center the view on the given object.
    *
    *   @param Target   The object to center the view on.
    */
    void CenterViewOn(ObjectPixmapItem* Target);

#pragma endregion

#pragma region Objects related

public:

    /*
    *   Increase / decrease the number of found objects by the given amount.
    *
    *   @param Count  The number of found object to add or remove.
    */
    void UpdateObjectCounts(ObjectItemTree* Caller, int Count);

    /*
    *   Update the matching scene object.
    *
    *   @param Object       The object in which the item has been found.
    *   @param ItemFound    The item that has been found.
    *
    *   @warning This should be executed by the main thread only at it can modify the GUI elements.
    */
    void ItemFound(ObjectInfo* Object, const ItemInfo* ItemFound);

protected:

    /*
    *   Find the object renderer matching the given object.
    *
    *   @param Object   The object to find the matching renderer.
    *
    *   @return The object renderer that match the given object type.
    */
    ObjectRenderer* FindObjectRendererCategory(ObjectInfo* Object);

#pragma endregion

#pragma region Scene context / Info

public:

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
    *   Update the room ID of the item that owns this renderer.
    *
    *   @param RoomID   The new active room ID.
    */
    void UpdateRoom(uint32_t RoomID);

    /*
    *   Update the global game context.
    *
    *   @param Context  The new game context.
    */
    void UpdateContext(ObjectContext Context);

    /*
    *   Update the scene object visibility based on the given filter.
    *
    *   @param Context  The filter containing the object types to display.
    */
    void UpdateSceneObjectVisibility();

    /*
    *   Refresh the scene based on the given context.
    *
    *   @param Context  Refresh the scene based on the given context : true = Spring / Adult, false = Winter / Child.
    */
    void RefreshSceneContext(bool Context);

#pragma endregion

};
