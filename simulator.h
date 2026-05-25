#pragma once
#include "netlist.h"
#include <map>

map<string, int> simulate(Netlist& netlist, map<string, int> inputValues);
