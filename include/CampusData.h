// CampusData.h
// -----------------------------------------------------------------------------
// WHAT: Reads a campus text file and builds a Graph (PRD 4.2: nothing hardcoded).
// FILE FORMAT (see data/campus.txt):
//     # comment
//     [LOCATIONS]
//     id,building,floor,type,x,y
//     [EDGES]
//     idA,idB,weight
// Errors throw std::runtime_error with "file:line: message" so you can fix data.
// -----------------------------------------------------------------------------
#pragma once
#include <string>

#include "Graph.h"

Graph loadCampus(const std::string& path);
