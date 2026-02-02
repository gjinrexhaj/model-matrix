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
        activeRuleset = RulesetNew(newRuleset, neighborCountingRule);
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
        return; // early ret if not running

    // cache grid sizes
    unsigned int depth = activeGrid.getDepth();
    unsigned int height = activeGrid.getHeight();
    unsigned int width = activeGrid.getWidth();

    int maxState = activeRuleset.numStates.at(0);

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
    const int depth = activeGrid.getDepth();
    const int height = activeGrid.getHeight();
    const int width = activeGrid.getWidth();


    // Center middle-most cube
    int rc = activeSimulationSpan/2;
    Vector3 translation3DOffset;
    if (activeSimulationSpan % 2)
    {
        translation3DOffset = Vector3(-rc, -rc, -rc);
    } else
    {
        translation3DOffset = Vector3(-rc + 0.5, -rc + 0.5, -rc + 0.5);
    }

    // Draw bounding-box
    DrawCubeWiresV(Vector3(0,0,0), Vector3(activeSimulationSpan, activeSimulationSpan, activeSimulationSpan),boundingBoxColor);

    // Iterate through grid DS and draw the state
    for (unsigned int z = 0; z < depth; ++z) {
        for (unsigned int y = 0; y < height; ++y) {
            for (unsigned int x = 0; x < width; ++x)
            {
                // Get cell in question
                int currentCellState = activeGrid.read(x,y,z);

                // Access using coordinates, draw if state > 0 (0 is death, no state)
                if (currentCellState > 0)
                {
                    // draw white cube if cell exceeds max state due to change
                    if (currentCellState > activeRuleset.numStates.at(0))
                    {
                        if (drawWireframe)
                        {
                            DrawCubeWires(Vector3Add(translation3DOffset, Vector3(x, y, z)), 1, 1, 1,
                            Color(255, 255, 255, 127));
                        } else
                        {
                            DrawCube(Vector3Add(translation3DOffset, Vector3(x, y, z)), 1, 1, 1,
                            Color(255, 255, 255, 127));
                        }
                    } else
                    {
                        if (drawWireframe)
                        {
                            DrawCubeWires(Vector3Add(translation3DOffset, Vector3(x, y, z)), 1, 1, 1,
                            activeStateColors.at(currentCellState - 1));
                        } else
                        {
                            DrawCube(Vector3Add(translation3DOffset, Vector3(x, y, z)), 1, 1, 1,
                            activeStateColors.at(currentCellState - 1));
                        }
                    }
                }
            }
        }
    }
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
                    try
                    {
                        activeGrid.write(x+origin[0],y+origin[1],z+origin[2],maxState);
                    } catch (std::exception& e)
                    {
                        std::cerr<<e.what()<<std::endl;
                    }
                } else if (!additive)
                {
                    try
                    {
                        activeGrid.write(x+origin[0],y+origin[1],z+origin[2],0);
                    } catch (std::exception& e)
                    {
                        std::cerr<<e.what()<<std::endl;
                    }
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
    std::cout<<"--- Simulation state destructor ---"<<std::endl;
}
