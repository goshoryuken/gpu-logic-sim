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

__global__ void gpu_simulator(int* gateTypes, int* outputIDs, int* numInputs, int* inputOffsets, int* inputIDs, int* signals, int numGates) {
    int i = threadIdx.x + blockIdx.x * blockDim.x;

    if (i >= numGates) {
        return;
    } else {
        int result = 0;
        int type = gateTypes[i];

        if (type == AND) {

            result = 1;
            for (int k = 0; k < numInputs[i]; k++) {
                result &= signals[inputIDs[inputOffsets[i] + k]];
            }

        } else if (type == OR) {

            result = 0;
            for (int k = 0; k < numInputs[i]; k++) {
                result |= signals[inputIDs[inputOffsets[i] + k]];
            }

        } else if (type == NOT) {

            result = 0;
            result = !signals[inputIDs[inputOffsets[i]]];

        } else if (type == NAND) {

            result = 1;
            for (int k = 0; k < numInputs[i]; k++) {
                result &= signals[inputIDs[inputOffsets[i] + k]];
            }
            result = !result;

        } else if (type == XOR) {

            result = 0;
            for (int k = 0; k < numInputs[i]; k++) {
                result ^= signals[inputIDs[inputOffsets[i] + k]];
            }

        } else if (type == NOR) {

            result = 0;
            for (int k = 0; k < numInputs[i]; k++) {
                result |= signals[inputIDs[inputOffsets[i] + k]];
            }
            result = !result;
        }
        
        signals[outputIDs[i]] = result;
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

        //Launch Kernel
        gpu_simulator<<<numBlocks, threadsPerBlock>>>(device_types, device_outputIDs, device_inputs, device_offsets, device_inputIDs, device_signals, numGates);

        //ensures kernel finishes before results are copied back
        CUDA_CHECK(cudaDeviceSynchronize());

        //copy signals back
        CUDA_CHECK(cudaMemcpy(signals.data(), device_signals, netlist.signalIDs.size() * sizeof(int), cudaMemcpyDeviceToHost));
        

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
