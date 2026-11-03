#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/MonsterData/MonsterModelDefinition.h"

#include <span>
#include <string>
#include <vector>

namespace Data::Monsters
{
// Checks the names in the models, which the reader takes as they are; each
// problem is an error: a sound id that is not one the list can name
// (MonsterSoundIds.h), a sound slot that names no sound of the model and no
// such id, a bitmap slot that is not one, a part slot that is not an NPC
// slot, and a part effect that the effect catalogue does not have (it must be
// loaded). `source` names the file in the problems.
void ValidateMonsterModelNames(std::span<const MonsterModelDefinition> models, const std::string& source,
                               std::vector<Items::ItemDataIssue>& issues);

// Whether a path of the model list, a file or a folder, is in the client
// folder (the working folder), found as the game finds it.
bool IsInClientFolder(const std::string& path, bool folder = false);

// Warns about each model and part file that is not in the client folder (the
// working folder): the model stays without a body.
void ValidateMonsterModelFiles(std::span<const MonsterModelDefinition> models, const std::string& source,
                               std::vector<Items::ItemDataIssue>& issues);
} // namespace Data::Monsters
