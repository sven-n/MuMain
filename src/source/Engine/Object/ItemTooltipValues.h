#pragma once

#include <array>
#include <optional>
#include <string_view>

// The values the item tooltip's stat lines show for an item and the hero, each computed as RenderItemInfo always has:
// the built-in placeholders of the item descriptions ({damageMin}, {requiredStrength}, ...), and the states the lines'
// colors follow.
struct ItemTooltipValues
{
    // A requirement as the tooltip shows it: what the item asks (after the Jewel of Harmony's and the sockets'
    // reductions), what the hero lacks of it (0 when nothing), and whether it was reduced.
    struct Requirement
    {
        int required = 0;
        int missing = 0;
        bool reduced = false;
    };

    // Damage as the damage line shows it: with the Jewel of Harmony's attack bonus, or the skill's wizardry damage of
    // a skill book.
    int damageMin = 0;
    int damageMax = 0;
    // The damage the line is coloured for is above 0 (a skill book whose skill has no damage shows no line; the
    // teleport scrolls show none either, though this is set for them).
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
};

// The values of `item` for the hero as he is (CharacterAttribute).
ItemTooltipValues ComputeItemTooltipValues(const ITEM& item);

// The value of a built-in placeholder (ItemDescriptionPlaceholders' names, without {level}), or nothing for another
// name.
std::optional<int> GetItemTooltipValue(const ItemTooltipValues& values, std::string_view name);
