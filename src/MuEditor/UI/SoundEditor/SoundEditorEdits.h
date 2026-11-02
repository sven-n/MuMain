#pragma once

#ifdef _EDITOR

#include "SoundEditorRows.h"

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/MapData/MapObjectSound.h"
#include "Data/GameData/MonsterData/MonsterSoundSet.h"
#include "Data/GameData/SoundData/SoundFile.h"

#include <map>
#include <optional>
#include <string>
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
};

// The row's values as they are.
RowValues ValuesOf(const SoundRow& row);

// One step of the history: the table before and after, of one file.
struct SoundEdit
{
    std::string label;
    std::optional<std::vector<Data::Sounds::SoundFile>> codeBefore;
    std::optional<std::vector<Data::Sounds::SoundFile>> codeAfter;
    std::optional<Data::Monsters::MonsterSoundSet> setBefore;
    std::optional<Data::Monsters::MonsterSoundSet> setAfter;
    std::optional<std::vector<Data::Maps::MapObjectSound>> objectsBefore;
    std::optional<std::vector<Data::Maps::MapObjectSound>> objectsAfter;
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

    bool CanUndo() const
    {
        return !m_undo.empty();
    }
    bool CanRedo() const
    {
        return !m_redo.empty();
    }
    bool Undo();
    bool Redo();

    // Whether a file's table differs from what was last read or saved.
    bool IsModified() const;
    // Writes the files whose tables changed: SoundFiles.json, the monster
    // data (the monster tools' Save, all three of its files) and
    // ObjectSounds.json. False when one was refused (the problems in
    // `issues`); the others are written.
    bool Save(std::vector<Data::Items::ItemDataIssue>& issues);
    // Reads the changed files again (a monster set: as it was saved); the
    // history goes.
    bool Revert(std::vector<Data::Items::ItemDataIssue>& issues);
    // The tables as they are now are what Save and Revert compare with.
    void TakeSaved();

    // Forgets the history and the saved state (tests).
    void Clear();

private:
    SoundEdits() = default;

    void Apply(const SoundEdit& edit, bool forward);

    std::vector<SoundEdit> m_undo;
    std::vector<SoundEdit> m_redo;
    std::optional<std::vector<Data::Sounds::SoundFile>> m_savedCode;
    // The sets changed since the last save, as they were then.
    std::map<std::string, Data::Monsters::MonsterSoundSet> m_savedSets;
    std::optional<std::vector<Data::Maps::MapObjectSound>> m_savedObjects;
};
} // namespace MuEditor::SoundEditor

#define g_SoundEdits MuEditor::SoundEditor::SoundEdits::GetInstance()

#endif // _EDITOR
