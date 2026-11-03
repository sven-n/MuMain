#pragma once

#ifdef _EDITOR

#include "Data/GameData/ItemData/ItemGroundLabels.h"

#include <array>
#include <cstddef>
#include <optional>
#include <string_view>

namespace MuEditor::Items
{
// How the item tooltip of a theme draws a line of a TEXT_COLOR_* (its tooltip.rcss): the line's class there, its text
// color, and the bar behind it for the four highlight colors (none for the others, which show the tooltip's box).
struct TooltipLineLook
{
    std::string_view className;
    Data::Items::GroundLabelColor text{255, 255, 255};
    std::optional<Data::Items::GroundLabelColor> background;
};

// The TEXT_COLOR_* the tooltip has a line class for, from white (0) to orange (13).
constexpr size_t TooltipLineLookCount = 14;

// The looks a tooltip style sheet gives the line classes, by TEXT_COLOR_*: the "color" and "background-color" of the
// rules ".tt-line" (white) and ".tt-line.<class>" ("#rrggbb", "rgb(...)" or "rgba(...)"), the later rule winning as in
// the theme. A class the sheet gives no color keeps white text, and no bar without a background.
std::array<TooltipLineLook, TooltipLineLookCount> ReadTooltipLineLooks(std::string_view styleSheet);

// The look of a line of TEXT_COLOR_* `color` in the active UI theme, read from its tooltip.rcss (or the one of the
// theme it inherits from) when the theme changes; white's for a number that is none.
const TooltipLineLook& GetTooltipLineLook(int color);
} // namespace MuEditor::Items

#endif // _EDITOR
