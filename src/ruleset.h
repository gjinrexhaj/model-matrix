//
// Created by Gjin on 10/28/25.
//

#pragma once

// TODO, ADJUST RULESET SO THAT MEMEBR VARS ARE VECOTRS OF STRINGS,
//  AND INPUT PARAMETER IS INPUT STRING THAT AUTOMATICALLY PARSES
//  AT CONSTRUCTION TO BUILD THE RULESET


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

        Ruleset(int survivalCondition, int birthCondition, int numStates, NeighborCountingRule neighborCountingRule) :
            survivalCondition(survivalCondition), birthCondition(birthCondition),
            numStates(numStates), neighborCountingRule(neighborCountingRule) {};
};