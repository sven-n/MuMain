#pragma once

#include "Data/GameData/ItemData/ItemDefinition.h"
#include "Data/GameData/ItemData/ItemStatProfiles.h"
#include "Data/GameData/ItemData/ItemTooltipLayouts.h"
#include "Data/GameData/ItemData/ItemType.h"

#include <array>
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

    // Fills ItemDefinition::name from the names in the item name language:
    // the UI language, or English while translated item names are off. A
    // missing translation falls back to English.
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

    // Like Find, but also an entry without a name: what the item file has at
    // the number (ItemDefinition::HasEntry). The game uses Find.
    const ItemDefinition* FindEntry(int itemType) const
    {
        if (!IsValidItemType(itemType))
        {
            return nullptr;
        }

        const ItemDefinition& definition = m_definitions[itemType];
        return definition.HasEntry() ? &definition : nullptr;
    }

    const ItemDefinition* FindEntry(int group, int number) const;

    // Fast checks for the rule code. They read a small table next to the
    // definitions. Invalid ids and empty slots have no tags, slot, wing tier
    // or weapon kind, and allow no action: an item the client does not know
    // cannot be traded, dropped, stored, sold or repaired.
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

    WeaponKind GetWeaponKind(int itemType) const
    {
        return IsValidItemType(itemType) ? m_ruleData[itemType].weaponKind : WeaponKind::None;
    }

    // Replaces the stat profiles (Data/Items/Stats/ItemStats.json) and resolves
    // every item's profile again. An item naming a profile they do not have
    // gets the default (the loader reports it as an error).
    void SetStatProfiles(ItemStatProfiles profiles);

    const ItemStatProfiles& GetStatProfiles() const
    {
        return m_statProfiles;
    }

    // The shared texts and layouts of the item tooltips (Data/Items/Tooltips/Tooltip.json). A version change, as the
    // items' and the stat file's: what the tooltips made of them (their compiled texts) is made again.
    void SetTooltipFile(ItemTooltipFile file)
    {
        ++m_version;
        m_tooltipFile = std::move(file);
    }

    const ItemTooltipFile& GetTooltipFile() const
    {
        return m_tooltipFile;
    }

    // The item's stat profile laid over the default; the default for empty
    // slots and invalid ids.
    const ItemStatProfile& GetStatProfile(int itemType) const
    {
        return m_statProfiles.Get(IsValidItemType(itemType) ? m_ruleData[itemType].statProfile
                                                            : ItemStatProfiles::DefaultProfile);
    }

    // The item types with the tag, lowest first. Built again whenever the
    // items change, so the span is valid until then.
    std::span<const int> GetItemTypesWithTag(ItemTag tag) const
    {
        const auto tagIndex = static_cast<size_t>(tag);
        return tagIndex < m_itemTypesByTag.size() ? std::span<const int>(m_itemTypesByTag[tagIndex])
                                                  : std::span<const int>();
    }

    // The item types with the tag in the order the game takes them: the
    // higher potionPriority first, then the higher item type. Built again
    // whenever the items change, so the span is valid until then.
    std::span<const int> GetItemTypesWithTagInUseOrder(ItemTag tag) const
    {
        const auto tagIndex = static_cast<size_t>(tag);
        return tagIndex < m_itemTypesByTagInUseOrder.size() ? std::span<const int>(m_itemTypesByTagInUseOrder[tagIndex])
                                                            : std::span<const int>();
    }

    // All MAX_ITEM slots, indexed by item type, including empty ones.
    std::span<const ItemDefinition> GetAllSlots() const
    {
        return m_definitions;
    }

    // Stable, language-neutral name for logs: "<English name> (<group>,<number>)".
    std::string GetLogName(int itemType) const;

    int GetExistingItemCount() const { return m_existingItemCount; }

    // Counts the changes of the items, the stat file and the tooltip file;
    // views taken from them (e.g. the item names the editor lists, the
    // tooltips' compiled texts) take them again when it changes.
    int GetVersion() const
    {
        return m_version;
    }

    // Counts the changes of the item names shown: a stored item or another
    // language of the names (SetDisplayLocale). Views that keep item names
    // take them again when it changes. Unlike GetVersion, a language change
    // leaves the item tools' open edits alone.
    int GetNameVersion() const
    {
        return m_nameVersion;
    }

    // Item editor changes. Set replaces the item at the definition's
    // (group, number); an item without names counts as not existing.
    void Set(const ItemDefinition& definition);
    // Like Set for each definition, with the counts and tag lists built once
    // at the end (a change to many items at once).
    void SetMany(std::span<const ItemDefinition> definitions);
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
        WeaponKind weaponKind = WeaponKind::None;
        int statProfile = ItemStatProfiles::DefaultProfile;
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
    // Fills m_itemTypesByTag and m_itemTypesByTagInUseOrder; after every
    // change of the items.
    void BuildItemTypesByTag();

    int ResolveStatProfile(const std::string& name) const;

    std::vector<ItemDefinition> m_definitions;
    std::vector<RuleData> m_ruleData;
    ItemStatProfiles m_statProfiles;
    ItemTooltipFile m_tooltipFile;
    // GetItemTypesWithTag's lists, one per tag.
    std::array<std::vector<int>, static_cast<size_t>(ItemTag::Count)> m_itemTypesByTag;
    // GetItemTypesWithTagInUseOrder's lists, one per tag.
    std::array<std::vector<int>, static_cast<size_t>(ItemTag::Count)> m_itemTypesByTagInUseOrder;
    std::string m_displayLocale{LocalizedString::NeutralLocale};
    int m_existingItemCount = 0;
    int m_version = 0;
    int m_nameVersion = 0;
};
} // namespace Data::Items

#define g_ItemDatabase Data::Items::ItemDatabase::GetInstance()
