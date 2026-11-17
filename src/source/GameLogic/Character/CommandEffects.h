#pragma once

// What the Dark Lord's command adds: the attack rate and the Dark Raven's damage and attack speed by OpenMU's
// ClassDarkLord formulas; the Dark Horse's damage and attack speed and the guild size by the original client's rules,
// which its pet and guild windows show (OpenMU has neither: its command reaches the horse through Earthshake's damage,
// and it does not check the guild size). Each part is the command divided by its divisor, rounded down, as the client
// always counted it.
namespace GameLogic::Character
{
// Command per point of the PvM attack rate.
inline constexpr int CommandPerAttackRate = 10;
// Command per point of the Dark Raven's minimum and maximum damage, and of its attack speed.
inline constexpr int CommandPerRavenMinimumDamage = 8;
inline constexpr int CommandPerRavenMaximumDamage = 4;
inline constexpr int CommandPerRavenAttackSpeed = 50;
// The client's rule (CalcPetInfo): the Dark Horse's minimum damage is strength / 10 plus command / 10 plus 5 per horse
// level, its maximum half again its minimum (rounded down), and its attack speed gets command / 50.
inline constexpr int CommandPerHorseDamage = 10;
inline constexpr int StrengthPerHorseDamage = 10;
inline constexpr int HorseDamagePerLevel = 5;
inline constexpr int CommandPerHorseAttackSpeed = 50;

// What the Dark Horse's damage counts besides the command: the hero's strength (with what items and buffs add) and
// the horse's level (0 without a horse).
struct HorseDamageBase
{
    int strength = 0;
    int horseLevel = 0;
};

struct CommandEffects
{
    int attackRate = 0;
    int ravenMinimumDamage = 0;
    int ravenMaximumDamage = 0;
    int ravenAttackSpeed = 0;
    int horseMinimumDamage = 0;
    int horseMaximumDamage = 0;
    int horseAttackSpeed = 0;
    // The guild members the command adds to what the Dark Lord's level gives, by the client's rule.
    int guildMembers = 0;

    bool operator==(const CommandEffects&) const = default;
};

int GetCommandAttackRate(int command);
int GetCommandRavenMinimumDamage(int command);
int GetCommandRavenMaximumDamage(int command);
int GetCommandRavenAttackSpeed(int command);
int GetHorseMinimumDamage(const HorseDamageBase& base, int command);
int GetHorseMaximumDamage(int minimumDamage);
int GetCommandHorseMinimumDamage(int command);
// What the command adds to the maximum: the maximum with the command less the maximum without it, since the maximum
// halves the whole minimum.
int GetCommandHorseMaximumDamage(const HorseDamageBase& base, int command);
int GetCommandHorseAttackSpeed(int command);
// The guild members the command adds at this level: less near the cap of 80, none at it.
int GetCommandGuildMembers(int level, int command);

CommandEffects GetCommandEffects(int level, int command, const HorseDamageBase& horse);
} // namespace GameLogic::Character
