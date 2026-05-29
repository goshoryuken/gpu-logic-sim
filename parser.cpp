#include "netlist.h"
#include <fstream>
#include <sstream>
#include <set>
#include <cctype>

void replaceJunk(string& s) {
    for (char& c : s) {
        if (c == '(' || c == ')' || c == ',' || c == ';') c = ' ';
    }
    
}

Netlist parseVerilog(const string& filename) {
    ifstream file(filename);
    string line;

    Netlist netlist;
    set<string> gates = {"and", "or", "not", "nand", "nor", "xor"};

    while (getline(file, line)) {
        replaceJunk(line);
        stringstream ss(line);
        
        string word;
        
        ss >> word; //grabs first word

        if (gates.count(word)) {
            

            Gate gate;
            string name = word;

            //convert uppercase
            for (auto& c: name) {
                c = std::toupper(static_cast<unsigned char>(c));
            }

            gate.type = name;

            ss >> word; //instance name, skip
            ss >> word; //now on gate name

            gate.name = word;

            while (ss >> word) { //reads the rest of the line to find the inputs;
                gate.inputs.push_back(word);
            }

            netlist.gates.push_back(gate);

        } else if (word == "input") {
            while (ss >> word) {
                if (word == "logic") continue;
                netlist.inputs.push_back(word);
            }
        } else if (word == "output") {
            while (ss >> word) {
                if (word == "logic") continue;
                netlist.outputs.push_back(word);
            }
        } else if (word == "$_DFF_P_" || word == "$_DFF_N_") { // handling DFF's
            Gate gate;
            gate.type = "DFF";
            while (ss >> word) {
                if (word == ".D") {
                    ss >> word;
                    gate.inputs.push_back(word);
                } else if (word == ".Q") {
                    ss >> word; 
                    gate.name = word;
                } else if (word == ".C") {
                    ss >> word;
                }
            }

            netlist.dffs.push_back(gate);
        }
    }
    return netlist;
}

Netlist parseNetlist(const string& filename) {
    ifstream file(filename);
    string line;

    Netlist netlist;

    while (getline(file, line)) {
        stringstream ss(line);
        string word;
        
        ss >> word; //grabs first word

        if (word == "INPUT") {
            string name;
            ss >> name; //gets the word after "INPUT"
            netlist.inputs.push_back(name);
        } else if (word == "OUTPUT") {
            string name;
            ss >> name; //gets the word after OUTPUT
            netlist.outputs.push_back(name);
        } else if (word == "GATE"){
            Gate gate;
            string name; 
            ss >> name; //moves onto the gate's name
            gate.name = name;
            ss >> name; //moves onto the gate's type
            gate.type = name;
            
            string input; //the rest of the inputs

            while (ss >> input) { //reads the rest of the line to find the inputs;
                gate.inputs.push_back(input);
            }

            netlist.gates.push_back(gate);
        }
    }
    return netlist;
}

void assignSignalIDs(Netlist& netlist) {
    int id = 0;
    for (int i = 0; i < netlist.inputs.size(); i++) {
        netlist.signalIDs[netlist.inputs[i]] = id;
        id++;
    }

    for (int j = 0; j < netlist.gates.size(); j++) {
        netlist.signalIDs[netlist.gates[j].name] = id;
        id++;
    }

    for (int k = 0; k < netlist.dffs.size(); k++) {
        netlist.signalIDs[netlist.dffs[k].name] = id;
        id++;
    }

    for (Gate& gate : netlist.gates) {
        gate.outputID = netlist.signalIDs[gate.name];

        for (string s : gate.inputs) {
            gate.inputIDs.push_back(netlist.signalIDs[s]);
        }
    }

    for (Gate& gate : netlist.dffs) {
        gate.outputID = netlist.signalIDs[gate.name];

        for (string s : gate.inputs) {
            gate.inputIDs.push_back(netlist.signalIDs[s]);
        }
    }
}