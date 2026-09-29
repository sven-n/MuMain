#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// One RenderText() of the window: centred on its box, shrunk to it like the original's.
struct EventItemEntryTextEntry
{
    Rml::String text;
    float left = 0.f;   // box left, reference px in the panel
    float top = 0.f;    // reference px in the panel
    float width = 0.f;  // box width, reference px
    float textPx = 0.f; // physical px
    bool bold = false;
    Rml::String color; // CSS colour of the native text colour
};

// A 53 x 23 CButton (newui_btn_empty_very_small) with its label; a locked one is tinted
// (100, 100, 100), its label grey, and never hovers.
struct EventItemEntryButtonEntry
{
    Rml::String label;
    int index = 0;
    bool locked = false;
    float left = 0.f; // reference px in the panel
    float top = 0.f;
};

struct EventItemEntryRmlModel
{
    // Right-docked window -- UI::Scaling::GetActiveTransform() while CManager runs it.
    float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
    float textPx = 0.f; // native normal text size in physical px (RmlRootTransform.h)
    // CButton::Render(): 23 / 2 - h / 2 whole units down, the native line height in physical px.
    float labelTop = 0.f;
    float labelLinePx = 0.f;

    std::vector<EventItemEntryTextEntry> texts;
    std::vector<EventItemEntryButtonEntry> buttons;
};

// The frame, in the background context under the live 3D preview.
struct EventItemEntryBgRmlModel
{
    float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
};
} // namespace mu::ui::window
