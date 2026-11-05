#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/MonsterData/MonsterDefinition.h"

#include <cstddef>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
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

// Writes the monsters to the file when they read back without fatal problems; the
// problems found are added to `issues`. The old file stays when the new one
// cannot be written whole.
bool SaveMonstersFile(const std::filesystem::path& file, std::span<const MonsterDefinition> monsters,
                      std::vector<Items::ItemDataIssue>& issues);

// The text SaveMonstersFile writes, so that a caller can check more files
// before it writes any (with WriteTextFiles): the text for the file when the
// monsters read back without fatal problems, nullopt otherwise; the problems found are
// added to `issues`.
std::optional<std::string> MakeMonstersFileText(const std::filesystem::path& file,
                                                std::span<const MonsterDefinition> monsters,
                                                std::vector<Items::ItemDataIssue>& issues);

// Makes `file` the source of the problems from `first` on: the checks of the
// monsters find them without a file.
void NameIssueSource(std::vector<Items::ItemDataIssue>& issues, std::size_t first, const std::filesystem::path& file);
} // namespace Data::Monsters
