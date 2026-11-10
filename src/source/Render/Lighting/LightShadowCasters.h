// LightShadowCasters.h: which of the frame's point lights cast a character's shadow on the ground.
#pragma once

#include "Render/Renderer/SceneLighting.h"

#include <array>
#include <cstddef>

namespace Render::Lighting
{

inline constexpr std::size_t kMaxLightShadowCasters = 4;

// A shadow a point light casts: where it falls, in radians counter-clockwise from world +x (away
// from the light), and how strong it is: the light's share of the point light at the character.
struct LightShadowCaster
{
    float fallAngle = 0.f;
    float strength = 0.f;
};

struct LightShadowCasters
{
    std::array<LightShadowCaster, kMaxLightShadowCasters> casters{};
    std::size_t count = 0;
};

// Lights nearer the character than this (horizontally, in world units) cast none: they are its own.
inline constexpr float kOwnLightDistance = 50.f;
// Weaker shares cast none.
inline constexpr float kMinLightShadowStrength = 0.05f;

// Up to `maxCasters` (at most kMaxLightShadowCasters) of the ground's lights (the first
// worldLightCount of `lights`) reaching `position`, strongest first. Every light reaching the
// character counts towards the shares, its own and skill lights (color w 0, which cast none) too.
[[nodiscard]] LightShadowCasters SelectLightShadowCasters(const mu::DynamicLightsUniform& lights,
                                                          const float (&position)[3], std::size_t maxCasters);

} // namespace Render::Lighting
