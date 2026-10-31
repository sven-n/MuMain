#pragma once

#include <cstdint>
#include <initializer_list>

namespace Data::Items
{
// Item categories the game code asks for. Stored as a bitmask, so a check is
// one bit test. The JSON names are in ItemEnumNames.cpp.
enum class ItemTag : uint8_t
{
    // Wings
    Cape,
    // Mounts and pets
    Mount,
    HornMount,
    Flying,
    DarkLordPet,
    GuardianPet,
    PandaOrSkeleton,
    // Rings
    TransformationRing,
    // Crafting materials
    Jewel,
    RefineStone,
    SocketSeed,
    SocketSphere,
    SocketSeedSphere,
    // Consumables
    HealingPotion,
    ManaPotion,
    ComplexPotion,
    ShieldPotion,
    ElitePotion,
    Elixir,
    BuffScroll,
    BattleOrStrengthScroll,
    Ammunition,
    // Event and quest items
    BloodCastleTicketPart,
    SecondClassQuestItem,
    ThirdClassQuestItem,
    // Class equipment
    SummonerBook,
    DivineArchangelWeapon,
    Sceptre,
    MagicSword,
    // Armour
    LuckyItem,
    // Cash shop
    CashShop,
    GambleItem,
    GemJewelry,
    LuckyItemTicket,
    // Asked for confirmation before selling or dropping
    Valuable,
    // Tooltip lines the item does not show though its data has the value (IsTooltipOnlyTag)
    NoLevelRequirement,
    NoDurabilityLine,
    NoBlankAfterOptions,
    NoClassLines,

    Count
};

// A tag that only leaves a line out of the item's tooltip: it says nothing about what the item is, so the item lists
// don't tell items of the same name apart by it. A new tag of that group belongs here too.
constexpr bool IsTooltipOnlyTag(ItemTag tag)
{
    return tag == ItemTag::NoLevelRequirement || tag == ItemTag::NoDurabilityLine ||
           tag == ItemTag::NoBlankAfterOptions || tag == ItemTag::NoClassLines;
}

class ItemTagSet
{
public:
    static_assert(static_cast<int>(ItemTag::Count) <= 64, "ItemTagSet holds up to 64 tags");

    constexpr ItemTagSet() = default;

    constexpr ItemTagSet(std::initializer_list<ItemTag> tags)
    {
        for (const ItemTag tag : tags)
        {
            Set(tag);
        }
    }

    constexpr bool Has(ItemTag tag) const
    {
        return (m_bits & Bit(tag)) != 0;
    }

    // True when at least one of the given tags is set.
    constexpr bool HasAny(const ItemTagSet& tags) const
    {
        return (m_bits & tags.m_bits) != 0;
    }

    constexpr void Set(ItemTag tag, bool value = true)
    {
        m_bits = value ? (m_bits | Bit(tag)) : (m_bits & ~Bit(tag));
    }

    constexpr bool IsEmpty() const
    {
        return m_bits == 0;
    }

    bool operator==(const ItemTagSet& other) const = default;

private:
    static constexpr uint64_t Bit(ItemTag tag)
    {
        return uint64_t{1} << static_cast<int>(tag);
    }

    uint64_t m_bits = 0;
};
} // namespace Data::Items
