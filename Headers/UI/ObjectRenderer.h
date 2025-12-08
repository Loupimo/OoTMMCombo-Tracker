#pragma once

#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QPixmap>
#include <QPair>
#include <QTreeWidget>
#include <QCollator>
#include "Combo/Objects.h"

class SceneRenderer;
class ObjectRenderer;
class ObjectItemTree;

typedef struct ObjectIcon
{
    const char* IconPath;   // The path to the object icon.
    int Scale[2];           // The scale of the image. ID 0 = width, ID 1 = height.
} ObjectIcon;

const ObjectIcon IconsMetaInfo[ObjectType::last] =
{
    {"", {0, 0}},                                          // ObjectType::none
    {"./Resources/Common/Chest.png", { 35, 35 }},          // ObjectType::chest
    {"./Resources/Common/Collectible.png", { 30, 30 }},    // ObjectType::collectible
    {"./Resources/Common/NPC.png", { 30, 30 }},            // ObjectType::npc
    {"./Resources/Common/Gold_Skulltula.png", { 50, 50 }}, // ObjectType::gs
    {"./Resources/Common/Stray_Fairy.png", { 30, 30 }},    // ObjectType::sf
    {"./Resources/Common/Cow.png", { 50, 50 }},            // ObjectType::cow
    {"./Resources/Common/Shop.png", { 30, 30 }},           // ObjectType::shop
    {"./Resources/Common/Scrub.png", { 55, 55 }},          // ObjectType::scrub
    {"./Resources/Common/Silver_Rupee.png", { 20, 20 }},   // ObjectType::sr
    {"./Resources/Common/Fish.png", { 30, 30 }},           // ObjectType::fish
    {"./Resources/Common/Wonder.png", { 30, 30 }},         // ObjectType::wonder
    {"./Resources/Common/Grass.png", { 20, 20 }},          // ObjectType::grass
    {"./Resources/Common/Crate.png", { 25, 25 }},          // ObjectType::crate
    {"./Resources/Common/Pot.png", { 25, 25 }},            // ObjectType::pot
    {"./Resources/Common/Hive.png", { 40, 40 }},           // ObjectType::hive
    {"./Resources/Common/Butterfly.png", { 20, 20 }},      // ObjectType::butterfly
    {"./Resources/Common/Rupee.png", { 20, 20 }},          // ObjectType::rupee
    {"./Resources/Common/Snowball.png", { 30, 30 }},       // ObjectType::snowball
    {"./Resources/Common/Barrel.png", { 20, 20 }},         // ObjectType::barrel
    {"./Resources/Common/Heart.png", { 30, 30 }},          // ObjectType::heart
    {"./Resources/Common/Fairy_Spot.png", { 30, 30 }},     // ObjectType::fairy_spot
    {"./Resources/Common/Fairy.png", { 25, 25 }},          // ObjectType::fairy
    {"./Resources/Common/Icicle.png", { 25, 25 }},         // ObjectType::icicle
    {"./Resources/Common/Red_Boulder.png", { 25, 25 }},    // ObjectType::redboulder
    {"./Resources/Common/Red_Ice.png", { 25, 25 }},        // ObjectType::redice
    {"./Resources/Common/Rock.png", { 25, 25 }},           // ObjectType::rock
    {"./Resources/Common/Soil.png", { 25, 25 }},           // ObjectType::soil
    {"./Resources/Common/Tree.png", { 25, 25 }},           // ObjectType::tree
    {"./Resources/Common/Bush.png", { 25, 25 }}            // ObjectType::bush
};

/*
* The class containing all object icons and their pixmap reprensentation.
*/
class ObjectIcons
{
public:

    QIcon Icons[ObjectType::last];            // The matching icon.
    QPixmap PixmapIcons[ObjectType::last];    // The matching pixmap.

public:

    /*
    *   Default constructor.
    */
    ObjectIcons();

    /*
    *   Default destructor.
    */
    ~ObjectIcons();

    /*
    *   Create the object icon class if it doesn't exist.
    */
    static void CreateObjectIcons();
};


/*
* The base class that all items that are added to the object list tree should inherit from.
*/
class CommonBaseItemTree : public QTreeWidgetItem
{
public:

    bool CalledFromGraph = false;   // A flag that indicates if actions should be performed in a graph item or item selection context.

public:

    /*
    *   Default constructor.
    *
    *   @param Parent              The parent tree item to attach this item to.
    */
    CommonBaseItemTree(QTreeWidgetItem* Parent = nullptr) : QTreeWidgetItem(Parent) {}

    /*
    *   Default destructor.
    */
    virtual ~CommonBaseItemTree() {}

    /*
    *   Tells if the action should be perform in a graph context.
    * 
    *   @return True = graph context, false = selection context.
    */
    bool IsCalledFromGraph() { return this->CalledFromGraph; }

    /*
    *   Set the calling context.
    *
    *   @param IsCalledFromGraph     The new calling context : true = graph context, false = selection context.
    */
    void SetCalledFromGraph(bool IsCalledFromGraph) { this->CalledFromGraph = IsCalledFromGraph; }

    /*
    *   Perform the desired action when clicked.
    */
    virtual void PerformAction() {}

    /*
    *   Reset the graphical effect of the associated object.
    */
    virtual void ResetObjectEffect() {}
};


/*
* This class is handling the graphical representation of each object in a scene.
*/
class ObjectPixmapItem : public QGraphicsPixmapItem
{
public:
        
    ObjectRenderer* Owner;          // The object renderer that owns this item.
    ObjectItemTree* ItemOwner;      // The object item tree that owns this item.

public:

    /*
    *   Construct a graph item using the given pixmap.
    *
    *   @param Pixmap              The pixmap that represent the object.
    *   @param Parent              The renderer that owns this item.
    *   @param Parent              The object item that owns this item.
    */
    ObjectPixmapItem(const QPixmap& Pixmap, ObjectRenderer* Owner, ObjectItemTree* ItemOwner);

    /*
    *   Updates the graphical effect of the object based on the given status.
    *
    *   @param ObjStatus           Update the graphical effect based on the object status.
    *   @param IsSelected          Adjust the effect based if the item is selected.
    */
    void UpdateObjectRendering(ObjectState ObjStatus, bool IsSelected);

protected:

    /*
    *   Perform action on the corresponding object when clicked.
    *
    *   @param event                The click event that triggered this function.
    */
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
};


/*
* The object item tree that is added at the end of the tree.
*/
class ObjectItemTree : public CommonBaseItemTree
{

public:

    ObjectRenderer* RendererOwner = nullptr;        // The object renderer that owns this item.
    ObjectInfo* Object;                             // The actual object information.
    ObjectPixmapItem* GraphItem = nullptr;          // The graphical representaion of the object.
    CommonBaseItemTree Item;                        // The item to display when the object is considered as collected.
    QColor DefaultTextColor;                        // The default text color to adapt the system color theme.

public:

    /*
    *   Construct a object item tree using the given object information.
    *
    *   @param Obj              The object information used to correctly render the final object.
    *   @param DefaultColor     The default text color.
    *   @param Owner            The renderer that owns this item.
    *   @param Parent           The parent tree item to attach this item to.
    */
    ObjectItemTree(ObjectInfo* Obj, QColor DefaultColor, ObjectRenderer* Owner, QTreeWidgetItem* Parent = nullptr);

    /*
    *   Default destructor.
    */
    ~ObjectItemTree();
    
    /*
    *   Gets the discovery state of the associated object.
    * 
    *   @return The discovery state of the associated object.
    */
    ObjectState GetStatus();

    /*
    *   Updates the object graphical icon based on the given type.
    *
    *   @param Type     The object type to use.
    */
    void UpdateIcon(ObjectType Type);

    /*
    *   Updates the object text style based on its discovery state.
    */
    void UpdateTextStyle();

    /*
    *   Remove the graphical item from the scene it rendered in.
    */
    void RemoveObjectFromScene();

    /*
    *   The less operator used to sort the item in alphabetic order.
    *
    *   @param Other    The other item to compare to.
    * 
    *   @return True if the text item is alphabetically lower than the one of the given object, false otherwise.
    */
    bool operator<(const QTreeWidgetItem& Other) const
    {
        QCollator collator;
        collator.setNumericMode(true);  // Active le tri naturel (interprète les nombres)

        return collator.compare(this->text(0), Other.text(0)) < 0;
    }

    /*
    *   Performs the matching actions corresponding to the object state.
    */
    void PerformAction() override;

    /*
    *   Resets the object graphical icon effect to the state corresponding to its object discovery state.
    */
    void ResetObjectEffect() override;
};


/*
* This class is handling the graphical representation of each object in a scene.
*/
class ObjectRenderer
{

public:

    SceneRenderer* SceneOwner = nullptr;    // The scene where the object should be renderer to.
    bool ShouldBeRendered = true;			// Tells if the objects should be rendered on the screen or not.
    ObjectType Type = ObjectType::none;     // The type of object, used to load the correct icon when needed.
    CommonBaseItemTree* ObjCat;             // The object category the rederer belongs to.

    std::vector<ObjectItemTree*> Objects;   // The list of all objects associated to this renderer.
    QPixmap Icon;					        // This icon that match the renderer category.

public:

    /*
    *   Construct an object renderer based on the given type.
    *
    *   @param Type             The object type to render.
    *   @param Parent           The parent tree item to attach this item to.
    */
    ObjectRenderer(ObjectType Type, SceneRenderer* Owner);

    /*
    *   Default destructor.
    */
    ~ObjectRenderer();

    /*
    *   Renders the objects to the scene based on the given context.
    *
    *   @param ActiveContext    The active context to match.
    */
    void RenderObjectToScene(ObjectContext ActiveContext);

    /*
    *   Adds a new object to this renderer.
    *
    *   @param Obj              The object information used to create a new object to render.
    *   @param DefaultColor     The default text color to use.
    */
    void AddObjectToRender(ObjectInfo* Obj, QColor DefaultColor);

    /*
    *   Removes all objects from the scene that belongs to this renderer.
    */
    void UnloadObjectsFromScene();

    /*
    *   Refreshes the object that match the given object information.
    *
    *   @param Object           The object to refresh.
    */
    void RefreshObject(ObjectInfo* Object);

    /*
    *   Updates the scene context with the given context.
    *
    *   @param Context          The new scene context.
    */
    void UpdateSceneContext(ObjectContext Context);

    /*
    *   Centers the view on the given target object.
    *
    *   @param Target           The object to center the view on.
    */
    void CenterViewOn(ObjectPixmapItem* Target);

    /*
    *   Gets the number of collected objects in this renderer.
    *
    *   @return The number of collected objects in this renderer.
    */
    size_t GetCollectedObject();

    /*
    *   Gets the number of objects in this renderer.
    *
    *   @return The number of objects in this renderer.
    */
    size_t GetTotalObject();

    /*
    *   Update the object category text of this renderer.
    */
    void UpdateText();

    /*
    *   Increase / decrease the number of found objects by the given amount.
    *
    *   @param Caller  The tree item that triggered the call.
    *   @param Count   The number of found object to add or remove.
    */
    void RefreshObjectCounts(ObjectItemTree* Caller, int Count);

    /*
    *   Remove all objects of this renderer from the given tree widget without destroying the object itself.
    *
    *   @param Tree             The tree widget to remove the object from.
    */
    void RemoveObjectFromList(QTreeWidget* Tree);
};
