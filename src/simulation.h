//
// Created by Gjin on 10/28/25.
//

#pragma once
#include <thread>
#include <utility>
#include <vector>

#include "raylib.h"
#include "ruleset.h"
#include "grid.h"
#include "rlgl.h"

class Simulation
{
    public:
        Color boundingBoxColor = WHITE;

        // Constructor
        Simulation(int simulationSpan, RulesetNew ruleset, std::pmr::vector<Color> stateColors) :
            activeSimulationSpan(simulationSpan), activeRuleset(std::move(ruleset)),
            activeStateColors(std::move(stateColors)), activeGrid(simulationSpan, simulationSpan, simulationSpan),
            tempGrid(activeGrid) {}
        // public methods
        void ChangeRuleset(const std::string& newRuleset, NeighborCountingRule neighborCountingRule);
        void ChangeStateColors(const std::pmr::vector<Color>& newStateColors);
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
        unsigned int getNumThreads();
        // Destructor
        ~Simulation();

    private:
        // private variables - initialized by constructor
        int activeSimulationSpan; // How many cells the simulation space can fit vertically and horizontally
        std::pmr::vector<Color> activeStateColors;
        RulesetNew activeRuleset;
        bool running = false;
        Grid activeGrid;
        Grid tempGrid;
        unsigned int numThreads;
};