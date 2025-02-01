#pragma once

#include <QObject>
#include <QGraphicsScene>
#include <QPixmap>
#include "Combo/Objects.h"
#include "Combo/Scenes.h"
#include "Combo/Items.h"
#include "Multi/Game.h"
#include "ObjectRenderer.h"

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

    void NotifyItemFound(ObjectInfo* Object, const ItemInfo* ItemFound);

public:

	uint32_t SceneID;			            // The actual OOTMM rom scene
    int GameID;                             // The game scene come from
    SceneObjects* Objects;                  // The scene objects
    SceneType Type;                         // The type of scene
    const SceneMetaInfo* Info;              // The scene name and image path

public:

    // Constructeur pour initialiser la structure
    SceneInfo(int PSceneID, int PGameID, SceneType PType = SceneType::None);

    // Destructeur pour libérer la mémoire
    ~SceneInfo();

};




class SceneRenderer : public QGraphicsScene
{
    Q_OBJECT

public:
    SceneInfo* CurrScene;
    bool IsRendered = false;    // Tells if the scene is currently being rendered by on the GUI

    QPixmap* SceneImage = nullptr;

    ObjectRenderer Pots = ObjectRenderer(ObjectType::pot);
    ObjectRenderer Cows = ObjectRenderer(ObjectType::cow);
    ObjectRenderer Grass = ObjectRenderer(ObjectType::grass);
    ObjectRenderer Chests = ObjectRenderer(ObjectType::chest);
    ObjectRenderer Collectibles = ObjectRenderer(ObjectType::collectible);
    ObjectRenderer NPCs = ObjectRenderer(ObjectType::npc);
    ObjectRenderer GoldSkulltulas = ObjectRenderer(ObjectType::gs);
    ObjectRenderer StrayFairies = ObjectRenderer(ObjectType::sf);
    ObjectRenderer Shops = ObjectRenderer(ObjectType::shop);
    ObjectRenderer Scrubs = ObjectRenderer(ObjectType::scrub);
    ObjectRenderer SilverRupees = ObjectRenderer(ObjectType::sr);
    ObjectRenderer Fishes = ObjectRenderer(ObjectType::fish);
    ObjectRenderer Wonders = ObjectRenderer(ObjectType::wonder);
    ObjectRenderer Crates = ObjectRenderer(ObjectType::crate);
    ObjectRenderer Hives = ObjectRenderer(ObjectType::hive);
    ObjectRenderer Butterflies = ObjectRenderer(ObjectType::butterfly);
    ObjectRenderer Rupees = ObjectRenderer(ObjectType::rupee);
    ObjectRenderer Snowballs = ObjectRenderer(ObjectType::snowball);
    ObjectRenderer Barrels = ObjectRenderer(ObjectType::barrel);
    ObjectRenderer Hearts = ObjectRenderer(ObjectType::heart);
    ObjectRenderer FairySpots = ObjectRenderer(ObjectType::fairy_spot);
    ObjectRenderer Fairies = ObjectRenderer(ObjectType::fairy);

    SceneRenderer(SceneInfo* SceneToRender);
    ~SceneRenderer();

public:

    const char* GetSceneName();
    uint8_t GetSceneParentRegion();

    void RenderScene();
    void UnloadScene();

public slots:

    /*
    *   Update the matching scene object .
    *
    *   @param Object       The object in which the item has been found
    *   @param ItemFound    The item that has been found
    * 
    *   @warning This should be executed bny the main thread only at it can modify the GUI elements.
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