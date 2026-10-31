#pragma once

#include "Data/GameData/MonsterData/MonsterSoundSet.h"

#include <vector>

// Which sound set each monster plays with each of its looks (monsters design,
// Q-MN78, Q-MN83), from the monster list (g_MonsterDatabase) and the sound
// sets (g_MonsterSoundSetDatabase). The model names are looked up once after
// either of them changes, not at each sound of a monster.
namespace Data::Monsters::SoundSetLinks
{
// The set the monster `number` plays when it appears with the model in
// `slot`, its entry's model or a random variant's (GetSoundSetWithModel);
// nullptr for another model, a number without an entry, or no set.
const MonsterSoundSet* FindSet(int number, int slot);

// The sets the monsters use with the model in `slot`, their own or a random
// variant's with that model, by monster number, each once.
std::vector<const MonsterSoundSet*> FindSetsOfSlot(int slot);

// How many monsters appear with the model in `slot`, with their entry's
// model or a random variant's: those that a change of its cues reaches.
int CountMonstersWithModel(int slot);
} // namespace Data::Monsters::SoundSetLinks
