#pragma once

#include "Core/Platform/WinCompat.h"
#include "Core/Globals/_define.h"
#include "Data/GameData/Common/LocalizedString.h"
#include "Data/GameData/ItemData/ItemTagSet.h"
#include "Data/GameData/ItemData/ItemTooltipConditions.h"
#include "Data/GameData/ItemData/ItemType.h"

#include <algorithm>
#include <array>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

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

// The kind of weapon, as OpenMU's item groups give it: swords and the Rage
// Fighter's gloves (group 0), axes (1), maces and sceptres (2), spears (3),
// bows and crossbows (group 4: bows have the slot offHand, crossbows
// mainHand; arrows and bolts are neither), staffs, sticks and the summoner's
// books (5: books have the slot offHand) and shields (6).
enum class WeaponKind : BYTE
{
    None,
    Spear,
    Bow,
    Crossbow,
    Shield,
    Sword,
    Axe,
    Mace,
    Staff,
    Glove,
    Book,
};

// The color of a tooltip line; the values are the tooltip's TEXT_COLOR_*.
enum class ItemTextColor : BYTE
{
    White = TEXT_COLOR_WHITE,
    Blue = TEXT_COLOR_BLUE,
    Red = TEXT_COLOR_RED,
    Yellow = TEXT_COLOR_YELLOW,
    Green = TEXT_COLOR_GREEN,
    DarkRed = TEXT_COLOR_DARKRED,
    Purple = TEXT_COLOR_PURPLE,
    DarkBlue = TEXT_COLOR_DARKBLUE,
    DarkYellow = TEXT_COLOR_DARKYELLOW,
    GreenBlue = TEXT_COLOR_GREEN_BLUE,
    Gray = TEXT_COLOR_GRAY,
    RedPurple = TEXT_COLOR_REDPURPLE,
    Violet = TEXT_COLOR_VIOLET,
    Orange = TEXT_COLOR_ORANGE,
    Count
};
constexpr size_t ItemTextColorCount = static_cast<size_t>(ItemTextColor::Count);

// A color rule of a tooltip line: the color when every condition holds.
struct TooltipColorRule
{
    std::vector<TooltipCondition> when;
    ItemTextColor color = ItemTextColor::White;

    bool operator==(const TooltipColorRule& other) const = default;
};

// The longest a description line's and a name line's text is shown, in UTF-16 characters with its placeholders filled
// (a description line before it wraps): the tooltip writes them into rows of these sizes and cuts a longer text there.
// Reading an item warns about a text written longer.
constexpr size_t MaxDescriptionLineLength = 511;
constexpr size_t MaxNameLineLength = 255;

// One line of an item's description in its tooltip: a text with its translations or a shared text of the tooltip file
// (neither: a blank line), the conditions it shows on (all of them; ItemTooltipConditions.h), its color, and the item
// levels it is shown at (both included; every level unless set).
struct ItemDescriptionLine
{
    LocalizedString text;
    // The shared text of Data/Items/Tooltips/Tooltip.json, without its "@"; empty for a text written in place.
    std::string textName;
    // A placeholder of the text and the value it shows on this line: a built-in value or a table of the item stat
    // file ({"percent": "table:wingDamageIncrease1st"}).
    std::vector<std::pair<std::string, std::string>> values;
    std::vector<TooltipCondition> when;
    ItemTextColor color = ItemTextColor::White;
    // The line's own color rules, as a layout line's: the first that holds gives the color, else `color` does.
    std::vector<TooltipColorRule> colors;
    bool bold = false;
    // Drawn after the item's options, at the end of the tooltip ("after": "options"), instead of with the description.
    bool afterOptions = false;
    int minLevel = 0;
    int maxLevel = ItemLevelCount - 1;

    bool IsShownAt(int level) const
    {
        return level >= minLevel && level <= maxLevel;
    }
    bool HasLevels() const
    {
        return minLevel != 0 || maxLevel != ItemLevelCount - 1;
    }

    bool operator==(const ItemDescriptionLine& other) const = default;
};

// A rule for the item's name in its tooltip: the first of an item's name lines whose levels and conditions hold gives
// the name (its text, written as a description line's) and its color. A line without a text keeps the standard name
// (the set's name, "Excellent", " +level"); one without a color or color rules keeps the standard color.
struct ItemNameLine : ItemDescriptionLine
{
    bool hasText = false;
    bool hasColor = false;

    bool operator==(const ItemNameLine& other) const = default;
};

// The values an item gives a placeholder of its description, one per item level from +0: each a text with its
// translations, or a shared text of the tooltip file. The Fruits' {stat} is "ENG" at +0, "STA" at +1, ...
struct ItemLevelValue
{
    LocalizedString text;
    // The shared text, without its "@"; empty for a text written in place.
    std::string textName;

    bool operator==(const ItemLevelValue& other) const = default;
};

struct ItemLevelValues
{
    std::string name;
    std::vector<ItemLevelValue> values;

    bool operator==(const ItemLevelValues& other) const = default;
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

// The requirements in the order of ItemRequirements.
enum class ItemRequirement : BYTE
{
    Level,
    Strength,
    Dexterity,
    Energy,
    Vitality,
    Leadership,

    Count
};

constexpr size_t ItemRequirementCount = static_cast<size_t>(ItemRequirement::Count);

// A requirement for each item level 0 to 15; above +15 the +15 value.
using RequirementByLevel = std::array<WORD, ItemLevelCount>;

// The requirements an item gives by level instead of by the stat file's
// formula. Few items have any, so the others keep no room for them.
class RequirementsByLevel
{
public:
    // The values of the requirement, or nullptr when it is not given by level.
    const RequirementByLevel* Find(ItemRequirement requirement) const
    {
        const auto found = std::ranges::find(m_entries, requirement, &Entry::requirement);
        return found != m_entries.end() ? &found->values : nullptr;
    }

    void Set(ItemRequirement requirement, const RequirementByLevel& values)
    {
        const auto position = std::ranges::lower_bound(m_entries, requirement, {}, &Entry::requirement);
        if (position != m_entries.end() && position->requirement == requirement)
        {
            position->values = values;
            return;
        }
        m_entries.insert(position, {requirement, values});
    }

    // The requirement is no longer given by level (the item editor).
    void Remove(ItemRequirement requirement)
    {
        std::erase_if(m_entries, [requirement](const Entry& entry) { return entry.requirement == requirement; });
    }

    bool operator==(const RequirementsByLevel& other) const = default;

private:
    struct Entry
    {
        ItemRequirement requirement;
        RequirementByLevel values;

        bool operator==(const Entry& other) const = default;
    };

    // In the order of ItemRequirement.
    std::vector<Entry> m_entries;
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
    // An entry of the item file without a name: the client uses the number (it has a model,
    // code names it), but the game does not know an item there (Exists). Never set with a name.
    bool entryWithoutName = false;

    // All names of the item, English first. Logs always use the English
    // name (names.GetNeutral()), never a translation.
    LocalizedString names;
    // The name in the item name language (the UI language, or English while
    // translated item names are off), shown to the player. Filled by
    // ItemDatabase from names.
    std::wstring name;

    ItemTagSet tags;

    // What the tooltip says about the item, line by line, after its own lines (empty for most items; the jewels'
    // texts of the original client are still in the tooltip code).
    std::vector<ItemDescriptionLine> description;
    // How the tooltip names the item, by level and condition (empty: the standard name and color).
    std::vector<ItemNameLine> nameLines;
    // The values of the description's own placeholders by item level ("levelValues").
    std::vector<ItemLevelValues> levelValues;

    const ItemLevelValues* FindLevelValues(std::string_view name) const
    {
        const auto found = std::find_if(levelValues.begin(), levelValues.end(),
                                        [&](const ItemLevelValues& entry) { return entry.name == name; });
        return found != levelValues.end() ? &*found : nullptr;
    }

    BYTE width = 0;
    BYTE height = 0;
    ItemSlot slot = ItemSlot::None;
    WingTier wingTier = WingTier::None;
    WeaponKind weaponKind = WeaponKind::None;
    // The item's profile in the item stat file (Data/Items/Stats/ItemStats.json):
    // what its + level, excellent options, ancient set and option level add to its
    // stats and requirements.
    // Empty for the default. See ItemDatabase::GetStatProfile.
    std::string statProfile;
    // Which potion of a kind the game takes first: the higher, the earlier;
    // see ItemDatabase::GetItemTypesWithTagInUseOrder.
    BYTE potionPriority = 0;
    bool twoHanded = false;
    WORD skill = 0;
    WORD level = 0;

    BYTE durability = 0;
    BYTE magicDurability = 0;
    // Damage and magic power pass 255 for later items (the .bmd item files hold one byte; see FindItemFileMisfits).
    WORD damageMin = 0;
    WORD damageMax = 0;
    BYTE blockRate = 0;
    BYTE defense = 0;
    BYTE magicDefense = 0;
    BYTE attackSpeed = 0;
    BYTE walkSpeed = 0;
    WORD magicPower = 0;
    // A fixed excellent bonus of damage and magic power instead of the stat
    // file's excellent term (three chaos weapons); 0 uses the term.
    BYTE excellentBonus = 0;
    // False: excellent options do not raise the level requirement (the stat
    // file's excellentLevelAdd), as for the lucky items and some sets.
    bool excellentRaisesLevel = true;
    BYTE attackType = 0;

    ItemRequirements requirements;
    // Requirements given per item level instead of the stat file's formula (the
    // Orb of Summoning's energy, the Transformation Ring's level requirement);
    // the base value of that requirement is not used.
    RequirementsByLevel requirementsByLevel;
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

    // An item the game knows: it has a name.
    bool Exists() const
    {
        return !names.IsEmpty();
    }

    // The item file has the number: an item, or an entry without a name.
    bool HasEntry() const
    {
        return entryWithoutName || Exists();
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

    // All fields; the item editor's undo compares definitions with it.
    bool operator==(const ItemDefinition& other) const = default;
};
} // namespace Data::Items
