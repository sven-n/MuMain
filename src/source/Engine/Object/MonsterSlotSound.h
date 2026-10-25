#pragma once

#include "Audio/DSPlaySound.h"

#include <optional>

class BMD;

// The sounds of a model's sound slots (BMD::Sounds: idle 0 and 1, attack 2
// and 3, die 4; each the sound's number minus SOUND_MONSTER), which the
// characters of the model play when they stand, walk, attack, are hit and
// die.
namespace Engine::Object::MonsterSlotSound
{
// The value of a slot without a sound.
constexpr short NoSound = -1;

// The sound of the slot, `offset` numbers further on (a monster with
// SubType 9 plays the sounds five numbers further, its second set); nullopt
// for a slot without a sound. The game picks one slot of a pair at random,
// so the second slot of a pair is checked, too (monsters design, P9).
std::optional<ESound> Find(const BMD& model, int slot, int offset);
} // namespace Engine::Object::MonsterSlotSound
