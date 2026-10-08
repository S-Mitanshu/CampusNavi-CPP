// Dijkstra.cpp
// -----------------------------------------------------------------------------
// IDEA: Always finalise the closest not-yet-finalised node. Because all weights
// are >= 0, once a node is the closest unfinalised one, no later route can reach
// it more cheaply, so its distance is final ("settled").
//
// STEPS
//   1. dist[source] = 0, push source onto a MIN-HEAP keyed by distance.
//   2. Pop the smallest (d, u). If u is already settled, skip it (stale entry).
//   3. Settle u. If u is the destination -> done.
//   4. For each neighbour v: if d + w(u,v) < dist[v], we found a better route:
//      update dist[v], remember parent[v] = u, push (newDist, v).
//
// WHY "lazy deletion"? std::priority_queue cannot lower an existing key, so we
// push a new, better entry and ignore the old one when it eventually pops.
//
// COMPLEXITY: O((V + E) log V) with a binary heap.
//
// C++ CONCEPTS: priority_queue with std::greater => MIN-heap (the default is a
// max-heap!), structured bindings, std::chrono for timing.
// -----------------------------------------------------------------------------
#include "Dijkstra.h"

#include <chrono>
#include <functional>
#include <queue>
#include <unordered_set>
#include <utility>

PathResult Dijkstra::findPath(const Graph& graph, const NodeID& source,
                              const NodeID& destination) const {
    requireLocations(graph, source, destination);
    const auto start = std::chrono::steady_clock::now();

    PathResult result;
    std::unordered_map<NodeID, double> dist;     // best known distance so far
    std::unordered_map<NodeID, NodeID> parent;   // for path reconstruction
    std::unordered_set<NodeID>         settled;  // finalised nodes

    using QueueItem = std::pair<double, NodeID>;  // (distance, node)
    std::priority_queue<QueueItem, std::vector<QueueItem>, std::greater<QueueItem>> queue;

    dist[source] = 0.0;
    queue.push({0.0, source});

    while (!queue.empty()) {
        const double d = queue.top().first;
        const NodeID u = queue.top().second;
        queue.pop();
        if (settled.count(u)) continue;  // stale entry
        settled.insert(u);
        ++result.nodesExplored;

        if (u == destination) {
            result.found = true;
            result.distance = d;
            result.path = reconstructPath(parent, source, destination);
            break;
        }
        for (const Edge& edge : graph.neighbors(u)) {
            const double candidate = d + edge.weight;
            auto it = dist.find(edge.to);
            if (it == dist.end() || candidate < it->second) {
                dist[edge.to] = candidate;
                parent[edge.to] = u;
                queue.push({candidate, edge.to});
            }
        }
    }

    result.timeMs = std::chrono::duration<double, std::milli>(
                        std::chrono::steady_clock::now() - start).count();
    return result;
}
