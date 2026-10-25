#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/MonsterData/MonsterModelDefinition.h"

#include <filesystem>
#include <vector>

// Reads the model list file of the monster area (Data/Monsters/Models.json),
// see MonsterModelsJson.h.
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
} // namespace Data::Monsters
