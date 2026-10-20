// SceneLighting.h: frame-wide lighting state shared by every Enhanced-lit draw.
//
// The scene sets it once per frame through IMuRenderer::SetSceneLighting(). The light direction
// and colour of a single object (the terrain light sampled at its feet) stay per draw, exactly as
// in Classic lighting; this block holds what is the same for every draw of the frame.
#pragma once

#include <cstddef>
#include <cstdint>

namespace mu
{

// Classic model diffuse curve: max(dot(n, L) * scale + ambient, floor).
inline constexpr float kClassicDiffuseScale = 0.8f;
inline constexpr float kClassicAmbient = 0.4f;
inline constexpr float kClassicDiffuseFloor = 0.2f;

inline constexpr float kDefaultRimStrength = 0.35f;
inline constexpr float kDefaultRimPower = 3.0f;
inline constexpr float kDefaultSpecularScale = 1.0f;
inline constexpr float kDefaultSpecularPower = 32.0f;

// Share of the fog colour an Enhanced-fogged pixel has at the Classic fog end distance.
inline constexpr float kEnhancedFogCoverageAtEnd = 0.99f;

struct SceneLighting
{
    // World-space direction towards the scene's main light (Classic's default model light).
    // Models keep their per-object direction; this is the frame's reference for world-space
    // consumers.
    float lightDirection[3]{0.0f, 1.0f, 0.0f};
    // Multiplies the specular highlight, on top of the per-object light colour.
    float lightColor[3]{1.0f, 1.0f, 1.0f};
    float diffuseScale = kClassicDiffuseScale;
    float ambient = kClassicAmbient;
    float diffuseFloor = kClassicDiffuseFloor;
    // Diffuse-lit colours above this roll off towards 1 instead of being cut off there (the
    // light mood's highlight shoulder); 1 is the Classic cut-off.
    float highlightKnee = 1.0f;
    float rimStrength = kDefaultRimStrength;
    float rimPower = kDefaultRimPower;
    // Multiplies every item model's declared specular strength; 0 turns highlights off.
    float specularScale = kDefaultSpecularScale;
    float specularPower = kDefaultSpecularPower;
    // World-space camera position.
    float cameraPosition[3]{};
};

// The frame's fog as configured through IMuRenderer::SetFog() when the scene lighting was set.
struct SceneFog
{
    bool enabled = false;
    float start = 0.0f;
    float end = 0.0f;
    float color[3]{};
};

// HLSL cbuffer layout of the scene lighting block (cbuffer SceneLighting, register(b1, space3) in
// skinned_lit.frag.hlsl and basic_textured_fog.frag.hlsl). Every member fills one 16-byte register.
struct SceneLightingUniform
{
    float lightDirection[4];     // xyz normalised, w unused
    float lightColor[4];         // rgb, w unused
    float diffuseCurve[4];       // scale, ambient, floor, highlight knee
    float rimParameters[4];      // strength, power, unused, unused
    float specularParameters[4]; // scale, power, unused, unused
    float fogColor[4];           // rgb, w = 1 when the scene has fog
    float fogParameters[4];      // start, end, Enhanced density, unused
    float cameraPosition[4];     // xyz world space, w unused
};

inline constexpr std::size_t kSceneLightingRegisterSize = 16;
static_assert(offsetof(SceneLightingUniform, lightDirection) == 0 * kSceneLightingRegisterSize);
static_assert(offsetof(SceneLightingUniform, lightColor) == 1 * kSceneLightingRegisterSize);
static_assert(offsetof(SceneLightingUniform, diffuseCurve) == 2 * kSceneLightingRegisterSize);
static_assert(offsetof(SceneLightingUniform, rimParameters) == 3 * kSceneLightingRegisterSize);
static_assert(offsetof(SceneLightingUniform, specularParameters) == 4 * kSceneLightingRegisterSize);
static_assert(offsetof(SceneLightingUniform, fogColor) == 5 * kSceneLightingRegisterSize);
static_assert(offsetof(SceneLightingUniform, fogParameters) == 6 * kSceneLightingRegisterSize);
static_assert(offsetof(SceneLightingUniform, cameraPosition) == 7 * kSceneLightingRegisterSize);
static_assert(sizeof(SceneLightingUniform) == 8 * kSceneLightingRegisterSize);

[[nodiscard]] SceneLightingUniform PackSceneLightingUniform(const SceneLighting& lighting, const SceneFog& fog);

// Point lights on models light a face with saturate(dot(n, l) * (1 - wrap) + wrap): 0 is Lambert,
// 0.5 (the built-in value) half-Lambert. Up to 0.5 a face turned away from the light gets nothing.
inline constexpr float kDefaultDynamicWrap = 0.5f;
// CPU mirror of the wrapped diffuse in skinned_lit.frag.hlsl's DynamicLighting(); the shader is the
// implementation.
[[nodiscard]] float WrappedDiffuse(float normalDotLight, float wrap);

// Most point lights an Enhanced-lit draw applies per frame.
inline constexpr std::size_t kMaxDynamicLights = 32;

// HLSL cbuffer layout of the frame's point lights (cbuffer DynamicLights, register(b2, space3) in
// skinned_lit.frag.hlsl). Each light adds colour * (radius - d) / radius * wrapped diffuse, d being
// the horizontal distance, like the terrain light it comes from.
struct DynamicLightsUniform
{
    std::uint32_t lightCount = 0;     // lights used
    float wrap = kDefaultDynamicWrap; // see WrappedDiffuse()
    std::uint32_t padding[2]{};
    float positionAndRadius[kMaxDynamicLights][4]; // xyz world position, w radius in world units
    float color[kMaxDynamicLights][4];             // rgb, w unused
};

static_assert(offsetof(DynamicLightsUniform, lightCount) == 0);
static_assert(offsetof(DynamicLightsUniform, wrap) == sizeof(std::uint32_t));
static_assert(offsetof(DynamicLightsUniform, positionAndRadius) == 1 * kSceneLightingRegisterSize);
static_assert(offsetof(DynamicLightsUniform, color) == (1 + kMaxDynamicLights) * kSceneLightingRegisterSize);
static_assert(sizeof(DynamicLightsUniform) == (1 + 2 * kMaxDynamicLights) * kSceneLightingRegisterSize);

// Enhanced fog: amount = 1 - exp(-(density * max(depth - start, 0))^2). The density makes the
// amount reach kEnhancedFogCoverageAtEnd at the fog end, so Enhanced fog is clear before the
// Classic fog start and covers objects at the Classic fog end. 0 when the fog range is empty.
[[nodiscard]] float EnhancedFogDensity(float start, float end);
// CPU mirror of the shaders' EnhancedFogAmount(); the shaders are the implementation.
[[nodiscard]] float EnhancedFogAmount(float eyeDepth, float start, float density);

} // namespace mu
