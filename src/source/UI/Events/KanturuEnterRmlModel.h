#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// One drawn line of Kanturu's entry window: RenderText(x, top, text, 230, 0, RT3_SORT_CENTER) after
// SeparateTextIntoLines(), its size shrunk to the 230-unit box like the original's.
struct KanturuEnterLineEntry
{
    Rml::String text;
    float top = 0.f;    // reference px in the panel
    float textPx = 0.f; // physical px
    bool bold = false;  // the subject
    int tone = 0;       // 0 subject (255, 176, 73), 1 first state text (145, 241, 97), 2 bright yellow
};

struct KanturuEnterRmlModel
{
    // Dialog layout -- UI::Scaling::GetActiveTransform() while CManager runs this window.
    float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
    float textPx = 0.f; // native normal text size in physical px (RmlRootTransform.h)

    std::vector<KanturuEnterLineEntry> lines;
    Rml::String refreshText;
    Rml::String enterText;
    Rml::String closeText;
    // A locked button is tinted RGBA(100, 100, 100) and never hovers, like the original's.
    bool refreshLocked = false;
    bool enterLocked = false;
    // CButton::Render(): 23 / 2 - h / 2 whole units down, the native line height in physical px.
    float labelTop = 0.f;
    float labelLinePx = 0.f;
};
} // namespace mu::ui::window
