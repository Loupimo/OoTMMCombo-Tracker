#include "UI/EntranceCostTab.h"
#include "UI/EntranceTable.h"
#include "Combo/Entrances.h"
#include "Combo/OoTEntrances.h"
#include "Combo/MMEntrances.h"
#include "Combo/Scenes.h"
#include "Multi/Game.h"
#include "Multi/Multi.h"

#include <QHeaderView>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QStringList>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QStyle>
#include <QApplication>
#include <QDir>
#include <QHBoxLayout>

#include <algorithm>
#include <map>
#include <set>
#include <tuple>


namespace
{
    // Same path as the TIMER_COST CSV writer in Entrances.cpp. Hard-coded so the
    // cost tab can read it back without the writer having to expose it.
    const QString ENTRANCE_COST_CSV_PATH = ".\\entrance_costs.csv";

    /*
    *   Return the display name of "the other side" of the given entrance relative to the current
    *   scene: the name of whichever of FromName / ToName describes the side NOT located in
    *   CurrentSceneID. This gives a short label like "Deku Tree" or "Lost Woods" instead of the
    *   full "Scene - OtherSide" / "FromName -> ToName" forms.
    *
    *   @param Game             OOT_GAME or MM_GAME.
    *   @param EntranceID       The entrance to label.
    *   @param CurrentSceneID   The scene the entrance is being shown in.
    *
    *   @return The name of the other side of the entrance.
    */
    QString OtherSideName(int Game, uint32_t EntranceID, uint32_t CurrentSceneID)
    {
        const EntranceMetaInfo* Meta = EntranceHelper::GetEntranceMetaInf(Game, EntranceID);
        if (Meta == nullptr)
        {
            return QString("?");
        }

        const char* Name = nullptr;
        if (Meta->FromSceneID == CurrentSceneID)
        {
            Name = Meta->ToName;
        }
        else if (Meta->ToSceneID == CurrentSceneID)
        {
            Name = Meta->FromName;
        }
        else
        {   // Neither side matches the current scene (shouldn't happen with the EntrancesInScene
            // grouping). Fall back to ToName so we still display something.
            Name = Meta->ToName;
        }
        return Name != nullptr ? QString::fromUtf8(Name) : QString("?");
    }


    /*
    *   Vertical header that paints a thin status stripe per row using the BackgroundRole
    *   returned by the model. Bypasses the global QSS (which overrides QHeaderView::section
    *   background) the same way AllEntranceView's StatusHeaderView does it.
    */
    class CostStatusHeaderView : public QHeaderView
    {
    public:
        explicit CostStatusHeaderView(QWidget* Parent = nullptr) : QHeaderView(Qt::Vertical, Parent)
        {
            this->setStyleSheet("QHeaderView, QHeaderView::section { background: transparent; border: none; padding: 0; margin: 0; }");
        }
    protected:
        void paintSection(QPainter* Painter, const QRect& Rect, int LogicalIndex) const override
        {
            if (this->model() == nullptr)
            {
                Painter->fillRect(Rect, QColor(0, 0, 0, 0));
                return;
            }
            const QVariant Bg = this->model()->headerData(LogicalIndex, Qt::Vertical, Qt::BackgroundRole);
            const QColor Color = Bg.isValid() ? Bg.value<QColor>() : QColor(60, 60, 60);
            Painter->fillRect(Rect, Color);
        }
    };


    /*
    *   Delegate that draws a thin translucent separator at the bottom of each cell, matching the
    *   look of the entrance table (which uses a similar trick inside EntranceInLinkDelegate). The
    *   separator is painted manually so we don't have to enable the full grid (which also draws
    *   vertical lines and looks heavier).
    */
    class CostRowSeparatorDelegate : public QStyledItemDelegate
    {
    public:
        using QStyledItemDelegate::QStyledItemDelegate;

        void paint(QPainter* Painter, const QStyleOptionViewItem& Option, const QModelIndex& Index) const override
        {
            QStyledItemDelegate::paint(Painter, Option, Index);
            Painter->save();
            Painter->setPen(QPen(QColor(255, 255, 255, 30), 1));
            Painter->drawLine(Option.rect.bottomLeft(), Option.rect.bottomRight());
            Painter->restore();
        }
    };
}


#pragma region // EntranceCostModel

EntranceCostModel::EntranceCostModel(QObject* Parent) : QAbstractTableModel(Parent)
{
    this->BuildRows();
    // Load any historical measurements written by previous sessions.
    this->LoadCsv(ENTRANCE_COST_CSV_PATH);
    // Live updates: the TIMER_COST writer emits this signal right after each successful
    // measurement so we can refresh just the matching row without re-parsing the CSV.
    connect(MultiLogger::GetLogger(), &MultiLogger::NotifyEntranceCostMeasured,
            this, &EntranceCostModel::OnEntranceCostMeasured);
}


void EntranceCostModel::BuildRows()
{
    this->beginResetModel();
    this->Rows.clear();
    this->BuildRowsForGame(OOT_GAME);
    this->BuildRowsForGame(MM_GAME);

    // Sort by Game, then SceneID, then FromEntranceID, then ToEntranceID so rows of the
    // same scene stay together. The user-driven sort via header click still works through
    // the proxy on top.
    std::stable_sort(this->Rows.begin(), this->Rows.end(),
        [](const EntranceCostRow& A, const EntranceCostRow& B)
        {
            if (A.Game != B.Game) return A.Game < B.Game;
            if (A.SceneID != B.SceneID) return A.SceneID < B.SceneID;
            if (A.FromEntranceID != B.FromEntranceID) return A.FromEntranceID < B.FromEntranceID;
            return A.ToEntranceID < B.ToEntranceID;
        });

    this->RebuildRowColors();
    this->RebuildRowStatusColors();
    this->endResetModel();
}


void EntranceCostModel::BuildRowsForGame(int Game)
{
    const std::map<int, EntranceMetaInfo>& Map = (Game == OOT_GAME) ? OoTEntrances : MMEntrances;

    // For each scene S, build two sets of entrance IDs:
    //   InSet[S]  = entrances through which the player can ARRIVE in S (used as "From" col)
    //   OutSet[S] = entrances through which the player can LEAVE S (used as "To" col)
    //
    // The engine emits the map's KEY (== EntranceMetaInfo.ToEntranceID for our data) both for
    // the OUT message in FromSceneID and the IN message in ToSceneID. So we index by the KEY,
    // NOT by V.FromEntranceID / V.ToEntranceID (which are the partner-direction's keys).
    //
    // Direction rules by EntranceType:
    //   - Normal       : both directions valid (IN at ToSceneID + OUT at FromSceneID).
    //   - One_Way_In   : arrival only (warp song spawn, ...). IN at ToSceneID. Never an OUT,
    //                    so it must NOT appear in the "To" column of any row.
    //   - One_Way_Out  : exit only (wallmaster, cutscene exit, ...). OUT at ToSceneID per the
    //                    ParseOutgoingMessage validation. Never an IN, so it must NOT appear in
    //                    the "From" column of any row.
    std::map<uint32_t, std::set<uint32_t>> InSet;
    std::map<uint32_t, std::set<uint32_t>> OutSet;
    for (const auto& Pair : Map)
    {
        const uint32_t Key = (uint32_t)Pair.first;
        const EntranceMetaInfo& V = Pair.second;
        switch (V.Type)
        {
            case EntranceType::Normal:
            {
                InSet[V.ToSceneID].insert(Key);
                OutSet[V.FromSceneID].insert(Key);
                break;
            }
            case EntranceType::One_Way_In:
            {
                InSet[V.ToSceneID].insert(Key);
                break;
            }
            case EntranceType::One_Way_Out:
            {
                // Warp songs / owl statues are modelled as a pair of one-way entries:
                //   - the IN side (Type::One_Way_In, ToSceneID = the destination scene)
                //   - the OUT "choice" side (Type::One_Way_Out, ToSceneID = OOT_SONGS / MM_OWLS),
                //     which represents the player playing the song from the current scene.
                // The OUT "choice" side doesn't model a real intra-scene destination (you don't
                // walk TO a warp song to leave through it; it's a song trigger). Skip it so it
                // doesn't pollute the To column of the cost table.
                if (Game == OOT_GAME && V.ToSceneID == OOT_SONGS) break;
                if (Game == MM_GAME && V.ToSceneID == MM_OWLS) break;

                // For one-way-out the player is in ToSceneID when triggering the OUT
                // (per the validation in ParseOutgoingMessage: SceneID must match ToSceneID).
                OutSet[V.ToSceneID].insert(Key);
                break;
            }
            case EntranceType::None:
            default:
            {
                break;
            }
        }
    }

    // Union of every scene that has at least one IN or OUT entrance.
    std::set<uint32_t> AllScenes;
    for (const auto& P : InSet)  AllScenes.insert(P.first);
    for (const auto& P : OutSet) AllScenes.insert(P.first);

    const QString GameName = (Game == OOT_GAME) ? "OoT" : "MM";

    for (uint32_t SceneID : AllScenes)
    {
        const char* SceneNameRaw = GetSceneName(Game, SceneID);
        const QString SceneName = SceneNameRaw != nullptr
            ? QString::fromUtf8(SceneNameRaw)
            : QString("Scene 0x%1").arg(SceneID, 0, 16).toUpper();

        const std::set<uint32_t>& Ins = InSet[SceneID];     // empty when scene has no INs
        const std::set<uint32_t>& Outs = OutSet[SceneID];   // empty when scene has no OUTs

        for (uint32_t From : Ins)
        {
            // Just "the other side" of the IN entrance: the name of the scene we came FROM.
            const QString FromName = OtherSideName(Game, From, SceneID);

            for (uint32_t To : Outs)
            {
                if (From == To) continue;

                EntranceCostRow Row;
                Row.Game = (uint8_t)Game;
                Row.SceneID = SceneID;
                Row.FromEntranceID = From;
                Row.ToEntranceID = To;
                Row.ElapsedSec = -1.0;
                Row.GameName = GameName;
                Row.SceneName = SceneName;
                Row.FromName = FromName;
                // Just "the other side" of the OUT entrance: the name of the scene we go TO.
                Row.ToName = OtherSideName(Game, To, SceneID);

                this->Rows.push_back(Row);
            }
        }
    }
}


namespace
{
    /*
    *   Parse an unsigned integer that may be written with or without an "0x" / "0X" hex prefix.
    *   Qt's QString::toUInt(&ok, 0) auto-detection has been unreliable across versions
    *   (sometimes interprets the leading "0" of "0x55" as octal and fails). Doing the prefix
    *   detection ourselves avoids that whole class of bugs.
    *
    *   @param Str    The field string to parse (may have leading/trailing whitespace).
    *   @param Ok     Set to true on success, false on failure.
    *
    *   @return The parsed value, or 0 if Ok is false.
    */
    uint32_t ParseUIntFlexible(const QString& Str, bool* Ok)
    {
        const QString S = Str.trimmed();
        if (S.isEmpty())
        {
            if (Ok != nullptr) *Ok = false;
            return 0;
        }
        if (S.startsWith("0x", Qt::CaseInsensitive))
        {
            return S.mid(2).toUInt(Ok, 16);
        }
        return S.toUInt(Ok, 10);
    }
}


void EntranceCostModel::LoadCsv(const QString& Path)
{
    // Reset every measured time. LoadCsv is re-callable (file-system watcher triggers it on
    // every CSV append), so without this reset a row whose time was removed from the CSV by
    // hand would stay green forever.
    for (EntranceCostRow& Row : this->Rows)
    {
        Row.ElapsedSec = -1.0;
    }

    QFile File(Path);
    QFileInfo Info(Path);
    if (!File.open(QIODevice::ReadOnly | QIODevice::Text))
    {   // CSV not present yet - leave every ElapsedSec at -1.
        MultiLogger::LogMessage("[CostTab] CSV not found at \"%s\" (resolved: \"%s\") - leaving every row empty.",
            Path.toUtf8().constData(),
            Info.absoluteFilePath().toUtf8().constData());
        this->RebuildRowStatusColors();
        if (!this->Rows.empty())
        {
            emit dataChanged(this->index(0, 0), this->index((int)this->Rows.size() - 1, this->columnCount() - 1));
            emit headerDataChanged(Qt::Vertical, 0, (int)this->Rows.size() - 1);
        }
        return;
    }

    MultiLogger::LogMessage("[CostTab] Loading CSV from \"%s\".", Info.absoluteFilePath().toUtf8().constData());

    // Build a fast lookup of every CSV row keyed by (Game, Scene, From, To). When the same key
    // appears in several rows (the writer always appends, so duplicates are expected after a few
    // measurement runs), keep the FASTEST time - the user's intent is to surface the best run for
    // each entrance pair rather than the latest or the average.
    std::map<std::tuple<uint8_t, uint32_t, uint32_t, uint32_t>, double> Times;

    int LineNo = 0;
    int Parsed = 0;
    int Skipped = 0;

    QTextStream Stream(&File);
    bool FirstLine = true;
    while (!Stream.atEnd())
    {
        ++LineNo;
        const QString Line = Stream.readLine().trimmed();
        if (Line.isEmpty()) continue;
        if (FirstLine)
        {
            FirstLine = false;
            if (Line.startsWith("Game", Qt::CaseInsensitive))
            {
                continue;       // skip header
            }
        }

        const QStringList Fields = Line.split(';');
        if (Fields.size() < 7)
        {
            ++Skipped;
            continue;       // Game;SceneID;FromEntranceID;From;ToEntranceID;To;ElapsedSec
        }

        bool Ok = false;
        const uint8_t Game = (uint8_t)Fields[0].trimmed().toInt(&Ok);
        if (!Ok) { ++Skipped; continue; }
        const uint32_t SceneID = ParseUIntFlexible(Fields[1], &Ok);
        if (!Ok) { ++Skipped; continue; }
        const uint32_t From = ParseUIntFlexible(Fields[2], &Ok);
        if (!Ok) { ++Skipped; continue; }
        const uint32_t To = ParseUIntFlexible(Fields[4], &Ok);
        if (!Ok) { ++Skipped; continue; }
        const double Elapsed = Fields[6].trimmed().toDouble(&Ok);
        if (!Ok || Elapsed < 0.0) { ++Skipped; continue; }

        ++Parsed;
        const auto Key = std::make_tuple(Game, SceneID, From, To);
        auto It = Times.find(Key);
        if (It == Times.end() || Elapsed < It->second)
        {
            Times[Key] = Elapsed;
        }
    }

    int Matched = 0;
    for (EntranceCostRow& Row : this->Rows)
    {
        auto It = Times.find(std::make_tuple(Row.Game, Row.SceneID, Row.FromEntranceID, Row.ToEntranceID));
        if (It != Times.end())
        {
            Row.ElapsedSec = It->second;
            ++Matched;
        }
    }

    MultiLogger::LogMessage("[CostTab] CSV: %d data rows parsed, %d malformed/skipped, %d unique (Game,Scene,From,To) keys, %d matched to a table row (out of %d table rows).",
        Parsed, Skipped, (int)Times.size(), Matched, (int)this->Rows.size());

    // Now that the elapsed times are merged in, refresh the status stripe colors so the rows
    // with known measurements light up green and the unknown ones stay red.
    this->RebuildRowStatusColors();

    if (!this->Rows.empty())
    {
        emit dataChanged(this->index(0, 0), this->index((int)this->Rows.size() - 1, this->columnCount() - 1));
        emit headerDataChanged(Qt::Vertical, 0, (int)this->Rows.size() - 1);
    }
}


int EntranceCostModel::rowCount(const QModelIndex&) const
{
    return (int)this->Rows.size();
}


int EntranceCostModel::columnCount(const QModelIndex&) const
{
    return 5;       // Game, Scene, From, To, Elapsed
}


QVariant EntranceCostModel::data(const QModelIndex& Index, int Role) const
{
    if (!Index.isValid() || Index.row() < 0 || Index.row() >= (int)this->Rows.size())
    {
        return QVariant();
    }

    const EntranceCostRow& Row = this->Rows[Index.row()];

    if (Role == Qt::DisplayRole)
    {
        switch (Index.column())
        {
            case 0: return Row.GameName;
            case 1: return Row.SceneName;
            case 2: return Row.FromName;
            case 3: return Row.ToName;
            case 4: return (Row.ElapsedSec < 0.0) ? QString() : QString::number(Row.ElapsedSec, 'f', 2);
            default: return QVariant();
        }
    }
    else if (Role == Qt::TextAlignmentRole)
    {
        if (Index.column() == 4)
        {
            return (int)(Qt::AlignRight | Qt::AlignVCenter);
        }
    }
    else if (Role == Qt::BackgroundRole)
    {
        if (Index.row() < (int)this->RowColors.size())
        {
            return this->RowColors[Index.row()];
        }
    }

    return QVariant();
}


QVariant EntranceCostModel::headerData(int Section, Qt::Orientation Orientation, int Role) const
{
    if (Orientation == Qt::Horizontal && Role == Qt::DisplayRole)
    {
        switch (Section)
        {
            case 0: return "Game";
            case 1: return "Scene";
            case 2: return "From";
            case 3: return "To";
            case 4: return "Elapsed (sec)";
        }
    }
    else if (Orientation == Qt::Vertical && Role == Qt::BackgroundRole)
    {   // Per-row status stripe consumed by CostStatusHeaderView.
        if (Section >= 0 && Section < (int)this->RowStatusColors.size())
        {
            return this->RowStatusColors[Section];
        }
    }
    return QVariant();
}


void EntranceCostModel::RebuildRowColors()
{
    this->RowColors.clear();
    this->RowColors.reserve(this->Rows.size());

    bool Toggle = false;
    uint32_t PrevSceneID = UINT32_MAX;
    uint8_t PrevGame = NO_GAME;
    for (const EntranceCostRow& Row : this->Rows)
    {
        if (Row.Game != PrevGame || Row.SceneID != PrevSceneID)
        {
            Toggle = !Toggle;
            PrevSceneID = Row.SceneID;
            PrevGame = Row.Game;
        }
        this->RowColors.push_back(Toggle ? QColor(45, 45, 45) : QColor(60, 60, 60));
    }
}


void EntranceCostModel::RebuildRowStatusColors()
{
    this->RowStatusColors.clear();
    this->RowStatusColors.reserve(this->Rows.size());
    for (const EntranceCostRow& Row : this->Rows)
    {
        if (Row.ElapsedSec < 0.0)
        {
            this->RowStatusColors.push_back(QColor(200, 60, 60));       // red - missing measurement
        }
        else
        {
            this->RowStatusColors.push_back(QColor(60, 180, 80));       // green - known measurement
        }
    }
}


void EntranceCostModel::OnEntranceCostMeasured(int Game, uint32_t SceneID, uint32_t FromEntranceID, uint32_t ToEntranceID, double ElapsedSec)
{
    if (ElapsedSec < 0.0) return;

    // Linear scan - rows aren't keyed, but ~5k entries take microseconds and this is called once
    // per traversal so the simplicity wins over a side index.
    for (size_t i = 0; i < this->Rows.size(); ++i)
    {
        EntranceCostRow& Row = this->Rows[i];
        if ((int)Row.Game != Game) continue;
        if (Row.SceneID != SceneID) continue;
        if (Row.FromEntranceID != FromEntranceID) continue;
        if (Row.ToEntranceID != ToEntranceID) continue;

        // Same "keep fastest" rule as the CSV loader.
        if (Row.ElapsedSec >= 0.0 && Row.ElapsedSec <= ElapsedSec)
        {
            return;
        }
        Row.ElapsedSec = ElapsedSec;
        if (i < this->RowStatusColors.size())
        {
            this->RowStatusColors[i] = QColor(60, 180, 80);     // green - measurement just landed
        }
        const QModelIndex Top = this->index((int)i, 0);
        const QModelIndex Bottom = this->index((int)i, this->columnCount() - 1);
        emit dataChanged(Top, Bottom);
        emit headerDataChanged(Qt::Vertical, (int)i, (int)i);
        return;
    }
}

#pragma endregion // EntranceCostModel


#pragma region // EntranceCostFilterProxy

EntranceCostFilterProxy::EntranceCostFilterProxy(QObject* Parent) : QSortFilterProxyModel(Parent)
{
}


void EntranceCostFilterProxy::SetHideKnown(bool Hide)
{
    if (this->HideKnown == Hide) return;
    this->HideKnown = Hide;
    this->invalidateFilter();
}


bool EntranceCostFilterProxy::filterAcceptsRow(int SourceRow, const QModelIndex& SourceParent) const
{
    // Defer to the inherited text-search filter first - keeps the user's typed query working.
    if (!QSortFilterProxyModel::filterAcceptsRow(SourceRow, SourceParent))
    {
        return false;
    }
    if (!this->HideKnown)
    {
        return true;
    }
    // Hide rows that already have a measured time (ElapsedSec >= 0).
    const EntranceCostModel* Src = qobject_cast<const EntranceCostModel*>(this->sourceModel());
    if (Src == nullptr || SourceRow < 0 || SourceRow >= (int)Src->Rows.size())
    {
        return true;
    }
    return Src->Rows[SourceRow].ElapsedSec < 0.0;
}

#pragma endregion // EntranceCostFilterProxy


#pragma region // EntranceCostTab

EntranceCostTab::EntranceCostTab(EntranceTab* Parent) : QWidget(Parent)
{
    this->MainLayout = new QVBoxLayout(this);
    this->MainLayout->setContentsMargins(8, 8, 8, 8);
    this->MainLayout->setSpacing(6);

    // Rules block: friendly intro to community contributors + the protocol they should
    // follow when measuring travel times so the resulting CSV stays comparable.
    this->RulesLabel = new QLabel(this);
    this->RulesLabel->setText(
        "<p>If you are here, you are interested in contributing to the tracker. "
        "You will find here all your travel times between every entrance. "
        "This is needed to make the GPS as accurate as possible.</p>"
        "<p>The tracker will automatically start a timer when you spawn in a scene through the "
        "matching entrance, and stop it when you leave that same scene. The results are saved in a "
        "CSV file that you can then share with me on GitHub: "
        "<a href=\"https://github.com/Loupimo/OoTMMCombo-Tracker/discussions/1\">"
        "https://github.com/Loupimo/OoTMMCombo-Tracker/discussions/1</a> to help improve the tracker.</p>"
        "<p><b>Huge thanks to everyone who contributes!</b></p>"
        "<p>To be as accurate as possible, please try to follow these rules:</p>"
        "<ol>"
        "<li>Be as vanilla as possible. If a specific trick or setting is needed, please document it.</li>"
        "<li>No cheats (e.g. <i>L to Levitate</i>).</li>"
        "<li>No emulator speedup.</li>"
        "<li>No entrance randomizer.</li>"
        "<li>No Bunny Hood.</li>"
        "<li>No Goron mask.</li>"
        "<li>No Epona travel (unless specifically required for that route).</li>"
        "<li>Don't waste your time with Wallmasters - since they are not randomized, the timer will not stop.</li>"
        "</ol>");
    this->RulesLabel->setTextFormat(Qt::RichText);
    this->RulesLabel->setWordWrap(true);
    this->RulesLabel->setOpenExternalLinks(true);
    this->RulesLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    this->RulesLabel->setStyleSheet("QLabel { padding: 8px; background: #222; border: 1px solid #444; border-radius: 4px; }");

    this->SearchBar = new QLineEdit(this);
    this->SearchBar->setPlaceholderText("Search...");

    // Toggle button to hide the rows that already have a measured time. Sits next to the
    // search bar on the same row so the user can narrow down what is still missing quickly.
    this->HideMeasuredButton = new QPushButton("Hide measured", this);
    this->HideMeasuredButton->setCheckable(true);
    this->HideMeasuredButton->setToolTip("Hide rows that already have an elapsed time so only "
                                          "the pairs that still need a measurement remain.");

    this->Model = new EntranceCostModel(this);

    this->Proxy = new EntranceCostFilterProxy(this);
    this->Proxy->setSourceModel(this->Model);
    this->Proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    this->Proxy->setFilterKeyColumn(-1);

    this->Table = new QTableView(this);
    this->Table->setModel(this->Proxy);
    this->Table->setSortingEnabled(true);
    this->Table->setAlternatingRowColors(false);
    this->Table->setShowGrid(false);
    this->Table->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->Table->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    // Enable hover so the table picks up the same visual feedback as the entrance tables.
    this->Table->setMouseTracking(true);
    this->Table->viewport()->setMouseTracking(true);
    this->Table->viewport()->setAttribute(Qt::WA_Hover, true);

    // Replace the default vertical header with a thin colored stripe driven by the model's
    // BackgroundRole, identical in spirit to AllEntranceView's StatusHeaderView. Red rows are
    // missing measurements, green rows are already filled.
    CostStatusHeaderView* statusHeader = new CostStatusHeaderView(this->Table);
    this->Table->setVerticalHeader(statusHeader);
    statusHeader->setVisible(true);
    statusHeader->setSectionResizeMode(QHeaderView::Fixed);
    statusHeader->setDefaultSectionSize(20);
    statusHeader->setSectionsClickable(false);
    statusHeader->setFixedWidth(6);

    // Thin row separators (drawn by the delegate) to break the uniform gray slab without enabling
    // the heavy full grid.
    this->Table->setItemDelegate(new CostRowSeparatorDelegate(this->Table));

    this->Table->horizontalHeader()->setSortIndicatorShown(true);
    this->Table->horizontalHeader()->setSortIndicator(1, Qt::AscendingOrder);
    this->Table->horizontalHeader()->setStretchLastSection(false);
    this->Table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    this->Table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    this->Table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    this->Table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    this->Table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);

    connect(this->SearchBar, &QLineEdit::textChanged, this->Proxy, &QSortFilterProxyModel::setFilterFixedString);
    connect(this->HideMeasuredButton, &QPushButton::toggled, this->Proxy, &EntranceCostFilterProxy::SetHideKnown);

    // Search bar + button on a single row so the controls stay compact above the table.
    QHBoxLayout* SearchRow = new QHBoxLayout();
    SearchRow->setContentsMargins(0, 0, 0, 0);
    SearchRow->setSpacing(6);
    SearchRow->addWidget(this->SearchBar, 1);       // search bar takes all remaining space
    SearchRow->addWidget(this->HideMeasuredButton, 0);

    this->MainLayout->addWidget(this->RulesLabel);
    this->MainLayout->addLayout(SearchRow);
    this->MainLayout->addWidget(this->Table);
    this->setLayout(this->MainLayout);
}

#pragma endregion // EntranceCostTab
