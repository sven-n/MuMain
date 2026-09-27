#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ItemData/ItemModelDefinition.h"

#include <span>
#include <string>
#include <string_view>
#include <vector>

// The item model file format: one JSON file per item group in
// Data/Items/Models, next to the item data files.
//
//   {
//     "formatVersion": 1,
//     "group": 0,
//     "models": [
//       { "number": 0, "file": "Data/Item/Sword01.bmd", "textureFolders": ["Item"] }
//     ]
//   }
//
// "file" is required and names a .bmd file. "textureFolders" (none when
// left out) and "noneBlendMeshes" (none when left out) are optional. Models
// are written sorted by number with a fixed field order.
namespace Data::Items
{
constexpr int ItemModelJsonFormatVersion = 1;

// Reads one group file. Appends the models it could read to `models` and the
// problems to `issues`. `source` names the file in messages.
void ReadItemModelGroupJson(std::string_view text, const std::string& source, std::vector<ItemModelDefinition>& models,
                            std::vector<ItemDataIssue>& issues);

// Writes the models of `group` (other models are ignored). Nothing in the
// client saves model files yet; the tests use it to check that the shipped
// files are in this format.
std::string WriteItemModelGroupJson(int group, std::span<const ItemModelDefinition> models);
} // namespace Data::Items
