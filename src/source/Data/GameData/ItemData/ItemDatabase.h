#pragma once

#include "Data/GameData/ItemData/ItemDefinition.h"
#include "Data/GameData/ItemData/ItemType.h"

#include <cstdint>
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

    // Fast checks for the rule code. They read a small table next to the
    // definitions. Invalid ids and empty slots have no tags and no slot, and
    // allow no action: an item the client does not know cannot be traded,
    // dropped, stored, sold or repaired.
    bool HasTag(int itemType, ItemTag tag) const
    {
        return IsValidItemType(itemType) && m_ruleData[itemType].tags.Has(tag);
    }

    bool HasAnyTag(int itemType, const ItemTagSet& tags) const
    {
        return IsValidItemType(itemType) && m_ruleData[itemType].tags.HasAny(tags);
    }

    bool IsAllowed(int itemType, ItemAction action) const
    {
        return IsValidItemType(itemType) && (m_ruleData[itemType].blockedActions & ActionBit(action)) == 0;
    }

    ItemSlot GetSlot(int itemType) const
    {
        return IsValidItemType(itemType) ? m_ruleData[itemType].slot : ItemSlot::None;
    }

    WingTier GetWingTier(int itemType) const
    {
        return IsValidItemType(itemType) ? m_ruleData[itemType].wingTier : WingTier::None;
    }

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
    // The part of a definition the rule code reads on hot paths. Derived
    // from m_definitions by Store(), the only function that changes them.
    struct RuleData
    {
        ItemTagSet tags;
        uint16_t blockedActions = AllActions;
        ItemSlot slot = ItemSlot::None;
        WingTier wingTier = WingTier::None;
    };

    static constexpr uint16_t ActionBit(ItemAction action)
    {
        return static_cast<uint16_t>(1u << static_cast<int>(action));
    }

    static constexpr uint16_t AllActions = static_cast<uint16_t>((1u << static_cast<int>(ItemAction::Count)) - 1);
    static_assert(static_cast<int>(ItemAction::Count) <= 16, "RuleData::blockedActions holds up to 16 actions");

    // Puts the definition into the slot of itemType (with that group and
    // number) and updates its display name and rule data.
    void Store(int itemType, ItemDefinition definition);
    void UpdateDisplayName(ItemDefinition& definition) const;
    void CountExistingItems();

    std::vector<ItemDefinition> m_definitions;
    std::vector<RuleData> m_ruleData;
    std::string m_displayLocale{LocalizedString::NeutralLocale};
    int m_existingItemCount = 0;
};
} // namespace Data::Items

#define g_ItemDatabase Data::Items::ItemDatabase::GetInstance()
