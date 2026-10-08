#include "Location.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <stdexcept>

namespace {
// Anonymous namespace = "private to this .cpp file".
std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}
}  // namespace

LocationType locationTypeFromString(const std::string& text) {
    const std::string t = toLower(text);
    if (t == "classroom") return LocationType::Classroom;
    if (t == "lab")       return LocationType::Lab;
    if (t == "office")    return LocationType::Office;
    if (t == "corridor")  return LocationType::Corridor;
    if (t == "staircase") return LocationType::Staircase;
    if (t == "elevator")  return LocationType::Elevator;
    if (t == "other")     return LocationType::Other;
    throw std::invalid_argument("Unknown location type: '" + text + "'");
}

std::string toString(LocationType type) {
    switch (type) {
        case LocationType::Classroom: return "Classroom";
        case LocationType::Lab:       return "Lab";
        case LocationType::Office:    return "Office";
        case LocationType::Corridor:  return "Corridor";
        case LocationType::Staircase: return "Staircase";
        case LocationType::Elevator:  return "Elevator";
        case LocationType::Other:     return "Other";
    }
    return "Other";
}

double Location::distanceTo(const Location& other) const {
    const double dx = x - other.x;
    const double dy = y - other.y;
    const double dz = (floor - other.floor) * FLOOR_HEIGHT_M;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}
