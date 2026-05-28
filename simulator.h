#pragma once
#include "netlist.h"
#include <map>

vector<int> simulate(const Netlist& netlist, const map<string, int>& inputValues);
