#include "netlist.h"
#include "levelizer.h"
#include <map>
#include <iostream>
#include <algorithm>


vector<vector<int>> simulate(const Netlist& netlist, const map<string, int>& inputValues, int numCycles) { //new function, can do both comb and sequential logic

    vector<int> signals(netlist.signalIDs.size(), 0);

    //checks if the input actually exists before tying to assign it
    for (auto& pair : inputValues) {
        if (netlist.signalIDs.count(pair.first) > 0) {
            signals[netlist.signalIDs.at(pair.first)] = pair.second;
        } else {
            std::cout << "[WARNING] Input '" << pair.first << "' missing from netlist. Skipping." << std::endl;
        }
    }
    

    vector<vector<int>> results;
    vector<int> nextState(netlist.signalIDs.size(), 0);

    for (int i = 0; i < numCycles; i++) {
        for (const Gate& gate : netlist.gates) {
            int result = 0;

        if (gate.gateTypeID == AND) {

            result = 1;
            for (int id : gate.inputIDs) {
                result &= signals[id];
            }

        } else if (gate.gateTypeID == OR) {

            result = 0;
            for (int id : gate.inputIDs) {
                result |= signals[id];
            }

        } else if (gate.gateTypeID == NOT) {

            result = 0;
            result = !signals[gate.inputIDs[0]];

        } else if (gate.gateTypeID == NAND) {

            result = 1;
            for (int id : gate.inputIDs) {
                result &= signals[id];
            }
            result = !result;

        } else if (gate.gateTypeID == XOR) {

            result = 0;
            for (int id : gate.inputIDs) {
                result ^= signals[id];
            }

        } else if (gate.gateTypeID == NOR) {

            result = 0;
            for (int id : gate.inputIDs) {
                result |= signals[id];
            }
            result = !result;

        } else if (gate.gateTypeID == XNOR) {

            result = 0;
            for (int id : gate.inputIDs) {
                result ^= signals[id];
            }
            result = !result;

        } else if (gate.gateTypeID == MUX) {

            result = signals[gate.inputIDs[2]] ? signals[gate.inputIDs[1]] : signals[gate.inputIDs[0]];

        } else if (gate.gateTypeID == ANDNOT) {

            result = signals[gate.inputIDs[0]] & !signals[gate.inputIDs[1]];

        } else if (gate.gateTypeID == ORNOT) {

            result = signals[gate.inputIDs[0]] | !signals[gate.inputIDs[1]];
            
        }

        signals[gate.outputID] = result;
        }

        

        for (const Gate& gate : netlist.dffs) {
            nextState[gate.outputID] = signals[gate.inputIDs[0]];
        }

        for (const Gate& gate : netlist.dffs) {
            signals[gate.outputID] = nextState[gate.outputID];
        }

        results.push_back(signals);
    }

    return results;

}


//legacy function inefficient but here bc yea
vector<int> oldSimulate(const Netlist& netlist, const map<string, int>& inputValues) { //can only do combinational

    vector<int> signals(netlist.signalIDs.size(), 0);
    for (auto& pair : inputValues) {
        signals[netlist.signalIDs.at(pair.first)] = pair.second;
    }

    

    for (Gate gate : netlist.gates) {
        string type = gate.type;
        int result = 0;

        if (type == "AND") {

            result = 1;
            for (int id : gate.inputIDs) {
                result &= signals[id];
            }

        } else if (type == "OR") {

            result = 0;
            for (int id : gate.inputIDs) {
                result |= signals[id];
            }

        } else if (type == "NOT") {

            result = 0;
            result = !signals[gate.inputIDs[0]];

        } else if (type == "NAND") {

            result = 1;
            for (int id : gate.inputIDs) {
                result &= signals[id];
            }
            result = !result;

        } else if (type == "XOR") {

            result = 0;
            for (int id : gate.inputIDs) {
                result ^= signals[id];
            }

        } else if (type == "NOR") {

            result = 0;
            for (int id : gate.inputIDs) {
                result |= signals[id];
            }
            result = !result;

        }

        signals[gate.outputID] = result;
    }

    return signals;

}