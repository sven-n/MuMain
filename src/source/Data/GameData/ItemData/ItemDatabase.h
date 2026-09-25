#pragma once

#include "Data/GameData/ItemData/ItemDefinition.h"
#include "Data/GameData/ItemData/ItemStructs.h"
#include "Data/GameData/ItemData/ItemType.h"

#include <span>
#include <string>
#include <vector>

namespace Data::Items
{
// In-memory item database. Built once from the loaded item data and read-only
// afterwards. Lookups index a flat table by item type, so they cost the same
// as reading ItemAttribute[itemType].
class ItemDatabase
{
public:
    static ItemDatabase& GetInstance();

    ItemDatabase();

    // Rebuilds all definitions from the given attributes (one per item type).
    // englishNames is either empty, then the attribute names are used as
    // English names, or holds one UTF-8 name per item type.
    void Build(std::span<const ITEM_ATTRIBUTE> attributes, std::span<const std::string> englishNames = {});

    // Returns nullptr for invalid ids and for empty item slots. Defined here
    // so it can be inlined on hot paths.
    const ItemDefinition* Find(int itemType) const
    {
        if (!IsValidItemType(itemType))
        {
            return nullptr;
        }

        const ItemDefinition& definition = m_definitions[itemType];
        return definition.Exists() ? &definition : nullptr;
    }

    const ItemDefinition* Find(int group, int number) const;

    // Stable, language-neutral name for logs: "<English name> (<group>,<number>)".
    std::string GetLogName(int itemType) const;

    int GetExistingItemCount() const { return m_existingItemCount; }

private:
    std::vector<ItemDefinition> m_definitions;
    int m_existingItemCount = 0;
};
} // namespace Data::Items

#define g_ItemDatabase Data::Items::ItemDatabase::GetInstance()
