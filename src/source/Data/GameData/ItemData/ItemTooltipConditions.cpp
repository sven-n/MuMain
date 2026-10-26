#include "stdafx.h"

#include "ItemTooltipConditions.h"
#include "ItemTooltipConditionsJson.h"
#include "ItemDescriptionPlaceholders.h"
#include "ItemEnumNames.h"
#include "ItemJsonFormat.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <charconv>

namespace Data::Items
{
namespace
{
constexpr std::string_view Negation = "!";
constexpr std::string_view TagPrefix = "tag:";
constexpr std::string_view ClassPrefix = "class:";

struct ComparisonName
{
    TooltipCondition::Comparison comparison;
    std::string_view text;
};
// The two-character ones first, so "<=" is not read as "<".
constexpr ComparisonName Comparisons[] = {
    {TooltipCondition::Comparison::NotEqual, "!="},
    {TooltipCondition::Comparison::LessOrEqual, "<="},
    {TooltipCondition::Comparison::GreaterOrEqual, ">="},
    {TooltipCondition::Comparison::Equal, "="},
    {TooltipCondition::Comparison::Less, "<"},
    {TooltipCondition::Comparison::Greater, ">"},
};

// Reads "<comparison><whole number>" at the start of `text`; false when it is not that.
bool ReadComparison(std::string_view text, TooltipCondition& condition)
{
    for (const ComparisonName& entry : Comparisons)
    {
        if (!text.starts_with(entry.text))
        {
            continue;
        }
        // Any int, from INT_MIN to INT_MAX: the whole rest is the number, with no sign but an optional "-".
        const std::string_view number = text.substr(entry.text.size());
        const char* const end = number.data() + number.size();
        int value = 0;
        const auto [parsed, error] = std::from_chars(number.data(), end, value);
        if (error != std::errc{} || parsed != end)
        {
            return false;
        }
        condition.comparison = entry.comparison;
        condition.number = value;
        return true;
    }
    return false;
}

constexpr std::array<std::string_view, 16> States = {
    "hasDamage",          "skillBook",           "twoHanded",      "excellent",       "ancient",
    "damageHarmonyBonus", "defenseHarmonyBonus", "armour",         "strengthReduced", "dexterityReduced",
    "strengthRequired",   "dexterityRequired",   "wizardryWeapon", "hasDurability",   "timed",
    "hasOptions",
};
} // namespace

std::span<const std::string_view> GetTooltipStates()
{
    return States;
}

bool IsTooltipState(std::string_view name)
{
    return std::find(States.begin(), States.end(), name) != States.end();
}

bool ReadTooltipCondition(std::string_view text, TooltipCondition& condition, std::string& error)
{
    std::string_view name = text;
    condition.negated = name.starts_with(Negation);
    if (condition.negated)
    {
        name.remove_prefix(Negation.size());
    }
    if (name.starts_with(TagPrefix))
    {
        name.remove_prefix(TagPrefix.size());
        condition.kind = TooltipCondition::Kind::Tag;
        if (!FindEnumValue(name, condition.tag))
        {
            error = "\"" + std::string(text) + "\" names no item tag (docs/item-data.md, \"Tags\")";
            return false;
        }
        return true;
    }
    if (name.starts_with(ClassPrefix))
    {
        name.remove_prefix(ClassPrefix.size());
        condition.kind = TooltipCondition::Kind::Class;
        condition.name = name;
        const std::span<const char* const> classes = GetClassKeys();
        const auto found = std::find(classes.begin(), classes.end(), name);
        if (found == classes.end())
        {
            error = "\"" + std::string(text) +
                    "\" names no class: a class is written as in \"classRequirements\" "
                    "(darkWizard, darkKnight, fairyElf, magicGladiator, darkLord, summoner, rageFighter)";
            return false;
        }
        condition.number = static_cast<int>(found - classes.begin());
        return true;
    }
    const size_t nameEnd =
        std::find_if(name.begin(), name.end(), [](char c) { return !std::isalnum(static_cast<unsigned char>(c)); }) -
        name.begin();
    const std::string_view comparison = name.substr(nameEnd);
    name = name.substr(0, nameEnd);
    condition.name = name;
    if (!comparison.empty())
    {
        if (!IsBuiltInPlaceholder(name))
        {
            error = "\"" + std::string(text) + "\" compares " +
                    (IsTooltipState(name) ? "a state, which is set or not" : "no built-in value") +
                    "; a comparison is a value, then =, !=, <, <=, > or >=, then a whole number";
            return false;
        }
        if (!ReadComparison(comparison, condition))
        {
            error = "\"" + std::string(text) +
                    "\" has no comparison with a whole number: a value, then =, !=, <, "
                    "<=, > or >=, then a whole number, such as \"durability=2\"";
            return false;
        }
        condition.kind = TooltipCondition::Kind::Value;
        return true;
    }
    if (IsTooltipState(name))
    {
        condition.kind = TooltipCondition::Kind::State;
        return true;
    }
    if (IsBuiltInPlaceholder(name))
    {
        condition.kind = TooltipCondition::Kind::Value;
        return true;
    }
    std::vector<std::string> known(GetBuiltInPlaceholders().begin(), GetBuiltInPlaceholders().end());
    known.insert(known.end(), States.begin(), States.end());
    const std::string suggestion = SuggestPlaceholder(name, known);
    error = "\"" + std::string(text) + "\" is neither a built-in value, a state nor \"tag:<tag>\"" +
            (suggestion.empty() ? std::string(" (docs/item-data.md)") : "; did you mean " + suggestion + "?");
    return false;
}

void ReadTooltipConditions(const Json::OrderedJson& json, const std::string& field, std::string_view example,
                           std::vector<TooltipCondition>& conditions,
                           const std::function<void(const std::string& field, const std::string& message)>& addError)
{
    if (!json.is_array())
    {
        addError(field, "must be a list of conditions, such as " + std::string(example));
        return;
    }
    for (size_t index = 0; index < json.size(); ++index)
    {
        const std::string entryField = field + "[" + std::to_string(index) + "]";
        TooltipCondition condition;
        std::string error;
        if (!json[index].is_string())
        {
            addError(entryField, "must be a text");
        }
        else if (!ReadTooltipCondition(json[index].get<std::string>(), condition, error))
        {
            addError(entryField, error);
        }
        else
        {
            conditions.push_back(std::move(condition));
        }
    }
}

std::string WriteTooltipCondition(const TooltipCondition& condition)
{
    std::string text(condition.negated ? Negation : std::string_view());
    if (condition.kind == TooltipCondition::Kind::Tag)
    {
        const char* tag = FindEnumName(condition.tag);
        return text + std::string(TagPrefix) + (tag != nullptr ? tag : "");
    }
    if (condition.kind == TooltipCondition::Kind::Class)
    {
        return text + std::string(ClassPrefix) + condition.name;
    }
    text += condition.name;
    for (const ComparisonName& entry : Comparisons)
    {
        if (entry.comparison == condition.comparison)
        {
            text += std::string(entry.text) + std::to_string(condition.number);
        }
    }
    return text;
}

bool CompareTooltipValue(const TooltipCondition& condition, int value)
{
    using Comparison = TooltipCondition::Comparison;
    switch (condition.comparison)
    {
    case Comparison::NotZero:
        return value != 0;
    case Comparison::Equal:
        return value == condition.number;
    case Comparison::NotEqual:
        return value != condition.number;
    case Comparison::Less:
        return value < condition.number;
    case Comparison::LessOrEqual:
        return value <= condition.number;
    case Comparison::Greater:
        return value > condition.number;
    case Comparison::GreaterOrEqual:
        return value >= condition.number;
    }
    return false;
}
} // namespace Data::Items
