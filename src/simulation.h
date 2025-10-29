//
// Created by Gjin on 10/28/25.
//

#pragma once
#include <vector>

#include "raylib.h"
#include "ruleset.h"
#include "grid.h"

// TODO: impl update and draw simulation methods once grid is fully figured out

class Simulation
{
    public:
        // Constructor
        Simulation(int simulationSpan, Ruleset ruleset, std::pmr::vector<Color> stateColors) :
            activeSimulationSpan(simulationSpan), activeRuleset(ruleset),
            activeStateColors(stateColors), activeGrid(simulationSpan, simulationSpan, simulationSpan) {};

        // public methods
        void ChangeRuleset(std::string newRuleset);
        void ChangeStateColors(std::pmr::vector<Color> newStateColors);
        std::string GetRulesetAsString();
        void CountLiveNeighbors();
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

};