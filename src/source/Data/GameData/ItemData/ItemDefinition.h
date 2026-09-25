#pragma once

#include "Core/Platform/WinCompat.h"
#include "Core/Globals/_define.h"
#include "Data/GameData/Common/LocalizedString.h"

#include <array>
#include <cstdint>
#include <string>

namespace Data::Items
{
// Where an item can be equipped. The values are the equipment indexes
// (EQUIPMENT_*) and the values of ITEM_ATTRIBUTE::m_byItemSlot.
enum class ItemSlot : BYTE
{
    MainHand = 0,
    OffHand = 1,
    Helm = 2,
    Armor = 3,
    Pants = 4,
    Gloves = 5,
    Boots = 6,
    Wings = 7,
    Pet = 8,
    Pendant = 9,
    Ring = 10,
    None = 255,
};

// The wing generation; the wing formulas (defense, damage increase and
// absorption) depend on it.
enum class WingTier : BYTE
{
    None,
    Small,
    First,
    Second,
    Third,
};

// Item categories the rule code asks for. Stored as a bitmask, so a check is
// one bit test. The JSON names are in ItemEnumNames.cpp.
enum class ItemTag : BYTE
{
    // Mounts and pets
    Mount,
    Flying,
    DarkLordPet,
    GuardianPet,
    PandaOrSkeleton,
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
    // Cash shop
    CashShop,
    GambleItem,
    GemJewelry,
    LuckyItemTicket,
    // Asked for confirmation before selling or dropping
    Valuable,

    Count
};

class ItemTagSet
{
public:
    static_assert(static_cast<int>(ItemTag::Count) <= 64, "ItemTagSet holds up to 64 tags");

    bool Has(ItemTag tag) const
    {
        return (m_bits & Bit(tag)) != 0;
    }

    void Set(ItemTag tag, bool value = true)
    {
        m_bits = value ? (m_bits | Bit(tag)) : (m_bits & ~Bit(tag));
    }

    bool IsEmpty() const
    {
        return m_bits == 0;
    }

    uint64_t GetBits() const
    {
        return m_bits;
    }

    bool operator==(const ItemTagSet& other) const = default;

private:
    static constexpr uint64_t Bit(ItemTag tag)
    {
        return uint64_t{1} << static_cast<int>(tag);
    }

    uint64_t m_bits = 0;
};

// What a player may do with an item. Everything is allowed by default.
enum class ItemAction : BYTE
{
    Trade,
    Drop,
    Store,
    Sell,
    SellInPersonalShop,
    Repair,

    Count
};

struct ItemRequirements
{
    WORD level = 0;
    WORD strength = 0;
    WORD dexterity = 0;
    WORD energy = 0;
    WORD vitality = 0;
    WORD leadership = 0;

    bool operator==(const ItemRequirements& other) const = default;
};

// One item as the client knows it. Filled once at startup and read-only
// afterwards, except for the item editor; see ItemDatabase.
struct ItemDefinition
{
    int group = 0;
    int number = 0;

    // All names of the item, English first. Logs always use the English
    // name (names.GetNeutral()), never a translation.
    LocalizedString names;
    // The name in the current UI locale, shown to the player. Filled by
    // ItemDatabase from names.
    std::wstring name;

    ItemTagSet tags;

    BYTE width = 0;
    BYTE height = 0;
    ItemSlot slot = ItemSlot::None;
    WingTier wingTier = WingTier::None;
    bool twoHanded = false;
    WORD skill = 0;
    WORD level = 0;

    BYTE durability = 0;
    BYTE magicDurability = 0;
    BYTE damageMin = 0;
    BYTE damageMax = 0;
    BYTE blockRate = 0;
    BYTE defense = 0;
    BYTE magicDefense = 0;
    BYTE attackSpeed = 0;
    BYTE walkSpeed = 0;
    BYTE magicPower = 0;
    BYTE attackType = 0;

    ItemRequirements requirements;
    std::array<BYTE, MAX_CLASS> classRequirements{};
    std::array<BYTE, MAX_RESISTANCE + 1> resistances{};

    BYTE sellValue = 0;
    int buyPrice = 0;

    // Rule flags. Some items have exceptions that depend on the item's
    // state (level, rental time, durability); those are in the rule code
    // (GameLogic/Items/TradeRestrictions and ShopRestrictions).
    bool tradable = true;
    bool droppable = true;
    bool storable = true;
    bool sellable = true;
    bool personalShopSellable = true;
    bool repairable = true;

    bool Exists() const
    {
        return !names.IsEmpty();
    }

    bool IsAllowed(ItemAction action) const
    {
        switch (action)
        {
        case ItemAction::Trade:
            return tradable;
        case ItemAction::Drop:
            return droppable;
        case ItemAction::Store:
            return storable;
        case ItemAction::Sell:
            return sellable;
        case ItemAction::SellInPersonalShop:
            return personalShopSellable;
        case ItemAction::Repair:
            return repairable;
        default:
            return true;
        }
    }
};
} // namespace Data::Items
