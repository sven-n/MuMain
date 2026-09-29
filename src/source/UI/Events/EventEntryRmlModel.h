#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// A line of the entry window's description: RenderText(x + 3, top, text, 190, 0, RT3_SORT_CENTER).
struct EventEntryLineEntry
{
    Rml::String text;
    float top = 0.f;    // reference px in the panel
    float textPx = 0.f; // the native size, shrunk to the 190-unit box like the original's
};

// One of the level buttons. Only the button for the hero's level is enabled: the original locked
// the others (frame 0, grey label) and never let them hover.
struct EventEntryButtonEntry
{
    Rml::String label;
    int index = 0;
    bool enabled = false;
    float top = 0.f;      // reference px in the panel
    float labelTop = 0.f; // CButton::Render(): 29 / 2 - textHeight / 2, whole units
    float labelLinePx = 0.f;
    float labelTextPx = 0.f;
};

struct EventEntryRmlModel
{
    // Right-docked window -- UI::Scaling::GetActiveTransform() while CManager runs it.
    float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
    float textPx = 0.f; // native normal text size in physical px (RmlRootTransform.h)
    // The title, bold, shrunk to its 72-unit box like the original's, on the native line height.
    float titleTextPx = 0.f;
    float titleLinePx = 0.f;

    Rml::String titleText;
    Rml::String exitTooltip;
    std::vector<EventEntryLineEntry> lines;
    std::vector<EventEntryButtonEntry> buttons;
};
} // namespace mu::ui::window
