#include "stdafx.h"

#include "GameLogic/Buffs/BuffValues.h"

#include "Character/CharacterManager.h"
#include "Data/GameData/BuffData/BuffDatabase.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInventory.h"
#include "GameLogic/Buffs/w_Buff.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace GameLogic::Buffs
{
namespace
{
using Data::Buffs::BuffTermAttribute;

constexpr size_t At(BuffTermAttribute attribute)
{
    return static_cast<size_t>(attribute);
}

// The party members the client sees, the hero not counted, like OpenMU's count of the members in the caster's view
// (Party.cs, NearbyPartyMemberCount). Party[].index is the member's character while it is in sight.
int CountPartyMembersInSight()
{
    int count = 0;
    for (int member = 0; member < PartyNumber && member < MAX_PARTYS; ++member)
    {
        const int character = Party[member].index;
        if (character >= 0 && character < MAX_CHARACTERS_CLIENT && &CharactersClient[character] != Hero)
        {
            ++count;
        }
    }
    return count;
}
} // namespace

BuffCasterStats GetHeroCasterStats()
{
    BuffCasterStats stats{};
    if (CharacterAttribute == nullptr)
    {
        return stats;
    }
    const CHARACTER_ATTRIBUTE& hero = *CharacterAttribute;
    const double masterLevel = gCharacterManager.GetHeroMasterLevel();
    stats[At(BuffTermAttribute::Level)] = hero.Level;
    stats[At(BuffTermAttribute::TotalLevel)] = hero.Level + masterLevel;
    stats[At(BuffTermAttribute::MasterLevel)] = masterLevel;
    stats[At(BuffTermAttribute::TotalStrength)] = hero.Strength + hero.AddStrength;
    stats[At(BuffTermAttribute::TotalAgility)] = hero.Dexterity + hero.AddDexterity;
    stats[At(BuffTermAttribute::TotalVitality)] = hero.Vitality + hero.AddVitality;
    stats[At(BuffTermAttribute::TotalEnergy)] = hero.Energy + hero.AddEnergy;
    stats[At(BuffTermAttribute::TotalLeadership)] = hero.Charisma + hero.AddCharisma;
    stats[At(BuffTermAttribute::TotalStrengthAndAgility)] =
        stats[At(BuffTermAttribute::TotalStrength)] + stats[At(BuffTermAttribute::TotalAgility)];
    stats[At(BuffTermAttribute::MaximumMana)] = hero.ManaMax;
    stats[At(BuffTermAttribute::NearbyPartyMemberCount)] = CountPartyMembersInSight();
    return stats;
}

double ComputeTerm(const Data::Buffs::BuffTerm& term, const BuffCasterStats& stats)
{
    const double attribute = stats[At(term.heroAttribute)];
    if (term.times > 0)
    {
        return attribute * term.times;
    }
    return term.divisor > 0 ? attribute / term.divisor : 0;
}

double ComputeValue(const Data::Buffs::BuffValue& value, const BuffCasterStats& stats)
{
    double total = value.values;
    for (const Data::Buffs::BuffTerm& term : value.per)
    {
        total += ComputeTerm(term, stats);
    }
    return value.maximum ? std::min(total, *value.maximum) : total;
}

int CutToWhole(double value)
{
    // A computed value like 0.1 * 3 can land a hair below the whole number it means.
    constexpr double Tolerance = 1e-9;
    // A term on a huge stat can leave an int's range; the cast of such a double is undefined.
    constexpr double Lowest = std::numeric_limits<int>::min();
    constexpr double Highest = std::numeric_limits<int>::max();
    const double cut = value < 0 ? std::ceil(value - Tolerance) : std::floor(value + Tolerance);
    return std::isnan(cut) ? 0 : static_cast<int>(std::clamp(cut, Lowest, Highest));
}

int GetPowerUpValue(const Data::Buffs::BuffDefinition& buff, size_t index)
{
    return index < buff.powerUps.size() ? CutToWhole(ComputeValue(buff.powerUps[index].value, GetHeroCasterStats()))
                                        : 0;
}

int GetDuration(const Data::Buffs::BuffDefinition& buff)
{
    return buff.duration ? CutToWhole(ComputeValue(*buff.duration, GetHeroCasterStats())) : 0;
}

int SumPowerUps(const Buff& buffs, Data::Buffs::BuffAttributeName attribute)
{
    int total = 0;
    const BuffCasterStats stats = GetHeroCasterStats();
    for (const auto& entry : buffs.m_Buff)
    {
        const Data::Buffs::BuffDefinition* buff = g_BuffDatabase.Find(entry.first);
        // A value only raises its attribute, so a debuff's would raise what it lowers (Buffs.json warns about one).
        if (buff == nullptr || buff->kind == Data::Buffs::BuffKind::Debuff ||
            buff->valuesFrom == Data::Buffs::BuffValuesFrom::Caster)
        {
            continue;
        }
        for (const Data::Buffs::BuffPowerUp& powerUp : buff->powerUps)
        {
            if (powerUp.attributeIndex == attribute.Index())
            {
                total += CutToWhole(ComputeValue(powerUp.value, stats));
            }
        }
    }
    return total;
}

int GetHeroPowerUpTotal(Data::Buffs::BuffAttributeName attribute)
{
    return Hero != nullptr ? SumPowerUps(Hero->Object.m_BuffMap, attribute) : 0;
}
} // namespace GameLogic::Buffs
