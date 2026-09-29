#pragma once

#include <RmlUi/Core/Types.h>

namespace mu::ui::window
{
// An event time HUD on newui_Figure_blood (EventTimerView): a first line, a second line and the
// time in the big font, each centred on the same box and shrunk to it like the original's.
struct EventTimerRmlModel
{
    // The Hud layout's W/640 x H/480 stretch (UI::Scaling::GetActiveTransform() while CManager
    // runs the window) and its inverse for the counter-scaled text leaves.
    float scaleX = 1.f, scaleY = 1.f;
    float inverseScaleX = 1.f, inverseScaleY = 1.f;

    // The window's top-left, reference px (m_Pos).
    float panelX = 0.f, panelY = 0.f;

    // The box every line is centred on, reference px from the frame's left.
    float boxLeft = 0.f, boxWidth = 124.f;

    Rml::String killsText; // the first line; empty: not drawn
    float killsTextPx = 0.f;
    Rml::String killsColor;
    Rml::String timeLeftText;
    float timeLeftTextPx = 0.f;
    Rml::String timeLeftColor;
    Rml::String timeText;
    float timeTextPx = 0.f;
    Rml::String timeColor;
};
} // namespace mu::ui::window
