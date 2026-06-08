#include "levelizer.h"
#include <map>
#include <iostream>
#include <algorithm>
#include <queue>
#include <set>
using namespace std;

void levelizeNetlist(Netlist& netlist) {
    map<string, int> levelMap; //temporary lookup table, maps signal names to level, used to check if a gate's inputs are ready

    //replaced with a vector because using a map overwrites gates that share the same output name
    //now used an int vector mapped exactly to the gate's index in netlist.gates
    vector<int> inDegree(netlist.gates.size());
    for (int i = 0; i < netlist.gates.size(); i++) {
        inDegree[i] = netlist.gates[i].inputs.size();
    }

    //given a signal name, which gates depend on it?
    map<string, vector<int>> dependents;
    //after this loop concludes, dependents["g1"] for example would give a list of every gate that uses g1 as an input.
    //UPDATE* since i changed inDegree to use numbers, the dependents map must save the gate's INDEX number instead of the name.
    for (int i = 0; i < netlist.gates.size(); i++) {
        for (string input : netlist.gates[i].inputs) {
            dependents[input].push_back(i);
        }
    }

    queue<string> q;
    set<string> readySignals; //track what we've already pushed to prevent dupes
    //primary inputs are always lvl 0, they have no dependencies
    for (string str : netlist.inputs) {
        q.push(str);
        levelMap[str] = 0;
        readySignals.insert(str);
    }

    for (Gate& gate : netlist.dffs) {
        if (readySignals.count(gate.name) == 0) {
            q.push(gate.name);
            levelMap[gate.name] = 0;
            readySignals.insert(gate.name);
        }
    }

    //catch constants
    set<string> drivenSignals;
    for (Gate& gate : netlist.gates) drivenSignals.insert(gate.name);
    for (Gate& gate : netlist.dffs) drivenSignals.insert(gate.name);

    for (Gate& gate : netlist.gates) {
        for (string input : gate.inputs) {
            //if the input isnt driven by a gate, and hasn't been pushed yet
            if (drivenSignals.count(input) == 0 && readySignals.count(input) == 0) {
                q.push(input);
                levelMap[input] = 0;
                readySignals.insert(input);
            }
        }
    }



    while (!q.empty()) {
        //grab the signal from the front
        string front = q.front();
        //remove
        q.pop();
        

        //check to see if the signal is in dependents
        if (dependents.find(front) != dependents.end()) {
            //then loop thru all the gates in that signal
            //decrement the degrees, find the correct level assignment
            for (int gateIndex : dependents[front]) {
                inDegree[gateIndex]--;
                string outName = netlist.gates[gateIndex].name;
                
                levelMap[outName] = max(levelMap[outName], levelMap[front] + 1);

                if (inDegree[gateIndex] == 0) {

                    //check readySignals to prevent dupes from bein pushed
                    if (readySignals.count(outName) == 0) {
                        q.push(outName);
                        readySignals.insert(outName);
                    }
                }
            }
        }
    }

    
    int stuck = 0;
    for (int i = 0; i < netlist.gates.size(); i++) {
        if (inDegree[i] > 0) {
            stuck++;
            if (stuck <= 5) { // Print the first 5 stuck gates to help debug
                std::cerr << "stuck gate: " << netlist.gates[i].name << " inDegree=" << inDegree[i] << " inputs:";
                for (auto& inp : netlist.gates[i].inputs) std::cerr << " " << inp;
                std::cerr << std::endl;
            }
        }
    }

    // If ANY gates are stuck, throw the error
    if (stuck > 0) {
        std::cerr << "Total stuck: " << stuck << " / " << netlist.gates.size() << std::endl;
        throw std::runtime_error("combinational loop detected");
    }
    
    //then loop thru the gates to assign the correct level to each
    for (Gate& gate : netlist.gates) {
        gate.level = levelMap[gate.name];
    }

    
    //sort the circuit
    sort(netlist.gates.begin(), netlist.gates.end(), [](Gate& a, Gate& b) {
        return a.level < b.level;
    });

}