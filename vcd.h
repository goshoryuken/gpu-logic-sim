#pragma once
#include "netlist.h"
#include <map>


void writeVCD(const std::string& filename, const Netlist& netlist, const std::vector<std::vector<int>>& allSignals);