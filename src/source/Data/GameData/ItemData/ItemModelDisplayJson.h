#pragma once

#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/ItemData/ItemModelDefinition.h"
#include "Data/GameData/ItemData/ItemModelJsonValues.h"

// The display values of the item model files:
//
//   "inventory": { "anchor": [0.8, 0.85], "offset": [-0.02, 0.03], "rotation": [180, 270, 15],
//                  "scale": 0.0039, "bodyHeight": -160 },
//   "ground": { "rotation": [60, 0, -45], "scale": 0.7, "bodyHeight": -160 },
//   "cloth": true
//
// Values equal to their default (ItemInventoryDisplay, ItemGroundDisplay)
// are left out, and so is an object without other values.
namespace Data::Items::DisplayJson
{
constexpr const char* InventoryKey = "inventory";
constexpr const char* GroundKey = "ground";
constexpr const char* ClothKey = "cloth";

// Keys of the lists that are written on one line.
constexpr const char* AnchorKey = "anchor";
constexpr const char* OffsetKey = "offset";
constexpr const char* RotationKey = "rotation";

using ReportIssue = ModelJson::ReportIssue;

// Adds the display values of the model to its JSON object.
void Write(const ItemModelDefinition& model, Json::OrderedJson& json);

// Reads the display values of a model's JSON object into the model.
void Read(const Json::OrderedJson& json, ItemModelDefinition& model, const ReportIssue& report);
} // namespace Data::Items::DisplayJson
