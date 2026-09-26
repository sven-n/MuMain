#pragma once

#include "Core/Platform/WinCompat.h"
#include "Core/Globals/_define.h"
#include "Data/GameData/Common/LocalizedString.h"
#include "Data/GameData/ItemData/ItemTagSet.h"

#include <array>
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

// What a player may do with an item. See the rule flags in ItemDefinition.
enum class ItemAction : BYTE
{
    Trade,
    Drop,
    Store,
    Sell,
    SellInPersonalShop,
    Repair,
    // Rented items (items with a rental time)
    DropWhileRented,
    SellInPersonalShopWhileRented,
    SellWhenRentalExpired,

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

    // Rule flags. The exceptions that depend on the item level, durability
    // or the player are in the rule code (GameLogic/Items/TradeRestrictions
    // and ShopRestrictions).
    bool tradable = true;
    bool droppable = true;
    bool storable = true;
    bool sellable = true;
    bool personalShopSellable = true;
    bool repairable = true;
    // Rented items cannot be stored. These say what else changes for them.
    bool droppableWhileRented = true;
    bool personalShopSellableWhileRented = true;
    // Sellable to an NPC once the rental time ran out, even when sellable is false.
    bool sellableWhenRentalExpired = false;

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
        case ItemAction::DropWhileRented:
            return droppableWhileRented;
        case ItemAction::SellInPersonalShopWhileRented:
            return personalShopSellableWhileRented;
        case ItemAction::SellWhenRentalExpired:
            return sellableWhenRentalExpired;
        default:
            return true;
        }
    }
};
} // namespace Data::Items
