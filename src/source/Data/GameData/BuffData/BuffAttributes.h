#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <string_view>

// The attributes a buff's power-up can raise (BuffPowerUp::attribute): OpenMU's Stats names in camelCase where OpenMU
// has the attribute, and names of Webzen's item add options (ADD_OPTION_*) where it has none. A name the list lacks is
// an error when Buffs.json loads (ValidateBuffs). See docs/buff-data.md, "Values".
namespace Data::Buffs
{
// How an attribute's value reads (buffs design B17): a flat number ("defenseBase" 60), or a fraction OpenMU keeps as
// one, which a text shows in percent (0.30 is 30 %).
enum class BuffAttributeUnit : std::uint8_t
{
    Flat,
    Fraction,
};

struct BuffAttribute
{
    std::string_view name;
    BuffAttributeUnit unit;
};

inline constexpr auto BuffAttributes = std::to_array<BuffAttribute>({
    // OpenMU's Stats the item buffs raise. Their values are Webzen's, flat (a rate in percent).
    {"attackSpeedAny", BuffAttributeUnit::Flat},
    {"baseDamageBonus", BuffAttributeUnit::Flat},
    {"defenseBase", BuffAttributeUnit::Flat},
    {"maximumHealth", BuffAttributeUnit::Flat},
    {"maximumMana", BuffAttributeUnit::Flat},
    {"maximumAbility", BuffAttributeUnit::Flat},
    {"maximumShield", BuffAttributeUnit::Flat},
    {"experienceRate", BuffAttributeUnit::Flat},
    {"bonusItemDropRate", BuffAttributeUnit::Flat},
    {"vulcanusExperienceRate", BuffAttributeUnit::Flat},
    {"totalStrength", BuffAttributeUnit::Flat},
    {"totalAgility", BuffAttributeUnit::Flat},
    {"totalVitality", BuffAttributeUnit::Flat},
    {"totalEnergy", BuffAttributeUnit::Flat},
    {"totalLeadership", BuffAttributeUnit::Flat},
    {"physicalBaseDmg", BuffAttributeUnit::Flat},
    {"wizardryBaseDmg", BuffAttributeUnit::Flat},
    {"healthRecoveryMultiplier", BuffAttributeUnit::Flat},
    {"manaRecoveryMultiplier", BuffAttributeUnit::Flat},
    {"criticalDamageChance", BuffAttributeUnit::Flat},
    {"excellentDamageChance", BuffAttributeUnit::Flat},
    {"deathPenaltyProtection", BuffAttributeUnit::Flat},
    {"itemDurabilityProtection", BuffAttributeUnit::Flat},
    // Webzen's item add options which OpenMU has no attribute for.
    {"disableExperience", BuffAttributeUnit::Flat},
    {"mobility", BuffAttributeUnit::Flat},
    {"vulcanusItemDropRate", BuffAttributeUnit::Flat},
    {"partyExperienceBonus", BuffAttributeUnit::Flat},
    {"vitalityBonus", BuffAttributeUnit::Flat},
    // OpenMU's Stats the skill buffs raise (PR 2b), in OpenMU's units; maximumWizBaseDmgIncrease and
    // defenseRatePvmIncrease stand for OpenMU's x1.0 placeholders a master skill raises. criticalDamageChance (above)
    // stays
    // in percent, as the Scroll of Battle has it, so its master skill parts add their percent as it is.
    {"greaterDamageBonus", BuffAttributeUnit::Flat},
    {"greaterDefenseBonus", BuffAttributeUnit::Flat},
    {"soulBarrierReceiveDecrement", BuffAttributeUnit::Fraction},
    {"soulBarrierManaTollPerHit", BuffAttributeUnit::Flat},
    {"criticalDamageBonus", BuffAttributeUnit::Flat},
    {"ammunitionConsumptionRate", BuffAttributeUnit::Fraction},
    {"attackDamageIncrease", BuffAttributeUnit::Fraction},
    {"swellLifeHealthIncrease", BuffAttributeUnit::Fraction},
    {"swellLifeManaIncrease", BuffAttributeUnit::Fraction},
    {"damageReflection", BuffAttributeUnit::Fraction},
    {"berserkerManaMultiplier", BuffAttributeUnit::Fraction},
    {"berserkerHealthDecrement", BuffAttributeUnit::Fraction},
    {"berserkerMinPhysDmgBonus", BuffAttributeUnit::Flat},
    {"berserkerMaxPhysDmgBonus", BuffAttributeUnit::Flat},
    {"berserkerWizardryMultiplier", BuffAttributeUnit::Fraction},
    {"berserkerCurseMultiplier", BuffAttributeUnit::Fraction},
    {"berserkerProficiencyMultiplier", BuffAttributeUnit::Fraction},
    {"minimumWizBaseDmg", BuffAttributeUnit::Flat},
    {"maximumWizBaseDmgIncrease", BuffAttributeUnit::Fraction},
    {"defenseIgnoreChance", BuffAttributeUnit::Fraction},
    {"increaseBlockBonus", BuffAttributeUnit::Flat},
    {"defenseRatePvmIncrease", BuffAttributeUnit::Fraction},
    {"defenseFinal", BuffAttributeUnit::Flat},
});

// The index in BuffAttributes of a name the list lacks.
inline constexpr int NoBuffAttribute = -1;

// The name's index in BuffAttributes, or NoBuffAttribute.
constexpr int FindBuffAttribute(std::string_view name)
{
    const auto found = std::find_if(BuffAttributes.begin(), BuffAttributes.end(),
                                    [name](const BuffAttribute& attribute) { return attribute.name == name; });
    return found != BuffAttributes.end() ? static_cast<int>(found - BuffAttributes.begin()) : NoBuffAttribute;
}

constexpr bool IsBuffAttribute(std::string_view name)
{
    return FindBuffAttribute(name) != NoBuffAttribute;
}

// The unit of the attribute at that index; Flat for NoBuffAttribute.
constexpr BuffAttributeUnit BuffAttributeUnitAt(int index)
{
    return index >= 0 && index < static_cast<int>(BuffAttributes.size()) ? BuffAttributes[index].unit
                                                                         : BuffAttributeUnit::Flat;
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
