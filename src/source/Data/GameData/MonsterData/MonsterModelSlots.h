#pragma once

#include "Core/Globals/_define.h"
#include "Core/Globals/_enum.h"

#include <optional>
#include <string>
#include <string_view>

// The model slots of the monster area (monsters design, D47) and the names
// the code gives them: a monster model by its model type
// ("MONSTER_MODEL_HOUND", slot MODEL_MONSTER01 + type), an NPC model by its
// slot ("MODEL_SMITH"; "MODEL_LITTLESANTA+3" for a slot without a name of its
// own).
namespace Data::Monsters
{
// The slot of a symbol, or nullopt for a text that names none.
std::optional<int> FindMonsterModelSlot(std::string_view symbol);

// The symbol of a slot of the monster or NPC range; empty for other slots.
std::string_view GetMonsterModelSymbol(int slot);

bool IsMonsterRangeSlot(int slot);
bool IsNpcRangeSlot(int slot);

// The name of a monster model type for logs; "UNKNOWN" outside the types.
std::string_view GetMonsterModelTypeName(int type);

// What a monster's "model" names (monsters design, D47): a model of the
// model list by its slot ("MONSTER_MODEL_HOUND", "MODEL_SMITH"), "player"
// (MODEL_PLAYER, a player-shaped monster), a model of another area by the
// code's name of its slot ("MODEL_WARCRAFT" for the Kalima gates,
// "MODEL_BALL" for the soccer ball), a map object ("mapObject 39" for a
// trap), or "none": NoModelSlot, a monster without a body (the Imperial
// Guardian's traps 523, P2). The slot, or nullopt for a text that names none.
std::optional<int> FindMonsterModelReference(std::string_view model);

// The reserved slot of "none": the last of the monster range, which no model
// and no code uses; it draws nothing and loads no sounds.
constexpr int NoModelSlot = MODEL_MONSTER01 + MAX_MODEL_MONSTER - 1;

// How a monster's "model" starts that names a map object.
constexpr std::string_view MapObjectPrefix = "mapObject ";

// How a monster's "model" names the slot; empty for a slot no monster can
// use.
std::string GetMonsterModelReference(int slot);

// The fixed bitmap slots that monster models load bitmaps into, which their
// drawing code uses: "BITMAP_MONSTER_SKIN", "BITMAP_MONSTER_SKIN+1",
// "BITMAP_ROBE+3", "BITMAP_ROBE+5". nullopt for other texts.
std::optional<int> FindMonsterModelBitmapSlot(std::string_view symbol);
} // namespace Data::Monsters
