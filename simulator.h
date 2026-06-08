#pragma once
#include "netlist.h"
#include <map>


std::vector<std::vector<int>> simulate(const Netlist& netlist, const std::map<std::string, int>& inputValues, int numCycles);
std::vector<int> oldSimulate(const Netlist& netlist, const std::map<std::string, int>& inputValues);
