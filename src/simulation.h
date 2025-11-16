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
        Color boundingBoxColor = WHITE;



        // Constructor
        Simulation(int simulationSpan, RulesetNew ruleset, std::pmr::vector<Color> stateColors) :
            activeSimulationSpan(simulationSpan), activeRuleset(ruleset),
            activeStateColors(stateColors), activeGrid(simulationSpan, simulationSpan, simulationSpan),
            tempGrid(activeGrid) {}
        // public methods
        void ChangeRuleset(std::string newRuleset, NeighborCountingRule neighborCountingRule);
        void ChangeStateColors(std::pmr::vector<Color> newStateColors);
        std::pmr::vector<Color> GetStateColors();
        int CountLiveNeighbors(int x, int y, int z);
        void ClearGrid();
        void UpdateSimulationState();
        void DrawSimulationState();
        void RandomizeSimulationState(float sparsity, int cubeRadius, bool additive);
        void ResizeSimulationSpan(int newSize);
        bool IsSimulationRunning();
        void StartSimulation();
        void StopSimulation();
        void LogSimulationState();

    private:
        // private variables - initialized by constructor
        int activeSimulationSpan; // How many cells the simulation space can fit vertically and horizontally
        std::pmr::vector<Color> activeStateColors;
        RulesetNew activeRuleset;
        bool running = false;
        Grid activeGrid;
        Grid tempGrid;

};