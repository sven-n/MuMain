#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace UI::Notices
{
// One notice row: its text and its colour class. An empty row keeps its place in the stack.
struct NoticeLineEntry
{
    Rml::String text;
    Rml::String kind; // "gold", "gold-dim" (the blink's half-alpha phase), "green", or "" when empty

    bool operator==(const NoticeLineEntry& other) const = default;
};

struct NoticesRmlModel
{
    // The native bold text size and line height, physical px.
    float textPx = 0.f;
    float lineHeightPx = 0.f;
    std::vector<NoticeLineEntry> lines;
};
} // namespace UI::Notices
