# CampusNavi

A C++17 campus navigation engine. The campus is a **weighted graph**; **Dijkstra** and **A\*** find
and compare shortest routes, including routes that cross floors via stairs and elevators.

## Build & run

```bash
# Option 1: CMake
cmake -S . -B build && cmake --build build
./build/campusnavi            # interactive CLI
ctest --test-dir build        # or ./build/test_pathfinding

# Option 2: no CMake
sh build.sh
./build/campusnavi
./build/test_pathfinding
./build/test_maps            # 100/300-node maps + benchmark printout
```

`./build/campusnavi data/campus_300.txt` loads a different map (try menu option 3 with `Gate-A` and `I301`).

## Layout

| File | Role |
|---|---|
| `include/Location.h`, `src/Location.cpp` | One campus place + 3D straight-line distance (the A* heuristic) |
| `include/Graph.h`, `src/Graph.cpp` | Adjacency-list graph, validation |
| `include/PathFinder.h`, `src/PathFinder.cpp` | Abstract algorithm interface, result struct, path reconstruction |
| `include/Dijkstra.h`, `src/Dijkstra.cpp` | Dijkstra with a min-heap |
| `include/AStar.h`, `src/AStar.cpp` | A\* (Dijkstra + heuristic) |
| `include/CampusData.h`, `src/CampusData.cpp` | Parses `data/campus.txt` into a Graph |
| `src/main.cpp` | CLI menu only, no algorithm logic |
| `tests/test_pathfinding.cpp` | 12 tests, 400+ checks, no external framework |
| `data/campus.txt` | Small sample campus (14 locations, 2 floors, 2 buildings) |
| `data/campus_100.txt` | Medium campus: 100 locations, 131 connections, 4 buildings, 2-3 floors |
| `data/campus_300.txt` | Large campus: 300 locations, 393 connections, 9 buildings, 3-4 floors |
| `tools/generate_campus.py` | Reproducible generator for the two big maps (`python3 tools/generate_campus.py`) |
| `tests/test_maps.cpp` | Validates big maps, checks Dijkstra == A* on 300 random routes each, prints benchmark |

**New here? Read [`docs/LEARNING_GUIDE.md`](docs/LEARNING_GUIDE.md) first.**

## Complexity

Dijkstra with adjacency list + binary heap: O((V + E) log V). A\* has the same worst case; in practice
it settles fewer nodes when the heuristic is informative. Time is measured, not assumed: on this
14-node map A\* settles fewer nodes (e.g. 10 vs 12 for A101 to B202) but is not faster in wall-clock
time, because the graph is tiny and the heuristic costs a little per node. Try a larger map to see the gap.

## Benchmark (300 random routes per map, -O2, one machine; your numbers will differ)

| Map | Nodes explored: Dijkstra | A* | Avg time: Dijkstra | A* |
|---|---|---|---|---|
| campus_100 | 51.1 | 25.9 (49% fewer) | 0.035 ms | 0.024 ms |
| campus_300 | 144.0 | 55.4 (62% fewer) | 0.099 ms | 0.055 ms |

The gap grows with map size, which is the whole point of the heuristic. Both always return the same distance.

## About the big maps

They are **synthetic, not real survey data**: generated with realistic campus structure (hallways,
rooms, stairs, elevators, quad paths, skybridges) so they fit the project's format exactly. A real
dataset (e.g. OpenStreetMap) would need a converter script; that is a possible future step.
