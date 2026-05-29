#pragma once

#include <QWidget>
#include <QTableView>
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QColor>
#include <QString>

#include <cstdint>
#include <vector>


/*
*   One row in the EntranceCostModel: the travel time from an arrival entrance
*   (FromEntranceID) to an exit entrance (ToEntranceID) inside a given scene.
*   ElapsedSec is < 0 when the time is unknown (not yet measured / not yet
*   contributed by the community).
*/
typedef struct EntranceCostRow
{
    uint8_t Game = 0;                       // OOT_GAME or MM_GAME.
    uint32_t SceneID = 0;                   // The scene the From and To entrances physically live in.
    uint32_t FromEntranceID = 0;            // The arrival entrance.
    uint32_t ToEntranceID = 0;              // The exit entrance.
    double BestSec = -1.0;                  // Fastest measured time so far in seconds, or < 0 when unknown.
    double LastSec = -1.0;                  // Most recent measured time in seconds, or < 0 when unknown.

    QString GameName;                       // Pre-formatted "OoT" or "MM".
    QString SceneName;                      // Pre-formatted scene name.
    QString FromName;                       // Pre-formatted arrival entrance (Leads format).
    QString ToName;                         // Pre-formatted exit entrance ("FromName -> ToName" format).
} EntranceCostRow;


class EntranceTab;


/*
*   QSortFilterProxyModel that adds an optional "hide rows whose ElapsedSec is set" filter on top
*   of the inherited text-search filter. Used by the cost tab so the user can focus on the rows
*   that still need a measurement.
*/
class EntranceCostFilterProxy : public QSortFilterProxyModel
{
    Q_OBJECT

public:

    /*
    *   Construct the proxy with the hide-known filter disabled.
    *
    *   @param Parent    The Qt parent.
    */
    explicit EntranceCostFilterProxy(QObject* Parent = nullptr);

    /*
    *   Toggle the "hide rows that already have a measured time" filter. No-op when the flag is
    *   already in the requested state.
    *
    *   @param Hide    True to hide rows whose ElapsedSec is >= 0, false to show every row.
    */
    void SetHideKnown(bool Hide);

protected:

    /*
    *   Combine the inherited text-search filter with the optional hide-known filter.
    *
    *   @param SourceRow       The row index in the source model.
    *   @param SourceParent    The parent index in the source model.
    *
    *   @return True when the row should remain visible.
    */
    bool filterAcceptsRow(int SourceRow, const QModelIndex& SourceParent) const override;

    /*
    *   Sort the numeric Best / Last columns by their underlying double value rather than the
    *   alphabetical order of their formatted DisplayRole strings. Unmeasured rows (value < 0)
    *   are pushed to the end (ascending) / start (descending) so they don't interleave with
    *   measured times. Other columns fall back to the default lexicographic comparison.
    *
    *   @param Left     The left-hand operand of the comparison.
    *   @param Right    The right-hand operand of the comparison.
    *
    *   @return True when Left should sort before Right.
    */
    bool lessThan(const QModelIndex& Left, const QModelIndex& Right) const override;

private:

    bool HideKnown = false;
};


/*
*   Read-only model exposing every possible intra-scene (From -> To) entrance
*   pair across OoT and MM. Times are populated from the entrance_costs.csv
*   file when present, otherwise the ElapsedSec column is left empty.
*/
class EntranceCostModel : public QAbstractTableModel
{
    Q_OBJECT

public:

    std::vector<EntranceCostRow> Rows;              // The exhaustive list of (Game, Scene, From, To) cells.
    std::vector<QColor> RowColors;                  // Alternating row background colors, grouped by Scene.
    std::vector<QColor> RowStatusColors;            // Status stripe color per row (green = known, red = unknown).

    /*
    *   Constructs the model. Enumerates every intra-scene entrance pair and
    *   tries to load the entrance_costs.csv file if present.
    *
    *   @param Parent    The Qt parent.
    */
    explicit EntranceCostModel(QObject* Parent = nullptr);

    /*
    *   Build the row list from OoTEntrances and MMEntrances. Clears the
    *   previous content first.
    */
    void BuildRows();

    /*
    *   Try to open the given CSV file and populate the ElapsedSec field of
    *   every matching row. Silently does nothing if the file cannot be opened.
    *
    *   @param Path    The CSV file path.
    */
    void LoadCsv(const QString& Path);

    /*
    *   Get the number of rows in the model.
    *
    *   @param Parent    Unused (Qt override).
    *
    *   @return The total number of (Game, Scene, From, To) cells.
    */
    int rowCount(const QModelIndex& Parent = QModelIndex()) const override;

    /*
    *   Get the number of columns in the model.
    *
    *   @param Parent    Unused (Qt override).
    *
    *   @return Always 5 (Game, Scene, From, To, Elapsed).
    */
    int columnCount(const QModelIndex& Parent = QModelIndex()) const override;

    /*
    *   Return the data displayed or styled for the given cell index.
    *
    *   @param Index    The cell index.
    *   @param Role     The Qt role describing what is requested.
    *
    *   @return The cell text for DisplayRole, the background for BackgroundRole, alignment for
    *           TextAlignmentRole, or an empty QVariant.
    */
    QVariant data(const QModelIndex& Index, int Role = Qt::DisplayRole) const override;

    /*
    *   Return the header text for the given section.
    *
    *   @param Section        The header section index.
    *   @param Orientation    Horizontal or vertical.
    *   @param Role           The Qt role describing what is requested.
    *
    *   @return The header label or an empty QVariant.
    */
    QVariant headerData(int Section, Qt::Orientation Orientation, int Role) const override;

private:

    /*
    *   Append rows for the given game by computing the cross-product of all
    *   entrances physically located in each scene.
    *
    *   @param Game    OOT_GAME or MM_GAME.
    */
    void BuildRowsForGame(int Game);

    /*
    *   Recompute the alternating background colors grouped by (Game, Scene).
    */
    void RebuildRowColors();

    /*
    *   Recompute the per-row status stripe color (green when ElapsedSec is known, red when not).
    *   Must be called after BuildRows and after LoadCsv so the colors reflect the loaded data.
    */
    void RebuildRowStatusColors();

public slots:

    /*
    *   Live update from the TIMER_COST writer. Looks up the matching (Game, Scene, From, To) row
    *   and updates its ElapsedSec in place using the "keep fastest" rule, then emits dataChanged
    *   for only that row so the view repaints just that line. No CSV re-read needed.
    *
    *   @param Game             OOT_GAME or MM_GAME.
    *   @param SceneID          The scene the trip took place in.
    *   @param FromEntranceID   The arrival entrance.
    *   @param ToEntranceID     The exit entrance.
    *   @param ElapsedSec       The measured travel time in seconds.
    */
    void OnEntranceCostMeasured(int Game, uint32_t SceneID, uint32_t FromEntranceID, uint32_t ToEntranceID, double ElapsedSec);
};


/*
*   The "Costs" sub-tab inside the EntranceTab. Shows the rules of the
*   measurement protocol followed by an exhaustive table of every possible
*   intra-scene entrance pair so the community can fill in the missing travel
*   times. The table mirrors the AllEntranceView design: search bar above the
*   table, alternating colors per scene, sortable headers.
*/
class EntranceCostTab : public QWidget
{
    Q_OBJECT

public:

    QVBoxLayout* MainLayout = nullptr;
    QLabel* RulesLabel = nullptr;
    QLineEdit* SearchBar = nullptr;
    QPushButton* HideMeasuredButton = nullptr;
    QTableView* Table = nullptr;
    EntranceCostModel* Model = nullptr;
    EntranceCostFilterProxy* Proxy = nullptr;

    /*
    *   Construct the cost tab as a child of the given entrance tab.
    *
    *   @param Parent    The owning entrance tab.
    */
    explicit EntranceCostTab(EntranceTab* Parent = nullptr);
};
