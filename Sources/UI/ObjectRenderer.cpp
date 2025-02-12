#include "UI/ObjectRenderer.h"
#include "UI/SceneRenderer.h"

static ObjectIcons* IconsRef = nullptr;

ObjectIcons::ObjectIcons()
{
    for (size_t i = 0; i < 23; i++)
    {
        this->PixmapIcons[i] = QPixmap(IconsMetaInfo[i].IconPath);
        this->PixmapIcons[i] = this->PixmapIcons[i].scaled(IconsMetaInfo[i].Scale[0], IconsMetaInfo[i].Scale[1], Qt::KeepAspectRatio);

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
        this->Item.setText(0, QString(Obj->Item->ItemName));
    }
    else
    {
        this->Item.setText(0, QString("Unknown"));
    }
    this->setText(0, Object->Name);
    this->UpdateTextStyle();
}

ObjectItemTree::~ObjectItemTree()
{
    this->RemoveObjectFromScene();
    //this->GraphItem = nullptr;  // Not free here as it should only points to singleton data
}

ObjectState ObjectItemTree::GetStatus()
{
    return this->Object->Status;
}

void ObjectItemTree::UpdateIcon(ObjectType Type)
{
    if (this->GraphItem == nullptr)
    {
        this->GraphItem = new QGraphicsPixmapItem(IconsRef->PixmapIcons[Type]);
    }
    this->GraphItem->setPos(this->Object->Position[0], this->Object->Position[1]);
    this->RendererOwner->SceneOwner->addItem(this->GraphItem);
    this->UpdateTextStyle();
}

void ObjectItemTree::RemoveObjectFromScene()
{
    this->RendererOwner->SceneOwner->removeItem(this->GraphItem);
    this->GraphItem->~QGraphicsPixmapItem();
    this->GraphItem = nullptr;
}

void ObjectItemTree::UpdateTextStyle()
{
    QFont font = this->font(0); // Get the actual font
    //QColor currentColor = this->foreground(0).color();

    if (this->GetStatus() != ObjectState::Hidden)
    {   // Object is considered as collected

        font.setStrikeOut(true);    // Cross out the text
        this->setForeground(0, QColor(this->DefaultTextColor.red(), this->DefaultTextColor.green(), this->DefaultTextColor.blue(), 128)); // (R, G, B, Alpha)

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

        this->Object->Status = ObjectState::Forced;
        this->setExpanded(true);
        this->RendererOwner->RefreshObjectCounts(1);
    }
    else if (this->Object->Status == ObjectState::Forced)
    {   // The item was shown and should now be hidden

        this->Object->Status = ObjectState::Hidden;
        this->setExpanded(false);
        this->RendererOwner->RefreshObjectCounts(-1);
    }

    this->UpdateTextStyle();
}


ObjectRenderer::ObjectRenderer(ObjectType Type, SceneRenderer* Owner)
{
    ObjectIcons::CreateObjectIcons();

    this->Type = Type;
    this->Icon = &IconsRef->PixmapIcons[this->Type];
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
    this->Icon = nullptr;
    for (ObjectItemTree * currObj : this->Objects)
    {
        currObj->~ObjectItemTree();
    }
}


void ObjectRenderer::AddObjectToRender(ObjectInfo * Obj, QColor DefaultColor)
{
    //this->Objects.push_back(Obj);
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

    this->Objects.push_back(new ObjectItemTree(Obj, DefaultColor, this, this->ObjCat));
}


void ObjectRenderer::AddObjectToScene()
{
    if (this->ShouldBeRendered)
    {
        for (ObjectItemTree* currObj : this->Objects)
        {   // Render all objects

            currObj->UpdateIcon(this->Type);
        }
    }
}


void ObjectRenderer::UnloadObjectsFromScene()
{
    if (this->ShouldBeRendered)
    {
        for (ObjectItemTree* currObj : this->Objects)
        {   // Render all objects

            currObj->RemoveObjectFromScene();
        }
    }
}


void ObjectRenderer::UpdateObjectState(ObjectInfo* Object)
{
    for (size_t i = 0; i < this->Objects.size(); i++)
    {
        if (this->Objects[i]->Object == Object)
        {
            this->Objects[i]->UpdateIcon(this->Type);
            break;
        }
    }
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

    size_t offset = 0;
    size_t typeLen = strlen(ObjTypeName[this->Type]);
    memcpy_s(finalName, max_size, ObjTypeName[this->Type], typeLen);
    offset += typeLen;
    finalName[offset] = ' ';
    finalName[offset + 1] = '(';
    offset += 2;

    _itoa_s((int) this->GetCollectedObject(), tmp, 10);

    memcpy_s(finalName + offset, max_size - offset, tmp, strlen(tmp));
    offset += strlen(tmp);

    finalName[offset] = ' ';
    finalName[offset + 1] = '/';
    finalName[offset + 2] = ' ';

    offset += 3;

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
    this->SceneOwner->RefreshObjectCounts(Count);
}


void ObjectRenderer::RemoveObjectFromList(QTreeWidget* Tree)
{
    Tree->takeTopLevelItem(Tree->indexOfTopLevelItem(this->ObjCat));
}