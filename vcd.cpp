#include "vcd.h"
#include <iostream>
#include <fstream>

void writeVCD(const string& filename, const Netlist& netlist, const vector<vector<int>>& allSignals) {
    ofstream file(filename);

    file << "$timescale 1ns $end\n";

    for (string s : netlist.inputs) {
        file << "$var wire 1 " << s << " " << s << " $end\n";
    }

    for (const Gate& g : netlist.gates) {
        file << "$var wire 1 " << g.name << " " << g.name << " $end\n";
    }

    for (const Gate& g : netlist.dffs) {
        file << "$var wire 1 " << g.name << " " << g.name << " $end\n";
    }

    file << "$enddefinitions $end\n";

    for (int i = 0; i < allSignals.size(); i++) {

        file << "#" << i << "\n";

        for (string s : netlist.inputs) {
            file << allSignals[i][netlist.signalIDs.at(s)] << s << "\n";
        }

        for (const Gate& g : netlist.gates) {
            file << allSignals[i][g.outputID] << g.name << "\n";
        }

        for (const Gate& g : netlist.dffs) {
            file << allSignals[i][g.outputID] << g.name << "\n";
        }
    }

}