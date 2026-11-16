#include "GameLogic/Character/CommandEffects.h"

#include "GameLogic/Social/GuildSize.h"

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

int GetCommandHorseDamage(int command)
{
    return command / CommandPerHorseDamage;
}

int GetCommandHorseAttackSpeed(int command)
{
    return command / CommandPerHorseAttackSpeed;
}

int GetCommandGuildMembers(int level, int command)
{
    constexpr bool DarkLord = true;
    return Social::GetMaxGuildMembers(DarkLord, level, command) - Social::GetMaxGuildMembers(DarkLord, level, 0);
}

CommandEffects GetCommandEffects(int level, int command)
{
    CommandEffects effects;
    effects.attackRate = GetCommandAttackRate(command);
    effects.ravenMinimumDamage = GetCommandRavenMinimumDamage(command);
    effects.ravenMaximumDamage = GetCommandRavenMaximumDamage(command);
    effects.ravenAttackSpeed = GetCommandRavenAttackSpeed(command);
    effects.horseDamage = GetCommandHorseDamage(command);
    effects.horseAttackSpeed = GetCommandHorseAttackSpeed(command);
    effects.guildMembers = GetCommandGuildMembers(level, command);
    return effects;
}
} // namespace GameLogic::Character
