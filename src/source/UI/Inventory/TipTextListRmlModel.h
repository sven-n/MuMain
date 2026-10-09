#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// A piece of the tooltip's chrome. Its box follows the text it wraps -- the whole tooltip sizes
// itself by measuring its lines -- but which piece it is (TipTextListRecord::BoxKind) is what the
// theme colours.
struct TipTextListBoxEntry
{
    float left = 0.f; // reference px
    float top = 0.f;
    float width = 0.f;
    float height = 0.f;
    int kind = 0;
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
    std::vector<TipTextListBoxEntry> boxes;
    std::vector<TipTextListLineEntry> lines;
};
} // namespace mu::ui::window
