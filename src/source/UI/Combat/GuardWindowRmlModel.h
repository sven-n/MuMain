#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// One RenderText() of the guardsman window: its box and alignment, font and colour, its size
// shrunk to the box like the original's.
struct GuardTextEntry
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

// A RenderColor() box: the lists' backdrops and selected lines.
struct GuardBoxEntry
{
    float left = 0.f;
    float top = 0.f;
    float width = 0.f;
    float height = 0.f;
    Rml::String color;
};

// A 53 x 23 newui_btn_empty_very_small button (Announce, Register, Abandon): locked ones tinted
// (100, 100, 100) with a grey label, never hovered.
struct GuardButtonEntry
{
    Rml::String label;
    int id = 0; // CGuardWindow::GUARD_BUTTON
    float top = 0.f;
    bool locked = false;
};

// One of the three newui_guild_tab04 radio tabs (56 x 22): the selected one on its second row,
// its label white, the others (181, 181, 181).
struct GuardTabEntry
{
    Rml::String label;
    float labelLeft = 0.f; // CRadioButton::Render(): whole-unit centre in the tab
    bool selected = false;
};

struct GuardWindowRmlModel
{
    // Right-docked window -- UI::Scaling::GetActiveTransform() while CManager runs it.
    float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
    float textPx = 0.f; // native normal text size in physical px (RmlRootTransform.h)
    float lineHeightPx = 0.f;
    float buttonLabelTop = 0.f; // CButton::Render(): 23 / 2 - h / 2 whole units
    float tabLabelTop = 0.f;    // CRadioButton::Render(): 22 / 2 - h / 2 whole units

    std::vector<GuardTabEntry> tabs;
    // The List tab's frame: 0 none, 1 the declared guilds (registration), 2 the siege guilds.
    int listFrame = 0;
    bool scrollShown = false;
    float scrollTop = 0.f; // the track, reference px
    float scrollHeight = 0.f;
    float thumbTop = 0.f;
    bool thumbDragged = false;
    std::vector<GuardBoxEntry> boxes;
    std::vector<GuardTextEntry> texts;
    std::vector<GuardButtonEntry> buttons;
    Rml::String exitTooltip;
};
} // namespace mu::ui::window
