#pragma once

#include "Data/GameData/BuffData/BuffAttributes.h"
#include "Data/GameData/BuffData/BuffDefinition.h"

#include <cstddef>

class Buff;

// The values and durations of the buffs (Data/Buffs/Buffs.json, buffs design B13) as the hero has them now: the
// character stats, the buff strip and the tooltips of the items that give a buff read them here.
namespace GameLogic::Buffs
{
// The value of the buff's power-up at `index` with its terms on the hero's attributes (the auras' per level); 0 when
// the buff has no power-up there.
int GetPowerUpValue(const Data::Buffs::BuffDefinition& buff, size_t index);

// How long the buff lasts in seconds when the server sends no time; 0 when only the server knows it.
int GetDuration(const Data::Buffs::BuffDefinition& buff);

// The sum of the power-ups of `attribute` over the buffs in `buffs` (a character's m_BuffMap), each buff once. A buff
// without an entry in the table adds nothing, and neither does a debuff.
int SumPowerUps(const Buff& buffs, Data::Buffs::BuffAttributeName attribute);

// The same over the hero's buffs; 0 without a hero. The character stats add it to the stat the attribute names.
int GetHeroPowerUpTotal(Data::Buffs::BuffAttributeName attribute);
} // namespace GameLogic::Buffs
