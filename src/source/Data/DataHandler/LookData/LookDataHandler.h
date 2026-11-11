#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/LookData/LookDefinition.h"

#include <filesystem>
#include <string>
#include <vector>

// Loads the looks of items as data (Data/Effects/RenderStyles.json and ItemEffects.json, see LookJson.h) into
// g_LookStore.
namespace Data::Looks
{
struct RenderStylesLoadResult
{
    std::vector<LookDefinition> looks;
    std::vector<Items::ItemDataIssue> issues;
};

// Reads the render style file of the folder (Data/Effects); a file that can't be read is fatal.
RenderStylesLoadResult LoadRenderStyleFile(const std::filesystem::path& directory);
// Reads the look file of a kind of the folder.
RenderStylesLoadResult LoadLookFile(const std::filesystem::path& directory, LookKind kind);

// Loads the render style file into g_LookStore and logs its problems; false, with the message for the player, when
// one is fatal.
bool LoadRenderStyles(std::string& errorMessage);
// The same for the item effect file.
bool LoadItemEffects(std::string& errorMessage);
} // namespace Data::Looks
