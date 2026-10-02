#pragma once

#include "Data/GameData/EffectData/EffectCreateParams.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/ItemData/ItemModelJsonValues.h"

#include <string>

// The creation values of an effect entry in EffectTypes.json:
//
//   "create": { "lifeTime": 200, "scale": 1.8, "blendMesh": -2, "light": [0.5, 0.5, 0.5] }
namespace Data::Effects
{
constexpr const char* CreateKey = "create";

// The key of the list in the written text that goes on one line.
constexpr const char* CreateLightKey = "light";

// Reads the "create" object `json`; the issues name the fields as
// "<objectKey>.<field>". Values with errors stay unset.
EffectCreateParams ReadEffectCreateParams(const Items::Json::OrderedJson& json, const std::string& objectKey,
                                          const Items::ModelJson::ReportIssue& report);

// The "create" object: fields in a fixed order, unset values left out.
Items::Json::OrderedJson WriteEffectCreateParams(const EffectCreateParams& params);
} // namespace Data::Effects
