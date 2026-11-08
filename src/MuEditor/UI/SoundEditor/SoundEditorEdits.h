#pragma once

#ifdef _EDITOR

#include "SoundEditorRows.h"
#include "SoundKeyFollow.h"

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/MapData/MapObjectSound.h"
#include "Data/GameData/MonsterData/MonsterSoundSet.h"
#include "Data/GameData/SoundData/SoundEvents.h"
#include "Data/GameData/SoundData/SoundFile.h"
#include "Data/GameData/SoundData/SoundFolders.h"

#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

// The Sound Editor's changes (sound editor design, Q-SE1): a row's name,
// file, channels and 3D, into the table of the file the row lives in, at once
// (a loaded sound loads again), in one undo history; Save writes the files
// that changed, Revert reads them again.
namespace MuEditor::SoundEditor
{
// What a change sets on a row.
struct RowValues
{
    std::string name;
    std::string file;
    int channels = 1;
    bool spatial = false;
    // A code sound's only.
    std::optional<Audio::SoundKind> type;
};

// The row's values as they are.
RowValues ValuesOf(const SoundRow& row);

// A set's sound before and after, and where its set's events named it (they
// follow its name).
struct SetSoundStep
{
    std::string set;
    size_t index = 0;
    Data::Monsters::MonsterSound before;
    Data::Monsters::MonsterSound after;
    // Event, then sound in the event.
    std::vector<std::pair<size_t, size_t>> eventNames;
};

// A sound of a set that this window changed since the set was last saved or
// read: as it was then and as this window left it, at its index then.
struct SetSoundTurn
{
    size_t index = 0;
    Data::Monsters::MonsterSound from;
    Data::Monsters::MonsterSound to;
};

// A map object sound before and after.
struct ObjectSoundStep
{
    size_t index = 0;
    Data::Maps::MapObjectSound before;
    Data::Maps::MapObjectSound after;
};

// One step of the history: the table before and after, of one file. The
// Monster Sounds window and the Map Editor's Sounds tab change the sets and
// the map object sounds too, so a step there is only the one sound it
// changed: an undo puts that back and keeps what they changed since.
struct SoundEdit
{
    std::string label;
    std::optional<std::vector<Data::Sounds::SoundFile>> codeBefore;
    std::optional<std::vector<Data::Sounds::SoundFile>> codeAfter;
    std::optional<SetSoundStep> setSound;
    std::optional<ObjectSoundStep> objectSound;
    std::optional<std::vector<Data::Sounds::SoundFolder>> foldersBefore;
    std::optional<std::vector<Data::Sounds::SoundFolder>> foldersAfter;
    std::optional<std::vector<Data::Sounds::SoundEvent>> eventsBefore;
    std::optional<std::vector<Data::Sounds::SoundEvent>> eventsAfter;
};

class SoundEdits
{
public:
    static SoundEdits& GetInstance();

    // Sets the row's values; a map object sound has no 3D and a row that
    // nothing loads takes none. False, with nothing changed, when nothing
    // differs, the row is gone or its file would refuse the change (the
    // monster tools' checks).
    bool Change(const SoundRow& row, const RowValues& values);

    // The folders (Q-SE4), read from Data/Audio/SoundFolders.json the first
    // time: the sounds whose keys changed since the file was written keep
    // their folders under the new keys.
    const std::vector<Data::Sounds::SoundFolder>& Folders();
    // Goes up each time the folders change (a step, a read of the file, a
    // key followed), for what the window counts of them.
    int GetFoldersVersion() const
    {
        return m_foldersVersion;
    }
    // The folders, also the history's steps, keep the sounds whose keys
    // changed since the last call in any window (FollowedKeys: a set renamed
    // in Monster Sounds, a file picked in the Map Editor's Sounds tab); Save
    // then writes the new keys. A set renamed elsewhere keeps this window's
    // changes and steps under its new name. The editor calls it each frame.
    void FollowKeyChanges();
    // What kept the folders' file from being read (its fatal problems): the
    // folders stay as they are, so Save never writes over the file, until
    // Revert reads it again. Empty when it was read.
    const std::vector<Data::Items::ItemDataIssue>& FolderProblems();
    // Replaces the folders as one step of the history; false when nothing
    // differs or the file was not read (FolderProblems).
    bool ChangeFolders(std::string label, std::vector<Data::Sounds::SoundFolder> after);

    // Replaces the sound events (the Events tab, Q-SE1) as one step of the
    // history; the game plays by them at once. False when nothing differs.
    bool ChangeEvents(std::string label, std::vector<Data::Sounds::SoundEvent> after);

    bool CanUndo() const
    {
        return !m_undo.empty();
    }
    bool CanRedo() const
    {
        return !m_redo.empty();
    }
    // False, with nothing changed, when there is nothing to do or the step's
    // sound changed since (in another window): the step stays, and
    // WasLastRefused says so.
    bool Undo();
    bool Redo();
    bool WasLastRefused() const
    {
        return m_lastRefused;
    }

    // Whether a file's table differs from what was last read or saved.
    bool IsModified() const;
    // Writes the files whose tables changed: SoundFiles.json, the sets this
    // window changed into Data/Monsters/Sounds.json (SaveSoundSets: the
    // monster tools' own unsaved changes stay unsaved),
    // ObjectSounds.json, SoundFolders.json and SoundEvents.json. The folders'
    // file names the sounds by the keys the files written give them, so a
    // set another window renamed and has not saved keeps its old keys there.
    // False when one was refused (the problems in `issues`); the others are
    // written.
    bool Save(std::vector<Data::Items::ItemDataIssue>& issues);
    // Reads the files whose tables changed again (a monster set this window
    // changed: as Data/Monsters/Sounds.json has it, under the name another
    // window gave it since) and the folders; the history goes. A table whose
    // file cannot be read, and a set that was not put back, stay not saved.
    bool Revert(std::vector<Data::Items::ItemDataIssue>& issues);
    // The tables as they are now, and the folders as their file has them, are
    // what Save and Revert compare with.
    void TakeSaved();
    // The Map Editor's Sounds tab wrote Data/Maps/ObjectSounds.json or read it
    // again: the map object sounds as they are now are the file's, here too.
    void TakeSavedObjects();

    // Forgets the history and the saved state (tests).
    void Clear();

private:
    SoundEdits() = default;

    // False, with nothing changed, when the step's sound is not as the other
    // direction left it.
    bool Apply(const SoundEdit& edit, bool forward);
    void ReadFolders();
    // The keys' tables changed since the last call (FollowKeyChanges).
    void FollowTables();
    // In the folders and in the steps' folders.
    void RenameInFolders(const std::map<std::string, std::string>& renamed);
    // Sets renamed in another window since (RenamedSets): the turns and the
    // steps name them by their new names, and m_fileNames keeps their names
    // in the file.
    void FollowRenamedSets(const std::map<std::string, std::string>& renamed);
    // The name in Data/Monsters/Sounds.json of the set that has `name` now.
    std::string FileNameOf(const std::string& name) const;
    // The tables as Data/Monsters/Sounds.json and Data/Maps/ObjectSounds.json
    // have them; m_savedTables' part for a file that cannot be read.
    KeyedTables ReadKeyedTables() const;
    // Whether the folders' file names its sounds as the files of
    // m_savedTables do (m_foldersFileStale).
    void CheckFoldersFile();
    // This window turned the set's sound at `index` from `from` into `to`
    // (a change, an undo or a redo): the turn that left it as `from` goes on,
    // or a new one starts; one back where it started goes.
    void Turn(const std::string& set, size_t index, const Data::Monsters::MonsterSound& from,
              const Data::Monsters::MonsterSound& to);
    // The sets this window changed, as Data/Monsters/Sounds.json has them now
    // (Revert): the Monster Sounds window's unsaved changes of them go too, a
    // set renamed there keeps its new name, and a set the file does not have
    // goes. False when the file could not be read or the monster tools
    // refused a set (a problem in `issues`); the turns of the sets not put
    // back are in `kept`.
    bool RevertSets(std::vector<Data::Items::ItemDataIssue>& issues,
                    std::map<std::string, std::vector<SetSoundTurn>>& kept);

    std::vector<SoundEdit> m_undo;
    std::vector<SoundEdit> m_redo;
    bool m_lastRefused = false;
    std::optional<std::vector<Data::Sounds::SoundFile>> m_savedCode;
    // The sounds this window changed in sets since the last save, by the
    // sets' names now (a rename in another window follows): IsModified, and
    // what Save writes into the file's sets, so the Monster Sounds window's
    // unsaved changes of the same sets stay its own; Revert reads those sets
    // from the file.
    std::map<std::string, std::vector<SetSoundTurn>> m_setTurns;
    // The sets renamed in another window since the monster tools last wrote
    // or read their files as a whole (MonsterDataHandler::GetFilesVersion,
    // m_filesVersion): name now to the name in Data/Monsters/Sounds.json,
    // which Save writes to and Revert reads from. The other sets have the
    // file's names.
    std::map<std::string, std::string> m_fileNames;
    int m_filesVersion = -1;
    std::optional<std::vector<Data::Maps::MapObjectSound>> m_savedObjects;
    // The folders, and the folders as their file has them, both under the
    // keys of now; and the file's text, under its own keys.
    std::optional<std::vector<Data::Sounds::SoundFolder>> m_folders;
    std::optional<std::vector<Data::Sounds::SoundFolder>> m_savedFolders;
    std::vector<Data::Sounds::SoundFolder> m_foldersFile;
    int m_foldersVersion = 0;
    std::vector<Data::Items::ItemDataIssue> m_folderProblems;
    // The tables whose keys the folders and the steps' folders name, with
    // their databases' versions.
    std::optional<KeyedTables> m_keyed;
    std::pair<int, int> m_keyedVersions{-1, -1};
    // The tables as their files have them (Data/Monsters/Sounds.json,
    // Data/Maps/ObjectSounds.json), whose keys Save writes into the folders'
    // file; the tables whose keys the folders' file has (when the editor
    // found them, or the files' at the last Save of the folders), with the
    // sets renamed since (name now to the name there); and whether a sound
    // the folders' file holds has another key in the files (its file was
    // saved since): then the folders' file is not saved.
    std::optional<KeyedTables> m_savedTables;
    std::optional<KeyedTables> m_foldersFileTables;
    std::map<std::string, std::string> m_folderNames;
    bool m_foldersFileStale = false;
    std::optional<std::vector<Data::Sounds::SoundEvent>> m_savedEvents;
};
} // namespace MuEditor::SoundEditor

#define g_SoundEdits MuEditor::SoundEditor::SoundEdits::GetInstance()

#endif // _EDITOR
