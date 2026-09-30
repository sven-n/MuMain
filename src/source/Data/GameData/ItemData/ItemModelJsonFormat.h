#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ItemData/ItemModelDefinition.h"
#include "Data/GameData/ItemData/SharedItemModel.h"

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
// "file" names a .bmd file. "textureFolders" (none when left out) and
// "noneBlendMeshes" (none when left out) are optional. Instead of these
// three, "model" can name a shared model of SharedModels.json. Models are
// written sorted by number with a fixed field order.
//
// The shared models are in SharedModels.json in the same folder:
//
//   {
//     "formatVersion": 1,
//     "models": [
//       { "name": "skillParchment", "file": "Data/Item/rollofpaper.bmd", "textureFolders": ["Item"] }
//     ]
//   }
//
// written sorted by name.
namespace Data::Items
{
constexpr int ItemModelJsonFormatVersion = 1;
constexpr const char* SharedItemModelsFileName = "SharedModels.json";

// Reads one group file. Appends the models it could read to `models` and the
// problems to `issues`. `source` names the file in messages.
void ReadItemModelGroupJson(std::string_view text, const std::string& source, std::vector<ItemModelDefinition>& models,
                            std::vector<ItemDataIssue>& issues);

// Writes the models of `group` (other models are ignored). Nothing in the
// client saves model files yet; the tests use it to check that the shipped
// files are in this format.
std::string WriteItemModelGroupJson(int group, std::span<const ItemModelDefinition> models);

// Reads the shared model file. Appends the models it could read to `models`
// and the problems to `issues`.
void ReadSharedItemModelsJson(std::string_view text, const std::string& source, std::vector<SharedItemModel>& models,
                              std::vector<ItemDataIssue>& issues);

std::string WriteSharedItemModelsJson(std::span<const SharedItemModel> models);
} // namespace Data::Items
