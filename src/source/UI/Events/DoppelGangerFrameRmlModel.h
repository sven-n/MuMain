#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// One of the frame's three lines: centred on 110 units from x 117 and shrunk to them, like
// RenderText() drew it. Where it sits and what colour it is are the theme's.
struct DoppelGangerFrameTextEntry
{
    Rml::String text;
    float textPx = 0.f; // physical px

    bool operator==(const DoppelGangerFrameTextEntry&) const = default;
};

// One gauge bar piece: the right-aligned part of a Double_bar texture, from its texel sourceX,
// sourceWidth wide (6 texels high), stretched over `width` reference px from `left` at y 78.
struct DoppelGangerFrameBarEntry
{
    Rml::String src;  // the bar texture, relative to the themed document
    Rml::String rect; // "sourceX 0 sourceWidth 6"
    float left = 0.f;
    float width = 0.f;
};

// A party member's position on the path: the hero's own marker or a member's.
struct DoppelGangerFrameMarkerEntry
{
    float left = 0.f;
    bool hero = false;
};

struct DoppelGangerFrameRmlModel
{
    DoppelGangerFrameTextEntry passedLine, timeLabel, timeLine;
    // How many monsters have got through: "none", "one", "several". The original reddened the
    // line as they did; the theme owns that now.
    Rml::String passedState;
    std::vector<DoppelGangerFrameBarEntry> bars;
    bool iceWalkerVisible = false;
    float iceWalkerLeft = 0.f;
    std::vector<DoppelGangerFrameMarkerEntry> markers;
};
} // namespace mu::ui::window
