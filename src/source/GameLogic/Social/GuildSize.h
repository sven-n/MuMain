#pragma once

// How many members a guild master's guild can have: the original client's rule, which the guild window and the Dark
// Lord's command hint show. OpenMU does not check the guild size, so a server can allow more.
namespace GameLogic::Social
{
// Levels per guild member, for every class.
inline constexpr int LevelPerGuildMember = 10;
// A Dark Lord's command per guild member, and the most members a Dark Lord's guild can have.
inline constexpr int CommandPerGuildMember = 10;
inline constexpr int MaxDarkLordGuildMembers = 80;

// The members a guild of this guild master can have: level / 10, and for a Dark Lord plus command / 10, at most 80.
// `command` counts with what items and buffs add to it; other classes ignore it.
int GetMaxGuildMembers(bool darkLord, int level, int command);
} // namespace GameLogic::Social
