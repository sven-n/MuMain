#pragma once

#include <glm/glm.hpp>

#include <optional>

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

// This frame's sun (towards it): from dynamic.shadowAngle and the map's shadow length, and the sun the
// map's baked light implies (kBakedSunShadowAngle). None without Enhanced dynamic lights (Classic).
struct FrameSunDirections
{
    glm::vec3 sun{0.0f, 0.0f, 1.0f};
    glm::vec3 baked{0.0f, 0.0f, 1.0f};
};
[[nodiscard]] std::optional<FrameSunDirections> FrameSun();

// `baked` (a world-space light vector of the baked light) following this frame's sun (FollowSun);
// unchanged without one.
[[nodiscard]] glm::vec3 FollowFrameSun(const glm::vec3& baked);

} // namespace Render::Lighting
