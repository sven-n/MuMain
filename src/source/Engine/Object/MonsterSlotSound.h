#pragma once

#include "Audio/DSPlaySound.h"

#include <optional>

class BMD;

// The sounds of a model's sound slots (BMD::Sounds: idle 0 and 1, attack 2
// and 3, die 4, and the same five again from 5 on, the second set; each the
// sound's number minus SOUND_MONSTER), which the characters of the model play
// when they stand, walk, attack, are hit and die.
namespace Engine::Object::MonsterSlotSound
{
// The value of a slot without a sound.
constexpr short NoSound = -1;
// The first slot of the second set.
constexpr int SecondSetSlot = 5;
// The SubType of a monster that plays the second set (the big Hellas
// monsters).
constexpr int SecondSetSubType = 9;

// The slot (0 to 9) a character with `subType` plays for the slot `slot` (0
// to 4) of the first set: the same slot of the second set for SubType 9.
int SlotFor(int slot, int subType);

// The sound of the slot (0 to 9); nullopt for a slot without a sound. The
// game picks one slot of a pair at random, so the second slot of a pair is
// checked, too (monsters design, P9).
std::optional<ESound> Find(const BMD& model, int slot);
} // namespace Engine::Object::MonsterSlotSound
