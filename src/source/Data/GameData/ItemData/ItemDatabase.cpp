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
    for (int itemType = 0; itemType < MAX_ITEM; ++itemType)
    {
        Store(itemType, ItemDefinition{});
    }
    for (const ItemDefinition& definition : definitions)
    {
        if (IsValidItemId(definition.group, definition.number))
        {
            Store(MakeItemType(definition.group, definition.number), definition);
        }
    }
    CountExistingItems();
    BuildItemTypesByTag();
}

void ItemDatabase::SetDisplayLocale(std::string_view locale)
{
    if (m_displayLocale == locale)
    {
        return;
    }
    ++m_nameVersion;
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

const ItemDefinition* ItemDatabase::FindEntry(int group, int number) const
{
    if (!IsValidItemId(group, number))
    {
        return nullptr;
    }

    return FindEntry(MakeItemType(group, number));
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
    Store(MakeItemType(definition.group, definition.number), definition);
    CountExistingItems();
    BuildItemTypesByTag();
}

void ItemDatabase::SetMany(std::span<const ItemDefinition> definitions)
{
    for (const ItemDefinition& definition : definitions)
    {
        if (IsValidItemId(definition.group, definition.number))
        {
            Store(MakeItemType(definition.group, definition.number), definition);
        }
    }
    CountExistingItems();
    BuildItemTypesByTag();
}

void ItemDatabase::Swap(int firstItemType, int secondItemType)
{
    if (!IsValidItemType(firstItemType) || !IsValidItemType(secondItemType))
    {
        return;
    }

    ItemDefinition first = m_definitions[firstItemType];
    ItemDefinition second = m_definitions[secondItemType];
    Store(firstItemType, std::move(second));
    Store(secondItemType, std::move(first));
    BuildItemTypesByTag();
}

void ItemDatabase::SetStatProfiles(ItemStatProfiles profiles)
{
    ++m_version;
    m_statProfiles = std::move(profiles);
    for (size_t itemType = 0; itemType < m_definitions.size(); ++itemType)
    {
        if (m_definitions[itemType].Exists())
        {
            m_ruleData[itemType].statProfile = ResolveStatProfile(m_definitions[itemType].statProfile);
        }
    }
}

int ItemDatabase::ResolveStatProfile(const std::string& name) const
{
    const int profile = m_statProfiles.Find(name);
    return profile == ItemStatProfiles::UnknownProfile ? ItemStatProfiles::DefaultProfile : profile;
}

void ItemDatabase::BuildItemTypesByTag()
{
    for (std::vector<int>& itemTypes : m_itemTypesByTag)
    {
        itemTypes.clear();
    }
    for (int itemType = 0; itemType < static_cast<int>(m_ruleData.size()); ++itemType)
    {
        const ItemTagSet& tags = m_ruleData[itemType].tags;
        for (size_t tagIndex = 0; tagIndex < m_itemTypesByTag.size(); ++tagIndex)
        {
            if (tags.Has(static_cast<ItemTag>(tagIndex)))
            {
                m_itemTypesByTag[tagIndex].push_back(itemType);
            }
        }
    }

    const auto higherPriority = [this](int firstItemType, int secondItemType)
    { return m_definitions[firstItemType].potionPriority > m_definitions[secondItemType].potionPriority; };
    for (size_t tagIndex = 0; tagIndex < m_itemTypesByTag.size(); ++tagIndex)
    {
        const std::vector<int>& itemTypes = m_itemTypesByTag[tagIndex];
        std::vector<int>& inUseOrder = m_itemTypesByTagInUseOrder[tagIndex];
        inUseOrder.assign(itemTypes.rbegin(), itemTypes.rend());
        std::stable_sort(inUseOrder.begin(), inUseOrder.end(), higherPriority);
    }
}

void ItemDatabase::UpdateDisplayName(ItemDefinition& definition) const
{
    definition.name =
        definition.Exists() ? Core::Text::FromUtf8(definition.names.Get(m_displayLocale)) : std::wstring();
}

void ItemDatabase::Store(int itemType, ItemDefinition definition)
{
    ++m_version;
    ++m_nameVersion;
    definition.group = GetItemGroup(itemType);
    definition.number = GetItemNumber(itemType);
    // A name makes the entry an item.
    if (definition.Exists())
    {
        definition.entryWithoutName = false;
    }
    UpdateDisplayName(definition);

    RuleData ruleData;
    if (definition.Exists())
    {
        ruleData.tags = definition.tags;
        ruleData.slot = definition.slot;
        ruleData.wingTier = definition.wingTier;
        ruleData.weaponKind = definition.weaponKind;
        ruleData.statProfile = ResolveStatProfile(definition.statProfile);
        ruleData.blockedActions = 0;
        for (int action = 0; action < static_cast<int>(ItemAction::Count); ++action)
        {
            if (!definition.IsAllowed(static_cast<ItemAction>(action)))
            {
                ruleData.blockedActions |= ActionBit(static_cast<ItemAction>(action));
            }
        }
    }

    m_definitions[itemType] = std::move(definition);
    m_ruleData[itemType] = ruleData;
}

void ItemDatabase::CountExistingItems()
{
    m_existingItemCount =
        static_cast<int>(std::count_if(m_definitions.begin(), m_definitions.end(),
                                       [](const ItemDefinition& definition) { return definition.Exists(); }));
}
} // namespace Data::Items
