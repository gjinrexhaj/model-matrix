//
// Created by Gjin on 10/28/25.
//

#include "simulation.h"

#include <iostream>
#include <ostream>
#include <random>

#include "raymath.h"


// TODO: impl change ruleset parameter given a string
// TODO: impl get ruleset as string
// TODO: impl update grid state in accordance with ruleset
// TODO: impl draw grid state in accordance with activeColors
// TODO: make bounding box have thicker lines



void Simulation::ChangeRuleset(std::string newRuleset)
{
    std::cout<<"Changing ruleset with parameter [" + newRuleset + "]"<<std::endl;
}

void Simulation::ChangeStateColors(std::pmr::vector<Color> newStateColors)
{
    std::cout<<"Changing state colors"<<std::endl;
}


std::string Simulation::GetRulesetAsString()
{
    std::cout<<"Getting simulation ruleset as string"<<std::endl;
    return "test";
}

int Simulation::CountLiveNeighbors(int x, int y, int z)
{
    // TODO: implement count live neighbors for moore and von neumann
    std::cout<<"\n--- Counting live neighbors for cell {"<<x<<","<<y<<","<<z<<"} ---"<<std::endl;

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
        // TODO: figure out if this is the case, or if cells that are dying are still technically 'alive'
        if (gridValue == activeRuleset.numStates)
        {
            liveNeighbors++;
            std::cout<<"FOUND A LIVE NEIGHBOR OF {"<<x<<","<<y<<","<<z<<"}"
            " AT {"<<x_offset+x<<","<<y_offset+y<<","<<z_offset+z<<"}"<<"\n";
        }
    }

    std::cout<<"RETURNED LIVE NEIGHBORS = "<<liveNeighbors<<"\n";
    return liveNeighbors;
}

void Simulation::UpdateSimulationState()
{
    //std::cout<<"Updating simulation state"<<std::endl;
    // for now, just fill with black cube
    // TODO: impl drawing with appropriate ruleset, colors, etc.
    // for (unsigned int z = 0; z < activeGrid.getDepth(); ++z) {
    //     for (unsigned int y = 0; y < activeGrid.getHeight(); ++y) {
    //         for (unsigned int x = 0; x < activeGrid.getWidth(); ++x) {
    //             activeGrid.write(x,y,z, 1);
    //         }
    //     }
    // }

    activeGrid.write(2,0,0,1);
    activeGrid.write(2,2,1,1);
    activeGrid.write(1,2,1,1);
    activeGrid.write(1,1,2,1);
    activeGrid.write(1,0,0,1);
    activeGrid.write(0,1,0,1);
    activeGrid.write(0,1,1,1);


    int ln = CountLiveNeighbors(1, 0, 1);


}

void Simulation::DrawSimulationState()
{


    //std::cout<<"Drawing simulation state"<<std::endl;

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
    DrawCubeWiresV(Vector3(0,0,0), Vector3(activeSimulationSpan, activeSimulationSpan, activeSimulationSpan),WHITE);

    // Iterate through grid DS and draw the state
    for (unsigned int z = 0; z < activeGrid.getDepth(); ++z) {
        for (unsigned int y = 0; y < activeGrid.getHeight(); ++y) {
            for (unsigned int x = 0; x < activeGrid.getWidth(); ++x)
            {
                // Access using coordinates, draw if state > 0
                if (activeGrid.read(x,y,z) > 0)
                {
                    DrawCube(Vector3Add(translation3DOffset,Vector3(x,y,z)), 1,1,1,BLACK);
                }
            }
        }
    }
}

bool Simulation::IsSimulationRunning()
{
    std::cout<<"Checking if simulation running"<<std::endl;
    return running;
}

void Simulation::StartSimulation()
{
    std::cout<<"Starting simulation"<<std::endl;
    running = true;
}

void Simulation::StopSimulation()
{
    std::cout<<"Stopping simulation"<<std::endl;
    running = false;
}

void Simulation::LogSimulationState()
{
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
}