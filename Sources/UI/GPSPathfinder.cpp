#include "UI/GPSPathfinder.h"
#include "Combo/Entrances.h"
#include "Combo/OoTEntrances.h"
#include "Combo/MMEntrances.h"
#include "Combo/Scenes.h"
#include "Multi/Game.h"

#include <unordered_map>
#include <unordered_set>
#include <set>
#include <map>
#include <vector>
#include <queue>
#include <utility>
#include <limits>
#include <cstdint>
#include <algorithm>


#pragma region // Internal graph types

namespace
{
    constexpr uint32_t  UnreachableCost = std::numeric_limits<uint32_t>::max();
    constexpr uint32_t  VirtSource = 0xFFFFFFFEu;
    constexpr uint32_t  VirtSink   = 0xFFFFFFFFu;

    struct Edge
    {
        uint32_t    To = 0;
        uint32_t    Cost = 0;
        bool        IsPortal = false;       // True for inter-scene portal edges, false for intra-scene walks.
    };

    struct Graph
    {
        // Out-edges per entrance node.
        std::unordered_map<uint32_t, std::vector<Edge>> Adj;
        // Reverse lookup: which physical scene each entrance lives in.
        std::unordered_map<uint32_t, uint32_t>          EntranceScene;
    };


    /*
    *   Build a directed graph for the given game from the entrance map. Every entry
    *   (FromEntr -> ToEntr) becomes a portal edge of cost 0 (using the portal itself
    *   is instantaneous), and every entrance gets intra-scene edges to all other
    *   entrances of its scene taken from EntranceMetaInfo::Cost (default 1).
    */
    Graph BuildGraph(int Game)
    {
        Graph G;

        std::map<int, EntranceMetaInfo>* Map = nullptr;
        if      (Game == OOT_GAME) Map = &OoTEntrances;
        else if (Game == MM_GAME)  Map = &MMEntrances;
        if (Map == nullptr) return G;

        // First pass: register every entrance with its physical scene and add the
        // portal edge. Skip None-typed entrances (placeholders / cutscene-only).
        for (auto& Pair : *Map)
        {
            const EntranceMetaInfo& V = Pair.second;
            if (V.Type == EntranceType::None) continue;

            G.EntranceScene[V.FromEntranceID] = V.FromSceneID;
            G.EntranceScene[V.ToEntranceID]   = V.ToSceneID;

            G.Adj[V.FromEntranceID].push_back({ V.ToEntranceID, 0u, true });
        }

        // Second pass: add intra-scene walk edges using EntranceMetaInfo::Cost.
        // Each entry's Cost table describes walks from V.FromEntranceID inside
        // V.FromSceneID. Entrance IDs are unique per game so the same edge cannot
        // be added twice — no de-duplication needed.
        for (auto& Pair : *Map)
        {
            const EntranceMetaInfo& V = Pair.second;
            if (V.Type == EntranceType::None) continue;
            for (const auto& KV : V.Cost.Costs)
            {
                if (KV.second == UnreachableCost) continue;
                G.Adj[V.FromEntranceID].push_back({ KV.first, KV.second, false });
            }
        }

        return G;
    }


    /*
    *   Attach virtual source and sink nodes to the graph: VirtSource has a 0-cost
    *   edge to every entrance of the start scene, and every entrance of the end
    *   scene has a 0-cost edge to VirtSink. Returns true if both virtual nodes got
    *   at least one connection (otherwise no path can exist).
    */
    bool AttachVirtualEndpoints(Graph& G, uint32_t StartScene, uint32_t EndScene)
    {
        bool HasStart = false;
        bool HasEnd   = false;

        for (const auto& Pair : G.EntranceScene)
        {
            if (Pair.second == StartScene)
            {
                G.Adj[VirtSource].push_back({ Pair.first, 0u, false });
                HasStart = true;
            }
            if (Pair.second == EndScene)
            {
                G.Adj[Pair.first].push_back({ VirtSink, 0u, false });
                HasEnd = true;
            }
        }

        return HasStart && HasEnd;
    }


    struct DijkstraResult
    {
        std::vector<uint32_t>   Path;       // Node IDs from source to sink (inclusive).
        uint32_t                Cost = 0;
        bool                    Found = false;
    };


    /*
    *   Run a Dijkstra search on the given graph from Source to Sink while ignoring
    *   any (u, v) edge in ForbiddenEdges and any node in ForbiddenNodes (Source is
    *   never forbidden so the search can start from it).
    */
    DijkstraResult RunDijkstra(const Graph& G, uint32_t Source, uint32_t Sink,
                               const std::set<std::pair<uint32_t, uint32_t>>& ForbiddenEdges,
                               const std::unordered_set<uint32_t>& ForbiddenNodes)
    {
        DijkstraResult Result;

        std::unordered_map<uint32_t, uint32_t> Dist;
        std::unordered_map<uint32_t, uint32_t> Prev;
        using NodeCost = std::pair<uint32_t, uint32_t>;     // (cost, node)
        std::priority_queue<NodeCost, std::vector<NodeCost>, std::greater<NodeCost>> Heap;

        Dist[Source] = 0;
        Heap.push({ 0u, Source });

        while (!Heap.empty())
        {
            const auto Top = Heap.top();
            Heap.pop();
            const uint32_t D = Top.first;
            const uint32_t U = Top.second;

            if (U == Sink) break;
            auto ItD = Dist.find(U);
            if (ItD != Dist.end() && D > ItD->second) continue;

            const auto AdjIt = G.Adj.find(U);
            if (AdjIt == G.Adj.end()) continue;

            for (const Edge& E : AdjIt->second)
            {
                if (E.To != Source && ForbiddenNodes.count(E.To)) continue;
                if (ForbiddenEdges.count({ U, E.To })) continue;

                const uint32_t NewCost = D + E.Cost;
                auto ItV = Dist.find(E.To);
                if (ItV == Dist.end() || NewCost < ItV->second)
                {
                    Dist[E.To] = NewCost;
                    Prev[E.To] = U;
                    Heap.push({ NewCost, E.To });
                }
            }
        }

        auto ItSink = Dist.find(Sink);
        if (ItSink == Dist.end()) return Result;

        Result.Cost = ItSink->second;
        // Reconstruct path.
        std::vector<uint32_t> Rev;
        uint32_t Cur = Sink;
        Rev.push_back(Cur);
        while (Cur != Source)
        {
            auto It = Prev.find(Cur);
            if (It == Prev.end()) return Result;     // disconnected; bail
            Cur = It->second;
            Rev.push_back(Cur);
        }
        Result.Path.assign(Rev.rbegin(), Rev.rend());
        Result.Found = true;
        return Result;
    }


    /*
    *   Recompute the cost of a node sequence by walking edges of the original graph.
    *   Skips Source-to-first and last-to-Sink virtual edges if those are present.
    */
    uint32_t PathCost(const Graph& G, const std::vector<uint32_t>& Path)
    {
        uint32_t Total = 0;
        for (size_t i = 0; i + 1 < Path.size(); ++i)
        {
            const uint32_t U = Path[i];
            const uint32_t V = Path[i + 1];
            const auto It = G.Adj.find(U);
            if (It == G.Adj.end()) return UnreachableCost;
            uint32_t Best = UnreachableCost;
            for (const Edge& E : It->second)
                if (E.To == V && E.Cost < Best) Best = E.Cost;
            if (Best == UnreachableCost) return UnreachableCost;
            Total += Best;
        }
        return Total;
    }


    /*
    *   Yen's K-shortest loopless paths algorithm. Returns up to K paths sorted by
    *   total cost ascending. Each path is the full node sequence including Source
    *   and Sink.
    */
    std::vector<std::vector<uint32_t>> YenKShortestPaths(const Graph& G,
                                                        uint32_t Source, uint32_t Sink,
                                                        int K)
    {
        std::vector<std::vector<uint32_t>> A;
        if (K <= 0) return A;

        DijkstraResult First = RunDijkstra(G, Source, Sink, {}, {});
        if (!First.Found) return A;
        A.push_back(First.Path);

        // Candidate paths heap, keyed on total cost. Vector + sort-on-pop is simpler
        // than maintaining a heap with custom comparator for vector<uint32_t> values.
        std::vector<std::pair<uint32_t, std::vector<uint32_t>>> B;
        std::set<std::vector<uint32_t>> Seen;
        Seen.insert(First.Path);

        for (int k = 1; k < K; ++k)
        {
            const std::vector<uint32_t>& Prev = A.back();

            for (size_t i = 0; i + 1 < Prev.size(); ++i)
            {
                const uint32_t SpurNode = Prev[i];
                const std::vector<uint32_t> RootPath(Prev.begin(), Prev.begin() + i + 1);

                std::set<std::pair<uint32_t, uint32_t>> ForbiddenEdges;
                std::unordered_set<uint32_t> ForbiddenNodes;

                // Remove edges that would re-derive the same RootPath -> next node
                // as one of the paths already accepted into A.
                for (const auto& P : A)
                {
                    if (P.size() <= i + 1) continue;
                    bool SameRoot = true;
                    for (size_t j = 0; j <= i; ++j)
                        if (P[j] != RootPath[j]) { SameRoot = false; break; }
                    if (SameRoot) ForbiddenEdges.insert({ P[i], P[i + 1] });
                }
                // Forbid all root-path nodes except the spur node so the spur path stays loopless.
                for (size_t j = 0; j + 1 < RootPath.size(); ++j)
                    ForbiddenNodes.insert(RootPath[j]);

                DijkstraResult Spur = RunDijkstra(G, SpurNode, Sink, ForbiddenEdges, ForbiddenNodes);
                if (!Spur.Found) continue;

                std::vector<uint32_t> Total = RootPath;
                Total.pop_back();
                for (uint32_t N : Spur.Path) Total.push_back(N);

                if (Seen.count(Total)) continue;
                const uint32_t TC = PathCost(G, Total);
                if (TC == UnreachableCost) continue;
                B.push_back({ TC, Total });
                Seen.insert(Total);
            }

            if (B.empty()) break;
            auto BestIt = std::min_element(B.begin(), B.end(),
                [](const std::pair<uint32_t, std::vector<uint32_t>>& a,
                   const std::pair<uint32_t, std::vector<uint32_t>>& b)
                { return a.first < b.first; });
            A.push_back(BestIt->second);
            B.erase(BestIt);
        }

        return A;
    }


    /*
    *   Convert a raw node sequence (entrance IDs, possibly bracketed by VirtSource
    *   and VirtSink) into a GPSPath of scene stations, collapsing consecutive nodes
    *   that live in the same scene into a single station.
    */
    GPSPath ToScenePath(const Graph& G, const std::vector<uint32_t>& Raw, int Game)
    {
        GPSPath Out;
        uint32_t LastScene = UINT32_MAX;
        uint32_t HopCost   = 0;

        for (uint32_t Node : Raw)
        {
            if (Node == VirtSource || Node == VirtSink) continue;
            const auto It = G.EntranceScene.find(Node);
            if (It == G.EntranceScene.end()) continue;
            const uint32_t Scene = It->second;

            if (Scene != LastScene)
            {
                if (!Out.Steps.empty())
                {
                    // Close the previous step with the accumulated portal-hop cost.
                    Out.Steps.last().Cost = HopCost;
                    Out.Steps.last().ViaText = "Walk";
                    Out.TotalCost += HopCost;
                }
                GPSPathStep Step;
                Step.SceneID = Scene;
                SceneMetaInfo* Meta = GetSceneMetaInfo(Scene, (uint32_t)Game);
                Step.SceneName = (Meta && Meta->Name) ? QString::fromUtf8(Meta->Name)
                                                       : QString("Scene %1").arg(Scene);
                Out.Steps.append(Step);
                LastScene = Scene;
                HopCost = 0;
            }
            else
            {
                // Same scene as previous node: don't create a new station, but the
                // walk cost between the two entrances still counts toward this hop.
            }
        }

        // Final step closes silently (no via, no cost).
        return Out;
    }
}

#pragma endregion


#pragma region // Public API

GPSPathfindResult FindGPSRoutes(int FromGame, uint32_t FromScene,
                                int ToGame,   uint32_t ToScene,
                                int MaxRoutes)
{
    GPSPathfindResult Out;

    if (FromGame == ToGame && FromScene == ToScene)
    {
        Out.Status = GPS_SameScene;
        return Out;
    }

    if (FromGame != ToGame)
    {
        Out.Status = GPS_CrossGameUnsupported;
        return Out;
    }

    // Pre-check 1 - Cannot leave Start Area: no entrance of the start scene
    // leads to a different scene.
    std::map<int, EntranceMetaInfo>* Map = nullptr;
    if      (FromGame == OOT_GAME) Map = &OoTEntrances;
    else if (FromGame == MM_GAME)  Map = &MMEntrances;
    if (Map == nullptr) { Out.Status = GPS_NoPath; return Out; }

    bool CanLeave = false;
    for (const auto& Pair : *Map)
    {
        const EntranceMetaInfo& V = Pair.second;
        if (V.Type == EntranceType::None) continue;
        if (V.FromSceneID == FromScene && V.ToSceneID != FromScene)
        {
            CanLeave = true;
            break;
        }
    }
    if (!CanLeave) { Out.Status = GPS_CannotLeaveStart; return Out; }

    // Pre-check 2 - Destination Unreachable: no entrance anywhere leads into the
    // destination scene.
    bool ReachableDest = false;
    for (const auto& Pair : *Map)
    {
        const EntranceMetaInfo& V = Pair.second;
        if (V.Type == EntranceType::None) continue;
        if (V.ToSceneID == ToScene && V.FromSceneID != ToScene)
        {
            ReachableDest = true;
            break;
        }
    }
    if (!ReachableDest) { Out.Status = GPS_DestinationUnreachable; return Out; }

    // Build the graph and attach virtual endpoints so Dijkstra can multi-source /
    // multi-sink over the entrances of the start / end scenes.
    Graph G = BuildGraph(FromGame);
    if (!AttachVirtualEndpoints(G, FromScene, ToScene))
    {
        Out.Status = GPS_NoPath;
        return Out;
    }

    auto Raw = YenKShortestPaths(G, VirtSource, VirtSink, MaxRoutes);
    if (Raw.empty()) { Out.Status = GPS_NoPath; return Out; }

    for (const auto& R : Raw)
    {
        GPSPath P = ToScenePath(G, R, FromGame);
        if (P.Steps.size() < 2) continue;
        Out.Routes.append(P);
    }

    if (Out.Routes.isEmpty()) { Out.Status = GPS_NoPath; return Out; }

    Out.Status = GPS_Ok;
    return Out;
}

#pragma endregion
