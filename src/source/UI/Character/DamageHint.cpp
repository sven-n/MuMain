#include "stdafx.h"

#include "UI/Character/DamageHint.h"

#include "Engine/Object/ZzzInfomation.h"
#include "I18N/All.h"
#include "UI/Character/HintLines.h"

namespace UI::Character
{
namespace
{
using UI::RmlBridge::Tooltip::Line;
using UI::RmlBridge::Tooltip::LineColor;

void AddCriticalDamageParts(std::vector<Line>& lines, const GameLogic::Combat::CriticalDamageBonus& bonus)
{
    Line spacer;
    spacer.kind = Line::Kind::HalfSpacer;
    lines.push_back(spacer);
    lines.push_back(MakeHintLine(StringUtils::WideToNarrow(I18N::Game::CriticalDamage), LineColor::Yellow, true));
    if (bonus.setOptions > 0)
    {
        lines.push_back(MakeHintLine(FormatHintText(I18N::Game::CriticalDamageFromSetOptionsD, bonus.setOptions)));
    }
    if (bonus.harmonyOptions > 0)
    {
        lines.push_back(
            MakeHintLine(FormatHintText(I18N::Game::CriticalDamageFromHarmonyOptionsD, bonus.harmonyOptions)));
    }
    if (bonus.socketOptions > 0)
    {
        lines.push_back(
            MakeHintLine(FormatHintText(I18N::Game::CriticalDamageFromSocketOptionsD, bonus.socketOptions)));
    }
    if (bonus.Buff() > 0)
    {
        lines.push_back(MakeHintLine(
            FormatHintText(I18N::Game::CriticalDamageFromABuffLsD, SkillName(bonus.buffSkill).c_str(), bonus.Buff()),
            LineColor::Purple));
    }
    lines.push_back(
        MakeHintLine(FormatHintText(I18N::Game::CriticalDamageTotalD, bonus.Total()), LineColor::White, true));
}
} // namespace

std::vector<Line> BuildDamageHint(DamageRange range, const GameLogic::Combat::CriticalDamageBonus& criticalBonus,
                                  int excellentBonus)
{
    const int criticalHit = GameLogic::Combat::GetCriticalHitDamage(range.maximum, criticalBonus);
    const int excellentHit = GameLogic::Combat::GetExcellentHitDamage(range.maximum, excellentBonus, criticalBonus);
    // The hits that the buff makes stronger take its colour.
    const LineColor buffed = criticalBonus.Buff() > 0 ? LineColor::Purple : LineColor::White;

    std::vector<Line> lines;
    lines.push_back(MakeHintLine(StringUtils::WideToNarrow(I18N::Game::Damage), LineColor::Yellow, true));
    lines.push_back(MakeHintLine(FormatHintText(I18N::Game::NormalHitDD, range.minimum, range.maximum)));
    lines.push_back(MakeHintLine(FormatHintText(I18N::Game::CriticalHitD, criticalHit), buffed));
    lines.push_back(MakeHintLine(FormatHintText(I18N::Game::ExcellentHitD, excellentHit), buffed));
    lines.push_back(MakeHintLine(StringUtils::WideToNarrow(I18N::Game::BeforeTheTargetSDefense), LineColor::Gray));
    if (criticalBonus.Total() > 0)
    {
        AddCriticalDamageParts(lines, criticalBonus);
    }
    return lines;
}
} // namespace UI::Character
