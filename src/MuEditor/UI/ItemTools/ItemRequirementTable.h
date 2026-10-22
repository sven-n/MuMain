#pragma once

#ifdef _EDITOR

#include "Data/GameData/ItemData/ItemDefinition.h"
#include "Data/GameData/ItemData/ItemStatProfiles.h"
#include "GameLogic/Items/ItemStats.h"

#include <array>
#include <cstdint>
#include <optional>
#include <string>

namespace MuEditor::Items
{
// What the game adds to an item's requirements and stats besides its + level
// (the option level only to the strength requirement).
struct RequirementVariant
{
    bool excellent = false;
    bool ancient = false;
    int optionLevel = 0;
};

// The stat profile storing an item gives it (CItemDataHandler::WithDerivedFields)
// and where it comes from: "" takes what the profile rules give, a name equal
// to the stored item's follows them while it is what they give, another name
// is a pick.
struct StatProfileChoice
{
    enum class Reason : uint8_t
    {
        // The profile rules give it: `rule`, or no rule matches (the default).
        ByRules,
        // Picked for the item, also "default"; the rules give `given`.
        Picked,
    };

    // "default" for the default.
    std::string name;
    Reason reason = Reason::ByRules;
    // The rule the item matches, or Data::Items::NoStatProfileRule.
    int rule = Data::Items::NoStatProfileRule;
    // What the rules give; "default" for the default.
    std::string given;
};

StatProfileChoice FindStatProfileChoice(const Data::Items::ItemDefinition& item);
// The resolved profile of FindStatProfileChoice; the default for a name the
// item stat file does not have.
const Data::Items::ItemStatProfile& FindStatProfile(const Data::Items::ItemDefinition& item);

// An item's requirements at one + level, in the order of ItemRequirement.
using RequirementRow = std::array<int, Data::Items::ItemRequirementCount>;
using RequirementTable = std::array<RequirementRow, Data::Items::ItemLevelCount>;

// The requirements of `item` at each + level as the game computes them
// (GameLogic::Items::ComputeItemStats), from the item as a tool has it, also
// before it is stored: its base values and drop level, its requirements by
// level, and the stat profile storing it would give (Item Basics' pick, or
// what its data gives). The normal variant at option level 0 is what the game's
// /<item name> window shows.
RequirementTable ComputeRequirementTable(const Data::Items::ItemDefinition& item, const RequirementVariant& variant);

// All the values the game computes for `item` at each + level (damage, magic
// power, defense, magic defense, block rate and the requirements), as
// ComputeRequirementTable takes them.
using ItemLevelValues = std::array<GameLogic::Items::ItemStatValues, Data::Items::ItemLevelCount>;
ItemLevelValues ComputeItemLevelValues(const Data::Items::ItemDefinition& item, const RequirementVariant& variant);
// The same at one + level.
GameLogic::Items::ItemStatValues ComputeItemValuesAt(const Data::Items::ItemDefinition& item,
                                                     const RequirementVariant& variant, int level);

// Whether the option level adds to the item's strength requirement (its stat
// profile's strength per option level is not 0).
bool OptionLevelAddsStrength(const Data::Items::ItemDefinition& item);

// The base value of `requirement` with which `item`, without that requirement
// by level, has `values` at every + level as a normal item: the level and a
// base kept as it is are the +0 value; a stat the smallest base that gives
// all 16 values, else the smallest that gives the +0 value, else the base
// whose +0 value is nearest.
WORD FindRequirementBase(const Data::Items::ItemDefinition& item, Data::Items::ItemRequirement requirement,
                         const Data::Items::RequirementByLevel& values);

// The weight of a stat requirement is its base value, which the item stat
// file's formula takes. Several weights can give the same requirement at +0,
// as the formula rounds down; they differ at higher + levels and for excellent
// items. The functions below work on the item without the requirement by
// level, as FindRequirementBase does.

// The weights first to last; empty when last < first.
struct WeightRange
{
    int first = 0;
    int last = -1;

    bool IsEmpty() const
    {
        return last < first;
    }
    int GetCount() const
    {
        return IsEmpty() ? 0 : last - first + 1;
    }
    bool Contains(int weight) const
    {
        return weight >= first && weight <= last;
    }
    bool operator==(const WeightRange& other) const = default;
};

// Whether the requirement grows by a formula from its base value: a stat
// requirement whose formula does not keep the base ("base"). The level has
// none.
bool HasWeight(const Data::Items::ItemDefinition& item, Data::Items::ItemRequirement requirement);
// What a normal item needs at +0 without an option level, as the game's
// /<item name> window shows it; also for a requirement given by level.
int ComputeZeroValue(const Data::Items::ItemDefinition& item, Data::Items::ItemRequirement requirement);
// The weights whose +0 value is `value`: one range, as the +0 value never
// falls when the weight grows (weight 0 gives 0).
WeightRange FindWeightsGiving(const Data::Items::ItemDefinition& item, Data::Items::ItemRequirement requirement,
                              int value);

// What a weight gives at +15, and for an excellent item at +0 and +15.
struct WeightValues
{
    int weight = 0;
    int plus15 = 0;
    int excellent = 0;
    int excellentPlus15 = 0;

    bool operator==(const WeightValues& other) const = default;
};
WeightValues ComputeWeightValues(const Data::Items::ItemDefinition& item, Data::Items::ItemRequirement requirement,
                                 int weight);

// A +0 value some weights give.
struct ReachableValue
{
    int value = 0;
    WeightRange weights;

    bool operator==(const ReachableValue& other) const = default;
};
// The +0 values nearest to one no weight gives: the highest below it and the
// lowest above it, where there are such.
struct NearestZeroValues
{
    std::optional<ReachableValue> below;
    std::optional<ReachableValue> above;
};
NearestZeroValues FindNearestZeroValues(const Data::Items::ItemDefinition& item,
                                        Data::Items::ItemRequirement requirement, int value);

// What a typed +0 value does with the weight.
struct WeightChoice
{
    enum class Kind : uint8_t
    {
        // The current weight gives it.
        Unchanged,
        // Only `weight` gives it.
        Take,
        // `weights` give it, the smallest is `weight`: one has to be picked.
        Pick,
        // No weight gives it; `nearest` has the values next to it.
        Unreachable,
    };

    Kind kind = Kind::Unchanged;
    int weight = 0;
    WeightRange weights;
    NearestZeroValues nearest;
};
WeightChoice ChooseWeight(const Data::Items::ItemDefinition& item, Data::Items::ItemRequirement requirement, int value,
                          int currentWeight);
} // namespace MuEditor::Items

#endif // _EDITOR
