#include "UI/ObjectRenderer.h"
#include "UI/SceneRenderer.h"
#include <QGraphicsColorizeEffect>

static ObjectIcons* IconsRef = nullptr;

ObjectIcons::ObjectIcons()
{
    for (size_t i = 0; i < 23; i++)
    {
        this->PixmapIcons[i] = QPixmap(IconsMetaInfo[i].IconPath);
        //this->PixmapIcons[i] = this->PixmapIcons[i].scaled(IconsMetaInfo[i].Scale[0], IconsMetaInfo[i].Scale[1], Qt::KeepAspectRatio);

        this->Icons[i] = QIcon(IconsMetaInfo[i].IconPath);
    }
}


ObjectIcons::~ObjectIcons()
{
    for (size_t i = 0; i < 23; i++)
    {
        this->Icons[i].~QIcon();
        this->PixmapIcons[i].~QPixmap();
    }
}


void ObjectIcons::CreateObjectIcons()
{
    if (IconsRef == nullptr)
    {
        IconsRef = new ObjectIcons();
    }
}


ObjectItemTree::ObjectItemTree(ObjectInfo* Obj, QColor DefaultColor, ObjectRenderer* Owner, QTreeWidgetItem* Parent) : CommonBaseItemTree(Parent)
{
    this->Object = Obj;
    this->DefaultTextColor = DefaultColor;
    this->RendererOwner = Owner;
    if (Obj->Item)
    {
        this->Item.setText(0, Obj->Item->ItemName);
    }
    else
    {
        this->Item.setText(0, "Unknown");
    }
    this->setText(0, Object->Name);
    this->UpdateTextStyle();
}

ObjectItemTree::~ObjectItemTree()
{   
    if (this->GraphItem)
    {
        delete this->GraphItem;
        this->GraphItem = nullptr;
    }
}

ObjectState ObjectItemTree::GetStatus()
{
    return this->Object->Status;
}

void ObjectItemTree::UpdateIcon(ObjectType Type)
{
    if (this->GraphItem == nullptr && this->RendererOwner->ShouldBeRendered)
    {
        this->GraphItem = new ObjectPixmapItem(this->RendererOwner->Icon, this->RendererOwner, this);

        if (!this->Object->PosSet)
        {
            this->Object->Position[0] = this->Object->Position[0] - (this->RendererOwner->Icon.width() / 2);

            if (this->Object->Position[0] < 0)
            {
                this->Object->Position[0] = 0;
            }

            this->Object->Position[1] = this->Object->Position[1] - (this->RendererOwner->Icon.height() / 2);

            if (this->Object->Position[1] < 0)
            {
                this->Object->Position[1] = 0;
            }
            this->Object->PosSet = true;
        }
    }

    this->GraphItem->UpdateObjectRendering(this->Object->Status, false);
    this->GraphItem->setPos(this->Object->Position[0], this->Object->Position[1]);
    this->RendererOwner->SceneOwner->addItem(this->GraphItem);
    this->UpdateTextStyle();
}

void ObjectItemTree::RemoveObjectFromScene()
{
    if (this->GraphItem)
    {
        this->RendererOwner->SceneOwner->removeItem(this->GraphItem);
        delete this->GraphItem;
        this->GraphItem = nullptr;
    }
}

void ObjectItemTree::UpdateTextStyle()
{
    QFont font = this->font(0); // Get the actual font
    //QColor currentColor = this->foreground(0).color();

    if (this->GetStatus() != ObjectState::Hidden)
    {   // Object is considered as collected

        font.setStrikeOut(true);    // Cross out the text
        if (this->GetStatus() == ObjectState::Collected)
        {
            this->setForeground(0, QColor(this->DefaultTextColor.red(), this->DefaultTextColor.green(), this->DefaultTextColor.blue(), 128)); // (R, G, B, Alpha)
        }
        else
        {
            this->setForeground(0, QColor(147, 112, 249, 128)); // (R, G, B, Alpha)
        }
        if (this->Object->Item)
        {   // An item is associated to this object

            this->Item.setText(0, this->Object->Item->ItemName);
        }

        this->removeChild(&this->Item);     // Remove just in case the item was already in the list
        this->addChild(&this->Item);
    }
    else
    {   // Object is considered as hidden

        font.setStrikeOut(false);
        this->setForeground(0, QColor(this->DefaultTextColor.red(), this->DefaultTextColor.green(), this->DefaultTextColor.blue(), 255)); // (R, G, B, Alpha)

        this->removeChild(&this->Item);         // Remove just in case the item was already in the list
        this->setText(0, this->Object->Name);
    }

    this->setFont(0, font);
}


void ObjectItemTree::PerformAction()
{
    if (this->Object->Status == ObjectState::Hidden)
    {   // The item was hidden and should now be shown

        if (this->CalledFromGraph)
        {
            if (this->isSelected() == true)
            {   // The caller is the graph item. The object was already selected so we need to trigger the function in order to properly sets event flags.

                this->treeWidget()->itemSelectionChanged();
            }
            else
            {   // Change the object selection status

                this->setSelected(true);    // The itemSelectionChanged will be triggered.
            }
            return;
        }
        this->Object->Status = ObjectState::Forced;
        this->setExpanded(true);
        this->RendererOwner->SceneOwner->UpdateRoom(this->Object->RoomID);  // We need to update the room ID in case the selected object is in another room than the active one
        this->RendererOwner->UpdateSceneContext(this->Object->Context);     // We need to update the context in case the selected object is in a different context than the active one
        this->RendererOwner->RefreshObjectCounts(1);                        // Increase the number of discovered object by one
        this->RendererOwner->CenterViewOn(this->GraphItem);                 // Center the scene view on the object
        this->GraphItem->UpdateObjectRendering(this->Object->Status, true); // Apply opacity and effect to the selected object
    }
    else if (this->Object->Status == ObjectState::Forced)
    {   // The item was forced to be shown and should now be hidden

        if (this->CalledFromGraph)
        {   // Update the selection and trigger the event to correctly set the flags

            this->setSelected(false);
        }
        this->Object->Status = ObjectState::Hidden;
        this->setExpanded(false);
        this->RendererOwner->RefreshObjectCounts(-1);                       // Decrease the number of discovered object by one
        
        if (this->GraphItem)
        {   // It can be null when the object has been forced and is only present in a context / room that is different from the current active one

            this->GraphItem->UpdateObjectRendering(this->Object->Status, false);
        }
    }
    else
    {
        if (this->CalledFromGraph)
        {
            if (this->isSelected() == true)
            {   // The caller is the graph item. The object was already selected so we need to trigger the function in order to properly sets event flags.

                this->treeWidget()->itemSelectionChanged();
            }
            else
            {   // Change the object selection status

                this->setSelected(true);    // The itemSelectionChanged will be triggered.
            }
            return;
        }
        if (this->Object->RoomID != this->RendererOwner->SceneOwner->ActiveRoom)
        {
            this->RendererOwner->SceneOwner->UpdateRoom(this->Object->RoomID);  // We need to update the room ID in case the selected object is in another room than the active one
            this->RendererOwner->UpdateSceneContext(this->Object->Context);     // We need to update the context in case the selected object is in a different context than the active one
        }
        this->RendererOwner->CenterViewOn(this->GraphItem);                 // Center the scene view on the object
    }

    this->UpdateTextStyle();
}

void ObjectItemTree::ResetObjectEffect()
{
    if (this->GraphItem)
    {
        this->GraphItem->UpdateObjectRendering(this->Object->Status, false);
    }
}


ObjectPixmapItem::ObjectPixmapItem(const QPixmap& Pixmap, ObjectRenderer* Owner, ObjectItemTree* ItemOwner) : QGraphicsPixmapItem(Pixmap)
{
    this->Owner = Owner;
    this->ItemOwner = ItemOwner;
}

void ObjectPixmapItem::UpdateObjectRendering(ObjectState ObjStatus, bool IsSelected)
{
    this->setGraphicsEffect(nullptr);
    switch (ObjStatus)
    {
        // The object has been collected
        case ObjectState::Collected:
        {
            if (IsSelected)
            {   // Only update the effect when the object is selected

                QGraphicsColorizeEffect * forcedEffect = new QGraphicsColorizeEffect();
                forcedEffect->setColor(QColor(253, 218, 0)); // Yellow
                forcedEffect->setStrength(0.8);
                this->setGraphicsEffect(forcedEffect);
            }
            this->setOpacity(0.5);
            break;
        }

        // The object is forced to be reveal
        case ObjectState::Forced:
        {
            QGraphicsColorizeEffect * forcedEffect = new QGraphicsColorizeEffect();
            if (IsSelected)
            {
                forcedEffect->setColor(QColor(253, 218, 0)); // Yellow
                forcedEffect->setStrength(0.8);
            }
            else
            {
                forcedEffect->setColor(QColor(150, 0, 255)); // Purple
                forcedEffect->setStrength(0.8);
            }
            this->setGraphicsEffect(forcedEffect);
            this->setOpacity(0.65);
            break;
        }

        // The object is considered as not revealed
        default:
        {
            this->setOpacity(1);
            break;
        }
    }
}

void ObjectPixmapItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (this->Owner)
    {
        this->Owner->CenterViewOn(this);            // Center the scene view on this object
    }
    if (this->ItemOwner)
    {
        this->ItemOwner->SetCalledFromGraph(true);  // Update the caller status
        this->ItemOwner->PerformAction();           // Update the object
    }
    QGraphicsPixmapItem::mousePressEvent(event);
}


ObjectRenderer::ObjectRenderer(ObjectType Type, SceneRenderer* Owner)
{
    ObjectIcons::CreateObjectIcons();

    this->Type = Type;
    //this->Icon = IconsRef->PixmapIcons[this->Type];
    this->SceneOwner = Owner;
    this->ObjCat = new CommonBaseItemTree();
    QFont font = this->ObjCat->font(0);
    font.setBold(true);
    this->ObjCat->setFont(0, font);
    this->ObjCat->setText(0, ObjTypeName[this->Type]);
    this->ObjCat->setIcon(0, IconsRef->Icons[this->Type]);
}


ObjectRenderer::~ObjectRenderer()
{
    // This should be deleted by the clear function of the object tree
    for (ObjectItemTree* currObj : this->Objects)
    {
        //this->SceneOwner->ObjectsTree->removeItemWidget(currObj, 0);
        this->ObjCat->removeChild(currObj);
        delete currObj;
    }

    //this->Icon.~QPixmap();
    //this->SceneOwner->ObjectsTree->removeItemWidget(this->ObjCat, 0);
    delete this->ObjCat;
    this->SceneOwner = nullptr;
}


void ObjectRenderer::AddObjectToRender(ObjectInfo * Obj, QColor DefaultColor)
{
    /*if (!Obj->PosSet)
    {
        Obj->Position[0] = Obj->Position[0] - (this->Icon->width() / 2);

        if (Obj->Position[0] < 0)
        {
            Obj->Position[0] = 0;
        }

        Obj->Position[1] = Obj->Position[1] - (this->Icon->height() / 2);

        if (Obj->Position[1] < 0)
        {
            Obj->Position[1] = 0;
        }
        Obj->PosSet = true;
    }*/
    this->Objects.push_back(new ObjectItemTree(Obj, DefaultColor, this, this->ObjCat));
}


void ObjectRenderer::RenderObjectToScene(ObjectContext ActiveContext)
{
    if (this->ShouldBeRendered)
    {
        double scaleFactor = 0.02;
        int iconWidth = this->SceneOwner->SceneImage->rect().width() * scaleFactor/* + IconsMetaInfo[Type].Scale[0]*/;
        int iconHeight = this->SceneOwner->SceneImage->rect().height() * scaleFactor /* + IconsMetaInfo[Type].Scale[1]*/;

        iconWidth = iconWidth < IconsMetaInfo[this->Type].Scale[0] ? IconsMetaInfo[this->Type].Scale[0] : iconWidth;
        iconHeight = iconHeight < IconsMetaInfo[this->Type].Scale[1] ? IconsMetaInfo[this->Type].Scale[1] : iconWidth;

        this->Icon = IconsRef->PixmapIcons[this->Type].scaled(iconWidth, iconHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        for (ObjectItemTree* currObj : this->Objects)
        {   // Browse all objects

            if ((currObj->Object->Context == ObjectContext::All || currObj->Object->Context == ActiveContext) && (this->SceneOwner->ActiveRoom == -1 || currObj->Object->RoomID == this->SceneOwner->ActiveRoom))
            {   // The object can be rendered

                currObj->UpdateIcon(this->Type);
            }
            else
            {   // The object should not be rendered and removed if it previously was

                currObj->RemoveObjectFromScene();
            }
        }
    }
}


void ObjectRenderer::UnloadObjectsFromScene()
{
    if (this->ShouldBeRendered)
    {
        for (ObjectItemTree* currObj : this->Objects)
        {   // Browse all objects

            currObj->RemoveObjectFromScene();
        }
    }
}


void ObjectRenderer::RefreshObject(ObjectInfo* Object)
{
    for (size_t i = 0; i < this->Objects.size(); i++)
    {   // Browse all objects

        if (this->Objects[i]->Object == Object)
        {   // This is the object we need to refresh

            this->SceneOwner->UpdateRoom(Object->RoomID);  // We need to update the room ID in case the selected object is in another room than the active one
            this->UpdateSceneContext(Object->Context);     // We need to update the context in case the selected object is in a different context than the active one
            this->Objects[i]->UpdateIcon(this->Type);
            this->Objects[i]->PerformAction();
            break;
        }
    }
}


void ObjectRenderer::UpdateSceneContext(ObjectContext Context)
{
    this->SceneOwner->UpdateContext(Context);
}

void ObjectRenderer::CenterViewOn(ObjectPixmapItem* Target)
{
    this->SceneOwner->CenterViewOn(Target);
}


size_t ObjectRenderer::GetCollectedObject()
{
    size_t count = 0;

    for (ObjectItemTree* currObj : this->Objects)
    {   // Count all collected objects

        if (currObj->GetStatus() != ObjectState::Hidden)
        {
            count++;
        }
    }

    return count;
}


size_t ObjectRenderer::GetTotalObject()
{
    return this->Objects.size();
}


void ObjectRenderer::UpdateText()
{
    const size_t max_size = 150;
    char finalName[max_size] = { 0 };
    char tmp[4] = { 0 };

    // Initialize the string with : ObjectType (
    size_t offset = 0;
    size_t typeLen = strlen(ObjTypeName[this->Type]);
    memcpy_s(finalName, max_size, ObjTypeName[this->Type], typeLen);
    offset += typeLen;
    finalName[offset] = ' ';
    finalName[offset + 1] = '(';
    offset += 2;

    // Add the number of found object : ObjectType (foundObjs / 
    _itoa_s((int) this->GetCollectedObject(), tmp, 10);
    memcpy_s(finalName + offset, max_size - offset, tmp, strlen(tmp));
    offset += strlen(tmp);
    finalName[offset] = ' ';
    finalName[offset + 1] = '/';
    finalName[offset + 2] = ' ';
    offset += 3;

    // Add the total number of object : ObjectType (foundObjs / totObjs) 
    _itoa_s((int) this->GetTotalObject(), tmp, 10);
    memcpy_s(finalName + offset, max_size - offset, tmp, strlen(tmp));
    offset += strlen(tmp);
    finalName[offset] = ')';
    finalName[offset + 1] = '\0';
    this->ObjCat->setText(0, finalName);
}

void ObjectRenderer::RefreshObjectCounts(int Count)
{
    this->UpdateText();
    this->SceneOwner->UpdateObjectCounts(Count);
}


void ObjectRenderer::RemoveObjectFromList(QTreeWidget* Tree)
{
    Tree->takeTopLevelItem(Tree->indexOfTopLevelItem(this->ObjCat));
}