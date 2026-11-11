#pragma once

#include <optional>
#include <string_view>

// The SOUND_* ids that the monster data can name (Data/Monsters/Sounds.json
// "sounds" and "soundEvents", Monsters.json "attacks.sound"):
// the sounds that code plays by their id, among them sounds the game loads
// for all maps or a map. A set's other sounds are named by their file and get
// a number when they are loaded (Audio/SoundNumbers.h). The ids are a list of
// their own; their names are the enum's (Audio/SoundIds.h).
namespace Data::Monsters
{
// The id of a SOUND_* name of the list, or nullopt.
std::optional<int> FindMonsterSoundId(std::string_view name);
// The first name of the list: a new sound field starts with it, so it names
// a sound the reader takes.
std::string_view GetFirstMonsterSoundName();
} // namespace Data::Monsters
