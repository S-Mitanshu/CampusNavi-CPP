// Graph.h
// -----------------------------------------------------------------------------
// WHAT: The campus as a weighted, UNDIRECTED graph stored as an ADJACENCY LIST.
// WHY : Pathfinding algorithms need one fast question answered over and over:
//       "from this node, which nodes can I walk to, and how far?"
//       An adjacency list answers that in O(degree) - no scanning of all edges.
//
//   adjacency_["A102"] -> [ {A101,5}, {A103,5}, {A-Hall-1,4} ]
//
// ADJACENCY LIST vs MATRIX: a matrix needs V*V cells. Campuses are SPARSE (each
// room touches a few others), so a list uses O(V+E) memory instead of O(V^2).
//
// C++ CONCEPTS HERE
//   * unordered_map : hash table, average O(1) lookup by string id
//   * returning `const T&` : give callers a read-only view, no copying
//   * exceptions : mutators throw on bad input instead of silently building a
//                  broken graph
// -----------------------------------------------------------------------------
#pragma once
#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

#include "Location.h"

using NodeID = std::string;

struct Edge {
    NodeID to;
    double weight;  // walking distance in metres
};

class Graph {
public:
    // Add a node. Throws std::invalid_argument if the id already exists.
    void addLocation(const Location& location);

    // Add a two-way walkable connection. Throws std::invalid_argument if either
    // id is unknown or the weight is negative (Dijkstra breaks on negatives).
    void addEdge(const NodeID& a, const NodeID& b, double weight);

    bool            hasLocation(const NodeID& id) const;
    const Location& getLocation(const NodeID& id) const;         // throws std::out_of_range
    const std::vector<Edge>& neighbors(const NodeID& id) const;  // throws std::out_of_range
    double          edgeWeight(const NodeID& a, const NodeID& b) const;  // cheapest a-b edge

    std::vector<NodeID> locationIds() const;  // sorted, for stable display
    std::size_t locationCount() const { return locations_.size(); }
    std::size_t edgeCount() const { return edgeCount_; }

    // A* is only guaranteed optimal if the heuristic never overestimates, which
    // holds when every edge weight >= straight-line distance of its endpoints.
    // This returns a message for each edge that violates that rule.
    std::vector<std::string> findHeuristicViolations() const;

private:
    std::unordered_map<NodeID, Location>          locations_;
    std::unordered_map<NodeID, std::vector<Edge>> adjacency_;
    std::size_t edgeCount_ = 0;
};
