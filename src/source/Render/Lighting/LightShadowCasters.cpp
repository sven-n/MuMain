#include "Render/Lighting/LightShadowCasters.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace Render::Lighting
{

float PointLightShadowHeight(float height)
{
    return std::max(height, kMinLightHeightRatio * kShadowCasterHeight);
}

LightShadowCasters SelectLightShadowCasters(const mu::DynamicLightsUniform& lights, const float (&position)[3],
                                            std::size_t maxCasters)
{
    struct Candidate
    {
        float contribution;
        float fallAngle;
        float distance;
        float height;
    };
    std::array<Candidate, mu::kMaxDynamicLights> candidates{};
    std::size_t candidateCount = 0;
    float total = 0.f;

    const std::size_t count = std::min<std::size_t>(lights.worldLightCount, mu::kMaxDynamicLights);
    for (std::size_t i = 0; i < count; ++i)
    {
        const float radius = lights.positionAndRadius[i][3];
        const float dx = position[0] - lights.positionAndRadius[i][0];
        const float dy = position[1] - lights.positionAndRadius[i][1];
        const float distance = std::sqrt(dx * dx + dy * dy);
        if (radius <= 0.f || distance >= radius)
        {
            continue;
        }
        const float contribution =
            (lights.color[i][0] + lights.color[i][1] + lights.color[i][2]) * (radius - distance) / radius;
        total += contribution;
        if (distance >= kOwnLightDistance && lights.color[i][3] > 0.f)
        {
            candidates[candidateCount++] = {contribution, std::atan2(dy, dx), distance,
                                            lights.positionAndRadius[i][2] - position[2]};
        }
    }

    LightShadowCasters result;
    if (total <= 0.f)
    {
        return result;
    }
    const std::size_t wanted = std::min({maxCasters, kMaxLightShadowCasters, candidateCount});
    std::sort(candidates.begin(), candidates.begin() + static_cast<std::ptrdiff_t>(candidateCount),
              [](const Candidate& left, const Candidate& right) { return left.contribution > right.contribution; });
    // Strongest first, each light joins the shadow of a stronger one falling the same way (a beam's
    // segments, a cluster of candles) or starts its own while there is room.
    std::array<Candidate, kMaxLightShadowCasters> shadows{};
    std::size_t shadowCount = 0;
    for (std::size_t i = 0; i < candidateCount; ++i)
    {
        const Candidate& light = candidates[i];
        Candidate* joined = nullptr;
        for (std::size_t s = 0; s < shadowCount && !joined; ++s)
        {
            const float apart = std::remainder(light.fallAngle - shadows[s].fallAngle, 2.f * std::numbers::pi_v<float>);
            joined = std::abs(apart) <= kMergeShadowAngle ? &shadows[s] : nullptr;
        }
        if (joined)
        {
            joined->contribution += light.contribution;
        }
        else if (shadowCount < wanted)
        {
            shadows[shadowCount++] = light;
        }
    }
    for (std::size_t s = 0; s < shadowCount; ++s)
    {
        const Candidate& shadow = shadows[s];
        const float strength = std::min(shadow.contribution / total, 1.f);
        if (strength >= kMinLightShadowStrength)
        {
            result.casters[result.count++] = {shadow.fallAngle, strength, shadow.distance,
                                              PointLightShadowHeight(shadow.height)};
        }
    }
    return result;
}

} // namespace Render::Lighting
