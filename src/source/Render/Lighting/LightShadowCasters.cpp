#include "Render/Lighting/LightShadowCasters.h"

#include <algorithm>
#include <cmath>

namespace Render::Lighting
{

LightShadowCasters SelectLightShadowCasters(const mu::DynamicLightsUniform& lights, const float (&position)[3],
                                            std::size_t maxCasters)
{
    struct Candidate
    {
        float contribution;
        float fallAngle;
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
            candidates[candidateCount++] = {contribution, std::atan2(dy, dx)};
        }
    }

    LightShadowCasters result;
    if (total <= 0.f)
    {
        return result;
    }
    const std::size_t wanted = std::min({maxCasters, kMaxLightShadowCasters, candidateCount});
    std::partial_sort(candidates.begin(), candidates.begin() + static_cast<std::ptrdiff_t>(wanted),
                      candidates.begin() + static_cast<std::ptrdiff_t>(candidateCount),
                      [](const Candidate& left, const Candidate& right)
                      { return left.contribution > right.contribution; });
    for (std::size_t i = 0; i < wanted; ++i)
    {
        const float strength = candidates[i].contribution / total;
        if (strength >= kMinLightShadowStrength)
        {
            result.casters[result.count++] = {candidates[i].fallAngle, strength};
        }
    }
    return result;
}

} // namespace Render::Lighting
