#include "UI/SceneRenderer.h"

// Constructeur pour initialiser la structure
SceneInfo::SceneInfo(int PSceneID, int PGameID, SceneType PType)
{
    this->SceneID = PSceneID;
    this->Info = GetSceneMetaInfo(PSceneID, PGameID);
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


SceneRenderer::SceneRenderer(SceneInfo* SceneToRender, QTreeWidget* ObjectsTreeWidget) : QGraphicsScene()
{
	this->CurrScene = SceneToRender;
    this->ObjectsTree = ObjectsTreeWidget;
	if (SceneToRender)
	{
		for (size_t i = 0; i < SceneToRender->Objects->NumOfObjs; i++)
		{   // Browse each scene objects

			ObjectInfo* currObject = &SceneToRender->Objects->Objects[i];
            if (currObject->RenderScene != this->CurrScene->SceneID)
            {   // Ignore the object if the render scene ID is different from this scene ID

                continue;
            }

			ObjectRenderer* dest = this->FindObjectRendererCategory(currObject);

			if (dest != nullptr)
			{	// Add the object to the renderer

                dest->AddObjectToRender(currObject);
                dest->ObjCat->setExpanded(true);
			}
		}
	}

    QObject::connect(this->CurrScene, &SceneInfo::NotifyItemFound, this, &SceneRenderer::UpdateItemFound);
}

SceneRenderer::~SceneRenderer()
{
    this->CurrScene = nullptr;
    if (this->SceneImage)
        this->SceneImage->~QPixmap();
}

const char* SceneRenderer::GetSceneName()
{
    return this->CurrScene->Info->Name;
}

uint8_t SceneRenderer::GetSceneParentRegion()
{
    return this->CurrScene->Info->ParentRegion;
}

void SceneRenderer::RenderScene()
{
    if (this->SceneImage == nullptr)
    {   // This is the first time the scene is rendered

        this->SceneImage = new QPixmap(this->CurrScene->Info->ImagePath);
    }

    this->IsRendered = true;
    this->addPixmap(*this->SceneImage);

    ObjectInfo tmp;
    for (size_t i = 0; i <= ObjectType::fairy; i++)
    {
        tmp.Type = (ObjectType)i;
        ObjectRenderer* objRdr = FindObjectRendererCategory(&tmp);

        if (objRdr && objRdr->GetTotalObject() > 0)
        {
            objRdr->UpdateText();
            objRdr->AddObjectToScene(this);
            this->ObjectsTree->addTopLevelItem(objRdr->ObjCat);
            objRdr->ObjCat->setExpanded(true);
        }
    }

    this->ObjectsTree->sortItems(0, Qt::SortOrder::AscendingOrder);
}

void SceneRenderer::UnloadScene()
{
    this->IsRendered = false;

    ObjectInfo tmp;
    for (size_t i = 0; i <= ObjectType::fairy; i++)
    {
        tmp.Type = (ObjectType)i;
        ObjectRenderer* objRdr = FindObjectRendererCategory(&tmp);

        if (objRdr && objRdr->GetTotalObject() > 0)
        {
            objRdr->RemoveObjectFromList(this->ObjectsTree);
            objRdr->UnloadObjectsFromScene(this);
        }
    }
    //this->ObjectsTree->clear();
    //this->clear();
}

void SceneRenderer::UpdateItemFound(ObjectInfo* Object, const ItemInfo* ItemFound)
{
    Object->Item = ItemFound;
    ObjectRenderer* dest = this->FindObjectRendererCategory(Object);

    if (dest != nullptr)
    {	// Update the object renderer

        dest->UpdateObjectState(Object, this);
    }
}

ObjectRenderer* SceneRenderer::FindObjectRendererCategory(ObjectInfo* Object)
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



SceneItemTree::SceneItemTree(SceneInfo* SceneToRender, QTreeWidget* ObjectsTreeWidget, QTreeWidgetItem* Parent) : QTreeWidgetItem(Parent)
{
    this->Scene = new SceneRenderer(SceneToRender, ObjectsTreeWidget);
    this->setText(0, this->Scene->GetSceneName());
}


SceneItemTree::~SceneItemTree()
{
    this->Scene->~SceneRenderer();
    this->Scene = nullptr;
}


void SceneItemTree::RenderScene()
{
    this->Scene->RenderScene();
}


void SceneItemTree::UnloadScene()
{
    this->Scene->UnloadScene();
}