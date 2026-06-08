#pragma once
#include "netlist.h"

std::vector<std::vector<int>> simulateGPU(const Netlist& netlist, const std::map<std::string, int>& inputValues, int numCycles);