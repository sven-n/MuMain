#pragma once

#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/ItemData/ItemRuleConditions.h"

#include <functional>
#include <string>
#include <string_view>

// Reads and writes the conditions of a rule (ItemRuleConditions) in the files that have rules: the profile rules of
// the item stat file and the option rules. A condition is a key with a list of one or more values:
//   {"group": [7, 8], "slot": ["helm"], "wingTier": ["second"], "weaponKind": ["bow"], "tags": ["cape"],
//    "requirements": ["level"]}
// The option rules also have "numbers" (item numbers, with "group") and tags an item must not have ("!ammunition").
namespace Data::Items
{
// Adds a problem of a condition, its field and message; the readers add them as fatal, since a condition left out
// would widen the rule.
using AddRuleConditionIssue = std::function<void(const std::string& field, const std::string& message)>;

// Which conditions a file's rules have: the profile rules have no numbers and no "!tag", so their format stays.
enum class RuleConditionSet : BYTE
{
    ProfileRules,
    OptionRules,
};

enum class RuleConditionRead : BYTE
{
    Read,
    // The key is a condition, but its values are not; their problems were added.
    Invalid,
    // The key is no condition of the set.
    NotACondition,
};

// Reads the condition `key` of a rule into `conditions`; `field` names it in messages.
RuleConditionRead ReadRuleCondition(std::string_view key, const Json::OrderedJson& value, const std::string& field,
                                    RuleConditionSet set, ItemRuleConditions& conditions,
                                    const AddRuleConditionIssue& addFatal);

// The message for a key that is no condition of the set: it names the set's conditions.
std::string DescribeRuleConditionKeys(RuleConditionSet set);

// Writes the conditions the rule has, in this order: group, numbers, slot, wingTier, weaponKind, tags (then the ones
// an item must not have), requirements.
void WriteRuleConditions(const ItemRuleConditions& conditions, Json::OrderedJson& json);
} // namespace Data::Items
