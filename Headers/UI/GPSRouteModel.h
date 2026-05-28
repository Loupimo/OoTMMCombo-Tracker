#pragma once

#include <QString>
#include <QChar>
#include <QColor>
#include <QVector>


#pragma region // Enums

/*
*   The kind of transition used to move from one station to the next on a GPS route.
*   Walk is on-foot travel, Owl is a fast-travel statue, Song is a warp song,
*   Warp is any other instant relocation (Farore's Wind, etc.).
*/
enum TransitionVia
{
    Via_Walk,
    Via_Owl,
    Via_Song,
    Via_Warp,
};

#pragma endregion


#pragma region // Structs

/*
*   A single step on a GPS route: the station to reach and how we got there from
*   the previous one. The first step's Via / DurationSec describe the leg leaving
*   that station toward the next one; on the last step they are ignored.
*/
typedef struct RouteStep
{
    QString         StationName;
    TransitionVia   Via = Via_Walk;
    int             DurationSec = 0;
    QString         ViaCustom;          // When non-empty, overrides ViaLabel(Via) in the card UI - used to show the name of the exit door to take.
} RouteStep;


/*
*   A full GPS route from a starting station to a destination. Rank / Label /
*   Notes are display strings; Accent is the per-route color used by GPSRouteCard
*   for the vertical line and intermediate circles; IsBest marks the card that
*   should receive the highlighted border in the UI.
*/
typedef struct Route
{
    QString             Rank;
    QString             TotalTime;
    QString             Label;
    QString             Notes;
    QColor              Accent;
    QVector<RouteStep>  Steps;
    bool                IsBest = false;
} Route;

#pragma endregion


#pragma region // Helpers

/*
*   Build the short human label of the given transition kind.
*
*   @param Via    The transition kind to format.
*
*   @return The display label ("Walk" / "Owl" / "Song" / "Warp").
*/
const char* ViaLabel(TransitionVia Via);


/*
*   Build the single glyph used to decorate the "via" line of a station row.
*
*   @param Via    The transition kind to format.
*
*   @return The display glyph (arrow, owl, music note, ...).
*/
QChar ViaGlyph(TransitionVia Via);


/*
*   Build the placeholder set of 3 routes used while the real pathfinder is not
*   wired yet. The intermediate stations are hardcoded; only the From / To names
*   reflect the current combo selection.
*
*   @param From    The starting station name.
*   @param To      The destination station name.
*
*   @return Three demo routes sorted from fastest to slowest. The first one has IsBest = true.
*/
QVector<Route> SampleRoutes(const QString& From, const QString& To);

#pragma endregion
