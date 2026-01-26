#include "UI/SceneRenderer.h"
#include "UI/RegionTab.h"
#include "UI/MapTab.h"
#include "UI/RoomRenderer.h"
#include "UI/AppConfig.h"
#include "UI/FilterManager.h"

#pragma region SceneItemTree

SceneInfo::SceneInfo(int PSceneID, int PGameID, SceneType PType)
{
    this->SceneID = PSceneID;
    this->GameID = PGameID;
    this->Info = GetSceneMetaInfo(PSceneID, PGameID);
    this->Type = PType;

    this->Objects = &GetGameSceneObjects(PGameID)[this->SceneID];
}

#pragma endregion

#pragma region SceneItemTree

SceneItemTree::SceneItemTree(SceneInfo* SceneToRender, FilterManager * Filter, QTreeWidgetItem* Parent) : QTreeWidgetItem(Parent)
{
    this->Scene = SceneToRender;
    this->CountSceneObjects();      // Count the scene objects
    this->UpdateObjectCounts(0);    // Used to refresh region counters and item name
    this->Filter = Filter;

    const QHash<int, std::vector<RoomInfo>>* tmp = GetSceneRooms(this->Scene);

    if (tmp->contains(this->Scene->SceneID))
    {   // The scene has several rooms

        for (size_t i = 0; i < tmp->value(this->Scene->SceneID).size(); i++)
        {   // Create the room info

            RoomInfo currRoom = tmp->value(this->Scene->SceneID).at(i);

            // Add a new room item to the map tree
            RoomItemTree* roomItem = new RoomItemTree(&currRoom, this);
            this->addChild(roomItem);
            this->Rooms.push_back(roomItem);
        }
    }
}


SceneItemTree::~SceneItemTree()
{
    this->UnloadScene();

    for (RoomItemTree* Room : this->Rooms)
    {   // Delete all scene's rooms
        
        delete Room;
    }
}

#pragma region Rendering

SceneRenderer* SceneItemTree::GetScene()
{
    return this->Renderer;
}

void SceneItemTree::RenderScene(QTreeWidget* ObjectsTreeWidget, bool Context, bool CreateNew)
{
    if (CreateNew)
    {   // We need to create a new renderer

        this->Renderer = new SceneRenderer(this->Scene, ObjectsTreeWidget, this, this->Filter);
    }
    else
    {
        if (this->ActiveRoom != nullptr)
        {   // Maybe the room has been changed by object selection. We need to unselect the previous room

            if (this->Rooms.size() > this->Renderer->ActiveRoom)
            {
                this->Rooms[this->Renderer->ActiveRoom]->setSelected(false);
            }
            this->Rooms[this->ActiveRoom->RoomID]->setSelected(true);
        }
    }

    // Render the scene
    this->Renderer->RenderScene(Context, this->ActiveRoom);
}


void SceneItemTree::UnloadScene()
{
    if (this->Renderer)
    {
        this->Renderer->UnloadScene();
        delete this->Renderer;
        this->Renderer = nullptr;
        this->ActiveRoom = nullptr;
    }
}

#pragma endregion

#pragma region  Scene context / Info

const char* SceneItemTree::GetSceneName()
{
    return this->Scene->Info->Name;
}


void SceneItemTree::UpdateRoom(uint32_t RoomID)
{
    if (this->Rooms.size() > RoomID)
    {
        this->ActiveRoom = &this->Rooms[RoomID]->Info;
    }
    else
    {
        this->ActiveRoom = nullptr;
    }
}


bool SceneItemTree::HasContext()
{
    return this->Scene->Info->HasContext;
}

#pragma endregion

#pragma region Objects related

int SceneItemTree::GetCollectedObjects()
{
    return this->FoundObjects;
}

int SceneItemTree::GetTotalObjects()
{
    return this->TotalObjects;
}


void SceneItemTree::ItemFound(ObjectInfo* Object, const ItemInfo* Item)
{
    if (Object->Scene != Object->RenderScene)
    {   // The rendering scene is not the same

        if (Object->Item == nullptr || Object->Item->ItemID != -1)
        {   // Only overwrite item if there is no existing item or the item is "Unknown"

            Object->Item = Item;
        }
        Object->Status = ObjectState::Collected;
        SceneObjects* currScenes = GetGameSceneObjects(this->Scene->GameID);
        
        for (size_t i = 0; i < currScenes[Object->RenderScene].NumOfObjs; i++)
        {
            ObjectInfo* currObj = &currScenes[Object->RenderScene].Objects[i];
            if (currObj->ObjectID == Object->ObjectID && currObj->Type == Object->Type && currObj->Scene == Object->Scene)
            {
                Object = currObj;
                break;
            }
        }
    }

    if (this->Renderer)
    {   // The scene is rendered. We just need to call the scene item found function

        if (Object->Status == ObjectState::Hidden)
        {   // The object is not already counted

            if (this->Rooms.size() > 0)
            {   // We can't use the room update function otherwise it will be counted twice

                this->Rooms[Object->RoomID]->FoundObjects++;
                this->Rooms[Object->RoomID]->RefreshItemName();
            };
        }
        this->Renderer->ItemFound(Object, Item);
        
    }
    else
    {   // The scene is not rendered. We just need to update the object and the scene count

        if (Object->Status == ObjectState::Hidden)
        {   // The object is not already counted

            if (this->Rooms.size() > 0)
            {
                this->Rooms[Object->RoomID]->UpdateObjectCounts(1);
            }
            else
            {
                this->FoundObjects++;
                this->UpdateObjectCounts(1);
            }
        }
        Object->Status = ObjectState::Collected;
        Object->Item = Item;
    }
}


void SceneItemTree::CountSceneObjects()
{
    this->FoundObjects = 0;
    this->TotalObjects = 0;
    for (size_t i = 0; i < this->Scene->Objects->NumOfObjs; i++)
    {   // Browse each scene objects

        ObjectInfo* currObject = &this->Scene->Objects->Objects[i];
        if (currObject->RenderScene != this->Scene->SceneID || currObject->Type == ObjectType::none)
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


void SceneItemTree::UpdateObjectCounts(int Count)
{
    // Update the region counters
    ((RegionTree*)this->parent())->AddObjectCounts(Count, 0);
    ((RegionTree*)this->parent())->RefreshObjsCountText();

    this->RefreshItemName();
}


void SceneItemTree::RefreshItemName()
{
    const size_t max_size = 150;
    char finalName[max_size] = { 0 };
    char tmp[4] = { 0 };

    // Initialize the string with : SceneName (
    size_t offset = 0;
    size_t typeLen = strlen(this->GetSceneName());
    memcpy_s(finalName, max_size, this->GetSceneName(), typeLen);
    offset += typeLen;
    finalName[offset] = ' ';
    finalName[offset + 1] = '(';
    offset += 2;

    // Add the number of found object : SceneName (foundObjs / 
    _itoa_s((int)this->GetCollectedObjects(), tmp, 10);
    memcpy_s(finalName + offset, max_size - offset, tmp, strlen(tmp));
    offset += strlen(tmp);
    finalName[offset] = ' ';
    finalName[offset + 1] = '/';
    finalName[offset + 2] = ' ';
    offset += 3;

    // Add the total number of object : SceneName (foundObjs / totObjs) 
    _itoa_s((int)this->GetTotalObjects(), tmp, 10);
    memcpy_s(finalName + offset, max_size - offset, tmp, strlen(tmp));
    offset += strlen(tmp);
    finalName[offset] = ')';
    finalName[offset + 1] = '\0';
    this->setText(0, finalName);
}

#pragma endregion

#pragma endregion

#pragma region SceneRenderer

SceneRenderer::SceneRenderer(SceneInfo* SceneToRender, QTreeWidget* ObjectsTreeWidget, SceneItemTree* Owner, FilterManager * Filter) : QGraphicsScene()
{
    this->ActiveRoom = -1;
    this->CurrScene = SceneToRender;
    this->ObjectsTree = ObjectsTreeWidget;
    this->ItemOwner = Owner;
    this->Filter = Filter;

    for (size_t i = 0; i < ObjectType::last - 1; i++)
    {   // Creates all objects renderer

        this->ObjectsRen[i] = nullptr;
    }

    // Fills all objects renderer
    for (size_t i = 0; i < this->CurrScene->Objects->NumOfObjs; i++)
    {   // Browse each scene objects

        ObjectInfo* currObject = &this->CurrScene->Objects->Objects[i];
        if (currObject->RenderScene != this->CurrScene->SceneID || currObject->Type == ObjectType::none)
        {   // Ignore the object if the render scene ID is different from this scene ID

            continue;
        }

        ObjectRenderer* dest = this->FindObjectRendererCategory(currObject);

        if (dest == nullptr)
        {   // The object renderer for this type of object doesn't exist yet

            this->ObjectsRen[currObject->RenderType - 1] = new ObjectRenderer(currObject->RenderType, this, this->Filter->ActiveFilter.contains(currObject->RenderType));
            dest = this->ObjectsRen[currObject->RenderType - 1];
            this->ObjectsTree->addTopLevelItem(dest->ObjCat);
        }

        if (dest != nullptr)
        {	// Add the object to the existing renderer

            dest->AddObjectToRender(currObject, this->ObjectsTree->palette().color(QPalette::Text));
            dest->ObjCat->setExpanded(true);
        }
    }
}


SceneRenderer::~SceneRenderer()
{
    this->CurrScene = nullptr;  // Do not destroy this, is it static data
    this->Filter = nullptr;

    if (this->SceneImage)
    {
        delete this->SceneImage;
        this->SceneImage = nullptr;
    }

    for (size_t i = 0; i < ObjectType::last - 1; i++)
    {   // Destroys all objects renderer

        if (this->ObjectsRen[i])
        {
            delete this->ObjectsRen[i];
            this->ObjectsRen[i] = nullptr;
        }
    }
}

#pragma region Rendering

void SceneRenderer::RenderScene(bool Context, RoomInfo* Room)
{
    const char* path = this->CurrScene->Info->ImagePath;

    if (Room != nullptr && this->ActiveRoom != Room->RoomID)
    {   // The active room has changed

        delete this->SceneImage;            // Delete the previously rendered scene image
        path = Room->ImagePath;             // Change the scene image path
        this->ActiveRoom = Room->RoomID;    // Change the scene renderer active room ID
        this->SceneImage = nullptr;
    }

    if (this->SceneImage == nullptr)
    {   // This is the first time the scene is rendered

        this->SceneImage = new QPixmap(path);
        this->addPixmap(*this->SceneImage);
    }

    // Refresh the scene content based on the context
    this->RefreshSceneContext(Context);

    this->ObjectsTree->sortItems(0, Qt::SortOrder::AscendingOrder);
}


void SceneRenderer::UnloadScene()
{

    ObjectInfo tmp;
    for (size_t i = 1; i < ObjectType::last; i++)
    {   // Browse all type of objects

        tmp.Type = (ObjectType)i;
        tmp.RenderType = (ObjectType)i;
        ObjectRenderer* objRdr = FindObjectRendererCategory(&tmp);

        if (objRdr && objRdr->GetTotalObject() > 0)
        {   // Remove all object from the object list and the scene

            objRdr->RemoveObjectFromList(this->ObjectsTree);
            objRdr->UnloadObjectsFromScene();
            objRdr->ShouldBeRendered = false;
        }
    }
}


void SceneRenderer::CenterViewOn(ObjectPixmapItem* Target)
{
    if (AppConfig::GetAutoSnapView() && Target != nullptr)
    {
        QGraphicsView* currView = this->views()[0];
        currView->resetTransform();

        // Find the target bounding box
        QRectF itemRect = Target->sceneBoundingRect();

        // Compute a zoom factor based on the object size
        if (AppConfig::GetAutoZoom())
        {
            double scaleFactor = min(currView->viewport()->width() / itemRect.width(), currView->viewport()->height() / itemRect.height()) * 0.15;
            currView->scale(scaleFactor, scaleFactor);
        }

        currView->centerOn(Target);
    }
}

#pragma endregion

#pragma region Objects related

void SceneRenderer::UpdateObjectCounts(ObjectItemTree * Caller, int Count)
{
    this->ItemOwner->FoundObjects += Count;
    if (this->ItemOwner->ActiveRoom)
    {
        this->ItemOwner->Rooms[Caller->Object->RoomID]->UpdateObjectCounts(Count);
    }
    else
    {
        this->ItemOwner->UpdateObjectCounts(Count);
    }
}


void SceneRenderer::ItemFound(ObjectInfo* Object, const ItemInfo* ItemFound)
{
    Object->Item = ItemFound;

    if (Object->Status == ObjectState::Hidden)
    {   // The object is not already counted

        this->ItemOwner->FoundObjects++;
        this->ItemOwner->UpdateObjectCounts(1);
    }

    Object->Status = ObjectState::Collected;

    ObjectRenderer* dest = this->FindObjectRendererCategory(Object);

    if (dest != nullptr)
    {	// Update the object renderer

        dest->RefreshObject(Object);
        dest->UpdateText();
    }

}


ObjectRenderer* SceneRenderer::FindObjectRendererCategory(ObjectInfo* Object)
{
    if (Object->RenderType == ObjectType::none)
    {
        return nullptr;
    }
    return this->ObjectsRen[Object->RenderType - 1];
}

#pragma endregion

#pragma region Scene context / Info

const char* SceneRenderer::GetSceneName()
{
    return this->CurrScene->Info->Name;
}


uint8_t SceneRenderer::GetSceneParentRegion()
{
    return this->CurrScene->Info->ParentRegion;
}


void SceneRenderer::UpdateRoom(uint32_t RoomID)
{
    this->ItemOwner->UpdateRoom(RoomID);
}


void SceneRenderer::UpdateContext(ObjectContext Context)
{
    ((MapView*)this->views()[0])->UpdateContext(Context);
}


void SceneRenderer::UpdateSceneObjectVisibility()
{
    for (size_t i = 0; i < ObjectType::last - 1; i++)
    {
        if (this->ObjectsRen[i] != nullptr)
        {   // The scene contains object of this type

            this->ObjectsRen[i]->ShouldBeRendered = this->Filter->ActiveFilter.contains((ObjectType) (i + 1));

            if (this->ObjectsRen[i]->ShouldBeRendered && this->ObjectsRen[i]->ObjCat->treeWidget() == nullptr)
            {   // The object category should be added to the tree

                this->ObjectsTree->addTopLevelItem(this->ObjectsRen[i]->ObjCat);
            }
            this->ObjectsRen[i]->RenderObjectToScene(this->CurrContext);
        }
    }

    this->ObjectsTree->sortItems(0, Qt::AscendingOrder);
}


void SceneRenderer::RefreshSceneContext(bool Context)
{
    if (this->ItemOwner->Scene->Info->HasContext)
    {
        if (this->ItemOwner->Scene->GameID == OOT_GAME)
        {
            this->CurrContext = Context ? ObjectContext::Adult : ObjectContext::Child;
        }
        else
        {
            this->CurrContext = Context ? ObjectContext::Spring : ObjectContext::Winter;
        }
    }
    else
    {
        this->CurrContext = ObjectContext::All;
    }

    ObjectInfo tmp;
    for (size_t i = 1; i < ObjectType::last; i++)
    {
        tmp.Type = (ObjectType)i;
        tmp.RenderType = (ObjectType)i;
        ObjectRenderer* objRdr = FindObjectRendererCategory(&tmp);

        if (objRdr && objRdr->GetTotalObject() > 0)
        {   // We only render object that are valid

            //objRdr->ShouldBeRendered = true;
            objRdr->UpdateText();
            objRdr->RenderObjectToScene(this->CurrContext);
            //this->ObjectsTree->addTopLevelItem(objRdr->ObjCat);
            objRdr->ObjCat->setExpanded(true);
        }
    }
}

#pragma endregion

#pragma endregion
