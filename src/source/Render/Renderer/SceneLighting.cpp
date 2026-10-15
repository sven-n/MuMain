#include "Render/Renderer/SceneLighting.h"

#include <algorithm>
#include <cmath>

namespace mu
{
namespace
{
// Fog ranges at or below this length count as "no fog", as in the Classic shaders.
constexpr float kMinimumFogRange = 0.001f;

void NormalizeInto(const float (&direction)[3], float (&destination)[4])
{
    const float length =
        std::sqrt(direction[0] * direction[0] + direction[1] * direction[1] + direction[2] * direction[2]);
    if (length <= 0.0f)
    {
        destination[0] = destination[1] = destination[2] = 0.0f;
        return;
    }
    destination[0] = direction[0] / length;
    destination[1] = direction[1] / length;
    destination[2] = direction[2] / length;
}

void CopyColor(const float (&color)[3], float (&destination)[4])
{
    destination[0] = color[0];
    destination[1] = color[1];
    destination[2] = color[2];
}
} // namespace

float EnhancedFogDensity(float start, float end)
{
    const float range = end - start;
    if (range <= kMinimumFogRange)
    {
        return 0.0f;
    }
    // 1 - exp(-(density * range)^2) = coverage  =>  density = sqrt(-ln(1 - coverage)) / range
    return std::sqrt(-std::log(1.0f - kEnhancedFogCoverageAtEnd)) / range;
}

float EnhancedFogAmount(float eyeDepth, float start, float density)
{
    const float scaledDistance = std::max(eyeDepth - start, 0.0f) * density;
    return std::clamp(1.0f - std::exp(-scaledDistance * scaledDistance), 0.0f, 1.0f);
}

float WrappedDiffuse(float normalDotLight, float wrap)
{
    return std::clamp(normalDotLight * (1.0f - wrap) + wrap, 0.0f, 1.0f);
}

SceneLightingUniform PackSceneLightingUniform(const SceneLighting& lighting, const SceneFog& fog)
{
    SceneLightingUniform uniform{};
    NormalizeInto(lighting.lightDirection, uniform.lightDirection);
    CopyColor(lighting.lightColor, uniform.lightColor);

    uniform.diffuseCurve[0] = lighting.diffuseScale;
    uniform.diffuseCurve[1] = lighting.ambient;
    uniform.diffuseCurve[2] = lighting.diffuseFloor;
    uniform.diffuseCurve[3] = lighting.highlightKnee;

    uniform.rimParameters[0] = lighting.rimStrength;
    uniform.rimParameters[1] = lighting.rimPower;

    uniform.specularParameters[0] = lighting.specularScale;
    uniform.specularParameters[1] = lighting.specularPower;

    CopyColor(fog.color, uniform.fogColor);
    uniform.fogColor[3] = fog.enabled ? 1.0f : 0.0f;
    uniform.fogParameters[0] = fog.start;
    uniform.fogParameters[1] = fog.end;
    uniform.fogParameters[2] = EnhancedFogDensity(fog.start, fog.end);

    uniform.cameraPosition[0] = lighting.cameraPosition[0];
    uniform.cameraPosition[1] = lighting.cameraPosition[1];
    uniform.cameraPosition[2] = lighting.cameraPosition[2];
    return uniform;
}

} // namespace mu
