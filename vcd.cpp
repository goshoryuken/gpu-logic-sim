#include "vcd.h"
#include <iostream>
#include <fstream>

void writeVCD(string filename, Netlist& netlist, vector<map<string,int>> allSignals) {
    ofstream file(filename);

    file << "$timescale 1ns $end\n";

    for (string s : netlist.inputs) {
        file << "$var wire 1 " << s << " " << s << " $end\n";
    }

    for (Gate& g : netlist.gates) {
        file << "$var wire 1 " << g.name << " " << g.name << " $end\n";
    }

    file << "$enddefinitions $end\n";

    for (int i = 0; i < allSignals.size(); i++) {

        file << "#" << i << "\n";

        for (string s : netlist.inputs) {
            file << allSignals[i][s] << s << "\n";
        }

        for (Gate& g : netlist.gates) {
            file << allSignals[i][g.name] << g.name << "\n";
        }
    }

}