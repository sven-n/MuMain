#pragma once

#include "Data/GameData/EffectData/EffectKind.h"
#include "Data/GameData/EffectData/EffectTypesJson.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"

#include <array>
#include <filesystem>
#include <vector>

// Reads the catalogue files of the effect types (Data/Effects), see
// EffectTypesJson.h.
namespace Data::Effects
{
struct EffectTypesLoadResult
{
    std::array<std::vector<EffectTypeEntry>, EffectKindCount> types;
    std::vector<Items::ItemDataIssue> issues;
};

// Data/Effects, relative to the client folder.
std::filesystem::path GetEffectDataDirectory();

// Reads and validates the catalogue file of every kind in the folder.
EffectTypesLoadResult LoadEffectTypeFiles(const std::filesystem::path& directory);
} // namespace Data::Effects
