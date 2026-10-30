#pragma once

// What the event NPCs that look like players (the lucky number registrar in Devias, the stone
// collector in Lorencia) say while they stand still.
namespace GameLogic::Npcs::PlayerNpcChat
{
// The line said in `world` when such an NPC stops: one picked at random from the world's lines
// when `actionChanged`, else the line said last there; nullptr in a world without lines.
const wchar_t* LineAtStop(int world, bool actionChanged);
} // namespace GameLogic::Npcs::PlayerNpcChat
