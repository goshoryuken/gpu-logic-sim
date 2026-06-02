#pragma once
#include "netlist.h"

vector<vector<int>> simulateGPU(const Netlist& netlist, const map<string, int>& inputValues, int numCycles);