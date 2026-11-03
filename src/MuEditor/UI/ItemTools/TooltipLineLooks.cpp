#include "stdafx.h"

#ifdef _EDITOR

#include "TooltipLineLooks.h"

#include <array>

namespace MuEditor::Items
{
namespace
{
// By TEXT_COLOR_* (Core/Globals/_define.h), from white (0) to orange (13).
const std::array<TooltipLineLook, 14> Looks = {{
    {"tt-line", {255, 255, 255}, std::nullopt},
    {"tt-blue", {128, 179, 255}, std::nullopt},
    {"tt-red", {255, 51, 26}, std::nullopt},
    {"tt-yellow", {255, 204, 26}, std::nullopt},
    {"tt-green", {26, 255, 128}, std::nullopt},
    {"tt-hl-darkred", {255, 255, 255}, Data::Items::GroundLabelColor{160, 0, 0}},
    {"tt-purple", {255, 26, 255}, std::nullopt},
    {"tt-hl-darkblue", {255, 255, 255}, Data::Items::GroundLabelColor{0, 0, 160}},
    {"tt-hl-darkyellow", {255, 255, 255}, Data::Items::GroundLabelColor{160, 102, 0}},
    {"tt-hl-greenblue", {0, 255, 0}, Data::Items::GroundLabelColor{60, 60, 200}},
    {"tt-gray", {102, 102, 102}, std::nullopt},
    {"tt-redpurple", {204, 128, 204}, std::nullopt},
    {"tt-violet", {179, 102, 255}, std::nullopt},
    {"tt-orange", {230, 107, 10}, std::nullopt},
}};
static_assert(TEXT_COLOR_WHITE == 0 && TEXT_COLOR_ORANGE == 13, "Looks is indexed by TEXT_COLOR_*");
} // namespace

const TooltipLineLook& GetTooltipLineLook(int color)
{
    return color >= 0 && color < static_cast<int>(Looks.size()) ? Looks[static_cast<size_t>(color)] : Looks[0];
}
} // namespace MuEditor::Items

#endif // _EDITOR
