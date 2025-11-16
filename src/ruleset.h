//
// Created by Gjin on 10/28/25.
//

#pragma once
#include "ruleset_new.h"

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