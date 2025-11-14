//
// Created by Gjin on 10/28/25.
//

#include "simulation.h"

#include <iostream>
#include <ostream>
#include <random>
#include <utility>
#include <valarray>

#include "raymath.h"




void Simulation::ChangeRuleset(std::string newRuleset, NeighborCountingRule neighborCountingRule)
{
    std::cout<<"Changing ruleset with parameter [" + newRuleset + "]"<<std::endl;
    activeRuleset = RulesetNew(newRuleset, neighborCountingRule);
    activeStateColors.resize(activeRuleset.numStates.at(0));

}

void Simulation::ChangeStateColors(std::pmr::vector<Color> newStateColors)
{
    activeStateColors = newStateColors;
}

std::pmr::vector<Color> Simulation::GetStateColors()
{
    return activeStateColors;
}

int Simulation::CountLiveNeighbors(int x, int y, int z)
{
    int liveNeighbors = 0;
    std::vector<std::tuple<int, int, int>> neighborOffsets;

    //        Moore: all neighbors
    // Von Neumann: all neighbors with intersecting faces
    if (activeRuleset.neighborCountingRule == NeighborCountingRule::MOORE)
    {
        neighborOffsets = {
            //x, y, z - POSITIVE
            { 0, 0, 1 },
            { 0, 1, 0 },
            { 0, 1, 1 },
            { 1, 0, 0 },
            { 1, 0, 1 },
            { 1, 1, 0 },
            { 1, 1, 1 },
            //x, y, z - NEGATIVE
            { 0, 0,-1 },
            { 0,-1, 0 },
            { 0,-1,-1 },
            {-1, 0, 0 },
            {-1, 0,-1 },
            {-1,-1, 0 },
            {-1,-1,-1 },
            //x, y, z - POS/NEG
            { 1, 1,-1 },
            { 1,-1, 1 },
            { 1,-1,-1 },
            {-1, 1, 1 },
            {-1, 1,-1 },
            {-1,-1, 1 },
            // ALL THREE VALUES CASES
            { 0,-1, 1 },
            {-1, 1, 0 },
            { 1, 0,-1 },
            { 1,-1, 0 },
            {-1, 0, 1 },
            { 0, 1,-1 }
        };
    }
    else if (activeRuleset.neighborCountingRule == NeighborCountingRule::VON_NEUMANN)
    {
        neighborOffsets = {
            //x, y, z - POSITIVE
            { 0, 0, 1 },
            { 0, 1, 0 },
            { 1, 0, 0 },
            //x, y, z - NEGATIVE
            { 0, 0,-1 },
            { 0,-1, 0 },
            {-1, 0, 0 },
        };
    }

    for (const auto& [x_offset, y_offset, z_offset] : neighborOffsets)
    {
        // Add grid wrapping behavior
        int neighborX = (x + x_offset + activeSimulationSpan) % activeSimulationSpan;
        int neighborY = (y + y_offset + activeSimulationSpan) % activeSimulationSpan;
        int neighborZ = (z + z_offset + activeSimulationSpan) % activeSimulationSpan;

        // Get grid value of neighbor being counted
        int gridValue = activeGrid.read(neighborX, neighborY, neighborZ);

        // Only count as neighbor if it's at max state
        if (gridValue == activeRuleset.numStates.at(0))
        {
            liveNeighbors++;
        }
    }

    return liveNeighbors;
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

    int maxState = activeRuleset.numStates.at(0);

    // Iterate through grid, apply ruleset and alter a temporary grid
    for (unsigned int z = 0; z < activeGrid.getDepth(); ++z) {
        for (unsigned int y = 0; y < activeGrid.getHeight(); ++y) {
            for (unsigned int x = 0; x < activeGrid.getWidth(); ++x) {
                int currentState = activeGrid.read(x,y,z);
                int numLiveNeighbors = CountLiveNeighbors(x,y,z);

                // if alive
                if (currentState == maxState)
                {
                    // check the survival condition(s)
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
                // if decaying (still shows, but is effectively dead
                else if (currentState > 0)
                {
                    tempGrid.write(x,y,z,currentState-1);
                }
                // if dead, check the birth condition
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

    // copy temporary grid into active grid afterwards
    activeGrid = tempGrid;
}

void Simulation::DrawSimulationState()
{
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
    for (unsigned int z = 0; z < activeGrid.getDepth(); ++z) {
        for (unsigned int y = 0; y < activeGrid.getHeight(); ++y) {
            for (unsigned int x = 0; x < activeGrid.getWidth(); ++x)
            {
                // Get cell in question
                int currentCellState = activeGrid.read(x,y,z);

                // Access using coordinates, draw if state > 0 (0 is death, no state)
                if (currentCellState > 0)
                {
                    // draw white cube if cell exceeds max state due to change
                    if (currentCellState > activeRuleset.numStates.at(0))
                    {
                        DrawCube(Vector3Add(translation3DOffset, Vector3(x, y, z)), 1, 1, 1,
                            WHITE);
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

void Simulation::RandomizeSimulationState(float sparsity, int cubeRadius, bool additive)
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
                    activeGrid.write(x,y,z,maxState);
                } else if (!additive)
                {
                    activeGrid.write(x,y,z,0);
                }
            }
        }
    }
}

void Simulation::ResizeSimulationSpan(int newSize)
{
    // TODO: impl simulation resizing - need to add resize method to grid
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

void Simulation::LogSimulationState()
{
    /*
    std::cout<<"---- LOGGING SIMULATION STATE ----"<<std::endl;
    // print simSpan
    std::cout<<"activeSimulationSpan: "<< activeSimulationSpan<<std::endl;
    // print stateColors
    std::cout<<"activeStateColors: ";
    for (const Color col : activeStateColors) {
        std::cout << "[R: " << (int)col.r << " G: " << (int)col.g << " B: " << (int)col.b << " A: " << (int)col.a << "] ";
    }
    std::cout << std::endl;
    // print ruleset
    std::cout <<"activeRuleset: ";
    std::cout <<"[" << activeRuleset.survivalCondition << ", " << activeRuleset.birthCondition
    << ", " << activeRuleset.numStates << ", ";
    switch (activeRuleset.neighborCountingRule)
    {
        case NeighborCountingRule::MOORE:
            std::cout << "MOORE";
            break;
        case NeighborCountingRule::VON_NEUMANN:
            std::cout << "VON_NEUMANN";
            break;
        default:
            std::cout << "NULL";
    }
    std::cout << "]" << std::endl;
    // print running
    std::cout<<"running: " << running << std::endl;
    */
}