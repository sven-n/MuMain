#include "UI/RmlBridge/RmlTooltipPlacement.h"

#include <algorithm>

namespace UI::RmlBridge::TooltipPlacement
{
namespace
{
constexpr float kHintCentreOffsetUnits = 3.f;
constexpr float kHintGapUnits = 2.f;

bool Fits(float top, float height, float viewportHeight)
{
    return top >= 0.f && top + height <= viewportHeight;
}
} // namespace

ButtonHintAnchor ForButton(float left, float top, float width, float height, float unit)
{
    ButtonHintAnchor anchor;
    anchor.x = left + width / 2.f + kHintCentreOffsetUnits * unit;
    anchor.belowY = top + height + kHintGapUnits * unit;
    anchor.aboveY = top - kHintGapUnits * unit;
    return anchor;
}

float Top(float anchorY, bool above, const float* flipAnchorY, float height, float viewportHeight)
{
    float top = above ? anchorY - height : anchorY;
    if (!Fits(top, height, viewportHeight) && flipAnchorY)
    {
        const float flipped = above ? *flipAnchorY : *flipAnchorY - height;
        if (Fits(flipped, height, viewportHeight))
            top = flipped;
    }
    return std::max(0.f, std::min(top, viewportHeight - height));
}
} // namespace UI::RmlBridge::TooltipPlacement
