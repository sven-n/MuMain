#include "stdafx.h"

#include "UI/Character/CommandHint.h"

#include "I18N/All.h"
#include "UI/Character/HintLines.h"

#include <algorithm>

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
    AddPartLine(lines, std::max(effects.raven.minimumDamage, effects.raven.maximumDamage),
                I18N::Game::DarkRavenDamageDD, effects.raven.minimumDamage, effects.raven.maximumDamage);
    AddPartLine(lines, effects.raven.attackSpeed, I18N::Game::DarkRavenAttackSpeedD, effects.raven.attackSpeed);
    AddPartLine(lines, std::max(effects.horse.minimumDamage, effects.horse.maximumDamage),
                I18N::Game::DarkHorseDamageDD, effects.horse.minimumDamage, effects.horse.maximumDamage);
    AddPartLine(lines, effects.horse.attackSpeed, I18N::Game::DarkHorseAttackSpeedD, effects.horse.attackSpeed);
    AddPartLine(lines, effects.guildMembers, I18N::Game::GuildMembersAsGuildMasterD, effects.guildMembers);
    // Only the title: the command adds nothing yet.
    if (lines.size() == 1)
    {
        lines.clear();
    }
    return lines;
}
} // namespace UI::Character
