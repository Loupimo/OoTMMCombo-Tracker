#include "UI/GPSPathfinder.h"
#include "UI/SceneEntrance.h"
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
    constexpr uint32_t  VirtSource      = 0xFFFFFFFEu;
    constexpr uint32_t  VirtSink        = 0xFFFFFFFFu;

    // Cross-game node encoding. OoT and MM use overlapping entrance ID ranges (OoT ~12 bits,
    // MM up to ~17 bits, both well below 0x80000000) so we use bit 31 of the graph's node ID
    // to flag MM. Together with the OoT/MM scene maps that are also game-keyed, this lets a
    // single unified Graph cover both games without ID collisions.
    constexpr uint32_t  MM_NODE_FLAG    = 0x80000000u;

    // Walk cost (seconds) used for cross-game warp access edges added directly in BuildGraph
    // (the intra-game walks for warp songs / owls live in V.Cost.Costs through Entrances.cpp
    // and use the same constant declared there).
    constexpr uint32_t  CROSS_GAME_WARP_COST = 10u;

    inline uint32_t EncodeNode(uint8_t Game, uint32_t EntranceID)
    {
        return (Game == MM_GAME) ? (EntranceID | MM_NODE_FLAG) : EntranceID;
    }

    inline uint8_t NodeGame(uint32_t Node)
    {
        return (Node & MM_NODE_FLAG) ? (uint8_t)MM_GAME : (uint8_t)OOT_GAME;
    }

    inline uint32_t NodeID(uint32_t Node)
    {
        return Node & ~MM_NODE_FLAG;
    }


    /*
    *   True when the given scene is one of the synthetic buckets the entrance data uses to park
    *   nodes that have no physical location: the warp song / owl choice menus, the generic grotto
    *   interiors and the cutscene / extra maps. "Walking" between two entrances of such a scene is
    *   meaningless - you cannot stroll from one grotto interior to another - so their layout costs
    *   must never become graph edges.
    *
    *   @param Game       OOT_GAME or MM_GAME.
    *   @param SceneID    The scene to test.
    *
    *   @return True when the scene is synthetic.
    */
    inline bool IsSyntheticScene(uint8_t Game, uint32_t SceneID)
    {
        if (Game == OOT_GAME)
        {
            return SceneID == OOT_GROTTOS || SceneID == OOT_CUTSCENE_MAP || SceneID == OOT_SONGS;
        }
        return SceneID == MM_GROTTOS || SceneID == MM_CUTSCENE_MAP || SceneID == MM_EXTRA || SceneID == MM_OWLS;
    }


    /*
    *   True when an entrance entry describes a real, walkable position inside its FromSceneID and
    *   should therefore contribute layout walk edges to the graph.
    *
    *   Type::None is NOT a reliable filter on its own. It marks entries the tracker deliberately
    *   keeps off the maps (spawn points, "wrong exit" loops, seasonal duplicates, half-typed doors
    *   like the Happy Mask Shop) - but several of those are the ONLY entry describing their node.
    *   Skipping them wholesale turned those nodes into dead ends: the router could reach them but
    *   never walk away, e.g. leaving the Mask Shop into the Market stranded the GPS. What actually
    *   has to be excluded is the synthetic scenes, so that is what we test.
    *
    *   @param Game    OOT_GAME or MM_GAME.
    *   @param V       The entrance entry.
    *
    *   @return True when the entry's layout costs are real walks.
    */
    inline bool ContributesLayoutWalks(uint8_t Game, const EntranceMetaInfo& V)
    {
        return V.Type != EntranceType::None || !IsSyntheticScene(Game, V.FromSceneID);
    }


    struct Edge
    {
        uint32_t    To = 0;
        uint32_t    Cost = 0;
        bool        IsPortal = false;       // True for inter-scene portal edges, false for intra-scene walks.
    };

    struct Graph
    {
        // Out-edges per encoded entrance node.
        std::unordered_map<uint32_t, std::vector<Edge>> Adj;
        // Reverse lookup: which physical scene each entrance lives in. The owning game is
        // derived from the encoded node via NodeGame() so we don't need a separate map.
        std::unordered_map<uint32_t, uint32_t>          EntranceScene;
    };


    /*
    *   Build a unified directed graph spanning BOTH OoT and MM. The GPS is meant for users who
    *   randomized their seed, so the "natural" portal layout in OoTEntrances / MMEntrances is
    *   NEVER trusted as the source of portals. Instead:
    *
    *     - The static map is used only to (a) register every entrance with its physical scene
    *       (entrances live where the data says they live - that's not randomized) and (b)
    *       provide intra-scene walking cost data (V.Cost.Costs, also a physical attribute).
    *     - Portal edges (entrance A leads to entrance B) come EXCLUSIVELY from the discovered
    *       SceneEntranceMetaInf.OutLink data. This includes intra-game randomized portals AND
    *       cross-game portals. The user has to actually traverse a portal in their game once
    *       for the GPS to know about it; an empty save will yield very few portals.
    *     - Cross-game warp access (synthetic walks from any entrance to the other game's
    *       warp-song / owl choices) is added on top, gated by the ROM parameters
    *       crossWarpOot (OoT player can play MM Song of Soaring) and crossWarpMm
    *       (MM player can play OoT warp songs).
    *
    *   @param CrossWarpOot    When true, OoT entrances get synthetic walks to MM owl choices.
    *   @param CrossWarpMm     When true, MM entrances get synthetic walks to OoT warp song choices.
    */
    Graph BuildGraph(bool CrossWarpOot, bool CrossWarpMm)
    {
        Graph G;

        auto AddForGame = [&G](uint8_t Game, std::map<int, EntranceMetaInfo>& Map)
        {
            // First pass: registration only. Each entrance gets its physical scene recorded so
            // ToScenePath can later resolve scene names from the right game.
            for (auto& Pair : Map)
            {
                const EntranceMetaInfo& V = Pair.second;
                if (!ContributesLayoutWalks(Game, V)) continue;

                const uint32_t FromN = EncodeNode(Game, V.FromEntranceID);
                const uint32_t ToN   = EncodeNode(Game, V.ToEntranceID);
                G.EntranceScene[FromN] = V.FromSceneID;
                G.EntranceScene[ToN]   = V.ToSceneID;
                // Static portal edge intentionally NOT added - portals are loaded from
                // discovered SceneEntranceMetaInf below so randomized seeds route correctly.
            }

            // Walks based on physical layout - the in-scene distance between two entrances
            // doesn't change with randomization, so these stay sourced from the static map.
            for (auto& Pair : Map)
            {
                const EntranceMetaInfo& V = Pair.second;
                if (!ContributesLayoutWalks(Game, V)) continue;
                const uint32_t FromN = EncodeNode(Game, V.FromEntranceID);
                for (const auto& KV : V.Cost.Costs)
                {
                    if (KV.second == UnreachableCost) continue;
                    G.Adj[FromN].push_back({ EncodeNode(Game, KV.first), KV.second, false });
                }
            }
        };

        AddForGame(OOT_GAME, OoTEntrances);
        AddForGame(MM_GAME,  MMEntrances);

        // Sole source of portal edges: discovered OutLinks for every entrance. Covers intra
        // and cross-game equally, and correctly reflects whatever randomization the seed has.
        // Entrances the player hasn't traversed yet contribute no portal - they're effectively
        // dead-ends in the GPS until the player learns where they lead.
        auto AddDiscoveredPortals = [&G](uint8_t Game, const std::map<uint32_t, SceneEntranceMetaInf>& Scenes)
        {
            for (const auto& ScenePair : Scenes)
            {
                const SceneEntranceMetaInf& Scene = ScenePair.second;
                for (const auto& EntrancePair : Scene.EntranceIDs)
                {
                    const uint32_t EID = EntrancePair.first;
                    const EntranceLink& Link = EntrancePair.second;
                    if (Link.OutLink == UINT32_MAX) continue;
                    if (Link.OutLinkGame == NO_GAME) continue;
                    const uint32_t From = EncodeNode(Game, EID);
                    const uint32_t To   = EncodeNode(Link.OutLinkGame, Link.OutLink);
                    G.Adj[From].push_back({ To, 0u, true });
                }
            }
        };
        AddDiscoveredPortals(OOT_GAME, OoTSceneEntranceMeta);
        AddDiscoveredPortals(MM_GAME,  MMSceneEntranceMeta);

        // Cross-game warp access: while the OoT / MM warp song / owl parameters are ON, the
        // player can play the OTHER game's warp from any entrance of their current game. We
        // model this by adding walk edges from every OoT entrance to every MM_OWL_X_CHOICE
        // (and vice versa) at CROSS_GAME_WARP_COST. The existing intra-MM_OWLS / OOT_SONGS
        // menu walks (set in InitializeEntranceCosts) handle the destination picking.
        auto CollectChoices = [](const std::map<int, EntranceMetaInfo>& Map, uint32_t WarpScene, uint8_t WarpGame, std::vector<uint32_t>& Out)
        {
            for (const auto& Pair : Map)
            {
                const EntranceMetaInfo& V = Pair.second;
                if (V.Type != EntranceType::One_Way_Out) continue;
                if (V.ToSceneID != WarpScene) continue;
                Out.push_back(EncodeNode(WarpGame, (uint32_t)Pair.first));
            }
        };
        std::vector<uint32_t> OoTSongChoices;
        std::vector<uint32_t> MMOwlChoices;
        CollectChoices(OoTEntrances, OOT_SONGS, OOT_GAME, OoTSongChoices);
        CollectChoices(MMEntrances,  MM_OWLS,   MM_GAME,  MMOwlChoices);

        auto AddCrossWarpWalks = [&G](uint8_t SourceGame, const std::map<int, EntranceMetaInfo>& SourceMap, uint32_t SkipScene,
                                      const std::vector<uint32_t>& Choices)
        {
            for (const auto& Pair : SourceMap)
            {
                const EntranceMetaInfo& V = Pair.second;
                if (V.Type == EntranceType::None) continue;
                if (V.FromSceneID == SkipScene) continue;            // skip entries inside the own warp scene
                const uint32_t From = EncodeNode(SourceGame, V.FromEntranceID);
                for (uint32_t Choice : Choices)
                {
                    G.Adj[From].push_back({ Choice, CROSS_GAME_WARP_COST, false });
                }
            }
        };
        // OoT entrances can play MM warp owls -> reach MM_OWLS choices. Gated by the
        // crossWarpOot ROM parameter ("Cross-Games OoT Warp Song" in the settings UI).
        if (CrossWarpOot)
        {
            AddCrossWarpWalks(OOT_GAME, OoTEntrances, OOT_SONGS, MMOwlChoices);
        }
        // MM entrances can play OoT warp songs -> reach OOT_SONGS choices. Gated by the
        // crossWarpMm ROM parameter ("Cross-Games MM Song of Soaring" in the settings UI).
        if (CrossWarpMm)
        {
            AddCrossWarpWalks(MM_GAME,  MMEntrances,  MM_OWLS,   OoTSongChoices);
        }

        return G;
    }


    /*
    *   Attach virtual source and sink nodes to the graph: VirtSource has a 0-cost edge to
    *   every entrance of the start (Game, Scene), and every entrance of the end (Game, Scene)
    *   has a 0-cost edge to VirtSink. Returns true if both virtual nodes got at least one
    *   connection (otherwise no path can exist).
    *
    *   When FromEntrance / ToEntrance is not UINT32_MAX the corresponding endpoint is pinned
    *   to that single entrance (by raw entrance ID, game-independent) instead of the whole
    *   scene: the route must then depart from / arrive at exactly that entrance.
    */
    bool AttachVirtualEndpoints(Graph& G, uint8_t StartGame, uint32_t StartScene, uint8_t EndGame, uint32_t EndScene,
                                uint32_t FromEntrance, uint32_t ToEntrance)
    {
        bool HasStart = false;
        bool HasEnd   = false;

        for (const auto& Pair : G.EntranceScene)
        {
            const uint32_t Node  = Pair.first;
            const uint32_t Scene = Pair.second;
            const uint8_t  Game  = NodeGame(Node);

            if (Game == StartGame && Scene == StartScene &&
                (FromEntrance == UINT32_MAX || NodeID(Node) == FromEntrance))
            {
                G.Adj[VirtSource].push_back({ Node, 0u, false });
                HasStart = true;
            }
            if (Game == EndGame && Scene == EndScene &&
                (ToEntrance == UINT32_MAX || NodeID(Node) == ToEntrance))
            {
                G.Adj[Node].push_back({ VirtSink, 0u, false });
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
    *   Convert a raw node sequence (encoded entrance IDs, possibly bracketed by VirtSource and
    *   VirtSink) into a GPSPath of scene stations, collapsing consecutive nodes that live in
    *   the same (Game, Scene) into a single station. The Game is derived from each node via
    *   NodeGame(), so the path can freely cross between OoT and MM.
    */
    GPSPath ToScenePath(const Graph& G, const std::vector<uint32_t>& Raw)
    {
        GPSPath Out;
        uint32_t LastScene = UINT32_MAX;
        uint8_t  LastGame  = NO_GAME;           // Sentinel so the first node always triggers a scene change.
        uint32_t HopCost   = 0;
        uint32_t PrevNode  = UINT32_MAX;

        // Returns the OTHER scene's display name for the given entrance node relative to
        // CurrentScene. The naming convention is asymmetric so we pick the side opposite to
        // CurrentScene. Game is derived from the node so this works for both OoT and MM.
        auto ExitDestinationName = [](uint32_t Node, uint32_t CurrentScene) -> QString
        {
            const uint8_t G = NodeGame(Node);
            const uint32_t ID = NodeID(Node);
            const EntranceMetaInfo* M = EntranceHelper::GetEntranceMetaInf(G, ID);
            if (M == nullptr) return QString();
            const char* Name = nullptr;
            if      (M->FromSceneID == CurrentScene) Name = M->ToName;
            else if (M->ToSceneID   == CurrentScene) Name = M->FromName;
            else                                     Name = M->ToName;
            return Name != nullptr ? QString::fromUtf8(Name) : QString();
        };

        auto IsWarpScene = [](uint32_t Scene, uint8_t Game)
        {
            return (Game == OOT_GAME && Scene == OOT_SONGS) ||
                   (Game == MM_GAME  && Scene == MM_OWLS);
        };

        for (uint32_t Node : Raw)
        {
            if (Node == VirtSource || Node == VirtSink) continue;
            const auto It = G.EntranceScene.find(Node);
            if (It == G.EntranceScene.end()) continue;
            const uint32_t Scene = It->second;
            const uint8_t  Game  = NodeGame(Node);

            // Scene change when either the scene or the game flips - a cross-game portal step
            // produces both, so the test triggers correctly on both flavors of transition.
            if (Scene != LastScene || Game != LastGame)
            {
                if (!Out.Steps.empty())
                {
                    // Cross-scene WALK edges (non-portal, non-zero cost) carry the song-playing
                    // time and belong to the leaving scene's hop. Portals (cost 0) need no
                    // accumulation.
                    const auto AdjIt = G.Adj.find(PrevNode);
                    if (AdjIt != G.Adj.end())
                    {
                        for (const Edge& E : AdjIt->second)
                        {
                            if (E.To == Node && !E.IsPortal)
                            {
                                HopCost += E.Cost;
                                break;
                            }
                        }
                    }

                    Out.Steps.last().Cost = HopCost;
                    QString ExitName;
                    const bool LeavingWarp = IsWarpScene(LastScene, LastGame);
                    const bool EnteringWarp = IsWarpScene(Scene, Game);
                    if (LeavingWarp)
                    {
                        const EntranceMetaInfo* PrevMeta = EntranceHelper::GetEntranceMetaInf(NodeGame(PrevNode), NodeID(PrevNode));
                        if (PrevMeta != nullptr && PrevMeta->FromName != nullptr)
                        {
                            ExitName = QString::fromUtf8(PrevMeta->FromName);
                        }
                    }
                    else if (EnteringWarp)
                    {
                        ExitName = (Game == OOT_GAME) ? QStringLiteral("Warp Song") : QStringLiteral("Warp Owl");
                    }
                    else
                    {
                        ExitName = ExitDestinationName(PrevNode, LastScene);
                    }
                    Out.Steps.last().ViaText = ExitName.isEmpty() ? QStringLiteral("Walk") : ExitName;
                    Out.TotalCost += HopCost;
                }
                GPSPathStep Step;
                Step.SceneID = Scene;
                Step.Game    = Game;
                SceneMetaInfo* Meta = GetSceneMetaInfo(Scene, (uint32_t)Game);
                Step.SceneName = (Meta && Meta->Name) ? QString::fromUtf8(Meta->Name)
                                                       : QString("Scene %1").arg(Scene);
                Out.Steps.append(Step);
                LastScene = Scene;
                LastGame  = Game;
                HopCost   = 0;
            }
            else
            {
                // Same scene as previous node: don't create a new station, but the walk cost
                // from PrevNode to Node belongs to this hop. Only non-portal edges count.
                const auto AdjIt = G.Adj.find(PrevNode);
                if (AdjIt != G.Adj.end())
                {
                    for (const Edge& E : AdjIt->second)
                    {
                        if (E.To == Node && !E.IsPortal)
                        {
                            HopCost += E.Cost;
                            break;
                        }
                    }
                }
            }
            PrevNode = Node;
        }

        // Final step closes silently (no via, no cost).
        return Out;
    }
}

#pragma endregion


#pragma region // Public API

GPSPathfindResult FindGPSRoutes(int FromGame, uint32_t FromScene,
                                int ToGame,   uint32_t ToScene,
                                int MaxRoutes,
                                bool CrossWarpOot, bool CrossWarpMm,
                                uint32_t FromEntrance, uint32_t ToEntrance)
{
    GPSPathfindResult Out;

    // Same scene is only a no-op when the endpoints truly coincide: same scene AND (no specific
    // entrance pinned on either side, or the same entrance on both). Two DIFFERENT pinned
    // entrances of one scene are a real query - the player wants to get from one to the other,
    // which (when they don't connect internally) means leaving the scene and coming back.
    if (FromGame == ToGame && FromScene == ToScene &&
        (FromEntrance == UINT32_MAX || ToEntrance == UINT32_MAX || FromEntrance == ToEntrance))
    {
        Out.Status = GPS_SameScene;
        return Out;
    }

    // Cross-game IS supported now: the unified graph spans both OoT and MM, and the user's
    // discovered cross-game portals (stored in SceneEntranceMetaInf) plus the always-on warp
    // song / owl cross-game access make any OoT <-> MM trip routable.

    auto SceneMetaForGame = [](int Game) -> std::map<uint32_t, SceneEntranceMetaInf>*
    {
        if (Game == OOT_GAME) return &OoTSceneEntranceMeta;
        if (Game == MM_GAME)  return &MMSceneEntranceMeta;
        return nullptr;
    };
    std::map<uint32_t, SceneEntranceMetaInf>* StartMeta = SceneMetaForGame(FromGame);
    std::map<uint32_t, SceneEntranceMetaInf>* EndMeta   = SceneMetaForGame(ToGame);
    if (StartMeta == nullptr || EndMeta == nullptr) { Out.Status = GPS_NoPath; return Out; }

    // Pre-check 1 - Cannot leave Start Area: no entrance in the start scene has a discovered
    // OutLink. We use discovered data (not the static map) because the GPS is meant for
    // randomized seeds: static FromSceneID/ToSceneID can be misleading if the player has
    // discovered different portals than the vanilla layout.
    bool CanLeave = false;
    {
        auto It = StartMeta->find(FromScene);
        if (It != StartMeta->end())
        {
            for (const auto& EP : It->second.EntranceIDs)
            {
                if (EP.second.OutLink != UINT32_MAX)
                {
                    CanLeave = true;
                    break;
                }
            }
        }
    }
    if (!CanLeave) { Out.Status = GPS_CannotLeaveStart; return Out; }

    // Pre-check 2 - Destination Unreachable: no entrance in the destination scene has any
    // discovered InLink (no one has ever arrived there). Discovered data again, same reasoning.
    bool ReachableDest = false;
    {
        auto It = EndMeta->find(ToScene);
        if (It != EndMeta->end())
        {
            for (const auto& EP : It->second.EntranceIDs)
            {
                if (!EP.second.InLinks.empty())
                {
                    ReachableDest = true;
                    break;
                }
            }
        }
    }
    if (!ReachableDest) { Out.Status = GPS_DestinationUnreachable; return Out; }

    // Build the unified graph (OoT + MM + cross-game portals + cross-game warp access) and
    // attach virtual endpoints over the (Game, Scene) of start and end.
    Graph G = BuildGraph(CrossWarpOot, CrossWarpMm);
    if (!AttachVirtualEndpoints(G, (uint8_t)FromGame, FromScene, (uint8_t)ToGame, ToScene, FromEntrance, ToEntrance))
    {
        Out.Status = GPS_NoPath;
        return Out;
    }

    auto Raw = YenKShortestPaths(G, VirtSource, VirtSink, MaxRoutes);
    if (Raw.empty()) { Out.Status = GPS_NoPath; return Out; }

    // Walking-only fallback: ensure the result set contains at least one path that doesn't
    // go through a warp song / owl, even if Yen's would naturally prefer warp variants. We
    // forbid every node living in BOTH OOT_SONGS and MM_OWLS (encoded with the right game)
    // so the fallback works for cross-game routes too.
    {
        std::unordered_set<uint32_t> WarpNodes;
        for (const auto& Pair : G.EntranceScene)
        {
            const uint32_t Node  = Pair.first;
            const uint32_t Scene = Pair.second;
            const uint8_t  Game  = NodeGame(Node);
            if ((Game == OOT_GAME && Scene == OOT_SONGS) ||
                (Game == MM_GAME  && Scene == MM_OWLS))
            {
                WarpNodes.insert(Node);
            }
        }
        if (!WarpNodes.empty())
        {
            DijkstraResult NonWarp = RunDijkstra(G, VirtSource, VirtSink, {}, WarpNodes);
            if (NonWarp.Found && !NonWarp.Path.empty())
            {
                Raw.push_back(NonWarp.Path);
            }
        }
    }

    for (const auto& R : Raw)
    {
        GPSPath P = ToScenePath(G, R);
        if (P.Steps.size() < 2) continue;
        Out.Routes.append(P);
    }

    if (Out.Routes.isEmpty()) { Out.Status = GPS_NoPath; return Out; }

    // Re-sort by accumulated TotalCost: the non-warp fallback appended above breaks Yen's
    // ascending order, and ToScenePath's hop accounting may shift a path's effective cost
    // away from what Yen's PathCost saw. Stable sort keeps Yen's tie-breaking order otherwise.
    std::stable_sort(Out.Routes.begin(), Out.Routes.end(),
        [](const GPSPath& A, const GPSPath& B) { return A.TotalCost < B.TotalCost; });

    Out.Status = GPS_Ok;
    return Out;
}

#pragma endregion
