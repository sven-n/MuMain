#pragma once

#include "Core/Platform/WinCompat.h"
#include "Core/Globals/_enum.h"

// The guild window and Command hint use the original client's guild limit formula and include Command bonuses from
// items and buffs. OpenMU does not enforce this limit.
namespace GameLogic::Social
{
// Levels per guild member, for every class.
inline constexpr int LevelPerGuildMember = 10;
// A Dark Lord's command per guild member, and the most members a Dark Lord's guild can have.
inline constexpr int CommandPerGuildMember = 10;
inline constexpr int MaxDarkLordGuildMembers = 80;

// The members a guild of this guild master can have: level / 10, and for a Dark Lord plus command / 10, at most 80.
// `baseClass` is the guild master's base class; `command` counts with what items and buffs add to it, and only a Dark
// Lord's counts.
int GetMaxGuildMembers(CLASS_TYPE baseClass, int level, int command);
} // namespace GameLogic::Social
