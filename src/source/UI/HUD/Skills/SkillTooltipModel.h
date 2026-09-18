#pragma once

#include <cstdint>

// Fixed-buffer model for skill hover tooltips, built by `BuildModel` and shared by the
// in-game renderer (legacy TextList) and the MuEditor renderer (ImGui). No heap allocations
// per render (CODING_RULES section 12).
//
// See also: SkillTooltip.{cpp,h} (in-game), MuEditor/UI/SkillEditor/SkillTooltipEditor.cpp (editor)

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

// Fixed-capacity buffer; `count` lines are valid. `skipCount` excludes lines (name, blanks)
// that don't participate in positioning math.
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

// Options driving BuildModel. Hero-context fields are only read when
// `includeCharacterSpecific` is true; the editor passes false and gets no character-conditional content.
struct BuildOptions
{
    int skillType;                // Resolved skill enum (e.g. AT_SKILL_FIRE_BALL)
    int skillSlotIndex;           // Hero's action-bar slot, or -1 in editor
    bool includeCharacterSpecific;
};

void BuildModel(const BuildOptions& options, Model& outModel);

}  // namespace UI::Skills::Tooltip
