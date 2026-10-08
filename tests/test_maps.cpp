// test_maps.cpp
// Validates the generated 100- and 300-node maps with the real C++ loader, checks that
// Dijkstra and A* agree on many random routes, and prints a benchmark summary.
// (Timing is only printed, never asserted: it varies between machines.)
#include <chrono>
#include <iomanip>
#include <iostream>
#include <queue>
#include <random>
#include <unordered_set>

#include "AStar.h"
#include "CampusData.h"
#include "Dijkstra.h"

static int failures = 0, checks = 0;
#define CHECK(cond) do { ++checks; if (!(cond)) { ++failures; std::cout << "  FAIL line " << __LINE__ << ": " #cond "\n"; } } while (0)

static bool isConnected(const Graph& g) {
    const auto ids = g.locationIds();
    std::unordered_set<NodeID> seen{ids.front()};
    std::queue<NodeID> q; q.push(ids.front());
    while (!q.empty()) {
        NodeID u = q.front(); q.pop();
        for (const Edge& e : g.neighbors(u)) if (seen.insert(e.to).second) q.push(e.to);
    }
    return seen.size() == ids.size();
}

static void runMap(const std::string& label, const std::string& path, std::size_t nodes) {
    std::cout << "[ MAP ] " << label << "\n";
    Graph g = loadCampus(path);
    CHECK(g.locationCount() == nodes);
    CHECK(g.findHeuristicViolations().empty());
    CHECK(isConnected(g));

    const auto ids = g.locationIds();
    std::mt19937 rng(12345);  // fixed seed => same pairs every run
    std::uniform_int_distribution<std::size_t> pick(0, ids.size() - 1);
    Dijkstra d; AStar a;
    constexpr int kPairs = 300;
    std::size_t exploredD = 0, exploredA = 0;
    double msD = 0, msA = 0;
    for (int i = 0; i < kPairs; ++i) {
        const NodeID& s = ids[pick(rng)]; const NodeID& t = ids[pick(rng)];
        auto t0 = std::chrono::steady_clock::now();
        PathResult rd = d.findPath(g, s, t);
        auto t1 = std::chrono::steady_clock::now();
        PathResult ra = a.findPath(g, s, t);
        auto t2 = std::chrono::steady_clock::now();
        CHECK(rd.found && ra.found);                       // maps are connected
        CHECK(std::abs(rd.distance - ra.distance) < 1e-9); // both must be optimal
        CHECK(ra.nodesExplored <= rd.nodesExplored + 0);   // A* never settles more here
        exploredD += rd.nodesExplored; exploredA += ra.nodesExplored;
        msD += std::chrono::duration<double, std::milli>(t1 - t0).count();
        msA += std::chrono::duration<double, std::milli>(t2 - t1).count();
    }
    std::cout << std::fixed << std::setprecision(1)
              << "        " << kPairs << " random routes, avg nodes explored: Dijkstra "
              << double(exploredD) / kPairs << " | A* " << double(exploredA) / kPairs
              << "  (A* explores " << 100.0 * (1.0 - double(exploredA) / exploredD) << "% fewer)\n"
              << std::setprecision(4) << "        avg time per route: Dijkstra " << msD / kPairs
              << " ms | A* " << msA / kPairs << " ms\n";
}

int main() {
    runMap("campus_100 (medium)", CAMPUS_100_PATH, 100);
    runMap("campus_300 (large)", CAMPUS_300_PATH, 300);
    std::cout << "\n" << checks << " checks, " << failures << " failures\n";
    return failures == 0 ? 0 : 1;
}
