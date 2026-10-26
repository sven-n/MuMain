#pragma once

#include <optional>
#include <string_view>

// The SOUND_* ids that the monster data can name (Data/Monsters/Models.json
// "sounds", "soundSlots" and "soundEvents", Monsters.json "attacks.sound"):
// the sounds that code plays by their id, among them sounds the game loads
// for all maps or a map. A model's other sounds are named by their file and
// get a number when they are loaded (Audio/SoundNumbers.h).
namespace Data::Monsters
{
// The id of a SOUND_* name of the list, or nullopt.
std::optional<int> FindMonsterSoundId(std::string_view name);
} // namespace Data::Monsters
