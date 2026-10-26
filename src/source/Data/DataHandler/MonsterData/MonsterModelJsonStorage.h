#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/MonsterData/MonsterModelDefinition.h"

#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <vector>

// Reads and writes the model list file of the monster area
// (Data/Monsters/Models.json), see MonsterModelsJson.h.
namespace Data::Monsters
{
// Data/Monsters/Models.json, relative to the client folder.
std::filesystem::path GetMonsterModelsFile();

struct MonsterModelsLoadResult
{
    std::vector<MonsterModelDefinition> models;
    std::vector<Items::ItemDataIssue> issues;
};

// Reads the file and checks its names (MonsterModelValidation.h, the effect
// catalogue must be loaded) and that its model files are in the working
// folder; a file that cannot be read is an error.
MonsterModelsLoadResult LoadMonsterModelsFile(const std::filesystem::path& file);

// Writes the models to the file when they read back without errors, with the
// checks of LoadMonsterModelsFile; the problems found are added to `issues`
// (a missing model file is a warning and does not stop the save). The old
// file stays when the new one cannot be written whole.
bool SaveMonsterModelsFile(const std::filesystem::path& file, std::span<const MonsterModelDefinition> models,
                           std::vector<Items::ItemDataIssue>& issues);

// The text SaveMonsterModelsFile writes, so that a caller can check more files
// before it writes any (with WriteTextFiles): the text for the file when the
// models read back without errors, nullopt otherwise; the problems found are
// added to `issues`.
std::optional<std::string> MakeMonsterModelsFileText(const std::filesystem::path& file,
                                                     std::span<const MonsterModelDefinition> models,
                                                     std::vector<Items::ItemDataIssue>& issues);
} // namespace Data::Monsters
