#pragma once

#ifdef _EDITOR

#include "Data/GameData/ItemData/ItemGroundLabels.h"

#include <array>
#include <cstddef>
#include <string_view>

namespace Rml
{
class ElementDocument;
}

namespace MuEditor::Items
{
// How the item tooltip of a theme draws a line of a TEXT_COLOR_*: the line's class in its document (tooltip.rml), its
// text color, and the bar behind it (alpha 0 for none, where the tooltip's box shows).
struct TooltipLineLook
{
    std::string_view className;
    Data::Items::GroundLabelColor text{255, 255, 255};
    Data::Items::GroundLabelColor background{0, 0, 0};
    BYTE backgroundAlpha = 0;
};

// The TEXT_COLOR_* the tooltip has a line class for, from white (0) to orange (13).
constexpr size_t TooltipLineLookCount = 14;

// How a theme's item tooltip looks: each line class's look, by TEXT_COLOR_*, and the box behind the lines.
struct TooltipLooks
{
    std::array<TooltipLineLook, TooltipLineLookCount> lines{};
    Data::Items::GroundLabelColor box{0, 0, 0};
    BYTE boxAlpha = 255;
};

// The looks the tooltip document gives its line classes, as RmlUi computes them from the theme's style sheets: a line
// of each class is put into its #tooltip_panel (the document itself without one) for as long as that takes, and the
// panel's background is the box.
TooltipLooks ComputeTooltipLooks(Rml::ElementDocument& document);

// The looks of the game's tooltip document in the active UI theme, computed again when the theme changes; white lines
// without bars on a black box without the RmlUi runtime.
const TooltipLooks& GetTooltipLooks();

// The look of a line of TEXT_COLOR_* `color`; white's for a number that is none.
const TooltipLineLook& GetTooltipLineLook(const TooltipLooks& looks, int color);
} // namespace MuEditor::Items

#endif // _EDITOR
