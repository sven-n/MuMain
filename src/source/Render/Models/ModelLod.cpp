#include "Render/Models/ModelLod.h"

#include <algorithm>

namespace Render::Models
{
int SelectLod(float screenHeight, int previousLod, const Data::Models::LodSettings& settings, int lodCount)
{
    const int levels = std::min(lodCount - 1, static_cast<int>(settings.screenHeights.size()));
    if (levels <= 0)
    {
        return 0;
    }
    int lod = previousLod >= 0 && previousLod <= levels ? previousLod : 0;
    while (lod < levels && screenHeight < settings.screenHeights[static_cast<size_t>(lod)])
    {
        ++lod;
    }
    while (lod > 0 && screenHeight > settings.screenHeights[static_cast<size_t>(lod - 1)] * (1.f + settings.hysteresis))
    {
        --lod;
    }
    return lod;
}

float ScreenHeightFraction(std::span<const float> rows, bool allInFront, float windowHeight)
{
    if (!allInFront || rows.empty() || windowHeight <= 0.f)
    {
        return 1.f;
    }
    const auto [lowest, highest] = std::minmax_element(rows.begin(), rows.end());
    return (*highest - *lowest) / windowHeight;
}
} // namespace Render::Models
