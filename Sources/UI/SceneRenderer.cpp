#include "UI/SceneRenderer.h"

// Constructeur pour initialiser la structure
SceneInfo::SceneInfo(int PSceneID, const char* PImage, int PGameID, SceneType PType)
{
    this->SceneID = PSceneID;
    this->Image = PImage;
    this->GameID = PGameID;
    this->Type = PType;

    size_t arrLen = 0;
    const ObjectInfo* arrObj = nullptr;

    this->Objects = &GetGameSceneObjects(PGameID)[this->SceneID];
    this->Objects->Owner = this;
}

// Destructeur pour libérer la mémoire
SceneInfo::~SceneInfo()
{
}


SceneRenderer::SceneRenderer(SceneInfo* SceneToRender) : QGraphicsScene()
{
	this->CurrScene = SceneToRender;
	if (SceneToRender)
	{
		for (size_t i = 0; i < SceneToRender->Objects->NumOfObjs; i++)
		{
			ObjectInfo* currObject = &SceneToRender->Objects->Objects[i];
            if (currObject->RenderScene != this->CurrScene->SceneID)
                continue;
			ObjectRenderer* dest = this->FindObjectRenderer(currObject);

			if (dest != nullptr)
			{	// Add the object to the renderer

				dest->AddObjectToRender(currObject->Position[0], currObject->Position[1]);
			}
		}
        this->SceneImage = new QPixmap(SceneToRender->Image);
	}

    connect(this->CurrScene, &SceneInfo::NotifyItemFound, this, &SceneRenderer::UpdateItemFound);
}

SceneRenderer::~SceneRenderer()
{
    this->CurrScene = nullptr;
    this->SceneImage->~QPixmap();
}

void SceneRenderer::RenderScene()
{
    this->IsRendered = true;
    this->addPixmap(*this->SceneImage);
    this->Pots.AddObjectToScene(this);
    this->Cows.AddObjectToScene(this);
    //this->addWidget(&this->Pots);
}

void SceneRenderer::UnloadScene()
{
    this->IsRendered = false;
    this->clear();
}

void SceneRenderer::UpdateItemFound(ObjectInfo* Object, const ItemInfo* ItemFound)
{
    ObjectRenderer* dest = this->FindObjectRenderer(Object);

    if (dest != nullptr)
    {	// Add the object to the renderer

        dest->UpdateObjectState(this);
    }
}

ObjectRenderer* SceneRenderer::FindObjectRenderer(ObjectInfo* Object)
{
    ObjectRenderer* renderer = nullptr;
    switch (Object->Type)
    {
        case ObjectType::pot:
        {
            renderer = &this->Pots;
            break;
        }
        case ObjectType::cow:
        {
            renderer = &this->Cows;
            break;
        }
        case ObjectType::grass:
        {
            renderer = &this->Grass;
            break;
        }
        case ObjectType::chest:
        {
            renderer = &this->Chests;
            break;
        }
        case ObjectType::collectible:
        {
            renderer = &this->Collectibles;
            break;
        }
        case ObjectType::npc:
        {
            renderer = &this->NPCs;
            break;
        }
        case ObjectType::gs:
        {
            renderer = &this->GoldSkulltulas;
            break;
        }
        case ObjectType::sf:
        {
            renderer = &this->StrayFairies;
            break;
        }
        case ObjectType::shop:
        {
            renderer = &this->Shops;
            break;
        }
        case ObjectType::scrub:
        {
            renderer = &this->Scrubs;
            break;
        }
        case ObjectType::sr:
        {
            renderer = &this->SilverRupees;
            break;
        }
        case ObjectType::fish:
        {
            renderer = &this->Fishes;
            break;
        }
        case ObjectType::wonder:
        {
            renderer = &this->Wonders;
            break;
        }
        case ObjectType::crate:
        {
            renderer = &this->Crates;
            break;
        }
        case ObjectType::hive:
        {
            renderer = &this->Hives;
            break;
        }
        case ObjectType::butterfly:
        {
            renderer = &this->Butterflies;
            break;
        }
        case ObjectType::rupee:
        {
            renderer = &this->Rupees;
            break;
        }
        case ObjectType::snowball:
        {
            renderer = &this->Snowballs;
            break;
        }
        case ObjectType::barrel:
        {
            renderer = &this->Barrels;
            break;
        }
        case ObjectType::heart:
        {
            renderer = &this->Hearts;
            break;
        }
        case ObjectType::fairy_spot:
        {
            renderer = &this->FairySpots;
            break;
        }
        case ObjectType::fairy:
        {
            renderer = &this->Fairies;
            break;
        }

        case ObjectType::none:
        default:
        {
            // Aucun traitement spécifique pour les types inconnus
            break;
        }
    }

    return renderer;
}