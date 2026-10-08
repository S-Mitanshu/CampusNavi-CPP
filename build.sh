#!/bin/sh
# Fallback build without CMake:  sh build.sh   (then ./build/campusnavi, ./build/test_pathfinding, ./build/test_maps)
set -e
mkdir -p build
SRC="src/Location.cpp src/Graph.cpp src/PathFinder.cpp src/Dijkstra.cpp src/AStar.cpp src/CampusData.cpp"
FLAGS="-std=c++17 -O2 -Wall -Wextra -Wpedantic -Iinclude"
g++ $FLAGS $SRC src/main.cpp -DDEFAULT_DATA_PATH="\"$PWD/data/campus.txt\"" -o build/campusnavi
g++ $FLAGS $SRC tests/test_pathfinding.cpp -DCAMPUS_DATA_PATH="\"$PWD/data/campus.txt\"" -o build/test_pathfinding
g++ $FLAGS $SRC tests/test_maps.cpp -DCAMPUS_100_PATH="\"$PWD/data/campus_100.txt\"" -DCAMPUS_300_PATH="\"$PWD/data/campus_300.txt\"" -o build/test_maps
echo "Built. Run ./build/test_pathfinding, ./build/test_maps, then ./build/campusnavi [map file]"
