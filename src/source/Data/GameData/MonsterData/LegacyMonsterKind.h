#pragma once

namespace Data::Monsters
{
// The OBJECT::Kind the client gave a monster number before the monster list
// (Data/Monsters/Monsters.json) had kinds: number ranges and lists of numbers
// (KIND_MONSTER, KIND_NPC, KIND_TRAP or KIND_TMP). The monster list holds the
// kind of every monster it has; this gives the kind of a number it has not
// (monsters design, D44).
int GetLegacyObjectKind(int number);
} // namespace Data::Monsters
