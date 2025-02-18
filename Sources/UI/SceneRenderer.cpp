#include "UI/SceneRenderer.h"
#include "UI/RegionTab.h"
#include "UI/MapTab.h"

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
    {   // Creates all objects renderer

        this->ObjectsRen[i] = nullptr;// new ObjectRenderer((ObjectType)(i + 1), this);
    }

    // Fills all objects renderer
    for (size_t i = 0; i < this->CurrScene->Objects->NumOfObjs; i++)
    {   // Browse each scene objects

        ObjectInfo* currObject = &this->CurrScene->Objects->Objects[i];
        if (currObject->RenderScene != this->CurrScene->SceneID)
        {   // Ignore the object if the render scene ID is different from this scene ID

            continue;
        }

        ObjectRenderer* dest = this->FindObjectRendererCategory(currObject);

        if (dest == nullptr)
        {
            this->ObjectsRen[currObject->Type - 1] = new ObjectRenderer(currObject->Type, this);
            dest = this->ObjectsRen[currObject->Type - 1];
        }

        if (dest != nullptr)
        {	// Add the object to the renderer

            dest->AddObjectToRender(currObject, this->ObjectsTree->palette().color(QPalette::Text));
            dest->ObjCat->setExpanded(true);
        }
    }

    //QObject::connect(this->CurrScene, &SceneInfo::NotifyItemFound, this, &SceneRenderer::UpdateItemFound);
}

SceneRenderer::~SceneRenderer()
{
    this->CurrScene = nullptr;
    if (this->SceneImage)
    {
        delete this->SceneImage;
        this->SceneImage = nullptr;
    }

    for (size_t i = 0; i < ObjectType::fairy; i++)
    {   // Destroys all objects renderer

        if (this->ObjectsRen[i])
        {
            delete this->ObjectsRen[i];
            this->ObjectsRen[i] = nullptr;
        }
    }
}

const char* SceneRenderer::GetSceneName()
{
    return this->CurrScene->Info->Name;
}

uint8_t SceneRenderer::GetSceneParentRegion()
{
    return this->CurrScene->Info->ParentRegion;
}


void SceneRenderer::RenderScene(bool Context)
{
    if (this->SceneImage == nullptr)
    {   // This is the first time the scene is rendered

        this->SceneImage = new QPixmap(this->CurrScene->Info->ImagePath);
        this->addPixmap(*this->SceneImage);
    }

    this->IsRendered = true;

    this->RefreshSceneContext(Context);

    this->ObjectsTree->sortItems(0, Qt::SortOrder::AscendingOrder);
}

void SceneRenderer::UnloadScene()
{
    this->IsRendered = false;

    ObjectInfo tmp;
    for (size_t i = 1; i <= ObjectType::fairy; i++)
    {
        tmp.Type = (ObjectType)i;
        ObjectRenderer* objRdr = FindObjectRendererCategory(&tmp);

        if (objRdr && objRdr->GetTotalObject() > 0)
        {
            objRdr->RemoveObjectFromList(this->ObjectsTree);
            objRdr->UnloadObjectsFromScene();
        }
    }
}

void SceneRenderer::RefreshSceneContext(bool Context)
{
    ObjectContext context = ObjectContext::All;

    if (this->ItemOwner->Scene->Info->HasContext)
    {
        if (this->ItemOwner->Scene->GameID == OOT_GAME)
        {
            context = Context ? ObjectContext::Adult : ObjectContext::Child;
        }
        else
        {
            context = Context ? ObjectContext::Spring : ObjectContext::Winter;
        }
    }

    ObjectInfo tmp;
    for (size_t i = 1; i <= ObjectType::fairy; i++)
    {
        tmp.Type = (ObjectType)i;
        ObjectRenderer* objRdr = FindObjectRendererCategory(&tmp);

        if (objRdr && objRdr->GetTotalObject() > 0)
        {
            objRdr->UpdateText();
            objRdr->AddObjectToScene(context);
            this->ObjectsTree->addTopLevelItem(objRdr->ObjCat);
            objRdr->ObjCat->setExpanded(true);
        }
    }
}

void SceneRenderer::RefreshObjectCounts(int Count)
{
    this->ItemOwner->FoundObjects += Count;
    this->ItemOwner->RefreshObjectCounts(Count);
}

void SceneRenderer::UpdateContext(ObjectContext Context)
{
    ((MapView*)this->views()[0])->UpdateContext(Context);
}
/*void SceneRenderer::UpdateItemFound(ObjectInfo* Object, const ItemInfo* ItemFound)
{
    Object->Item = ItemFound;
    ObjectRenderer* dest = this->FindObjectRendererCategory(Object);

    if (dest != nullptr)
    {	// Update the object renderer

        dest->UpdateObjectState(Object);
    }

    this->ItemOwner->RefreshObjectCounts(1);
}*/
void SceneRenderer::ItemFound(ObjectInfo* Object, const ItemInfo* ItemFound)
{
    Object->Item = ItemFound;

    if (Object->Status == ObjectState::Hidden)
    {   // The object is not already counted

        this->ItemOwner->FoundObjects++;
        this->ItemOwner->RefreshObjectCounts(1);
    }

    Object->Status = ObjectState::Collected;

    ObjectRenderer* dest = this->FindObjectRendererCategory(Object);

    if (dest != nullptr)
    {	// Update the object renderer

        dest->UpdateObjectState(Object);
        dest->UpdateText();
    }

}

ObjectRenderer* SceneRenderer::FindObjectRendererCategory(ObjectInfo* Object)
{
    if (Object->Type == ObjectType::none)
    {
        return nullptr;
    }
    return this->ObjectsRen[Object->Type - 1];
}


SceneItemTree::SceneItemTree(SceneInfo* SceneToRender, QTreeWidgetItem* Parent) : QTreeWidgetItem(Parent)
{
    this->Scene = SceneToRender;
    this->CountSceneObjects();
    this->RefreshObjectCounts(0);
}


void SceneItemTree::CountSceneObjects()
{
    this->FoundObjects = 0;
    this->TotalObjects = 0;
    for (size_t i = 0; i < this->Scene->Objects->NumOfObjs; i++)
    {   // Browse each scene objects

        ObjectInfo* currObject = &this->Scene->Objects->Objects[i];
        if (currObject->RenderScene != this->Scene->SceneID)
        {   // Ignore the object if the render scene ID is different from this scene ID

            continue;
        }

        this->TotalObjects++;  // We only add the object that should be rendered

        if (currObject->Status != ObjectState::Hidden)
        {   // The object is considered as found

            this->FoundObjects++;
        }
    }
}


SceneItemTree::~SceneItemTree()
{
    this->UnloadScene();
}


void SceneItemTree::RenderScene(QTreeWidget* ObjectsTreeWidget, bool Context)
{
    this->Renderer = new SceneRenderer(this->Scene, ObjectsTreeWidget, this);
    this->Renderer->RenderScene(Context);
}


void SceneItemTree::UnloadScene()
{
    if (this->Renderer)
    {
        delete this->Renderer;
        this->Renderer = nullptr;
    }
}

bool SceneItemTree::HasContext()
{
    return this->Scene->Info->HasContext;
}

const char* SceneItemTree::GetSceneName()
{
    return this->Scene->Info->Name;
}

int SceneItemTree::GetCollectedObjects()
{
    return this->FoundObjects;
}

int SceneItemTree::GetTotalObjects()
{
    return this->TotalObjects;
}

void SceneItemTree::RefreshObjectCounts(int Count)
{
    ((RegionTree*)this->parent())->AddObjectCounts(Count, 0);
    ((RegionTree*)this->parent())->RefreshObjsCountText();

    const size_t max_size = 150;
    char finalName[max_size] = { 0 };
    char tmp[4] = { 0 };

    size_t offset = 0;
    size_t typeLen = strlen(this->GetSceneName());
    memcpy_s(finalName, max_size, this->GetSceneName(), typeLen);
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


void SceneItemTree::ItemFound(ObjectInfo* Object, const ItemInfo* Item)
{
    if (this->Renderer)
    {   // The scene is rendered. We just need to call the scene item found function

        this->Renderer->ItemFound(Object, Item);
    }
    else
    {   // The scene is not rendered. We just need to update the object and the scene count

        if (Object->Status == ObjectState::Hidden)
        {   // The object is not already counted

            this->FoundObjects++;
            this->RefreshObjectCounts(1);
        }
        Object->Status = ObjectState::Collected;
        Object->Item = Item;
    }
}


void SceneRenderer::CenterViewOn(ObjectPixmapItem* Target)
{
    QGraphicsView* currView = this->views()[0];
    currView->resetTransform();

    // Récupérer la bounding box de l'élément
    QRectF itemRect = Target->sceneBoundingRect();

    // Calculer un facteur de zoom basé sur la taille de l'objet
    double scaleFactor = min(currView->viewport()->width() / itemRect.width(), currView->viewport()->height() / itemRect.height()) * 0.15;

    currView->scale(scaleFactor, scaleFactor);
    currView->centerOn(Target);
}