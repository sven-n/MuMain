#pragma once

// What the Golden Archer (NPC 236, the Rena event NPC, drawn as a player) says while it stands
// still: the lucky number lines in Devias, the stone collecting line in Lorencia. It is the only
// NPC that AnimationCharacter treats as such an event NPC.
namespace GameLogic::Npcs::PlayerNpcChat
{
// The line said in `world` when such an NPC stops: one picked at random from the world's lines
// when `actionChanged`, else the line said last there; nullptr in a world without lines.
const wchar_t* LineAtStop(int world, bool actionChanged);
} // namespace GameLogic::Npcs::PlayerNpcChat
