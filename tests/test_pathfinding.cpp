// test_pathfinding.cpp
// A tiny hand-rolled test harness (no external libraries needed). Each test runs
// against BOTH algorithms, because they must agree on the answer.
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "AStar.h"
#include "CampusData.h"
#include "Dijkstra.h"

static int failures = 0;
static int checks = 0;
#define CHECK(cond)                                                                      \
    do {                                                                                 \
        ++checks;                                                                        \
        if (!(cond)) { ++failures; std::cout << "  FAIL line " << __LINE__ << ": " #cond "\n"; } \
    } while (0)

static bool near(double a, double b) { return std::fabs(a - b) < 1e-9; }

static std::vector<std::unique_ptr<PathFinder>> allAlgorithms() {
    std::vector<std::unique_ptr<PathFinder>> v;
    v.push_back(std::make_unique<Dijkstra>());
    v.push_back(std::make_unique<AStar>());
    return v;
}

static Location loc(const std::string& id, double x, double y, int floor = 1) {
    Location l; l.id = id; l.building = "T"; l.floor = floor; l.x = x; l.y = y;
    return l;
}

static void testBasicPath() {   // A -> B -> C
    Graph g;
    g.addLocation(loc("A", 0, 0)); g.addLocation(loc("B", 3, 0)); g.addLocation(loc("C", 6, 0));
    g.addEdge("A", "B", 3); g.addEdge("B", "C", 3);
    for (auto& f : allAlgorithms()) {
        auto r = f->findPath(g, "A", "C");
        CHECK(r.found); CHECK(near(r.distance, 6));
        CHECK((r.path == std::vector<NodeID>{"A", "B", "C"}));
    }
}

static void testDirectPath() {  // A -> B
    Graph g;
    g.addLocation(loc("A", 0, 0)); g.addLocation(loc("B", 3, 0)); g.addEdge("A", "B", 3);
    for (auto& f : allAlgorithms()) {
        auto r = f->findPath(g, "A", "B");
        CHECK(r.found); CHECK(near(r.distance, 3)); CHECK(r.path.size() == 2);
    }
}

static void testPicksCheaperRoute() {  // A-B-D costs 10, A-C-D costs 4, direct A-D costs 9
    Graph g;
    g.addLocation(loc("A", 0, 0)); g.addLocation(loc("C", 2, 0));
    g.addLocation(loc("D", 4, 0)); g.addLocation(loc("B", 2, 2));
    g.addEdge("A", "B", 5); g.addEdge("B", "D", 5);
    g.addEdge("A", "C", 2); g.addEdge("C", "D", 2);
    g.addEdge("A", "D", 9);
    for (auto& f : allAlgorithms()) {
        auto r = f->findPath(g, "A", "D");
        CHECK(near(r.distance, 4));
        CHECK((r.path == std::vector<NodeID>{"A", "C", "D"}));
    }
}

static void testNoPath() {
    Graph g;
    g.addLocation(loc("A", 0, 0)); g.addLocation(loc("B", 9, 9));  // B disconnected
    for (auto& f : allAlgorithms()) {
        auto r = f->findPath(g, "A", "B");
        CHECK(!r.found); CHECK(r.path.empty());
    }
}

static void testSameSourceAndDestination() {
    Graph g;
    g.addLocation(loc("A", 0, 0));
    for (auto& f : allAlgorithms()) {
        auto r = f->findPath(g, "A", "A");
        CHECK(r.found); CHECK(near(r.distance, 0)); CHECK(r.path.size() == 1);
    }
}

static void testInvalidInputThrows() {
    Graph g; g.addLocation(loc("A", 0, 0));
    for (auto& f : allAlgorithms()) {
        bool threw = false;
        try { f->findPath(g, "A", "NOPE"); } catch (const std::invalid_argument&) { threw = true; }
        CHECK(threw);
    }
    bool dupThrew = false;
    try { g.addLocation(loc("A", 1, 1)); } catch (const std::invalid_argument&) { dupThrew = true; }
    CHECK(dupThrew);
    bool negThrew = false;
    g.addLocation(loc("B", 1, 0));
    try { g.addEdge("A", "B", -1); } catch (const std::invalid_argument&) { negThrew = true; }
    CHECK(negThrew);
}

static Graph campus() { return loadCampus(CAMPUS_DATA_PATH); }

static void testMultiFloorRoute() {
    Graph g = campus();
    for (auto& f : allAlgorithms()) {
        auto r = f->findPath(g, "A101", "B202");
        // 5 + 4 + 4 + 6 (stairs up) + 12 (skybridge) + 5 = 36
        CHECK(r.found); CHECK(near(r.distance, 36));
        bool crossedFloor = false;
        for (size_t i = 0; i + 1 < r.path.size(); ++i)
            if (g.getLocation(r.path[i]).floor != g.getLocation(r.path[i + 1]).floor) crossedFloor = true;
        CHECK(crossedFloor);
    }
}

static void testIsolatedLocation() {
    Graph g = campus();
    for (auto& f : allAlgorithms()) CHECK(!f->findPath(g, "A101", "Z999").found);
}

static void testAlgorithmsAgreeOnEveryPair() {
    Graph g = campus();
    Dijkstra d; AStar a;
    for (const auto& s : g.locationIds())
        for (const auto& t : g.locationIds()) {
            auto rd = d.findPath(g, s, t), ra = a.findPath(g, s, t);
            CHECK(rd.found == ra.found);
            if (rd.found) CHECK(near(rd.distance, ra.distance));
        }
}

static void testAStarExploresNoMoreThanDijkstra() {
    Graph g = campus(); Dijkstra d; AStar a;
    CHECK(a.findPath(g, "A101", "B202").nodesExplored <= d.findPath(g, "A101", "B202").nodesExplored);
}

static void testCampusDataSatisfiesHeuristicRule() {
    CHECK(campus().findHeuristicViolations().empty());
}

static void testMalformedFileThrows() {
    const std::string path = "malformed_test_campus.txt";
    { std::ofstream out(path); out << "[LOCATIONS]\nA,A,1,Classroom,0\n"; }  // 5 fields, needs 6
    bool threw = false;
    try { loadCampus(path); } catch (const std::runtime_error& e) {
        threw = std::string(e.what()).find(":2:") != std::string::npos;  // reports line 2
    }
    CHECK(threw);
    std::remove(path.c_str());
    bool missing = false;
    try { loadCampus("does_not_exist.txt"); } catch (const std::runtime_error&) { missing = true; }
    CHECK(missing);
}

int main() {
    struct { const char* name; void (*fn)(); } tests[] = {
        {"basic path", testBasicPath}, {"direct path", testDirectPath},
        {"picks cheaper route", testPicksCheaperRoute}, {"no path", testNoPath},
        {"same source and destination", testSameSourceAndDestination},
        {"invalid input throws", testInvalidInputThrows},
        {"multi-floor route", testMultiFloorRoute}, {"isolated location", testIsolatedLocation},
        {"algorithms agree on every pair", testAlgorithmsAgreeOnEveryPair},
        {"A* explores <= Dijkstra", testAStarExploresNoMoreThanDijkstra},
        {"data satisfies heuristic rule", testCampusDataSatisfiesHeuristicRule},
        {"malformed file throws", testMalformedFileThrows},
    };
    for (auto& t : tests) { std::cout << "[ RUN ] " << t.name << "\n"; t.fn(); }
    std::cout << "\n" << checks << " checks, " << failures << " failures\n";
    return failures == 0 ? 0 : 1;
}
