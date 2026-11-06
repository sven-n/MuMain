#pragma once

namespace Render::Lighting
{

// Switches Enhanced dynamic lights from the next frame on (the look preset,
// Render::Look::ApplyLookPreset); nothing is saved. They only take effect while Enhanced lighting
// is on.
void SetDynamicLightsEnabled(bool enabled);

// True when lit models get this frame's dynamic lights per pixel: they are on and the renderer
// draws Enhanced lighting.
[[nodiscard]] bool DynamicLightsActive();

} // namespace Render::Lighting
