#pragma once

#include "Data/GameData/ItemData/ItemTagSet.h"

#include <span>
#include <string>
#include <string_view>
#include <vector>

// The conditions of tooltip lines, in the tooltip layouts (ItemTooltipLayouts.h) and in item descriptions: "defense"
// (a built-in value that is not 0), "durability=2" (a value compared with a whole number by =, !=, <, <=, > or >=),
// "excellent" (a state), "tag:sceptre" (an item tag), each turned around by a "!" in front.
namespace Data::Items
{
// The states a condition can name besides the built-in values: what the game knows of the item and the hero
// (Engine/Object/ItemTooltipValues), listed in docs/item-data.md.
std::span<const std::string_view> GetTooltipStates();
bool IsTooltipState(std::string_view name);

struct TooltipCondition
{
    enum class Kind : unsigned char
    {
        Value, // a built-in value (or {level}), compared with `number`
        State,
        Tag,
    };
    enum class Comparison : unsigned char
    {
        NotZero,
        Equal,
        NotEqual,
        Less,
        LessOrEqual,
        Greater,
        GreaterOrEqual,
    };
    Kind kind = Kind::Value;
    std::string name; // of the value or state
    ItemTag tag = ItemTag::Count;
    bool negated = false;
    Comparison comparison = Comparison::NotZero;
    int number = 0;

    bool operator==(const TooltipCondition& other) const = default;
};

// Reads a condition as the files write it. False, with the problem in `error`, for one that names no built-in value,
// state or item tag; the error suggests the name meant for a typo.
bool ReadTooltipCondition(std::string_view text, TooltipCondition& condition, std::string& error);

// The condition as the files write it.
std::string WriteTooltipCondition(const TooltipCondition& condition);

// Whether a value meets a value condition (not yet turned around by its "!").
bool CompareTooltipValue(const TooltipCondition& condition, int value);
} // namespace Data::Items
