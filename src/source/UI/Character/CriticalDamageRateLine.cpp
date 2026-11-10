#include "stdafx.h"

#include "UI/Character/CriticalDamageRateLine.h"

#include "Engine/Object/ZzzInfomation.h"
#include "I18N/All.h"
#include "UI/Character/HintLines.h"

namespace UI::Character
{
namespace
{
using UI::RmlBridge::Tooltip::Line;
using UI::RmlBridge::Tooltip::LineColor;
} // namespace

CriticalDamageRateLine BuildCriticalDamageRateLine(const GameLogic::Combat::CriticalDamageRate& rate)
{
    if (rate.Buff() > 0)
    {
        return {FormatHintText(I18N::Game::CriticalDamageRateDD, rate.Items(), rate.Buff()), "buffed"};
    }
    return {FormatHintText(I18N::Game::CriticalDamageRateD, rate.Items()), rate.Items() > 0 ? "boosted" : "base"};
}

std::vector<Line> BuildCriticalDamageRateHint(const GameLogic::Combat::CriticalDamageRate& rate)
{
    std::vector<Line> lines;
    lines.push_back(MakeHintLine(StringUtils::WideToNarrow(I18N::Game::CriticalDamageRate), LineColor::Yellow, true));
    if (rate.luck > 0)
    {
        lines.push_back(MakeHintLine(FormatHintText(I18N::Game::LuckDD, rate.luckItems, rate.luck)));
    }
    if (rate.setOptions > 0)
    {
        lines.push_back(MakeHintLine(FormatHintText(I18N::Game::SetOptionsD, rate.setOptions)));
    }
    if (rate.socketOptions > 0)
    {
        lines.push_back(MakeHintLine(FormatHintText(I18N::Game::SocketOptionsD, rate.socketOptions)));
    }
    if (rate.MasterSkillBuff() > 0)
    {
        lines.push_back(
            MakeHintLine(FormatHintText(I18N::Game::LsD, SkillName(rate.buffSkill).c_str(), rate.MasterSkillBuff()),
                         LineColor::Purple));
    }
    if (rate.scrollOfBattle > 0)
    {
        const std::wstring scroll = ItemName(ITEM_SCROLL_OF_BATTLE);
        lines.push_back(
            MakeHintLine(FormatHintText(I18N::Game::LsD, scroll.c_str(), rate.scrollOfBattle), LineColor::Purple));
    }
    lines.push_back(MakeHintLine(FormatHintText(I18N::Game::TotalD, rate.Total()), LineColor::White, true));
    return lines;
}
} // namespace UI::Character
