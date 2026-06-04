#include <cuda_runtime.h>
#include "netlist.h"
#include <map>
#include <vector>
#include <iostream>

#define CUDA_CHECK(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, const char *file, int line, bool abort=true) {
   if (code != cudaSuccess) {
      std::fprintf(stderr, "GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
      if (abort) exit(code);
   }
}

__global__ void gpu_simulator(int* gateTypes, int* outputIDs, int* numInputs, int* inputOffsets, int* inputIDs, int* signals, int startGate, int numGates) {
    //thread index within this specific launch
    int i = threadIdx.x + blockIdx.x * blockDim.x;

    if (i >= numGates) {
        return;
    } else {
        //calculate the actual global index of the gate we simulate
        int gateIdx = startGate + i;
        int result = 0;
        int type = gateTypes[gateIdx];

        if (type == AND) {

            result = 1;
            for (int k = 0; k < numInputs[gateIdx]; k++) {
                result &= signals[inputIDs[inputOffsets[gateIdx] + k]];
            }

        } else if (type == OR) {

            result = 0;
            for (int k = 0; k < numInputs[gateIdx]; k++) {
                result |= signals[inputIDs[inputOffsets[gateIdx] + k]];
            }

        } else if (type == NOT) {

            result = 0;
            result = !signals[inputIDs[inputOffsets[gateIdx]]];

        } else if (type == NAND) {

            result = 1;
            for (int k = 0; k < numInputs[gateIdx]; k++) {
                result &= signals[inputIDs[inputOffsets[gateIdx] + k]];
            }
            result = !result;

        } else if (type == XOR) {

            result = 0;
            for (int k = 0; k < numInputs[gateIdx]; k++) {
                result ^= signals[inputIDs[inputOffsets[gateIdx] + k]];
            }

        } else if (type == NOR) {

            result = 0;
            for (int k = 0; k < numInputs[gateIdx]; k++) {
                result |= signals[inputIDs[inputOffsets[gateIdx] + k]];
            }
            result = !result;
        } else if (type == XNOR) {

            result = 0;
            for (int k = 0; k < numInputs[gateIdx]; k++) {
                result ^= signals[inputIDs[inputOffsets[gateIdx] + k]];
            }
            result = !result;
        }
        
        signals[outputIDs[gateIdx]] = result;
    }
}

vector<vector<int>> simulateGPU(const Netlist& netlist, const map<string, int>& inputValues, int numCycles) {
    vector<int> gateTypes;
    vector<int> outputIDs;
    vector<int> numInputs;

    for (const Gate& gate : netlist.gates) {
        gateTypes.push_back(gate.gateTypeID);
        outputIDs.push_back(gate.outputID);
        numInputs.push_back(gate.inputIDs.size());
    }

    vector<int> inputOffsets;

    int offset = 0;
    for (const Gate& gate : netlist.gates) {
        inputOffsets.push_back(offset);
        offset += gate.inputIDs.size();
    }

    vector<int> inputIDs;
    for (const Gate& gate : netlist.gates) {
        for (int input : gate.inputIDs) {
            inputIDs.push_back(input);
        }
    }

    vector<int> signals(netlist.signalIDs.size(), 0);
    for (auto& pair : inputValues) {
        signals[netlist.signalIDs.at(pair.first)] = pair.second;
    }

    vector<int> levelStarts;
    vector<int> levelEnds;

    if (!netlist.gates.empty()) {
        int currentLevel = netlist.gates[0].level;
        levelStarts.push_back(0);
        for (int i = 0; i < netlist.gates.size(); i++) {
            if (netlist.gates[i].level != currentLevel) {
                levelEnds.push_back(i);
                currentLevel = netlist.gates[i].level;
                levelStarts.push_back(i);
            }
        }
        levelEnds.push_back(netlist.gates.size());
    }

    int threadsPerBlock = 256;
    int numBlocks = (netlist.gates.size() + 255) / 256;

    int* device_types = nullptr;
    int* device_outputIDs = nullptr;
    int* device_inputIDs = nullptr;
    int* device_offsets = nullptr;
    int* device_signals = nullptr;
    int* device_inputs = nullptr;

    //for gateTypes
    CUDA_CHECK(cudaMalloc((void**)&device_types, netlist.gates.size() * sizeof(int))); 
    CUDA_CHECK(cudaMemcpy(device_types, gateTypes.data(), netlist.gates.size() * sizeof(int), cudaMemcpyHostToDevice));

    //for outputIDs

    CUDA_CHECK(cudaMalloc((void**)&device_outputIDs, netlist.gates.size() * sizeof(int))); 
    CUDA_CHECK(cudaMemcpy(device_outputIDs, outputIDs.data(), netlist.gates.size() * sizeof(int), cudaMemcpyHostToDevice));

    //for numInputs

    CUDA_CHECK(cudaMalloc((void**)&device_inputs, netlist.gates.size() * sizeof(int))); 
    CUDA_CHECK(cudaMemcpy(device_inputs, numInputs.data(), netlist.gates.size() * sizeof(int), cudaMemcpyHostToDevice));

    //for inputOffsets

    CUDA_CHECK(cudaMalloc((void**)&device_offsets, netlist.gates.size() * sizeof(int))); 
    CUDA_CHECK(cudaMemcpy(device_offsets, inputOffsets.data(), netlist.gates.size() * sizeof(int), cudaMemcpyHostToDevice));

    //for inputIDs

    CUDA_CHECK(cudaMalloc((void**)&device_inputIDs, inputIDs.size() * sizeof(int))); 
    CUDA_CHECK(cudaMemcpy(device_inputIDs, inputIDs.data(), inputIDs.size() * sizeof(int), cudaMemcpyHostToDevice));

    //for signals

    CUDA_CHECK(cudaMalloc((void**)&device_signals, netlist.signalIDs.size() * sizeof(int)));
    

    //deploy kernel, clock cycle loop, dff handling, results collection

    int numGates = netlist.gates.size();

    vector<int> nextState(netlist.signalIDs.size(), 0);
    vector<vector<int>> results;

    for (int i = 0; i < numCycles; i++) {

        //copying signals to GPU
        CUDA_CHECK(cudaMemcpy(device_signals, signals.data(), netlist.signalIDs.size() * sizeof(int), cudaMemcpyHostToDevice));

        for (int lvl = 0; lvl < levelStarts.size(); lvl++) {
            int startGate = levelStarts[lvl];
            int numGatesInLevel = levelEnds[lvl] - startGate;

            int blocks = (numGatesInLevel + 255) / 256;

            //Launch Kernel
            gpu_simulator<<<blocks, threadsPerBlock>>>(device_types, device_outputIDs, device_inputs, device_offsets, device_inputIDs, device_signals, startGate, numGatesInLevel);

            //ensures kernel finishes before results are copied back
            CUDA_CHECK(cudaDeviceSynchronize());
        }

        //copy signals back
        CUDA_CHECK(cudaMemcpy(signals.data(), device_signals, netlist.signalIDs.size() * sizeof(int), cudaMemcpyDeviceToHost));
        
        //update dffs
        for (const Gate& gate : netlist.dffs) {
            nextState[gate.outputID] = signals[gate.inputIDs[0]];
        }

        for (const Gate& gate : netlist.dffs) {
            signals[gate.outputID] = nextState[gate.outputID];
        }

        results.push_back(signals);
    }

    //freeing up all the stuff

    CUDA_CHECK(cudaFree(device_types));
    CUDA_CHECK(cudaFree(device_outputIDs));
    CUDA_CHECK(cudaFree(device_inputIDs));
    CUDA_CHECK(cudaFree(device_inputs));
    CUDA_CHECK(cudaFree(device_signals));
    CUDA_CHECK(cudaFree(device_offsets));

    return results;
}
