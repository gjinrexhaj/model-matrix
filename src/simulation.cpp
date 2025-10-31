//
// Created by Gjin on 10/28/25.
//

#include "simulation.h"

#include <iostream>
#include <ostream>

// TODO: test if class works using test-harness
// TODO: impl change ruleset parameter given a string
// TODO: impl get ruleset as string
// TODO: impl change state colors
// TODO: impl grid to be used in sim



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

void Simulation::CountLiveNeighbors()
{
    std::cout<<"Counting live neighbors"<<std::endl;
}

void Simulation::UpdateSimulationState()
{
    std::cout<<"Updating simulation state"<<std::endl;
    // for now, just fill with black cube
    for (unsigned int z = 0; z < activeGrid.getDepth(); ++z) {
        for (unsigned int y = 0; y < activeGrid.getHeight(); ++y) {
            for (unsigned int x = 0; x < activeGrid.getWidth(); ++x) {
                if (true)
                {
                    activeGrid.write(x,y,z, 1);
                }
            }
        }
    }
}

void Simulation::DrawSimulationState()
{
    std::cout<<"Drawing simulation state"<<std::endl;

    // Center middle-most cube

    // Iterate through grid DS and draw the state
    for (unsigned int z = 0; z < activeGrid.getDepth(); ++z) {
        for (unsigned int y = 0; y < activeGrid.getHeight(); ++y) {
            for (unsigned int x = 0; x < activeGrid.getWidth(); ++x)
            {
                // Access using coordinates, draw if state > 0
                if (activeGrid.read(x,y,z) > 0)
                {
                    DrawCube(Vector3(x,y,z), 1,1,1,BLACK);
                }
                // std::cout << "Value at (" << x << ", " << y << ", " << z << "): "
                //           << grid(x, y, z) << std::endl;
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