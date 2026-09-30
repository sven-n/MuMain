#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// One RenderText() of the window: centred on its box (or from its left edge), shrunk to it like the
// original's.
struct EventItemEntryTextEntry
{
    Rml::String text;
    float left = 0.f;   // box left, reference px in the panel
    float top = 0.f;    // reference px in the panel
    float width = 0.f;  // box width, reference px
    float textPx = 0.f; // physical px
    bool bold = false;
    Rml::String color;        // CSS colour of the native text colour
    bool leftAligned = false; // RT3_SORT_LEFT: from the box's left edge
};

// A CButton with its label (53 x 23 newui_btn_empty_very_small unless the window's theme styles it
// otherwise by its `style`); a locked one is tinted, its label grey, and never hovers.
struct EventItemEntryButtonEntry
{
    Rml::String label;
    Rml::String style; // the window's own button kind, for its theme ("" for the default)
    int index = 0;
    bool locked = false;
    bool bold = false; // the label in the bold font
    float left = 0.f;  // reference px in the panel
    float top = 0.f;
    float width = 0.f; // reference px
    float height = 0.f;
    // CButton::Render(): height / 2 - h / 2 whole units down, the label font's line height in
    // physical px.
    float labelTop = 0.f;
    float labelLinePx = 0.f;
};

struct EventItemEntryRmlModel
{
    // The window's own text field, for a document that has one (gold_bowman.rml today).
    // Two-way through data-value, so the typed text lives here rather than in the element.
    Rml::String inputValue;

    // Right-docked window -- UI::Scaling::GetActiveTransform() while CManager runs it.
    float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
    float textPx = 0.f;     // native normal text size in physical px (RmlRootTransform.h)
    float boldTextPx = 0.f; // native bold text size, for a bold button label

    std::vector<EventItemEntryTextEntry> texts;
    std::vector<EventItemEntryButtonEntry> buttons;
};

// The frame, in the background context under the live 3D preview.
struct EventItemEntryBgRmlModel
{
    float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
};
} // namespace mu::ui::window
