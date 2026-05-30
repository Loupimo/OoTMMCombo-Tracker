#pragma once

#include <QString>
#include <QVector>
#include <cstdint>


#pragma region // Result types

/*
*   Status returned by the GPS pathfinder. Only Ok carries a meaningful set of
*   routes; the other two statuses describe pre-check failures detected before
*   running Dijkstra, so the UI can display a localized error message instead.
*/
enum GPSPathfindStatus
{
    GPS_Ok,
    GPS_SameScene,                  // From and To point to the same scene; nothing to compute.
    GPS_CannotLeaveStart,           // No entrance of the start scene leads to a different scene.
    GPS_DestinationUnreachable,     // No entrance anywhere leads into the destination scene.
    GPS_CrossGameUnsupported,       // From and To live in different games; the static graph does not model that yet.
    GPS_NoPath,                     // Pre-checks passed but Dijkstra found no path.
};


/*
*   A single hop of a computed GPS path: the scene the player is in at that
*   point, the entrance used to reach the next scene, and the cost of that hop.
*   The last step of a route has SceneID set to the destination scene; ViaText
*   and Cost are unused in that final step.
*/
typedef struct GPSPathStep
{
    uint32_t    SceneID = 0;
    uint8_t     Game = 0;           // The game this scene belongs to (OOT_GAME or MM_GAME). Needed to disambiguate cross-game routes where OoT and MM scene IDs can numerically collide.
    QString     SceneName;
    QString     ViaText;            // Short label for the transition leaving this scene (e.g. "Walk").
    uint32_t    Cost = 0;           // Cost of this hop (intra + portal). 0 on the last step.
} GPSPathStep;


/*
*   A full computed GPS path: the ordered sequence of scenes visited and the
*   total accumulated cost.
*/
typedef struct GPSPath
{
    QVector<GPSPathStep>    Steps;
    uint32_t                TotalCost = 0;
} GPSPath;


/*
*   The full result of a GPS pathfinder call: a status code and, when Status ==
*   GPS_Ok, up to 3 routes ordered from cheapest to most expensive.
*/
typedef struct GPSPathfindResult
{
    GPSPathfindStatus       Status = GPS_NoPath;
    QVector<GPSPath>        Routes;
} GPSPathfindResult;

#pragma endregion


#pragma region // API

/*
*   Run the GPS pathfinder from (FromGame, FromScene) to (ToGame, ToScene). Builds
*   an entrance-level graph from the OoT / MM entrance metadata, runs the pre-checks
*   required by the spec (cannot leave start area, destination unreachable), then
*   returns up to MaxRoutes shortest paths via Yen's K-shortest paths algorithm.
*
*   @param FromGame     The starting scene's game (OOT_GAME or MM_GAME).
*   @param FromScene    The starting scene ID.
*   @param ToGame       The destination scene's game (OOT_GAME or MM_GAME).
*   @param ToScene      The destination scene ID.
*   @param MaxRoutes    The maximum number of distinct shortest paths to return (typically 3).
*
*   @return A GPSPathfindResult whose Status describes the outcome and whose Routes
*           holds the computed paths sorted from cheapest to most expensive (empty
*           on any non-Ok status).
*/
GPSPathfindResult FindGPSRoutes(int FromGame, uint32_t FromScene,
                                int ToGame,   uint32_t ToScene,
                                int MaxRoutes = 3);

#pragma endregion
