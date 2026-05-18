#pragma once

#include <cstdint>

// Fixed-buffer model for skill hover tooltips. Built once per render call by
// `BuildModel`, then consumed by either the in-game renderer (which writes
// into the legacy TextList array) or the MuEditor renderer (which renders
// via ImGui).
//
// Centralized so the two renderers stay in sync on content order and
// special-case logic. Performance-conscious per CODING_RULES section 12:
// no heap allocations per render, fixed-size buffers throughout.
//
// See also:
//   - `SkillTooltip.{cpp,h}`              - in-game renderer
//   - `MuEditor/UI/SkillEditor/SkillTooltipEditor.cpp` - editor renderer

namespace UI::Skills::Tooltip
{

enum class LineColor : uint8_t
{
    White,
    Blue,
    Red,        // plain red text (unmet requirement lines, "(lacking N)" deficits)
    DarkRed,    // white text on a red background (warnings, siege badge, brand info)
};

constexpr int MAX_TOOLTIP_LINE_TEXT = 128;
constexpr int MAX_TOOLTIP_LINES = 50;

struct Line
{
    wchar_t text[MAX_TOOLTIP_LINE_TEXT];
    LineColor color;
    bool isBold;
    bool isBlank;   // emit a blank "\n" line for vertical spacing
};

// Fixed-capacity buffer. `count` lines are valid. `skipCount` mirrors the
// game tooltip's existing concept: certain lines (name, blanks) don't
// participate in positioning math, so the renderer counts them separately.
struct Model
{
    Line lines[MAX_TOOLTIP_LINES];
    int count;
    int skipCount;

    void Reset()
    {
        count = 0;
        skipCount = 0;
    }
};

// Options driving BuildModel. The hero-context fields are only read when
// `includeCharacterSpecific` is true (i.e. in-game). The editor passes
// `false` and the model omits all character-conditional content (class-
// specific damage calc, color-coded requirement comparisons, party teleport
// warnings, etc.).
struct BuildOptions
{
    int skillType;                // Resolved skill enum (e.g. AT_SKILL_FIRE_BALL)
    int skillSlotIndex;           // Hero's action-bar slot, or -1 in editor
    bool includeCharacterSpecific;
};

void BuildModel(const BuildOptions& options, Model& outModel);

}  // namespace UI::Skills::Tooltip
