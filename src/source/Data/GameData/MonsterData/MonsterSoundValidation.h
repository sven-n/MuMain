#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/MonsterData/MonsterDefinition.h"
#include "Data/GameData/MonsterData/MonsterModelDefinition.h"
#include "Data/GameData/MonsterData/MonsterSoundSet.h"

#include <span>
#include <string>
#include <vector>

namespace Data::Monsters
{
// Checks the names in the sound sets, which the reader takes as they are.
// Fatal: a sound id that is not one the list can name (MonsterSoundIds.h),
// two sounds of a set with one name (its id, or its file when it has none),
// and an event's sound that names no sound of the set and no such id. An
// error: a SOUND_ id that sets load from different files. `source` names the
// file in the problems.
void ValidateMonsterSoundSetNames(std::span<const MonsterSoundSet> sets, const std::string& source,
                                  std::vector<Items::ItemDataIssue>& issues);

// Checks what the monster list, the model list and the sound sets name of
// each other (Q-MN78, Q-MN79, Q-MN83): errors, which the game runs with,
// for a monster or a random variant that names a set that is not there;
// warnings for an
// event's cue that the model of a monster that plays the set lacks (the
// event never plays for it), for a set with sound files on a monster whose
// model is not in the model list (no model loads them: the player model, a
// map object) and for a set that no monster plays.
// `monstersSource` and `soundsSource` name the files in the problems.
void ValidateMonsterSoundLinks(std::span<const MonsterDefinition> monsters,
                               std::span<const MonsterModelDefinition> models, std::span<const MonsterSoundSet> sets,
                               const std::string& monstersSource, const std::string& soundsSource,
                               std::vector<Items::ItemDataIssue>& issues);
// The same on the entries of the tables, which it does not copy.
void ValidateMonsterSoundLinks(std::span<const MonsterDefinition* const> monsters,
                               std::span<const MonsterModelDefinition* const> models,
                               std::span<const MonsterSoundSet> sets, const std::string& monstersSource,
                               const std::string& soundsSource, std::vector<Items::ItemDataIssue>& issues);
} // namespace Data::Monsters
