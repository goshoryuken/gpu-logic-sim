#pragma once
#include "netlist.h"
#include <map>


void writeVCD(const string& filename, const Netlist& netlist, const vector<vector<int>>& allSignals);