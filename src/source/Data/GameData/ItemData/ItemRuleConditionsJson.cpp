#include "stdafx.h"

#include "ItemRuleConditionsJson.h"
#include "ItemEnumNames.h"

namespace Data::Items
{
namespace
{
using Json::OrderedJson;

namespace Keys
{
constexpr const char* Group = "group";
constexpr const char* Slot = "slot";
constexpr const char* WingTier = "wingTier";
constexpr const char* WeaponKind = "weaponKind";
constexpr const char* Tags = "tags";
constexpr const char* Requirements = "requirements";
} // namespace Keys

void AddValue(ItemTagSet& values, ItemTag tag)
{
    values.Set(tag);
}

template <size_t Size, typename TEnum> void AddValue(std::bitset<Size>& values, TEnum value)
{
    values.set(static_cast<size_t>(value));
}

bool HasValue(const ItemTagSet& values, ItemTag tag)
{
    return values.Has(tag);
}

template <size_t Size, typename TEnum> bool HasValue(const std::bitset<Size>& values, TEnum value)
{
    const auto index = static_cast<size_t>(value);
    return index < Size && values.test(index);
}

// A condition's list of values: the names of `TEnum`, `what` being their kind ("slot") for messages.
template <typename TEnum, typename TSet>
bool ReadValues(const OrderedJson& json, const std::string& field, const char* what, TSet& values,
                const AddRuleConditionIssue& addFatal)
{
    if (!json.is_array() || json.empty())
    {
        addFatal(field, std::string("must be a list of one or more ") + what + "s");
        return false;
    }
    bool valid = true;
    for (size_t index = 0; index < json.size(); ++index)
    {
        const OrderedJson& entry = json[index];
        TEnum value{};
        if (!entry.is_string() || !FindEnumValue(entry.get_ref<const std::string&>(), value))
        {
            addFatal(field + "[" + std::to_string(index) + "]",
                     entry.dump() + " is not a " + what + "; " + what + "s are " + JoinEnumNames<TEnum>());
            valid = false;
            continue;
        }
        AddValue(values, value);
    }
    return valid;
}

bool ReadGroups(const OrderedJson& json, const std::string& field, std::bitset<MAX_ITEM_TYPE>& groups,
                const AddRuleConditionIssue& addFatal)
{
    if (!json.is_array() || json.empty())
    {
        addFatal(field, "must be a list of one or more groups");
        return false;
    }
    bool valid = true;
    for (size_t index = 0; index < json.size(); ++index)
    {
        long long group = 0;
        if (!json[index].is_number_integer() || !Json::ReadWholeNumber(json[index], group) || group < 0 ||
            group >= MAX_ITEM_TYPE)
        {
            addFatal(field + "[" + std::to_string(index) + "]",
                     "must be a whole number from 0 to " + std::to_string(MAX_ITEM_TYPE - 1));
            valid = false;
            continue;
        }
        groups.set(static_cast<size_t>(group));
    }
    return valid;
}

// The names of the values, in the order of the enum's names.
template <typename TEnum, typename TSet> OrderedJson WriteValues(const TSet& values)
{
    OrderedJson json = OrderedJson::array();
    for (const EnumName<TEnum>& entry : GetEnumNames(TEnum{}))
    {
        if (HasValue(values, entry.value))
        {
            json.push_back(entry.name);
        }
    }
    return json;
}

OrderedJson WriteGroups(const std::bitset<MAX_ITEM_TYPE>& groups)
{
    OrderedJson json = OrderedJson::array();
    for (size_t group = 0; group < groups.size(); ++group)
    {
        if (groups.test(group))
        {
            json.push_back(group);
        }
    }
    return json;
}
} // namespace

RuleConditionRead ReadRuleCondition(std::string_view key, const OrderedJson& value, const std::string& field,
                                    ItemRuleConditions& conditions, const AddRuleConditionIssue& addFatal)
{
    bool read = true;
    if (key == Keys::Group)
    {
        read = ReadGroups(value, field, conditions.groups, addFatal);
    }
    else if (key == Keys::Slot)
    {
        read = ReadValues<ItemSlot>(value, field, "slot", conditions.slots, addFatal);
    }
    else if (key == Keys::WingTier)
    {
        read = ReadValues<WingTier>(value, field, "wing tier", conditions.wingTiers, addFatal);
    }
    else if (key == Keys::WeaponKind)
    {
        read = ReadValues<WeaponKind>(value, field, "weapon kind", conditions.weaponKinds, addFatal);
    }
    else if (key == Keys::Tags)
    {
        read = ReadValues<ItemTag>(value, field, "tag", conditions.tags, addFatal);
    }
    else if (key == Keys::Requirements)
    {
        read = ReadValues<ItemRequirement>(value, field, "requirement", conditions.requirements, addFatal);
    }
    else
    {
        return RuleConditionRead::NotACondition;
    }
    return read ? RuleConditionRead::Read : RuleConditionRead::Invalid;
}

std::string DescribeRuleConditionKeys(std::string_view ownKeys)
{
    std::string text = std::string("is not a condition; the conditions are \"") + Keys::Group + "\", \"" + Keys::Slot +
                       "\", \"" + Keys::WingTier + "\", \"" + Keys::WeaponKind + "\", \"" + Keys::Tags + "\"";
    if (!ownKeys.empty())
    {
        text += ", \"" + std::string(Keys::Requirements) + "\" and " + std::string(ownKeys);
        return text;
    }
    return text + " and \"" + Keys::Requirements + "\"";
}

void WriteRuleConditions(const ItemRuleConditions& conditions, OrderedJson& json)
{
    if (conditions.groups.any())
    {
        json[Keys::Group] = WriteGroups(conditions.groups);
    }
    if (conditions.slots.any())
    {
        json[Keys::Slot] = WriteValues<ItemSlot>(conditions.slots);
    }
    if (conditions.wingTiers.any())
    {
        json[Keys::WingTier] = WriteValues<WingTier>(conditions.wingTiers);
    }
    if (conditions.weaponKinds.any())
    {
        json[Keys::WeaponKind] = WriteValues<WeaponKind>(conditions.weaponKinds);
    }
    if (!conditions.tags.IsEmpty())
    {
        json[Keys::Tags] = WriteValues<ItemTag>(conditions.tags);
    }
    if (conditions.requirements.any())
    {
        json[Keys::Requirements] = WriteValues<ItemRequirement>(conditions.requirements);
    }
}
} // namespace Data::Items
