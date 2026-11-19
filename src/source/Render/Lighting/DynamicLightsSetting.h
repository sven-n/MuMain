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

// The sun ([Render] Sunlight). Needs dynamic lights: its ground shadows draw in the per-pixel ground pass.
void SetSunlightEnabled(bool enabled);
[[nodiscard]] bool SunlightActive();

// True when the sun's shadows come from the shadow map: in the game, with dynamic lights active,
// sun.shadow above 0 and a renderer that can draw it, taken once per frame (UpdateFrameSun). Characters then cast no
// flat sun shadow.
[[nodiscard]] bool SunShadowsActive();
// sun.shadow where sun shadows are active, else 0: everything that comes with them fades by it.
[[nodiscard]] float SunShadowShare();

// This frame's sun (towards it): from dynamic.shadowAngle and the map's shadow length, and the sun the
// map's baked light implies (kBakedSunShadowAngle). None without Enhanced dynamic lights (Classic).
// Taken once per frame (UpdateFrameSun), so every part of a frame sees the same sun.
struct FrameSunDirections
{
    glm::vec3 sun{0.0f, 0.0f, 1.0f};
    glm::vec3 baked{0.0f, 0.0f, 1.0f};
};
[[nodiscard]] std::optional<FrameSunDirections> FrameSun();

// Takes this frame's sun; called first thing each frame (InitTerrainLight).
void UpdateFrameSun();

// `baked` (a world-space light vector of the baked light) following this frame's sun (FollowSun);
// unchanged without one.
[[nodiscard]] glm::vec3 FollowFrameSun(const glm::vec3& baked);

} // namespace Render::Lighting
