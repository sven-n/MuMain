#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// A complete translated fragment. The theme wraps and scrolls the description.
struct EventEntryLineEntry
{
    Rml::String text;

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
    // Native-size compatibility metrics; the theme owns wrapping and all box dimensions.
    float textPx = 0.f;
    float boldTextPx = 0.f;

    Rml::String titleText;
    Rml::String exitTooltip;
    std::vector<EventEntryLineEntry> lines;
    std::vector<EventEntryButtonEntry> buttons;
};
} // namespace mu::ui::window
