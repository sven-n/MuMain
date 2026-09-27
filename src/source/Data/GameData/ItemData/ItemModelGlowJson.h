#pragma once

#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/ItemData/ItemModelDefinition.h"
#include "Data/GameData/ItemData/ItemModelJsonValues.h"

// The glow of the item model files:
//
//   "glow": { "level": 8, "color": [0.5, 0.8, 0.9], "meshes": [1], "shineColor": [1, 0.5, 0],
//             "shineHiddenMesh": 0, "ancientColor": [1, 0.7, 0.2], "excellent": false }
//
// Values equal to their default (ItemGlow) are left out, and so is "glow"
// without other values.
namespace Data::Items::GlowJson
{
constexpr const char* GlowKey = "glow";

// Keys of the lists that are written on one line.
constexpr const char* ColorKey = "color";
constexpr const char* MeshesKey = "meshes";
constexpr const char* ShineColorKey = "shineColor";
constexpr const char* ShineMeshesKey = "shineMeshes";
constexpr const char* AncientColorKey = "ancientColor";

// Glow levels are item levels.
constexpr int MaxLevel = 15;

// Adds the glow of the model to its JSON object.
void Write(const ItemModelDefinition& model, Json::OrderedJson& json);

// Reads the glow of a model's JSON object into the model.
void Read(const Json::OrderedJson& json, ItemModelDefinition& model, const ModelJson::ReportIssue& report);
} // namespace Data::Items::GlowJson
