#pragma once

#ifdef _EDITOR

#include "../MuEditor/UI/ItemTools/ItemEditDraft.h"
#include "Data/GameData/MonsterData/MonsterModelDefinition.h"

#include <string>

// The sound events of a model entry in the Monster Model tool (monsters
// design, MN7): a row per event with its animations, frames, death effect,
// chance, pick and sounds, which it picks from the model's own sounds, and
// the maps the events play on. Each change is one undo step.
namespace MuEditor::Monsters::SoundEventsEditor
{
using ModelDraft = Items::ItemEditDraft<Data::Monsters::MonsterModelDefinition>;

// `slot` is the model's slot, `modelOpen` whether the game has it open (its
// animations are offered then; else the named ones). `mapsText` keeps the
// text of the maps field while it is edited.
void Render(Data::Monsters::MonsterModelDefinition& model, int slot, bool modelOpen, ModelDraft& draft,
            std::string& mapsText);
} // namespace MuEditor::Monsters::SoundEventsEditor

#endif // _EDITOR
