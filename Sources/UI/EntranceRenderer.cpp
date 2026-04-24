#include "UI/EntranceRenderer.h"
#include "UI/ObjectRenderer.h"
#include "UI/EntranceTable.h"

#include <QBrush>
#include <QtMinMax>
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

    //QString prefix = this->IsInLink ? QStringLiteral("In: ") : QStringLiteral("Out: ");
    QString name = this->IsInLink ? row->InLinkName : row->OutLinkName;
    this->setText(0, /*prefix + */name);
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

    const int* pos = meta->TextPos;
    /*const int* pos = this->IsInLink ? meta->InPosition : meta->OutPosition;
    qreal rotDeg = this->IsInLink ? meta->ArrowRot : meta->ArrowRot + 180.0;*/

    //EntranceRenderer::PlaceLabelAroundArrow(this->TextItem, pos[0], pos[1], rotDeg, pos[2]);
    EntranceRenderer::PlaceLabelAroundArrow(this->TextItem, pos[0], pos[1], 0, pos[2]);
}


const int* EntranceLinkItemTree::GetPosition() const
{
    const EntranceMetaInfo* meta = this->EntranceItem->GetMetaInfo();
    if (meta == nullptr)
    {
        return nullptr;
    }
    return meta->AnchorPos;//this->IsInLink ? meta->InPosition : meta->OutPosition;
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

    // Defer the navigation to the next event-loop tick: FocusEntranceInGame will rebuild the
    // entrance tree and the overlay, which would otherwise delete the very graphics item whose
    // click handler is still running on the call stack (use-after-free). Capturing by value
    // isolates the lambda from the soon-to-be-destroyed tree item, and QPointer guards against
    // the tab widget itself being torn down before the tick fires (e.g. user closing the app).
    // We jump to the *target* entrance (not just the scene) so the view also zooms in on the
    // corresponding marker instead of dropping the user at whatever the scene's default framing is.
    QPointer<EntranceTab> tab = gameView->Owner;
    uint32_t destSceneID = target->ToSceneID;
    uint32_t destEntranceID = targetEntranceID;
    uint8_t destGame = targetGame;
    QTimer::singleShot(0, [tab, destGame, destSceneID, destEntranceID]()
    {
        if (!tab.isNull())
        {
            tab->FocusEntranceInGame(destGame, destSceneID, destEntranceID);
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

    // Propagate to the grouped overlay box when the current scene is rendered, so a newly
    // discovered entrance name immediately resizes the box and the connecting curve instead of
    // waiting for the next scene re-render.
    if (this->GroupBox != nullptr)
    {
        this->GroupBox->RefreshText();
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

    const int* pos = meta->AnchorPos;

    // Pick the position that visually represents the entrance pair as a whole: Normal entrances
    // and OneWayIn entrances get their In side; OneWayOut entrances only have an Out side. This
    // mirrors which leaf children were spawned in the constructor.
    /*const int* pos = nullptr;
    if (meta->Type == EntranceType::Normal || meta->Type == EntranceType::One_Way_In)
    {
        pos = meta->InPosition;
    }
    else if (meta->Type == EntranceType::One_Way_Out)
    {
        pos = meta->OutPosition;
    }*/

    this->RendererOwner->CenterAndZoomViewOn(pos);
}


int EntranceItemTree::GetTotalObjectAvailable()
{
    return this->childCount();
}

#pragma endregion // EntranceItemTree


#pragma region // EntranceGroupBoxItem

EntranceGroupBoxItem::EntranceGroupBoxItem(EntranceItemTree* PaTree, QGraphicsItem* Parent)
    : QGraphicsItem(Parent), Tree(PaTree)
{
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setCursor(Qt::PointingHandCursor);
    setZValue(10);
    RefreshText();
}


void EntranceGroupBoxItem::RefreshText()
{
    if (!Tree) return;
    const EntranceMetaInfo* meta = Tree->GetMetaInfo();
    GlobalEntranceRow* row = Tree->GetRow();

    // The box width depends on the text, so we must notify the scene graph before the bounding
    // rect changes — otherwise Qt's scene index caches a stale rect and hover/click regions drift.
    prepareGeometryChange();

    HasIn = meta && meta->Type != EntranceType::One_Way_Out;
    HasOut = meta && meta->Type != EntranceType::One_Way_In;
    Title = row ? row->EntranceName : "";
    InText = row ? row->InLinkName : "?";
    OutText = row ? row->OutLinkName : "?";

    // A freshly discovered name may grow the longest row, which changes the box perimeter; the
    // curve endpoint (closest point on the perimeter) must be recomputed to follow the new edge.
    RebuildCurve();
    update();
}


qreal EntranceGroupBoxItem::BoxWidth() const
{
    // Measure the real on-screen width of each line with the same fonts used in paint(). The row
    // prefix glyphs (triangles) are included because they participate in the rendered width. Use
    // the same \u escapes as paint() so the measurement is independent of the source encoding.
    static const QFont titleFont("Consolas", 8, QFont::Bold);
    static const QFont rowFont("Consolas", 7);
    const QFontMetricsF titleFM(titleFont);
    const QFontMetricsF rowFM(rowFont);

    // The title row starts with a RenderIcon pulled from EntranceMetaInfo, so the title line's
    // effective width is icon + gap + text advance. Row lines have no icon, just the prefix glyph.
    qreal maxW = kTitleIcon + kTitleIconGap + titleFM.horizontalAdvance(Title);
    if (HasIn)  maxW = qMax(maxW, rowFM.horizontalAdvance(QString::fromUtf8("\xE2\x96\xB2 ") + InText));
    if (HasOut) maxW = qMax(maxW, rowFM.horizontalAdvance(QString::fromUtf8("\xE2\x96\xBC ") + OutText));

    return qBound(kMinBoxW, maxW + kPadX * 2.0, kMaxBoxW);
}


qreal EntranceGroupBoxItem::BoxHeight() const
{
    return kTitleH
        + (HasIn ? kRowH : 0)
        + (HasOut ? kRowH : 0)
        + 4;
}


QRectF EntranceGroupBoxItem::boundingRect() const
{
    return QRectF(0, 0, BoxWidth(), BoxHeight());
}


void EntranceGroupBoxItem::paint(
    QPainter* p, const QStyleOptionGraphicsItem*, QWidget*)
{
    p->setRenderHint(QPainter::Antialiasing, true);

    const qreal w = BoxWidth(), h = BoxHeight();
    const QRectF r(0, 0, w, h);

    // ── Background ─────────────────────────────────────────────────────────
    p->fillRect(r, Highlighted ? QColor(14, 14, 30, 250) : QColor(6, 6, 18, 230));

    // ── Border ──────────────────────────────────────────────────────────────
    p->setPen(QPen(Highlighted
        ? QColor(255, 255, 255, 90)
        : QColor(255, 255, 255, 25), Highlighted ? 1.4 : 1.0));
    p->drawRect(r.adjusted(0.5, 0.5, -0.5, -0.5));

    // ── Title row ───────────────────────────────────────────────────────────
    // Pull the matching RenderIcon from the entrance meta info and paint it at the left of the
    // title row. Silently skipped if the pixmap is missing (null) so a stale index never crashes.
    const EntranceMetaInfo* titleMeta = Tree ? Tree->GetMetaInfo() : nullptr;
    qreal titleTextX = kPadX;
    qreal titleTextW = w - kPadX * 2;
    if (titleMeta != nullptr)
    {
        const QPixmap& iconPix = ObjectIcons::GetIconsRef()->PixmapEntranceIcons[titleMeta->RenderIcon];
        if (!iconPix.isNull())
        {
            const QRectF iconRect(kPadX, (kTitleH - kTitleIcon) / 2.0, kTitleIcon, kTitleIcon);
            p->drawPixmap(iconRect, iconPix, QRectF(iconPix.rect()));
            titleTextX += kTitleIcon + kTitleIconGap;
            titleTextW -= kTitleIcon + kTitleIconGap;
        }
    }
    p->setFont(QFont("Consolas", 8, QFont::Bold));
    p->setPen(Highlighted ? Qt::white : QColor(255, 255, 255, 190));
    p->drawText(
        QRectF(titleTextX, 0, titleTextW, kTitleH),
        Qt::AlignVCenter,
        p->fontMetrics().elidedText(Title, Qt::ElideRight, static_cast<int>(titleTextW)));

    // ── Title separator ─────────────────────────────────────────────────────
    p->setPen(QPen(QColor(255, 255, 255, 18), 1));
    p->drawLine(QLineF(0, kTitleH, w, kTitleH));

    p->setFont(QFont("Consolas", 7));
    int y = kTitleH;
    const qreal rowTextW = w - kPadX * 2;

    // ── In-link row (green) ─────────────────────────────────────────────────
    if (HasIn)
    {
        if (HoveredRow == 1)
        {
            p->fillRect(QRectF(kAccentW, y, w - kAccentW, kRowH), QColor(255, 255, 255, 22));
        }
        p->fillRect(QRectF(0, y, kAccentW, kRowH), QColor(61, 220, 132));
        p->setPen(QColor(101, 224, 154));
        // Elide the full prefixed line against the full text area: BoxWidth already measured the
        // prefix so the line fits when the box was not capped, and when it is capped the end of
        // InText gets truncated cleanly. Eliding InText alone with a hardcoded prefix reserve was
        // trimming one character off even when the box was wide enough.
        p->drawText(
            QRectF(kPadX, y, rowTextW, kRowH), Qt::AlignVCenter,
            p->fontMetrics().elidedText("\u25b2 " + InText, Qt::ElideRight, static_cast<int>(rowTextW)));
        y += kRowH;
    }

    // ── Out-link row (red) ──────────────────────────────────────────────────
    if (HasOut)
    {
        if (HoveredRow == 2)
        {
            p->fillRect(QRectF(kAccentW, y, w - kAccentW, kRowH), QColor(255, 255, 255, 22));
        }
        p->fillRect(QRectF(0, y, kAccentW, kRowH), QColor(255, 82, 82));
        p->setPen(QColor(255, 144, 144));
        p->drawText(
            QRectF(kPadX, y, rowTextW, kRowH), Qt::AlignVCenter,
            p->fontMetrics().elidedText("\u25bc " + OutText, Qt::ElideRight, static_cast<int>(rowTextW)));
    }
}


void EntranceGroupBoxItem::SetHighlighted(bool H)
{
    Highlighted = H;
    setZValue(H ? 20 : 10);
    update();
}


void EntranceGroupBoxItem::SetTreeHighlighted(bool H)
{
    if (!Tree) return;

    // Colours used when highlighted
    const QColor kBgH(30, 40, 70, 220);  // dark blue tint
    const QColor kFgH(255, 255, 255);         // white
    const QColor kInFgH(101, 224, 154);         // green
    const QColor kOutFgH(255, 144, 144);         // red

    // Restored colours (transparent bg = theme default)
    const QColor kBgNorm = Qt::transparent;
    const QColor kFgNorm = QColor(255, 255, 255, 180);

    // Parent row (entrance name)
    Tree->setBackground(0, H ? kBgH : kBgNorm);
    Tree->setForeground(0, H ? kFgH : kFgNorm);

    // In child row
    if (Tree->InItem)
    {
        Tree->InItem->setBackground(0, H ? kBgH : kBgNorm);
        Tree->InItem->setForeground(0, H ? kInFgH : kFgNorm);
    }

    // Out child row
    if (Tree->OutItem)
    {
        Tree->OutItem->setBackground(0, H ? kBgH : kBgNorm);
        Tree->OutItem->setForeground(0, H ? kOutFgH : kFgNorm);
    }

    // Ensure the highlighted row is visible in the tree widget
    if (H && Tree->treeWidget())
    {
        Tree->treeWidget()->scrollToItem(Tree, QAbstractItemView::EnsureVisible);
    }
}


void EntranceGroupBoxItem::hoverEnterEvent(QGraphicsSceneHoverEvent* e)
{
    SetHighlighted(true);
    if (Anchor)    Anchor->SetHighlighted(true);
    if (CurveLine) CurveLine->setZValue(11);
    if (Renderer)  Renderer->SetGroupFocus(this);
    HoveredRow = RowFromY(e->pos().y());
    SetTreeHighlighted(true);
    update();
    QGraphicsItem::hoverEnterEvent(e);
}


void EntranceGroupBoxItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* e)
{
    SetHighlighted(false);
    if (Anchor)    Anchor->SetHighlighted(false);
    if (CurveLine) CurveLine->setZValue(9);
    if (Renderer)  Renderer->SetGroupFocus(nullptr);
    HoveredRow = -1;
    SetTreeHighlighted(false);
    update();
    QGraphicsItem::hoverLeaveEvent(e);
}


void EntranceGroupBoxItem::hoverMoveEvent(QGraphicsSceneHoverEvent* e)
{
    const int newRow = RowFromY(e->pos().y());
    if (newRow != HoveredRow)
    {
        HoveredRow = newRow;
        update();
    }
    QGraphicsItem::hoverMoveEvent(e);
}


int EntranceGroupBoxItem::RowFromY(qreal Y) const
{
    if (Y < kTitleH)
    {
        return 0;
    }
    qreal rowTop = kTitleH;
    if (HasIn)
    {
        if (Y < rowTop + kRowH)
        {
            return 1;
        }
        rowTop += kRowH;
    }
    if (HasOut && Y < rowTop + kRowH)
    {
        return 2;
    }
    return -1;
}


void EntranceGroupBoxItem::mousePressEvent(QGraphicsSceneMouseEvent* e)
{
    // Dispatch the click based on which row was hit so the user can target either arm of the pair
    // independently: title → focus this entrance on the current map, In/Out rows → same pipeline
    // as clicking the equivalent marker used to (PerformAction in graph-mode navigates to the
    // destination scene, or selects the tree leaf when the link is still undiscovered).
    if (!Tree)
    {
        return;
    }

    auto clickLink = [](EntranceLinkItemTree* Link)
    {
        if (Link == nullptr)
        {
            return;
        }
        Link->SetCalledFromGraph(true);
        Link->PerformAction();
        Link->SetCalledFromGraph(false);
    };

    switch (RowFromY(e->pos().y()))
    {
        case 1:  clickLink(Tree->InItem);  return;
        case 2:  clickLink(Tree->OutItem); return;
        default: Tree->PerformAction();    return;  // title or bottom padding
    }
}


void EntranceGroupBoxItem::RebuildCurve()
{
    if (!CurveLine || !Anchor)
    {
        return;
    }

    const QRectF boxRect = sceneBoundingRect();
    const QPointF anchorPt = Anchor->Center;

    // Closest point on the axis-aligned rectangle to the anchor: clamping the anchor coordinates
    // into the box range always lands on the perimeter (edge or corner) when the anchor is
    // outside the box, which is the shortest path the user asked for.
    const QPointF connPt(
        qBound(boxRect.left(), anchorPt.x(), boxRect.right()),
        qBound(boxRect.top(),  anchorPt.y(), boxRect.bottom())
    );

    QPainterPath path;
    path.moveTo(anchorPt);
    path.lineTo(connPt);
    CurveLine->setPath(path);
}

#pragma endregion // EntranceGroupBoxItem

#pragma region // EntranceAnchorItem

EntranceAnchorItem::EntranceAnchorItem(QPointF Center, EntranceGroupBoxItem* PaBox)
    : BoxItem(PaBox), Center(Center)
{
    constexpr qreal r = 5.0;
    QPolygonF d;
    d << QPointF(Center.x(), Center.y() - r)
        << QPointF(Center.x() + r, Center.y())
        << QPointF(Center.x(), Center.y() + r)
        << QPointF(Center.x() - r, Center.y());
    setPolygon(d);
    setBrush(QBrush(QColor(255, 255, 255, 204)));
    setPen(QPen(QColor(0, 0, 0, 140), 0.8));
    setAcceptHoverEvents(true);
    setZValue(12);
}


void EntranceAnchorItem::SetHighlighted(bool H)
{
    setBrush(QBrush(H ? Qt::white : QColor(255, 255, 255, 204)));
    setZValue(H ? 22 : 12);
    if (CurveLine)
    {
        QPen pen;
        if (H)
        {
            pen = QPen(QColor(255, 255, 255, 220), 1.5);
            pen.setStyle(Qt::SolidLine);
        }
        else
        {
            pen = QPen(QColor(255, 255, 255, 77), 0.8);
            pen.setStyle(Qt::CustomDashLine);
            pen.setDashPattern({ 4.0, 3.5 });
        }
        CurveLine->setPen(pen);
        CurveLine->setZValue(H ? 11 : 9);
    }
}


void EntranceAnchorItem::hoverEnterEvent(QGraphicsSceneHoverEvent* e)
{
    SetHighlighted(true);
    if (BoxItem)
    {
        BoxItem->SetHighlighted(true);
        BoxItem->SetTreeHighlighted(true);

        if (BoxItem->Renderer)
        {
            BoxItem->Renderer->SetGroupFocus(BoxItem);
        }
    }
    QGraphicsPolygonItem::hoverEnterEvent(e);
}


void EntranceAnchorItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* e)
{
    SetHighlighted(false);
    if (BoxItem)
    {
        BoxItem->SetHighlighted(false);
        BoxItem->SetTreeHighlighted(false);

        if (BoxItem->Renderer)
        {
            BoxItem->Renderer->SetGroupFocus(nullptr);
        }
    }
    QGraphicsPolygonItem::hoverLeaveEvent(e);
}


void EntranceAnchorItem::mousePressEvent(QGraphicsSceneMouseEvent*)
{
    if (BoxItem && BoxItem->Tree) BoxItem->Tree->PerformAction();
}

#pragma endregion

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


void EntranceRenderer::SetGroupFocus(EntranceGroupBoxItem* PaFocused)
{
    constexpr qreal kDimOpacity = 0.12;
    constexpr qreal kFullOpacity = 1.00;

    for (const OverlayGroup& g : this->Groups)
    {
        const bool isFocused = (PaFocused == nullptr || g.Box == PaFocused);
        const qreal opacity = isFocused ? kFullOpacity : kDimOpacity;

        if (g.Box)    g.Box->setOpacity(opacity);
        if (g.Anchor) g.Anchor->setOpacity(opacity);
        if (g.Curve)  g.Curve->setOpacity(opacity);
    }
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
    this->Groups.clear();
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


void EntranceRenderer::RenderSceneOverlayGrouped(SceneEntranceMetaInf* SceneMeta)
{
    if (!SceneMeta || !this->Scene) return;

    for (auto& [entranceID, link] : SceneMeta->EntranceIDs)
    {
        const EntranceMetaInfo* meta = EntranceHelper::GetEntranceMetaInf(
            this->GetGameID(), entranceID);
        if (!meta || meta->Type == EntranceType::None) continue;

        EntranceItemTree* tree = this->FindEntrance(SceneMeta->SceneID, entranceID);
        if (!tree) continue;

        // ── Grouped box at TextPos ─────────────────────────────────────────────
        EntranceGroupBoxItem* box = new EntranceGroupBoxItem(tree);
        box->setPos(meta->TextPos[0], meta->TextPos[1]);
        this->Scene->addItem(box);
        this->OverlayItems.push_back(box);

        // ── Diamond anchor at AnchorPos ────────────────────────────────────────
        const QPointF anchorPt(meta->AnchorPos[0], meta->AnchorPos[1]);
        EntranceAnchorItem* anchor = new EntranceAnchorItem(anchorPt, box);
        this->Scene->addItem(anchor);
        this->OverlayItems.push_back(anchor);

        // ── Dashed line ────────────────────────────────────────────────────────
        // The curve path itself is computed by EntranceGroupBoxItem::RebuildCurve so the exact
        // same logic runs again when a discovered name grows the box. Create an empty path item
        // first, wire up the back-pointers, then ask the box to populate the path.
        QGraphicsPathItem* curve = new QGraphicsPathItem();
        QPen pen(QColor(255, 255, 255, 77), 0.8);
        pen.setStyle(Qt::CustomDashLine);
        pen.setDashPattern({ 4.0, 3.5 });
        curve->setPen(pen);
        curve->setZValue(9);
        this->Scene->addItem(curve);
        this->OverlayItems.push_back(curve);

        // ── Cross-wire highlight pointers ──────────────────────────────────────
        box->Anchor = anchor;
        box->CurveLine = curve;
        anchor->CurveLine = curve;
        tree->GroupBox = box;
        box->RebuildCurve();

        // ── Register InItem/OutItem so tree↔map interactions still work ────────
        if (tree->InItem)
        {
            tree->InItem->GraphItem = anchor;   // reuse GraphItem as the clickable proxy
            tree->InItem->TextItem = nullptr;  // no floating text label in this mode
        }
        if (tree->OutItem)
        {
            tree->OutItem->GraphItem = anchor;
            tree->OutItem->TextItem = nullptr;
        }

        this->Groups.push_back({ box, anchor, curve });
        box->Renderer = this;
    }
}


void EntranceRenderer::AddLinkMarker(EntranceLinkItemTree* Link, bool IsIn, const EntranceMetaInfo* Meta)
{
    if (Link == nullptr || Meta == nullptr || this->Scene == nullptr)
    {
        return;
    }

    const int* pos = Meta->AnchorPos;
    /*const int* pos = IsIn ? Meta->InPosition : Meta->OutPosition;
    qreal rotDeg = IsIn ? Meta->ArrowRot : Meta->ArrowRot + 180.0;*/

    // Arrow marker: the shared placeholder pixmap is already centered on its own origin so setPos
    // pins the arrow's center to the entrance coordinates and setRotation pivots in place. The Out
    // side is rotated by an extra 180 degrees so a Normal entrance's In / Out pair always point in
    // visually opposite directions.
    //EntrancePixmapItem* arrow = new EntrancePixmapItem(IsIn ? ObjectIcons::GetIconsRef()->PixmapEntranceIcons[1] : ObjectIcons::GetIconsRef()->PixmapEntranceIcons[2], this, Link);
   
    double scaleFactor = 0.02;
    int iconWidth = this->Scene->sceneRect().width() * scaleFactor;
    int iconHeight = this->Scene->sceneRect().height() * scaleFactor;

    iconWidth = iconWidth < EntranceIconsMetaInfo[Meta->RenderIcon].Scale[0] ? EntranceIconsMetaInfo[Meta->RenderIcon].Scale[0] : iconWidth > EntranceIconsMetaInfo[Meta->RenderIcon].MaxScale[0] ? EntranceIconsMetaInfo[Meta->RenderIcon].MaxScale[0] : iconWidth;
    iconHeight = iconHeight < EntranceIconsMetaInfo[Meta->RenderIcon].Scale[1] ? EntranceIconsMetaInfo[Meta->RenderIcon].Scale[1] : iconHeight > EntranceIconsMetaInfo[Meta->RenderIcon].MaxScale[1] ? EntranceIconsMetaInfo[Meta->RenderIcon].MaxScale[1] : iconHeight;

    
    EntrancePixmapItem* arrow = new EntrancePixmapItem(ObjectIcons::GetIconsRef()->PixmapEntranceIcons[Meta->RenderIcon].scaled(iconWidth, iconHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation), this, Link);

    arrow->setPos(pos[0], pos[1]);
    //arrow->setRotation(rotDeg);
    this->Scene->addItem(arrow);
    this->OverlayItems.push_back(arrow);
    //Link->GraphItem = arrow;

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
        //PlaceLabelAroundArrow(label, pos[0], pos[1], rotDeg, pos[2]);
        pos = Meta->TextPos;
        PlaceLabelAroundArrow(label, pos[0], pos[1], 0, pos[2]);

        this->Scene->addItem(label);
        this->OverlayItems.push_back(label);
        Link->TextItem = label;
    }
}


void EntranceRenderer::ResolveOverlaps()
{
    // Collect every visible label together with its anchor (matching arrow center).
    struct LabelEntry
    {
        QGraphicsSimpleTextItem* Label;
        QPointF                  Anchor;  // arrow center in scene space
        QRectF                   Rect;    // scene-space bounding rect (mutated each iter)
    };

    std::vector<LabelEntry> entries;

    for (EntranceItemTree* tree : this->Entrances)
    {
        if (tree == nullptr) continue;

        auto collect = [&](EntranceLinkItemTree* link)
            {
                if (!link || !link->TextItem || !link->TextItem->isVisible()) return;
                if (!link->GraphItem) return;
                entries.push_back({
                    link->TextItem,
                    link->GraphItem->scenePos(),
                    link->TextItem->sceneBoundingRect()
                    });
            };

        collect(tree->InItem);
        collect(tree->OutItem);
    }

    if (entries.size() < 2) return;

    constexpr qreal kMGap = 3.0;   // minimum gap enforced between two labels
    constexpr qreal kPush = 50;   // pixels pushed apart per iteration
    constexpr qreal kAttract = 0;  // attraction factor toward anchor (0 = none, 1 = snap)
    constexpr int   kMaxIter = 120;

    auto overlaps = [&](const QRectF& a, const QRectF& b) -> bool
        {
            return a.adjusted(-kMGap, -kMGap, kMGap, kMGap).intersects(b);
        };

    for (int iter = 0; iter < kMaxIter; ++iter)
    {
        bool anyOverlap = false;

        for (size_t i = 0; i < entries.size(); ++i)
        {
            for (size_t j = i + 1; j < entries.size(); ++j)
            {
                LabelEntry& ei = entries[i];
                LabelEntry& ej = entries[j];
                if (!overlaps(ei.Rect, ej.Rect)) continue;
                anyOverlap = true;

                const QPointF ci = ei.Rect.center();
                const QPointF cj = ej.Rect.center();
                QPointF       diff = ci - cj;
                const qreal   dist = std::hypot(diff.x(), diff.y());
                if (dist < 0.01) diff = QPointF(1.0, 0.0);
                else            diff /= dist;

                const QPointF push = diff * kPush;
                const QPointF attrI = (ei.Anchor - ci) * kAttract;
                const QPointF attrJ = (ej.Anchor - cj) * kAttract;

                ei.Rect.moveCenter(ci + push + attrI);
                ej.Rect.moveCenter(cj - push + attrJ);
            }
        }

        if (!anyOverlap) break;
    }

    // Commit resolved positions. Labels are direct scene children (no parent
    // item transform), so scene pos == item local pos.
    for (LabelEntry& e : entries)
    {
        const QPointF delta = e.Rect.topLeft() - e.Label->sceneBoundingRect().topLeft();
        e.Label->setPos(e.Label->pos() + delta);
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
    constexpr qreal kOffset = 20.0;

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
