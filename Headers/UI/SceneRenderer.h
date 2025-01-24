#pragma once

#include <QGraphicsScene>
#include <QPixmap>
#include "Combo/Objects.h"
#include "Combo/Scenes.h"
#include "Multi/Game.h"
#include "ObjectRenderer.h"


typedef struct SceneInfo
{
	uint32_t SceneID;			            // The actual OOTMM rom scene
    int GameID;                             // The game scene come from
	const char* Image;			            // The path to the corresponding image
    SceneObjects* Objects;                  // The scene objects

    // Constructeur pour initialiser la structure
    SceneInfo(int PSceneID, const char* PImage, int PGameID);

    // Destructeur pour libérer la mémoire
    ~SceneInfo();

} SceneInfo;


const SceneInfo OoTScenes[1] =
{
    SceneInfo(LINK_HOUSE, "./Resources/OoT/Kokiri_Forest/Link_House.png", OOT_GAME)
};


class SceneRenderer : public QGraphicsScene
{
    Q_OBJECT

public:
    const SceneInfo* CurrScene;

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

    SceneRenderer(const SceneInfo* SceneToRender);
    ~SceneRenderer();


    void RenderScene();
    void UnloadScene();
};