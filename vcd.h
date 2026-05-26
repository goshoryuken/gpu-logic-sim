#pragma once
#include "netlist.h"
#include <map>


void writeVCD(string filename, Netlist& netlist, vector<map<string,int>> allSignals);