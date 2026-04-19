#include "UI/EntranceRenderer.h"
#include "UI/ObjectRenderer.h"
#include "UI/EntranceTable.h"

#include <QBrush>
#include <QGraphicsSceneHoverEvent>
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


void EntrancePixmapItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    if (this->ItemOwner != nullptr)
    {
        this->ItemOwner->SetHighlighted(true);
    }
    QGraphicsPixmapItem::hoverEnterEvent(event);
}


void EntrancePixmapItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    if (this->ItemOwner != nullptr)
    {
        this->ItemOwner->SetHighlighted(false);
    }
    QGraphicsPixmapItem::hoverLeaveEvent(event);
}

#pragma endregion // EntrancePixmapItem


#pragma region // EntranceLabelItem

EntranceLabelItem::EntranceLabelItem(const QString& Text, EntranceRenderer* PaOwner, EntranceLinkItemTree* PaItem)
    : QGraphicsSimpleTextItem(Text)
{
    this->Owner = PaOwner;
    this->ItemOwner = PaItem;
    this->setAcceptHoverEvents(true);
    this->setFlag(QGraphicsItem::ItemIsSelectable, true);
}


void EntranceLabelItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (this->ItemOwner != nullptr)
    {
        this->ItemOwner->SetCalledFromGraph(true);
        this->ItemOwner->PerformAction();
        this->ItemOwner->SetCalledFromGraph(false);
    }
    QGraphicsSimpleTextItem::mousePressEvent(event);
}


void EntranceLabelItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    if (this->ItemOwner != nullptr)
    {
        this->ItemOwner->SetHighlighted(true);
    }
    QGraphicsSimpleTextItem::hoverEnterEvent(event);
}


void EntranceLabelItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    if (this->ItemOwner != nullptr)
    {
        this->ItemOwner->SetHighlighted(false);
    }
    QGraphicsSimpleTextItem::hoverLeaveEvent(event);
}

#pragma endregion // EntranceLabelItem


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
    qreal rotDeg = this->IsInLink ? meta->ArrowRot : meta->ArrowRot + 180.0;

    EntranceRenderer::PlaceLabelAroundArrow(this->TextItem, pos[0], pos[1], rotDeg, pos[2]);
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
        // Undiscovered link: navigation has no destination, so fall back to selecting the matching
        // tree leaf instead of doing nothing. The user gets visible feedback that the click was
        // registered and learns which tree row the on-map marker corresponds to.
        if (this->IsTargetKnown())
        {
            this->NavigateToTarget();
        }
        else
        {
            this->SelectInTree();
        }
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


bool EntranceLinkItemTree::IsTargetKnown() const
{
    if (this->EntranceItem == nullptr || this->EntranceItem->RendererOwner == nullptr)
    {
        return false;
    }
    SceneEntranceMetaInf* sceneInf = GetSceneEntranceMetaInf(this->EntranceItem->RendererOwner->GetGameID(), this->EntranceItem->SceneID);
    if (sceneInf == nullptr)
    {
        return false;
    }
    auto it = sceneInf->EntranceIDs.find(this->EntranceItem->EntranceID);
    if (it == sceneInf->EntranceIDs.end())
    {
        return false;
    }

    const EntranceLink& link = it->second;
    uint32_t targetEntranceID = this->IsInLink ? link.InLink : link.OutLink;
    uint8_t targetGame = this->IsInLink ? link.InLinkGame : link.OutLinkGame;
    return targetEntranceID != UINT32_MAX && targetGame != NO_GAME;
}


void EntranceLinkItemTree::SelectInTree()
{
    QTreeWidget* tree = this->treeWidget();
    if (tree == nullptr)
    {
        return;
    }

    // Walk the parent chain to expand every collapsed ancestor: scrollToItem only scrolls items
    // that are currently visible in the tree, so a collapsed parent would silently no-op.
    QTreeWidgetItem* parent = this->parent();
    while (parent != nullptr)
    {
        parent->setExpanded(true);
        parent = parent->parent();
    }

    tree->setCurrentItem(this);
    tree->scrollToItem(this);
}


void EntranceLinkItemTree::SetHighlighted(bool Highlighted)
{
    // Highlight the pair visually: the arrow scales up and rises above other arrows; the label
    // switches to yellow and also rises above everything so nearby labels never cover it. Both
    // items pivot on their own origin so no extra re-centering is needed when scaling.
    constexpr qreal kHighlightScale = 1.3;
    constexpr qreal kHighlightZ = 10.0;

    if (this->GraphItem != nullptr)
    {
        this->GraphItem->setScale(Highlighted ? kHighlightScale : 1.0);
        this->GraphItem->setZValue(Highlighted ? kHighlightZ : 0.0);
    }

    if (this->TextItem != nullptr)
    {
        this->TextItem->setBrush(QBrush(Highlighted ? QColor(255, 230, 0) : QColor(255, 255, 255)));
        this->TextItem->setZValue(Highlighted ? kHighlightZ : 0.0);
    }
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
    if (this->RendererOwner == nullptr)
    {
        return;
    }
    const EntranceMetaInfo* meta = this->GetMetaInfo();
    if (meta == nullptr)
    {
        return;
    }

    // Pick the position that visually represents the entrance pair as a whole: Normal entrances
    // and OneWayIn entrances get their In side; OneWayOut entrances only have an Out side. This
    // mirrors which leaf children were spawned in the constructor.
    const int* pos = nullptr;
    if (meta->Type == EntranceType::Normal || meta->Type == EntranceType::One_Way_In)
    {
        pos = meta->InPosition;
    }
    else if (meta->Type == EntranceType::One_Way_Out)
    {
        pos = meta->OutPosition;
    }

    this->RendererOwner->CenterAndZoomViewOn(pos);
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


void EntranceRenderer::CenterAndZoomViewOn(const int* Position)
{
    if (Position == nullptr || this->View == nullptr)
    {
        return;
    }

    // Reset before scaling so successive calls do not stack zoom factors. The chosen factor (3x)
    // is large enough to make the entrance pair clearly visible even on dense scenes, while still
    // showing enough surrounding map for the user to keep their bearings.
    constexpr qreal kFocusZoom = 3.0;
    this->View->resetTransform();
    this->View->scale(kFocusZoom, kFocusZoom);
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
        EntranceLabelItem* label = new EntranceLabelItem(name, this, Link);
        label->setBrush(QBrush(QColor(255, 255, 255)));
        label->setFont(QFont("Arial", 12, QFont::Weight::Bold));

        // Stroke the glyph outlines with black so the white fill stays readable on any map
        // background. RoundJoin + RoundCap avoid spiky artefacts where the stroke would otherwise
        // poke out of thin letter corners.
        QPen outline(QColor(0, 0, 0));
        outline.setWidth(0.1);
        //outline.setJoinStyle(Qt::RoundJoin);
        //outline.setCapStyle(Qt::RoundCap);
        label->setPen(outline);

        // In and Out rotations already differ by 180 deg, so anchoring at the arrow tip naturally
        // puts a Normal entrance's two labels on opposite sides of the pair. The Z component of
        // the position encodes an override TextPlacement value (Default / Up / Down / Left / Right
        // / NoText) so each entrance can force its label to a specific side when the tip-based
        // default collides with map geometry.
        PlaceLabelAroundArrow(label, pos[0], pos[1], rotDeg, pos[2]);

        this->Scene->addItem(label);
        this->OverlayItems.push_back(label);
        Link->TextItem = label;
    }
}


void EntranceRenderer::PlaceLabelAroundArrow(QGraphicsSimpleTextItem* Label, qreal CenterX, qreal CenterY, qreal RotDeg, int Placement)
{
    if (Label == nullptr)
    {
        return;
    }

    // NoText hides the label entirely. We keep the QGraphicsSimpleTextItem alive so the tree /
    // graph stay in sync and so later refreshes can toggle visibility back on if the Placement
    // value ever changes.
    if (Placement == TextPlacement::NoText)
    {
        Label->setVisible(false);
        return;
    }
    Label->setVisible(true);

    // Offset distance: In / Out arrow icons are 30x21 px centered on their origin, so the worst
    // case rotated half-extent sits near 18 px. The margin keeps the text visually clear of the
    // arrow without drifting too far away.
    constexpr qreal kOffset = 15.0;

    QRectF rect = Label->boundingRect();
    qreal w = rect.width();
    qreal h = rect.height();

    // Forced cardinal placements ignore the arrow rotation and pin the label to one side of the
    // arrow center. This is the escape hatch for entrances where the default tip-aligned placement
    // collides with nearby pixmaps or map geometry.
    if (Placement == TextPlacement::Up)
    {
        Label->setPos(CenterX - w / 2.0, CenterY - kOffset - h);
        return;
    }
    if (Placement == TextPlacement::Down)
    {
        Label->setPos(CenterX - w / 2.0, CenterY + kOffset);
        return;
    }
    if (Placement == TextPlacement::Left)
    {
        Label->setPos(CenterX - kOffset - w, CenterY - h / 2.0);
        return;
    }
    if (Placement == TextPlacement::Right)
    {
        Label->setPos(CenterX + kOffset, CenterY - h / 2.0);
        return;
    }

    // Default path: anchor the label at the arrow tip. The In / Out arrow pixmaps are 30x21 px and
    // point to the right by default at rotation 0 — so the tip unit vector is (cos, sin) and NOT
    // (sin, -cos) as it would be for an up-pointing arrow. For Normal entrances the In and Out
    // rotations already differ by 180 deg, so the two labels end up on opposite sides of the pair
    // without any extra sibling-aware logic.
    qreal rad = qDegreesToRadians(RotDeg);
    qreal dirX = std::cos(rad);
    qreal dirY = std::sin(rad);

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
