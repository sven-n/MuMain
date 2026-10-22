#include "Render/PostProcessing/BloomComposite.h"

namespace Render::PostProcessing
{

float BloomWeightSum(std::size_t levelCount, float levelFalloff)
{
    float sum = 0.0f;
    float weight = 1.0f;
    for (std::size_t level = 0; level < levelCount; ++level)
    {
        sum += weight;
        weight *= levelFalloff;
    }
    return sum;
}

float CompositeGradingStrength(bool hasTable, bool diagnosticView, float strength)
{
    if (!hasTable || diagnosticView)
    {
        return 0.0f;
    }
    return strength;
}

float BlendGrading(float ungraded, float graded, float strength)
{
    return ungraded + strength * (graded - ungraded);
}

} // namespace Render::PostProcessing
