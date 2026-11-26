#pragma once

#include "Core/Globals/_struct.h"
#include "Data/GameData/BuffData/BuffDefinition.h"
#include "Data/GameData/SkillData/MasterSkillDefinition.h"
#include "GameLogic/Buffs/BuffValues.h"

#include <array>
#include <cstddef>

// What a buff skill gives when the hero casts it (buffs design B9, B18, B21): its buff's values and duration with the
// hero as the caster, split into the parts the skill tooltip lists. Fixed sizes, no allocation: the tooltip asks every
// frame while it is open.
namespace GameLogic::Buffs
{
// The most power-ups, master skill parts per value and skills from a master skill down to its base the computation
// keeps; more are left out.
constexpr size_t MaxSkillBuffPowerUps = 8;
constexpr size_t MaxMasterParts = 4;
constexpr size_t MaxSkillChain = 8;

// A learned master skill's part of a value: its skill, its value as the server sends it and how it joins.
struct MasterPart
{
    int skill = 0;
    double value = 0;
    Data::Skills::MasterSkillHow how = Data::Skills::MasterSkillHow::Add;
};

// One value: what the constant and the terms reach (`reached`), stopped at the maximum (`base`), then the master
// skills' parts, the chained power-up and the total. The duration has no chain.
struct SkillBuffValue
{
    double reached = 0;
    double base = 0;
    bool stoppedAtMaximum = false;
    std::array<MasterPart, MaxMasterParts> masters{};
    size_t masterCount = 0;
    // The power-ups whose totals this one adds (valueOf, valueOfCount of them), and the one it is multiplied by
    // (multipliedBy, or Data::Buffs::NoPowerUp).
    std::array<int, Data::Buffs::MaxValueOf> valueOf{};
    size_t valueOfCount = 0;
    int multipliedBy = Data::Buffs::NoPowerUp;
    double total = 0;
};

struct SkillBuffValues
{
    const Data::Buffs::BuffDefinition* buff = nullptr;
    std::array<SkillBuffValue, MaxSkillBuffPowerUps> powerUps{};
    size_t powerUpCount = 0;
    bool hasDuration = false;
    SkillBuffValue duration;
};

// The buff the skill casts: the castsBuff of the skill or of a base skill in Data/Skills/MasterSkills.json (B20), else
// the skill data's (Data::Skills::FindSkillDataBuff); 0 for none.
int FindSkillBuff(int skill);

// The skill's buff with `caster`'s attributes (`stats`) and learned master skills
// (CHARACTER_ATTRIBUTE::MasterSkillInfo, counted from the skill down to its base skills, as OpenMU does). False when
// the skill casts no buff, or a buff without values from its caster.
bool ComputeSkillBuff(int skill, const CHARACTER_ATTRIBUTE& caster, const BuffCasterStats& stats,
                      SkillBuffValues& values);
} // namespace GameLogic::Buffs
