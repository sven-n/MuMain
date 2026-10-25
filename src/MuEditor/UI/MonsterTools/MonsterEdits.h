#pragma once

#ifdef _EDITOR

#include "MonsterEditHistory.h"

#include "Data/GameData/ItemData/ItemDataIssue.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace MuEditor::Monsters
{
// The one way the monster tools change monsters: each change goes through
// MonsterDataHandler (the monster table and the names of living monsters stay
// in step) and is recorded in the undo history under `label`, the text the
// Undo button shows. A change that changes nothing records nothing.
class MonsterEdits
{
public:
    static MonsterEdits& GetInstance();

    // Stores the monster at its number, which must have an entry (a kind
    // change). False, with nothing changed, when it has none or a name has the
    // separator or a language code that is not one.
    bool SetMonster(std::string label, const Data::Monsters::MonsterDefinition& monster);
    // Sets the monster's name in `locale`; an empty text removes the
    // translation. False, with nothing changed, when the number has no entry,
    // the locale is not a language code, the text has the separator
    // (LocalizedString::Separator) or the English name would be empty. An
    // English name that is empty already (252) may stay so.
    bool SetMonsterName(std::string label, int number, std::string_view locale, std::string text);
    // Adds the monster at its number. False, with nothing changed, when the
    // number is outside 0-1023 or has an entry (also one with an empty name),
    // the English name is empty, or a name has the separator or a language
    // code that is not one.
    bool AddMonster(std::string label, const Data::Monsters::MonsterDefinition& monster);
    // Removes the monster's entry; false when it has none.
    bool RemoveMonster(std::string label, int number);

    // Stores the model entry at its slot (MonsterModelDefinition::slot),
    // which must have an entry, for the monster the tool shows it for (its
    // undo selects it again; -1 for none). False, with nothing changed, when
    // the slot has no entry or the model list's save would refuse it (no
    // texture folder, a sound slot that names no sound, a part slot that is
    // not one).
    bool SetModel(std::string label, const Data::Monsters::MonsterModelDefinition& model, int monster);

    // The changes until EndGroup are one undo step under `label` (an import of
    // names): a number changed twice keeps its first "before" and its last
    // "after".
    void BeginGroup(std::string label);
    void EndGroup();

    // Undo and Redo select the monster they changed. False when there is
    // nothing to do or the monsters changed past the history
    // (MonsterEditHistory::GetLastRefusal).
    bool Undo();
    bool Redo();

    // MonsterDataHandler::Save and SaveModels, the monster list and the
    // model list; the saved monsters and models are the history's saved
    // state. The model list is not written when the monster list was not.
    bool Save(std::vector<Data::Items::ItemDataIssue>& issues);
    // MonsterDataHandler::Revert and RevertModels; the history is dropped
    // (OnDataReplaced) and the monsters and models read are the saved state.
    bool Revert(std::vector<Data::Items::ItemDataIssue>& issues);
    // Drops the history: the monsters or models were replaced as a whole, so
    // no step before can be undone.
    void OnDataReplaced();

    MonsterEditHistory& GetHistory()
    {
        return m_history;
    }

private:
    MonsterEdits();

    // Stores `after` at `number` (nullopt removes the entry) and records the
    // change as one entry.
    void Change(std::string label, int number, const std::optional<Data::Monsters::MonsterDefinition>& after);
    // The same for a model slot.
    void ChangeModel(std::string label, int slot, const std::optional<Data::Monsters::MonsterModelDefinition>& after,
                     int monster);
    // Into the history, or into the open group.
    void Record(MonsterEditEntry entry);

    MonsterEditHistory m_history;
    std::optional<MonsterEditEntry> m_group;
};
} // namespace MuEditor::Monsters

#define g_MonsterEdits MuEditor::Monsters::MonsterEdits::GetInstance()

#endif // _EDITOR
