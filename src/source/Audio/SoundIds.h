#pragma once

#include "Audio/DSPlaySound.h"

#include <optional>
#include <string_view>

// The names of the SOUND_* ids (DSPlaySound.h): the hidden identity by which
// the code and the data name a sound of the code (sound editor design,
// Q-SE1). SoundIds.cpp lists every name of the enum.
namespace Audio::SoundIds
{
// The id of a name; nullopt for a name that is none.
std::optional<ESound> Find(std::string_view name);

// The name of an id; "" for a number that has none.
std::string_view NameOf(ESound id);
} // namespace Audio::SoundIds
