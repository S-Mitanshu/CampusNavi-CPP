#include "CampusData.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace {

std::string trim(const std::string& s) {
    const auto first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    const auto last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}

std::vector<std::string> splitCsv(const std::string& line) {
    std::vector<std::string> fields;
    std::stringstream stream(line);
    std::string field;
    while (std::getline(stream, field, ',')) fields.push_back(trim(field));
    return fields;
}

enum class Section { None, Locations, Edges };

}  // namespace

Graph loadCampus(const std::string& path) {
    std::ifstream file(path);
    if (!file) throw std::runtime_error("Cannot open campus file: " + path);

    Graph graph;
    Section section = Section::None;
    std::string raw;
    int lineNo = 0;

    while (std::getline(file, raw)) {
        ++lineNo;
        const std::string line = trim(raw);
        if (line.empty() || line[0] == '#') continue;  // skip blanks and comments

        try {
            if (line == "[LOCATIONS]") { section = Section::Locations; continue; }
            if (line == "[EDGES]")     { section = Section::Edges;     continue; }

            const auto f = splitCsv(line);
            if (section == Section::Locations) {
                if (f.size() != 6) throw std::runtime_error("expected 6 fields: id,building,floor,type,x,y");
                Location loc;
                loc.id       = f[0];
                loc.building = f[1];
                loc.floor    = std::stoi(f[2]);
                loc.type     = locationTypeFromString(f[3]);
                loc.x        = std::stod(f[4]);
                loc.y        = std::stod(f[5]);
                graph.addLocation(loc);
            } else if (section == Section::Edges) {
                if (f.size() != 3) throw std::runtime_error("expected 3 fields: idA,idB,weight");
                graph.addEdge(f[0], f[1], std::stod(f[2]));
            } else {
                throw std::runtime_error("data found before a [LOCATIONS] or [EDGES] header");
            }
        } catch (const std::exception& e) {
            // Re-throw with file/line context. std::stoi/stod throw their own
            // exception types for non-numbers; this unifies them all.
            throw std::runtime_error(path + ":" + std::to_string(lineNo) + ": " + e.what());
        }
    }
    return graph;
}
