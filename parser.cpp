#include "netlist.h"
#include <fstream>
#include <sstream>


Netlist parseNetlist(string filename) {
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