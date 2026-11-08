#pragma once

#include "Audio/SoundSource.h"

#include <optional>

class CHARACTER;
class OBJECT;

// The places of the game's own sounds for the editor's Sound Mixer (Q-SM13):
// who or what a sound plays for, which gives it its type and the overlay's
// marker. In the game's own builds they do nothing.
namespace Audio
{
#ifdef _EDITOR
// The type of a character's sounds by its object's kind (KIND_*): the hero
// or another player, a monster (or trap), an NPC; none for the rest.
std::optional<SoundKind> CharacterSoundKind(int objectKind, bool isHero);

// Names the character as the place of the sounds played while `place` lives;
// nothing for a character without a type.
void PlaceCharacter(std::optional<ScopedSoundPlace>& place, const CHARACTER& c);

// Remembers the living place as the maker of the effect or joint just created
// in `effect`'s slot; with none living it forgets the slot's last maker.
void RememberMaker(const void* effect);

// An effect's or joint's place as it moves: at its own position, with its
// maker's type and number; one nobody made is an Effect of its own type.
SoundPlace EffectPlace(const void* effect, int type, const float* position);

// Names the effect or joint as the place of the sounds played while `place`
// lives (EffectPlace).
void PlaceEffect(std::optional<ScopedSoundPlace>& place, const void* effect, int type, const float* position);

// The place of the object PlayBuffer was given: the hero, a character by its
// kind, a ground item, a bird or fish, a mount by its rider, an effect by its
// maker; else everything else at the object's position. None for nullptr.
std::optional<SoundPlace> PlaceOfObject(const OBJECT* object);

// Forgets every maker (tests).
void ForgetMakers();
#else
inline void PlaceCharacter(std::optional<ScopedSoundPlace>&, const CHARACTER&) {}
inline void RememberMaker(const void*) {}
inline void PlaceEffect(std::optional<ScopedSoundPlace>&, const void*, int, const float*) {}
#endif
} // namespace Audio
