#pragma once

// What the Dark Lord's command adds: the attack rate and the Dark Raven's and Dark Horse's damage and attack speed by
// OpenMU's ClassDarkLord formulas, and the guild size by the original client's rule (GameLogic/Social/GuildSize.h;
// OpenMU does not check the guild size). Each part is the command divided by its divisor, rounded down, as the client
// always counted it.
namespace GameLogic::Character
{
// Command per point of the PvM attack rate.
inline constexpr int CommandPerAttackRate = 10;
// Command per point of the Dark Raven's minimum and maximum damage, and of its attack speed.
inline constexpr int CommandPerRavenMinimumDamage = 8;
inline constexpr int CommandPerRavenMaximumDamage = 4;
inline constexpr int CommandPerRavenAttackSpeed = 50;
// Command per point of the Dark Horse's minimum damage (its maximum is half again its minimum), and of its attack
// speed.
inline constexpr int CommandPerHorseDamage = 10;
inline constexpr int CommandPerHorseAttackSpeed = 50;

struct CommandEffects
{
    int attackRate = 0;
    int ravenMinimumDamage = 0;
    int ravenMaximumDamage = 0;
    int ravenAttackSpeed = 0;
    int horseDamage = 0;
    int horseAttackSpeed = 0;
    // The guild members the command adds to what the Dark Lord's level gives, by the client's rule.
    int guildMembers = 0;

    bool operator==(const CommandEffects&) const = default;
};

int GetCommandAttackRate(int command);
int GetCommandRavenMinimumDamage(int command);
int GetCommandRavenMaximumDamage(int command);
int GetCommandRavenAttackSpeed(int command);
int GetCommandHorseDamage(int command);
int GetCommandHorseAttackSpeed(int command);
// The guild members the command adds at this level: less near the cap of 80, none at it.
int GetCommandGuildMembers(int level, int command);

CommandEffects GetCommandEffects(int level, int command);
} // namespace GameLogic::Character
