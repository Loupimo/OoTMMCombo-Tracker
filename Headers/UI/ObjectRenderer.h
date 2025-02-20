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
    const char* IconPath;
    int Scale[2];       // The scale of the image. ID 0 = width, ID 1 = height
} ObjectIcon;

const ObjectIcon IconsMetaInfo[23] =
{
    {"", {0, 0}},                                          // ObjectType::none
    {"./Resources/Common/Chest.png", { 40, 40 }},          // ObjectType::chest
    {"./Resources/Common/Collectible.png", { 50, 50 }},    // ObjectType::collectible
    {"./Resources/Common/NPC.png", { 50, 50 }},            // ObjectType::npc
    {"./Resources/Common/Gold_Skulltula.png", { 50, 50 }}, // ObjectType::gs
    {"./Resources/Common/Stray_Fairy.png", { 30, 30 }},    // ObjectType::sf
    {"./Resources/Common/Cow.png", { 100, 100 }},          // ObjectType::cow
    {"./Resources/Common/Shop.png", { 30, 30 }},           // ObjectType::shop
    {"./Resources/Common/Scrub.png", { 55, 55 }},          // ObjectType::scrub
    {"./Resources/Common/Silver_Rupee.png", { 30, 30 }},   // ObjectType::sr
    {"./Resources/Common/Fish.png", { 30, 30 }},           // ObjectType::fish
    {"./Resources/Common/Wonder.png", { 40, 40 }},         // ObjectType::wonder
    {"./Resources/Common/Grass.png", { 30, 30 }},          // ObjectType::grass
    {"./Resources/Common/Crate.png", { 30, 30 }},          // ObjectType::crate
    {"./Resources/Common/Pot.png", { 40, 40 }},            // ObjectType::pot
    {"./Resources/Common/Hive.png", { 40, 40 }},           // ObjectType::hive
    {"./Resources/Common/Butterfly.png", { 30, 30 }},      // ObjectType::butterfly
    {"./Resources/Common/Rupee.png", { 30, 30 }},          // ObjectType::rupee
    {"./Resources/Common/Snowball.png", { 30, 30 }},       // ObjectType::snowball
    {"./Resources/Common/Barrel.png", { 30, 30 }},         // ObjectType::barrel
    {"./Resources/Common/Heart.png", { 30, 30 }},          // ObjectType::heart
    {"./Resources/Common/Fairy_Spot.png", { 40, 40 }},     // ObjectType::fairy_spot
    {"./Resources/Common/Fairy.png", { 30, 30 }}           // ObjectType::fairy
};


class CommonBaseItemTree : public QTreeWidgetItem
{
public:

    bool CalledFromGraph = false;   // A flag that indicates if actions should be performed in a graph item context

public:

    CommonBaseItemTree(QTreeWidgetItem* Parent = nullptr) : QTreeWidgetItem(Parent) {}
    virtual ~CommonBaseItemTree() { }
    virtual void PerformAction() {}
    virtual void ResetObjectEffect() {}
    bool IsCalledFromGraph() { return this->CalledFromGraph; }
    void SetCalledFromGraph(bool IsCalledFromGraph) { this->CalledFromGraph = IsCalledFromGraph; }
};


class ObjectIcons
{
public:

    QIcon Icons[23];
    QPixmap PixmapIcons[23];

public:

    ObjectIcons();
    ~ObjectIcons();
    static void CreateObjectIcons();
};


class ObjectPixmapItem : public QGraphicsPixmapItem
{
public:

    ObjectRenderer* Owner;
    ObjectItemTree* ItemOwner;

public:

    ObjectPixmapItem(const QPixmap& Pixmap, ObjectRenderer* Owner, ObjectItemTree* ItemOwner);

    void UpdateObjectRendering(ObjectState ObjStatus, bool IsSelected);

protected:

    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
};


class ObjectItemTree : public CommonBaseItemTree
{
public:

    ObjectRenderer* RendererOwner = nullptr;
    ObjectInfo* Object;
    ObjectPixmapItem* GraphItem = nullptr;
    CommonBaseItemTree Item;
    QColor DefaultTextColor;

public:

    ObjectItemTree(ObjectInfo* Obj, QColor DefaultColor, ObjectRenderer* Owner, QTreeWidgetItem* Parent = nullptr);
    ~ObjectItemTree();

    ObjectState GetStatus();
    void UpdateIcon(ObjectType Type);
    void UpdateTextStyle();

    void RemoveObjectFromScene();

    bool operator<(const QTreeWidgetItem& Other) const
    {
        QCollator collator;
        collator.setNumericMode(true);  // Active le tri naturel (interprète les nombres)

        return collator.compare(this->text(0), Other.text(0)) < 0;
    }

    void PerformAction() override;
    void ResetObjectEffect() override;
};


class ObjectRenderer
{

public:

    SceneRenderer* SceneOwner = nullptr;
    bool ShouldBeRendered = true;			// Tells if the objects should be rendered on the screen or not
    ObjectType Type = ObjectType::none;     // The type of object, used to load the correct icon when needed
    CommonBaseItemTree* ObjCat;

    std::vector<ObjectItemTree*> Objects;
protected:
    QPixmap* Icon;					        // Image à afficher

public:
    ObjectRenderer(ObjectType Type, SceneRenderer* Owner);
    ~ObjectRenderer();

    void AddObjectToScene(ObjectContext ActiveContext);
    void AddObjectToRender(ObjectInfo* Obj, QColor DefaultColor);
    void UnloadObjectsFromScene();
    void UpdateObjectState(ObjectInfo* Object);
    void UpdateContext(ObjectContext Context);
    void CenterViewOn(ObjectPixmapItem* Target);

    size_t GetCollectedObject();
    size_t GetTotalObject();
    void UpdateText();
    void RefreshObjectCounts(int Count);
    void RemoveObjectFromList(QTreeWidget* Tree);
};
