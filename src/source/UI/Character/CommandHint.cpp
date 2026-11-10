#include "stdafx.h"

#include "UI/Character/CommandHint.h"

#include "I18N/All.h"
#include "UI/Character/HintLines.h"

namespace UI::Character
{
using UI::RmlBridge::Tooltip::Line;
using UI::RmlBridge::Tooltip::LineColor;

std::vector<Line> BuildCommandHint(const GameLogic::Character::CommandEffects& effects)
{
    std::vector<Line> lines;
    lines.push_back(MakeHintLine(StringUtils::WideToNarrow(I18N::Game::Command), LineColor::Yellow, true));
    lines.push_back(MakeHintLine(FormatHintText(I18N::Game::AttackRatePvMD, effects.attackRate)));
    lines.push_back(MakeHintLine(
        FormatHintText(I18N::Game::DarkRavenDamageDD, effects.ravenMinimumDamage, effects.ravenMaximumDamage)));
    lines.push_back(MakeHintLine(FormatHintText(I18N::Game::DarkRavenAttackSpeedD, effects.ravenAttackSpeed)));
    lines.push_back(MakeHintLine(FormatHintText(I18N::Game::GuildMembersAsGuildMasterD, effects.guildMembers)));
    return lines;
}

bool CommandHintTexts::Update(const GameLogic::Character::CommandEffects& effects, const std::string& locale)
{
    if (m_BuiltEffects == effects && m_BuiltLocale == locale)
    {
        return false;
    }
    m_BuiltEffects = effects;
    m_BuiltLocale = locale;
    m_Lines = BuildCommandHint(effects);
    return true;
}
} // namespace UI::Character
