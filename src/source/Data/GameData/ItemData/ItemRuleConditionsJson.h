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
namespace Data::Items
{
// Adds a problem of a condition, its field and message; the readers add them as fatal, since a condition left out
// would widen the rule.
using AddRuleConditionIssue = std::function<void(const std::string& field, const std::string& message)>;

enum class RuleConditionRead : BYTE
{
    Read,
    // The key is a condition, but its values are not; their problems were added.
    Invalid,
    // The key is no condition of a rule.
    NotACondition,
};

// Reads the condition `key` of a rule into `conditions`; `field` names it in messages.
RuleConditionRead ReadRuleCondition(std::string_view key, const Json::OrderedJson& value, const std::string& field,
                                    ItemRuleConditions& conditions, const AddRuleConditionIssue& addFatal);

// The message for a key that is no condition: it names the conditions, with the reader's own keys after them.
std::string DescribeRuleConditionKeys(std::string_view ownKeys = {});

// Writes the conditions the rule has, in this order: group, slot, wingTier, weaponKind, tags, requirements.
void WriteRuleConditions(const ItemRuleConditions& conditions, Json::OrderedJson& json);
} // namespace Data::Items
