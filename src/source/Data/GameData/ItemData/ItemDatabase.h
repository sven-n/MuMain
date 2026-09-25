#pragma once

#include "Data/GameData/ItemData/ItemDefinition.h"
#include "Data/GameData/ItemData/ItemType.h"

#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace Data::Items
{
// In-memory item database: one definition per item type in a flat table.
// Lookups index the table by item type, so they cost about the same as
// reading ItemAttribute[itemType]. Built once at startup; only the item
// editor changes it afterwards.
class ItemDatabase
{
public:
    static ItemDatabase& GetInstance();

    ItemDatabase();

    // Replaces all items. Each definition goes to its (group, number);
    // definitions with invalid ids are skipped (validation reports them).
    void Build(std::span<const ItemDefinition> definitions);

    // Fills ItemDefinition::name from the names for this UI locale, falling
    // back to English.
    void SetDisplayLocale(std::string_view locale);
    const std::string& GetDisplayLocale() const
    {
        return m_displayLocale;
    }

    // Returns nullptr for invalid ids and for empty item slots. Defined here
    // so it can be inlined on hot paths. The table never moves, so returned
    // pointers stay valid, but Build() and the editor functions below
    // overwrite what they point to.
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

    // All MAX_ITEM slots, indexed by item type, including empty ones.
    std::span<const ItemDefinition> GetAllSlots() const
    {
        return m_definitions;
    }

    // Stable, language-neutral name for logs: "<English name> (<group>,<number>)".
    std::string GetLogName(int itemType) const;

    int GetExistingItemCount() const { return m_existingItemCount; }

    // Item editor changes. Set replaces the item at the definition's
    // (group, number); an item without names counts as not existing.
    void Set(const ItemDefinition& definition);
    // Swaps two slots and updates the ids of the moved items.
    void Swap(int firstItemType, int secondItemType);

private:
    void UpdateDisplayName(ItemDefinition& definition) const;
    void CountExistingItems();

    std::vector<ItemDefinition> m_definitions;
    std::string m_displayLocale{LocalizedString::NeutralLocale};
    int m_existingItemCount = 0;
};
} // namespace Data::Items

#define g_ItemDatabase Data::Items::ItemDatabase::GetInstance()
