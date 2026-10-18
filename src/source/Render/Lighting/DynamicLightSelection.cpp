#include "Render/Lighting/DynamicLightSelection.h"

#include "Render/Lighting/DynamicLightList.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace Render::Lighting
{
namespace
{
struct Candidate
{
    float contribution;
    float distanceSquared;
    std::uint32_t index;
};

bool IsBrightening(const DynamicLight& light)
{
    return light.color[0] > 0.f || light.color[1] > 0.f || light.color[2] > 0.f;
}

float HorizontalDistanceSquared(const DynamicLight& light, const float (&position)[3])
{
    const float dx = light.position[0] - position[0];
    const float dy = light.position[1] - position[1];
    return dx * dx + dy * dy;
}

bool RanksBefore(const Candidate& left, const Candidate& right)
{
    if (left.contribution != right.contribution)
    {
        return left.contribution > right.contribution;
    }
    if (left.distanceSquared != right.distanceSquared)
    {
        return left.distanceSquared < right.distanceSquared;
    }
    return left.index < right.index;
}

void PackLight(const DynamicLight& light, std::size_t slot, mu::DynamicLightsUniform& uniform)
{
    for (int i = 0; i < 3; ++i)
    {
        uniform.positionAndRadius[slot][i] = light.position[i];
        uniform.color[slot][i] = std::max(light.color[i], 0.f);
    }
    uniform.positionAndRadius[slot][3] = light.radius;
}
} // namespace

float ContributionAt(const DynamicLight& light, const float (&position)[3])
{
    if (light.radius <= 0.f)
    {
        return 0.f;
    }
    const float distance = std::sqrt(HorizontalDistanceSquared(light, position));
    const float falloff = std::max((light.radius - distance) / light.radius, 0.f);
    const float brightness =
        std::max(light.color[0], 0.f) + std::max(light.color[1], 0.f) + std::max(light.color[2], 0.f);
    return brightness * falloff;
}

DynamicLightSelection SelectDynamicLights(std::span<const DynamicLight> lights, const float (&reference)[3])
{
    // Main-thread, once per frame: a reused buffer instead of a per-frame allocation.
    thread_local std::array<Candidate, DynamicLightList::kCapacity> candidates;

    std::size_t candidateCount = 0;
    for (std::size_t i = 0; i < lights.size() && candidateCount < candidates.size(); ++i)
    {
        if (!IsBrightening(lights[i]) || lights[i].radius <= 0.f)
        {
            continue;
        }
        candidates[candidateCount++] = {ContributionAt(lights[i], reference),
                                        HorizontalDistanceSquared(lights[i], reference), static_cast<std::uint32_t>(i)};
    }

    DynamicLightSelection selection{};
    selection.brighteningLights = candidateCount;
    const std::size_t used = std::min(candidateCount, mu::kMaxDynamicLights);
    selection.dropped = candidateCount - used;

    const auto first = candidates.begin();
    std::partial_sort(first, first + static_cast<std::ptrdiff_t>(used),
                      first + static_cast<std::ptrdiff_t>(candidateCount), RanksBefore);
    for (std::size_t slot = 0; slot < used; ++slot)
    {
        PackLight(lights[candidates[slot].index], slot, selection.uniform);
    }
    selection.uniform.lightCount = static_cast<std::uint32_t>(used);
    return selection;
}

bool DroppedLightReportLimiter::ShouldReport(std::chrono::steady_clock::time_point now)
{
    if (m_lastReport && now - *m_lastReport < kDroppedLightReportInterval)
    {
        return false;
    }
    m_lastReport = now;
    return true;
}

} // namespace Render::Lighting
