#include <iostream>
#include "netlist.h"
#include "parser.h"
#include "levelizer.h"
#include <map>
#include "simulator.h"
#include "vcd.h"
#include "gpu_simulator.h"
#include <chrono> // Added for high-resolution benchmarking

int main() {
    try {
        // parse 
        Netlist netlist = parseVerilog("cpu_synth.v");
        
        assignSignalIDs(netlist);
        
        std::cout << "Gates: " << netlist.gates.size() << " DFFs: " << netlist.dffs.size() << " Signals: " << netlist.signalIDs.size() << std::endl;
        levelizeNetlist(netlist);
        std::cout << "IDs assigned" << std::endl;

        map<string, int> inputValues;
        inputValues["clk"] = 0;
        inputValues["reset"] = 0;
        // Force Yosys constants to their correct boolean values
        if (netlist.signalIDs.count("1'h1")) inputValues["1'h1"] = 1;
        if (netlist.signalIDs.count("1'h0")) inputValues["1'h0"] = 0;
        if (netlist.signalIDs.count("1'b1")) inputValues["1'b1"] = 1;
        if (netlist.signalIDs.count("1'b0")) inputValues["1'b0"] = 0;

        // benchmark parameters
        int cycleCount = 100000; // Cranked up to give the GPU a real workload

        //cpu benchmark
        std::cout << "starting CPU Simulation for " << cycleCount << " cycles" << std::endl;
        auto startCPU = std::chrono::high_resolution_clock::now();

        std::cout << "Starting CPU sim" << std::endl;
        vector<vector<int>> results = simulate(netlist, inputValues, cycleCount);
        
        auto endCPU = std::chrono::high_resolution_clock::now();

        //gpu benchmark
        std::cout << "Starting GPU Simulation for " << cycleCount << " cycles..." << std::endl;
        auto startGPU = std::chrono::high_resolution_clock::now();
        
        vector<vector<int>> resultsGPU = simulateGPU(netlist, inputValues, cycleCount);
        
        auto endGPU = std::chrono::high_resolution_clock::now();

        // calc and print results
        auto cpuTime = std::chrono::duration_cast<std::chrono::milliseconds>(endCPU - startCPU).count();
        auto gpuTime = std::chrono::duration_cast<std::chrono::milliseconds>(endGPU - startGPU).count();

        std::cout << "\n=== BENCHMARK RESULTS ===" << std::endl;
        std::cout << "CPU Time: " << cpuTime << " ms" << std::endl;
        std::cout << "GPU Time: " << gpuTime << " ms" << std::endl;

        // output VCD for waveform viewing
        writeVCD("output.vcd", netlist, results);
        
        /* ===================================================================
        PRINT LOOPS COMMENTED OUT FOR BENCHMARKING
        (takes hella time and will prob skew the results)
        
        
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

        printf("DFFs found: %d\n", netlist.dffs.size());

        printf("\ncpu output!!!\n");
        for (int i = 0; i < 3; i++) { // Just printing first 3 to check correctness
            printf("cycle %d: count = %d%d%d%d\n", i,
                results[i][netlist.signalIDs["count[3]"]],
                results[i][netlist.signalIDs["count[2]"]],
                results[i][netlist.signalIDs["count[1]"]],
                results[i][netlist.signalIDs["count[0]"]]
            );
        }

        printf("\ngpu output!!!\n");
        for (int i = 0; i < 3; i++) {
            printf("cycle %d: count = %d%d%d%d\n", i,
                resultsGPU[i][netlist.signalIDs["count[3]"]],
                resultsGPU[i][netlist.signalIDs["count[2]"]],
                resultsGPU[i][netlist.signalIDs["count[1]"]],
                resultsGPU[i][netlist.signalIDs["count[0]"]]
            );
        }
        */

    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
    }
    
    return 0;
}