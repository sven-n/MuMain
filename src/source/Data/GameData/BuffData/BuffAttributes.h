#pragma once

#include <algorithm>
#include <array>
#include <string_view>

// The attributes a buff's power-up can raise (BuffPowerUp::attribute): OpenMU's Stats names in camelCase where OpenMU
// has the attribute, and names of Webzen's item add options (ADD_OPTION_*) where it has none. A name the list lacks is
// an error when Buffs.json loads (ValidateBuffs). See docs/buff-data.md, "Values".
namespace Data::Buffs
{
inline constexpr auto BuffAttributes = std::to_array<std::string_view>({
    // OpenMU's Stats.
    "attackSpeedAny",
    "baseDamageBonus",
    "defenseBase",
    "maximumHealth",
    "maximumMana",
    "maximumAbility",
    "maximumShield",
    "experienceRate",
    "bonusItemDropRate",
    "vulcanusExperienceRate",
    "totalStrength",
    "totalAgility",
    "totalVitality",
    "totalEnergy",
    "totalLeadership",
    "physicalBaseDmg",
    "wizardryBaseDmg",
    "healthRecoveryMultiplier",
    "manaRecoveryMultiplier",
    "criticalDamageChance",
    "excellentDamageChance",
    "deathPenaltyProtection",
    "itemDurabilityProtection",
    // Webzen's item add options which OpenMU has no attribute for.
    "disableExperience",
    "mobility",
    "vulcanusItemDropRate",
    "partyExperienceBonus",
    "vitalityBonus",
});

// The index in BuffAttributes of a name the list lacks.
inline constexpr int NoBuffAttribute = -1;

// The name's index in BuffAttributes, or NoBuffAttribute.
constexpr int FindBuffAttribute(std::string_view name)
{
    const auto found = std::find(BuffAttributes.begin(), BuffAttributes.end(), name);
    return found != BuffAttributes.end() ? static_cast<int>(found - BuffAttributes.begin()) : NoBuffAttribute;
}

constexpr bool IsBuffAttribute(std::string_view name)
{
    return FindBuffAttribute(name) != NoBuffAttribute;
}

// A name of the list, checked when the code compiles: the game code that reads an attribute names it this way, so a
// name the list lacks doesn't build. It holds the name's index, which the game compares with the index the buff table
// resolves for each power-up when it loads (BuffPowerUp::attributeIndex), never a name.
class BuffAttributeName
{
public:
    consteval BuffAttributeName(const char* name) : m_index(FindBuffAttribute(name))
    {
        if (m_index == NoBuffAttribute)
        {
            throw "not a name of BuffAttributes";
        }
    }

    constexpr int Index() const
    {
        return m_index;
    }

private:
    int m_index;
};
} // namespace Data::Buffs
