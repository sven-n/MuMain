#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/MonsterData/MonsterSoundSet.h"

#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <vector>

// Reads and writes the sound sets of the monster area
// (Data/Monsters/Sounds.json), see MonsterSoundsJson.h.
namespace Data::Monsters
{
// Data/Monsters/Sounds.json, relative to the client folder.
std::filesystem::path GetMonsterSoundsFile();

struct MonsterSoundsLoadResult
{
    std::vector<MonsterSoundSet> sets;
    std::vector<Items::ItemDataIssue> issues;
};

// Reads the file and checks its names (MonsterSoundValidation.h); a file that
// cannot be read is an error.
MonsterSoundsLoadResult LoadMonsterSoundsFile(const std::filesystem::path& file);

// The text the file gets for these sets, so that a caller can check more
// files before it writes any (with WriteTextFiles): the text when the sets
// read back without fatal problems, nullopt otherwise; the problems found
// are added to `issues`.
std::optional<std::string> MakeMonsterSoundsFileText(const std::filesystem::path& file,
                                                     std::span<const MonsterSoundSet> sets,
                                                     std::vector<Items::ItemDataIssue>& issues);
} // namespace Data::Monsters
