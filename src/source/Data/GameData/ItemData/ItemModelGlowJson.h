#pragma once

#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/ItemData/ItemModelDefinition.h"
#include "Data/GameData/ItemData/ItemModelJsonValues.h"

#include <functional>
#include <string>

// The glow of the item model files:
//
//   "glow": { "level": 8, "color": "ice", "meshes": [1], "glowAgainInWhite": 1, "shineColor": "orange",
//             "shineHiddenMesh": 0, "shineBelowPlus3": "seal", "ancientColor": "gold", "excellent": false }
//
// The colors are names from the glow color list, the shine below +3 one from
// the shine list; that they are in them is checked when the models are loaded
// (ValidateItemModelGlowColors, ValidateItemModelShines). Values
// equal to their default (ItemGlow) are left out, and so is "glow" without
// other values.
namespace Data::Items::GlowJson
{
constexpr const char* GlowKey = "glow";

// Keys of the lists that are written on one line.
constexpr const char* LevelKey = "level";
constexpr const char* MeshesKey = "meshes";
constexpr const char* ShineMeshesKey = "shineMeshes";
constexpr const char* ShineBelowPlus3Key = "shineBelowPlus3";

// Levels above 15 do not glow; arrows +15 glow like 31.
constexpr int MaxLevel = 31;

// Adds the glow of the model to its JSON object.
void Write(const ItemModelDefinition& model, Json::OrderedJson& json);

// Reads the glow of a model's JSON object into the model.
void Read(const Json::OrderedJson& json, ItemModelDefinition& model, const ModelJson::ReportIssue& report);

// Calls visit(field, mesh) for every mesh the glow names, with the field as
// "glow.meshes", "glow.excellentMesh", ...
void ForEachMesh(const ItemGlow& glow, const std::function<void(const std::string& field, int mesh)>& visit);
} // namespace Data::Items::GlowJson
