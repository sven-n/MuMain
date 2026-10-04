#include "stdafx.h"

#include "UI/Tooltip/LegacyTextListTooltip.h"

#include "Core/Utilities/StringUtils.h"
#include "Engine/Object/ZzzInventory.h"
#include "UI/RmlBridge/RmlTooltip.h"

#include <utility>
#include <vector>

namespace UI::Tooltip
{
namespace
{
UI::RmlBridge::Tooltip::LineColor ConvertColor(int color)
{
    using LineColor = UI::RmlBridge::Tooltip::LineColor;
    switch (color)
    {
    case TEXT_COLOR_BLUE: return LineColor::Blue;
    case TEXT_COLOR_GRAY: return LineColor::Gray;
    case TEXT_COLOR_RED: return LineColor::Red;
    case TEXT_COLOR_YELLOW: return LineColor::Yellow;
    case TEXT_COLOR_GREEN: return LineColor::Green;
    case TEXT_COLOR_PURPLE: return LineColor::Purple;
    case TEXT_COLOR_REDPURPLE: return LineColor::RedPurple;
    case TEXT_COLOR_VIOLET: return LineColor::Violet;
    case TEXT_COLOR_ORANGE: return LineColor::Orange;
    case TEXT_COLOR_DARKRED: return LineColor::DarkRedHighlight;
    case TEXT_COLOR_DARKBLUE: return LineColor::DarkBlueHighlight;
    case TEXT_COLOR_DARKYELLOW: return LineColor::DarkYellowHighlight;
    case TEXT_COLOR_GREEN_BLUE: return LineColor::GreenBlueHighlight;
    case TEXT_COLOR_WHITE: default: return LineColor::White;
    }
}

std::vector<UI::RmlBridge::Tooltip::Line> BuildLines(int lineCount)
{
    using Line = UI::RmlBridge::Tooltip::Line;
    std::vector<Line> lines;
    lines.reserve(static_cast<size_t>(lineCount));

    for (int i = 0; i < lineCount; ++i)
    {
        if (TextList[i][0] == L'\0')
            break;

        Line line;
        if (TextList[i][0] == L'\n')
            line.kind = Line::Kind::HalfSpacer;
        else if (TextList[i][0] == L' ' && TextList[i][1] == L'\0')
            line.kind = Line::Kind::FullSpacer;
        else
        {
            line.text = StringUtils::WideToNarrow(TextList[i]);
            line.bold = TextBold[i] != 0;
            line.color = ConvertColor(TextListColor[i]);
        }
        lines.push_back(std::move(line));
    }
    return lines;
}
} // namespace

void ShowLegacyTextList(int lineCount, float anchorX, float anchorY, Placement placement, const void* owner)
{
    UI::RmlBridge::Tooltip::Config config;
    config.lines = BuildLines(lineCount);
    config.anchorX = anchorX;
    config.anchorY = anchorY;
    config.centerHorizontally = true;
    config.anchor = placement == Placement::Above ? UI::RmlBridge::Tooltip::AnchorPoint::AboveLeft
                                                   : UI::RmlBridge::Tooltip::AnchorPoint::BelowLeft;
    config.textAlign = UI::RmlBridge::Tooltip::Config::TextAlign::Center;
    UI::RmlBridge::Tooltip::Show(config, owner);
}

void HideLegacyTextList(const void* owner)
{
    UI::RmlBridge::Tooltip::Hide(owner);
}
} // namespace UI::Tooltip
