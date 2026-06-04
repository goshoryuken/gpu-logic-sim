#include <iostream>
#include "netlist.h"
#include "parser.h"
#include "levelizer.h"
#include <map>
#include "simulator.h"
#include "vcd.h"
#include "gpu_simulator.h"

int main() {
    try {

        Netlist netlist = parseVerilog("circuit_synth.v");

    assignSignalIDs(netlist);
    levelizeNetlist(netlist);
    map<string, int> inputValues;
    inputValues["clk"] = 0;
    inputValues["rst"] = 0;
    vector<vector<int>> results = simulate(netlist, inputValues, 3);
    vector<vector<int>> resultsGPU = simulateGPU(netlist, inputValues, 3);
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

    //finally testing
    printf("\ncpu output!!!\n");
    for (int i = 0; i < results.size(); i++) {
        printf("cycle %d: count = %d%d%d%d\n", i,
            results[i][netlist.signalIDs["count[3]"]],
            results[i][netlist.signalIDs["count[2]"]],
            results[i][netlist.signalIDs["count[1]"]],
            results[i][netlist.signalIDs["count[0]"]]
        );
    }

    printf("\ngpu output!!!\n");
    for (int i = 0; i < resultsGPU.size(); i++) {
        printf("cycle %d: count = %d%d%d%d\n", i,
            resultsGPU[i][netlist.signalIDs["count[3]"]],
            resultsGPU[i][netlist.signalIDs["count[2]"]],
            resultsGPU[i][netlist.signalIDs["count[1]"]],
            resultsGPU[i][netlist.signalIDs["count[0]"]]
        );
    }

    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << std::endl;
    }
    
}
