#pragma once
#include "netlist.h"
#include <map>


vector<vector<int>> simulate(const Netlist& netlist, const map<string, int>& inputValues, int numCycles);
vector<int> oldSimulate(const Netlist& netlist, const map<string, int>& inputValues);
