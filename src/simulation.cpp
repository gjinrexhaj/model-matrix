//
// Created by Gjin on 10/28/25.
//

#include "simulation.h"

#include <iostream>
#include <ostream>
#include <random>
#include <thread>
#include <vector>
#include <algorithm>

#include "raymath.h"
#include "ruleset.h"


namespace
{
    // 26 neighbors (Moore)
    constexpr int mooreDeltas[26][3] = {
        {-1,-1,-1},{0,-1,-1},{1,-1,-1},
        {-1, 0,-1},{0, 0,-1},{1, 0,-1},
        {-1, 1,-1},{0, 1,-1},{1, 1,-1},

        {-1,-1, 0},{0,-1, 0},{1,-1, 0},
        {-1, 0, 0},           {1, 0, 0},
        {-1, 1, 0},{0, 1, 0},{1, 1, 0},

        {-1,-1, 1},{0,-1, 1},{1,-1, 1},
        {-1, 0, 1},{0, 0, 1},{1, 0, 1},
        {-1, 1, 1},{0, 1, 1},{1, 1, 1}
    };

    // 6 neighbors (Von Neumann)
    constexpr int vonNeumannDeltas[6][3] = {
        { 1, 0, 0}, {-1, 0, 0},
        { 0, 1, 0}, { 0,-1, 0},
        { 0, 0, 1}, { 0, 0,-1}
    };
}

void Simulation::ChangeRuleset(const std::string& newRuleset, const NeighborCountingRule neighborCountingRule)
{
    std::cout<<"Changing ruleset with parameter [" + newRuleset + "]"<<std::endl;
    try
    {
        activeRuleset = Ruleset(newRuleset, neighborCountingRule);
        activeStateColors.resize(activeRuleset.numStates.at(0));
    } catch (std::exception &e)
    {
        std::cout<<e.what()<<std::endl;
    }

}

void Simulation::ChangeStateColors(const std::pmr::vector<Color>& newStateColors)
{
    activeStateColors = newStateColors;
}

std::pmr::vector<Color> Simulation::GetStateColors()
{
    return activeStateColors;
}

inline int Simulation::CountLiveNeighbors(int x, int y, int z)
{
    int count = 0;

    // cache grid dimensions
    const int w = activeGrid.getWidth();
    const int h = activeGrid.getHeight();
    const int d = activeGrid.getDepth();

    int maxState = activeRuleset.numStates.at(0);

    const int (*deltas)[3];
    int deltaCount;

    if (activeRuleset.neighborCountingRule == NeighborCountingRule::MOORE)
    {
        deltas = mooreDeltas;
        deltaCount = 26;
    } else // Von Neumann
    {
        deltas = vonNeumannDeltas;
        deltaCount = 6;
    }

    for (int i = 0; i < 26; ++i)
    {
        int nx = x + deltas[i][0];
        int ny = y + deltas[i][1];
        int nz = z + deltas[i][2];

        if (wrapGrid)  // wrap around edges
        {
            // modulo-based wrapping
            nx = (nx + w) % w;
            ny = (ny + h) % h;
            nz = (nz + d) % d;
            count += (activeGrid.read(nx, ny, nz) == maxState);
        }
        else  // hard boundaries
        {
            if ((unsigned)nx < (unsigned)w &&
                (unsigned)ny < (unsigned)h &&
                (unsigned)nz < (unsigned)d)
            {
                count += (activeGrid.read(nx, ny, nz) == maxState);
            }
        }
    }

    return count;
}

void Simulation::ClearGrid()
{
    for (unsigned int z = 0; z < activeGrid.getDepth(); ++z) {
        for (unsigned int y = 0; y < activeGrid.getHeight(); ++y) {
            for (unsigned int x = 0; x < activeGrid.getWidth(); ++x) {
                tempGrid.write(x,y,z,0);
                activeGrid.write(x,y,z,0);
            }
        }
    }
}

void Simulation::UpdateSimulationState()
{
    if (!running)
    {
        return;
    }

    // cache important vars
    unsigned int depth = activeGrid.getDepth();
    unsigned int height = activeGrid.getHeight();
    unsigned int width = activeGrid.getWidth();
    int maxState = activeRuleset.numStates[0];

    // determing number of threads
    numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0) numThreads = 4; // fallback



    // anonymous func which takes a range of Z layers to compute
    auto processSlice = [&](unsigned int zStart, unsigned int zEnd)
    {
        // loop over slize of z layers assigned to current thread
        for (unsigned int z = zStart; z < zEnd; ++z)
        {
            for (unsigned int y = 0; y < height; ++y)
            {
                for (unsigned int x = 0; x < width; ++x)
                {
                    int currentState = activeGrid.read(x,y,z);
                    int numLiveNeighbors = CountLiveNeighbors(x,y,z);

                    // apply logic in accordance to the ruleset
                    if (currentState == maxState)
                    {
                        bool metCondition = false;
                        for (auto survivalCondition : activeRuleset.survivalConditions)
                        {
                            if (numLiveNeighbors == survivalCondition)
                            {
                                tempGrid.write(x,y,z,maxState);
                                metCondition = true;
                            }
                        }
                        if (!metCondition)
                        {
                            tempGrid.write(x,y,z,maxState-1);
                        }
                    }
                    else if (currentState > 0)
                    {
                        tempGrid.write(x,y,z,currentState-1);
                    }
                    else
                    {
                        for (auto birthCondition : activeRuleset.birthConditions)
                        {
                            if (numLiveNeighbors == birthCondition)
                            {
                                tempGrid.write(x,y,z,maxState);
                            }
                        }
                    }
                }
            }
        }
    };

    // launch all the threads
    std::vector<std::thread> threads;
    unsigned int sliceSize = (depth + numThreads - 1) / numThreads; // ceiling division

    // for each thread, compute start/end of z-slice
    for (unsigned int t = 0; t < numThreads; ++t)
    {
        unsigned int zStart = t * sliceSize;
        unsigned int zEnd = std::min(zStart + sliceSize, depth);
        if (zStart >= depth) break; // no work for this thread

        threads.emplace_back(processSlice, zStart, zEnd);
    }

    // join all the threads
    for (auto& th : threads) th.join();

    // Copy temporary grid into active grid
    activeGrid = tempGrid;
}

void Simulation::DrawSimulationState()
{
    // cache vars and prepare state buffer
    const int depth = activeGrid.getDepth();
    const int height = activeGrid.getHeight();
    const int width = activeGrid.getWidth();
    const size_t numStates = activeStateColors.size();
    stateBuffers.assign(numStates, {});

    // calculate offset
    float rc = activeSimulationSpan / 2.0f;
    Vector3 offset = (activeSimulationSpan % 2) ?
                      Vector3{-rc, -rc, -rc} :
                      Vector3{-rc + 0.5f, -rc + 0.5f, -rc + 0.5f};


    // handle culling and sorting
    for (int z = 0; z < depth; ++z) {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int state = activeGrid.read(x, y, z);

                if (state > 0) {
                    // occlusion culling
                    if (x > 0 && x < width - 1 && y > 0 && y < height - 1 && z > 0 && z < depth - 1) {
                        if (activeGrid.read(x+1,y,z) != 0 && activeGrid.read(x-1,y,z) != 0 &&
                            activeGrid.read(x,y+1,z) != 0 && activeGrid.read(x,y-1,z) != 0 &&
                            activeGrid.read(x,y,z+1) != 0 && activeGrid.read(x,y,z-1) != 0)
                        {
                            continue;
                        }
                    }

                    // convert state number to buffer index
                    size_t colorIdx = static_cast<size_t>(state - 1);

                    // Safety check for dynamic rulesets
                    if (colorIdx < numStates) {
                        Matrix mat = MatrixTranslate(offset.x + x, offset.y + y, offset.z + z);
                        stateBuffers[colorIdx].push_back(mat);
                    }
                }
            }
        }
    }

    // begin batch rendering
    if (drawWireframe) {
        rlEnableWireMode();
    }

    for (size_t i = 0; i < numStates; ++i) {
        if (!stateBuffers[i].empty()) {
            // update mat color for given batch
            instanceMaterial.maps[MATERIAL_MAP_DIFFUSE].color = activeStateColors[i];

            DrawMeshInstanced(cubeMesh, instanceMaterial, stateBuffers[i].data(), (int)stateBuffers[i].size());
        }
    }

    if (drawWireframe)
    {
        rlDisableWireMode();
    }

    // draw bounding box
    DrawCubeWiresV(Vector3Zero(), {(float)activeSimulationSpan, (float)activeSimulationSpan, (float)activeSimulationSpan}, boundingBoxColor);
}

void Simulation::RandomizeSimulationState(float sparsity, int cubeRadius, bool additive, int origin[3])
{
    std::cout<<"--- Randomizing simulation state ---"<<std::endl;
    std::cout<<"sparsity: "<<sparsity<<"/10.0"<<std::endl;
    std::cout<<"cubeRadius: "<<cubeRadius<<std::endl;
    std::cout<<"additive: "<<additive<<std::endl;

    int maxState = activeRuleset.numStates.at(0);

    if (cubeRadius > activeGrid.getDepth())
    {
        cubeRadius = activeGrid.getDepth();
    }

    int startingPoint = activeGrid.getDepth()/2 - cubeRadius/2;
    int endingPoint = startingPoint + cubeRadius;

    for (unsigned int z = startingPoint; z < endingPoint; ++z) {
        for (unsigned int y = startingPoint; y < endingPoint; ++y) {
            for (unsigned int x = startingPoint; x < endingPoint; ++x) {
                double determine = static_cast<double>(std::rand()) / (RAND_MAX + 1.0) * 10;
                if (determine > sparsity)
                {
                    activeGrid.write(x+origin[0],y+origin[1],z+origin[2],maxState);
                } else if (!additive)
                {
                    activeGrid.write(x+origin[0],y+origin[1],z+origin[2],0);
                }
            }
        }
    }
}

void Simulation::ResizeSimulationSpan(int newSize)
{
    std::cout<<"--- Resizing simulation span ---"<<std::endl;
    std::cout<<"newSize: "<<newSize<<std::endl;
    activeSimulationSpan = newSize;
    activeGrid.resize(newSize, newSize, newSize);
    tempGrid.resize(newSize, newSize, newSize);
}

bool Simulation::IsSimulationRunning()
{
    return running;
}

void Simulation::StartSimulation()
{
    running = true;
}

void Simulation::StopSimulation()
{
    running = false;
}

unsigned int Simulation::getNumThreads()
{
    return numThreads;
}

void Simulation::toggleDrawWireframe()
{
    drawWireframe = !drawWireframe;
}

void Simulation::toggleGridWrapping()
{
    wrapGrid = !wrapGrid;
}

Simulation::~Simulation()
{
    UnloadMesh(cubeMesh);
    std::cout<<"--- Simulation state destructor ---"<<std::endl;
}
