#pragma once

#include "Data/GameData/ItemData/ItemTooltipLayouts.h"

#include <array>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// The values the item tooltip's stat lines show for an item and the hero, each computed as RenderItemInfo always has:
// the built-in placeholders of the item descriptions ({damageMin}, {requiredStrength}, ...), and the states the
// conditions of the tooltip layouts name (Data::Items::GetTooltipStates).
struct ItemTooltipValues
{
    // A requirement as the tooltip shows it: what the item asks (after the Jewel of Harmony's and the sockets'
    // reductions), what the hero lacks of it (0 when nothing), and whether it was reduced.
    struct Requirement
    {
        int required = 0;
        int missing = 0;
        bool reduced = false;
        // The item asks for it at all (its own requirement, before the reductions): a reduction to 0 still shows
        // the line.
        bool asked = false;
    };

    // Damage as the damage line shows it: with the Jewel of Harmony's attack bonus, or the skill's wizardry damage of
    // a skill book.
    int damageMin = 0;
    int damageMax = 0;
    // The item has damage above 0 to show (a skill book or scroll: its skill's, which the teleport scrolls' skills
    // don't have).
    bool hasDamage = false;
    bool damageHarmonyBonus = false;
    // Defense with the Jewel of Harmony's defense bonus.
    int defense = 0;
    bool defenseHarmonyBonus = false;
    int magicDefense = 0;
    int blockRate = 0;
    int attackSpeed = 0;
    int walkSpeed = 0;
    int magicPower = 0;
    int durability = 0;
    int maxDurability = 0;
    // Ice, poison, lightning, fire, earth, wind, water: the item's level + 1 for a resistance it has, else 0.
    std::array<int, 7> resistances{};
    Requirement level;
    Requirement strength;
    Requirement dexterity;
    Requirement vitality;
    Requirement energy;
    Requirement leadership;
    bool excellent = false;
    bool ancient = false;
    // A skill book or scroll (group 15), whose damage is its skill's wizardry damage.
    bool skillBook = false;
    bool twoHanded = false;
    // A helm, armor, pants, gloves or boots (groups 7 to 11).
    bool armour = false;
    // A staff, a book or a Magic Gladiator's magic sword: it shows its wizardry or curse damage increase.
    bool wizardryWeapon = false;
    // The item keeps a durability: one of groups 0 to 11 or 13 with durability or magic durability in its data, or
    // the wings and capes 12/0-6, 12/36-43 and 12/49-50 (the tooltip's durability rule).
    bool hasDurability = false;
    // A timed item (one that expires).
    bool timed = false;
    // The item's add option (the potions', scrolls' and seals' effects of ItemAddOption): its two values, and how long
    // it lasts as the tooltip writes it ("1 Hour").
    int addOptionValue1 = 0;
    int addOptionValue2 = 0;
    std::string addOptionDuration;
};

namespace Engine::Items::TooltipValues
{
// The values of `item` for the hero as he is (CharacterAttribute); without a hero (none set yet), the requirements
// are shown but nothing is missing.
ItemTooltipValues ComputeItemTooltipValues(const ITEM& item);

// The value of a built-in placeholder (ItemDescriptionPlaceholders' names, without {level}), or nothing for another
// name.
std::optional<int> GetItemTooltipValue(const ItemTooltipValues& values, std::string_view name);

// The value of a built-in placeholder that is a text ({addOptionDuration}), or nothing for another name.
std::optional<std::string> GetItemTooltipText(const ItemTooltipValues& values, std::string_view name);

// The state of that name (Data::Items::GetTooltipStates); false for another name.
bool GetItemTooltipState(const ItemTooltipValues& values, std::string_view name);

// Whether every condition holds for the item of `type` at item level `level`: a value (a built-in, or {level}) is not 0
// or compares as the condition says, a state is set, the item has the tag; each turned around when negated.
bool AreTooltipConditionsMet(std::span<const Data::Items::TooltipCondition> conditions, int type, int level,
                             const ItemTooltipValues& values);

// The color of a tooltip line: that of its first color rule whose conditions hold, else `color`.
Data::Items::ItemTextColor GetTooltipColor(std::span<const Data::Items::TooltipColorRule> rules,
                                           Data::Items::ItemTextColor color, int type, int level,
                                           const ItemTooltipValues& values);

// The color of a layout line (GetTooltipColor of its rules and color).
Data::Items::ItemTextColor GetTooltipLineColor(const Data::Items::TooltipLayoutLine& line, int type, int level,
                                               const ItemTooltipValues& values);
// Every name GetItemTooltipValue answers for, so a test can check them against the placeholder checks' list.
std::vector<std::string_view> GetItemTooltipValueNames();
} // namespace Engine::Items::TooltipValues
