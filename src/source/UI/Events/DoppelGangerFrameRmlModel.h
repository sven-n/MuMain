#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// One RenderText() of the frame: centred on 110 units from x 117, shrunk to them.
struct DoppelGangerFrameTextEntry
{
    Rml::String text;
    float top = 0.f;    // reference px in the frame
    float textPx = 0.f; // physical px
    bool big = false;   // the time, in the big font
    Rml::String color;  // CSS colour of the native text colour
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
    // The Hud layout's W/640 x H/480 stretch (UI::Scaling::GetActiveTransform() while CManager
    // runs the window) and its inverse for the counter-scaled text leaves.
    float scaleX = 1.f, scaleY = 1.f;
    float inverseScaleX = 1.f, inverseScaleY = 1.f;

    // The frame's top-left, reference px (m_Pos).
    float panelX = 0.f, panelY = 0.f;

    std::vector<DoppelGangerFrameTextEntry> texts;
    std::vector<DoppelGangerFrameBarEntry> bars;
    bool iceWalkerVisible = false;
    float iceWalkerLeft = 0.f;
    std::vector<DoppelGangerFrameMarkerEntry> markers;
};
} // namespace mu::ui::window
