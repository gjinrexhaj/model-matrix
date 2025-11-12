//
// Created by Gjin on 10/28/25.
//

#pragma once
#include <vector>

#include "raylib.h"
#include "ruleset.h"
#include "grid.h"

class Simulation
{
    public:
        // Constructor
        Simulation(int simulationSpan, Ruleset ruleset, std::pmr::vector<Color> stateColors) :
            activeSimulationSpan(simulationSpan), activeRuleset(ruleset),
            activeStateColors(stateColors), activeGrid(simulationSpan, simulationSpan, simulationSpan),
            tempGrid(activeGrid) {}
        // public methods
        void ChangeRuleset(std::string newRuleset);
        void ChangeStateColors(std::pmr::vector<Color> newStateColors);
        std::string GetRulesetAsString();
        int CountLiveNeighbors(int x, int y, int z);
        void ClearGrid();
        void UpdateSimulationState();
        void DrawSimulationState();
        bool IsSimulationRunning();
        void StartSimulation();
        void StopSimulation();
        void LogSimulationState();

    private:
        // private variables - initialized by constructor
        int activeSimulationSpan; // How many cells the simulation space can fit vertically and horizontally
        std::pmr::vector<Color> activeStateColors;
        Ruleset activeRuleset;
        bool running = false;
        Grid activeGrid;
        Grid tempGrid;

};