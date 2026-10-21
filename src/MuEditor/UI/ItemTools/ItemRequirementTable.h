#pragma once

#ifdef _EDITOR

#include "Data/GameData/ItemData/ItemDefinition.h"
#include "Data/GameData/ItemData/ItemStatProfiles.h"
#include "GameLogic/Items/ItemStats.h"

#include <array>
#include <cstdint>
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
} // namespace MuEditor::Items

#endif // _EDITOR
