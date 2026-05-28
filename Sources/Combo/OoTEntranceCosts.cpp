#include "Combo/Entrances.h"
#include "Combo/OoTEntrances.h"

/*
*   Measured intra-scene travel times for OoT, imported from entrance_costs.csv
*   by Pool Transform.py (parse_entrance_costs). Edit the CSV and re-run the
*   script - do not hand-edit this file.
*/
static const EntranceCostMeasurement OoTMeasuredCosts[] =
{
    { 0x026, 0x0c9, 0x026a,   12 },
    { 0x027, 0x09c, 0x033c,    4 },
    { 0x028, 0x433, 0x0443,    6 },
    { 0x034, 0x272, 0x0211,    2 },
    { 0x055, 0x211, 0x0000,   40 },
    { 0x055, 0x211, 0x00c1,   18 },
    { 0x055, 0x211, 0x00c9,   17 },
    { 0x055, 0x211, 0x011e,   32 },
    { 0x055, 0x211, 0x0272,    2 },
    { 0x055, 0x211, 0x0433,   15 },
    { 0x055, 0x211, 0x0437,   11 },
    { 0x055, 0x211, 0x05e0,   19 },
    { 0x055, 0x211, 0x1000,   43 },
    { 0x055, 0x286, 0x0000,   38 },
    { 0x055, 0x286, 0x009c,   18 },
    { 0x055, 0x286, 0x00c1,   12 },
    { 0x055, 0x286, 0x00c9,   16 },
    { 0x055, 0x286, 0x0272,   22 },
    { 0x055, 0x286, 0x0433,   10 },
    { 0x055, 0x286, 0x0437,   16 },
    { 0x055, 0x286, 0x05e0,   12 },
    { 0x055, 0x286, 0x1000,   13 },
    { 0x055, 0x33c, 0x0000,   30 },
    { 0x055, 0x33c, 0x00c1,    8 },
    { 0x055, 0x33c, 0x00c9,   17 },
    { 0x055, 0x33c, 0x011e,   30 },
    { 0x055, 0x33c, 0x0272,   15 },
    { 0x055, 0x33c, 0x0433,   13 },
    { 0x055, 0x33c, 0x0437,    8 },
    { 0x055, 0x33c, 0x05e0,   20 },
    { 0x055, 0x33c, 0x1000,   41 },
    { 0x055, 0x443, 0x00c9,   10 },
    { 0x055, 0x443, 0x05e0,   25 },
    { 0x055, 0x1100, 0x0000,   39 },
    { 0x055, 0x1100, 0x009c,   21 },
    { 0x055, 0x1100, 0x00c1,   15 },
    { 0x055, 0x1100, 0x00c9,   15 },
    { 0x055, 0x1100, 0x011e,    5 },
    { 0x055, 0x1100, 0x0272,   25 },
    { 0x055, 0x1100, 0x0433,   10 },
    { 0x055, 0x1100, 0x0437,   20 },
    { 0x055, 0x1100, 0x05e0,   13 },
    { 0x05b, 0x11e, 0x0286,    8 },
    { 0x065, 0x1000, 0x1100,    4 },
};


void InitializeOoTMeasuredCosts(std::map<int, EntranceMetaInfo>& Map)
{
    for (const EntranceCostMeasurement& M : OoTMeasuredCosts)
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
