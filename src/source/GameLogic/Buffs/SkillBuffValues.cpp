#include "stdafx.h"

#include "GameLogic/Buffs/SkillBuffValues.h"

#include "Data/DataHandler/SkillData/MasterSkillStorage.h"
#include "Data/GameData/BuffData/BuffDatabase.h"
#include "Data/GameData/SkillData/MasterSkillDatabase.h"
#include "GameLogic/Skills/SkillManager.h"

#include <algorithm>

namespace GameLogic::Buffs
{
namespace
{
using Data::Skills::MasterSkillDefinition;
using Data::Skills::MasterSkillHow;

// A percent as the server sends a master skill's value, as a fraction.
constexpr double PercentPerFraction = 100.0;

// The skill and its base skills, the skill first (SKILL_REPLACEMENTS: a master skill replaces the skill below it).
size_t WalkSkillChain(int skill, std::array<int, MaxSkillChain>& chain)
{
    size_t count = 0;
    auto at = static_cast<ActionSkillType>(skill);
    while (count < chain.size())
    {
        chain[count++] = at;
        const auto next = SKILL_REPLACEMENTS.find(at);
        if (next == SKILL_REPLACEMENTS.end())
        {
            break;
        }
        at = next->second;
    }
    return count;
}

// A learned master skill on the way down from the cast skill.
struct LearnedMaster
{
    const MasterSkillDefinition* definition = nullptr;
    double value = 0;
};

// The value of one power-up with its master parts, the power-up it adds or is multiplied by first.
class PowerUpTotals
{
public:
    PowerUpTotals(const Data::Buffs::BuffDefinition& buff, SkillBuffValues& values) : m_buff(buff), m_values(values) {}

    double Total(size_t index, size_t depth = 0)
    {
        if (m_done[index])
        {
            return m_values.powerUps[index].total;
        }
        SkillBuffValue& value = m_values.powerUps[index];
        double total = value.base;
        double factor = 1;
        const Data::Buffs::BuffAttributeUnit unit =
            Data::Buffs::BuffAttributeUnitAt(m_buff.powerUps[index].attributeIndex);
        for (size_t part = 0; part < value.masterCount; ++part)
        {
            const MasterPart& master = value.masters[part];
            if (master.how == MasterSkillHow::Multiply)
            {
                // OpenMU adds the master skills' factors before it multiplies (a + b on the x1.0 placeholder).
                factor += master.value / PercentPerFraction;
            }
            else
            {
                total +=
                    unit == Data::Buffs::BuffAttributeUnit::Fraction ? master.value / PercentPerFraction : master.value;
            }
        }
        // Buffs.json rejects a chain that leads back to where it starts; the depth only guards the recursion. A chain
        // to a power-up past the ones kept (MaxSkillBuffPowerUps) is left out.
        for (size_t added = 0; added < value.valueOfCount; ++added)
        {
            if (Kept(value.valueOf[added]) && depth < m_values.powerUpCount)
            {
                total += Total(static_cast<size_t>(value.valueOf[added]), depth + 1);
            }
        }
        total *= factor;
        if (Kept(value.multipliedBy) && depth < m_values.powerUpCount)
        {
            total *= Total(static_cast<size_t>(value.multipliedBy), depth + 1);
        }
        value.total = total;
        m_done[index] = true;
        return total;
    }

private:
    bool Kept(int index) const
    {
        return index != Data::Buffs::NoPowerUp && index >= 0 && static_cast<size_t>(index) < m_values.powerUpCount;
    }

    const Data::Buffs::BuffDefinition& m_buff;
    SkillBuffValues& m_values;
    std::array<bool, MaxSkillBuffPowerUps> m_done{};
};

void ComputeBase(const Data::Buffs::BuffValue& data, const BuffCasterStats& stats, SkillBuffValue& value)
{
    value.reached = data.values;
    for (const Data::Buffs::BuffTerm& term : data.per)
    {
        value.reached += ComputeTerm(term, stats);
    }
    value.stoppedAtMaximum = data.maximum && value.reached > *data.maximum;
    value.base = value.stoppedAtMaximum ? *data.maximum : value.reached;
}

void AddMasterPart(SkillBuffValue& value, const LearnedMaster& master, MasterSkillHow how)
{
    if (value.masterCount < value.masters.size())
    {
        value.masters[value.masterCount++] = {master.definition->skill, master.value, how};
    }
}
} // namespace

int FindSkillBuff(int skill)
{
    return Data::Skills::FindMasterSkillBuff(skill, g_MasterSkillDatabase.GetAll());
}

bool ComputeSkillBuff(int skill, const CHARACTER_ATTRIBUTE& caster, const BuffCasterStats& stats,
                      SkillBuffValues& values)
{
    values = SkillBuffValues{};
    const Data::Buffs::BuffDefinition* buff = g_BuffDatabase.Find(FindSkillBuff(skill));
    if (buff == nullptr || buff->valuesFrom != Data::Buffs::BuffValuesFrom::Caster ||
        (buff->powerUps.empty() && !buff->duration))
    {
        return false;
    }
    values.buff = buff;

    // OpenMU counts the master skills from the cast skill down to its base skills (MagicEffectPowerUpExtensions).
    std::array<int, MaxSkillChain> chain{};
    const size_t chainLength = WalkSkillChain(skill, chain);
    std::array<LearnedMaster, MaxSkillChain> learned{};
    size_t learnedCount = 0;
    for (size_t step = 0; step < chainLength; ++step)
    {
        const int at = chain[step];
        const MasterSkillDefinition* master = g_MasterSkillDatabase.Find(at);
        if (master != nullptr && at >= 0 && at <= AT_SKILL_MASTER_END && caster.MasterSkillInfo[at].GetSkillLevel() > 0)
        {
            learned[learnedCount++] = {master, caster.MasterSkillInfo[at].GetSkillValue()};
        }
    }

    values.powerUpCount = std::min(buff->powerUps.size(), values.powerUps.size());
    for (size_t index = 0; index < values.powerUpCount; ++index)
    {
        const Data::Buffs::BuffPowerUp& powerUp = buff->powerUps[index];
        SkillBuffValue& value = values.powerUps[index];
        ComputeBase(powerUp.value, stats, value);
        for (const int added : powerUp.valueOfIndices)
        {
            if (value.valueOfCount < value.valueOf.size())
            {
                value.valueOf[value.valueOfCount++] = added;
            }
        }
        value.multipliedBy = powerUp.multipliedByIndex;
        for (size_t master = 0; master < learnedCount; ++master)
        {
            // A master skill that raises nothing (it only extends the duration) is no part of a power-up whose
            // attribute BuffAttributes lacks either.
            const int raises = learned[master].definition->raisesIndex;
            if (raises != Data::Buffs::NoBuffAttribute && raises == powerUp.attributeIndex)
            {
                AddMasterPart(value, learned[master], learned[master].definition->how);
            }
        }
    }
    PowerUpTotals totals(*buff, values);
    for (size_t index = 0; index < values.powerUpCount; ++index)
    {
        totals.Total(index);
    }

    if (buff->duration)
    {
        values.hasDuration = true;
        ComputeBase(*buff->duration, stats, values.duration);
        values.duration.total = values.duration.base;
        for (size_t master = 0; master < learnedCount; ++master)
        {
            if (learned[master].definition->extendsDuration)
            {
                AddMasterPart(values.duration, learned[master], MasterSkillHow::Add);
                // Seconds, after the duration's maximum.
                values.duration.total += learned[master].value;
            }
        }
    }
    return true;
}
} // namespace GameLogic::Buffs
