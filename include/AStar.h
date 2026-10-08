// AStar.h - see AStar.cpp for the algorithm explained step by step.
#pragma once
#include "PathFinder.h"

class AStar : public PathFinder {
public:
    std::string name() const override { return "A*"; }
    PathResult findPath(const Graph& graph, const NodeID& source,
                        const NodeID& destination) const override;
};
