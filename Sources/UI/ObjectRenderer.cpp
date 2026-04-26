#include "UI/ObjectRenderer.h"
#include "UI/SceneRenderer.h"
#include <QGraphicsColorizeEffect>
#include <QGraphicsSceneHoverEvent>
#include <QToolTip>
#include <QCursor>
#include <QApplication>
#include <QFontMetrics>
#include <QPainter>
#include <QStyle>
#include <UI/AppConfig.h>
#include "UI/FilterManager.h"

static ObjectIcons* IconsRef = nullptr;

ObjectIcons::ObjectIcons()
{
    for (size_t i = 0; i < ObjectType::last; i++)
    {
        this->PixmapIcons[i] = QPixmap(IconsMetaInfo[i].IconPath);
        this->Icons[i] = QIcon(IconsMetaInfo[i].IconPath);
    }

    for (size_t i = 0; i < ObjectIconMap::type; i++)
    {
        this->PixmapSpeIcons[i] = QPixmap(SpecificIconsMetaInfo[i].IconPath);
    }

    for (size_t i = 0; i < EntranceIcons::Entrance_Last; i++)
    {
        this->EntranceIcons[i] = QIcon(EntranceIconsMetaInfo[i].IconPath);
        this->PixmapEntranceIcons[i] = QPixmap(EntranceIconsMetaInfo[i].IconPath);
    }
}


ObjectIcons::~ObjectIcons()
{
    for (size_t i = 0; i < ObjectType::last; i++)
    {
        this->Icons[i].~QIcon();
        this->PixmapIcons[i].~QPixmap();
    }

    for (size_t i = 0; i < ObjectIconMap::type; i++)
    {
        this->PixmapSpeIcons[i].~QPixmap();
    }
}


void ObjectIcons::CreateObjectIcons()
{
    if (IconsRef == nullptr)
    {
        IconsRef = new ObjectIcons();
    }
}


QIcon* ObjectIcons::GetObjectIcons()
{
    if (IconsRef == nullptr)
    {
        IconsRef = new ObjectIcons();
    }

    return IconsRef->Icons;
}


ObjectIcons* ObjectIcons::GetIconsRef()
{
    if (IconsRef == nullptr)
    {
        IconsRef = new ObjectIcons();
    }

    return IconsRef;
}

ObjectItemTree::ObjectItemTree(ObjectInfo* Obj, QColor DefaultColor, ObjectRenderer* Owner, QTreeWidgetItem* Parent) : CommonBaseItemTree(Parent)
{
    this->Object = Obj;
    this->DefaultTextColor = DefaultColor;
    this->RendererOwner = Owner;
    if (Obj->Item)
    {
        this->Item.setText(0, Obj->Item->ItemName);
        this->setData(0, ItemNameRole, QString(Obj->Item->ItemName));
    }
    else
    {
        this->Item.setText(0, "Unknown");
        this->setData(0, ItemNameRole, QString());
    }
    this->setText(0, Object->Name);
    this->setData(0, ObjectStatusRole, static_cast<int>(this->Object->Status));

    // Per-row icon resolved from the object's MapIcon: specific icon when set,
    // otherwise the RenderType / Type icon. The categories keep their own icon —
    // only the leaves use RenderType so e.g. each mask shows its actual artwork
    // instead of the generic "MASK" category icon.
    ObjectIcons* iconsRef = ObjectIcons::GetIconsRef();
    if (iconsRef)
    {
        if (Obj->MapIcon == ObjectIconMap::type)
        {
            this->setIcon(0, iconsRef->Icons[Obj->Type]);
        }
        else if (Obj->MapIcon == ObjectIconMap::render_type)
        {
            this->setIcon(0, iconsRef->Icons[Obj->RenderType]);
        }
        else
        {
            const QPixmap& spe = iconsRef->PixmapSpeIcons[Obj->MapIcon];
            if (!spe.isNull())
            {
                this->setIcon(0, QIcon(spe));
            }
            else
            {
                this->setIcon(0, iconsRef->Icons[Obj->RenderType]);
            }
        }
    }

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
        double scaleFactor = 0.02;
        int iconWidth = this->RendererOwner->SceneOwner->SceneImage->rect().width() * scaleFactor;
        int iconHeight = this->RendererOwner->SceneOwner->SceneImage->rect().height() * scaleFactor;

        switch (this->Object->MapIcon)
        {
            case ObjectIconMap::type:
            case ObjectIconMap::render_type:
            {   // This is a common icon

                iconWidth = this->RendererOwner->Icon.width();
                iconHeight = this->RendererOwner->Icon.height();

                this->GraphItem = new ObjectPixmapItem(this->RendererOwner->Icon, this->RendererOwner, this);
                this->GraphItem->setOpacity(IconsMetaInfo[Type].Alpha);
                break;
            }

            default:
            {   // This is a specific icon

                iconWidth = iconWidth < SpecificIconsMetaInfo[this->Object->MapIcon].Scale[0] ? SpecificIconsMetaInfo[this->Object->MapIcon].Scale[0] : iconWidth > SpecificIconsMetaInfo[this->Object->MapIcon].MaxScale[0] ? IconsMetaInfo[this->Type].MaxScale[0] : iconWidth;
                iconHeight = iconHeight < SpecificIconsMetaInfo[this->Object->MapIcon].Scale[1] ? SpecificIconsMetaInfo[this->Object->MapIcon].Scale[1] : iconHeight > SpecificIconsMetaInfo[this->Object->MapIcon].MaxScale[1] ? SpecificIconsMetaInfo[this->Object->MapIcon].MaxScale[1] : iconHeight;

                this->GraphItem = new ObjectPixmapItem(IconsRef->PixmapSpeIcons[this->Object->MapIcon].scaled(iconWidth, iconHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation), this->RendererOwner, this);
                break;
            }
        }

        if (!this->Object->PosSet)
        {
            this->Object->Position[0] = this->Object->Position[0] - (iconWidth / 2);

            if (this->Object->Position[0] < 0)
            {
                this->Object->Position[0] = 0;
            }

            this->Object->Position[1] = this->Object->Position[1] - (iconHeight / 2);

            if (this->Object->Position[1] < 0)
            {
                this->Object->Position[1] = 0;
            }
            this->Object->PosSet = true;
        }
    }

    if (this->GraphItem != nullptr)
    {   // Only valid if the item is rendered

        this->GraphItem->UpdateObjectRendering(this->Object->Status, false);
        this->GraphItem->setPos(this->Object->Position[0], this->Object->Position[1]);
        this->GraphItem->setZValue(this->Object->Position[2]);

        if (this->GraphItem->scene() == nullptr)
        {   // Only add the item if it is not already in the scene

            if (this->Object->Type == ObjectType::grass)
            {

            }

            this->RendererOwner->SceneOwner->addItem(this->GraphItem);
        }
    }

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

void ObjectItemTree::SetSceneObjectVisibility(bool Visibility)
{
    if (this->GraphItem)
    {
        this->GraphItem->setVisible(Visibility);
    }
}

void ObjectItemTree::SetListObjectVisibility(bool Visibility)
{
    this->setHidden(!Visibility);
}

void ObjectItemTree::UpdateTextStyle()
{
    QFont font = this->font(0); // Get the actual font

    // Object name is shown on the upper line and the item name on the lower line
    // by ObjectItemDelegate. The state (Hidden / Forced / Collected) drives the
    // strikethrough + color in the delegate; we just keep the data in sync here.
    if (this->GetStatus() != ObjectState::Hidden)
    {   // Object is considered as collected (Forced or Collected)

        font.setStrikeOut(true);
        if (this->GetStatus() == ObjectState::Collected)
        {
            this->setForeground(0, QColor(this->DefaultTextColor.red(), this->DefaultTextColor.green(), this->DefaultTextColor.blue(), 128));
        }
        else
        {
            this->setForeground(0, QColor(147, 112, 249, 128));
        }
        if (this->Object->Item)
        {
            this->Item.setText(0, this->Object->Item->ItemName);
        }
    }
    else
    {   // Object is considered as hidden

        font.setStrikeOut(false);
        this->setForeground(0, QColor(this->DefaultTextColor.red(), this->DefaultTextColor.green(), this->DefaultTextColor.blue(), 255));
    }

    this->setText(0, this->Object->Name);
    this->setData(0, ItemNameRole, this->Object->Item ? QString(this->Object->Item->ItemName) : QString());
    this->setData(0, ObjectStatusRole, static_cast<int>(this->Object->Status));
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
        this->RendererOwner->SceneOwner->UpdateRoom(this->Object->RoomID);  // We need to update the room ID in case the selected object is in another room than the active one
        this->RendererOwner->UpdateSceneContext(this->Object->Context);     // We need to update the context in case the selected object is in a different context than the active one
        this->RendererOwner->RefreshObjectCounts(this, 1);                  // Increase the number of discovered object by one
        this->RendererOwner->CenterViewOn(this->GraphItem);                 // Center the scene view on the object
        if (this->GraphItem)
        {   // It can be null when collected object should be hidden

            this->GraphItem->UpdateObjectRendering(this->Object->Status, true); // Apply opacity and effect to the selected object
        }
    }
    else if (this->Object->Status == ObjectState::Forced)
    {   // The item was forced to be shown and should now be hidden

        if (this->CalledFromGraph)
        {   // Update the selection and trigger the event to correctly set the flags

            this->setSelected(false);
        }
        this->Object->Status = ObjectState::Hidden;
        this->RendererOwner->RefreshObjectCounts(this, -1);                 // Decrease the number of discovered object by one
        
        if (this->GraphItem)
        {   // It can be null when the object has been forced and is only present in a context / room that is different from the current active one

            this->GraphItem->UpdateObjectRendering(this->Object->Status, false);
        }
        else
        {
            this->UpdateIcon(this->Object->RenderType);
        }
    }
    else
    {   // The item was forced or hidden and is now considered as collected

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

        if (AppConfig::GetAutoSnapView())
        {
            // Expand only the parent category so the row is visible — the row itself
            // no longer has children, so expanding it would just toggle a chevron.
            this->parent()->setExpanded(true);
            this->treeWidget()->scrollToItem(this, QAbstractItemView::PositionAtCenter);
            this->treeWidget()->setCurrentItem(this); // The PerformAction will be triggered
        }
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


const char* ObjectItemTree::GetObjectToolTip()
{
    return this->Object->Tooltip;
}


int ObjectItemTree::GetTotalObjectAvailable()
{
    return 1;
}


ObjectPixmapItem::ObjectPixmapItem(const QPixmap& Pixmap, ObjectRenderer* Owner, ObjectItemTree* ItemOwner) : QGraphicsPixmapItem(Pixmap)
{
    this->Owner = Owner;
    this->ItemOwner = ItemOwner;

    this->setAcceptHoverEvents(true);
    this->TimerStart = false;
    this->setToolTip(this->ItemOwner->GetObjectToolTip());
    /*this->TimerStart = false;
    this->HoverTimer = new QTimer();
    this->HoverTimer->setSingleShot(true);
    this->HoverTimer->setInterval(600); // 600 ms = temps à attendre

    // Quand le timer expire → montrer le tooltip
    QObject::connect(this->HoverTimer, &QTimer::timeout, [this]()
        {
        if (this->ItemOwner->GetObjectToolTip() != NULL)
        {
            QToolTip::showText(QCursor::pos(), this->ItemOwner->GetObjectToolTip());
        }
    });*/
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
            this->setOpacity(IconsMetaInfo[this->Owner->Type].Alpha);
            //this->setOpacity(1);
            break;
        }
    }
}

void ObjectPixmapItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    event->accept();

    if (this->Owner)
    {
        this->Owner->CenterViewOn(this);            // Center the scene view on this object
    }
    if (this->ItemOwner)
    {
        this->ItemOwner->SetCalledFromGraph(true);  // Update the caller status
        this->ItemOwner->PerformAction();           // Update the object
    }
}


void ObjectPixmapItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    /*QGraphicsPixmapItem::hoverEnterEvent(event);
    if (this->TimerStart == false && this->ItemOwner->GetObjectToolTip() != NULL)
    {
        //this->HoverTimer->start();
        this->TimerStart = true;
        QToolTip::showText(QCursor::pos(), QString(this->ItemOwner->GetObjectToolTip()), this, {}, 100000000000);
    }
    */
}


void ObjectPixmapItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    // Empêche la disparition : ne relance pas le timer !
    QGraphicsPixmapItem::hoverMoveEvent(event);
}


void ObjectPixmapItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    //this->HoverTimer->stop();
    /*this->TimerStart = false;
    QToolTip::hideText();
    QGraphicsPixmapItem::hoverLeaveEvent(event);*/
}


ObjectRenderer::ObjectRenderer(ObjectType Type, SceneRenderer* Owner, bool ShouldBeRendered)
{
    ObjectIcons::CreateObjectIcons();

    this->Type = Type;
    this->ShouldBeRendered = ShouldBeRendered;
    this->SceneOwner = Owner;
    this->ObjCat = new CommonBaseItemTree();
    QFont font = this->ObjCat->font(0);
    font.setBold(true);
    font.setCapitalization(QFont::AllUppercase);
    this->ObjCat->setFont(0, font);
    this->ObjCat->setFont(1, font);
    this->ObjCat->setText(0, ObjTypeName[this->Type]);
    this->ObjCat->setIcon(0, IconsRef->Icons[this->Type]);
    this->ObjCat->setTextAlignment(1, Qt::AlignRight | Qt::AlignVCenter);

    // Section-header look (object categories / parents in the object tree)
    int gameID = (Owner && Owner->CurrScene) ? Owner->CurrScene->GameID : OOT_GAME;
    QColor accent(gameID == OOT_GAME ? "#4a9edb" : "#9b5de5");
    QColor bg = accent; bg.setAlpha(64);
    this->ObjCat->setBackground(0, QBrush(bg));
    this->ObjCat->setBackground(1, QBrush(bg));
    this->ObjCat->setForeground(0, QBrush(accent));
    this->ObjCat->setForeground(1, QBrush(accent));
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
    this->Objects.push_back(new ObjectItemTree(Obj, DefaultColor, this, this->ObjCat));
}


void ObjectRenderer::RenderObjectToScene(ObjectContext ActiveContext)
{
    if (this->ShouldBeRendered)
    {
        this->ObjCat->setHidden(false);

        double scaleFactor = 0.02;
        int iconWidth = this->SceneOwner->SceneImage->rect().width() * scaleFactor;
        int iconHeight = this->SceneOwner->SceneImage->rect().height() * scaleFactor;

        iconWidth = iconWidth < IconsMetaInfo[this->Type].Scale[0] ? IconsMetaInfo[this->Type].Scale[0] : iconWidth > IconsMetaInfo[this->Type].MaxScale[0] ? IconsMetaInfo[this->Type].MaxScale[0] : iconWidth;
        iconHeight = iconHeight < IconsMetaInfo[this->Type].Scale[1] ? IconsMetaInfo[this->Type].Scale[1] : iconHeight > IconsMetaInfo[this->Type].MaxScale[1] ? IconsMetaInfo[this->Type].MaxScale[1] : iconHeight;

        this->Icon = IconsRef->PixmapIcons[this->Type].scaled(iconWidth, iconHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        for (ObjectItemTree* currObj : this->Objects)
        {   // Browse all objects

            if (AppConfig::GetHideCollectedFromMap() && currObj->GetStatus() != ObjectState::Hidden)
            {   // The object is not hidden and should be hidden when collected

                //currObj->RemoveObjectFromScene();
                currObj->SetSceneObjectVisibility(false);
            }
            else if ((currObj->Object->Context == ObjectContext::All || currObj->Object->Context == ActiveContext) && (this->SceneOwner->ActiveRoom == -1 || currObj->Object->RoomID == this->SceneOwner->ActiveRoom))
            {   // The object can be rendered

                currObj->SetSceneObjectVisibility(true);
                currObj->UpdateIcon(this->Type);
            }
            else
            {   // The object should not be rendered and removed if it previously was

                currObj->RemoveObjectFromScene();
            }

            if (AppConfig::GetHideCollectedFromObjectList() && currObj->GetStatus() != ObjectState::Hidden)
            {   // The object is not hidden and should be hidden from the list when collected

                currObj->SetListObjectVisibility(false);
            }
            else
            {
                currObj->SetListObjectVisibility(true);
            }
        }
    }
    else
    {
        for (ObjectItemTree* currObj : this->Objects)
        {   // Browse all objects

            currObj->SetSceneObjectVisibility(false);
        }

        this->ObjCat->setHidden(true);
    }
}


void ObjectRenderer::UnloadObjectsFromScene()
{
    for (ObjectItemTree* currObj : this->Objects)
    {   // Browse all objects

        currObj->RemoveObjectFromScene();
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
    SetCountedTreeLabel(this->ObjCat, ObjTypeName[this->Type], (uint32_t)this->GetCollectedObject(), (uint32_t)this->GetTotalObject());
}

void ObjectRenderer::RefreshObjectCounts(ObjectItemTree * Caller, int Count)
{
    this->UpdateText();
    this->SceneOwner->UpdateObjectCounts(Caller, Count);
}


void ObjectRenderer::RemoveObjectFromList(QTreeWidget* Tree)
{
    Tree->takeTopLevelItem(Tree->indexOfTopLevelItem(this->ObjCat));
}


void CountObjectsMatching(ObjectInfo* Objects, size_t Count, FilterManager* Filter, GameLayout Layout, uint32_t SceneID, int RoomID, int* FoundOut, int* TotalOut)
{
    *FoundOut = 0;
    *TotalOut = 0;

    for (size_t i = 0; i < Count; i++)
    {   // Browse each object

        ObjectInfo* currObj = &Objects[i];

        if (currObj->Type == ObjectType::none || currObj->RenderScene != SceneID || !currObj->HasCorrectLayout(Layout) || Filter->IsObjectExcluded(currObj))
        {   // Skip excluded or mismatched objects

            continue;
        }

        if (RoomID != -1 && currObj->RoomID != RoomID)
        {   // Skip if filtering by room and it doesn't match

            continue;
        }

        (*TotalOut)++;

        if (currObj->Status != ObjectState::Hidden)
        {   // The object is considered as found

            (*FoundOut)++;
        }
    }
}


#pragma region ObjectItemDelegate

namespace {
    constexpr int kRowHeight       = 46;   // Total row height for object rows.
    constexpr int kIconSize        = 32;   // Square icon dimension.
    constexpr int kPadX            = 8;    // Horizontal padding inside the row.
    constexpr int kIconTextSpacing = 10;   // Gap between the icon block and the text block.
    constexpr int kLineSpacing     = 2;    // Vertical gap between the object name and the item name.
}


QSize ObjectItemDelegate::sizeHint(const QStyleOptionViewItem& Option, const QModelIndex& Index) const
{
    if (!Index.parent().isValid())
    {   // Top-level (category) row: keep the default sizing.

        return QStyledItemDelegate::sizeHint(Option, Index);
    }

    QSize def = QStyledItemDelegate::sizeHint(Option, Index);
    return QSize(def.width(), kRowHeight);
}


void ObjectItemDelegate::paint(QPainter* Painter, const QStyleOptionViewItem& Option, const QModelIndex& Index) const
{
    if (!Index.parent().isValid())
    {   // Top-level (category) row: keep the default rendering so the existing
        // CHEST / GRASS / ... headers stay untouched.

        QStyledItemDelegate::paint(Painter, Option, Index);
        return;
    }

    if (Index.column() != 0)
    {   // Object rows only carry data on column 0. The custom layout paints across
        // the row from column 0; secondary columns just get the standard background
        // so selection / hover highlights still cover them.

        QStyledItemDelegate::paint(Painter, Option, Index);
        return;
    }

    QStyleOptionViewItem opt = Option;
    initStyleOption(&opt, Index);

    // Paint the standard row chrome (selection / hover background) but suppress the
    // default text + icon — we paint them ourselves below.
    opt.text.clear();
    opt.icon = QIcon();
    opt.features &= ~QStyleOptionViewItem::HasDecoration;

    QStyle* style = opt.widget ? opt.widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &opt, Painter, opt.widget);

    Painter->save();
    Painter->setRenderHint(QPainter::Antialiasing, true);
    Painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

    const QRect rowRect = Option.rect;
    const ObjectState status = static_cast<ObjectState>(Index.data(ObjectStatusRole).toInt());
    const bool collected = (status != ObjectState::Hidden);

    // "Card" highlight for discovered objects: a slightly lighter rounded rect over
    // the row to make collected entries stand out from the uncollected ones. Forced
    // entries (manually marked by the user) get an amber tint so they read as
    // "user-flagged" against both OoT blue and MM violet themes.
    if (collected && !(Option.state & QStyle::State_Selected))
    {
        QRect cardRect = rowRect.adjusted(4, 3, -4, -3);
        QPainterPath cardPath;
        cardPath.addRoundedRect(cardRect, 6, 6);

        QColor cardBg = (status == ObjectState::Forced)
            ? QColor(255, 190, 90, 32)      // amber for user-forced entries
            : QColor(255, 255, 255, 18);    // neutral white tint for auto-collected
        Painter->fillPath(cardPath, cardBg);
    }

    // Resolve the icon: prefer the row's own DecorationRole, fall back to the parent
    // category icon so all leaves share the chest / grass / scrub artwork without
    // having to set it on every ObjectItemTree.
    QIcon icon = qvariant_cast<QIcon>(Index.data(Qt::DecorationRole));
    if (icon.isNull())
    {
        icon = qvariant_cast<QIcon>(Index.parent().data(Qt::DecorationRole));
    }

    QRect iconRect(rowRect.left() + kPadX, rowRect.top() + (rowRect.height() - kIconSize) / 2, kIconSize, kIconSize);

    // Tinted square behind the icon.
    QColor iconBg(255, 255, 255, 18);
    QPainterPath iconBgPath;
    iconBgPath.addRoundedRect(iconRect, 6, 6);
    Painter->fillPath(iconBgPath, iconBg);

    if (!icon.isNull())
    {
        QPixmap pix = icon.pixmap(QSize(kIconSize - 8, kIconSize - 8));
        QRect target = pix.rect();
        target.moveCenter(iconRect.center());
        Painter->drawPixmap(target, pix);
    }

    // Text area to the right of the icon.
    QRect textRect(iconRect.right() + kIconTextSpacing,
                   rowRect.top(),
                   rowRect.right() - (iconRect.right() + kIconTextSpacing) - kPadX,
                   rowRect.height());

    // Object name (top line).
    QString objectName = Index.data(Qt::DisplayRole).toString();

    QFont nameFont = Option.font;
    nameFont.setBold(true);
    nameFont.setPointSizeF(nameFont.pointSizeF() + 0.5);
    nameFont.setStrikeOut(collected);

    QFontMetrics nameMetrics(nameFont);

    // Item name (bottom line). Empty / unknown is shown as "???".
    // When the "Reveal Uncollected Items" option is off, uncollected objects also
    // display "???" — even if their item is known from the spoiler log.
    QString itemName = Index.data(ItemNameRole).toString();
    const bool revealUncollected = AppConfig::GetRevealUncollectedItems();
    if (!collected && !revealUncollected)
    {
        itemName = "???";
    }
    else if (itemName.isEmpty() || itemName.compare("Unknown", Qt::CaseInsensitive) == 0)
    {
        itemName = "???";
    }

    QFont itemFont = Option.font;
    itemFont.setBold(false);
    itemFont.setStrikeOut(collected);

    QFontMetrics itemMetrics(itemFont);

    const int totalTextHeight = nameMetrics.height() + kLineSpacing + itemMetrics.height();
    int yTop = textRect.top() + (textRect.height() - totalTextHeight) / 2;

    // Color the text based on selection / collection state.
    QColor nameColor;
    QColor itemColor;

    if (status == ObjectState::Forced)
    {   // User-forced entries: amber-tinted text echoing the card bg.
        nameColor = QColor(248, 200, 120);
        itemColor = QColor(208, 168, 110, 200);
    }
    else if (collected)
    {   // Auto-collected: dimmer, neutral.
        nameColor = QColor(204, 218, 240, 150);
        itemColor = QColor(141, 162, 192, 140);
    }
    else
    {   // Not yet discovered.
        nameColor = QColor(221, 238, 255);
        itemColor = QColor(122, 154, 191);
    }

    if (Option.state & QStyle::State_Selected)
    {
        nameColor = (status == ObjectState::Forced) ? QColor(255, 215, 140) : QColor(255, 255, 255);
        itemColor = collected ? QColor(220, 220, 235, 220) : QColor(190, 200, 220);
    }

    Painter->setFont(nameFont);
    Painter->setPen(nameColor);
    Painter->drawText(QRect(textRect.left(), yTop, textRect.width(), nameMetrics.height()),
                      Qt::AlignLeft | Qt::AlignVCenter,
                      nameMetrics.elidedText(objectName, Qt::ElideRight, textRect.width()));

    Painter->setFont(itemFont);
    Painter->setPen(itemColor);
    Painter->drawText(QRect(textRect.left(), yTop + nameMetrics.height() + kLineSpacing, textRect.width(), itemMetrics.height()),
                      Qt::AlignLeft | Qt::AlignVCenter,
                      itemMetrics.elidedText(itemName, Qt::ElideRight, textRect.width()));

    Painter->restore();
}

#pragma endregion
