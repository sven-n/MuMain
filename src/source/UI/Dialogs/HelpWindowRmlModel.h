#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// One row of the open help page. Row heights and gaps are the native text renderer's, in
// physical px (RenderTipTextList(): a row is one text height, the next starts 1.1 heights
// below it, a half spacer takes half of that).
struct HelpLineEntry
{
    Rml::String text;
    bool heading = false;
    bool halfSpacer = false;
    float heightPx = 0.f;
    float gapPx = 0.f;
};

// Everything is physical px: the document carries no transform of its own. The box is where
// RenderTipTextList(1, 1, ...) put it under the Dialog layout: a text box two units wider than
// the widest line, one unit of padding each side, framed by a 1-unit border.
struct HelpWindowRmlModel
{
    float panelX = 0.f; // the panel's border box
    float panelY = 0.f;
    float contentWidth = 0.f;
    float paddingPx = 0.f;
    float borderPx = 0.f;
    float textPx = 0.f;
    float boldTextPx = 0.f;

    std::vector<HelpLineEntry> lines;
};
} // namespace mu::ui::window
