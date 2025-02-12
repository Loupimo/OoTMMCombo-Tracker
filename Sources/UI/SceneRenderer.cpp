#include "UI/SceneRenderer.h"
#include "UI/RegionTab.h"

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


SceneRenderer::SceneRenderer(SceneInfo* SceneToRender, QTreeWidget* ObjectsTreeWidget, SceneItemTree* Owner) : QGraphicsScene()
{
	this->CurrScene = SceneToRender;
    this->ObjectsTree = ObjectsTreeWidget;
    this->ItemOwner = Owner;

    for (size_t i = 0; i < ObjectType::fairy; i++)
    {   // Initializes all objects renderer

        this->ObjectsRen[i] = new ObjectRenderer((ObjectType)(i + 1), this);
    }

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

                dest->AddObjectToRender(currObject, this->ObjectsTree->palette().color(QPalette::Text));
                dest->ObjCat->setExpanded(true);
			}

            if (currObject->Status == ObjectState::Collected)
            {
                this->FoundObjs++;
            }
            this->TotalObjs++;
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
            objRdr->AddObjectToScene();
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
            objRdr->UnloadObjectsFromScene();
        }
    }
    //this->ObjectsTree->clear();
    //this->clear();
}

void SceneRenderer::RefreshObjectCounts(int Count)
{
    this->FoundObjs += Count;
    this->ItemOwner->RefreshObjectCounts(Count);
}

void SceneRenderer::UpdateItemFound(ObjectInfo* Object, const ItemInfo* ItemFound)
{
    Object->Item = ItemFound;
    ObjectRenderer* dest = this->FindObjectRendererCategory(Object);

    if (dest != nullptr)
    {	// Update the object renderer

        dest->UpdateObjectState(Object);
    }

    this->FoundObjs++;
    this->ItemOwner->RefreshObjectCounts(1);
}

ObjectRenderer* SceneRenderer::FindObjectRendererCategory(ObjectInfo* Object)
{
    if (Object->Type == ObjectType::none)
    {
        return nullptr;
    }
    return this->ObjectsRen[Object->Type - 1];
}



SceneItemTree::SceneItemTree(SceneInfo* SceneToRender, QTreeWidget* ObjectsTreeWidget, QTreeWidgetItem* Parent) : QTreeWidgetItem(Parent)
{
    this->Scene = new SceneRenderer(SceneToRender, ObjectsTreeWidget, this);
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


int SceneItemTree::GetCollectedObjects()
{
    return this->Scene->FoundObjs;
}

int SceneItemTree::GetTotalObjects()
{
    return this->Scene->TotalObjs;
}


void SceneItemTree::RefreshObjectCounts(int Count)
{
    ((RegionTree*)this->parent())->AddObjectCounts(Count, 0);
    ((RegionTree*)this->parent())->RefreshObjsCountText();

    const size_t max_size = 150;
    char finalName[max_size] = { 0 };
    char tmp[4] = { 0 };

    size_t offset = 0;
    size_t typeLen = strlen(this->Scene->GetSceneName());
    memcpy_s(finalName, max_size, this->Scene->GetSceneName(), typeLen);
    offset += typeLen;
    finalName[offset] = ' ';
    finalName[offset + 1] = '(';
    offset += 2;

    _itoa_s((int)this->GetCollectedObjects(), tmp, 10);

    memcpy_s(finalName + offset, max_size - offset, tmp, strlen(tmp));
    offset += strlen(tmp);

    finalName[offset] = ' ';
    finalName[offset + 1] = '/';
    finalName[offset + 2] = ' ';

    offset += 3;

    _itoa_s((int)this->GetTotalObjects(), tmp, 10);

    memcpy_s(finalName + offset, max_size - offset, tmp, strlen(tmp));

    offset += strlen(tmp);
    finalName[offset] = ')';
    finalName[offset + 1] = '\0';
    this->setText(0, finalName);
}