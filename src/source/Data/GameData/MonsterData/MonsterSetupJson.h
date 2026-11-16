#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/MonsterData/MonsterSetup.h"

#include <functional>
#include <string>

// The setup fields of a monster or a number in the monster list
// (Data/Monsters/Monsters.json), flat beside its other values:
//
//   "scale": 1.2, "glowMesh": 1, "noTurnOnHit": true,
//   "equipment": {"weapons": [{"group": 5, "number": 0, "level": 11}]},
//   "spawnEffects": [{"joint": "BITMAP_JOINT_ENERGY", "subType": 2, "scale": 30}],
//
// A number's own value null clears a field the monster has (no equipment, no
// scale of its own: the game's).
namespace Data::Monsters
{
using SetupProblem =
    std::function<void(Items::ItemDataIssueSeverity severity, const std::string& field, const std::string& message)>;

// Reads the setup fields the object has into `setup`; each replaces the
// field, null clears it. `fieldPrefix` names the problems ("monster 36 ").
// False when one is fatal (a value of the wrong kind); a name the code does
// not know (a joint, an object, a buff) is an Error and the field keeps it.
bool ReadMonsterSetup(const Items::Json::OrderedJson& json, const std::string& fieldPrefix, MonsterSetup& setup,
                      const SetupProblem& problem);

// Writes each field the setup has.
void WriteMonsterSetup(Items::Json::OrderedJson& json, const MonsterSetup& setup);

// Writes each field the entry has differently from the shared setup; null
// where the entry has none.
void WriteMonsterSetupDifferences(Items::Json::OrderedJson& json, const MonsterSetup& shared,
                                  const MonsterSetup& entry);

// The equipment of a random variant (D63), the same object as the setup
// field's; `field` names the problems ("monster 163 randomVariants[0].equipment").
bool ReadMonsterEquipment(const Items::Json::OrderedJson& value, const std::string& field, MonsterEquipment& equipment,
                          const SetupProblem& problem);
Items::Json::OrderedJson WriteMonsterEquipment(const MonsterEquipment& equipment);

// Adds the keys of the setup fields.
void AddMonsterSetupKeys(Items::Json::KeySet& keys);
} // namespace Data::Monsters
