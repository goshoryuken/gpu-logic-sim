#include <iostream>
#include "netlist.h"
#include "parser.h"
#include "levelizer.h"
#include <map>
#include "simulator.h"
#include "vcd.h"

int main() {
    Netlist netlist = parseVerilog("circuit.sv");

    assignSignalIDs(netlist);
    levelizeNetlist(netlist);
    map<string, int> inputValues;
    inputValues["clk"] = 0;
    vector<vector<int>> results = simulate(netlist, inputValues, 3);
    writeVCD("output.vcd", netlist, results);
    
    
    for (int i = 0; i < netlist.inputs.size(); i++) {
        printf("%s\n", netlist.inputs[i].c_str());
    }

    for (int j = 0; j < netlist.outputs.size(); j++) {
        printf("%s\n", netlist.outputs[j].c_str());
    }

    for (int k = 0; k < netlist.gates.size(); k++) {
        printf("%s, %s\n", netlist.gates[k].name.c_str(), netlist.gates[k].type.c_str());

        for (string inp : netlist.gates[k].inputs) {
            cout << inp << " ";
        }
        
        printf("%s, %s, level %d\n", netlist.gates[k].name.c_str(), netlist.gates[k].type.c_str(), netlist.gates[k].level);

        cout << endl;
    }


    //testing VCD output
    vector<int> lastSignals = results.back();
    for (string out : netlist.outputs) {
        printf("%s = %d\n", out.c_str(), lastSignals[netlist.signalIDs[out]]);
    }

    printf("DFFs found: %d\n", netlist.dffs.size());

    
}