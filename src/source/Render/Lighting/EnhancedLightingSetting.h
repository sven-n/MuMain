#pragma once

#include "Render/Renderer/SceneLighting.h"

namespace Render::Lighting
{

// Switches Enhanced lighting from the next frame on (the look preset, Render::Look::ApplyLookPreset);
// nothing is saved. Returns false when it was requested but the renderer could not build it; the
// renderer then keeps drawing Classic.
bool SetEnhancedLightingEnabled(bool enabled);

// True while the renderer draws Enhanced lighting (requested and available).
[[nodiscard]] bool EnhancedLightingActive();

// Fills a frame's scene lighting with the look profile's diffuse curve, rim and specular values
// (Render::Look, session overrides included).
void ApplyTuning(mu::SceneLighting& lighting);

} // namespace Render::Lighting
