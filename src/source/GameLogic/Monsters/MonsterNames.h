#pragma once

#include "Data/DataHandler/MonsterData/MonsterDataHandler.h"

class CHARACTER;

// The names of monsters and NPCs in the game: the monster list's name in the
// UI language, written into the character.
namespace GameLogic::Monsters::Names
{
// Writes the monster list's name of `number` into the character's name; a
// number without an entry keeps the name the character has.
void WriteShownName(CHARACTER& character, int number);

// Writes the names again into the living monsters and NPCs of this number, or
// of all numbers (MonsterDataHandler::AllNumbers): after a language switch
// or a name edit in the editor.
// Summons, transformed players and Kalima gates have names made of a monster
// and a player ("<monster> of <owner>"); they keep theirs until they appear
// again.
void RefreshShownNames(int number = Data::Monsters::MonsterDataHandler::AllNumbers);
} // namespace GameLogic::Monsters::Names
