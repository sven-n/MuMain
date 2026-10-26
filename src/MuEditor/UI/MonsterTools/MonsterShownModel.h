#pragma once

#ifdef _EDITOR

#include "Data/GameData/MonsterData/MonsterDefinition.h"

#include <optional>

// The model of the selected monster, as the Monster Model and Monster
// Preview tools show it.
namespace MuEditor::Monsters::ShownModel
{
// The slot of the monster's model in the model list; nullopt when the
// monster names no model, or one outside the list ("player", a map object)
// or a slot without an entry.
std::optional<int> FindSlot(const Data::Monsters::MonsterDefinition& monster);

// The tools show the animations of an open model: a model the game does not
// have open (no monster of it appeared on this map) is opened between frames
// (MonsterDataHandler::OpenModelForTools). Called each frame the slot is
// shown, it asks once per slot, map and model table, so a model whose file
// is missing is not tried again each frame.
void OpenWhenShown(int slot);

// Whether the game has the slot's model open now.
bool IsOpen(int slot);

// "2 Budge Dragon", or that no monster is selected.
void RenderSelectedMonster(int number, const Data::Monsters::MonsterDefinition* monster);

// Why the monster's model is not shown: it names none, or one outside the
// model list.
void RenderWhyNoModel(const Data::Monsters::MonsterDefinition& monster);
} // namespace MuEditor::Monsters::ShownModel

#endif // _EDITOR
