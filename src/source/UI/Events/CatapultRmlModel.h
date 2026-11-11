#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// A target area button (newui_Btn_gate 46 x 36, or newui_Btn_round 77 x 47): the selected one
// locked, tinted (100, 100, 100) with a grey label.
struct CatapultAreaEntry
{
    Rml::String label;
    int index = 0;
    bool big = false;   // newui_Btn_round 77 x 47, else newui_Btn_gate 46 x 36
    bool locked = false;
    // The text shrunk to the button's width when wider; the centring itself is the theme's.
    float labelPx = 0.f;
};

// One of the three instruction lines, shrunk to its 190-unit box like the original's.
struct CatapultLineEntry
{
    Rml::String text;
    float textPx = 0.f;
};

struct CatapultRmlModel
{
    // CCatapultWindow::CATAPULT_ATTACK or _DEFENSE: the two have different target areas,
    // and the theme lays each set out.
    int mode = 0;
    float textPx = 0.f; // native normal text size in physical px (RmlNativeTextSize.h)
    float lineHeightPx = 0.f;

    Rml::String title;
    float titlePx = 0.f;
    std::vector<CatapultLineEntry> lines;
    std::vector<CatapultAreaEntry> areas;
    Rml::String fireText;
    bool fireLocked = true;
    Rml::String exitTooltip;
};
} // namespace mu::ui::window
