#pragma once

#include "Core/Globals/_enum.h"
#include "Data/GameData/SkillData/MasterSkillDefinition.h"

#include <array>
#include <span>
#include <vector>

// The master skills of Data/Skills/MasterSkills.json in memory (buffs design B18), looked up by skill number.
namespace Data::Skills
{
class MasterSkillDatabase
{
public:
    static MasterSkillDatabase& GetInstance();

    // Replaces all entries and resolves the attributes they raise.
    void Build(std::span<const MasterSkillDefinition> skills);

    // The entry of that skill, or nullptr.
    const MasterSkillDefinition* Find(int skill) const;

    const std::vector<MasterSkillDefinition>& GetAll() const
    {
        return m_skills;
    }

private:
    static constexpr int NoEntry = -1;
    static constexpr int MaxSkill = AT_SKILL_MASTER_END;

    MasterSkillDatabase();

    std::vector<MasterSkillDefinition> m_skills;
    // The index in m_skills by skill number, or NoEntry.
    std::array<int, MaxSkill + 1> m_indices{};
};
} // namespace Data::Skills

#define g_MasterSkillDatabase Data::Skills::MasterSkillDatabase::GetInstance()
