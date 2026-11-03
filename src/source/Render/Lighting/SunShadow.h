// SunShadow.h: the sun's direction and the shadow map's view around what the camera sees.
#pragma once

#include <glm/glm.hpp>

#include <cstdint>

namespace Render::Lighting
{

// How far a character's sun shadow falls per unit of its height: the body shadow shear's
// shearX / shearHeight (ZzzBMD.cpp, RenderBodyShadow).
inline constexpr float kSunShadowFall = 0.5f;
inline constexpr float kSunShadowFallBattleCastle = 0.625f;
// How far above the ground the shadow map still catches casters (trees, buildings, flying pets).
inline constexpr float kSunShadowCasterHeight = 3000.0f;

// Unit vector towards the sun whose shadows fall towards `fallAngleDegrees` (counter-clockwise from
// world +x, as dynamic.shadowAngle) and `fallPerHeight` units per unit of height.
[[nodiscard]] glm::vec3 SunDirection(float fallAngleDegrees, float fallPerHeight);

// The dynamic.shadowAngle the maps' baked light implies: its terrain sun, (0.5, -0.5, 0.5), stands at
// azimuth 315°, so shadows fall towards 135°.
inline constexpr float kBakedSunShadowAngle = 135.0f;

// A baked light vector (the terrain's slope shading, the models' light) moved as far as the sun has
// moved from `bakedSun`, the sun the bake implies: its azimuth and elevation each change by the
// sun's, its length stays, and its elevation stays within ±89°. Exactly `baked` while the sun is
// `bakedSun`, so the maps keep their look at the baked angle.
[[nodiscard]] glm::vec3 FollowSun(const glm::vec3& baked, const glm::vec3& bakedSun, const glm::vec3& sun);

// The square, texel-snapped area of the shadow map: light-space x and y from `origin`, `size` wide,
// depth from `nearDistance` to `farDistance` along the sun's rays.
struct SunShadowBox
{
    glm::vec2 origin{};
    float size = 0.0f;
    float nearDistance = 0.0f;
    float farDistance = 0.0f;

    friend bool operator==(const SunShadowBox&, const SunShadowBox&) = default;
};

struct SunShadowView
{
    SunShadowBox box;
    glm::mat4 viewProjection{1.0f};
    // viewProjection * inverse(the scene's view-projection): turns a draw's recorded scene matrix
    // into its matrix from the sun.
    glm::mat4 sunFromScene{1.0f};
};

// The shadow map's view from the sun at `toSun` over the ground the camera sees between
// `groundLow` and `groundHigh`, at `resolution` texels a side. Clip w is 2 everywhere, so the
// vertex shaders' near-plane clip (w - 1) keeps every vertex.
[[nodiscard]] SunShadowView BuildSunShadowView(const glm::vec3& toSun, const glm::mat4& sceneViewProjection,
                                               float groundLow, float groundHigh, std::uint32_t resolution);

} // namespace Render::Lighting
