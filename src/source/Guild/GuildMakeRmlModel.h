#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// One cell of the guild mark editor (RenderGuildColor()): its palette colour, or the empty cell's
// black box with a grey cross.
// One cell of the mark editor: the colour a palette index stands for, or empty. A mark's pixels
// are the guild's own data; which cell of the grid or palette this is comes from its place in the
// list, and the theme lays both out (RenderEditGuildMark()'s 8 x 8 grid and 2 x 8 palette).
struct GuildMakeCellEntry
{
    Rml::String color; // "#rrggbbaa"
    bool empty = false;

    bool operator==(const GuildMakeCellEntry&) const = default;
};

struct GuildMakeRmlModel
{
    float textPx = 0.f; // native normal text size in physical px (RmlRootTransform.h)

    int page = 0; // CGuildMakeWindow::GUILDMAKE_STATE
    Rml::String titleText;
    float titlePx = 0.f;
    Rml::String infoText;
    float infoPx = 0.f;
    Rml::String makeText;
    Rml::String backText;
    Rml::String nextText;
    Rml::String nameText;   // "NAME" beside the name field
    Rml::String resultText; // "NAME : <guild name>" on the last page
    float resultPx = 0.f;
    Rml::String paletteHint1;
    Rml::String paletteHint2;
    Rml::String exitTooltip;
    // CButton::Render(): 29 / 2 - h / 2 whole units down, the native line height in physical px.
    float labelLinePx = 0.f;

    // The typed guild name. Two-way: the <input> writes it back through data-value, so this is
    // where the name lives -- not the element's value attribute.
    Rml::String guildName;

    std::vector<GuildMakeCellEntry> cells;   // the 8 x 8 editor, row by row
    std::vector<GuildMakeCellEntry> palette; // the 16 colours, two rows of eight
    GuildMakeCellEntry selected;             // the colour being drawn with
    std::vector<Rml::String> markCells;      // the finished mark (index 0 transparent)
};
} // namespace mu::ui::window
