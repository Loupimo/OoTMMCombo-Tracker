#include "UI/GPSRouteWidget.h"
#include "UI/GPSRouteCard.h"
#include "UI/GPSPathfinder.h"
#include "UI/OoTMMComboTracker.h"
#include "UI/Settings.h"
#include "Combo/Entrances.h"
#include "Combo/OoTEntrances.h"
#include "Combo/MMEntrances.h"
#include "Combo/Scenes.h"
#include "Multi/Game.h"

#include <QSignalBlocker>
#include <QVariant>
#include <QSet>
#include <QStringList>
#include <QCompleter>
#include <QLineEdit>

#include <algorithm>
#include <set>
#include <vector>


namespace
{
    constexpr int       SceneRoleGame  = Qt::UserRole + 1;
    constexpr int       SceneRoleID    = Qt::UserRole + 2;

    /*
    *   Format a raw cost (already expressed in seconds since the per-edge values come from
    *   the real measurements in OoT/MMEntranceCosts.cpp) as a M:SS string. No * 30 multiplier
    *   anymore: a cost of 20 is 20 seconds, not 20 cost-units of 30s each.
    */
    QString FormatCost(uint32_t Cost)
    {
        const uint32_t Seconds = Cost;
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
            R.Label  = "Fastest";
            R.Notes  = QString("%1 transitions \xC2\xB7 direct route").arg(qMax(0, int(Path.Steps.size()) - 1));
            R.Accent = QColor("#3ddc84");
        }
        else if (Index == 1)
        {
            R.Label  = QString::fromUtf8("Balanced");
            R.Notes  = QString("%1 transitions \xC2\xB7 alternative").arg(qMax(0, int(Path.Steps.size()) - 1));
            R.Accent = QColor("#e8a82d");
        }
        else
        {
            R.Label  = QString::fromUtf8("Detour");
            R.Notes  = QString("%1 transitions \xC2\xB7 panoramic").arg(qMax(0, int(Path.Steps.size()) - 1));
            R.Accent = QColor("#ff5252");
        }

        for (int i = 0; i < Path.Steps.size(); ++i)
        {
            const GPSPathStep& S = Path.Steps[i];
            RouteStep Step;
            Step.StationName = S.SceneName;
            Step.Via = Via_Walk;
            Step.DurationSec = int(S.Cost);
            // Show the actual exit door name in the "via" caption instead of the generic
            // "Walk" label. ToScenePath populates ViaText with the destination scene of the
            // exit entrance (e.g. "Saria's House" when leaving Kokiri Forest through that door).
            Step.ViaCustom = S.ViaText;
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
        this->Summary->setText(QString("%1 routes found").arg(Routes.size()));
    }
}


void GPSRouteWidget::SetEmpty()
{
    this->SetMessage(QString::fromUtf8("No route found"));
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
    this->MakeComboSearchable(this->FromCombo);
    this->TopBar->addWidget(this->FromCombo);

    this->ArrowLabel = new QLabel(QString(QChar(0x2192)), this);
    this->ArrowLabel->setObjectName("GpsArrow");
    this->TopBar->addWidget(this->ArrowLabel);

    this->ToCombo = new QComboBox(this);
    this->ToCombo->setObjectName("GpsToCombo");
    this->ToCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    this->MakeComboSearchable(this->ToCombo);
    this->TopBar->addWidget(this->ToCombo);

    this->SwapButton = new QPushButton(QString(QChar(0x21C5)) + " Invert", this);
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
    // Anchor the cards to the top so a card that grew tall (long route) gets clipped by the
    // scroll viewport on the bottom edge instead of stretching the row vertically.
    this->CardsRow->setAlignment(Qt::AlignTop);

    this->Placeholder = new QLabel("No route found", this->CardsHost);
    this->Placeholder->setObjectName("GpsPlaceholder");
    this->Placeholder->setAlignment(Qt::AlignCenter);
    this->Placeholder->setVisible(false);
    this->CardsRow->addWidget(this->Placeholder, 1);

    // Wrap the cards host in a scroll viewport so that:
    //   - vertical: a long route (many directions) scrolls inside the GPS tab instead of
    //     pushing the main window taller.
    //   - horizontal: when many wide cards stack side by side they scroll instead of
    //     forcing the window wider.
    // The viewport is transparent so the tab background still shines through; the host
    // widget keeps its own object name so the existing QSS rules remain in effect.
    this->CardsScroll = new QScrollArea(this);
    this->CardsScroll->setObjectName("GpsCardsScroll");
    this->CardsScroll->setWidget(this->CardsHost);
    this->CardsScroll->setWidgetResizable(true);
    this->CardsScroll->setFrameShape(QFrame::NoFrame);
    this->CardsScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    this->CardsScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    this->CardsScroll->setStyleSheet("QScrollArea, QScrollArea > QWidget > QWidget { background: transparent; }");

    this->MainLayout->addWidget(this->CardsScroll, 1);
}


void GPSRouteWidget::PopulateSceneCombos()
{
    QSignalBlocker BlockFrom(this->FromCombo);
    QSignalBlocker BlockTo(this->ToCombo);

    this->FromCombo->clear();
    this->ToCombo->clear();

    // Local entry describing one scene candidate: kept in a vector so we can sort the whole
    // list before pushing into the combo (Qt combo sorting doesn't honor a custom multi-key
    // ordering out of the box).
    struct SceneEntry
    {
        int     Game;           // OOT_GAME (= 0) sorts before MM_GAME (= 1) by integer order.
        uint32_t Id;
        QString  Display;       // The combo label, e.g. "[OoT] Kokiri Forest".
        QString  Name;          // Plain scene name used as the sort key (no tag prefix).
    };

    auto CollectScenesForGame = [](int Game, const char* Tag, std::vector<SceneEntry>& Out)
    {
        // Build the set of scenes that actually participate in the entrance network, i.e. that
        // have at least one entrance with FromSceneID == S or ToSceneID == S in the game's
        // entrance map. This matches what AllEntranceView shows: only scenes the player can
        // ever leave or arrive in. Scenes that exist only as data placeholders (cutscene maps,
        // unused IDs) are filtered out so they don't pollute the dropdown.
        const std::map<int, EntranceMetaInfo>& Map = (Game == OOT_GAME) ? OoTEntrances : MMEntrances;

        std::set<uint32_t> Valid;
        for (const auto& Pair : Map)
        {
            const EntranceMetaInfo& V = Pair.second;
            if (V.Type == EntranceType::None) continue;
            Valid.insert(V.FromSceneID);
            Valid.insert(V.ToSceneID);
        }

        for (uint32_t SceneID : Valid)
        {
            SceneMetaInfo* Meta = GetSceneMetaInfo(SceneID, (uint32_t)Game);
            if (Meta == nullptr) continue;
            if (Meta->Name == nullptr || Meta->Name[0] == '\0') continue;

            SceneEntry E;
            E.Game = Game;
            E.Id = SceneID;
            E.Name = QString::fromUtf8(Meta->Name);
            E.Display = QString("[%1] %2").arg(Tag, E.Name);
            Out.push_back(std::move(E));
        }
    };

    std::vector<SceneEntry> Entries;
    CollectScenesForGame(OOT_GAME, "OoT", Entries);
    CollectScenesForGame(MM_GAME,  "MM",  Entries);

    // Sort: OoT first (Game == 0), then MM (Game == 1), then alphabetically by scene name
    // within each game (locale-aware so accents and diacritics sort sensibly).
    std::sort(Entries.begin(), Entries.end(),
        [](const SceneEntry& A, const SceneEntry& B)
        {
            if (A.Game != B.Game) return A.Game < B.Game;
            return QString::localeAwareCompare(A.Name, B.Name) < 0;
        });

    for (const SceneEntry& E : Entries)
    {
        const int Idx = this->FromCombo->count();
        this->FromCombo->addItem(E.Display);
        this->FromCombo->setItemData(Idx, E.Game,           SceneRoleGame);
        this->FromCombo->setItemData(Idx, (uint)E.Id,       SceneRoleID);
        this->ToCombo->addItem(E.Display);
        this->ToCombo->setItemData(Idx, E.Game,             SceneRoleGame);
        this->ToCombo->setItemData(Idx, (uint)E.Id,         SceneRoleID);
    }

    // Default to two different scenes if possible so the initial render isn't empty.
    if (this->FromCombo->count() > 0) this->FromCombo->setCurrentIndex(0);
    if (this->ToCombo->count() > 1)   this->ToCombo->setCurrentIndex(1);
}


void GPSRouteWidget::MakeComboSearchable(QComboBox* Combo)
{
    if (Combo == nullptr) return;

    Combo->setEditable(true);
    Combo->setInsertPolicy(QComboBox::NoInsert);            // The edit field is a search box: it never adds new scenes.
    Combo->lineEdit()->setPlaceholderText("Find Scene...");

    QCompleter* Completer = Combo->completer();
    if (Completer != nullptr)
    {
        Completer->setCompletionMode(QCompleter::PopupCompletion);
        Completer->setFilterMode(Qt::MatchContains);        // Match anywhere in the scene name, not only its prefix.
        Completer->setCaseSensitivity(Qt::CaseInsensitive);
    }

    // If the field is left holding text that matches no scene (typed then clicked away), snap it back
    // to the current selection so the combo never displays a non-existent scene. A full valid name that
    // resolves to another item selects it (and re-triggers the route computation via currentIndexChanged).
    connect(Combo->lineEdit(), &QLineEdit::editingFinished, this, [Combo]()
    {
        const int Match = Combo->findText(Combo->lineEdit()->text(), Qt::MatchFixedString);
        if (Match >= 0)
        {
            Combo->setCurrentIndex(Match);
        }
        else
        {
            Combo->lineEdit()->setText(Combo->itemText(Combo->currentIndex()));
        }
    });
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

    // The two cross-game warp ROM parameters live in the main window's ROMSettings. We
    // walk up the parent chain to find the OoTMMComboTracker instance and read them as
    // ShuffleSetting::all == enabled. If the lookup fails (test harness, orphan widget)
    // we fall back to "enabled" so the GPS keeps the most permissive behavior.
    bool CrossWarpOot = true;
    bool CrossWarpMm  = true;
    if (auto* Main = qobject_cast<OoTMMComboTracker*>(this->window()))
    {
        auto& Filters = Main->ROMSettings.FilterSettings;
        auto OotIt = Filters.find(QStringLiteral("crossWarpOot"));
        auto MmIt  = Filters.find(QStringLiteral("crossWarpMm"));
        if (OotIt != Filters.end()) CrossWarpOot = (OotIt.value().Value == ShuffleSetting::all);
        if (MmIt  != Filters.end()) CrossWarpMm  = (MmIt.value().Value  == ShuffleSetting::all);
    }

    // K = 5 gives Yen's algorithm enough breathing room to surface meaningful alternatives
    // (different routes through warps, walking-only fallback, etc.) before our dedup pass trims
    // them down to 1-3 unique cards. K=3 was too tight: when all of Yen's first three paths
    // were minor variants of the same warp-based route they collapsed to a single card.
    const GPSPathfindResult Result = FindGPSRoutes(FromGame, FromScene, ToGame, ToScene, 5,
                                                   CrossWarpOot, CrossWarpMm);

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

    // Deduplicate routes by:
    //   1) Scene-sequence signature: Yen's algorithm can return paths that look identical to
    //      the user (same scenes visited in the same order) when alternative intermediate
    //      entrances produce the same scene trip. Showing 3 visually identical cards is useless.
    //   2) Warp usage: once one route has used a warp song / owl (= passes through MM_OWLS or
    //      OOT_SONGS), subsequent routes that ALSO use a warp are filtered out. The point of
    //      alternative routes is to offer the player a fallback when the warp doesn't fit -
    //      stacking near-identical warp routes that differ only by which choice was picked is
    //      noise.
    QVector<int> UniqueIndices;
    QSet<QString> Seen;
    bool WarpRouteShown = false;
    for (int idx = 0; idx < Result.Routes.size(); ++idx)
    {
        const GPSPath& P = Result.Routes[idx];

        bool UsesWarp = false;
        QStringList SceneIds;
        SceneIds.reserve(P.Steps.size());
        for (const GPSPathStep& S : P.Steps)
        {
            // Signature includes Game so that OoT and MM scenes with colliding numeric IDs
            // (rare but possible in the spoiler-log sentinel range) don't get falsely merged.
            SceneIds.append(QStringLiteral("%1:%2").arg(S.Game).arg(S.SceneID));
            if ((S.Game == OOT_GAME && S.SceneID == OOT_SONGS) ||
                (S.Game == MM_GAME  && S.SceneID == MM_OWLS))
            {
                UsesWarp = true;
            }
        }

        if (UsesWarp && WarpRouteShown) continue;

        const QString Signature = SceneIds.join('>');
        if (Seen.contains(Signature)) continue;
        Seen.insert(Signature);

        UniqueIndices.append(idx);
        if (UsesWarp) WarpRouteShown = true;
        if (UniqueIndices.size() >= 3) break;
    }

    if (UniqueIndices.isEmpty())
    {   // Pathfinder said OK but every returned path was empty (shouldn't happen) - treat as none.
        this->SetEmpty();
        return;
    }

    QVector<Route> Display;
    Display.reserve(UniqueIndices.size());
    for (int i = 0; i < UniqueIndices.size(); ++i)
    {
        Display.append(ToDisplayRoute(Result.Routes[UniqueIndices[i]], i, i == 0));
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
