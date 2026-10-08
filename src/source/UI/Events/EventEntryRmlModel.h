#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// A line of the entry window's description. Which row it lands on is the theme's
// (event_entry.rcss and each window's own :nth-child table).
struct EventEntryLineEntry
{
    Rml::String text;
    float textPx = 0.f; // the native size, shrunk to the 190-unit box like the original's

    bool operator==(const EventEntryLineEntry&) const = default;
};

// One of the level buttons. Only the button for the hero's level is enabled: the original locked
// the others (frame 0, grey label) and never let them hover.
struct EventEntryButtonEntry
{
    Rml::String label;
    bool enabled = false;

    bool operator==(const EventEntryButtonEntry&) const = default;
};

struct EventEntryRmlModel
{
    float textPx = 0.f; // native normal text size in physical px (RmlRootTransform.h)
    // The title, bold, shrunk to its 72-unit box like the original's, on the native line height.
    float titleTextPx = 0.f;
    float titleLinePx = 0.f;
    // Every button's label sits the same way -- CButton::Render() derives it from the font, not
    // from which button it is.
    float buttonLabelLinePx = 0.f;
    float buttonLabelTextPx = 0.f;

    Rml::String titleText;
    Rml::String exitTooltip;
    std::vector<EventEntryLineEntry> lines;
    std::vector<EventEntryButtonEntry> buttons;
};
} // namespace mu::ui::window
