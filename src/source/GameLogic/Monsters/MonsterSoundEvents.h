#pragma once

#include "Data/GameData/MonsterData/MonsterModelDefinition.h"

#include <functional>
#include <optional>

class OBJECT;

// The sound events of the monster models (monsters design, MN7, Q-MN76): the
// sounds that the game plays for a character of the model in an animation or
// at a moment, from the model entries of Data/Monsters/Models.json.
namespace GameLogic::Monsters::SoundEvents
{
// What decides which of a model's sound events play.
struct Situation
{
    // The character's animation and its frame.
    int animation = 0;
    float frame = 0.0f;
    // A moment of the game: the events of the moment play, whatever the
    // animation. nullopt: the events of the animation.
    std::optional<Data::Monsters::MonsterSoundMoment> moment;
    bool deathEffectArmed = false;
    int subType = 0;
    // Within 500 units of the hero and on the model's soundEventMaps; an
    // event that is not heard anywhere plays only then.
    bool heard = true;
};

// Plays the sound events of the object's model that its animation and frame
// call for (PlayMonsterSound). False, with nothing done, when the model's
// entry has no sound events.
bool Play(OBJECT& object);

// Plays the sound events of the object's model at `moment`, whatever its
// animation. False, with nothing done, when the model's entry has no sound
// events.
bool PlayMoment(OBJECT& object, Data::Monsters::MonsterSoundMoment moment);

// The model's sound events that play in `situation`, in order, each event's
// chance drawn and its pick picked; `play` gets the number of each sound and
// whether it plays with the character as PlayBuffer's object.
void PlayEvents(const Data::Monsters::MonsterModelDefinition& model, const Situation& situation,
                const std::function<void(int sound, bool atMonster)>& play);
} // namespace GameLogic::Monsters::SoundEvents
