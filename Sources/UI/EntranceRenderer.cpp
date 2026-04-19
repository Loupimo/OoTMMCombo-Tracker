#include "UI/EntranceRenderer.h"
#include "UI/ObjectRenderer.h"
#include "UI/EntranceTable.h"

#include <QBrush>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSimpleTextItem>
#include <QPainter>
#include <QPen>
#include <QPointer>
#include <QPolygonF>
#include <QTimer>
#include <QtMath>

#include <cmath>


#pragma region // EntrancePixmapItem

EntrancePixmapItem::EntrancePixmapItem(const QPixmap& Pixmap, EntranceRenderer* PaOwner, EntranceLinkItemTree* PaItem)
    : QGraphicsPixmapItem(Pixmap)
{
    this->Owner = PaOwner;
    this->ItemOwner = PaItem;
    this->setAcceptHoverEvents(true);
    this->setFlag(QGraphicsItem::ItemIsSelectable, true);

    // Shift the pixmap so its visual center coincides with the item's local (0, 0). Combined with
    // the default transform origin, setPos(x, y) then places the arrow's center at (x, y) and
    // setRotation() pivots around that same center without any extra bookkeeping at each placement.
    this->setOffset(-Pixmap.width() / 2.0, -Pixmap.height() / 2.0);
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
    this->setIcon(0, this->IsInLink ? ObjectIcons::GetIconsRef()->EntranceIcons[1] : ObjectIcons::GetIconsRef()->EntranceIcons[2]);

    // Propagate the name change to the on-map label. Without this the overlay stays stuck on the
    // previous text (usually "?") until the next scene re-render, even though the tree and the
    // table both show the newly discovered name.
    this->UpdateOverlayLabel();
}


void EntranceLinkItemTree::UpdateOverlayLabel()
{
    if (this->TextItem == nullptr || this->EntranceItem == nullptr)
    {
        return;
    }

    GlobalEntranceRow* row = this->EntranceItem->GetRow();
    const EntranceMetaInfo* meta = this->EntranceItem->GetMetaInfo();
    if (row == nullptr || meta == nullptr)
    {
        return;
    }

    // Re-apply the text first: the width drives the anchoring in PlaceLabelAroundArrow so the
    // reposition must happen against the updated bounding rect (e.g. "?" -> "Bolero of Fire").
    QString name = this->IsInLink ? row->InLinkName : row->OutLinkName;
    this->TextItem->setText(name);

    const int* pos = this->IsInLink ? meta->InPosition : meta->OutPosition;
    const int* otherPos = (meta->Type == EntranceType::Normal)
        ? (this->IsInLink ? meta->OutPosition : meta->InPosition)
        : nullptr;
    qreal rotDeg = this->IsInLink ? meta->ArrowRot : meta->ArrowRot + 180.0;

    EntranceRenderer::PlaceLabelAroundArrow(this->TextItem, pos[0], pos[1], rotDeg, otherPos);
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

    // Two entry points share this single action: a tree click (CalledFromGraph == false) asks to
    // re-center the map on the link's position, while a map arrow click (CalledFromGraph == true)
    // asks to jump to the destination scene. The flag is the only way to distinguish them because
    // both paths go through the same QTreeWidgetItem handler.
    if (this->IsCalledFromGraph())
    {
        this->NavigateToTarget();
    }
    else
    {
        this->EntranceItem->RendererOwner->CenterViewOn(this->GetPosition());
    }
}


int EntranceLinkItemTree::GetTotalObjectAvailable()
{
    return 1;
}


void EntranceLinkItemTree::NavigateToTarget()
{
    if (this->EntranceItem == nullptr || this->EntranceItem->RendererOwner == nullptr)
    {
        return;
    }
    EntranceRenderer* renderer = this->EntranceItem->RendererOwner;
    EntranceGameTabView* gameView = renderer->Owner;
    if (gameView == nullptr || gameView->Owner == nullptr)
    {
        return;
    }

    // Resolve the link from the live scene meta info (and not from the cached GlobalEntranceRow)
    // because the meta info is the single source of truth written by EntranceHelper: it always
    // reflects the current save state, even before the model has been notified of an update.
    SceneEntranceMetaInf* sceneInf = GetSceneEntranceMetaInf(renderer->GetGameID(), this->EntranceItem->SceneID);
    if (sceneInf == nullptr)
    {
        return;
    }
    auto it = sceneInf->EntranceIDs.find(this->EntranceItem->EntranceID);
    if (it == sceneInf->EntranceIDs.end())
    {
        return;
    }

    const EntranceLink& link = it->second;
    uint32_t targetEntranceID = this->IsInLink ? link.InLink : link.OutLink;
    uint8_t targetGame = this->IsInLink ? link.InLinkGame : link.OutLinkGame;

    // Undiscovered side: the user clicked an arrow for a traversal that has not happened yet.
    // GetEntranceMetaInf would dereference an out-of-range ID so we must bail out before calling it.
    if (targetEntranceID == UINT32_MAX || targetGame == NO_GAME)
    {
        return;
    }

    const EntranceMetaInfo* target = EntranceHelper::GetEntranceMetaInf(targetGame, targetEntranceID);
    if (target == nullptr)
    {
        return;
    }

    // Defer the navigation to the next event-loop tick: FocusSceneInGame will rebuild the entrance
    // tree and the overlay, which would otherwise delete the very EntrancePixmapItem whose click
    // handler is still running on the call stack (use-after-free). Capturing by value isolates the
    // lambda from the soon-to-be-destroyed tree item, and QPointer guards against the tab widget
    // itself being torn down before the tick fires (e.g. user closing the application).
    QPointer<EntranceTab> tab = gameView->Owner;
    uint32_t destSceneID = target->ToSceneID;
    uint8_t destGame = targetGame;
    QTimer::singleShot(0, [tab, destGame, destSceneID]()
    {
        if (!tab.isNull())
        {
            tab->FocusSceneInGame(destGame, destSceneID);
        }
    });
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

    // Overlay items are owned by the renderer: the scene could auto-delete them on removeItem but
    // going through an explicit removeItem + delete keeps the lifetime obvious and guarantees the
    // pointer still cached on EntranceLinkItemTree::GraphItem becomes stale at a predictable point.
    if (this->Scene != nullptr)
    {
        for (QGraphicsItem* item : this->OverlayItems)
        {
            if (item != nullptr)
            {
                this->Scene->removeItem(item);
                delete item;
            }
        }
    }
    this->OverlayItems.clear();
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


void EntranceRenderer::RenderSceneOverlay(SceneEntranceMetaInf* Scene)
{
    if (Scene == nullptr || this->Scene == nullptr)
    {
        return;
    }

    for (auto& [entranceID, link] : Scene->EntranceIDs)
    {
        const EntranceMetaInfo* meta = EntranceHelper::GetEntranceMetaInf(this->GetGameID(), entranceID);
        if (meta == nullptr || meta->Type == EntranceType::None)
        {
            continue;
        }

        // The tree items must already exist (PopulateEntranceList runs before this) so the overlay
        // markers can piggyback on their click handler and display state without duplicating any
        // lookup logic. A missing tree item means the entrance was filtered out upstream (e.g. the
        // side is None) and we legitimately skip it.
        EntranceItemTree* tree = this->FindEntrance(Scene->SceneID, entranceID);
        if (tree == nullptr)
        {
            continue;
        }

        if (tree->InItem != nullptr)
        {
            this->AddLinkMarker(tree->InItem, true, meta);
        }
        if (tree->OutItem != nullptr)
        {
            this->AddLinkMarker(tree->OutItem, false, meta);
        }
    }
}


void EntranceRenderer::AddLinkMarker(EntranceLinkItemTree* Link, bool IsIn, const EntranceMetaInfo* Meta)
{
    if (Link == nullptr || Meta == nullptr || this->Scene == nullptr)
    {
        return;
    }

    const int* pos = IsIn ? Meta->InPosition : Meta->OutPosition;
    qreal rotDeg = IsIn ? Meta->ArrowRot : Meta->ArrowRot + 180.0;

    // Arrow marker: the shared placeholder pixmap is already centered on its own origin so setPos
    // pins the arrow's center to the entrance coordinates and setRotation pivots in place. The Out
    // side is rotated by an extra 180 degrees so a Normal entrance's In / Out pair always point in
    // visually opposite directions.
    EntrancePixmapItem* arrow = new EntrancePixmapItem(IsIn ? ObjectIcons::GetIconsRef()->PixmapEntranceIcons[1] : ObjectIcons::GetIconsRef()->PixmapEntranceIcons[2], this, Link);
    arrow->setPos(pos[0], pos[1]);
    arrow->setRotation(rotDeg);
    this->Scene->addItem(arrow);
    this->OverlayItems.push_back(arrow);
    Link->GraphItem = arrow;

    // Name label: uses the same string displayed in the entrance tree so the two views stay in
    // sync automatically on every model update. Caching the pointer on Link lets RefreshText
    // update the live text/position without waiting for the next scene re-render.
    GlobalEntranceRow* row = Link->EntranceItem != nullptr ? Link->EntranceItem->GetRow() : nullptr;
    if (row != nullptr)
    {
        QString name = IsIn ? row->InLinkName : row->OutLinkName;
        QGraphicsSimpleTextItem* label = new QGraphicsSimpleTextItem(name);
        label->setBrush(QBrush(QColor(255, 255, 255)));

        // For Normal entrances, hand the sibling position to the placer so the two labels are
        // pushed outward from each other. One-Way sides fall back to arrow-opposite placement.
        const int* otherPos = (Meta->Type == EntranceType::Normal)
            ? (IsIn ? Meta->OutPosition : Meta->InPosition)
            : nullptr;
        PlaceLabelAroundArrow(label, pos[0], pos[1], rotDeg, otherPos);

        this->Scene->addItem(label);
        this->OverlayItems.push_back(label);
        Link->TextItem = label;
    }
}


void EntranceRenderer::PlaceLabelAroundArrow(QGraphicsSimpleTextItem* Label, qreal CenterX, qreal CenterY, qreal RotDeg, const int* OtherPos)
{
    if (Label == nullptr)
    {
        return;
    }

    // Preferred placement direction: pointing away from the sibling side. This keeps both labels
    // of a Normal entrance on opposite sides of the In / Out pair no matter the arrow rotation,
    // which is the common case that produced overlapping labels before.
    qreal dirX = 0.0;
    qreal dirY = 0.0;
    bool useArrowFallback = true;
    if (OtherPos != nullptr)
    {
        qreal dx = CenterX - static_cast<qreal>(OtherPos[0]);
        qreal dy = CenterY - static_cast<qreal>(OtherPos[1]);
        qreal len = std::hypot(dx, dy);
        if (len > 4.0)
        {
            dirX = dx / len;
            dirY = dy / len;
            useArrowFallback = false;
        }
    }
    if (useArrowFallback)
    {
        // In / Out rotations already differ by 180 deg so using the arrow-opposite direction here
        // still yields two opposite labels when the sibling position is missing or coincident.
        qreal rad = qDegreesToRadians(RotDeg);
        dirX = -std::sin(rad);
        dirY = std::cos(rad);
    }

    // Offset distance: In / Out arrow icons are 30x21 px centered on their origin, so the worst
    // case rotated half-extent sits near 18 px. The margin keeps the text visually clear of the
    // arrow without drifting too far away.
    constexpr qreal kOffset = 20.0;

    QRectF rect = Label->boundingRect();
    qreal w = rect.width();
    qreal h = rect.height();

    qreal x = CenterX + dirX * kOffset;
    qreal y = CenterY + dirY * kOffset;

    // Anchor the edge of the label closest to the arrow against the offset point, so text always
    // extends outward. Mostly-axial directions pin one edge and center the other axis, which is
    // what looks clean for the four cardinal orientations.
    constexpr qreal kThreshold = 0.3;
    if (dirX > kThreshold)
    {
        // Label sits to the right: its left edge is the anchor, no extra shift needed.
    }
    else if (dirX < -kThreshold)
    {
        x -= w;
    }
    else
    {
        x -= w / 2.0;
    }

    if (dirY > kThreshold)
    {
        // Label sits below: its top edge is the anchor.
    }
    else if (dirY < -kThreshold)
    {
        y -= h;
    }
    else
    {
        y -= h / 2.0;
    }

    Label->setPos(x, y);
}


const QPixmap& EntranceRenderer::PlaceholderArrowPixmap()
{
    // Built once per process and shared across every marker. Keeping it process-wide static avoids
    // re-rasterizing the same triangle for every arrow and guarantees all markers share the exact
    // same visual footprint (important for consistent rotation pivots).
    static QPixmap cached;
    if (cached.isNull())
    {
        const int size = 16;
        cached = QPixmap(size, size);
        cached.fill(Qt::transparent);

        QPainter painter(&cached);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(QPen(QColor(0, 0, 0), 1));
        painter.setBrush(QBrush(QColor(220, 60, 60)));

        // Triangle pointing "up" at 0 degrees. Qt's setRotation is clockwise-positive, so ArrowRot
        // can be applied directly once the real resource replaces this placeholder.
        QPolygonF tri;
        tri << QPointF(size / 2.0, 0.0)
            << QPointF(static_cast<qreal>(size), static_cast<qreal>(size))
            << QPointF(0.0, static_cast<qreal>(size));
        painter.drawPolygon(tri);
    }
    return cached;
}

#pragma endregion // EntranceRenderer
