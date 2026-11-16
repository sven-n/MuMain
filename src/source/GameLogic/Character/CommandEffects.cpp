#include "stdafx.h"

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

int GetCommandHorseMinimumDamage(int command)
{
    return command / CommandPerHorseDamage;
}

int GetCommandHorseMaximumDamage(int command)
{
    const int minimum = GetCommandHorseMinimumDamage(command);
    return minimum + minimum / 2;
}

int GetCommandHorseAttackSpeed(int command)
{
    return command / CommandPerHorseAttackSpeed;
}

int GetCommandGuildMembers(int level, int command)
{
    return Social::GetMaxGuildMembers(CLASS_DARK_LORD, level, command) -
           Social::GetMaxGuildMembers(CLASS_DARK_LORD, level, 0);
}

CommandEffects GetCommandEffects(int level, int command)
{
    CommandEffects effects;
    effects.attackRate = GetCommandAttackRate(command);
    effects.ravenMinimumDamage = GetCommandRavenMinimumDamage(command);
    effects.ravenMaximumDamage = GetCommandRavenMaximumDamage(command);
    effects.ravenAttackSpeed = GetCommandRavenAttackSpeed(command);
    effects.horseMinimumDamage = GetCommandHorseMinimumDamage(command);
    effects.horseMaximumDamage = GetCommandHorseMaximumDamage(command);
    effects.horseAttackSpeed = GetCommandHorseAttackSpeed(command);
    effects.guildMembers = GetCommandGuildMembers(level, command);
    return effects;
}
} // namespace GameLogic::Character
