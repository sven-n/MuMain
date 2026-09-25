#include "stdafx.h"

#include "ItemDatabase.h"
#include "ItemAttributeConversion.h"

#include <algorithm>

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

ItemDatabase::ItemDatabase()
    : m_definitions(MAX_ITEM)
{
}

void ItemDatabase::Build(std::span<const ITEM_ATTRIBUTE> attributes, std::span<const std::string> englishNames)
{
    const bool hasEnglishNames = englishNames.size() == attributes.size();
    const size_t itemCount = std::min(attributes.size(), static_cast<size_t>(MAX_ITEM));

    std::fill(m_definitions.begin(), m_definitions.end(), ItemDefinition{});
    m_existingItemCount = 0;

    for (size_t itemType = 0; itemType < itemCount; ++itemType)
    {
        ItemDefinition& definition = m_definitions[itemType];
        definition = ToItemDefinition(attributes[itemType], static_cast<int>(itemType));
        if (hasEnglishNames && !englishNames[itemType].empty())
        {
            definition.englishName = englishNames[itemType];
        }

        if (definition.Exists())
        {
            ++m_existingItemCount;
        }
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

    return definition->englishName + itemId;
}
} // namespace Data::Items
