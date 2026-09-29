#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// A RenderColor() box of RenderTipTextList(): its black frame, fill, or a line's coloured text box.
struct TipTextListBoxEntry
{
    float left = 0.f; // reference px
    float top = 0.f;
    float width = 0.f;
    float height = 0.f;
    Rml::String color;
};

// A line of RenderTipTextList(): RenderText() in its box, shrunk to it like the original's.
struct TipTextListLineEntry
{
    Rml::String text;
    float left = 0.f; // reference px
    float top = 0.f;
    float width = 0.f;
    float textPx = 0.f;
    int align = 1; // 0 left, 1 centred, 2 right
    bool bold = false;
    Rml::String color;
};

struct TipTextListRmlModel
{
    // The window's layout -- UI::Scaling::GetActiveTransform() while CManager runs it.
    float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
    std::vector<TipTextListBoxEntry> boxes;
    std::vector<TipTextListLineEntry> lines;
};
} // namespace mu::ui::window
