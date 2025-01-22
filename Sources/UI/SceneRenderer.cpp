#include "UI/SceneRenderer.h"

// Constructeur pour initialiser la structure
SceneInfo::SceneInfo(int PSceneID, const char* PImage, int PGameID)
{
    this->SceneID = PSceneID;
    this->Image = PImage;
    this->GameID = PGameID;

    size_t arrLen = 0;
    const ObjectInfo* arrObj = nullptr;

    if (this->GameID == OOT_GAME)
    {   // Ocarina of time

        arrLen = NUM_OOT_OBJ;
        arrObj = OoTObjects;
    }
    else
    {   // Majora's Mask

        arrLen = NUM_MM_OBJ;
        arrObj = MMObjects;
    }

    for (size_t i = 0; i < arrLen; i++)
    {
        const ObjectInfo* currObj = &arrObj[i];
        if (currObj->RenderScene == this->SceneID)
        {   // We can add the object to the scene

            this->Objects.push_back(currObj);
        }
    }
}

// Destructeur pour libérer la mémoire
SceneInfo::~SceneInfo()
{
}



SceneRenderer::SceneRenderer(const SceneInfo* SceneToRender) : QGraphicsScene()
{
	this->CurrScene = SceneToRender;
	if (SceneToRender)
	{
		for (size_t i = 0; i < SceneToRender->Objects.size(); i++)
		{
			const ObjectInfo* currObject = SceneToRender->Objects[i];
			ObjectRenderer* dest = nullptr;
			switch (currObject->Type)
			{
                case ObjectType::pot:
                {
                    dest = &this->Pots;
                    break;
                }
                case ObjectType::cow:
                {
                    dest = &this->Cows;
                    break;
                }
                case ObjectType::grass:
                {
                    dest = &this->Grass;
                    break;
                }
                case ObjectType::chest:
                {
                    dest = &this->Chests;
                    break;
                }
                case ObjectType::collectible:
                {
                    dest = &this->Collectibles;
                    break;
                }
                case ObjectType::npc:
                {
                    dest = &this->NPCs;
                    break;
                }
                case ObjectType::gs:
                {
                    dest = &this->GoldSkulltulas;
                    break;
                }
                case ObjectType::sf:
                {
                    dest = &this->StrayFairies;
                    break;
                }
                case ObjectType::shop:
                {
                    dest = &this->Shops;
                    break;
                }
                case ObjectType::scrub:
                {
                    dest = &this->Scrubs;
                    break;
                }
                case ObjectType::sr:
                {
                    dest = &this->SilverRupees;
                    break;
                }
                case ObjectType::fish:
                {
                    dest = &this->Fishes;
                    break;
                }
                case ObjectType::wonder:
                {
                    dest = &this->Wonders;
                    break;
                }
                case ObjectType::crate:
                {
                    dest = &this->Crates;
                    break;
                }
                case ObjectType::hive:
                {
                    dest = &this->Hives;
                    break;
                }
                case ObjectType::butterfly:
                {
                    dest = &this->Butterflies;
                    break;
                }
                case ObjectType::rupee:
                {
                    dest = &this->Rupees;
                    break;
                }
                case ObjectType::snowball:
                {
                    dest = &this->Snowballs;
                    break;
                }
                case ObjectType::barrel:
                {
                    dest = &this->Barrels;
                    break;
                }
                case ObjectType::heart:
                {
                    dest = &this->Hearts;
                    break;
                }
                case ObjectType::fairy_spot:
                {
                    dest = &this->FairySpots;
                    break;
                }
                case ObjectType::fairy:
                {
                    dest = &this->Fairies;
                    break;
                }

                case ObjectType::none:
                default:
                {
                    // Aucun traitement spécifique pour les types inconnus
                    break;
                }
			}

			if (dest != nullptr)
			{	// Add the object to the renderer

				dest->AddObjectToRender(currObject->Position[0], currObject->Position[1]);
			}
		}
        this->SceneImage = new QPixmap(SceneToRender->Image);
	}

}

SceneRenderer::~SceneRenderer()
{
    this->CurrScene = nullptr;
    this->SceneImage->~QPixmap();
}

void SceneRenderer::RenderScene()
{
    this->addPixmap(*this->SceneImage);
    this->Pots.AddObjectToScene(this);
    this->Cows.AddObjectToScene(this);
    //this->addWidget(&this->Pots);
}

void SceneRenderer::UnloadScene()
{
    this->clear();
}