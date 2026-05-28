#include "UI/GPSRouteCard.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QStyle>
#include <QLinearGradient>
#include <QPen>
#include <QBrush>
#include <QFont>


// Visual constants kept local to the file: the geometry of the diagram column
// (line X, circle diameter, halo size) must match the QSS padding chosen for
// GPSRouteCard so the line passes pile au centre des cercles.
namespace
{
    constexpr int   DiagramLeftPad  = 22;       // Left padding of the diagram container relative to the card content area.
    constexpr int   LineX           = 8;        // X position (inside the diagram container) of the vertical line and the circles centers.
    constexpr int   CircleDiameter  = 14;
    constexpr int   HaloDiameter    = 20;       // Diameter of the dark "halo" around each circle, mimics box-shadow: 0 0 0 3px #0a0a14.
    constexpr int   StationRowSpacing = 14;     // Vertical gap between two consecutive station rows.
    const     QColor PanelColor     = QColor("#0d1827");
    const     QColor StartColor     = QColor("#3ddc84");
    const     QColor EndColor       = QColor("#ff5252");
}


#pragma region // Class creation

GPSRouteCard::GPSRouteCard(const Route& Data, QWidget* Parent)
    : QFrame(Parent)
    , RouteData(Data)
    , Best(Data.IsBest)
{
    this->setObjectName("GPSRouteCard");
    this->setProperty("best", this->Best);

    QVBoxLayout* Layout = new QVBoxLayout(this);
    Layout->setContentsMargins(14, 14, 14, 10);
    Layout->setSpacing(10);

    this->BuildHeader(Layout);
    this->BuildDiagram(Layout);
    Layout->addStretch(1);
}

#pragma endregion


#pragma region // Methods

void GPSRouteCard::SetBest(bool Value)
{
    if (this->Best == Value) return;
    this->Best = Value;
    this->setProperty("best", Value);
    this->style()->unpolish(this);
    this->style()->polish(this);
    this->update();
}

#pragma endregion


#pragma region // Helpers

void GPSRouteCard::BuildHeader(QVBoxLayout* ParentLayout)
{
    QHBoxLayout* HeaderRow = new QHBoxLayout();
    HeaderRow->setContentsMargins(0, 0, 0, 0);
    HeaderRow->setSpacing(8);

    // Left column: rank (+ optional FASTEST pill), time + label, notes.
    QVBoxLayout* Left = new QVBoxLayout();
    Left->setContentsMargins(0, 0, 0, 0);
    Left->setSpacing(4);

    QHBoxLayout* RankRow = new QHBoxLayout();
    RankRow->setContentsMargins(0, 0, 0, 0);
    RankRow->setSpacing(6);

    QLabel* Rank = new QLabel(this->RouteData.Rank, this);
    Rank->setObjectName("RouteRank");
    Rank->setStyleSheet(QString("color: %1;").arg(this->RouteData.Accent.name()));
    RankRow->addWidget(Rank);

    if (this->Best)
    {
        QLabel* Pill = new QLabel(QString::fromUtf8("\xE2\x98\x85 FASTEST"), this);
        Pill->setObjectName("RouteBestPill");
        Pill->setStyleSheet(QString(
            "color: %1; border: 1px solid %1; padding: 1px 6px; border-radius: 3px;")
            .arg(this->RouteData.Accent.name()));
        RankRow->addWidget(Pill);
    }
    RankRow->addStretch(1);
    Left->addLayout(RankRow);

    QHBoxLayout* TimeRow = new QHBoxLayout();
    TimeRow->setContentsMargins(0, 0, 0, 0);
    TimeRow->setSpacing(6);

    QLabel* Time = new QLabel(this->RouteData.TotalTime, this);
    Time->setObjectName("RouteTime");
    TimeRow->addWidget(Time);

    QLabel* Label = new QLabel(this->RouteData.Label, this);
    Label->setObjectName("RouteLabel");
    TimeRow->addWidget(Label, 0, Qt::AlignBottom);
    TimeRow->addStretch(1);
    Left->addLayout(TimeRow);

    QLabel* Notes = new QLabel(this->RouteData.Notes, this);
    Notes->setObjectName("RouteNotes");
    Left->addWidget(Notes);

    HeaderRow->addLayout(Left, 1);

    // Right column: transitions count.
    const int Transitions = qMax(0, int(this->RouteData.Steps.size()) - 1);

    QVBoxLayout* Right = new QVBoxLayout();
    Right->setContentsMargins(0, 0, 0, 0);
    Right->setSpacing(0);

    QLabel* Count = new QLabel(QString::number(Transitions), this);
    Count->setObjectName("RouteCount");
    Count->setAlignment(Qt::AlignRight | Qt::AlignTop);
    Count->setStyleSheet(QString("color: %1;").arg(this->RouteData.Accent.name()));
    Right->addWidget(Count);

    QLabel* CountCaption = new QLabel("TRANSITIONS", this);
    CountCaption->setObjectName("RouteSmallCaption");
    CountCaption->setAlignment(Qt::AlignRight | Qt::AlignTop);
    Right->addWidget(CountCaption);

    HeaderRow->addLayout(Right);

    ParentLayout->addLayout(HeaderRow);
}


void GPSRouteCard::BuildDiagram(QVBoxLayout* ParentLayout)
{
    this->Diagram = new QWidget(this);
    this->Diagram->setObjectName("GpsDiagram");

    QVBoxLayout* DLayout = new QVBoxLayout(this->Diagram);
    DLayout->setContentsMargins(DiagramLeftPad, 4, 0, 4);
    DLayout->setSpacing(StationRowSpacing);

    this->StationLabels.clear();
    this->StationLabels.reserve(this->RouteData.Steps.size());

    for (int i = 0; i < this->RouteData.Steps.size(); ++i)
    {
        const RouteStep& Step = this->RouteData.Steps[i];
        const bool IsLast = (i == this->RouteData.Steps.size() - 1);

        QVBoxLayout* Row = new QVBoxLayout();
        Row->setContentsMargins(0, 0, 0, 0);
        Row->setSpacing(2);

        QLabel* Name = new QLabel(Step.StationName, this->Diagram);
        Name->setObjectName("StationName");
        Row->addWidget(Name);
        this->StationLabels.append(Name);

        if (!IsLast)
        {
            QLabel* Via = new QLabel(this->Diagram);
            Via->setObjectName("StationVia");
            const QString GlyphText = QString(ViaGlyph(Step.Via));
            // Prefer the route's custom exit-door label (set by GPSPathfinder via ViaText) over
            // the generic ViaLabel("Walk"/"Owl"/...) so the player sees exactly which entrance
            // to take instead of a generic transit kind.
            const QString ViaText = Step.ViaCustom.isEmpty()
                ? QString::fromUtf8(ViaLabel(Step.Via))
                : Step.ViaCustom;
            Via->setText(QString("%1  via %2  ·  ~%3s")
                .arg(GlyphText)
                .arg(ViaText)
                .arg(Step.DurationSec));
            Row->addWidget(Via);
        }

        DLayout->addLayout(Row);
    }

    DLayout->addStretch(1);
    ParentLayout->addWidget(this->Diagram, 1);
}

#pragma endregion


#pragma region // Events

void GPSRouteCard::paintEvent(QPaintEvent* Event)
{
    // Let QSS paint the frame (background, border, etc.) first.
    QFrame::paintEvent(Event);

    if (this->Diagram == nullptr || this->StationLabels.isEmpty()) return;

    // Compute the X of the vertical line in this card's coordinates.
    const QPoint DiagOrigin = this->Diagram->mapTo(this, QPoint(0, 0));
    const int X = DiagOrigin.x() + LineX;

    // Compute the Y of each station-name label center in this card's coordinates.
    QVector<int> CenterY;
    CenterY.reserve(this->StationLabels.size());
    for (QLabel* L : this->StationLabels)
    {
        if (L == nullptr) continue;
        const QPoint C = L->mapTo(this, QPoint(0, L->height() / 2));
        CenterY.append(C.y());
    }
    if (CenterY.size() < 2) return;

    const int YFirst = CenterY.first();
    const int YLast  = CenterY.last();

    QPainter P(this);
    P.setRenderHint(QPainter::Antialiasing, true);

    // Vertical line: top alpha 255 to bottom alpha 102.
    QColor Top    = this->RouteData.Accent;
    QColor Bottom = this->RouteData.Accent;
    Bottom.setAlpha(102);
    QLinearGradient G(QPointF(X, YFirst), QPointF(X, YLast));
    G.setColorAt(0.0, Top);
    G.setColorAt(1.0, Bottom);
    P.setPen(QPen(QBrush(G), 2.0, Qt::SolidLine, Qt::FlatCap));
    P.drawLine(X, YFirst, X, YLast);

    // Circles: outer halo (panel color) + filled circle with bordure + glyph.
    QFont GlyphFont = this->font();
    GlyphFont.setBold(true);
    GlyphFont.setPointSizeF(qMax(7.0, GlyphFont.pointSizeF() - 2.0));
    P.setFont(GlyphFont);

    for (int i = 0; i < CenterY.size(); ++i)
    {
        const int Y = CenterY[i];
        const bool IsStart = (i == 0);
        const bool IsEnd   = (i == CenterY.size() - 1);

        // Halo - simulates box-shadow: 0 0 0 3px panel
        P.setPen(Qt::NoPen);
        P.setBrush(PanelColor);
        P.drawEllipse(QPoint(X, Y), HaloDiameter / 2, HaloDiameter / 2);

        // Body circle
        QColor Fill;
        QColor Border;
        QString GlyphStr;
        QColor GlyphColor;

        if (IsStart)
        {
            Fill = StartColor;
            Border = QColor("#ffffff");
            GlyphStr = "A";
            GlyphColor = QColor("#000000");
        }
        else if (IsEnd)
        {
            Fill = EndColor;
            Border = QColor("#ffffff");
            GlyphStr = "B";
            GlyphColor = QColor("#000000");
        }
        else
        {
            Fill = PanelColor;
            Border = this->RouteData.Accent;
            GlyphStr = QString::number(i);
            GlyphColor = this->RouteData.Accent;
        }

        P.setPen(QPen(Border, 2.0));
        P.setBrush(Fill);
        P.drawEllipse(QPoint(X, Y), CircleDiameter / 2, CircleDiameter / 2);

        P.setPen(GlyphColor);
        const QRect TextRect(X - CircleDiameter / 2, Y - CircleDiameter / 2,
                             CircleDiameter, CircleDiameter);
        P.drawText(TextRect, Qt::AlignCenter, GlyphStr);
    }
}


void GPSRouteCard::resizeEvent(QResizeEvent* Event)
{
    QFrame::resizeEvent(Event);
    this->update();
}

#pragma endregion
