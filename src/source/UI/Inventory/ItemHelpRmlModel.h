#pragma once

#include <RmlUi/Core/Types.h>

#include <vector>

namespace mu::ui::window
{
// A line of an item help box: its text, colour and weight, or a spacer ("half" for the original's
// "\n" line, "full" for its " " line).
struct ItemHelpLineEntry
{
    Rml::String text;
    Rml::String color; // "white", "blue", ... or a highlight: "hl-darkred", "hl-darkblue", ...
    bool bold = false;
    Rml::String spacer;

    bool operator==(const ItemHelpLineEntry&) const = default;
};

// A value of the item's levels table, and whether the hero can equip the item at its level.
struct ItemHelpCellEntry
{
    Rml::String text;
    bool canEquip = true;

    bool operator==(const ItemHelpCellEntry&) const = default;
};

// A column of the item's levels table: its heading and a value per level.
struct ItemHelpColumnEntry
{
    Rml::String heading;
    std::vector<ItemHelpCellEntry> cells;

    bool operator==(const ItemHelpColumnEntry&) const = default;
};

// An item help box (the /<item> and /<set> chat commands): lines, an optional levels table and
// lines under it. The theme lays it out.
struct ItemHelpRmlModel
{
    float textPx = 0.f; // the native text sizes, physical px
    float boldTextPx = 0.f;
    float lineHeight = 0.f; // the native line heights, reference px
    float boldLineHeight = 0.f;

    std::vector<ItemHelpLineEntry> lines;
    std::vector<ItemHelpColumnEntry> columns;
    std::vector<ItemHelpLineEntry> tailLines;
};
} // namespace mu::ui::window
