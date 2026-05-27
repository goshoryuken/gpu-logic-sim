#include "levelizer.h"
#include <map>
#include <iostream>
#include <algorithm>
#include <queue>

void levelizeNetlist(Netlist& netlist) {
    map<string, int> levelMap; //temporary lookup table, maps signal names to level, used to check if a gate's inputs are ready

    //in degree map, how many inputs each gate is still waiting on
    map<string, int> inDegree;
    for (Gate& gate : netlist.gates) {
        inDegree[gate.name] = gate.inputs.size();
    }

    //given a signal name, which gates depend on it?
    map<string, vector<string>> dependents;
    //after this loop concludes, dependents["g1"] for example would give a list of every gate that uses g1 as an input.
    for (Gate& gate : netlist.gates) {
        for (string input : gate.inputs) {
            dependents[input].push_back(gate.name);
        }
    }

    queue<string> q;
    //primary inputs are always lvl 0, they have no dependencies
    for (string str : netlist.inputs) {
        q.push(str);
        levelMap[str] = 0;
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
            for(string s : dependents[front]) {
                inDegree[s]--;
                levelMap[s] = max(levelMap[s], levelMap[front] + 1);
                //if the degree hits zero, all the inputs are ready, assign the level and push to the queue
                if (inDegree[s] <= 0) {
                    q.push(s);
                }
            }
        }
    }
    //then loop thru the gates to assign the correct level to each
    for (Gate& gate : netlist.gates) {
        gate.level = levelMap[gate.name];
    }

}