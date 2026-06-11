#include <cuda_runtime.h>
#include "netlist.h"
#include <map>
#include <vector>
#include <iostream>
#include <cooperative_groups.h>
#include <algorithm>

namespace cg = cooperative_groups;
using namespace std;

#define CUDA_CHECK(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, const char *file, int line, bool abort=true) {
   if (code != cudaSuccess) {
      std::fprintf(stderr, "GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
      if (abort) exit(code);
   }
}

__global__ void gpu_simulator(int* gateTypes, int* outputIDs, int* numInputs, int* inputOffsets, int* inputIDs, int* signals, int numGates,
    int maxLevel, int numCycles, int* dff_inputs, int* dff_outputs, int numDFFs, int* nextState,
    int* levelStart, int* levelCount) {
   
    //making the grid level cooperative group handle
    cg::grid_group grid = cg::this_grid();
    int tid = threadIdx.x + blockIdx.x * blockDim.x;
    int stride = blockDim.x * gridDim.x;

    for (int i = 0; i < numCycles; i++) {
        for (int j = 0; j <= maxLevel; j++) {

            int startIdx = levelStart[j];
            int count = levelCount[j];

            for (int n = tid; n < count; n += stride) {
                int gateIdx = startIdx + n;

                int type = gateTypes[gateIdx];
                int result = 0;

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
                } else if (type == MUX) {
            
                    result = signals[inputIDs[inputOffsets[gateIdx] + 2]] ? signals[inputIDs[inputOffsets[gateIdx] + 1]]: signals[inputIDs[inputOffsets[gateIdx]]];

                } else if (type == ANDNOT) {

                    result = signals[inputIDs[inputOffsets[gateIdx]]] & !signals[inputIDs[inputOffsets[gateIdx] + 1]];

                } else if (type == ORNOT) {

                    result = signals[inputIDs[inputOffsets[gateIdx]]] | !signals[inputIDs[inputOffsets[gateIdx] + 1]];

                }
        
                signals[outputIDs[gateIdx]] = result;

            }

            //every thread must hit this barrier
            grid.sync();

        }

        //all levels done for this cycle, time for DFF update
        for (int d = tid; d < numDFFs; d += stride) {
            nextState[dff_outputs[d]] = signals[dff_inputs[d]];
        }
        grid.sync();

        for (int d = tid; d < numDFFs; d += stride) {
            signals[dff_outputs[d]] = nextState[dff_outputs[d]];
        }
        grid.sync();
    }

}

vector<vector<int>> simulateGPU(const Netlist& netlist, const map<string, int>& inputValues, int numCycles) {
    vector<int> gateTypes;
    vector<int> outputIDs;
    vector<int> numInputs;
    vector<int> gateLevels;
    vector<int> dff_inputs;
    vector<int> dff_outputs;

    

    for (const Gate& gate : netlist.gates) {
        gateTypes.push_back(gate.gateTypeID);
        outputIDs.push_back(gate.outputID);
        numInputs.push_back(gate.inputIDs.size());
        gateLevels.push_back(gate.level);
    }

    for (const Gate& gate : netlist.dffs) {
        dff_inputs.push_back(gate.inputIDs[0]);
        dff_outputs.push_back(gate.outputID);
    }

    int maxLevel = *std::max_element(gateLevels.begin(), gateLevels.end());

    //tell the gpu exactly where each level begins and how many gates are in it
    vector<int> levelStart(maxLevel + 1, 0);
    vector<int> levelCount(maxLevel + 1, 0);

    for (int i = 0; i < gateLevels.size(); i++) {
        int lvl = gateLevels[i];
        if (levelCount[lvl] == 0) {
            levelStart[lvl] = i; //mark starting index for lvl
        }
        levelCount[lvl]++; //count how many gates in this level
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
        if (netlist.signalIDs.count(pair.first) > 0) {
            signals[netlist.signalIDs.at(pair.first)] = pair.second;
        } else {
            // Optional warning to know what's happening
            std::cout << "[GPU WARNING] Input '" << pair.first << "' missing! Skipping." << std::endl;
        }
    }


    int* device_types = nullptr;
    int* device_outputIDs = nullptr;
    int* device_inputIDs = nullptr;
    int* device_offsets = nullptr;
    int* device_signals = nullptr;
    int* device_inputs = nullptr;
    int* device_dff_inputs = nullptr;
    int* device_dff_outputs = nullptr;
    int* device_nextState = nullptr;
    int* device_levelStart = nullptr;
    int* device_levelCount = nullptr;

    //for levelStart
    CUDA_CHECK(cudaMalloc((void**)&device_levelStart, levelStart.size() * sizeof(int)));
    CUDA_CHECK(cudaMemcpy(device_levelStart, levelStart.data(), levelStart.size() * sizeof(int), cudaMemcpyHostToDevice));

    //for levelCount
    CUDA_CHECK(cudaMalloc((void**)&device_levelCount, levelCount.size() * sizeof(int)));
    CUDA_CHECK(cudaMemcpy(device_levelCount, levelCount.data(), levelCount.size() * sizeof(int), cudaMemcpyHostToDevice));

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

    //for signals

    CUDA_CHECK(cudaMalloc((void**)&device_signals, netlist.signalIDs.size() * sizeof(int)));
    CUDA_CHECK(cudaMemcpy(device_signals, signals.data(), netlist.signalIDs.size() * sizeof(int), cudaMemcpyHostToDevice));

    //for inputIDs

    CUDA_CHECK(cudaMalloc((void**)&device_inputIDs, inputIDs.size() * sizeof(int))); 
    CUDA_CHECK(cudaMemcpy(device_inputIDs, inputIDs.data(), inputIDs.size() * sizeof(int), cudaMemcpyHostToDevice));

    //for dff_inputs

    CUDA_CHECK(cudaMalloc((void**)&device_dff_inputs, netlist.dffs.size() * sizeof(int))); 
    CUDA_CHECK(cudaMemcpy(device_dff_inputs, dff_inputs.data(), netlist.dffs.size() * sizeof(int), cudaMemcpyHostToDevice));

    //for dff_outputs

    CUDA_CHECK(cudaMalloc((void**)&device_dff_outputs, netlist.dffs.size() * sizeof(int))); 
    CUDA_CHECK(cudaMemcpy(device_dff_outputs, dff_outputs.data(), netlist.dffs.size() * sizeof(int), cudaMemcpyHostToDevice));

    //allocating next_state
    CUDA_CHECK(cudaMalloc(&device_nextState, netlist.signalIDs.size() * sizeof(int)));

    //determining grid dimensions

    int numBlocksPerSM = 0;
    int numSMs = 0;
    int threadsPerBlock = 256;

    cudaOccupancyMaxActiveBlocksPerMultiprocessor(
        &numBlocksPerSM,
        (void*)gpu_simulator,
        threadsPerBlock,
        0
    );

    CUDA_CHECK(cudaDeviceGetAttribute(&numSMs, cudaDevAttrMultiProcessorCount, 0));

    int gridDimensions = numSMs * numBlocksPerSM;

    dim3 dimBlock(threadsPerBlock);
    dim3 dimGrid(gridDimensions);

    int numGates = (int)netlist.gates.size();
    int numDFFs = (int)netlist.dffs.size();
    int numSignals = (int)netlist.signalIDs.size();

    void* kernelArgs[] = {
        &device_types,
        &device_outputIDs,
        &device_inputs,
        &device_offsets,
        &device_inputIDs,
        &device_signals,
        &numGates,
        &maxLevel,
        &numCycles,
        &device_dff_inputs,
        &device_dff_outputs,
        &numDFFs,
        &device_nextState,
        &device_levelStart,
        &device_levelCount
    };
    
    CUDA_CHECK(cudaLaunchCooperativeKernel(
        (void*)gpu_simulator,
        dimGrid,
        dimBlock,
        kernelArgs,
        0,
        nullptr
    ));
    
    CUDA_CHECK(cudaGetLastError());

    CUDA_CHECK(cudaDeviceSynchronize());

    CUDA_CHECK(cudaMemcpy(signals.data(), device_signals, netlist.signalIDs.size() * sizeof(int), cudaMemcpyDeviceToHost));

    vector<vector<int>> results;
    results.push_back(signals);

   
    //freeing up all the stuff

    CUDA_CHECK(cudaFree(device_types));
    CUDA_CHECK(cudaFree(device_outputIDs));
    CUDA_CHECK(cudaFree(device_inputIDs));
    CUDA_CHECK(cudaFree(device_inputs));
    CUDA_CHECK(cudaFree(device_signals));
    CUDA_CHECK(cudaFree(device_offsets));
    CUDA_CHECK(cudaFree(device_dff_inputs));
    CUDA_CHECK(cudaFree(device_dff_outputs));
    CUDA_CHECK(cudaFree(device_nextState));
    CUDA_CHECK(cudaFree(device_levelStart));
    CUDA_CHECK(cudaFree(device_levelCount));

    return results;
}
