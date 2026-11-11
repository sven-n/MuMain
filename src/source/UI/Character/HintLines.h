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

// The skill table's name of the skill, empty when there is none; no copy.
inline const wchar_t* TableSkillName(ActionSkillType skill)
{
    return SkillAttribute != nullptr ? SkillAttribute[skill].Name : L"";
}

// The item table's name of the item, empty when there is none; no copy.
inline const wchar_t* TableItemName(int itemType)
{
    return ItemAttribute != nullptr ? ItemAttribute[itemType].Name : L"";
}

// The skill's name, or its number ("#383") when the skill table has none, so two missing names stay apart.
inline std::wstring SkillName(ActionSkillType skill)
{
    if (const wchar_t* name = TableSkillName(skill); name[0] != L'\0')
    {
        return name;
    }
    return L"#" + std::to_wstring(static_cast<int>(skill));
}

// The item's name, or its group and number ("14/97") when the item table has none.
inline std::wstring ItemName(int itemType)
{
    if (const wchar_t* name = TableItemName(itemType); name[0] != L'\0')
    {
        return name;
    }
    return std::to_wstring(itemType / MAX_ITEM_INDEX) + L"/" + std::to_wstring(itemType % MAX_ITEM_INDEX);
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
