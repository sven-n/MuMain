#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
struct KanturuInfoRmlModel
{
    // The HudFrame layout's scale (UI::Scaling::GetActiveTransform() while CManager
    // runs the window) and its inverse for the counter-scaled text leaves.
    float scaleX = 1.f, scaleY = 1.f;
    float boldTextPx = 0.f; // native bold text size in physical px

    // The frame's top-left, reference px (m_Pos).
    float panelX = 0.f, panelY = 0.f;

    Rml::String usersText;
    Rml::String monstersText;
    bool colonVisible = true; // the minute:second colon blinks every half second
    // The minutes' and the seconds' newui_number1 cells (UI::RmlBridge::DigitCells).
    std::vector<Rml::String> minuteDigits;
    std::vector<Rml::String> secondDigits;
};
} // namespace mu::ui::window
