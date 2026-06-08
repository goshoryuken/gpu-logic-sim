#include "netlist.h"
#include <fstream>
#include <sstream>
#include <set>
#include <cctype>
using namespace std;

void replaceJunk(string& s) {
    for (char& c : s) {
        if (c == '(' || c == ')' || c == ',' || c == ';' || c == '\\' || c == '\n' || c == '\r') {
            c = ' ';
        } 
    }
    
}

void removeCommentsAndAttributes(string& s) {
    size_t start;
    // remove /* ... */ comments
    while ((start = s.find("/*")) != string::npos) {
        size_t end = s.find("*/", start);
        if (end != string::npos) {
            s.erase(start, end - start + 2);
        } else {
            break;
        } 
    }

    // remove (* ... *) attributes
    while ((start = s.find("(*")) != string::npos) {
        size_t end = s.find("*)", start);
        if (end != string::npos) {
            s.erase(start, end - start + 2);
        } else {
            break;
        }
    }
}

Netlist parseVerilog(const string& filename) {
    ifstream file(filename);
    string line;

    Netlist netlist;
    set<string> gates = {"$_AND_", "$_OR_", "$_NOT_", "$_NAND_", "$_NOR_", "$_XOR_", "$_XNOR_", "$_MUX_", "$_ANDNOT_", "$_ORNOT_"};
    map<string, string> aliases;


    while (getline(file, line, ';')) {
        removeCommentsAndAttributes(line);
        replaceJunk(line);
        stringstream ss(line);
        
        string word;
        
        if (!(ss >> word)) continue; //grabs first word

        

        if (gates.count(word)) {
            

            Gate gate;
            


            if (word == "$_AND_") gate.type = "AND";
            else if (word == "$_OR_") gate.type = "OR";
            else if (word == "$_NOT_") gate.type = "NOT";
            else if (word == "$_NAND_") gate.type = "NAND";
            else if (word == "$_NOR_") gate.type = "NOR";
            else if (word == "$_XOR_") gate.type = "XOR";
            else if (word == "$_XNOR_") gate.type = "XNOR";
            else if (word == "$_MUX_") gate.type = "MUX";
            else if (word == "$_ANDNOT_") gate.type = "ANDNOT";
            else if (word == "$_ORNOT_") gate.type = "ORNOT";

            ss >> word; //instance name, skip

            string tempA = "";
            string tempB = "";
            string tempS = "";

            //.A and .B are both inputs, .Y is outputs
            while (ss >> word) {
                if (word == ".A" || word == ".B" || word == ".S") {
                    //muxes have a simple S ? A : B order so it can be fixed, everything else gets determined in the loop.
                    if (gate.type == "MUX") {
                        if (word == ".A") {
                            ss >> word;
                            tempA = word;
                            continue;
                        } else if (word == ".B") {
                            ss >> word;
                            tempB = word;
                            continue;
                        } else if (word == ".S") {
                            ss >> word;
                            tempS = word;
                            continue;
                        }
                    } else if (gate.type == "ANDNOT" || gate.type == "ORNOT") {
                        if (word == ".A") {
                            ss >> word;
                            tempA = word;
                            continue;
                        } else if (word == ".B") {
                            ss >> word;
                            tempB = word;
                            continue;
                        }
                    } else {
                        ss >> word;
                        gate.inputs.push_back(word);
                    }
                    
                } else if (word == ".Y") {
                    ss >> word;
                    gate.name = word;
                }
            }

            if (gate.type == "MUX") {
                gate.inputs.push_back(tempA);
                gate.inputs.push_back(tempB);
                gate.inputs.push_back(tempS);
            } else if (gate.type == "ANDNOT" || gate.type == "ORNOT") {
                gate.inputs.push_back(tempA);
                gate.inputs.push_back(tempB);
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
        } else if (word == "$_DFF_P_" || word == "$_DFF_N_" || word == "$_SDFF_PP0_") { // handling DFF's
            Gate gate;
            gate.type = "DFF";
            while (ss >> word) {
                if (word == ".D") {
                    ss >> word;
                    gate.inputs.push_back(word);
                } else if (word == ".Q") {
                    ss >> word; 
                    gate.name = word;
                } else if (word == ".C" || word == ".R") {
                    ss >> word;
                } 
            }

            netlist.dffs.push_back(gate);
        } else if (word == "assign") {
            //sample line: "assign target = source"
            ss >> word; //move on to target
            string target = word;
            ss >> word; //move on to =
            ss >> word; //move on to source
            string source = word;
            aliases[target] = source;
        }
    }

    for (Gate& gate : netlist.gates) {
        if (aliases.count(gate.name)) {
            gate.name = aliases[gate.name];
        }
        for (string& s : gate.inputs) {
            if (aliases.count(s)) s = aliases[s];
        }
    }

    for (Gate& gate : netlist.dffs) {
        if (aliases.count(gate.name)) {
            gate.name = aliases[gate.name];
        }
        for (string& s : gate.inputs) {
            if (aliases.count(s)) s = aliases[s];
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
        if (netlist.signalIDs.count(netlist.inputs[i]) == 0) {
            netlist.signalIDs[netlist.inputs[i]] = id;
            id++;
        }
    }

    for (int j = 0; j < netlist.gates.size(); j++) {
        if (netlist.signalIDs.count(netlist.gates[j].name) == 0) {
            netlist.signalIDs[netlist.gates[j].name] = id;
            id++;
        }
    }

    for (int k = 0; k < netlist.dffs.size(); k++) {
        if (netlist.signalIDs.count(netlist.dffs[k].name) == 0) {
            netlist.signalIDs[netlist.dffs[k].name] = id;
            id++;
        }
    }

    for (Gate& gate : netlist.gates) {
        gate.outputID = netlist.signalIDs[gate.name];

        for (string s : gate.inputs) {
            if (netlist.signalIDs.count(s) == 0) {
                //std::cerr << "Missing signal: " << s << " in gate: " << gate.name << ". Adding to map." << std::endl;
                // Add the missing signal to the map to prevent crashes
                netlist.signalIDs[s] = id;
                id++;
            }
            gate.inputIDs.push_back(netlist.signalIDs.at(s));
        }
    }

    for (Gate& gate : netlist.dffs) {
        gate.outputID = netlist.signalIDs[gate.name];

        for (string s : gate.inputs) {
            if (netlist.signalIDs.count(s) == 0) {
                //std::cerr << "Missing signal: " << s << " in DFF: " << gate.name << ". Adding to map." << std::endl;
                // Add the missing signal to the map to prevent crashes
                netlist.signalIDs[s] = id;
                id++;
            }
            gate.inputIDs.push_back(netlist.signalIDs.at(s));
        }
    }

    for (Gate& gate : netlist.gates) {
        if (gate.type == "AND") gate.gateTypeID = AND;
        else if (gate.type == "OR") gate.gateTypeID = OR;
        else if (gate.type == "NOT") gate.gateTypeID = NOT;
        else if (gate.type == "NAND") gate.gateTypeID = NAND;
        else if (gate.type == "NOR") gate.gateTypeID = NOR;
        else if (gate.type == "XOR") gate.gateTypeID = XOR;
        else if (gate.type == "DFF") gate.gateTypeID = DFF;
        else if (gate.type == "XNOR") gate.gateTypeID = XNOR;
        else if (gate.type == "MUX") gate.gateTypeID = MUX;
        else if (gate.type == "ANDNOT") gate.gateTypeID = ANDNOT;
        else if (gate.type == "ORNOT") gate.gateTypeID = ORNOT;
    }
    for (Gate& gate : netlist.dffs) {
        if (gate.type == "AND") gate.gateTypeID = AND;
        else if (gate.type == "OR") gate.gateTypeID = OR;
        else if (gate.type == "NOT") gate.gateTypeID = NOT;
        else if (gate.type == "NAND") gate.gateTypeID = NAND;
        else if (gate.type == "NOR") gate.gateTypeID = NOR;
        else if (gate.type == "XOR") gate.gateTypeID = XOR;
        else if (gate.type == "DFF") gate.gateTypeID = DFF;
    }
}