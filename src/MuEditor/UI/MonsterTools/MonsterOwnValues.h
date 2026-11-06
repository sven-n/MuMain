#pragma once

#ifdef _EDITOR

#include "MonsterShownValues.h"

#include "Data/GameData/MonsterData/MonsterDefinition.h"
#include "Data/GameData/MonsterData/MonsterGroup.h"

#include <string>
#include <vector>

// The shared and own values of a number of a monster with several numbers in
// the monster tools (Q-MN50, Q-MN95, Q-MN96): a shared value is grey and
// locked until "Make it this number's own", an own value yellow with "Back
// to..." the monster's value or another number's.
namespace MuEditor::Monsters::OwnValues
{
enum class State
{
    // A monster with one number, or a monster as a whole: no colours.
    Plain,
    // The number shares the monster's value: locked.
    Shared,
    // The number shares it but may change it: made its own, not changed yet.
    Unlocked,
    // The number has a value of its own.
    Own,
};

State GetState(const ShownValues::ShownMonster& shown, Data::Monsters::MonsterValueField field);
// The same for a number, as the selection would show it.
State GetNumberState(int number, Data::Monsters::MonsterValueField field);
// Whether the number has the field's value from its monster with several
// numbers (Shared or Unlocked): it is changed on the monster as a whole.
bool SharesValue(int number, Data::Monsters::MonsterValueField field);

// The line over a field: grey "Shared by the 3 numbers of Death Centurion"
// with "Make it this number's own", or yellow "336's own value" with "Back
// to...". Nothing for Plain.
void RenderLine(const ShownValues::ShownMonster& shown, Data::Monsters::MonsterValueField field);

// The field's controls in its colour until EndField, disabled while shared.
void BeginField(State state);
void EndField(State state);
// A value as text in its colour: grey while shared, yellow when own (the
// Monster Browser's names).
void RenderValueText(State state, const std::string& text);

// The items of a row's context menu for the field: "Kind: Make it this
// number's own" or "Kind: Back to...".
void RenderMenuItems(int number, Data::Monsters::MonsterValueField field);

// The name of the field in the editor language: "Kind".
const char* GetFieldName(Data::Monsters::MonsterValueField field);

// A value "Back to..." offers: the monster's, or another number's own.
struct Source
{
    std::string label;
    Data::Monsters::MonsterDefinition values;
};

// The monster's value and each other number's own value of the field, once
// per value, without the number's own value.
std::vector<Source> FindSources(int number, Data::Monsters::MonsterValueField field);

// Gives the number the field's value of the source, as one undo step; the
// field is locked again when it is shared then. False when the store
// refuses it.
bool TakeValue(int number, Data::Monsters::MonsterValueField field, const Source& source);

// "Make it this number's own": the field may be changed on the number while
// it is in this monster. A split, move or removal that takes the number out
// of the monster or into another one locks it again, also once undone.
void Unlock(int number, Data::Monsters::MonsterValueField field);
// Locks the fields unlocked on the number: an action took it into or out of
// a monster (MonsterEdits).
void LockNumber(int number);
// After a revert: every shared field is locked again.
void LockAll();
// Counts Unlock, LockNumber, LockAll and the locks of TakeValue, so a list of
// the unlocked fields can tell it is out of date.
int GetUnlockVersion();
} // namespace MuEditor::Monsters::OwnValues

#endif // _EDITOR
