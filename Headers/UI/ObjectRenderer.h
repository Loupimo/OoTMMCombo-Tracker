#pragma once

#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QPixmap>
#include <QPair>
#include "Combo/Objects.h"

typedef struct ObjectIcon
{
    const char* IconPath;
    int Scale[2];       // The scale of the image. ID 0 = width, ID 1 = height
} ObjectIcon;

const ObjectIcon IconsMetaInfo[23] =
{
    {"", {0, 0}},                                          // ObjectType::none
    {"./Resources/Common/Chest.png", { 30, 30 }},          // ObjectType::chest
    {"./Resources/Common/Collectible.png", { 30, 30 }},    // ObjectType::collectible
    {"./Resources/Common/NPC.png", { 30, 30 }},            // ObjectType::npc
    {"./Resources/Common/Gold_Skulltula.png", { 30, 30 }}, // ObjectType::gs
    {"./Resources/Common/Stray_Fairy.png", { 30, 30 }},    // ObjectType::sf
    {"./Resources/Common/Cow.png", { 100, 100 }},          // ObjectType::cow
    {"./Resources/Common/Shop.png", { 30, 30 }},           // ObjectType::shop
    {"./Resources/Common/Scrub.png", { 30, 30 }},          // ObjectType::scrub
    {"./Resources/Common/Silver_Rupee.png", { 30, 30 }},   // ObjectType::sr
    {"./Resources/Common/Fish.png", { 30, 30 }},           // ObjectType::fish
    {"./Resources/Common/Wonder.png", { 30, 30 }},         // ObjectType::wonder
    {"./Resources/Common/Grass.png", { 30, 30 }},          // ObjectType::grass
    {"./Resources/Common/Crate.png", { 30, 30 }},          // ObjectType::crate
    {"./Resources/Common/Pot.png", { 40, 40 }},            // ObjectType::pot
    {"./Resources/Common/Hive.png", { 30, 30 }},           // ObjectType::hive
    {"./Resources/Common/Butterfly.png", { 30, 30 }},      // ObjectType::butterfly
    {"./Resources/Common/Rupee.png", { 30, 30 }},          // ObjectType::rupee
    {"./Resources/Common/Snowball.png", { 30, 30 }},       // ObjectType::snowball
    {"./Resources/Common/Barrel.png", { 30, 30 }},         // ObjectType::barrel
    {"./Resources/Common/Heart.png", { 30, 30 }},          // ObjectType::heart
    {"./Resources/Common/Fairy_Spot.png", { 30, 30 }},     // ObjectType::fairy_spot
    {"./Resources/Common/Fairy.png", { 30, 30 }}           // ObjectType::fairy
};


class ObjectIcons
{
public:

    QPixmap Icons[23];

public:

    ObjectIcons();
    ~ObjectIcons();
    static void CreateObjectIcons();
};

class ObjectRenderer : public QGraphicsPixmapItem
{

public:

	bool ShouldBeRendered = true;			// Tells if the objects should be rendered on the screen or not
    ObjectType Type = ObjectType::none;     // The type of object, used to load the correct icon when needed

protected:
	QPixmap * Icon;					        // Image à afficher
	QList<QPair<int, int>> ObjectsLocation; // Liste des positions (X, Y) des objets

public:
	ObjectRenderer(ObjectType Type);
	void AddObjectToScene(QGraphicsScene* Destination);
	void AddObjectToRender(int X, int Y);
    void UpdateObjectState(QGraphicsScene* Destination);
};