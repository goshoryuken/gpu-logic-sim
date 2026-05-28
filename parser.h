#pragma once
#include "netlist.h"

Netlist parseNetlist(const string& filename);

Netlist parseVerilog(const string& filename);

void assignSignalIDs(Netlist& netlist);