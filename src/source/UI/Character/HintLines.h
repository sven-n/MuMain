#pragma once

#include "Core/Utilities/StringUtils.h"
#include "Engine/Object/ZzzInfomation.h"
#include "UI/RmlBridge/RmlTooltip.h"

#include <cwchar>
#include <limits>
#include <string>
#include <utility>

// Lines of the character window's hints.
namespace UI::Character
{
constexpr size_t HintArgumentLength(int)
{
    return std::numeric_limits<int>::digits10 + 2;
}

inline size_t HintArgumentLength(const wchar_t* text)
{
    return std::wcslen(text);
}

template <typename... Args> std::string FormatHintText(const wchar_t* format, Args... args)
{
    // The hint formats use decimal integers and wide strings. The format itself can include expanded item names.
    std::wstring text(std::wcslen(format) + (size_t{0} + ... + HintArgumentLength(args)) + 1, L'\0');
    const int length = std::swprintf(text.data(), text.size(), format, args...);
    if (length < 0 || static_cast<size_t>(length) >= text.size())
    {
        return {};
    }
    return StringUtils::WideToNarrow(text.c_str());
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
