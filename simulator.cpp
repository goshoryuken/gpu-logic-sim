#include "netlist.h"
#include "levelizer.h"
#include <map>
#include <iostream>
#include <algorithm>

map<string, int> simulate(Netlist& netlist, map<string, int> inputValues) {

    map<string, int> signals = inputValues;

    sort(netlist.gates.begin(), netlist.gates.end(), [](Gate& a, Gate& b) {
        return a.level < b.level;
    });

    for (Gate gate : netlist.gates) {
        string type = gate.type;
        int result = 0;

        if (type == "AND") {

            result = 1;
            for (string input : gate.inputs) {
                result &= signals[input];
            }

        } else if (type == "OR") {

            result = 0;
            for (string input : gate.inputs) {
                result |= signals[input];
            }

        } else if (type == "NOT") {

            result = 0;
            result = !signals[gate.inputs[0]];

        } else if (type == "NAND") {

            result = 1;
            for (string input : gate.inputs) {
                result &= signals[input];
            }
            result = !result;

        } else if (type == "XOR") {

            result = 0;
            for (string input : gate.inputs) {
                result ^= signals[input];
            }

        } else if (type == "NOR") {

            result = 0;
            for (string input : gate.inputs) {
                result |= signals[input];
            }
            result = !result;

        }

        signals[gate.name] = result;
    }

    return signals;

}