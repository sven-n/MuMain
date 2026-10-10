#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// One drawn line of Kanturu's entry window: RenderText(x, top, text, 230, 0, RT3_SORT_CENTER) after
// SeparateTextIntoLines(), its size shrunk to the 230-unit box like the original's. Its `top` is
// where the wrapping put it: the subject's lines, then the state texts', each group starting
// under however many lines the one before it took.
struct KanturuEnterLineEntry
{
    Rml::String text;
    bool groupStart = false; // the first line of a text, which the theme spaces from the one before
    float textPx = 0.f;      // physical px
    // What the line is: "subject", "state" for the first state text, "note" for the rest. Each
    // theme gives it a weight and a colour; the original drew the subject bold.
    Rml::String kind;

    bool operator==(const KanturuEnterLineEntry&) const = default;
};

struct KanturuEnterRmlModel
{
    float textPx = 0.f; // native normal text size in physical px (RmlNativeTextSize.h)

    std::vector<KanturuEnterLineEntry> lines;
    Rml::String refreshText;
    Rml::String enterText;
    Rml::String closeText;
    // A locked button is tinted RGBA(100, 100, 100) and never hovers, like the original's.
    bool refreshLocked = false;
    bool enterLocked = false;
    // CButton::Render(): 23 / 2 - h / 2 whole units down, the native line height in physical px.
    float labelLinePx = 0.f;
};
} // namespace mu::ui::window
