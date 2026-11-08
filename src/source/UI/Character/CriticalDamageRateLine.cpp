#include "stdafx.h"

#include "UI/Character/CriticalDamageRateLine.h"

#include "Core/Utilities/StringUtils.h"
#include "Engine/Object/ZzzInfomation.h"
#include "I18N/All.h"

namespace UI::Character
{
namespace
{
using UI::RmlBridge::Tooltip::Line;
using UI::RmlBridge::Tooltip::LineColor;

// Room for a formatted line in any language.
constexpr size_t MaxLineLength = 128;

template <typename... Args> std::string Format(const wchar_t* format, Args... args)
{
    wchar_t text[MaxLineLength] = {};
    mu_swprintf_s(text, format, args...);
    return StringUtils::WideToNarrow(text);
}

Line MakeLine(std::string text, LineColor color = LineColor::White, bool bold = false)
{
    Line line;
    line.text = std::move(text);
    line.color = color;
    line.bold = bold;
    return line;
}

// The buff's master skill by its name, or the title's words when the skill table has none.
const wchar_t* BuffName(ActionSkillType skill)
{
    if (SkillAttribute == nullptr || SkillAttribute[skill].Name[0] == L'\0')
    {
        return I18N::Game::CriticalDamageRate;
    }
    return SkillAttribute[skill].Name;
}
} // namespace

CriticalDamageRateLine BuildCriticalDamageRateLine(const GameLogic::Combat::CriticalDamageRate& rate)
{
    if (rate.Buff() > 0)
    {
        return {Format(I18N::Game::CriticalDamageRateDD, rate.Items(), rate.Buff()), "buffed"};
    }
    return {Format(I18N::Game::CriticalDamageRateD, rate.Items()), rate.Items() > 0 ? "boosted" : "base"};
}

std::vector<Line> BuildCriticalDamageRateHint(const GameLogic::Combat::CriticalDamageRate& rate)
{
    std::vector<Line> lines;
    lines.push_back(MakeLine(StringUtils::WideToNarrow(I18N::Game::CriticalDamageRate), LineColor::Yellow, true));
    if (rate.luck > 0)
    {
        lines.push_back(MakeLine(Format(I18N::Game::LuckDD, rate.luckItems, rate.luck)));
    }
    if (rate.setOptions > 0)
    {
        lines.push_back(MakeLine(Format(I18N::Game::SetOptionsD, rate.setOptions)));
    }
    if (rate.socketOptions > 0)
    {
        lines.push_back(MakeLine(Format(I18N::Game::SocketOptionsD, rate.socketOptions)));
    }
    if (rate.Buff() > 0)
    {
        lines.push_back(MakeLine(Format(I18N::Game::LsD, BuffName(rate.buffSkill), rate.Buff()), LineColor::Purple));
    }
    lines.push_back(MakeLine(Format(I18N::Game::TotalD, rate.Total()), LineColor::White, true));
    return lines;
}
} // namespace UI::Character
