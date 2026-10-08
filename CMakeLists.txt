cmake_minimum_required(VERSION 3.14)
project(CampusNavi VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)   # benchmarks are meaningless in Debug
endif()

# 1) A library with everything EXCEPT main(): the app and the tests both reuse it.
add_library(campusnavi_core
  src/Location.cpp src/Graph.cpp src/PathFinder.cpp
  src/Dijkstra.cpp src/AStar.cpp src/CampusData.cpp)
target_include_directories(campusnavi_core PUBLIC include)
if(MSVC)
  target_compile_options(campusnavi_core PUBLIC /W4)
else()
  target_compile_options(campusnavi_core PUBLIC -Wall -Wextra -Wpedantic)
endif()

# 2) The command-line program.
add_executable(campusnavi src/main.cpp)
target_link_libraries(campusnavi PRIVATE campusnavi_core)
target_compile_definitions(campusnavi PRIVATE
  DEFAULT_DATA_PATH="${CMAKE_SOURCE_DIR}/data/campus.txt")

# 3) Tests, run with `ctest` or `./build/test_pathfinding`.
enable_testing()
add_executable(test_pathfinding tests/test_pathfinding.cpp)
target_link_libraries(test_pathfinding PRIVATE campusnavi_core)
target_compile_definitions(test_pathfinding PRIVATE
  CAMPUS_DATA_PATH="${CMAKE_SOURCE_DIR}/data/campus.txt")
add_test(NAME pathfinding COMMAND test_pathfinding)

# 4) Tests on the generated 100- and 300-node maps (+ benchmark printout).
add_executable(test_maps tests/test_maps.cpp)
target_link_libraries(test_maps PRIVATE campusnavi_core)
target_compile_definitions(test_maps PRIVATE
  CAMPUS_100_PATH="${CMAKE_SOURCE_DIR}/data/campus_100.txt"
  CAMPUS_300_PATH="${CMAKE_SOURCE_DIR}/data/campus_300.txt")
add_test(NAME maps COMMAND test_maps)
