#include "stdafx.h"

#include "ItemDatabase.h"
#include "Core/Text/Utf8.h"

#include <algorithm>
#include <utility>

namespace Data::Items
{
namespace
{
constexpr const char* UnknownItemLogName = "<unknown item>";
constexpr const char* InvalidItemTypeLogName = "<invalid item type ";

std::string FormatItemId(int group, int number)
{
    return " (" + std::to_string(group) + "," + std::to_string(number) + ")";
}
} // namespace

ItemDatabase& ItemDatabase::GetInstance()
{
    static ItemDatabase instance;
    return instance;
}

ItemDatabase::ItemDatabase() : m_definitions(MAX_ITEM), m_ruleData(MAX_ITEM) {}

void ItemDatabase::Build(std::span<const ItemDefinition> definitions)
{
    std::fill(m_definitions.begin(), m_definitions.end(), ItemDefinition{});
    for (const ItemDefinition& definition : definitions)
    {
        if (!IsValidItemId(definition.group, definition.number))
        {
            continue;
        }

        ItemDefinition& slot = m_definitions[MakeItemType(definition.group, definition.number)];
        slot = definition;
        UpdateDisplayName(slot);
    }

    for (int itemType = 0; itemType < MAX_ITEM; ++itemType)
    {
        UpdateRuleData(itemType);
    }
    CountExistingItems();
}

void ItemDatabase::SetDisplayLocale(std::string_view locale)
{
    m_displayLocale = std::string(locale);
    for (ItemDefinition& definition : m_definitions)
    {
        UpdateDisplayName(definition);
    }
}

const ItemDefinition* ItemDatabase::Find(int group, int number) const
{
    if (!IsValidItemId(group, number))
    {
        return nullptr;
    }

    return Find(MakeItemType(group, number));
}

std::string ItemDatabase::GetLogName(int itemType) const
{
    if (!IsValidItemType(itemType))
    {
        return InvalidItemTypeLogName + std::to_string(itemType) + ">";
    }

    const std::string itemId = FormatItemId(GetItemGroup(itemType), GetItemNumber(itemType));
    const ItemDefinition* definition = Find(itemType);
    if (definition == nullptr)
    {
        return UnknownItemLogName + itemId;
    }

    return definition->names.GetNeutral() + itemId;
}

void ItemDatabase::Set(const ItemDefinition& definition)
{
    if (!IsValidItemId(definition.group, definition.number))
    {
        return;
    }

    // Stats are kept even without names, so an item whose name is cleared
    // and typed again in the editor keeps its values.
    const int itemType = MakeItemType(definition.group, definition.number);
    ItemDefinition& slot = m_definitions[itemType];
    slot = definition;
    UpdateDisplayName(slot);
    UpdateRuleData(itemType);
    CountExistingItems();
}

void ItemDatabase::Swap(int firstItemType, int secondItemType)
{
    if (!IsValidItemType(firstItemType) || !IsValidItemType(secondItemType))
    {
        return;
    }

    ItemDefinition& first = m_definitions[firstItemType];
    ItemDefinition& second = m_definitions[secondItemType];
    std::swap(first, second);
    first.group = GetItemGroup(firstItemType);
    first.number = GetItemNumber(firstItemType);
    second.group = GetItemGroup(secondItemType);
    second.number = GetItemNumber(secondItemType);
    UpdateRuleData(firstItemType);
    UpdateRuleData(secondItemType);
}

void ItemDatabase::UpdateDisplayName(ItemDefinition& definition) const
{
    definition.name =
        definition.Exists() ? Core::Text::FromUtf8(definition.names.Get(m_displayLocale)) : std::wstring();
}

void ItemDatabase::UpdateRuleData(int itemType)
{
    const ItemDefinition& definition = m_definitions[itemType];
    RuleData& ruleData = m_ruleData[itemType];
    ruleData = RuleData{};
    if (!definition.Exists())
    {
        return;
    }

    ruleData.tags = definition.tags;
    ruleData.slot = definition.slot;
    ruleData.wingTier = definition.wingTier;
    for (int action = 0; action < static_cast<int>(ItemAction::Count); ++action)
    {
        if (!definition.IsAllowed(static_cast<ItemAction>(action)))
        {
            ruleData.blockedActions |= ActionBit(static_cast<ItemAction>(action));
        }
    }
}

void ItemDatabase::CountExistingItems()
{
    m_existingItemCount =
        static_cast<int>(std::count_if(m_definitions.begin(), m_definitions.end(),
                                       [](const ItemDefinition& definition) { return definition.Exists(); }));
}
} // namespace Data::Items
