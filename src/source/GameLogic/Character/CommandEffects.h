#pragma once

// What the Dark Lord's command adds: the attack rate and the Dark Raven's damage and attack speed by OpenMU's
// ClassDarkLord formulas, and the guild size by the original client's rule (OpenMU does not check the guild size).
// Each part is the command divided by its divisor, rounded down, as the client always counted it.
namespace GameLogic::Character
{
// Command per point of the PvM attack rate.
inline constexpr int CommandPerAttackRate = 10;
// Command per point of the Dark Raven's minimum and maximum damage, and of its attack speed.
inline constexpr int CommandPerRavenMinimumDamage = 8;
inline constexpr int CommandPerRavenMaximumDamage = 4;
inline constexpr int CommandPerRavenAttackSpeed = 50;
// Level and command per guild member a Dark Lord's guild can take, and the most members it can have: the original
// client's rule, which the guild window shows.
inline constexpr int LevelPerGuildMember = 10;
inline constexpr int CommandPerGuildMember = 10;
inline constexpr int MaxDarkLordGuildMembers = 80;

struct CommandEffects
{
    int attackRate = 0;
    int ravenMinimumDamage = 0;
    int ravenMaximumDamage = 0;
    int ravenAttackSpeed = 0;
    // The guild members a guild of this Dark Lord can have, level and command together, by the client's rule.
    int guildMembers = 0;

    bool operator==(const CommandEffects&) const = default;
};

int GetCommandAttackRate(int command);
int GetCommandRavenMinimumDamage(int command);
int GetCommandRavenMaximumDamage(int command);
int GetCommandRavenAttackSpeed(int command);
int GetDarkLordGuildMembers(int level, int command);

CommandEffects GetCommandEffects(int level, int command);
} // namespace GameLogic::Character
