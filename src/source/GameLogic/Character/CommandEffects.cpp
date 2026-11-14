#include "GameLogic/Character/CommandEffects.h"

#include <algorithm>

namespace GameLogic::Character
{
int GetCommandAttackRate(int command)
{
    return command / CommandPerAttackRate;
}

int GetCommandRavenMinimumDamage(int command)
{
    return command / CommandPerRavenMinimumDamage;
}

int GetCommandRavenMaximumDamage(int command)
{
    return command / CommandPerRavenMaximumDamage;
}

int GetCommandRavenAttackSpeed(int command)
{
    return command / CommandPerRavenAttackSpeed;
}

int GetDarkLordGuildMembers(int level, int command)
{
    return std::min(level / LevelPerGuildMember + command / CommandPerGuildMember, MaxDarkLordGuildMembers);
}

CommandEffects GetCommandEffects(int level, int command)
{
    CommandEffects effects;
    effects.attackRate = GetCommandAttackRate(command);
    effects.ravenMinimumDamage = GetCommandRavenMinimumDamage(command);
    effects.ravenMaximumDamage = GetCommandRavenMaximumDamage(command);
    effects.ravenAttackSpeed = GetCommandRavenAttackSpeed(command);
    effects.guildMembers = GetDarkLordGuildMembers(level, command);
    return effects;
}
} // namespace GameLogic::Character
