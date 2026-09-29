#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// One RenderText() of the Gens ranking window: its box, alignment and colour, its size shrunk to
// the box like the original's.
struct GensRankingTextEntry
{
    Rml::String text;
    float left = 0.f; // reference px in the panel
    float top = 0.f;
    float width = 0.f; // 0 = no box
    float textPx = 0.f;
    int align = 0; // 0 left, 1 centred, 2 right
    bool bold = false;
    Rml::String color;
};

struct GensRankingRmlModel
{
    // Right-docked window -- UI::Scaling::GetActiveTransform() while CManager runs it. Its layout
    // mode is Hud (ScreenOverlayTransform): the original drew it stretched per axis, so rootScale
    // (x) and rootScaleY differ on a window that is not 4:3.
    float rootX = 0.f, rootY = 0.f, rootScale = 1.f, rootScaleY = 1.f;
    float textPx = 0.f; // native normal text size in physical px (RmlRootTransform.h)

    // The family mark: "d" (Duprian) or "v" (Vanert) and the rank's cell 0..13, empty for none.
    Rml::String markSprite;
    std::vector<GensRankingTextEntry> texts;
    float thumbTop = 0.f; // the scroll bar's thumb, reference px
    bool thumbActive = true;
    Rml::String exitTooltip;
};
} // namespace mu::ui::window
