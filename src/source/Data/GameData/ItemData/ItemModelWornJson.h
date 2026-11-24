#pragma once

#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/ItemData/ItemModelDefinition.h"
#include "Data/GameData/ItemData/ItemModelJsonValues.h"

// Where a worn wing or cape hangs on the character:
//
//   "worn": { "bone": 19, "link": true, "offset": [0, 0, 15] }
//
// Values equal to their default (ItemWorn) are left out, and so is "worn"
// without other values.
namespace Data::Items::WornJson
{
constexpr const char* WornKey = "worn";

constexpr int MaxBone = 255;

// Adds where the model is worn to its JSON object.
void Write(const ItemModelDefinition& model, Json::OrderedJson& json);

// Reads where the model is worn from its JSON object into the model.
void Read(const Json::OrderedJson& json, ItemModelDefinition& model, const ModelJson::ReportIssue& report);
} // namespace Data::Items::WornJson
