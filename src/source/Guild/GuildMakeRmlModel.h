#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// One cell of the guild mark editor (RenderGuildColor()): its palette colour, or the empty cell's
// black box with a grey cross.
struct GuildMakeCellEntry
{
    Rml::String color; // "#rrggbbaa"
    bool empty = false;
    float left = 0.f; // the cell's box (colour inset 1), reference px in the panel
    float top = 0.f;
};

struct GuildMakeRmlModel
{
    // Right-docked window -- UI::Scaling::GetActiveTransform() while CManager runs it.
    float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
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
    float labelTop = 0.f;
    float labelLinePx = 0.f;

    std::vector<GuildMakeCellEntry> cells;   // the 8 x 8 editor, row by row
    std::vector<GuildMakeCellEntry> palette; // the 16 colours, two rows of eight
    GuildMakeCellEntry selected;             // the colour being drawn with
    std::vector<Rml::String> markCells;      // the finished mark (index 0 transparent)
};
} // namespace mu::ui::window
