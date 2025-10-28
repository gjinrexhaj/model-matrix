//
// Created by Gjin on 10/28/25.
//

#pragma once
#include <vector>

#include "raylib.h"
#include "ruleset.h"


class Simulation
{
    public:
        // Constructor
        Simulation(int simulationSpan, Ruleset ruleset, std::pmr::vector<Color> stateColors);

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
        int activeSimulationSpan = {}; // How many cells the simulation space can fit vertically and horizontally
        std::pmr::vector<Color> activeStateColors;
        Ruleset activeRuleset = {4,4,4, NeighborCountingRule::MOORE};
        bool running = false;

};