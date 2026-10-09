#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// One row of the open help page. Row heights and gaps are the native text renderer's, in
// reference px (RenderTipTextList(): a row is one text height, the next starts 1.1 heights
// below it, a half spacer takes half of that).
struct HelpLineEntry
{
    Rml::String text;
    bool heading = false;
    bool halfSpacer = false;
    float heightPx = 0.f;
    float gapPx = 0.f;
};

// The page sits on the theme's .stage: lengths are reference px, the text sizes physical px.
// RenderTipTextList()'s text box is two units wider than the widest line.
struct HelpWindowRmlModel
{
    float contentWidth = 0.f;
    float textPx = 0.f;
    float boldTextPx = 0.f;

    std::vector<HelpLineEntry> lines;
};
} // namespace mu::ui::window
