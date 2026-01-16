//
// Created by Gjin on 11/12/25.
//

#pragma once
#include <string>
#include <vector>


enum class NeighborCountingRule: int
{
    MOORE = 0,
    VON_NEUMANN = 1,
};

class RulesetNew
{
public:
    std::vector<int> survivalConditions;
    std::vector<int> birthConditions;
    std::vector<int> numStates;
    NeighborCountingRule neighborCountingRule;

    RulesetNew(std::string rulesetParameterString, NeighborCountingRule neighborCountingRule);

    std::string GetRulesetAsString();
};