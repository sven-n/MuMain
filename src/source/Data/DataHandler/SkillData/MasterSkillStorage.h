#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/SkillData/MasterSkillDefinition.h"

#include <filesystem>
#include <span>
#include <string>
#include <vector>

// Reads Data/Skills/MasterSkills.json (buffs design B18) into the table (g_MasterSkillDatabase).
namespace Data::Skills
{
// "Data/Skills/MasterSkills.json", from the game folder.
std::filesystem::path GetMasterSkillsFile();

struct MasterSkillsLoadResult
{
    std::vector<MasterSkillDefinition> skills;
    std::vector<Items::ItemDataIssue> issues;
};

// Reads the file and its own checks (ValidateMasterSkills).
MasterSkillsLoadResult LoadMasterSkillsFile(const std::filesystem::path& file);

// The buff a skill casts by the skill data (SKILL_ATTRIBUTE::Effect), following a master skill down to its base
// skills until one names a buff; 0 for none.
int FindSkillDataBuff(int skill);

// The buff a skill casts (buffs design B20): the castsBuff of the skill or of a base skill below it among `skills`
// (Data/Skills/MasterSkills.json), else the skill data's (FindSkillDataBuff); 0 for none. The skill tooltip and the
// checks of the file find a skill's buff the same way.
int FindMasterSkillBuff(int skill, std::span<const MasterSkillDefinition> skills);

// At start, after the buffs and the skill data: the file into the table, checked against both, its problems logged.
// False, with the table left as it was and `errorMessage` describing them, on fatal ones.
bool LoadMasterSkills(std::string& errorMessage);
} // namespace Data::Skills
