#pragma once

#include "Data/GameData/EffectData/EffectCreateParams.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/ItemData/ItemModelJsonValues.h"

#include <array>
#include <string>

// The creation values of an effect entry in EffectTypes.json:
//
//   "create": {
//     "lifeTime": 20,
//     "angle": {"y": 0},
//     "offset": {
//       "position": {"z": {"value": 280, "timesFrameFactor": true}}
//     },
//     "copy": {
//       "startPosition": "position"
//     }
//   }
namespace Data::Effects
{
constexpr const char* CreateKey = "create";

// The keys of the lists and the objects of components in the written text
// that go on one line.
constexpr const char* CreateLightKey = "light";
constexpr std::array<const char*, 4> CreateVectorKeys = {"position", "angle", "direction", "startPosition"};

// Reads the "create" object `json`; the issues name the fields as
// "<objectKey>.<field>". Values with errors stay unset.
EffectCreateParams ReadEffectCreateParams(const Items::Json::OrderedJson& json, const std::string& objectKey,
                                          const Items::ModelJson::ReportIssue& report);

// The "create" object: fields in a fixed order, unset values left out.
Items::Json::OrderedJson WriteEffectCreateParams(const EffectCreateParams& params);
} // namespace Data::Effects
