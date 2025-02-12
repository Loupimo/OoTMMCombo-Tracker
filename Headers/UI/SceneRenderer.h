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

class SceneItemTree;

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

    ObjectRenderer * ObjectsRen [ObjectType::fairy];

    /*ObjectRenderer Pots = ObjectRenderer(ObjectType::pot);                  // Contains all pot objects of this scene
    ObjectRenderer Cows = ObjectRenderer(ObjectType::cow);                  // Contains all cow objects of this scene
    ObjectRenderer Grass = ObjectRenderer(ObjectType::grass);               // Contains all grass objects of this scene
    ObjectRenderer Chests = ObjectRenderer(ObjectType::chest);              // Contains all chest objects of this scene
    ObjectRenderer Collectibles = ObjectRenderer(ObjectType::collectible);  // Contains all collectible objects of this scene
    ObjectRenderer NPCs = ObjectRenderer(ObjectType::npc);                  // Contains all npc / event objects of this scene
    ObjectRenderer GoldSkulltulas = ObjectRenderer(ObjectType::gs);         // Contains all gold skulltula objects of this scene
    ObjectRenderer StrayFairies = ObjectRenderer(ObjectType::sf);           // Contains all stray fairy objects of this scene
    ObjectRenderer Shops = ObjectRenderer(ObjectType::shop);                // Contains all shop item objects of this scene
    ObjectRenderer Scrubs = ObjectRenderer(ObjectType::scrub);              // Contains all scrub objects of this scene
    ObjectRenderer SilverRupees = ObjectRenderer(ObjectType::sr);           // Contains all silver rupee objects of this scene
    ObjectRenderer Fishes = ObjectRenderer(ObjectType::fish);               // Contains all fish objects of this scene
    ObjectRenderer Wonders = ObjectRenderer(ObjectType::wonder);            // Contains all wonder item objects of this scene
    ObjectRenderer Crates = ObjectRenderer(ObjectType::crate);              // Contains all crate objects of this scene
    ObjectRenderer Hives = ObjectRenderer(ObjectType::hive);                // Contains all hive objects of this scene
    ObjectRenderer Butterflies = ObjectRenderer(ObjectType::butterfly);     // Contains all butterfly objects of this scene
    ObjectRenderer Rupees = ObjectRenderer(ObjectType::rupee);              // Contains all rupee objects of this scene
    ObjectRenderer Snowballs = ObjectRenderer(ObjectType::snowball);        // Contains all snowball objects of this scene
    ObjectRenderer Barrels = ObjectRenderer(ObjectType::barrel);            // Contains all barrel objects of this scene
    ObjectRenderer Hearts = ObjectRenderer(ObjectType::heart);              // Contains all heart objects of this scene
    ObjectRenderer FairySpots = ObjectRenderer(ObjectType::fairy_spot);     // Contains all big fairy objects of this scene
    ObjectRenderer Fairies = ObjectRenderer(ObjectType::fairy);             // Contains all fairy objects of this scene*/

    int FoundObjs = 0;
    int TotalObjs = 0;

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

public slots:

    /*
    *   Update the matching scene object.
    *
    *   @param Object       The object in which the item has been found.
    *   @param ItemFound    The item that has been found.
    * 
    *   @warning This should be executed by the main thread only at it can modify the GUI elements.
    */
    void UpdateItemFound(ObjectInfo* Object, const ItemInfo* ItemFound);

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


/*
*   As the scenes are not sorted in alphabetic order this class act as a wrapper between the clicked tree widget item and the actual scene it refers to.
*/
class SceneItemTree : public QTreeWidgetItem
{

#pragma region Attributes

public:

    SceneRenderer* Scene;   // The associated scene to load when this item is active.

#pragma endregion

public:

    /*
    *   Construct the associated scene and tree item and it to the given parent.
    *
    *   @param SceneToRender       The actual scene information used to create the scene.
    *   @param ObjectsTreeWidget   The object tree list to fill when this scene is active.
    *   @param Parent              The parent tree item to attach this item to.
    */
    SceneItemTree(SceneInfo* SceneToRender, QTreeWidget* ObjectsTreeWidget, QTreeWidgetItem * Parent = nullptr);

    /*
    *   Default destructor.
    */
    ~SceneItemTree();

    /*
    *   Render the scene associated to this item.
    */
    void RenderScene();

    /*
    *   Unload the scene associated to this item.
    */
    void UnloadScene();

    int GetCollectedObjects();
    
    int GetTotalObjects();

    void RefreshObjectCounts(int Count);
};

