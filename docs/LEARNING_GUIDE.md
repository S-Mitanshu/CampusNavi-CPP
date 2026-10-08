# CampusNavi Learning Guide

Read the files in this order. Each file has a header comment explaining WHAT, WHY and the C++ concepts.

## Reading order
1. `include/Location.h` - structs, `enum class`, `const` methods. Note `distanceTo`: it becomes the A* heuristic.
2. `include/Graph.h` + `src/Graph.cpp` - adjacency list, `unordered_map`, exceptions, `const&` returns.
3. `include/PathFinder.h` - abstract class, `virtual`, `= 0`, `override`. Why both algorithms share one interface.
4. `src/Dijkstra.cpp` - the core algorithm. Trace it by hand on `A101 -> A103` (3 nodes is enough).
5. `src/AStar.cpp` - compare with Dijkstra: only the heap key changes (`g` becomes `g + h`).
6. `src/CampusData.cpp` - file I/O, string splitting, error messages with line numbers.
7. `src/main.cpp` - how a thin UI uses the library; `unique_ptr`, `optional`.
8. `tests/test_pathfinding.cpp` - what "correct" means, edge cases.

## The 5 ideas to be able to explain in an interview
1. **Why a graph?** Rooms = nodes, walkable links = edges, metres = weights. Floors are just edges between stair/elevator nodes.
2. **Why an adjacency list?** Campuses are sparse: O(V+E) memory, and "neighbours of u" is O(degree).
3. **Why Dijkstra works:** with non-negative weights, the closest unfinalised node can't be improved later. That is why `addEdge` rejects negative weights.
4. **Why lazy deletion?** `priority_queue` can't decrease a key; we push duplicates and skip stale ones (`settled` check). Complexity stays O((V+E) log V).
5. **Why A* stays correct:** the heuristic must be admissible (never overestimates). Straight-line distance qualifies when every edge >= its straight-line length. `Graph::findHeuristicViolations` enforces this.

## Experiments (do these to really learn)
- Change `A101,A102,5` to `1` in `data/campus.txt`. Run: you get a heuristic warning. Why is that a problem for A*?
- In `AStar.cpp` replace `heuristic(...)` with `0`. Confirm the explored counts now equal Dijkstra's.
- Replace the heuristic with `2 * distanceTo`. Run the "agree on every pair" test. Does it still pass? (Inadmissible heuristics can return longer paths.)
- Add a third building and a 30-node map; use option 3 (Compare) and watch the node-count gap grow.
- Remove the `settled` check in Dijkstra; observe the explored count change, and explain why.
- Add `--route A101 B202` argument parsing to `main.cpp`.
- Run `./build/test_maps`, then repeat with the heuristic multiplied by 0 in `AStar.cpp`: the "A* explores fewer" gap disappears. Why?
- Open `data/campus_300.txt`, pick two far-apart rooms, and trace by hand why A* skips the other buildings.
- Edit `tools/generate_campus.py` (e.g. change `SPACING` or `k`) to build your own map, then load it with `./build/campusnavi your_map.txt`.
