#pragma once

#ifdef _EDITOR

#include "../MuEditor/UI/ItemTools/ItemEditDraft.h"
#include "Data/GameData/MonsterData/MonsterModelDefinition.h"

#include <string>
#include <vector>

// The sound events of a model entry in the Monster Model tool (monsters
// design, MN7, Q-MN76): a row per event with when it plays (animations or
// moments), its frames, death effect, SubType, whether it is heard anywhere
// and played at the monster, chance, pick and sounds, which it picks from the
// model's own sounds, and the maps the events play on. Each change is one
// undo step.
namespace MuEditor::Monsters::SoundEventsEditor
{
using ModelDraft = Items::ItemEditDraft<Data::Monsters::MonsterModelDefinition>;

// The animations or moments the event plays at: "attack1, attack2", or "on"
// and the moments.
std::string WhenText(const Data::Monsters::MonsterSoundEvent& event);

// Takes the moment out of the event's moments, or puts it in; an event of
// animations becomes one of this moment. False, with nothing changed, for
// the event's last moment: an event plays at one at least.
bool ToggleMoment(Data::Monsters::MonsterSoundEvent& event, Data::Monsters::MonsterSoundMoment moment);

// As ToggleMoment, for an animation; an event of moments becomes one of this
// animation.
bool ToggleAnimation(Data::Monsters::MonsterSoundEvent& event, int animation);

// Which characters' SubType the event plays for.
enum class SubTypeCondition
{
    Any,
    Only,
    Not,
};

SubTypeCondition GetSubTypeCondition(const Data::Monsters::MonsterSoundEvent& event);

// Keeps the event's SubType value when it had one, else names 9 (the second
// set of the Death Centurion and Shriker models).
void SetSubTypeCondition(Data::Monsters::MonsterSoundEvent& event, SubTypeCondition condition);

// The sounds Play plays for the event, without its chance: all of them, or
// with a pick the `draw`th (0 or 1, drawn at random), the only one when it has
// one sound.
std::vector<std::string> PreviewSounds(const Data::Monsters::MonsterSoundEvent& event, int draw);

// `slot` is the model's slot, `modelOpen` whether the game has it open (its
// animations are offered then; else the named ones).
void Render(Data::Monsters::MonsterModelDefinition& model, int slot, bool modelOpen, ModelDraft& draft);
} // namespace MuEditor::Monsters::SoundEventsEditor

#endif // _EDITOR
