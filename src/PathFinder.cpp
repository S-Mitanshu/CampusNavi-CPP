#include "PathFinder.h"

#include <algorithm>
#include <stdexcept>

std::vector<NodeID> reconstructPath(const std::unordered_map<NodeID, NodeID>& parent,
                                    const NodeID& source, const NodeID& destination) {
    std::vector<NodeID> path;
    NodeID current = destination;
    path.push_back(current);
    while (current != source) {
        auto it = parent.find(current);
        if (it == parent.end()) throw std::logic_error("Broken parent chain at " + current);
        current = it->second;
        path.push_back(current);
    }
    std::reverse(path.begin(), path.end());  // we built it destination -> source
    return path;
}

void requireLocations(const Graph& graph, const NodeID& source, const NodeID& destination) {
    if (!graph.hasLocation(source))
        throw std::invalid_argument("Unknown source location: " + source);
    if (!graph.hasLocation(destination))
        throw std::invalid_argument("Unknown destination location: " + destination);
}
