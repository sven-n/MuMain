#pragma once

#include "Core/Utilities/StringUtils.h"
#include "UI/RmlBridge/RmlTooltip.h"

#include <string>
#include <utility>

// Lines of the character window's hints.
namespace UI::Character
{
// Room for a formatted hint line in any language.
constexpr size_t MaxHintLineLength = 128;

template <typename... Args> std::string FormatHintText(const wchar_t* format, Args... args)
{
    wchar_t text[MaxHintLineLength] = {};
    mu_swprintf_s(text, format, args...);
    return StringUtils::WideToNarrow(text);
}

inline UI::RmlBridge::Tooltip::Line
MakeHintLine(std::string text, UI::RmlBridge::Tooltip::LineColor color = UI::RmlBridge::Tooltip::LineColor::White,
             bool bold = false)
{
    UI::RmlBridge::Tooltip::Line line;
    line.text = std::move(text);
    line.color = color;
    line.bold = bold;
    return line;
}
} // namespace UI::Character
