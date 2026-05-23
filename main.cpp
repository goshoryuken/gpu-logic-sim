#include <iostream>
#include "netlist.h"
#include "parser.h"

int main() {
    Netlist netlist = parseNetlist("testcircuit.txt");

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
        cout << endl;
    }
}