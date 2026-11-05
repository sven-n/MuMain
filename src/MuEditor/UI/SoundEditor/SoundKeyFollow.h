#pragma once

#ifdef _EDITOR

#include "Data/GameData/MapData/MapObjectSound.h"
#include "Data/GameData/MonsterData/MonsterSoundSet.h"

#include <map>
#include <string>
#include <vector>

// A folder names a set's sound and a map object sound by a key with the set's
// name and the file in it (SoundRow::key), which the Monster Sounds window and
// the Map Editor's Sounds tab change too: which sound took which key.
namespace MuEditor::SoundEditor
{
// The tables those keys come from.
struct KeyedTables
{
    // In the database's order (by name).
    std::vector<Data::Monsters::MonsterSoundSet> sets;
    std::vector<Data::Maps::MapObjectSound> objects;
};

// The sets as they are now.
std::vector<Data::Monsters::MonsterSoundSet> CurrentKeyedSets();

// The keys of the sounds `after` still has under another key, before to
// after: a set's sounds when the set was renamed (it is the same otherwise),
// a sound or an entry changed in its place, and a "#2" that moved when a sound
// of the same key came or went. Where sounds came, went and changed between
// the same neighbours, which one became which is not known: none of those.
// `names`: the sets known to have another name in `after` (before's name to
// after's), found so also when they changed otherwise.
std::map<std::string, std::string> FollowedKeys(const KeyedTables& before, const KeyedTables& after,
                                                const std::map<std::string, std::string>& names = {});

// The sets of `before` that `after` has under another name (renamed, alike
// otherwise, as FollowedKeys finds them), old name to new.
std::map<std::string, std::string> RenamedSets(const KeyedTables& before, const KeyedTables& after);
} // namespace MuEditor::SoundEditor

#endif // _EDITOR
