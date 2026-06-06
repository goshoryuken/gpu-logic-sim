#pragma once
using namespace std;
#include <iostream>
#include <string>
#include <vector>
#include <map>

enum GateType {AND, OR, NOT, NAND, NOR, XOR, DFF, XNOR, MUX};

struct Gate {
    string name;
    string type;
    vector<string> inputs;
    int level = -1;
    int outputID;
    vector<int> inputIDs; 
    int gateTypeID;
};

struct Netlist {
    vector<string> inputs;
    vector<string> outputs;
    map<string, int> signalIDs;
    vector<Gate> gates;
    vector<Gate> dffs;
};

