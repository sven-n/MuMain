#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// One RenderNumber() digit: newui_number1's 12 x 14 texel cell at `rect`, drawn 8.4 x 11.2
// reference px at `left` (the original's scale 1: 12 x 16 texel units times 0.7).
struct KanturuInfoDigitEntry
{
    float left = 0.f; // reference px in the frame
    Rml::String rect; // "digit*12 0 12 14"
};

struct KanturuInfoRmlModel
{
    // The Hud layout's W/640 x H/480 stretch (UI::Scaling::GetActiveTransform() while CManager
    // runs the window) and its inverse for the counter-scaled text leaves.
    float scaleX = 1.f, scaleY = 1.f;
    float inverseScaleX = 1.f, inverseScaleY = 1.f;
    float boldTextPx = 0.f; // native bold text size in physical px

    // The frame's top-left, reference px (m_Pos).
    float panelX = 0.f, panelY = 0.f;

    Rml::String usersText;
    Rml::String monstersText;
    bool colonVisible = true; // the minute:second colon blinks every half second
    std::vector<KanturuInfoDigitEntry> digits;
};
} // namespace mu::ui::window
