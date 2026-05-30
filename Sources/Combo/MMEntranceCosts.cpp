#include "Combo/Entrances.h"
#include "Combo/MMEntrances.h"

/*
*   Measured intra-scene travel times for MM, imported from entrance_costs.csv
*   by Pool Transform.py (parse_entrance_costs). Edit the CSV and re-run the
*   script - do not hand-edit this file.
*/
static const EntranceCostMeasurement MMMeasuredCosts[1] =
{
};


void InitializeMMMeasuredCosts(std::map<int, EntranceMetaInfo>& Map)
{
    for (const EntranceCostMeasurement& M : MMMeasuredCosts)
    {
        // Resolve the OUT key into the actual physical walk target: the OUT entry's
        // FromEntranceID is the spawn point in M.Scene that triggers the portal. Without
        // this translation, M.To might point at the destination scene's spawn instead, and
        // the cost would never reach the right intra-scene walking edge.
        auto ToIt = Map.find((int)M.To);
        if (ToIt == Map.end()) continue;
        const uint32_t WalkTarget = ToIt->second.FromEntranceID;

        // The Cost table for "walks starting at M.From in M.Scene" lives on the entrance
        // whose FromEntranceID == M.From AND FromSceneID == M.Scene. Within a single game
        // and scene the pair is unique, so one match is enough.
        for (auto& Pair : Map)
        {
            EntranceMetaInfo& V = Pair.second;
            if (V.FromEntranceID != M.From) continue;
            if (V.FromSceneID != M.Scene) continue;
            V.Cost.Costs[WalkTarget] = M.CostSeconds;
            break;
        }
    }
}
