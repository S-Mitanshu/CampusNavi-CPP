#include "Graph.h"

#include <algorithm>
#include <limits>
#include <sstream>
#include <stdexcept>

void Graph::addLocation(const Location& location) {
    if (location.id.empty()) throw std::invalid_argument("Location id must not be empty");
    // emplace returns {iterator, bool inserted}; `.second` is false on duplicates.
    if (!locations_.emplace(location.id, location).second)
        throw std::invalid_argument("Duplicate location id: " + location.id);
    adjacency_[location.id];  // creates an empty neighbour list, even for isolated nodes
}

void Graph::addEdge(const NodeID& a, const NodeID& b, double weight) {
    if (!hasLocation(a)) throw std::invalid_argument("Unknown location in edge: " + a);
    if (!hasLocation(b)) throw std::invalid_argument("Unknown location in edge: " + b);
    if (weight < 0) throw std::invalid_argument("Negative edge weight between " + a + " and " + b);
    // Undirected: store the edge in BOTH directions.
    adjacency_[a].push_back({b, weight});
    adjacency_[b].push_back({a, weight});
    ++edgeCount_;
}

bool Graph::hasLocation(const NodeID& id) const { return locations_.count(id) > 0; }

const Location& Graph::getLocation(const NodeID& id) const {
    auto it = locations_.find(id);
    if (it == locations_.end()) throw std::out_of_range("Unknown location: " + id);
    return it->second;
}

const std::vector<Edge>& Graph::neighbors(const NodeID& id) const {
    auto it = adjacency_.find(id);
    if (it == adjacency_.end()) throw std::out_of_range("Unknown location: " + id);
    return it->second;
}

double Graph::edgeWeight(const NodeID& a, const NodeID& b) const {
    double best = std::numeric_limits<double>::infinity();
    for (const Edge& e : neighbors(a))
        if (e.to == b) best = std::min(best, e.weight);
    if (best == std::numeric_limits<double>::infinity())
        throw std::out_of_range("No edge between " + a + " and " + b);
    return best;
}

std::vector<NodeID> Graph::locationIds() const {
    std::vector<NodeID> ids;
    ids.reserve(locations_.size());
    for (const auto& entry : locations_) ids.push_back(entry.first);
    std::sort(ids.begin(), ids.end());
    return ids;
}

std::vector<std::string> Graph::findHeuristicViolations() const {
    std::vector<std::string> problems;
    constexpr double kEpsilon = 1e-9;  // tolerate floating-point noise
    for (const auto& entry : adjacency_) {
        const NodeID& from = entry.first;
        for (const Edge& e : entry.second) {
            if (from >= e.to) continue;  // each undirected edge is stored twice; report once
            const double straight = getLocation(from).distanceTo(getLocation(e.to));
            if (e.weight + kEpsilon < straight) {
                std::ostringstream msg;
                msg << from << " <-> " << e.to << ": weight " << e.weight
                    << " is shorter than straight-line " << straight;
                problems.push_back(msg.str());
            }
        }
    }
    std::sort(problems.begin(), problems.end());
    return problems;
}
