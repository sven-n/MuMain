#pragma once

#include "Render/Look/TuningSession.h"

// The client's player tuning of the Enhanced look, in every build: the [EnhancedTuning] section of
// config.ini (Data/GameConfig/PersonalTuning.h) applied over the look profile and the map
// (Render/Look/LookProfileSetting.h) and as the light mood's strength (Render/Look/LookPresetSetting.h).
// The options window changes it; Classic ignores it and keeps it.
namespace Render::Look
{

// Reads the section (problems are logged, at most a few) and applies it. Call once at start-up,
// after LoadProfile() and before the look preset is applied.
void LoadPersonalTuning();

[[nodiscard]] TuningSession& PersonalTuningSession();

// Applies a pending light mood strength. The frame loop calls it once per rendered frame, before
// the frame (Winmain.cpp); nothing else may, so the map light is rebuilt at most once per frame.
void ApplyPendingTuning();

} // namespace Render::Look
