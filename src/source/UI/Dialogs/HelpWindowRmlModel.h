#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// One row of the open help page; the theme spaces the rows from the line heights.
struct HelpLineEntry
{
    Rml::String text;
    bool heading = false;
    bool halfSpacer = false;
};

// The page sits on the theme's .stage: the text sizes are physical px, the line heights the native
// renderer's, in reference px.
struct HelpWindowRmlModel
{
    float textPx = 0.f;
    float boldTextPx = 0.f;
    float lineHeight = 0.f;
    float boldLineHeight = 0.f;

    std::vector<HelpLineEntry> lines;
};
} // namespace mu::ui::window
