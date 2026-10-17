#pragma once

#include "Data/GameData/MapData/LookProfile.h"
#include "Render/Lighting/DynamicLightHistory.h"

#include <cstdint>

// The effective look profile: the live values of Data/Render/Look.json for the active world
// (developer edits included, Render/Look/LookTuning.h).
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

} // namespace Render::Look
