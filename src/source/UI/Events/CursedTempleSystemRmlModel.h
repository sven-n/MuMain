#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// One image of the Illusion Temple HUD, in the original's drawing order: `src` relative to
// cursed_temple_system.rml, `rect` its texel cell, the box in reference px on the 640x480 screen and
// its colour (the buttons' transparency).
struct CursedTempleSpriteEntry
{
    float left = 0.f;
    float top = 0.f;
    float width = 0.f;
    float height = 0.f;
    Rml::String src;
    Rml::String rect;
    Rml::String color;

    bool operator==(const CursedTempleSpriteEntry&) const = default;
};

// One line of the tutorial step, left-aligned in its 300 px box in the normal font.
struct CursedTempleTextEntry
{
    float top = 0.f; // reference px
    Rml::String text;
    Rml::String color;
    float textPx = 0.f; // its native size, shrunk to the box like the original's

    bool operator==(const CursedTempleTextEntry&) const = default;
};

struct CursedTempleSystemRmlModel
{
    // The Hud layout's W/640 x H/480 stretch (UI::Scaling::GetActiveTransform() while CManager
    // runs the window) and its inverse for the counter-scaled text leaves.
    float scaleX = 1.f, scaleY = 1.f;
    float inverseScaleX = 1.f, inverseScaleY = 1.f;

    // The time, the mini map and the skill panel, then the score effect.
    std::vector<CursedTempleSpriteEntry> sprites;
    std::vector<CursedTempleTextEntry> tutorialLines;
};
} // namespace mu::ui::window
