#pragma once

#ifdef _EDITOR

#include "MonsterCodeReferences.h"

namespace MuEditor::Monsters
{
// The lines of the sources the editor was built from that name a monster
// number. Read on the first call, so the game does not pay for it unless a
// monster tool asks; not loaded when the sources are gone.
const MonsterCodeReferences& GetMonsterCodeReferences();
} // namespace MuEditor::Monsters

#endif // _EDITOR
