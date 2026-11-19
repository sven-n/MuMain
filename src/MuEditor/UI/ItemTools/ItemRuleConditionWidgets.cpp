#include "stdafx.h"

#ifdef _EDITOR

#include "ItemRuleConditionWidgets.h"

#include "ItemFieldWidgets.h"
#include "ItemGroupNames.h"
#include "ItemTextInput.h"

#include "Data/GameData/ItemData/ItemEnumNames.h"
#include "I18N/All.h"
#include "imgui.h"

#include <bitset>
#include <cctype>
#include <string>
#include <vector>

namespace MuEditor::Items
{
namespace
{
std::string JoinNames(const std::vector<std::string>& names)
{
    std::string text;
    for (const std::string& name : names)
    {
        text += (text.empty() ? "" : ", ") + name;
    }
    return text;
}

bool HasValue(const Data::Items::ItemTagSet& values, Data::Items::ItemTag tag)
{
    return values.Has(tag);
}

void SetValue(Data::Items::ItemTagSet& values, Data::Items::ItemTag tag, bool on)
{
    values.Set(tag, on);
}

template <size_t Size, typename TEnum> bool HasValue(const std::bitset<Size>& values, TEnum value)
{
    const auto index = static_cast<size_t>(value);
    return index < Size && values.test(index);
}

template <size_t Size, typename TEnum> void SetValue(std::bitset<Size>& values, TEnum value, bool on)
{
    values.set(static_cast<size_t>(value), on);
}

// The names of the values a condition has, in the order of the enum's names.
template <typename TEnum, typename TSet> std::string DescribeValues(const TSet& values)
{
    std::vector<std::string> names;
    for (const Data::Items::EnumName<TEnum>& entry : Data::Items::GetEnumNames(TEnum{}))
    {
        if (HasValue(values, entry.value))
        {
            names.push_back(entry.name);
        }
    }
    return JoinNames(names);
}

std::string DescribeGroups(const std::bitset<MAX_ITEM_TYPE>& groups)
{
    std::vector<std::string> names;
    for (size_t group = 0; group < groups.size(); ++group)
    {
        if (groups.test(group))
        {
            names.push_back(std::to_string(group));
        }
    }
    return JoinNames(names);
}

std::string DescribeNumbers(const std::bitset<MAX_ITEM_INDEX>& numbers)
{
    std::vector<std::string> names;
    for (size_t number = 0; number < numbers.size(); ++number)
    {
        if (numbers.test(number))
        {
            names.push_back(std::to_string(number));
        }
    }
    return JoinNames(names);
}

// The numbers of "0, 1, 41-49" (a range by its ends); what isn't a number in range is left out.
std::bitset<MAX_ITEM_INDEX> ParseNumbers(const std::string& text)
{
    std::bitset<MAX_ITEM_INDEX> numbers;
    size_t at = 0;
    while (at < text.size())
    {
        while (at < text.size() && !std::isdigit(static_cast<unsigned char>(text[at])))
        {
            ++at;
        }
        size_t end = at;
        while (end < text.size() && std::isdigit(static_cast<unsigned char>(text[end])))
        {
            ++end;
        }
        if (end == at || end - at > 4)
        {
            at = end + 1;
            continue;
        }
        const int first = std::stoi(text.substr(at, end - at));
        int last = first;
        if (end < text.size() && text[end] == '-')
        {
            size_t lastEnd = end + 1;
            while (lastEnd < text.size() && std::isdigit(static_cast<unsigned char>(text[lastEnd])))
            {
                ++lastEnd;
            }
            if (lastEnd > end + 1 && lastEnd - end - 1 <= 4)
            {
                last = std::stoi(text.substr(end + 1, lastEnd - end - 1));
            }
            end = lastEnd;
        }
        for (int number = first; number <= last && number < MAX_ITEM_INDEX; ++number)
        {
            numbers.set(static_cast<size_t>(number));
        }
        at = end;
    }
    return numbers;
}

// A condition as a list to tick its values in; true when it changed.
template <typename TEnum, typename TSet> bool ConditionCombo(const char* label, TSet& values)
{
    bool changed = false;
    const std::string preview = DescribeValues<TEnum>(values);
    ImGui::SetNextItemWidth(Fields::ScaledWidth(Fields::NumberWidth * 3.0f));
    if (ImGui::BeginCombo(label, preview.empty() ? I18N::Editor::Any : preview.c_str()))
    {
        for (const Data::Items::EnumName<TEnum>& entry : Data::Items::GetEnumNames(TEnum{}))
        {
            const bool on = HasValue(values, entry.value);
            if (ImGui::Selectable(entry.name, on, ImGuiSelectableFlags_NoAutoClosePopups))
            {
                SetValue(values, entry.value, !on);
                changed = true;
            }
        }
        ImGui::EndCombo();
    }
    return changed;
}

} // namespace

std::string DescribeRuleConditions(const Data::Items::ItemRuleConditions& conditions)
{
    std::vector<std::string> parts;
    const auto add = [&parts](const char* label, const std::string& values)
    {
        if (!values.empty())
        {
            parts.push_back(std::string(label) + ": " + values);
        }
    };
    add(I18N::Editor::Group, DescribeGroups(conditions.groups));
    add(I18N::Editor::ItemNumbers, DescribeNumbers(conditions.numbers));
    add(I18N::Metadata::Slot, DescribeValues<Data::Items::ItemSlot>(conditions.slots));
    add(I18N::Editor::WingTier, DescribeValues<Data::Items::WingTier>(conditions.wingTiers));
    add(I18N::Editor::WeaponKind, DescribeValues<Data::Items::WeaponKind>(conditions.weaponKinds));
    add(I18N::Editor::Tags, DescribeValues<Data::Items::ItemTag>(conditions.tags));
    add(I18N::Editor::WithoutTags, DescribeValues<Data::Items::ItemTag>(conditions.withoutTags));
    add(I18N::Editor::BaseRequirementAbove0, DescribeValues<Data::Items::ItemRequirement>(conditions.requirements));
    if (parts.empty())
    {
        return I18N::Editor::EveryItem;
    }
    std::string text;
    for (const std::string& part : parts)
    {
        text += (text.empty() ? "" : "; ") + part;
    }
    return text;
}

void RenderRuleConditionFields(Data::Items::ItemRuleConditions& conditions, bool optionRuleConditions)
{
    std::string groups = DescribeGroups(conditions.groups);
    ImGui::SetNextItemWidth(Fields::ScaledWidth(Fields::NumberWidth * 3.0f));
    if (ImGui::BeginCombo(I18N::Editor::Group, groups.empty() ? I18N::Editor::Any : groups.c_str()))
    {
        for (int group = 0; group < MAX_ITEM_TYPE; ++group)
        {
            const bool on = conditions.groups.test(static_cast<size_t>(group));
            if (ImGui::Selectable(GetItemGroupLabel(group).c_str(), on, ImGuiSelectableFlags_NoAutoClosePopups))
            {
                conditions.groups.set(static_cast<size_t>(group), !on);
            }
        }
        ImGui::EndCombo();
    }
    if (optionRuleConditions)
    {
        std::string numbers = DescribeNumbers(conditions.numbers);
        ImGui::SetNextItemWidth(Fields::ScaledWidth(Fields::NumberWidth * 3.0f));
        if (InputStringWithHint(I18N::Editor::ItemNumbers, I18N::Editor::Any, numbers))
        {
            conditions.numbers = ParseNumbers(numbers);
        }
    }
    ConditionCombo<Data::Items::ItemSlot>(I18N::Metadata::Slot, conditions.slots);
    ConditionCombo<Data::Items::WingTier>(I18N::Editor::WingTier, conditions.wingTiers);
    ConditionCombo<Data::Items::WeaponKind>(I18N::Editor::WeaponKind, conditions.weaponKinds);
    ConditionCombo<Data::Items::ItemTag>(I18N::Editor::Tags, conditions.tags);
    if (optionRuleConditions)
    {
        ConditionCombo<Data::Items::ItemTag>(I18N::Editor::WithoutTags, conditions.withoutTags);
    }
    ConditionCombo<Data::Items::ItemRequirement>(I18N::Editor::BaseRequirementAbove0, conditions.requirements);
}
} // namespace MuEditor::Items

#endif // _EDITOR
