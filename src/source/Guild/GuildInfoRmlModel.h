#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// One RenderText() of the guild window: its box, alignment and colour, its size shrunk to the box
// like the original's.
struct GuildInfoTextEntry
{
    Rml::String text;
    float left = 0.f; // reference px in the panel
    float top = 0.f;
    float width = 0.f; // 0 = no box (left-aligned, never shrunk)
    float textPx = 0.f;
    int align = 0; // 0 left, 1 centred, 2 right edge at left + width
    bool bold = false;
    Rml::String color;
};

// A list line's RenderColor() box (the selected line, and the master / assistant / battle master
// lines of the member list): g_renderColor, white.
struct GuildInfoBoxEntry
{
    float left = 0.f;
    float top = 0.f;
    float width = 0.f;
    float height = 0.f;
};

// An alliance guild's 8 x 8 mark beside its line.
struct GuildInfoMarkEntry
{
    float left = 0.f;
    float top = 0.f;
    std::vector<Rml::String> cells; // 64 CSS colours, row by row
};

// A 64 x 29 newui_btn_empty_small button RmlUi reports with guild_info_button(index).
struct GuildInfoButtonEntry
{
    Rml::String label;
    int index = 0;
    float left = 0.f;
    float top = 0.f;
};

struct GuildInfoRmlModel
{
    // Right-docked window -- UI::Scaling::GetActiveTransform() while CManager runs it.
    float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
    float textPx = 0.f; // native normal text size in physical px (RmlRootTransform.h)

    bool noGuild = true;
    int tab = 1; // GuildConstants::GuildTab
    bool unionShown = false;
    float scrollTop = 0.f;              // the page's scroll track top, reference px
    float thumbTop = 0.f;               // the scroll thumb's top, reference px
    std::vector<Rml::String> markCells; // the hero's guild mark (Guild tab)

    std::vector<GuildInfoTextEntry> texts;
    std::vector<GuildInfoBoxEntry> boxes;
    std::vector<GuildInfoMarkEntry> marks;
    std::vector<GuildInfoButtonEntry> buttons;
    Rml::String exitTooltip;
    // CButton::Render(): 29 / 2 - h / 2 whole units down, the native line height in physical px.
    float labelTop = 0.f;
    float labelLinePx = 0.f;
};
} // namespace mu::ui::window
