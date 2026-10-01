#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// One of the entry window's own text lines: what it says, and the size RenderText() shrank it to
// so it fits the window's box. The theme places it.
struct CursedTempleEnterLine
{
    Rml::String text;
    float textPx = 0.f; // physical px

    bool operator==(const CursedTempleEnterLine&) const = default;
};

// One level band on the entry list: the band's text and whether it is the hero's, which the theme
// marks. The original drew a red text box behind the hero's band.
struct CursedTempleEnterBand
{
    Rml::String text;
    float textPx = 0.f; // physical px
    bool hero = false;

    bool operator==(const CursedTempleEnterBand&) const = default;
};

struct CursedTempleEnterRmlModel
{
    // Dialog layout -- UI::Scaling::GetActiveTransform() while CManager runs this window.
    float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
    float textPx = 0.f;       // native normal text size in physical px (RmlRootTransform.h)
    float boldTextPx = 0.f;   // native bold text size
    float lineHeightPx = 0.f; // native normal line height, physical px (the hero band's height)

    Rml::String title;
    // Whether the hero's level reaches a band at all: with one, the temple, the bands and the
    // member count are shown; without one, the minimum-level notice is.
    bool eligible = false;
    CursedTempleEnterLine templeLine;
    std::vector<CursedTempleEnterBand> bands;
    CursedTempleEnterLine membersLine;
    CursedTempleEnterLine notice;
    Rml::String enterText;
    Rml::String closeText;
    // CButton::Render(): 23 / 2 - h / 2 whole units down, the native line height in physical px.
    float labelTop = 0.f;
    float labelLinePx = 0.f;
};
} // namespace mu::ui::window
