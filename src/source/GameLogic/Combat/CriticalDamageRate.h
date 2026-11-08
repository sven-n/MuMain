#pragma once

#include "Core/Globals/_enum.h" // ActionSkillType

// The hero's critical damage rate in percent, by where it comes from, added up the way the server adds up its
// critical damage chance (OpenMU's CriticalDamageChance): the luck of the worn items, the active ancient set options,
// the socket options, the hero's own mastered buff and the Scroll of Battle while they are on. No stat changes it. A
// critical hit deals the maximum damage.
namespace GameLogic::Combat
{
struct CriticalDamageRate
{
    // The worn items whose luck counts, and their luck values added up (Data/Items/Options/Luck.json).
    int luckItems = 0;
    int luck = 0;
    // The active ancient set options "Increase critical damage rate".
    int setOptions = 0;
    // The socket options "Critical damage rate" of the worn items' seed spheres.
    int socketOptions = 0;
    // The master skill whose buff is on and adds to the rate, and what it adds: the master skill's value as the server
    // sent it, in percent.
    ActionSkillType buffSkill = AT_SKILL_UNDEFINED;
    float buff = 0.f;
    // The Scroll of Battle's part while its buff is on.
    int scrollOfBattle = 0;

    // What the worn items add.
    int Items() const
    {
        return luck + setOptions + socketOptions;
    }
    // What the master skill's buff adds, in whole percent.
    int MasterSkillBuff() const;
    // What the buffs add: the master skill's and the Scroll of Battle.
    int Buff() const
    {
        return MasterSkillBuff() + scrollOfBattle;
    }
    int Total() const
    {
        return Items() + Buff();
    }

    bool operator==(const CriticalDamageRate&) const = default;
};

// The rate of the hero (CharacterMachine's equipment, CharacterAttribute's master skills, Hero's buffs), with the
// set options and socket options the client added up for the worn items (CSItemOption::CheckItemSetOptions,
// CSocketItemMgr::CalcSocketStatusBonus).
CriticalDamageRate GetHeroCriticalDamageRate();
} // namespace GameLogic::Combat
