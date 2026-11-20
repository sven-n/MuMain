#pragma once

#ifdef _EDITOR

#include "Data/GameData/ItemData/ItemRuleConditions.h"

#include <string>

// The conditions of a rule that gives items something by their data, as Item Formulas (the profile rules) and Item
// Options (the option rules) show and edit them.
namespace MuEditor::Items
{
// "slot: helm, boots; group: 15", or that the rule matches every item.
std::string DescribeRuleConditions(const Data::Items::ItemRuleConditions& conditions);

// A list per condition to tick its values in; with `optionRuleConditions` also the two only the option rules have:
// the item numbers and the tags an item must not have.
void RenderRuleConditionFields(Data::Items::ItemRuleConditions& conditions, bool optionRuleConditions);
} // namespace MuEditor::Items

#endif // _EDITOR
