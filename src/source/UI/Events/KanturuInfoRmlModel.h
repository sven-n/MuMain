#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
struct KanturuInfoRmlModel
{
    float boldTextPx = 0.f; // native bold text size in physical px

    Rml::String usersText;
    Rml::String monstersText;
    bool colonVisible = true; // the minute:second colon blinks every half second
    // The minutes' and the seconds' newui_number1 cells (UI::RmlBridge::DigitCells).
    std::vector<Rml::String> minuteDigits;
    std::vector<Rml::String> secondDigits;
};
} // namespace mu::ui::window
