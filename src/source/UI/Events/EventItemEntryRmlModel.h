#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// One RenderText() of the window. Where it sits, what colour it is and how it is aligned are the
// theme's (each document's own <document>_rows.rcss, one :nth-child rule per line), and so is the
// box it centres on.
struct EventItemEntryTextEntry
{
    Rml::String text;
    float textPx = 0.f; // physical px

    bool operator==(const EventItemEntryTextEntry&) const = default;
};

// A CButton with its label. Which button it is -- where it sits, how big it is and which sprite it
// wears -- is the theme's, by the same row rules; a locked one is tinted, its label grey, and never
// hovers.
struct EventItemEntryButtonEntry
{
    Rml::String label;
    bool locked = false;
    bool bold = false; // the label in the bold font
    // CButton::Render(): height / 2 - h / 2 whole units down, the label font's line height in
    // physical px. Both follow the button's own height, so they stay per button.
    float labelLinePx = 0.f;
    Rml::String hint;

    bool operator==(const EventItemEntryButtonEntry&) const = default;
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

} // namespace mu::ui::window
