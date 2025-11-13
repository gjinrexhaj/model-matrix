//
// Created by Gjin on 11/12/25.
//

#pragma once

#include "ruleset.h"

#include <iostream>


// EXAMPLE RULE: "4/4/5/M"
// EXAMPLE RULE: "4-2/4/5/M"
// EXAMPLE RULE: "4-2/4,8,9/5/M"
// EXAMPLE RULE: "4-2/4,8-10/5/M"

// TODO: link new ruleset with program
// TODO: add rule syntax checking (parse proper - numStates can only be one number)

RulesetNew::RulesetNew(std::string rulesetParameterString, NeighborCountingRule neighborCountingRule) {
    // Parse string
    this->neighborCountingRule = neighborCountingRule;

    std::string currentValueString = "";
    std::vector<std::vector<int>> conditionCollection;
    std::vector<int> currentCondition;

    for (auto character : rulesetParameterString)
    {
        // check for number
        if (std::isdigit(character))
        {
            // append to currentValue
            currentValueString += character;
        }
        // check for special symbols
        else if (character == ',')
        {
            // if comma, push the current value into the appropriate array
            int pushedValue = std::stoi(currentValueString);
            currentValueString = "";
            currentCondition.push_back(pushedValue);
        }
        // TODO: impl hyphen check and functionality
        // check if slash
        else if (character == '/')
        {
            int pushedValue = std::stoi(currentValueString);
            currentCondition.push_back(pushedValue);
            conditionCollection.push_back(currentCondition);
            currentCondition.clear();
            currentValueString = "";
        }
    }
    // append last element
    int pushedValue = std::stoi(currentValueString);
    currentCondition.push_back(pushedValue);
    conditionCollection.push_back(currentCondition);

    survivalConditions = {conditionCollection.at(0)};
    birthConditions = {conditionCollection.at(1)};
    numStates = {conditionCollection.at(2)};
}

std::string RulesetNew::GetRulesetAsString() {


    std::string sc;
    for (const int& num : survivalConditions)
    {
        sc += std::to_string(num);
        if (&num != &survivalConditions.back())
        {
            sc += ",";
        }
    }

    std::string bc;
    for (const int& num : birthConditions)
    {
        bc += std::to_string(num);
        if (&num != &birthConditions.back())
        {
            bc += ",";
        }
    }

    std::string ns;
    for (const int& num : numStates)
    {
        ns += std::to_string(num);
        if (&num != &numStates.back())
        {
            ns += ",";
        }
    }

    std::string ncr;
    switch (neighborCountingRule)
    {
        case NeighborCountingRule::MOORE:
            ncr += "M";
            break;
        case NeighborCountingRule::VON_NEUMANN:
            ncr += "VN";
            break;
    }

    std::string conditionsString = sc + "/" + bc + "/" + ns + "/" + ncr;
    return conditionsString;
}