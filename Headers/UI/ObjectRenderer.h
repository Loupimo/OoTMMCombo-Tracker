#pragma once

#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QPixmap>
#include <QPair>
#include <QTreeWidget>
#include <QStyledItemDelegate>
#include <QCollator>
#include <QTimer>
#include <QString>
#include "Combo/Objects.h"

class SceneRenderer;
class ObjectRenderer;
class ObjectItemTree;

/* Custom data roles for ObjectItemTree rows. */
enum {
    ItemNameRole     = Qt::UserRole + 1,    // QString — the item label, "" when unknown.
    ObjectStatusRole = Qt::UserRole + 2,    // int — ObjectState cast to int.
};


/*
*   Two-line item delegate for the object list: icon on the left, object name
*   on top, item name (or "???") underneath. Categories (top-level rows) keep
*   the default rendering.
*/
class ObjectItemDelegate : public QStyledItemDelegate
{
public:
    explicit ObjectItemDelegate(QObject* Parent = nullptr) : QStyledItemDelegate(Parent) {}

    /*
    *   Paint the two-line row: icon on the left, object name on top, item name (or "???") underneath.
    *   Category rows fall back to the default QStyledItemDelegate rendering.
    *
    *   @param Painter    The painter to draw with.
    *   @param Option     The style options for the item, including the rect and state.
    *   @param Index      The model index identifying the row to paint.
    */
    void paint(QPainter* Painter, const QStyleOptionViewItem& Option, const QModelIndex& Index) const override;

    /*
    *   Return the preferred size for a two-line row, or the default size for category rows.
    *
    *   @param Option    The style options for the item.
    *   @param Index     The model index identifying the row.
    *
    *   @return The recommended size for the row.
    */
    QSize sizeHint(const QStyleOptionViewItem& Option, const QModelIndex& Index) const override;
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

    /*
    *   Get the total number of visible objects in the item tree.
    *
    *   @return The total of visible objects in the item tree.
    */
    virtual int GetTotalObjectAvailable() { return 1; }
};


/*
* This class is handling the graphical representation of each object in a scene.
*/
class ObjectPixmapItem : public QGraphicsPixmapItem
{
    
public:
        
    ObjectRenderer* Owner;          // The object renderer that owns this item.
    ObjectItemTree* ItemOwner;      // The object item tree that owns this item.
    QTimer* HoverTimer;             // The timer to wait before printing the tooltip
    bool TimerStart;                // Tells if the timer is already started or not

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

    /*
    *   Perform action on the corresponding object when the mouse is hover it.
    *
    *   @param event                The mouse hover move event that triggered this function.
    */
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;

    /*
    *   Perform action on the corresponding object when the mouse is moving hover it.
    *
    *   @param event                The mouse hover enter event that triggered this function.
    */
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;

    /*
    *   Perform action on the corresponding object when the mouse is not hover it anymore.
    *
    *   @param event                The mouse hover leave event that triggered this function.
    */
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
};


/*
* The object item tree that is added at the end of the tree.
*/
class ObjectItemTree : public CommonBaseItemTree
{

public:

    ObjectRenderer* RendererOwner = nullptr;        // The object renderer that owns this item.
    ObjectInfo* Object;                             // The actual object information.
    ObjectPixmapItem* GraphItem = nullptr;          // The graphical representation of the object.
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
    *   Set the scene object visibility.
    *
    *   @param Visivility    <b>True</b> if the object should be rendered on the scene, <b>false</b> otherwise.
    */
    void SetSceneObjectVisibility(bool Visivility);

    /*
    *   Set the list object visibility.
    *
    *   @param Visivility    <b>True</b> if the object should be rendered on the object list, <b>false</b> otherwise.
    */
    void SetListObjectVisibility(bool Visivility);

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

    /*
    *   Gets the tooltip text associated to this object.
    * 
    *   @return The tooltip associated to this object.
    */
    const char* GetObjectToolTip();

    /*
    *   Get the total number of visible objects in the item tree.
    *
    *   @return The total of visible objects in the item tree.
    */
    int GetTotalObjectAvailable() override;
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
    QPixmap Icon;					        // This icon that match the item or category to render.

public:

    /*
    *   Construct an object renderer based on the given type.
    *
    *   @param Type             The object type to render.
    *   @param Parent           The parent tree item to attach this item to.
    *   @param ShoulBeRendered  Tells if the objects should be rendered.
    */
    ObjectRenderer(ObjectType Type, SceneRenderer* Owner, bool ShouldBeRendered);

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

/*
* Count matching objects in an array, filtering by scene, room, layout, and exclusion status.
*
* @param Objects       The objects array to count.
* @param Count         The number of objects in the array.
* @param Filter        The filter manager to check exclusions.
* @param Layout        The active game layout.
* @param SceneID       The scene ID to match.
* @param RoomID        The room ID to match (-1 = all rooms, match all RoomIDs).
* @param FoundOut      Output pointer for found count.
* @param TotalOut      Output pointer for total count.
*/
void CountObjectsMatching(ObjectInfo* Objects, size_t Count, class FilterManager* Filter, GameLayout Layout, uint32_t SceneID, int RoomID, int* FoundOut, int* TotalOut);
