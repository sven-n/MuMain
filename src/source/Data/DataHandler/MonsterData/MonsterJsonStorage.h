#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/MonsterData/MonsterDefinition.h"

#include <filesystem>
#include <span>
#include <vector>

// Reads and writes the monster list file (Data/Monsters/Monsters.json), see
// MonstersJson.h.
namespace Data::Monsters
{
// Data/Monsters/Monsters.json, relative to the client folder.
std::filesystem::path GetMonstersFile();

struct MonsterDataLoadResult
{
    std::vector<MonsterDefinition> monsters;
    std::vector<Items::ItemDataIssue> issues;
};

// Reads and checks the file; one that cannot be read is an error.
MonsterDataLoadResult LoadMonstersFile(const std::filesystem::path& file);

// Writes the monsters to the file when they read back without errors; the
// problems found are added to `issues`. The old file stays when the new one
// cannot be written whole.
bool SaveMonstersFile(const std::filesystem::path& file, std::span<const MonsterDefinition> monsters,
                      std::vector<Items::ItemDataIssue>& issues);
} // namespace Data::Monsters
