#pragma once

#include "Data/GameData/BuffData/BuffDefinition.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/SkillData/MasterSkillDefinition.h"

#include <functional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// Data/Skills/MasterSkills.json: reading, writing and the checks of its entries (buffs design B18).
namespace Data::Skills
{
// "masterSkills[3]", or "masterSkills[3].raises" with a key.
std::string MasterSkillField(size_t entry, std::string_view key);

// Appends the file's entries to `skills` and its problems to `issues`; an entry with a fatal problem is left out.
// `fileEntries`, when given, gets each read entry's index in the file.
void ReadMasterSkillsJson(std::string_view text, const std::string& source, std::vector<MasterSkillDefinition>& skills,
                          std::vector<Items::ItemDataIssue>& issues, std::vector<size_t>* fileEntries = nullptr);

std::string WriteMasterSkillsJson(std::span<const MasterSkillDefinition> skills);

// The checks of the entries among themselves: a skill twice, a name that is no buff attribute, an entry that adds
// nothing. `entries` gives each entry's index in the file (empty: the position).
void ValidateMasterSkills(std::span<const MasterSkillDefinition> skills, std::span<const size_t> entries,
                          const std::string& source, std::vector<Items::ItemDataIssue>& issues);

// The checks against the buffs: castsBuff names a buff of the list, and the attribute a skill raises is a power-up of
// the buff it casts. `castBuff` gives the buff number a skill casts (FindMasterSkillBuff, as the skill tooltip finds
// it); where it gives 0, or is empty, the entry's own castsBuff counts.
void ValidateMasterSkillBuffs(std::span<const MasterSkillDefinition> skills, std::span<const size_t> entries,
                              std::span<const Buffs::BuffDefinition> buffs,
                              const std::function<int(int skill)>& castBuff, const std::string& source,
                              std::vector<Items::ItemDataIssue>& issues);
} // namespace Data::Skills
