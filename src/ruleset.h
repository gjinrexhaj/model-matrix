//
// Created by Gjin on 10/28/25.
//

#pragma once


enum class NeighborCountingRule: int
{
    MOORE = 0,
    VON_NEUMANN = 1,
};

class Ruleset
{
    public:
        int survivalCondition;
        int birthCondition;
        int numStates;
        NeighborCountingRule neighborCountingRule;

        Ruleset(int survivalCondition, int birthCondition, int numStates, NeighborCountingRule neighborCountingRule);
};