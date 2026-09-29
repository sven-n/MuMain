#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// One RenderText() of the result window: a left-aligned cell of a player row, or a centred text
// shrunk to its box like the original's.
struct CursedTempleResultTextEntry
{
    Rml::String text;
    float left = 0.f;   // reference px in the panel
    float top = 0.f;    // reference px in the panel
    float width = 0.f;  // centred box width, reference px; 0 = left-aligned, no box
    float textPx = 0.f; // physical px
    Rml::String color;  // CSS colour of the native RGBA text colour
};

struct CursedTempleResultRmlModel
{
    // Dialog layout -- UI::Scaling::GetActiveTransform() while CManager runs this window.
    float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
    float textPx = 0.f;       // native normal text size in physical px (RmlRootTransform.h)
    float lineHeightPx = 0.f; // native normal line height, physical px (the hero row's height)

    std::vector<CursedTempleResultTextEntry> texts;
    std::vector<float> heroRowTops; // reference px: the hero's rows, on a red text box
    Rml::String closeText;
    // CButton::Render(): 23 / 2 - h / 2 whole units down, the native line height in physical px.
    float labelTop = 0.f;
    float labelLinePx = 0.f;

    // The victory / defeat banner fading in above the window: 0 none, 1 success, 2 failure.
    int banner = 0;
    float bannerLeft = 0.f; // reference px in the panel
    float bannerAlpha = 0.f;
};
} // namespace mu::ui::window
