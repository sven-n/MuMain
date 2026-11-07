#pragma once

#ifdef _EDITOR

#include "MonsterEditHistory.h"

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/MapData/MapRef.h"
#include "Data/GameData/MonsterData/MonsterGroup.h"

#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
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
    // Removes the monster's entry, and the number from its monster; false
    // when it has none.
    bool RemoveMonster(std::string label, int number);

    // The monsters with their numbers (D61). A number's monster is the one
    // MonsterDataHandler lists it in, else a monster of its own; the actions
    // name a monster by one of its numbers.

    // The numbers of the number's monster that have an entry, sorted; empty
    // when the number has none.
    std::vector<int> ReadMonsterNumbers(int number) const;
    // Their count and the first of them without a copy, for the tools' reads
    // each frame; 0 and -1 when the number has no entry.
    int CountMonsterNumbers(int number) const;
    int FindFirstNumber(int number) const;
    // Every monster once, as ReadMonsterNumbers gives its numbers, in the
    // order of their first numbers.
    std::vector<std::vector<int>> ReadMonsters() const;
    // The maps the numbers appear on (MonsterDefinition::maps), sorted, each
    // once; numbers without an entry have none.
    std::vector<Data::Maps::MapRef> ReadMaps(std::span<const int> numbers) const;
    // The values the numbers of the number's monster share
    // (Data::Monsters::MakeSharedValues): its entry's when it is the
    // monster's only number; nullopt when it has no entry.
    std::optional<Data::Monsters::MonsterDefinition> ReadSharedValues(int number) const;
    // The same without a copy, for the tools' reads each frame: the shared
    // values of a monster with several numbers; nullptr for a number of a
    // monster of its own or without an entry. Valid until the next change.
    const Data::Monsters::MonsterDefinition* FindSharedValues(int number) const;
    // Sets the shared values of the number's monster (`number` and `maps` of
    // `values` are not used): a field that changes changes in the numbers
    // that share it; the numbers' own values stay (Q-MN96). For a monster
    // with one number it is SetMonster. False, with nothing changed, when the
    // number has no entry or a name has the separator or a language code
    // that is not one.
    bool SetSharedValues(std::string label, int number, const Data::Monsters::MonsterDefinition& values);
    // Adds `number` to the monster of `memberNumber`, with the monster's
    // shared values and no maps. False, with nothing changed, when
    // `memberNumber` has no entry or `number` is not free (IsFreeMonsterNumber).
    bool AddNumber(std::string label, int memberNumber, int number);
    // Adds `number` to the monster of `from` with from's values, its own
    // ones included, and no maps (Q-MN99). False like AddNumber.
    bool CopyNumber(std::string label, int from, int number);
    // Moves the number into the monster of `memberNumber`; its values stay,
    // and those equal to that monster's are shared there (Q-MN97). False,
    // with nothing changed, when one has no entry or both are in one monster.
    bool MoveNumber(std::string label, int number, int memberNumber);
    // Makes the number a monster of its own, with its values (Q-MN97). False
    // when it has no entry or is its monster's only number.
    bool SplitNumber(std::string label, int number);
    // Removes every number of the number's monster; false when it has no
    // entry.
    bool RemoveMonsterWithNumbers(std::string label, int number);
    // When every number of the number's monster has the same value in the
    // field and the monster another one, the monster takes it: the numbers
    // share it again (an import that gave each number the same name). No
    // entry changes. False when nothing changed.
    bool ShareEqualValues(std::string label, int number, Data::Monsters::MonsterValueField field);

    // Stores the model entry at its slot (MonsterModelDefinition::slot),
    // which must have an entry, for the monster the tool shows it for (its
    // undo selects it again; -1 for none). False, with nothing changed, when
    // the slot has no entry or the model list's save would refuse it (no
    // texture folder, a sound slot that names no sound, a part slot that is
    // not one).
    bool SetModel(std::string label, const Data::Monsters::MonsterModelDefinition& model, int monster);

    // Stores the sound set under its name, a new one or over the set of that
    // name, for the monster the tool shows it for (-1 for none). False, with
    // nothing changed, when the name is empty or the sound sets' save would
    // refuse it (a SOUND_ id that is not one, an event's sound that names no
    // sound of the set).
    bool SetSoundSet(std::string label, const Data::Monsters::MonsterSoundSet& set, int monster);
    // Removes the set of the name; false when there is none.
    bool RemoveSoundSet(std::string label, const std::string& name, int monster);
    // Gives the set another name, and the monsters that name it the new one,
    // in one undo step. False, with nothing changed, when there is no set
    // `from` or `to` is empty or the name of another set.
    bool RenameSoundSet(std::string label, const std::string& from, const std::string& to, int monster);

    // The changes until EndGroup are one undo step under `label` (an import of
    // names): a number changed twice keeps its first "before" and its last
    // "after".
    void BeginGroup(std::string label);
    void EndGroup();

    // Undo and Redo select the monster they changed, as a whole when the
    // change was made on its shared values. False when there is nothing to do
    // or the monsters changed past the history
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
    // The same for a sound set.
    void ChangeSoundSet(std::string label, const std::string& name,
                        const std::optional<Data::Monsters::MonsterSoundSet>& after, int monster);
    // The same for a monster with its numbers (nullopt removes it, as does a
    // layout with fewer than two numbers). The numbers it takes in or out
    // lose their unlocked fields (OwnValues::LockNumber).
    void ChangeGroup(std::string label, int id, const std::optional<Data::Monsters::MonsterGroupLayout>& after,
                     int monster);
    // The id and layout of the number's monster, which must have an entry.
    // Numbers without an entry are left out; a monster of its own (one
    // number left) is the number alone with its entry's values, under a new
    // id when no monster lists it.
    std::pair<int, Data::Monsters::MonsterGroupLayout> ReadLayoutOf(int number);
    // The layout of the number's monster when the number has an entry and
    // the monster two or more numbers with one; else nullptr.
    const Data::Monsters::MonsterGroupLayout* FindLayoutWithNumbers(int number) const;
    // Adds the entry, whose number is free, to the monster of `memberNumber`.
    void AddToMonsterOf(std::string label, int memberNumber, const Data::Monsters::MonsterDefinition& entry);
    // RenameSoundSet's changes of the entries and the shared values that name
    // the set.
    void RenameSetInMonsters(const std::string& label, const std::string& from, const std::string& to, int monster);
    // Into the history, or into the open group.
    void Record(MonsterEditEntry entry);
    // Selects what the last Undo or Redo changed.
    void SelectLastChanged();

    MonsterEditHistory m_history;
    std::optional<MonsterEditEntry> m_group;
};
} // namespace MuEditor::Monsters

#define g_MonsterEdits MuEditor::Monsters::MonsterEdits::GetInstance()

#endif // _EDITOR
