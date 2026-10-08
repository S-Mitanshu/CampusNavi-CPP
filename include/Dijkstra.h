// Dijkstra.h - see Dijkstra.cpp for the algorithm explained step by step.
#pragma once
#include "PathFinder.h"

class Dijkstra : public PathFinder {
public:
    std::string name() const override { return "Dijkstra"; }
    PathResult findPath(const Graph& graph, const NodeID& source,
                        const NodeID& destination) const override;
};
