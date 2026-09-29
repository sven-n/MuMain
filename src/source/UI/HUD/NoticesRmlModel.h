#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace UI::Notices
{
// One drawn notice line: its text, its colour class and its top edge in physical px.
struct NoticeLineEntry
{
    Rml::String text;
    Rml::String kind; // "gold", "gold-dim" (the blink's half-alpha phase) or "green"
    float top = 0.f;

    bool operator==(const NoticeLineEntry& other) const
    {
        return top == other.top && kind == other.kind && text == other.text;
    }
};

struct NoticesRmlModel
{
    // Physical px: the rows span 0..rowWidth so their centre is the original's x 320; the native
    // bold text size and line height.
    float rowWidth = 0.f;
    float textPx = 0.f;
    float lineHeightPx = 0.f;
    std::vector<NoticeLineEntry> lines;
};
} // namespace UI::Notices
