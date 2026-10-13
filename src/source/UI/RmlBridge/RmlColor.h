#pragma once

#include <RmlUi/Core/Types.h>

#include <string>

#include "Core/Utilities/_GlobalFunctions.h"

namespace UI::RmlBridge
{
// A native RGBA() colour (red in the low byte) as a CSS colour; 0 = none (empty string), so
// the theme's own colour applies.
inline Rml::String RgbaToCss(unsigned long rgba)
{
    if (rgba == 0)
        return {};
    return "rgba(" + std::to_string(GetRed(rgba)) + ", " + std::to_string(GetGreen(rgba)) + ", " +
           std::to_string(GetBlue(rgba)) + ", " + std::to_string(GetAlpha(rgba)) + ")";
}
} // namespace UI::RmlBridge
