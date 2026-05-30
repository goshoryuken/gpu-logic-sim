#include "netlist.h"
#include "levelizer.h"
#include <map>
#include <iostream>
#include <algorithm>


vector<vector<int>> simulate(const Netlist& netlist, const map<string, int>& inputValues, int numCycles) { //new function, can do both comb and sequential logic

    vector<int> signals(netlist.signalIDs.size(), 0);
    for (auto& pair : inputValues) {
        signals[netlist.signalIDs.at(pair.first)] = pair.second;
    }

    

    vector<vector<int>> results;
    vector<int> nextState(netlist.signalIDs.size(), 0);

    for (int i = 0; i <= numCycles; i++) {
        for (const Gate& gate : netlist.gates) {
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