#pragma once

#ifdef _EDITOR

#include "Data/GameData/MonsterData/MonsterGroup.h"
#include "Data/GameData/MonsterData/MonsterSetup.h"

#include <optional>
#include <span>
#include <string_view>

// How Monster Setup shows the setup fields (monsters design, section 9,
// D55, Q-MN39): the section each is in and when a living monster takes a
// change of it.
namespace MuEditor::Monsters::SetupFields
{
enum class Section
{
    Look,
    Spawn,
    Appear,
    Behaviour,
    GateAndHooks,
    NpcRules,
};

// When a living monster of the number takes a change (D55): at once (its
// look, its speed), or when it appears next (Respawn live monsters shows it
// now).
enum class Takes
{
    AtOnce,
    NextSpawn,
};

struct Field
{
    // The monster list's key (MonsterSetupField::key).
    std::string_view key;
    // Its name in the editor language.
    const char* const* label;
    Section section;
    Takes takes;
};

// Every setup field once, in the order the window shows them.
std::span<const Field> GetFields();
const Field* Find(std::string_view key);
// The field's name in the editor language ("Scale"); the key for a key that
// is none.
const char* GetLabel(std::string_view key);

// The monster list's description of the field (isSet, isSame, copy); nullptr
// for a key that is none.
const Data::Monsters::MonsterSetupField* FindSetupField(std::string_view key);
// The value field a number may have as its own (MonsterValueField::SetupFirst
// + the field's index).
std::optional<Data::Monsters::MonsterValueField> GetValueField(std::string_view key);

// Whether the setup has the field (a variant changes only the fields it has).
bool IsSet(std::string_view key, const Data::Monsters::MonsterSetup& setup);
// Gives `to` the field's value of `from` (a variant takes the monster's value
// to change it); with an empty `from` the field is cleared.
void CopyField(std::string_view key, Data::Monsters::MonsterSetup& to, const Data::Monsters::MonsterSetup& from);
void ClearField(std::string_view key, Data::Monsters::MonsterSetup& setup);
} // namespace MuEditor::Monsters::SetupFields

#endif // _EDITOR
