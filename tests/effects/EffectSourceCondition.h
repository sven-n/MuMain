#pragma once

#include <functional>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

// Evaluates a condition read from the source of ZzzEffect.cpp for one effect
// type and SubType, so a test can compare it with a copy of it. It reads ||,
// &&, the comparisons, +, parentheses, o->Type, o->SubType, numbers and names;
// anything else is a problem, so a condition the test cannot evaluate fails it.
namespace EffectSourceCases
{
struct ConditionValue
{
    bool value = false;
    // The numbers written in the condition.
    std::set<int> numbers;
    std::vector<std::string> problems;
};

// The number of a name in a condition; nullopt for a name it does not know.
using NameValue = std::function<std::optional<int>(std::string_view)>;

// `condition` is written without spaces, as ReadSharedMoveConditions gives it.
ConditionValue EvaluateCondition(std::string_view condition, int type, int subType, const NameValue& nameValue);
} // namespace EffectSourceCases
