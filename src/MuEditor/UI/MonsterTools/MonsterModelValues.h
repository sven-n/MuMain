#pragma once

#ifdef _EDITOR

#include "Data/GameData/MonsterData/MonsterModelDefinition.h"

// The animation values of a model entry as the loader gives them to the
// model (MonsterModelLoader ApplyAnimations): the entry's own, else the
// defaults of the slot's range; and the edits of the Monster Model tool, which
// keep in the entry only what differs from those defaults.
namespace MuEditor::Monsters::ModelValues
{
float GetDefaultSpeed(int slot, int animation);
bool IsHeldByDefault(int slot, int animation);

float GetSpeed(const Data::Monsters::MonsterModelDefinition& model, int slot, int animation);
bool HasOwnSpeed(const Data::Monsters::MonsterModelDefinition& model, int animation);
bool HoldsLastFrame(const Data::Monsters::MonsterModelDefinition& model, int slot, int animation);

// Sets the speed; one that is the range's default leaves the entry.
void SetSpeed(Data::Monsters::MonsterModelDefinition& model, int slot, int animation, float speed);
// Sets whether the animation holds its last frame, out of the model's
// `count` animations; when they all hold as the range's defaults, the entry
// lists none.
void SetHold(Data::Monsters::MonsterModelDefinition& model, int slot, int count, int animation, bool hold);
} // namespace MuEditor::Monsters::ModelValues

#endif // _EDITOR
