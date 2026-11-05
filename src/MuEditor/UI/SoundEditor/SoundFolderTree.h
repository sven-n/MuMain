#pragma once

#ifdef _EDITOR

#include "Data/GameData/SoundData/SoundFolders.h"

#include <map>
#include <set>
#include <string>
#include <vector>

// What the Sound Editor does to its folders (sound editor design, Q-SE4), as
// in the Colors window: folders are made, renamed, removed (their contents
// move up), moved among their siblings, and sounds are moved between them.
namespace MuEditor::SoundEditor
{
// The names from the top to a folder; {} for the top.
using FolderPath = std::vector<std::string>;

// The folder at the path; nullptr for none or for the top.
Data::Sounds::SoundFolder* FindFolder(std::vector<Data::Sounds::SoundFolder>& folders, const FolderPath& path);

// "New folder", else "New folder 2", 3, ... ("Old 2" goes on with "Old 3"):
// a name no folder of `siblings` has.
std::string FreeFolderName(const std::vector<Data::Sounds::SoundFolder>& siblings, const std::string& base);

// Adds a folder named FreeFolderName(base) at the end of `parent`'s folders
// (the top for {}); its path, or {} when there is no such parent.
FolderPath AddFolder(std::vector<Data::Sounds::SoundFolder>& folders, const FolderPath& parent,
                     const std::string& base);

// Gives the folder another name; false, with nothing changed, for an empty
// name, one a sibling has, or no such folder.
bool RenameFolder(std::vector<Data::Sounds::SoundFolder>& folders, const FolderPath& path, const std::string& name);

// Removes the folder; its folders (a name its new siblings have gets a free
// one) and its sounds move into its parent, where it was. False for no such
// folder.
bool RemoveFolder(std::vector<Data::Sounds::SoundFolder>& folders, const FolderPath& path);

// Moves the folder `by` places among its siblings (-1 up, 1 down); false at
// the end or for no such folder.
bool MoveFolder(std::vector<Data::Sounds::SoundFolder>& folders, const FolderPath& path, int by);

// Puts the sound into the folder, out of the one it was in; {} takes it out
// of every folder ("Not sorted"). False for no such folder or no change.
bool MoveSound(std::vector<Data::Sounds::SoundFolder>& folders, const std::string& key, const FolderPath& to);

// The sounds whose keys changed (`renamed`: before to after, SoundKeyFollow)
// stay where they are under the new keys, all at once, so keys may trade
// places; another entry of a new key (a sound that is gone) goes, so the
// sound is in one folder. False when nothing changed.
bool RenameSounds(std::vector<Data::Sounds::SoundFolder>& folders, const std::map<std::string, std::string>& renamed);

// The folders that hold a key no row has: the keys are dropped (a sound
// that is gone), the count returned.
size_t DropStaleSounds(std::vector<Data::Sounds::SoundFolder>& folders, const std::vector<std::string>& keys);

// How many keys of the folders no row has (`keys`), which DropStaleSounds
// would drop.
size_t CountStaleSounds(const std::vector<Data::Sounds::SoundFolder>& folders, const std::set<std::string>& keys);
} // namespace MuEditor::SoundEditor

#endif // _EDITOR
