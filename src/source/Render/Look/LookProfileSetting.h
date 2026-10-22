#pragma once

#include "Data/GameData/MapData/LookProfile.h"
#include "Render/Lighting/DynamicLightHistory.h"
#include "Render/Look/TuningControls.h"

#include <cstdint>

// The effective look profile: the live values of Data/Render/Look.json for the active world, then
// the player's own values (Render/Look/TuningSetting.h), then the values a developer changed this
// session (Render/Look/LookTuning.h).
namespace Render::Look
{

// Reads Data/Render/Look.json and Data/Maps/LightMood.json (their problems are logged once; an
// unusable look profile keeps the built-in values, an unusable light mood the Classic light) and
// hands the light mood its data. Call once at startup.
void LoadProfile();

// The values of the active world (gMapManager.WorldActive).
[[nodiscard]] const Data::Maps::LookValues& Active();
[[nodiscard]] float Value(Data::Maps::LookValue value);

// Changes whenever the live profile changes (not with the world), so a cache of derived values
// knows when to recompute.
[[nodiscard]] std::uint32_t Revision();

// The light history's settings from the active profile.
[[nodiscard]] Lighting::LightHistorySettings LightHistorySettings();

// The player's look values ([EnhancedTuning] of config.ini), applied over the map's values from the
// next frame on. They are never written into the look profile's files.
void SetPersonalLookOverrides(const Data::Maps::LookOverrides& overrides);
// Which groups' player values apply (Render::Look::TuningGroupsApplying of the look's features);
// the others are kept but not shown. Set whenever the features are applied.
void SetPersonalTuningGroups(const TuningGroupSet& applying);
// The values of the active world without the player's: what a control that follows the map shows.
[[nodiscard]] Data::Maps::LookValues InheritedValues();

} // namespace Render::Look
