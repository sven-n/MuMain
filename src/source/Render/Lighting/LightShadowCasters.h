// LightShadowCasters.h: which of the frame's point lights cast a character's shadow on the ground.
#pragma once

#include "Render/Renderer/SceneLighting.h"

#include <algorithm>
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
    // The light as mu::ShadowProjection (pointLight) takes it: its horizontal distance from the
    // character and its height above it (PointLightShadowHeight).
    float shearX = 0.f;
    float shearHeight = 0.f;
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
// Lights whose shadows fall within this angle (radians) of a stronger one's cast that one shadow
// together, their shares added: a beam's segments cast one shadow, not several faint ones.
inline constexpr float kMergeShadowAngle = 0.5f;
// How tall a casting character is taken to be, in world units.
inline constexpr float kShadowCasterHeight = 200.f;
// A light is taken to stand at least this many times the caster's height above it: lower, its
// shadow would run off to infinity.
inline constexpr float kMinLightHeightRatio = 1.25f;

// Heights within a point-light shadow are clamped to this share of the light's: a taller model
// (wings, large monsters) would otherwise project to infinity or flip.
inline constexpr float kMaxShadowHeightRatio = 0.9f;

// shearHeight for a light `height` above the character: at least kMinLightHeightRatio *
// kShadowCasterHeight, so a low light (a skill cast at mid-body) casts a long but finite shadow.
[[nodiscard]] float PointLightShadowHeight(float height);

// Where the shadow of a point `xy` (horizontally, from the origin) and `height` above the origin
// falls, as a horizontal offset from the origin. Classic (and the sun): the legacy shear along
// `axis`. pointLight: where the ray from a light `shearX` along `axis` and `shearHeight` up through the
// point meets the ground, so the shadow spreads both ways from the light.
inline void ProjectShadow(float (&xy)[2], float height, const float (&axis)[2], float shearX, float shearHeight,
                          bool pointLight)
{
    if (!pointLight)
    {
        const float along = xy[0] * axis[0] + xy[1] * axis[1];
        const float shift = height * ((along + shearX) / (height - shearHeight));
        xy[0] += shift * axis[0];
        xy[1] += shift * axis[1];
        return;
    }
    height = (std::min)(height, kMaxShadowHeightRatio * shearHeight);
    const float spread = shearHeight / (shearHeight - height);
    for (int i = 0; i < 2; ++i)
    {
        const float light = axis[i] * shearX;
        xy[i] = light + (xy[i] - light) * spread;
    }
}

// Up to `maxCasters` (at most kMaxLightShadowCasters) of the ground's lights (the first
// worldLightCount of `lights`) reaching `position`, strongest first. Every light reaching the
// character counts towards the shares, its own and skill lights (color w 0, which cast none) too.
[[nodiscard]] LightShadowCasters SelectLightShadowCasters(const mu::DynamicLightsUniform& lights,
                                                          const float (&position)[3], std::size_t maxCasters);

} // namespace Render::Lighting
