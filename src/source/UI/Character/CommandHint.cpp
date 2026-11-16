#include "stdafx.h"

#include "UI/Character/CommandHint.h"

#include "I18N/All.h"
#include "UI/Character/HintLines.h"

namespace UI::Character
{
using UI::RmlBridge::Tooltip::Line;
using UI::RmlBridge::Tooltip::LineColor;

namespace
{
// Adds the line of a part only when the part adds something, like the damage and critical damage rate hints.
template <typename... Args> void AddPartLine(std::vector<Line>& lines, int value, const wchar_t* format, Args... args)
{
    if (value > 0)
    {
        lines.push_back(MakeHintLine(FormatHintText(format, args...)));
    }
}
} // namespace

std::vector<Line> BuildCommandHint(const GameLogic::Character::CommandEffects& effects)
{
    std::vector<Line> lines;
    lines.push_back(MakeHintLine(StringUtils::WideToNarrow(I18N::Game::Command), LineColor::Yellow, true));
    AddPartLine(lines, effects.attackRate, I18N::Game::AttackRatePvMD, effects.attackRate);
    AddPartLine(lines, effects.ravenMaximumDamage, I18N::Game::DarkRavenDamageDD, effects.ravenMinimumDamage,
                effects.ravenMaximumDamage);
    AddPartLine(lines, effects.ravenAttackSpeed, I18N::Game::DarkRavenAttackSpeedD, effects.ravenAttackSpeed);
    AddPartLine(lines, effects.horseMaximumDamage, I18N::Game::DarkHorseDamageDD, effects.horseMinimumDamage,
                effects.horseMaximumDamage);
    AddPartLine(lines, effects.horseAttackSpeed, I18N::Game::DarkHorseAttackSpeedD, effects.horseAttackSpeed);
    AddPartLine(lines, effects.guildMembers, I18N::Game::GuildMembersAsGuildMasterD, effects.guildMembers);
    // Only the title: the command adds nothing yet.
    if (lines.size() == 1)
    {
        lines.clear();
    }
    return lines;
}
} // namespace UI::Character
