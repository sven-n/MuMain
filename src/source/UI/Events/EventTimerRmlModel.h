#pragma once

#include <RmlUi/Core/Types.h>

namespace mu::ui::window
{
// An event time HUD on newui_Figure_blood (EventTimerView): a first line, a second line and the
// time in the big font, each centred on the same box and shrunk to it like the original's. Each
// line carries how pressing it is (EventTimerView::Line::state), which the theme colours.
struct EventTimerRmlModel
{
    float textPx = 0.f; // the native text sizes in physical px
    float bigTextPx = 0.f;

    Rml::String killsText; // the first line; empty: not drawn
    Rml::String killsState;
    Rml::String timeLeftText;
    Rml::String timeLeftState;
    Rml::String timeText;
    Rml::String timeState;
};
} // namespace mu::ui::window
