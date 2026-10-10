#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// One marker of the Illusion Temple HUD's mini map, which the window projects itself: `src` is
// relative to cursed_temple_system.rml, `rect` its texel cell, the box reference px from the
// projection's origin (#mini_map_markers). The HUD's own frames and buttons are the theme's.
struct CursedTempleSpriteEntry
{
    float left = 0.f;
    float top = 0.f;
    float width = 0.f;
    float height = 0.f;
    Rml::String src;
    Rml::String rect;
    float opacity = 1.f;

    bool operator==(const CursedTempleSpriteEntry&) const = default;
};

// One line of the tutorial step, left-aligned in its 300 px box in the normal font. The step's
// first line is its title, which the theme colours apart.
struct CursedTempleTextEntry
{
    Rml::String text;
    float textPx = 0.f; // its native size, shrunk to the box like the original's
    bool title = false;

    bool operator==(const CursedTempleTextEntry&) const = default;
};

struct CursedTempleSystemRmlModel
{

    // The time, the minimap and the skill panel are hidden together when every panel the original
    // checked is open; the score effect shows for a while after a team scores.
    bool panelsShown = false;
    bool scoreShown = false;
    // The panels' buttons' opacity: the transparency the player toggled.
    float alpha = 1.f;

    // The current skill's icon: greyed until the hero has the kill points for it.
    Rml::String skillIconSrc;
    Rml::String skillIconRect;

    // The score effect's digits. A team past nine points gets a tens digit and its ones digit
    // moves out of the centre.
    Rml::String alliedTensSrc, alliedOnesSrc;
    Rml::String illusionTensSrc, illusionOnesSrc;
    bool alliedTwoDigits = false;
    bool illusionTwoDigits = false;

    // The digit runs' cells (UI::RmlBridge::DigitCells): the time (newui_number1), the mini map's
    // transparency and the teams' points (FontTest), and the skill's kill points needed and held
    // (newui_number1).
    std::vector<Rml::String> minuteDigits, secondDigits;
    std::vector<Rml::String> alphaDigits, alliedDigits, illusionDigits;
    std::vector<Rml::String> killsNeededDigits, killsDigits;

    std::vector<CursedTempleSpriteEntry> sprites;
    std::vector<CursedTempleTextEntry> tutorialLines;
};
} // namespace mu::ui::window
