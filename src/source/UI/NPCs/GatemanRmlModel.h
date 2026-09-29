#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// One RenderText() of the gatekeeper window: its box and alignment, font and colour, its size
// shrunk to the box like the original's.
struct GatemanTextEntry
{
    Rml::String text;
    float left = 0.f; // reference px in the panel
    float top = 0.f;
    float width = 0.f; // 0 = no box
    float textPx = 0.f;
    int align = 0; // 0 left, 1 centred, 2 right edge at left + width
    bool bold = false;
    Rml::String color;
};

// A 53 x 23 newui_btn_empty_very_small button (Enter, Confirm): locked ones tinted
// (100, 100, 100) with a grey label, never hovered.
struct GatemanButtonEntry
{
    Rml::String label;
    int id = 0; // CGatemanWindow::GATEMAN_BUTTON
    float left = 0.f;
    float top = 0.f;
    bool locked = false;
};

struct GatemanRmlModel
{
    // Right-docked window -- UI::Scaling::GetActiveTransform() while CManager runs it.
    float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
    float textPx = 0.f; // native normal text size in physical px (RmlRootTransform.h)
    float lineHeightPx = 0.f;
    float labelTop = 0.f; // CButton::Render(): 23 / 2 - h / 2 whole units

    std::vector<GatemanTextEntry> texts;
    std::vector<GatemanButtonEntry> buttons;
    bool masterMode = false; // the checkbox, the fee backdrop and the fee arrows
    bool isPublic = false;
    Rml::String exitTooltip;
};
} // namespace mu::ui::window
