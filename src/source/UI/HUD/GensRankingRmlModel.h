#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// One line of the Gens ranking window. Where it sits, what colour it is and how it is aligned are
// the theme's; only what it says and the size the native renderer shrank it to travel.
struct GensLine
{
    Rml::String text;
    float textPx = 0.f;

    bool operator==(const GensLine&) const = default;
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

    GensLine title;
    GensLine gensLabel;
    GensLine gensName;
    GensLine levelLabel;
    GensLine titleName;
    GensLine rankLabel;
    GensLine rankValue;
    GensLine contribLabel;
    GensLine contribValue;
    GensLine descLabel;

    // How much more contribution the next rank needs, cut to the window's width -- present only
    // while there is a next rank, so the theme gives the group its own rows.
    std::vector<GensLine> promoLines;

    // CTextBox::Render()'s visible lines from its scroll position. Their pitch is one measured
    // text height + 2, so it is the renderer's, not the theme's; the box they start from is the
    // theme's.
    std::vector<GensLine> descLines;
    float descLineStep = 0.f;

    float thumbTop = 0.f; // the scroll bar's thumb, reference px
    bool thumbActive = true;
    Rml::String exitTooltip;
};
} // namespace mu::ui::window
