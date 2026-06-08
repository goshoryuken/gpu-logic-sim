#pragma once
#include "netlist.h"

Netlist parseNetlist(const std::string& filename);

Netlist parseVerilog(const std::string& filename);

void assignSignalIDs(Netlist& netlist);