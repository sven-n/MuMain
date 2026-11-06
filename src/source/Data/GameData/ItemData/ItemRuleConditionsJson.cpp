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
constexpr const char* Numbers = "numbers";
constexpr const char* Slot = "slot";
constexpr const char* WingTier = "wingTier";
constexpr const char* WeaponKind = "weaponKind";
constexpr const char* Tags = "tags";
constexpr const char* Requirements = "requirements";
} // namespace Keys

// Before a tag an item must not have: "!ammunition".
constexpr char WithoutMark = '!';

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

// A list of whole numbers from 0 to Size - 1: the groups, the item numbers.
template <size_t Size>
bool ReadNumbers(const OrderedJson& json, const std::string& field, const char* what, std::bitset<Size>& numbers,
                 const AddRuleConditionIssue& addFatal)
{
    if (!json.is_array() || json.empty())
    {
        addFatal(field, std::string("must be a list of one or more ") + what);
        return false;
    }
    bool valid = true;
    for (size_t index = 0; index < json.size(); ++index)
    {
        long long number = 0;
        if (!json[index].is_number_integer() || !Json::ReadWholeNumber(json[index], number) || number < 0 ||
            number >= static_cast<long long>(Size))
        {
            addFatal(field + "[" + std::to_string(index) + "]",
                     "must be a whole number from 0 to " + std::to_string(Size - 1));
            valid = false;
            continue;
        }
        numbers.set(static_cast<size_t>(number));
    }
    return valid;
}

// The tags of an option rule: a tag the item must have, or one it must not have ("!ammunition").
bool ReadTagsWithout(const OrderedJson& json, const std::string& field, ItemRuleConditions& conditions,
                     const AddRuleConditionIssue& addFatal)
{
    if (!json.is_array() || json.empty())
    {
        addFatal(field, "must be a list of one or more tags");
        return false;
    }
    bool valid = true;
    for (size_t index = 0; index < json.size(); ++index)
    {
        const OrderedJson& entry = json[index];
        const std::string* name = entry.is_string() ? &entry.get_ref<const std::string&>() : nullptr;
        const bool without = name != nullptr && !name->empty() && name->front() == WithoutMark;
        ItemTag tag{};
        if (name == nullptr || !FindEnumValue(without ? name->substr(1) : *name, tag))
        {
            addFatal(field + "[" + std::to_string(index) + "]",
                     entry.dump() + " is not a tag, or \"!\" and a tag; tags are " + JoinEnumNames<ItemTag>());
            valid = false;
            continue;
        }
        (without ? conditions.withoutTags : conditions.tags).Set(tag);
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

template <size_t Size> OrderedJson WriteNumbers(const std::bitset<Size>& numbers)
{
    OrderedJson json = OrderedJson::array();
    for (size_t number = 0; number < numbers.size(); ++number)
    {
        if (numbers.test(number))
        {
            json.push_back(number);
        }
    }
    return json;
}

// The tags an item must have, then those it must not have ("!ammunition").
OrderedJson WriteTags(const ItemRuleConditions& conditions)
{
    OrderedJson json = WriteValues<ItemTag>(conditions.tags);
    for (const OrderedJson& without : WriteValues<ItemTag>(conditions.withoutTags))
    {
        json.push_back(WithoutMark + without.get<std::string>());
    }
    return json;
}
} // namespace

RuleConditionRead ReadRuleCondition(std::string_view key, const OrderedJson& value, const std::string& field,
                                    RuleConditionSet set, ItemRuleConditions& conditions,
                                    const AddRuleConditionIssue& addFatal)
{
    const bool optionRule = set == RuleConditionSet::OptionRules;
    bool read = true;
    if (key == Keys::Group)
    {
        read = ReadNumbers(value, field, "groups", conditions.groups, addFatal);
    }
    else if (optionRule && key == Keys::Numbers)
    {
        read = ReadNumbers(value, field, "item numbers", conditions.numbers, addFatal);
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
        read = optionRule ? ReadTagsWithout(value, field, conditions, addFatal)
                          : ReadValues<ItemTag>(value, field, "tag", conditions.tags, addFatal);
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

std::string DescribeRuleConditionKeys(RuleConditionSet set)
{
    std::string text = std::string("is not a condition; the conditions are \"") + Keys::Group + "\", \"";
    if (set == RuleConditionSet::OptionRules)
    {
        text += std::string(Keys::Numbers) + "\", \"";
    }
    return text + Keys::Slot + "\", \"" + Keys::WingTier + "\", \"" + Keys::WeaponKind + "\", \"" + Keys::Tags +
           "\" and \"" + Keys::Requirements + "\"";
}

void WriteRuleConditions(const ItemRuleConditions& conditions, OrderedJson& json)
{
    if (conditions.groups.any())
    {
        json[Keys::Group] = WriteNumbers(conditions.groups);
    }
    if (conditions.numbers.any())
    {
        json[Keys::Numbers] = WriteNumbers(conditions.numbers);
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
    if (!conditions.tags.IsEmpty() || !conditions.withoutTags.IsEmpty())
    {
        json[Keys::Tags] = WriteTags(conditions);
    }
    if (conditions.requirements.any())
    {
        json[Keys::Requirements] = WriteValues<ItemRequirement>(conditions.requirements);
    }
}
} // namespace Data::Items
