#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/MonsterData/MonsterModelDefinition.h"

#include <span>
#include <string>
#include <string_view>
#include <vector>

// The model list of the monster area (Data/Monsters/Models.json):
//
//   { "formatVersion": 1,
//     "models": [
//       {
//         "slot": "MONSTER_MODEL_BUDGE_DRAGON",
//         "file": "Data/Monster/Monster03.bmd",
//         "textureFolders": ["Monster"],
//         "animations": {"walk": 0.7},
//         "headBone": 7,
//         "sounds": [
//           {"file": "Data/Sound/mBudge1.wav", "channels": 2, "spatial": true}
//         ],
//         "soundSlots": {"idle": ["Data/Sound/mBudge1.wav", ...], "attack": [...], "die": ...}
//       } ] }
//
// docs/monster-data.md "Models" describes the fields. The reader checks the
// form of the values; whether the names in them (slots, sound ids, effect
// types, bitmap slots) exist is checked when the models are loaded.
namespace Data::Monsters
{
constexpr int MonsterModelsFormatVersion = 1;

// Reads the models of the text into `models`, in their order. A model with an
// error is left out; a slot that is there twice is an error.
void ReadMonsterModelsJson(std::string_view text, const std::string& source,
                           std::vector<MonsterModelDefinition>& models, std::vector<Items::ItemDataIssue>& issues);

// The text of the file with these models, sorted by slot.
std::string WriteMonsterModelsJson(std::span<const MonsterModelDefinition> models);

// How a problem of a model names its field: "model MONSTER_MODEL_HOUND file".
std::string MonsterModelField(std::string_view slot, std::string_view key);
} // namespace Data::Monsters
