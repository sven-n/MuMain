#pragma once

#ifdef _EDITOR

#include "Data/GameData/MonsterData/MonsterKind.h"

#include <span>

// The kind of a monster as the monster tools show and edit it.
namespace MuEditor::Monsters::KindField
{
// The kinds, in the order the tools list them.
std::span<const Data::Monsters::MonsterKind> GetKinds();

// "Monster", "NPC", "Trap", "Other" in the editor language; read it when
// drawing, as it changes with the language.
const char* GetLabel(Data::Monsters::MonsterKind kind);

// The kind the game gives a number without an entry: the old kind rules of
// the number (Data::Monsters::GetLegacyObjectKind).
Data::Monsters::MonsterKind GetLegacyKind(int number);

// A combo of the kinds; true when another kind was picked.
bool RenderCombo(const char* label, Data::Monsters::MonsterKind& kind);
} // namespace MuEditor::Monsters::KindField

#endif // _EDITOR
