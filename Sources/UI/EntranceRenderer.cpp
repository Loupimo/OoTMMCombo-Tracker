#include "UI/EntranceRenderer.h"
#include "UI/EntranceTable.h"

#include <QGraphicsSceneMouseEvent>


#pragma region // EntrancePixmapItem

EntrancePixmapItem::EntrancePixmapItem(const QPixmap& Pixmap, EntranceRenderer* PaOwner, EntranceLinkItemTree* PaItem)
    : QGraphicsPixmapItem(Pixmap)
{
    this->Owner = PaOwner;
    this->ItemOwner = PaItem;
    this->setAcceptHoverEvents(true);
    this->setFlag(QGraphicsItem::ItemIsSelectable, true);
}


void EntrancePixmapItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (this->ItemOwner != nullptr)
    {
        this->ItemOwner->SetCalledFromGraph(true);
        this->ItemOwner->PerformAction();
        this->ItemOwner->SetCalledFromGraph(false);
    }
    QGraphicsPixmapItem::mousePressEvent(event);
}

#pragma endregion // EntrancePixmapItem


#pragma region // EntranceLinkItemTree

EntranceLinkItemTree::EntranceLinkItemTree(EntranceItemTree* PaEntrance, bool InIsInLink, QTreeWidgetItem* Parent) : CommonBaseItemTree(Parent)
{
    this->EntranceItem = PaEntrance;
    this->IsInLink = InIsInLink;
    this->RefreshText();
}


void EntranceLinkItemTree::RefreshText()
{
    GlobalEntranceRow* row = this->EntranceItem->GetRow();
    if (row == nullptr)
    {
        return;
    }

    QString prefix = this->IsInLink ? QStringLiteral("In: ") : QStringLiteral("Out: ");
    QString name = this->IsInLink ? row->InLinkName : row->OutLinkName;
    this->setText(0, prefix + name);
}


const int* EntranceLinkItemTree::GetPosition() const
{
    const EntranceMetaInfo* meta = this->EntranceItem->GetMetaInfo();
    if (meta == nullptr)
    {
        return nullptr;
    }
    return this->IsInLink ? meta->InPosition : meta->OutPosition;
}


void EntranceLinkItemTree::PerformAction()
{
    if (this->EntranceItem == nullptr || this->EntranceItem->RendererOwner == nullptr)
    {
        return;
    }
    this->EntranceItem->RendererOwner->CenterViewOn(this->GetPosition());
}


int EntranceLinkItemTree::GetTotalObjectAvailable()
{
    return 1;
}

#pragma endregion // EntranceLinkItemTree


#pragma region // EntranceItemTree

EntranceItemTree::EntranceItemTree(GlobalEntranceRow* Row, EntranceRenderer* PaOwner, QTreeWidgetItem* Parent) : CommonBaseItemTree(Parent)
{
    this->RendererOwner = PaOwner;
    this->SceneID = Row->SceneID;
    this->EntranceID = Row->EntranceID;
    this->setText(0, Row->EntranceName);

    const EntranceMetaInfo* entrance = this->GetMetaInfo();
    if (entrance != nullptr)
    {
        switch (entrance->Type)
        {
            case EntranceType::Normal:
            {
                this->InItem = new EntranceLinkItemTree(this, true, this);
                this->OutItem = new EntranceLinkItemTree(this, false, this);
                break;
            }
            case EntranceType::One_Way_In:
            {
                this->InItem = new EntranceLinkItemTree(this, true, this);
                break;
            }
            case EntranceType::One_Way_Out:
            {
                this->OutItem = new EntranceLinkItemTree(this, false, this);
                break;
            }
            default:
            {
                break;
            }
        }
    }

    if (this->RendererOwner != nullptr)
    {
        this->RendererOwner->RegisterEntrance(this);
    }
}


GlobalEntranceRow* EntranceItemTree::GetRow() const
{
    if (this->RendererOwner == nullptr)
    {
        return nullptr;
    }
    GlobalEntranceTableModel* model = this->RendererOwner->GetModel();
    if (model == nullptr)
    {
        return nullptr;
    }

    for (auto& row : model->m_rows)
    {
        if (row.SceneID == this->SceneID && row.EntranceID == this->EntranceID)
        {
            return &row;
        }
    }
    return nullptr;
}


const EntranceMetaInfo* EntranceItemTree::GetMetaInfo() const
{
    if (this->RendererOwner == nullptr)
    {
        return nullptr;
    }
    return EntranceHelper::GetEntranceMetaInf(this->RendererOwner->GetGameID(), this->EntranceID);
}


void EntranceItemTree::RefreshText()
{
    GlobalEntranceRow* row = this->GetRow();
    if (row != nullptr)
    {
        this->setText(0, row->EntranceName);
    }
    if (this->InItem != nullptr)
    {
        this->InItem->RefreshText();
    }
    if (this->OutItem != nullptr)
    {
        this->OutItem->RefreshText();
    }
}


void EntranceItemTree::PerformAction()
{
    if (this->InItem != nullptr)
    {
        this->InItem->PerformAction();
    }
    else if (this->OutItem != nullptr)
    {
        this->OutItem->PerformAction();
    }
}


int EntranceItemTree::GetTotalObjectAvailable()
{
    return this->childCount();
}

#pragma endregion // EntranceItemTree


#pragma region // EntranceRenderer

EntranceRenderer::EntranceRenderer(EntranceGameTabView* PaOwner)
{
    this->Owner = PaOwner;
}


void EntranceRenderer::SetTarget(QGraphicsScene* PaScene, QGraphicsView* PaView)
{
    this->Scene = PaScene;
    this->View = PaView;
}


void EntranceRenderer::Clear()
{
    this->Entrances.clear();
}


void EntranceRenderer::RegisterEntrance(EntranceItemTree* Item)
{
    this->Entrances.push_back(Item);
}


EntranceItemTree* EntranceRenderer::FindEntrance(uint32_t SceneID, uint32_t EntranceID)
{
    for (EntranceItemTree* item : this->Entrances)
    {
        if (item != nullptr && item->SceneID == SceneID && item->EntranceID == EntranceID)
        {
            return item;
        }
    }
    return nullptr;
}


void EntranceRenderer::CenterViewOn(const int* Position)
{
    if (Position == nullptr || this->View == nullptr)
    {
        return;
    }
    this->View->centerOn(Position[0], Position[1]);
}


int EntranceRenderer::GetGameID() const
{
    return this->Owner != nullptr ? this->Owner->GameID : -1;
}


GlobalEntranceTableModel* EntranceRenderer::GetModel() const
{
    if (this->Owner == nullptr || this->Owner->AllView == nullptr)
    {
        return nullptr;
    }
    return this->Owner->AllView->Model;
}

#pragma endregion // EntranceRenderer
