#pragma once

#include "Core/Globals/_enum.h" // ActionSkillType

// What a critical hit adds to the hero's maximum damage, by where it comes from, added up the way the server adds up
// its critical damage bonus (OpenMU's CriticalDamageBonus): the active ancient set options, the worn items' harmony
// options and socket options, and the hero's own Increase Critical Damage while it is on. A critical hit deals the
// maximum damage plus this; an excellent hit deals the maximum damage times 1.2 plus the excellent damage bonus plus
// this.
namespace GameLogic::Combat
{
struct CriticalDamageBonus
{
    int setOptions = 0;
    int harmonyOptions = 0;
    int socketOptions = 0;
    // The hero's own Increase Critical Damage while it is on (the skill the hero has of it, for its name), and what it
    // adds: 1 per 30 energy and 1 per 25 command, plus its first power-up's value as the server sent it.
    ActionSkillType buffSkill = AT_SKILL_UNDEFINED;
    float buff = 0.f;

    // What the buff adds, in whole damage.
    int Buff() const;
    int Total() const
    {
        return setOptions + harmonyOptions + socketOptions + Buff();
    }

    bool operator==(const CriticalDamageBonus&) const = default;
};

// The bonus of the hero (CharacterMachine's equipment, CharacterAttribute's stats and skills, Hero's buffs), with the
// set options and socket options the client added up for the worn items.
CriticalDamageBonus GetHeroCriticalDamageBonus();

// What an excellent hit adds to the maximum damage times 1.2 (OpenMU's ExcellentDamageBonus): the active ancient set
// options and the worn items' socket options.
int GetHeroExcellentDamageBonus();

// What a critical hit deals before the target's defense: the maximum damage plus the critical damage bonus.
int GetCriticalHitDamage(int maximumDamage, const CriticalDamageBonus& criticalBonus);
// What an excellent hit deals before the target's defense: the maximum damage times 1.2, plus the excellent damage
// bonus and the critical damage bonus (OpenMU's AttackableExtensions, for physical damage).
int GetExcellentHitDamage(int maximumDamage, int excellentBonus, const CriticalDamageBonus& criticalBonus);
} // namespace GameLogic::Combat
