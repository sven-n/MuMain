// DynamicLightSelection.h: which of the frame's dynamic lights the Enhanced-lit draws apply.
#pragma once

#include "Render/Lighting/DynamicLight.h"
#include "Render/Renderer/SceneLighting.h"

#include <chrono>
#include <cstddef>
#include <optional>
#include <span>

namespace Render::Lighting
{

struct DynamicLightSelection
{
    mu::DynamicLightsUniform uniform{};
    // Lights with a brightening (positive) colour component; only these are applied per pixel.
    std::size_t brighteningLights = 0;
    // Brightening lights left out because the cap was reached.
    std::size_t dropped = 0;
};

// How much a light adds at a position: the sum of its positive colour components times its
// falloff over the horizontal distance; 0 at or beyond its radius.
[[nodiscard]] float ContributionAt(const DynamicLight& light, const float (&position)[3]);

// Packs the brightening lights for the shader, at most mu::kMaxDynamicLights of them. With more,
// the ones contributing most at the reference position (the player's character) win; equal
// contributions go to the nearer light, then to the earlier one. Negative colour components are
// zeroed: darkening is part of the models' base light (RequestStaticTerrainLight).
[[nodiscard]] DynamicLightSelection SelectDynamicLights(std::span<const DynamicLight> lights,
                                                        const float (&reference)[3]);

// Shortest time between two log lines about dropped lights.
inline constexpr std::chrono::minutes kDroppedLightReportInterval{1};

// Lets the dropped-light report through at most once per kDroppedLightReportInterval.
class DroppedLightReportLimiter
{
public:
    // True when a report is due at now; the next one is then due an interval later.
    [[nodiscard]] bool ShouldReport(std::chrono::steady_clock::time_point now);

private:
    std::optional<std::chrono::steady_clock::time_point> m_lastReport;
};

} // namespace Render::Lighting
