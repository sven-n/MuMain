#pragma once

#ifdef _EDITOR

#include "Data/GameData/ItemData/ItemGroundLabels.h"

#include <optional>
#include <string_view>

namespace MuEditor::Items
{
// How the legacy theme's item tooltip draws a line of a TEXT_COLOR_* (Data/Interface/RmlUi/themes/legacy/tooltip.rcss,
// whose colors are those of the original RenderTipTextList): the line's class there, its text color, and the bar behind
// it for the four highlight colors (none for the others, which show the tooltip's black box).
struct TooltipLineLook
{
    std::string_view className;
    Data::Items::GroundLabelColor text{};
    std::optional<Data::Items::GroundLabelColor> background;
};

// The look of a line of TEXT_COLOR_* `color`; white's for a number that is none.
const TooltipLineLook& GetTooltipLineLook(int color);
} // namespace MuEditor::Items

#endif // _EDITOR
