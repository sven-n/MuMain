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
static_assert(States.size() == static_cast<size_t>(TooltipState::Count), "a name for every TooltipState, in order");
} // namespace

std::span<const std::string_view> GetTooltipStates()
{
    return States;
}

bool IsTooltipState(std::string_view name)
{
    return FindTooltipState(name).has_value();
}

std::optional<TooltipState> FindTooltipState(std::string_view name)
{
    const auto found = std::find(States.begin(), States.end(), name);
    return found == States.end() ? std::nullopt
                                 : std::optional<TooltipState>(static_cast<TooltipState>(found - States.begin()));
}

std::string_view TooltipCondition::GetName() const
{
    switch (kind)
    {
    case Kind::Value:
        return value != TooltipValue::Count ? GetBuiltInPlaceholderName(value) : std::string_view();
    case Kind::State:
        return state != TooltipState::Count ? States[static_cast<size_t>(state)] : std::string_view();
    case Kind::Tag:
    {
        const char* name = FindEnumName(tag);
        return name != nullptr ? std::string_view(name) : std::string_view();
    }
    case Kind::Class:
    {
        const std::span<const char* const> classes = GetClassKeys();
        return number >= 0 && static_cast<size_t>(number) < classes.size() ? std::string_view(classes[number])
                                                                           : std::string_view();
    }
    }
    return {};
}

bool ReadTooltipCondition(std::string_view text, TooltipCondition& condition, std::string& error)
{
    std::string_view name = text;
    condition.value = TooltipValue::Count;
    condition.state = TooltipState::Count;
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
    const std::optional<TooltipValue> value = FindBuiltInPlaceholder(name);
    if (!comparison.empty())
    {
        if (!value)
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
        condition.value = *value;
        return true;
    }
    if (const std::optional<TooltipState> state = FindTooltipState(name))
    {
        condition.kind = TooltipCondition::Kind::State;
        condition.state = *state;
        return true;
    }
    if (value)
    {
        condition.kind = TooltipCondition::Kind::Value;
        condition.value = *value;
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
        return text + std::string(TagPrefix) + std::string(condition.GetName());
    }
    if (condition.kind == TooltipCondition::Kind::Class)
    {
        return text + std::string(ClassPrefix) + std::string(condition.GetName());
    }
    text += condition.GetName();
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
