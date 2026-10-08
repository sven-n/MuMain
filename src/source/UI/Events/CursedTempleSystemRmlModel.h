#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// One image of the Illusion Temple HUD that the window itself has to place: the minimap's
// projected markers, the digit runs (packed by how many digits the value has) and the three
// native buttons, whose box comes off the live CButton that hit-tests them. `src` is relative to
// cursed_temple_system.rml, `rect` its texel cell, the box reference px on the 640x480 screen, and
// `opacity` the transparency the player toggled. The HUD's own frames are the theme's.
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
    // The HUD board (LayoutMode::HudBoard while CManager runs the window): the 640x480 frame's
    // offset in physical px and its one scale, cancelled by the counter-scaled text leaves.
    float rootX = 0.f, rootY = 0.f, rootScale = 1.f;

    // The time, the minimap and the skill panel are hidden together when every panel the original
    // checked is open; the score effect shows for a while after a team scores.
    bool panelsShown = false;
    bool scoreShown = false;

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
