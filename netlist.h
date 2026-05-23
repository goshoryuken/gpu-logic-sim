#include <iostream>
#include <string>
#include <vector>
using namespace std;
#pragma once

struct Gate {
    string name;
    string type;
    vector<string> inputs;
};

struct Netlist {
    vector<string> inputs;
    vector<string> outputs;
    vector<Gate> gates;
};