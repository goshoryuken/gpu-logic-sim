#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>

enum GateType {AND, OR, NOT, NAND, NOR, XOR, DFF, XNOR, MUX, ANDNOT, ORNOT};

struct Gate {
    std::string name;
    std::string type;
    std::vector<std::string> inputs;
    int level = -1;
    int outputID;
    std::vector<int> inputIDs; 
    int gateTypeID;
};

struct Netlist {
    std::vector<std::string> inputs;
    std::vector<std::string> outputs;
    std::map<std::string, int> signalIDs;
    std::vector<Gate> gates;
    std::vector<Gate> dffs;
};