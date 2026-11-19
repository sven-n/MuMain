#include "stdafx.h"

#include "UI/Character/CommandHint.h"

#include "I18N/All.h"
#include "UI/Character/HintLines.h"

#include <algorithm>
#include <cwchar>
#include <limits>
#include <string>
#include <type_traits>

namespace UI::Character
{
using UI::RmlBridge::Tooltip::Line;
using UI::RmlBridge::Tooltip::LineColor;

namespace
{
// Adds the line of a part only when the part adds something, like the damage and critical damage rate hints.
template <typename... Args> void AddPartLine(std::vector<Line>& lines, int value, const wchar_t* format, Args... args)
{
    if (value <= 0)
    {
        return;
    }

    // The localized formats use %d. The expanded format already includes the full item name.
    static_assert((std::is_same_v<Args, int> && ...));
    constexpr size_t MaxIntCharacters = std::numeric_limits<int>::digits10 + 2;
    std::wstring text(std::wcslen(format) + sizeof...(args) * MaxIntCharacters + 1, L'\0');
    const int length = std::swprintf(text.data(), text.size(), format, args...);
    if (length >= 0 && static_cast<size_t>(length) < text.size())
    {
        text.resize(static_cast<size_t>(length));
        lines.push_back(MakeHintLine(StringUtils::WideToNarrow(text.c_str())));
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
