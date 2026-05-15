#include "UI/GPSRouteWidget.h"
#include "UI/GPSRouteCard.h"
#include "UI/GPSPathfinder.h"
#include "Combo/Scenes.h"
#include "Multi/Game.h"

#include <QSignalBlocker>
#include <QVariant>


namespace
{
    constexpr int       SceneRoleGame  = Qt::UserRole + 1;
    constexpr int       SceneRoleID    = Qt::UserRole + 2;

    /*
    *   Format a raw cost (sum of per-edge units) as a M:SS string. Each cost unit
    *   is treated as 30 seconds of travel for now; this will be revisited once
    *   real per-entrance travel times are wired in.
    */
    QString FormatCost(uint32_t Cost)
    {
        const uint32_t Seconds = Cost * 30u;
        const uint32_t M = Seconds / 60u;
        const uint32_t S = Seconds % 60u;
        return QString("%1:%2").arg(M).arg(S, 2, 10, QChar('0'));
    }

    /*
    *   Convert one GPSPath returned by the pathfinder into the Route format the
    *   GPSRouteCard knows how to render.
    */
    Route ToDisplayRoute(const GPSPath& Path, int Index, bool IsBest)
    {
        Route R;
        R.Rank = QString("ROUTE %1").arg(Index + 1);
        R.TotalTime = FormatCost(Path.TotalCost);
        R.IsBest = IsBest;

        if (IsBest)
        {
            R.Label  = "rapide";
            R.Notes  = QString("%1 transitions \xC2\xB7 trajet direct").arg(qMax(0, int(Path.Steps.size()) - 1));
            R.Accent = QColor("#3ddc84");
        }
        else if (Index == 1)
        {
            R.Label  = QString::fromUtf8("\xC3\xA9quilibr\xC3\xA9""e");      // "équilibrée"
            R.Notes  = QString("%1 transitions \xC2\xB7 alternative").arg(qMax(0, int(Path.Steps.size()) - 1));
            R.Accent = QColor("#e8a82d");
        }
        else
        {
            R.Label  = QString::fromUtf8("d\xC3\xA9tour");                   // "détour"
            R.Notes  = QString("%1 transitions \xC2\xB7 panoramique").arg(qMax(0, int(Path.Steps.size()) - 1));
            R.Accent = QColor("#ff5252");
        }

        for (int i = 0; i < Path.Steps.size(); ++i)
        {
            const GPSPathStep& S = Path.Steps[i];
            RouteStep Step;
            Step.StationName = S.SceneName;
            Step.Via = Via_Walk;
            Step.DurationSec = int(S.Cost) * 30;
            R.Steps.append(Step);
        }

        return R;
    }
}


#pragma region // Class creation

GPSRouteWidget::GPSRouteWidget(QWidget* Parent)
    : QWidget(Parent)
{
    this->setObjectName("GPSRouteWidget");

    this->MainLayout = new QVBoxLayout(this);
    this->MainLayout->setContentsMargins(14, 14, 14, 14);
    this->MainLayout->setSpacing(12);

    this->BuildTopBar();
    this->BuildCardsHost();

    this->PopulateSceneCombos();

    // Trigger the initial render once the combos are populated.
    this->OnSelectionChanged();
}

#pragma endregion


#pragma region // Methods

void GPSRouteWidget::SetRoutes(const QVector<Route>& Routes)
{
    this->ClearCards();

    if (Routes.isEmpty())
    {
        this->SetEmpty();
        return;
    }

    if (this->Placeholder != nullptr) this->Placeholder->setVisible(false);

    for (const Route& R : Routes)
    {
        GPSRouteCard* Card = new GPSRouteCard(R, this->CardsHost);
        this->CardsRow->addWidget(Card, 1);
        this->Cards.append(Card);
    }

    if (this->Summary != nullptr)
    {
        this->Summary->setText(QString("%1 routes trouvées").arg(Routes.size()));
    }
}


void GPSRouteWidget::SetEmpty()
{
    this->SetMessage(QString::fromUtf8("Aucun chemin trouv\xC3\xA9"));
}


void GPSRouteWidget::SetMessage(const QString& Message)
{
    this->ClearCards();
    if (this->Placeholder != nullptr)
    {
        this->Placeholder->setText(Message);
        this->Placeholder->setVisible(true);
    }
    if (this->Summary != nullptr)
    {
        this->Summary->setText(Message);
    }
}

#pragma endregion


#pragma region // Helpers

void GPSRouteWidget::BuildTopBar()
{
    this->TopBar = new QHBoxLayout();
    this->TopBar->setContentsMargins(0, 0, 0, 0);
    this->TopBar->setSpacing(8);

    this->Tag = new QLabel("GPS", this);
    this->Tag->setObjectName("GpsTag");
    this->TopBar->addWidget(this->Tag);

    this->FromCombo = new QComboBox(this);
    this->FromCombo->setObjectName("GpsFromCombo");
    this->FromCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    this->TopBar->addWidget(this->FromCombo);

    this->ArrowLabel = new QLabel(QString(QChar(0x2192)), this);
    this->ArrowLabel->setObjectName("GpsArrow");
    this->TopBar->addWidget(this->ArrowLabel);

    this->ToCombo = new QComboBox(this);
    this->ToCombo->setObjectName("GpsToCombo");
    this->ToCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    this->TopBar->addWidget(this->ToCombo);

    this->SwapButton = new QPushButton(QString(QChar(0x21C5)) + " Inverser", this);
    this->SwapButton->setObjectName("GpsSwapButton");
    this->SwapButton->setCursor(Qt::PointingHandCursor);
    this->TopBar->addWidget(this->SwapButton);

    this->TopBar->addStretch(1);

    this->Summary = new QLabel("0 routes trouvées", this);
    this->Summary->setObjectName("GpsSummary");
    this->Summary->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    this->TopBar->addWidget(this->Summary);

    this->MainLayout->addLayout(this->TopBar);

    connect(this->FromCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GPSRouteWidget::OnSelectionChanged);
    connect(this->ToCombo,   QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &GPSRouteWidget::OnSelectionChanged);
    connect(this->SwapButton, &QPushButton::clicked,
            this, &GPSRouteWidget::OnSwapClicked);
}


void GPSRouteWidget::BuildCardsHost()
{
    this->CardsHost = new QWidget(this);
    this->CardsHost->setObjectName("GpsCardsHost");

    this->CardsRow = new QHBoxLayout(this->CardsHost);
    this->CardsRow->setContentsMargins(0, 0, 0, 0);
    this->CardsRow->setSpacing(10);

    this->Placeholder = new QLabel("Aucun chemin trouvé", this->CardsHost);
    this->Placeholder->setObjectName("GpsPlaceholder");
    this->Placeholder->setAlignment(Qt::AlignCenter);
    this->Placeholder->setVisible(false);
    this->CardsRow->addWidget(this->Placeholder, 1);

    this->MainLayout->addWidget(this->CardsHost, 1);
}


void GPSRouteWidget::PopulateSceneCombos()
{
    QSignalBlocker BlockFrom(this->FromCombo);
    QSignalBlocker BlockTo(this->ToCombo);

    this->FromCombo->clear();
    this->ToCombo->clear();

    auto AddScenesForGame = [&](uint32_t Game, uint32_t Count, const char* Tag)
    {
        for (uint32_t i = 0; i < Count; ++i)
        {
            SceneMetaInfo* Meta = GetSceneMetaInfo(i, Game);
            if (Meta == nullptr) continue;
            if (Meta->Name == nullptr || Meta->Name[0] == '\0') continue;

            const QString Display = QString("[%1] %2").arg(Tag).arg(Meta->Name);
            const int Idx = this->FromCombo->count();
            this->FromCombo->addItem(Display);
            this->FromCombo->setItemData(Idx, (int)Game, SceneRoleGame);
            this->FromCombo->setItemData(Idx, (uint)i,   SceneRoleID);
            this->ToCombo->addItem(Display);
            this->ToCombo->setItemData(Idx, (int)Game, SceneRoleGame);
            this->ToCombo->setItemData(Idx, (uint)i,   SceneRoleID);
        }
    };

    AddScenesForGame(OOT_GAME, OOT_NUM_SCENES, "OoT");
    AddScenesForGame(MM_GAME,  MM_NUM_SCENES,  "MM");

    // Default to two different scenes if possible so the initial demo isn't empty.
    if (this->FromCombo->count() > 0) this->FromCombo->setCurrentIndex(0);
    if (this->ToCombo->count() > 1)   this->ToCombo->setCurrentIndex(1);
}


void GPSRouteWidget::ClearCards()
{
    for (GPSRouteCard* Card : this->Cards)
    {
        if (Card == nullptr) continue;
        this->CardsRow->removeWidget(Card);
        Card->deleteLater();
    }
    this->Cards.clear();
}

#pragma endregion


#pragma region // Slots

void GPSRouteWidget::OnSelectionChanged()
{
    if (this->FromCombo == nullptr || this->ToCombo == nullptr) return;
    if (this->FromCombo->currentIndex() < 0 || this->ToCombo->currentIndex() < 0) return;

    const int       FromGame  = this->FromCombo->currentData(SceneRoleGame).toInt();
    const uint32_t  FromScene = this->FromCombo->currentData(SceneRoleID).toUInt();
    const int       ToGame    = this->ToCombo->currentData(SceneRoleGame).toInt();
    const uint32_t  ToScene   = this->ToCombo->currentData(SceneRoleID).toUInt();

    const GPSPathfindResult Result = FindGPSRoutes(FromGame, FromScene, ToGame, ToScene, 3);

    switch (Result.Status)
    {
        case GPS_SameScene:
            this->SetMessage("Start and destination are the same scene");
            return;

        case GPS_CrossGameUnsupported:
            this->SetMessage("Cross-game routing not supported yet");
            return;

        case GPS_CannotLeaveStart:
            this->SetMessage("Cannot leave Start Area");
            return;

        case GPS_DestinationUnreachable:
            this->SetMessage("Destination Unreachable");
            return;

        case GPS_NoPath:
            this->SetEmpty();
            return;

        case GPS_Ok:
            break;
    }

    QVector<Route> Display;
    Display.reserve(Result.Routes.size());
    for (int i = 0; i < Result.Routes.size(); ++i)
    {
        Display.append(ToDisplayRoute(Result.Routes[i], i, i == 0));
    }
    this->SetRoutes(Display);
}


void GPSRouteWidget::OnSwapClicked()
{
    if (this->FromCombo == nullptr || this->ToCombo == nullptr) return;

    const int FromIdx = this->FromCombo->currentIndex();
    const int ToIdx   = this->ToCombo->currentIndex();

    QSignalBlocker BlockFrom(this->FromCombo);
    QSignalBlocker BlockTo(this->ToCombo);
    this->FromCombo->setCurrentIndex(ToIdx);
    this->ToCombo->setCurrentIndex(FromIdx);

    this->OnSelectionChanged();
}

#pragma endregion
