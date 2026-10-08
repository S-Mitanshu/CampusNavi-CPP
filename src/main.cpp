// main.cpp - the command-line front end (PRD section 6).
// It only handles input/output; all logic lives in the library classes.
#include <cctype>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>

#include "AStar.h"
#include "CampusData.h"
#include "Dijkstra.h"

#ifndef DEFAULT_DATA_PATH
#define DEFAULT_DATA_PATH "data/campus.txt"
#endif

namespace {

std::string trim(const std::string& s) {
    const auto a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    return s.substr(a, s.find_last_not_of(" \t\r\n") - a + 1);
}

std::string upper(std::string s) {
    for (char& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return s;
}

// Case-insensitive lookup so "a102" finds "A102".
std::optional<NodeID> resolveId(const Graph& graph, const std::string& input) {
    const std::string wanted = upper(input);
    for (const NodeID& id : graph.locationIds())
        if (upper(id) == wanted) return id;
    return std::nullopt;
}

void describe(const Graph& graph, const NodeID& id) {
    const Location& loc = graph.getLocation(id);
    std::cout << id << "  (Building " << loc.building << ", floor " << loc.floor << ", "
              << toString(loc.type) << ")";
}

void listLocations(const Graph& graph) {
    std::cout << "\nKnown locations:\n";
    for (const NodeID& id : graph.locationIds()) { std::cout << "  "; describe(graph, id); std::cout << "\n"; }
    std::cout << "\n";
}

// Returns nullopt on end-of-input or an empty line (= cancel).
std::optional<NodeID> askLocation(const Graph& graph, const std::string& prompt) {
    std::string line;
    while (true) {
        std::cout << prompt << " (? to list, Enter to cancel): ";
        if (!std::getline(std::cin, line)) return std::nullopt;
        line = trim(line);
        if (line.empty()) return std::nullopt;
        if (line == "?") { listLocations(graph); continue; }
        if (auto id = resolveId(graph, line)) return id;
        std::cout << "  Unknown location '" << line << "'. Try again.\n";
    }
}

void printRoute(const Graph& graph, const PathResult& r) {
    if (!r.found) { std::cout << "\nNo route found.\n"; return; }
    std::cout << "\nRoute:\n";
    for (std::size_t i = 0; i < r.path.size(); ++i) {
        std::cout << "  " << (i + 1) << ". "; describe(graph, r.path[i]); std::cout << "\n";
        if (i + 1 < r.path.size())
            std::cout << "       | " << graph.edgeWeight(r.path[i], r.path[i + 1]) << "m\n";
    }
    std::cout << "\nTotal Distance: " << r.distance << "m\n"
              << "Nodes Explored: " << r.nodesExplored << "\n";
}

// One run is too fast to time reliably, so average many runs (a micro-benchmark).
double averageTimeMs(const PathFinder& finder, const Graph& g, const NodeID& s, const NodeID& d) {
    constexpr int kRuns = 2000;
    volatile std::size_t sink = 0;  // stops the compiler deleting the "unused" calls
    const auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < kRuns; ++i) sink = sink + finder.findPath(g, s, d).nodesExplored;
    return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count() / kRuns;
}

std::string joinPath(const PathResult& r) {
    if (!r.found) return "(no route)";
    std::string out;
    for (std::size_t i = 0; i < r.path.size(); ++i) out += (i ? " -> " : "") + r.path[i];
    return out;
}

void searchLocation(const Graph& graph) {
    auto id = askLocation(graph, "Search location");
    if (!id) return;
    std::cout << "\nLocation Found\n";
    const Location& loc = graph.getLocation(*id);
    std::cout << "Building: " << loc.building << "\nFloor: " << loc.floor
              << "\nType: " << toString(loc.type) << "\nCoordinates: (" << loc.x << ", " << loc.y << ")\n"
              << "Connected to:\n";
    for (const Edge& e : graph.neighbors(*id)) std::cout << "  " << e.to << " (" << e.weight << "m)\n";
    if (graph.neighbors(*id).empty()) std::cout << "  (nothing - isolated location)\n";
}

void findPath(const Graph& graph) {
    auto src = askLocation(graph, "Enter source     ");
    if (!src) return;
    auto dst = askLocation(graph, "Enter destination");
    if (!dst) return;

    std::cout << "\nSelect Algorithm:\n1. Dijkstra\n2. A*\nEnter choice: ";
    std::string choice;
    if (!std::getline(std::cin, choice)) return;
    std::unique_ptr<PathFinder> finder;  // smart pointer: freed automatically
    if (trim(choice) == "1") finder = std::make_unique<Dijkstra>();
    else if (trim(choice) == "2") finder = std::make_unique<AStar>();
    else { std::cout << "Invalid algorithm choice.\n"; return; }

    std::cout << "\nCalculating route with " << finder->name() << "...\n";
    printRoute(graph, finder->findPath(graph, *src, *dst));
}

void compareAlgorithms(const Graph& graph) {
    auto src = askLocation(graph, "Enter source     ");
    if (!src) return;
    auto dst = askLocation(graph, "Enter destination");
    if (!dst) return;

    Dijkstra dijkstra;
    AStar astar;
    const PathFinder* finders[] = {&dijkstra, &astar};

    std::cout << "\n" << std::left << std::setw(12) << "Algorithm" << std::setw(12) << "Distance"
              << std::setw(18) << "Nodes Explored" << "Avg Time\n" << std::string(55, '-') << "\n";
    PathResult results[2];
    for (int i = 0; i < 2; ++i) {
        results[i] = finders[i]->findPath(graph, *src, *dst);
        std::ostringstream dist;
        if (results[i].found) dist << std::fixed << std::setprecision(1) << results[i].distance << "m";
        else dist << "-";
        std::cout << std::left << std::setw(12) << finders[i]->name() << std::setw(12) << dist.str()
                  << std::setw(18) << results[i].nodesExplored << std::fixed << std::setprecision(4)
                  << averageTimeMs(*finders[i], graph, *src, *dst) << " ms\n";
    }
    std::cout << std::defaultfloat << "\nDijkstra path: " << joinPath(results[0])
              << "\nA* path:       " << joinPath(results[1]) << "\n";
}

}  // namespace

int main(int argc, char* argv[]) {
    const std::string dataPath = (argc > 1) ? argv[1] : DEFAULT_DATA_PATH;
    Graph graph;
    try {
        graph = loadCampus(dataPath);
    } catch (const std::exception& e) {
        std::cerr << "Error loading campus: " << e.what() << "\n";
        return 1;
    }
    for (const std::string& warning : graph.findHeuristicViolations())
        std::cerr << "Warning (A* may be inexact): " << warning << "\n";

    std::cout << "Loaded " << graph.locationCount() << " locations and " << graph.edgeCount() << " connections.\n";

    std::string choice;
    while (true) {
        std::cout << "\n================================\n        CAMPUSNAVI\n"
                     "================================\n\n1. Search Location\n2. Find Shortest Path\n"
                     "3. Compare Algorithms\n4. Exit\n\nEnter choice: ";
        if (!std::getline(std::cin, choice)) break;  // end of input (Ctrl+D)
        choice = trim(choice);
        try {
            if (choice == "1") searchLocation(graph);
            else if (choice == "2") findPath(graph);
            else if (choice == "3") compareAlgorithms(graph);
            else if (choice == "4") break;
            else std::cout << "Please enter 1-4.\n";
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << "\n";  // the program survives bad input
        }
    }
    std::cout << "Goodbye!\n";
    return 0;
}
