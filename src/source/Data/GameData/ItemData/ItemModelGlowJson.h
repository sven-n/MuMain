#pragma once

#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/ItemData/ItemModelDefinition.h"
#include "Data/GameData/ItemData/ItemModelJsonValues.h"

// The glow of the item model files:
//
//   "glow": { "level": 8, "color": "ice", "meshes": [1], "shineColor": "orange", "shineHiddenMesh": 0,
//             "ancientColor": "gold", "excellent": false }
//
// The colors are names from the glow color list; that they are in it is
// checked when the models are loaded (ValidateItemModelGlowColors). Values
// equal to their default (ItemGlow) are left out, and so is "glow" without
// other values.
namespace Data::Items::GlowJson
{
constexpr const char* GlowKey = "glow";

// Keys of the lists that are written on one line.
constexpr const char* LevelKey = "level";
constexpr const char* MeshesKey = "meshes";
constexpr const char* ShineMeshesKey = "shineMeshes";

// A level per item level has one value for each of the levels 0 to 15.
constexpr size_t ItemLevelCount = 16;
// Levels above 15 do not glow; arrows +15 glow like 31.
constexpr int MaxLevel = 31;

// Adds the glow of the model to its JSON object.
void Write(const ItemModelDefinition& model, Json::OrderedJson& json);

// Reads the glow of a model's JSON object into the model.
void Read(const Json::OrderedJson& json, ItemModelDefinition& model, const ModelJson::ReportIssue& report);
} // namespace Data::Items::GlowJson
