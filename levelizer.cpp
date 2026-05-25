#include "levelizer.h"
#include <map>
#include <iostream>
#include <algorithm>

void levelizeNetlist(Netlist& netlist) {
    map<string, int> levelMap; //temporary lookup table, maps signal names to level, used to check if a gate's inputs are ready

    //primary inputs are always lvl 0, they have no dependencies
    for (string str : netlist.inputs) {
        levelMap[str] = 0;
    }


    bool unleveled = true;
    //keep looping until every gate has a level
    //at the start of each pass assume we're done (set to false), and then set back to true if we find any unleveled gate
    while (unleveled) {
        unleveled = false;

        for (Gate& gate : netlist.gates) {
            if (gate.level == -1) {
                unleveled = true;
                //logic for finding an unleveled gate
                bool inputMissing = false;
                for (string s : gate.inputs) {
                    if (levelMap.count(s) == 0) {
                        inputMissing = true;
                        break;
                    }
                }
                //all gates are ready, so we find the highest lvl among them
                if (!inputMissing) {
                    int maxLvl = 0;
                    for (string input : gate.inputs) {
                        if (levelMap[input] > maxLvl) maxLvl = levelMap[input];
                    }
                    gate.level = maxLvl + 1;
                    levelMap[gate.name] = gate.level;
                    //this gate's level is one above its deepest input
                    //add to the map so gates that depend on it can be leveled later
                }
            }
        }
    }


}