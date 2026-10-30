#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/MonsterData/MonsterDefinition.h"
#include "Data/GameData/MonsterData/MonsterModelDefinition.h"
#include "Data/GameData/MonsterData/MonsterSoundSet.h"

#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace Data::Monsters
{
// Checks the names in the sound sets, which the reader takes as they are;
// each problem is fatal: a sound id that is not one the list can name
// (MonsterSoundIds.h), and an event's sound that names no sound of the set
// and no such id. `source` names the file in the problems.
void ValidateMonsterSoundSetNames(std::span<const MonsterSoundSet> sets, const std::string& source,
                                  std::vector<Items::ItemDataIssue>& issues);

// The models the game draws a monster's look from at random, in code until
// MN4 (Q-MN83): the Chaos Castle elf and wizard of 163, 165 ... 427. Empty
// for a monster whose look is not drawn.
std::span<const std::string_view> GetDrawnLookModels(int number);

// Checks what the monster list, the model list and the sound sets name of
// each other (Q-MN78, Q-MN79, Q-MN83): errors, which the game runs with,
// for a monster or a random variant that names a set that is not there and
// for a random variant whose look the code never draws; warnings for an
// event's cue that the model of a monster that plays the set lacks (the
// event never plays for it) and for a set that no monster plays.
// `monstersSource` and `soundsSource` name the files in the problems.
void ValidateMonsterSoundLinks(std::span<const MonsterDefinition> monsters,
                               std::span<const MonsterModelDefinition> models, std::span<const MonsterSoundSet> sets,
                               const std::string& monstersSource, const std::string& soundsSource,
                               std::vector<Items::ItemDataIssue>& issues);
} // namespace Data::Monsters
