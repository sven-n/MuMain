#pragma once

#include "Data/GameData/BuffData/BuffAttributes.h"
#include "Data/GameData/BuffData/BuffDefinition.h"

#include <array>
#include <cstddef>

class Buff;

// The values and durations of the buffs (Data/Buffs/Buffs.json, buffs design B13, B17) as the hero has them now: the
// character stats, the buff strip and the tooltips of the items that give a buff read them here.
namespace GameLogic::Buffs
{
// The attributes a buff value's terms read, by Data::Buffs::BuffTermAttribute: the caster's for a caster's buff, the
// owner's for an owner's.
using BuffCasterStats = std::array<double, Data::Buffs::BuffTermAttributeCount>;

// The hero's: total stats, levels, maximum mana and the party members the client sees (buffs design B19).
BuffCasterStats GetHeroCasterStats();

// What one term adds: the attribute times `times`, or divided by `divisor`.
double ComputeTerm(const Data::Buffs::BuffTerm& term, const BuffCasterStats& stats);

// The constant and the terms, stopped at the value's maximum.
double ComputeValue(const Data::Buffs::BuffValue& value, const BuffCasterStats& stats);

// A computed number as the game shows and counts it: cut to a whole number toward 0, as OpenMU casts it.
int CutToWhole(double value);

// The value of the buff's power-up at `index` with the hero's attributes (the auras' per level), cut to a whole
// number; 0 when the buff has no power-up there.
int GetPowerUpValue(const Data::Buffs::BuffDefinition& buff, size_t index);

// How long the buff lasts in seconds when the server sends no time; 0 when only the server knows it.
int GetDuration(const Data::Buffs::BuffDefinition& buff);

// The sum of the power-ups of `attribute` over the buffs in `buffs` (a character's m_BuffMap), each buff once. A buff
// without an entry in the table adds nothing, and neither does a debuff or a caster's buff: only the server knows
// the caster's attributes (buffs design B16).
int SumPowerUps(const Buff& buffs, Data::Buffs::BuffAttributeName attribute);

// The same over the hero's buffs; 0 without a hero. The character stats add it to the stat the attribute names.
int GetHeroPowerUpTotal(Data::Buffs::BuffAttributeName attribute);
} // namespace GameLogic::Buffs
