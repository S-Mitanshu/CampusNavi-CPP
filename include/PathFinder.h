// PathFinder.h
// -----------------------------------------------------------------------------
// WHAT: The common "shape" of a shortest-path algorithm + its result type.
// WHY : Dijkstra and A* take the same inputs and give the same kind of output.
//       An ABSTRACT BASE CLASS lets main.cpp and the tests treat them
//       interchangeably (polymorphism) - e.g. loop over both to compare.
//
// C++ CONCEPTS HERE
//   * pure virtual function (= 0): the base class demands subclasses implement it
//   * virtual destructor: makes deleting a derived object via a base pointer safe
// -----------------------------------------------------------------------------
#pragma once
#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

#include "Graph.h"

struct PathResult {
    bool                found = false;
    std::vector<NodeID> path;               // source ... destination (empty if !found)
    double              distance = 0.0;     // total metres
    std::size_t         nodesExplored = 0;  // nodes "settled" (popped & finalised)
    double              timeMs = 0.0;       // wall-clock time of ONE run
};

class PathFinder {
public:
    virtual ~PathFinder() = default;
    virtual std::string name() const = 0;
    // Throws std::invalid_argument if source or destination is not in the graph.
    virtual PathResult findPath(const Graph& graph, const NodeID& source,
                                const NodeID& destination) const = 0;
};

// Shared helper (PRD 4.5): walk the `parent` map backwards from destination to
// source, then reverse. parent[v] = the node we came from on v's best route.
std::vector<NodeID> reconstructPath(const std::unordered_map<NodeID, NodeID>& parent,
                                    const NodeID& source, const NodeID& destination);

// Throws std::invalid_argument with a clear message for unknown ids.
void requireLocations(const Graph& graph, const NodeID& source, const NodeID& destination);
