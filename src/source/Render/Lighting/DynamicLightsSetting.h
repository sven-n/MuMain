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

// True when the sun's shadows come from the shadow map: in the game, with dynamic lights active,
// sun.shadow above 0 and a renderer that can draw it. Characters then cast no flat sun shadow.
[[nodiscard]] bool SunShadowsActive();

} // namespace Render::Lighting
