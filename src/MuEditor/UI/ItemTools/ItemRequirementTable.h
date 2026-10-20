#pragma once

#ifdef _EDITOR

#include "Data/GameData/ItemData/ItemDefinition.h"

#include <array>

namespace MuEditor::Items
{
// What the game adds to an item's requirements besides its + level.
struct RequirementVariant
{
    bool excellent = false;
    bool ancient = false;
    int optionLevel = 0;
};

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

// The base value of `requirement` with which `item`, without that requirement
// by level, has `values` at every + level as a normal item: the level and a
// base kept as it is are the +0 value; a stat the smallest base that gives
// all 16 values, else the smallest that gives the +0 value, else the base
// whose +0 value is nearest.
WORD FindRequirementBase(const Data::Items::ItemDefinition& item, Data::Items::ItemRequirement requirement,
                         const Data::Items::RequirementByLevel& values);
} // namespace MuEditor::Items

#endif // _EDITOR
