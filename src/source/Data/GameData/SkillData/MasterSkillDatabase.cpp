#include "stdafx.h"

#include "MasterSkillDatabase.h"

namespace Data::Skills
{
MasterSkillDatabase& MasterSkillDatabase::GetInstance()
{
    static MasterSkillDatabase instance;
    return instance;
}

MasterSkillDatabase::MasterSkillDatabase()
{
    m_indices.fill(NoEntry);
}

void MasterSkillDatabase::Build(std::span<const MasterSkillDefinition> skills)
{
    m_skills.assign(skills.begin(), skills.end());
    m_indices.fill(NoEntry);
    m_raised.fill(false);
    for (size_t index = 0; index < m_skills.size(); ++index)
    {
        MasterSkillDefinition& skill = m_skills[index];
        // The tooltip matches power-ups by index, never by name.
        skill.raisesIndex = skill.raises.empty() ? Buffs::NoBuffAttribute : Buffs::FindBuffAttribute(skill.raises);
        if (skill.raisesIndex != Buffs::NoBuffAttribute)
        {
            m_raised[skill.raisesIndex] = true;
        }
        if (skill.skill >= 0 && skill.skill <= MaxSkill)
        {
            m_indices[skill.skill] = static_cast<int>(index);
        }
    }
}

const MasterSkillDefinition* MasterSkillDatabase::Find(int skill) const
{
    if (skill < 0 || skill > MaxSkill || m_indices[skill] == NoEntry)
    {
        return nullptr;
    }
    return &m_skills[m_indices[skill]];
}
} // namespace Data::Skills
