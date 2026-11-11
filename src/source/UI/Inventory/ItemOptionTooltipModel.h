#pragma once

#include <cstdint>

// Fixed-buffer tooltip model for CMyInventory's Set/Socket option hover text; built once by a
// BuildXxxTooltipModel() and consumed by either the legacy TextList path or RmlUi. Kept separate
// from UI::Skills::Tooltip's model since the color sets differ. Also included from GameLogic-side
// headers (CSItemOption.h, SocketSystem.h), not just UI code.
namespace UI::Inventory::Tooltip
{

enum class LineColor : uint8_t
{
    White,
    Blue,
    Yellow,
    Green,
    Purple,
};

constexpr int MAX_TOOLTIP_LINE_TEXT = 128;
constexpr int MAX_TOOLTIP_LINES = 64;

struct Line
{
    wchar_t text[MAX_TOOLTIP_LINE_TEXT];
    LineColor color;
    bool isBold;
};

// Fixed-capacity buffer. `count` lines are valid.
struct Model
{
    Line lines[MAX_TOOLTIP_LINES];
    int count;

    void Reset() { count = 0; }
};

}  // namespace UI::Inventory::Tooltip
