#pragma once

#include "Audio/SoundSource.h"

#include <optional>
#include <string>

// The sounds the game's code loads by their SOUND_* id, with their files
// (Data/Audio/SoundFiles.json, sound editor design Q-SE1): the code still
// loads each where it did (LoadGameSound), the file, channels and 3D come
// from here.
namespace Data::Sounds
{
// A sound's channels (MAX_CHANNEL): how many copies of it play at once.
constexpr int MaxSoundChannels = 4;

struct SoundFile
{
    // "SOUND_CLICK01": the sound's identity, which the code and the data
    // point at (Q-SE1).
    std::string id;
    // Its number (ESound).
    int number = -1;
    // What the editor shows instead of the id; "" for none.
    std::string name;
    // "Data/Sound/iButtonClick.wav", below the client folder.
    std::string file;
    int channels = 1;
    // Placed in 3D at the object it plays at.
    bool spatial = false;
    // Its type where nothing names a source or place (Q-SE2); none: the
    // mixer's own rule.
    std::optional<::Audio::SoundKind> type;

    bool operator==(const SoundFile&) const = default;
};
} // namespace Data::Sounds
