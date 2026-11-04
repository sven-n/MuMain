#pragma once

#ifdef _EDITOR

#include "SoundEditorRows.h"

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
    // time.
    const std::vector<Data::Sounds::SoundFolder>& Folders();
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
    // Writes the files whose tables changed: SoundFiles.json, the monster
    // data (the monster tools' Save, all three of its files),
    // ObjectSounds.json, SoundFolders.json and SoundEvents.json. False when
    // one was refused (the problems in `issues`); the others are written.
    bool Save(std::vector<Data::Items::ItemDataIssue>& issues);
    // Reads the files whose tables changed again (a monster set: as it was
    // saved) and the folders; the history goes.
    bool Revert(std::vector<Data::Items::ItemDataIssue>& issues);
    // The tables as they are now are what Save and Revert compare with.
    void TakeSaved();

    // Forgets the history and the saved state (tests).
    void Clear();

private:
    SoundEdits() = default;

    // False, with nothing changed, when the step's sound is not as the other
    // direction left it.
    bool Apply(const SoundEdit& edit, bool forward);

    std::vector<SoundEdit> m_undo;
    std::vector<SoundEdit> m_redo;
    bool m_lastRefused = false;
    std::optional<std::vector<Data::Sounds::SoundFile>> m_savedCode;
    // The sets changed since the last save, as they were then.
    std::map<std::string, Data::Monsters::MonsterSoundSet> m_savedSets;
    std::optional<std::vector<Data::Maps::MapObjectSound>> m_savedObjects;
    std::optional<std::vector<Data::Sounds::SoundFolder>> m_folders;
    std::optional<std::vector<Data::Sounds::SoundFolder>> m_savedFolders;
    std::vector<Data::Items::ItemDataIssue> m_folderProblems;
    std::optional<std::vector<Data::Sounds::SoundEvent>> m_savedEvents;
};
} // namespace MuEditor::SoundEditor

#define g_SoundEdits MuEditor::SoundEditor::SoundEdits::GetInstance()

#endif // _EDITOR
