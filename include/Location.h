// Location.h
// -----------------------------------------------------------------------------
// WHAT: Describes ONE place on campus (a classroom, a staircase, ...).
// WHY : Every graph node needs data. We keep that data in a plain struct so the
//       Graph class doesn't have to know anything about "buildings" or "floors".
//
// C++ CONCEPTS HERE
//   * `#pragma once`  - include guard: the header is only pasted in once per .cpp
//   * `enum class`    - a scoped, type-safe enumeration (LocationType::Lab, not Lab)
//   * `struct`        - a class whose members are public by default; good for data
//   * `const` method  - `distanceTo(...) const` promises not to modify the object
// -----------------------------------------------------------------------------
#pragma once
#include <string>

// Each floor is this many metres above the one below. A* needs it to turn
// "floor 2" into a height (z coordinate), so stairs/elevators cost something.
constexpr double FLOOR_HEIGHT_M = 4.0;

enum class LocationType { Classroom, Lab, Office, Corridor, Staircase, Elevator, Other };

// Convert "classroom" -> LocationType::Classroom (case-insensitive). Throws
// std::invalid_argument on unknown text, so bad data files fail loudly.
LocationType locationTypeFromString(const std::string& text);
std::string  toString(LocationType type);

struct Location {
    std::string  id;        // unique, e.g. "A102" or "A-Stair-1"
    std::string  building;  // "A", "B", ...
    int          floor = 1;
    LocationType type  = LocationType::Other;
    double       x = 0.0;   // metres, campus-wide coordinates
    double       y = 0.0;

    // Straight-line (Euclidean) distance in 3D: x, y and floor height.
    // This is the A* HEURISTIC. It can never overestimate the real walking
    // distance as long as every edge weight >= this value (see Graph.h).
    double distanceTo(const Location& other) const;
};
