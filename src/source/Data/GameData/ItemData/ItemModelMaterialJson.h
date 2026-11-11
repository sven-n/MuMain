#pragma once

#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/ItemData/ItemModelDefinition.h"
#include "Data/GameData/ItemData/ItemModelJsonValues.h"

// The surface of the item model files under Enhanced lighting:
//
//   "material": { "specular": 0.6 }
//
// Values equal to their default (ItemMaterial) are left out, and so is
// "material" without other values.
namespace Data::Items::MaterialJson
{
constexpr const char* MaterialKey = "material";

constexpr double MaxSpecular = 1.0;

// Adds the material of the model to its JSON object.
void Write(const ItemModelDefinition& model, Json::OrderedJson& json);

// Reads the material of a model's JSON object into the model.
void Read(const Json::OrderedJson& json, ItemModelDefinition& model, const ModelJson::ReportIssue& report);
} // namespace Data::Items::MaterialJson
