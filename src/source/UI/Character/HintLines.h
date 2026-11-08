#pragma once

#include "Core/Utilities/StringUtils.h"
#include "Engine/Object/ZzzInfomation.h"
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

// The skill's name, or `fallback` when the skill table has none.
inline const wchar_t* SkillNameOr(ActionSkillType skill, const wchar_t* fallback)
{
    if (SkillAttribute == nullptr || SkillAttribute[skill].Name[0] == L'\0')
    {
        return fallback;
    }
    return SkillAttribute[skill].Name;
}

// The item's name, or `fallback` when the item table has none.
inline const wchar_t* ItemNameOr(int itemType, const wchar_t* fallback)
{
    if (ItemAttribute == nullptr || ItemAttribute[itemType].Name[0] == L'\0')
    {
        return fallback;
    }
    return ItemAttribute[itemType].Name;
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
