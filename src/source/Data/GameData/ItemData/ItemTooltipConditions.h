#pragma once

#include "Data/GameData/ItemData/ItemDescriptionPlaceholders.h"
#include "Data/GameData/ItemData/ItemTagSet.h"

#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// The conditions of tooltip lines, in the tooltip layouts (ItemTooltipLayouts.h) and in item descriptions: "defense"
// (a built-in value that is not 0), "durability=2" (a value compared with a whole number by =, !=, <, <=, > or >=),
// "excellent" (a state), "tag:sceptre" (an item tag), "class:darkLord" (the hero's class, as "classRequirements"
// names it), each turned around by a "!" in front.
namespace Data::Items
{
// The states a condition can name besides the built-in values: what the game knows of the item and the hero
// (Engine/Object/ItemTooltipValues), listed in docs/item-data.md.
std::span<const std::string_view> GetTooltipStates();
bool IsTooltipState(std::string_view name);

// The states by number, in GetTooltipStates' order: what a condition naming one is resolved to when it is read.
enum class TooltipState : unsigned char
{
    HasDamage,
    SkillBook,
    TwoHanded,
    Excellent,
    Ancient,
    DamageHarmonyBonus,
    DefenseHarmonyBonus,
    Armour,
    StrengthReduced,
    DexterityReduced,
    StrengthRequired,
    DexterityRequired,
    WizardryWeapon,
    HasDurability,
    Timed,
    HasOptions,
    Count,
};
// The state of that name, or nothing for another name.
std::optional<TooltipState> FindTooltipState(std::string_view name);

struct TooltipCondition
{
    enum class Kind : unsigned char
    {
        Value, // a built-in value (or {level}), compared with `number`
        State,
        Tag,
        Class, // the hero's class is `number` (CLASS_WIZARD, ...)
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
    // What the condition names, by number, so a condition is checked without comparing names: its name as the file
    // writes it is GetName()'s, made from these, so the two can't disagree.
    Kind kind = Kind::Value;
    TooltipValue value = TooltipValue::Count; // Kind::Value
    TooltipState state = TooltipState::Count; // Kind::State
    ItemTag tag = ItemTag::Count;             // Kind::Tag
    bool negated = false;
    Comparison comparison = Comparison::NotZero;
    int number = 0; // the number a value is compared with; the class of Kind::Class

    // The name of the value, state or class as the file writes it ("durability", "excellent", "darkLord"; the tag's
    // for Kind::Tag), without the "!" and the comparison; empty for one that names nothing.
    std::string_view GetName() const;

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
