// AStar.cpp
// -----------------------------------------------------------------------------
// IDEA: Dijkstra explores in all directions, like a ripple. A* adds a GUESS of
// the remaining distance, so the search leans toward the destination.
//
//     f(n) = g(n) + h(n)
//     g(n) = real cost from source to n (what Dijkstra tracks)
//     h(n) = heuristic: straight-line 3D distance from n to the destination
//
// The min-heap is ordered by f instead of g. If h = 0 everywhere, A* IS Dijkstra.
//
// WHEN IS A* CORRECT?
//   * Admissible: h never OVERestimates the true remaining cost. A straight line
//     is the shortest possible way between two points, so it never overestimates
//     - PROVIDED every edge weight >= straight-line distance of its endpoints
//     (Graph::findHeuristicViolations checks this).
//   * Consistent: h(a) <= w(a,b) + h(b). The triangle inequality gives this for
//     Euclidean distance. Consistency means a settled node is never improved
//     later, so the simple "settled set" below is valid.
//
// FLOORS: h uses z = floor * FLOOR_HEIGHT_M, so the guess accounts for climbing.
// -----------------------------------------------------------------------------
#include "AStar.h"

#include <chrono>
#include <functional>
#include <queue>
#include <unordered_set>
#include <utility>

PathResult AStar::findPath(const Graph& graph, const NodeID& source,
                           const NodeID& destination) const {
    requireLocations(graph, source, destination);
    const auto start = std::chrono::steady_clock::now();

    const Location& goal = graph.getLocation(destination);
    auto heuristic = [&](const NodeID& id) { return graph.getLocation(id).distanceTo(goal); };

    PathResult result;
    std::unordered_map<NodeID, double> g;        // real cost from source
    std::unordered_map<NodeID, NodeID> parent;
    std::unordered_set<NodeID>         settled;

    using QueueItem = std::pair<double, NodeID>;  // (f = g + h, node)
    std::priority_queue<QueueItem, std::vector<QueueItem>, std::greater<QueueItem>> queue;

    g[source] = 0.0;
    queue.push({heuristic(source), source});

    while (!queue.empty()) {
        const NodeID u = queue.top().second;
        queue.pop();
        if (settled.count(u)) continue;
        settled.insert(u);
        ++result.nodesExplored;

        if (u == destination) {
            result.found = true;
            result.distance = g[u];
            result.path = reconstructPath(parent, source, destination);
            break;
        }
        for (const Edge& edge : graph.neighbors(u)) {
            const double candidate = g[u] + edge.weight;
            auto it = g.find(edge.to);
            if (it == g.end() || candidate < it->second) {
                g[edge.to] = candidate;
                parent[edge.to] = u;
                queue.push({candidate + heuristic(edge.to), edge.to});
            }
        }
    }

    result.timeMs = std::chrono::duration<double, std::milli>(
                        std::chrono::steady_clock::now() - start).count();
    return result;
}
