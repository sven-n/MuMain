#pragma once

#ifdef _EDITOR

#include "../MuEditor/UI/ItemTools/ItemEditDraft.h"
#include "Data/GameData/MonsterData/MonsterModelDefinition.h"
#include "Data/GameData/MonsterData/MonsterSoundSet.h"

#include <optional>
#include <string>
#include <utility>
#include <vector>

// The sound events of a sound set in the Monster Sounds tool (monsters
// design, MN7, Q-MN76, Q-MN78 to Q-MN82): a row per event with when it plays
// (moments, animations or cues of the shown monster's model), on a cue's row
// the cue's frames, which change the model's cue, the death effect, whether
// it is heard anywhere and played at the monster, chance, pick and sounds,
// which it picks from the set's own sounds, and the maps the events play on.
// Each change is one undo step.
namespace MuEditor::Monsters::SoundEventsEditor
{
using SetDraft = Items::ItemEditDraft<Data::Monsters::MonsterSoundSet>;
using ModelDraft = Items::ItemEditDraft<Data::Monsters::MonsterModelDefinition>;

// The drafts of a sound set and of the model whose cues the rows change: the
// rows keep the event and the cue they draw while they store a change, so
// the drafts are taken back from the data at the next frame
// (ReloadAfterStore::NextFrame).
SetDraft MakeSetDraft(SetDraft::Load load, SetDraft::Store store);
ModelDraft MakeModelDraft(ModelDraft::Load load, ModelDraft::Store store);

// The moments, animations or cues the event plays at: "attack1, attack2",
// "on" and the moments, or "cue" and the cues.
std::string WhenText(const Data::Monsters::MonsterSoundEvent& event);

// Takes the moment out of the event's moments, or puts it in; an event of
// animations or cues becomes one of this moment. False, with nothing changed,
// for the event's last moment: an event plays at one at least.
bool ToggleMoment(Data::Monsters::MonsterSoundEvent& event, Data::Monsters::MonsterSoundMoment moment);

// As ToggleMoment, for an animation.
bool ToggleAnimation(Data::Monsters::MonsterSoundEvent& event, int animation);

// As ToggleMoment, for a cue of the model.
bool ToggleCue(Data::Monsters::MonsterSoundEvent& event, const std::string& cue);

// The sounds Play plays for the event, without its chance: all of them, or
// with a pick the `draw`th (0 or 1, drawn at random), the only one when it has
// one sound.
std::vector<std::string> PreviewSounds(const Data::Monsters::MonsterSoundEvent& event, int draw);

// A frame as the tools show it and name a new cue after it: "4.50".
std::string FrameText(float frame);

// Keeps frames that a cue's inputs changed at frame 0 or later, the cue at
// least 0.1 frames long.
void KeepCueFrames(std::pair<float, float>& frames);

// A cue at `frame` of `animation`, one frame long, named after them ("walk
// 4.50") unless the model has that name, then with a number added.
Data::Monsters::MonsterModelCue MakeCue(const Data::Monsters::MonsterModelDefinition& model, int animation,
                                        float frame);

// Whether an event of a set that the monsters play with the model in `slot`
// names the cue, leaving out the event `except` of the set `exceptSet`.
bool IsCueUsed(int slot, const std::string& cue, const std::string& exceptSet, size_t except);

// The model's cues that no event of a set that the monsters play with the
// model in `slot` names, in the model's order: Monster Model removes them.
std::vector<std::string> FindUnusedCues(const Data::Monsters::MonsterModelDefinition& model, int slot);

// What the event table edits besides the set: the model of the shown
// monster's look, whose cues the cue rows show and change.
// The cue rows change the cues of `model` only: the model of the selected
// monster's shown look while the set shown is the one that look plays.
struct Context
{
    // The model's slot and draft; -1 and nullptr when the monster has no
    // model in the list.
    int slot = -1;
    Data::Monsters::MonsterModelDefinition* model = nullptr;
    ModelDraft* modelDraft = nullptr;
    // Whether the game has the model open (its animations are offered then;
    // else the named ones).
    bool modelOpen = false;
    // The monsters with the model, which a cue's change reaches.
    int modelMonsters = 0;
    // The monster shown, which the undo steps select again.
    int monster = -1;
    // The event to mark and scroll to once (Monster Preview's cue tags).
    std::optional<size_t> focusedEvent;
};

void Render(Data::Monsters::MonsterSoundSet& set, SetDraft& draft, const Context& context);
} // namespace MuEditor::Monsters::SoundEventsEditor

#endif // _EDITOR
