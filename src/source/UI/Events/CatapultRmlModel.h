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
    float left = 0.f; // reference px in the panel
    float top = 0.f;
    bool big = false;
    bool locked = false;
    float labelTop = 0.f; // CButton::Render(): h / 2 - text height / 2, whole units
    // CButton::Render(): w / 2 - text width / 2 whole units in, the text shrunk to the button's
    // width when wider (then running from that point, left of the button).
    float labelLeft = 0.f;
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
    // Right-docked window -- UI::Scaling::GetActiveTransform() while CManager runs it.
    float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
    float textPx = 0.f; // native normal text size in physical px (RmlRootTransform.h)
    float lineHeightPx = 0.f;

    Rml::String title;
    float titlePx = 0.f;
    std::vector<CatapultLineEntry> lines;
    std::vector<CatapultAreaEntry> areas;
    Rml::String fireText;
    bool fireLocked = true;
    float fireLabelTop = 0.f;
    Rml::String exitTooltip;
};
} // namespace mu::ui::window
