#pragma once

#ifdef _EDITOR

#include "ItemValueFormulas.h"

#include "Data/GameData/ItemData/ItemDefinition.h"

#include <string>

// The formula of each value of an item as one line, with the item's numbers
// and where each part comes from ("[default]", "[weapon]", "[item:
// excellentBonus]"), for Item Basics and Item Requirements.
namespace MuEditor::Items
{
struct FormulaLine
{
    std::string text;
    // More about the line; may be empty.
    std::string tooltip;
};

// "default", the profile, "item: excellentBonus" or "dropLevelBonus".
std::string DescribeFormulaSource(const FormulaSource& source);

// The formula of a stat of `item` under `profile`: its level bonus, excellent
// and ancient parts. `maxDamage` picks the maximum damage for ItemStat::Damage.
FormulaLine DescribeStatFormula(const Data::Items::ItemDefinition& item, Data::Items::ItemStat stat, bool maxDamage,
                                const ExpandedStatProfile& profile);

// The formula of a requirement of `item` under `profile`.
FormulaLine DescribeRequirementFormula(const Data::Items::ItemDefinition& item,
                                       Data::Items::ItemRequirement requirement, const ExpandedStatProfile& profile);
} // namespace MuEditor::Items

#endif // _EDITOR
