#pragma once

#include <RmlUi/Core/Types.h>

namespace mu::ui::window
{
struct GateSwitchRmlModel
{
    float textPx = 0.f;     // native normal text size in physical px (RmlRootTransform.h)
    float boldTextPx = 0.f; // native bold text size

    Rml::String title;
    float titlePx = 0.f; // shrunk to its 160-unit box like the original's
    Rml::String line1, line2, warning;
    bool gateOpened = false;
    Rml::String buttonText;
    // The native line height in physical px; the label centres on its button.
    float labelLinePx = 0.f;
    Rml::String exitTooltip;
};
} // namespace mu::ui::window
