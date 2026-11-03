#pragma once

#ifdef _EDITOR

#include "Data/DataHandler/MonsterData/MonsterDataHandler.h"
#include "Data/GameData/MonsterData/MonsterDefinition.h"
#include "Data/GameData/MonsterData/MonsterModelDefinition.h"

#include <cstddef>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace MuEditor::Monsters
{
// One monster number an edit changed: its entry before and after (nullopt: no
// entry). Entries hold only what the monster list file holds, so a language
// switch between an edit and its undo changes nothing they compare.
struct MonsterEditStep
{
    int number = 0;
    std::optional<Data::Monsters::MonsterDefinition> before;
    std::optional<Data::Monsters::MonsterDefinition> after;
};

// One model slot an edit changed: its entry before and after (nullopt: no
// entry), and the monster the tools showed it for, which its undo selects
// again (-1: none).
struct MonsterModelEditStep
{
    int slot = 0;
    std::optional<Data::Monsters::MonsterModelDefinition> before;
    std::optional<Data::Monsters::MonsterModelDefinition> after;
    int monster = -1;
};

// What one action of a tool changed, undone and redone as a whole: an edited
// name, a kind, an added monster, an import of names, a model's speed. Each
// number and each model slot is in one step of an entry at most.
struct MonsterEditEntry
{
    std::string label;
    std::vector<MonsterEditStep> steps;
    std::vector<MonsterModelEditStep> models;
};

// The undo history of the monster tools for the session. Entries are applied
// back through the hooks (the editor's go through MonsterDataHandler), so the
// monster and model tables, the names of living monsters and the open models
// stay in step. An entry is only undone while the monsters and models are as
// it left them: a change that went past the history makes the undo refuse
// instead of overwriting it.
class MonsterEditHistory
{
public:
    struct Hooks
    {
        std::function<std::optional<Data::Monsters::MonsterDefinition>(int number)> readMonster;
        // Puts these entries back exactly (nullopt removes the entry).
        std::function<void(std::span<const Data::Monsters::MonsterSnapshot> snapshots)> restoreMonsters;
        // The same for the model entries; needed only for entries with
        // model steps.
        std::function<std::optional<Data::Monsters::MonsterModelDefinition>(int slot)> readModel;
        std::function<void(std::span<const Data::Monsters::MonsterModelSnapshot> snapshots)> restoreModels;
    };

    // Why the last Undo or Redo did nothing.
    enum class Refusal
    {
        None,
        NothingToDo,
        // The monsters or models are no longer as the entry expects.
        MonstersChanged,
    };

    // How many entries are kept; the oldest go first.
    static constexpr std::size_t MaxEntries = 256;

    explicit MonsterEditHistory(Hooks hooks);

    // Adds an entry after the current one; the entries that were undone are
    // gone. An entry without steps is not added.
    void Record(MonsterEditEntry entry);

    bool CanUndo() const;
    bool CanRedo() const;
    const std::string& GetUndoLabel() const;
    const std::string& GetRedoLabel() const;

    // Undoes the last entry, or redoes the next one. False, with nothing
    // changed, when there is none or the monsters are no longer as it expects
    // (GetLastRefusal says which).
    bool Undo();
    bool Redo();
    Refusal GetLastRefusal() const
    {
        return m_lastRefusal;
    }

    // The number the last Undo or Redo changed first, or the monster its
    // first model step was made for, to select it.
    std::optional<int> GetLastChangedMonster() const
    {
        return m_lastChangedMonster;
    }

    // Drops every entry, for data that replaced all monsters: it is not the
    // saved data until MarkSaved.
    void Clear();
    // The data as it is now was saved.
    void MarkSaved();
    // Whether the data changed since it was saved (or loaded).
    bool IsModifiedSinceSave() const;

private:
    bool TryApply(const MonsterEditEntry& entry, bool undoing);
    bool IsAsLeft(const MonsterEditEntry& entry, bool undoing) const;
    void Apply(const MonsterEditEntry& entry, bool undoing);

    Hooks m_hooks;
    std::vector<MonsterEditEntry> m_entries;
    // Entries before this index are done; the others were undone.
    std::size_t m_done = 0;
    // m_done when the data was saved; nullopt when that state is gone.
    std::optional<std::size_t> m_saved = 0;
    std::optional<int> m_lastChangedMonster;
    Refusal m_lastRefusal = Refusal::None;
};
} // namespace MuEditor::Monsters

#endif // _EDITOR
