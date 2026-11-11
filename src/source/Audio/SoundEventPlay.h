#pragma once

#include "Audio/DSPlaySound.h"
#include "Data/GameData/SoundData/SoundEvents.h"

#include <optional>
#include <string_view>

class CHARACTER;

// The game's code asks the sound events which sound to play (sound editor
// design, Q-SE1): Data/Audio/SoundEvents.json decides.
namespace Audio
{
// The sound of the event's first rule that matches, picked at random among
// its sounds (Random::Rand() % count, only when it has more than one); nullopt for
// no rule or no such event.
std::optional<ESound> PickEventSound(std::string_view event, const Data::Sounds::SoundEventContext& context);

// Plays PickEventSound's sound, if any, as an interface sound (no place in
// the world): the clicks of buttons and of characters (Q-UI1).
void PlayInterfaceEventSound(std::string_view event, const Data::Sounds::SoundEventContext& context);

// What the player holds, as the swing at an attack's start tells it: a bow,
// a crossbow, the bow group's items 13 and 14, a Light Saber or Spear in the
// right hand, any other weapon, nothing.
Data::Sounds::HeldWeapon HeldWeaponOf(const CHARACTER& c);
} // namespace Audio
