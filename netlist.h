#pragma once
using namespace std;
#include <iostream>
#include <string>
#include <vector>
#include <map>

struct Gate {
    string name;
    string type;
    vector<string> inputs;
    int level = -1;
    int outputID;
    vector<int> inputIDs;
};

struct Netlist {
    vector<string> inputs;
    vector<string> outputs;
    map<string, int> signalIDs;
    vector<Gate> gates;
};