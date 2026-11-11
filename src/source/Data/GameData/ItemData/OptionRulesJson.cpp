#include "stdafx.h"

#include "OptionRulesJson.h"
#include "ItemJsonCommon.h"
#include "ItemRuleConditionsJson.h"

namespace Data::Items
{
namespace
{
using Json::OrderedJson;

constexpr const char* RulesKey = "rules";
constexpr const char* OptionsKey = "options";
constexpr const char* GroupKey = "group";
constexpr const char* NumbersKey = "numbers";

void AddFatal(std::vector<ItemDataIssue>& issues, const std::string& source, const std::string& field,
              const std::string& message)
{
    Json::AddFileIssue(issues, source, ItemDataIssue::NoItem, field, message);
}

bool ReadOptions(const OrderedJson& json, const std::string& field, const std::string& source,
                 std::vector<std::string>& options, std::vector<ItemDataIssue>& issues)
{
    const auto list = json.find(OptionsKey);
    if (list == json.end() || !list->is_array())
    {
        AddFatal(issues, source, field + "." + OptionsKey,
                 "missing or not a list of option definitions (an empty list gives no options)");
        return false;
    }
    for (const OrderedJson& entry : *list)
    {
        if (!entry.is_string() || !Json::IsName(entry.get_ref<const std::string&>()))
        {
            AddFatal(issues, source, field + "." + OptionsKey, "must be a list of option definition names");
            return false;
        }
        options.push_back(entry.get<std::string>());
    }
    return true;
}

bool ReadRule(const OrderedJson& json, const std::string& field, const std::string& source, OptionRule& rule,
              std::vector<ItemDataIssue>& issues)
{
    if (!json.is_object())
    {
        AddFatal(issues, source, field, "a rule must be an object");
        return false;
    }
    bool valid = ReadOptions(json, field, source, rule.options, issues);
    // An unknown condition is fatal: leaving it out would widen the rule.
    for (const auto& [key, value] : json.items())
    {
        if (key == OptionsKey)
        {
            continue;
        }
        const std::string conditionField = field + "." + key;
        const RuleConditionRead read =
            ReadRuleCondition(key, value, conditionField, RuleConditionSet::OptionRules, rule.conditions,
                              [&](const std::string& errorField, const std::string& message)
                              { AddFatal(issues, source, errorField, message); });
        if (read == RuleConditionRead::NotACondition)
        {
            AddFatal(issues, source, conditionField, DescribeRuleConditionKeys(RuleConditionSet::OptionRules));
        }
        valid = valid && read == RuleConditionRead::Read;
    }
    if (rule.conditions.numbers.any() && rule.conditions.groups.none())
    {
        AddFatal(issues, source, field + "." + NumbersKey,
                 std::string("item numbers need a \"") + GroupKey + "\": a number is in every group");
        valid = false;
    }
    return valid;
}
} // namespace

void ReadOptionRulesJson(std::string_view text, const std::string& source, std::vector<OptionRule>& rules,
                         std::vector<ItemDataIssue>& issues)
{
    OrderedJson root;
    if (!Json::ReadFileVersion(text, source, OptionRulesFormatVersion, root, issues))
    {
        return;
    }
    const auto list = root.find(RulesKey);
    if (list == root.end() || !list->is_array())
    {
        AddFatal(
            issues, source, RulesKey,
            R"(missing or not a list of rules {"options": [...], <conditions>}, the first that matches gives them)");
        return;
    }
    for (size_t index = 0; index < list->size(); ++index)
    {
        OptionRule rule;
        if (ReadRule((*list)[index], std::string(RulesKey) + "[" + std::to_string(index) + "]", source, rule, issues))
        {
            rules.push_back(std::move(rule));
        }
    }
    Json::WarnAboutUnknownKeys(root, Json::KeySet{Json::Keys::FormatVersion, RulesKey}, "",
                               [&](const std::string& unknown, const std::string& message)
                               {
                                   issues.push_back({ItemDataIssueSeverity::Warning, source, ItemDataIssue::NoItem,
                                                     ItemDataIssue::NoItem, unknown, message});
                               });
}

std::string WriteOptionRulesJson(std::span<const OptionRule> rules)
{
    OrderedJson root;
    root[Json::Keys::FormatVersion] = OptionRulesFormatVersion;
    root[RulesKey] = OrderedJson::array();
    for (const OptionRule& rule : rules)
    {
        OrderedJson json;
        json[OptionsKey] = rule.options;
        WriteRuleConditions(rule.conditions, json);
        root[RulesKey].push_back(std::move(json));
    }
    const std::string text = root.dump(Json::Indent, ' ', false, OrderedJson::error_handler_t::replace);
    return Json::PutListEntriesOnOneLine(text, RulesKey) + "\n";
}
} // namespace Data::Items
