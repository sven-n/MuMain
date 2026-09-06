#pragma once

#include <cstdint>

// Fixed-buffer model for CMyInventory's Set/Socket option hover tooltips. Same shape and rationale
// as UI::Skills::Tooltip's own model (SkillTooltipModel.h) -- a renderer-agnostic Model built once
// by a BuildXxxTooltipModel() function with no drawing, then consumed either by the legacy
// TextList/RenderTipTextList path or bound into RmlUi (CMyInventory::SyncRmlModel()). Kept as its
// own type rather than reusing UI::Skills::Tooltip's: the color set differs (Yellow/Green/Purple
// here, no skill-tooltip Red/DarkRed), and the two are otherwise unrelated features.
//
// This header is included from GameLogic/Items/CSItemOption.h and Network/Server/SocketSystem.h
// (both GameLogic-side) -- the same cross-layer precedent GIPetManager.h already sets by including
// SkillTooltipModel.h.
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
