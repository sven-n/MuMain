#pragma once

#include <optional>
#include <string_view>

// The SOUND_* ids that the monster model list can name (Data/Monsters/
// Models.json "sounds" and "soundSlots"): the sounds of the models that code
// plays by their id. A model's other sounds are named by their file and get a
// number when they are loaded (Audio/SoundNumbers.h).
namespace Data::Monsters
{
// The id of a SOUND_* name of the list, or nullopt.
std::optional<int> FindMonsterSoundId(std::string_view name);

// The name of an id of the list; empty for others.
std::string_view GetMonsterSoundIdName(int id);
} // namespace Data::Monsters
