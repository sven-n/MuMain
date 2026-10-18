#pragma once

#include <RmlUi/Core/Types.h>

namespace UI::Reconnect
{
struct ReconnectDialogRmlModel
{
    // The screen dim: its alpha 0 .. 1 (0.45 over the game or the frozen frame, 1 without one).
    float dimAlpha = 0.f;
    // The panel's reference-space corner in physical px and the active transform's scale.
    float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
    float textPx = 0.f;     // native normal text size in physical px
    float boldTextPx = 0.f; // native bold text size in physical px
    // The panel's three status lines. Each one's row, weight and colour are the theme's; the
    // countdown is empty until there is one.
    Rml::String titleText;
    Rml::String stepText;
    Rml::String countdownText;
    float progressWidth = 0.f; // the gauge's fill, 0 .. 150 reference px
};
} // namespace UI::Reconnect
