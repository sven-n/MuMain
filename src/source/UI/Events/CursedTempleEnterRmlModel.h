#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// One line of the entry window (RenderText at y, centred on its box): its size shrunk to the box
// like the original's, and, for the hero's level row, the red text box behind it.
struct CursedTempleEnterLineEntry
{
    Rml::String text;
    float top = 0.f;          // reference px in the panel
    float left = 0.f;         // box left, reference px
    float width = 0.f;        // box width, reference px
    float textPx = 0.f;       // physical px
    bool highlighted = false; // RGBA(255, 0, 0, 255) text box
    bool red = false;         // red text (members count, level too low)
};

struct CursedTempleEnterRmlModel
{
    // Dialog layout -- UI::Scaling::GetActiveTransform() while CManager runs this window.
    float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
    float textPx = 0.f;       // native normal text size in physical px (RmlRootTransform.h)
    float boldTextPx = 0.f;   // native bold text size
    float lineHeightPx = 0.f; // native normal line height, physical px (the highlight's height)

    Rml::String title;
    std::vector<CursedTempleEnterLineEntry> lines;
    Rml::String enterText;
    Rml::String closeText;
    // CButton::Render(): 23 / 2 - h / 2 whole units down, the native line height in physical px.
    float labelTop = 0.f;
    float labelLinePx = 0.f;
};
} // namespace mu::ui::window
