#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace UI::Reconnect
{
// A status line of the reconnect panel: RenderText() centred in the panel's width at `top`.
struct ReconnectLineEntry
{
    Rml::String text;
    float left = 0.f; // reference px in the panel
    float top = 0.f;
    bool bold = false;

    bool operator==(const ReconnectLineEntry& other) const = default;
};

struct ReconnectDialogRmlModel
{
    // The screen dim: its alpha 0 .. 1 (0.45 over the game or the frozen frame, 1 without one).
    float dimAlpha = 0.f;
    // The panel's reference-space corner in physical px and the active transform's scale.
    float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
    float textPx = 0.f;     // native normal text size in physical px
    float boldTextPx = 0.f; // native bold text size in physical px
    std::vector<ReconnectLineEntry> lines;
    float progressWidth = 0.f; // the gauge's fill, 0 .. 150 reference px
};
} // namespace UI::Reconnect
