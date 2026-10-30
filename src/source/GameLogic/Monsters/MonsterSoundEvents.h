#pragma once

#include "Data/GameData/MonsterData/MonsterModelDefinition.h"
#include "Data/GameData/MonsterData/MonsterSoundSet.h"

#include <functional>
#include <optional>
#include <span>

class CHARACTER;

// The sound events of the monsters (monsters design, MN7, Q-MN76, Q-MN78): the
// sounds that the game plays for a monster in an animation, on a cue of its
// model or at a moment, from the sound set its entry names
// (Data/Monsters/Sounds.json).
namespace GameLogic::Monsters::SoundEvents
{
// What decides which of a set's sound events play.
struct Situation
{
    // The character's animation and its frame.
    int animation = 0;
    float frame = 0.0f;
    // A moment of the game: the events of the moment play, whatever the
    // animation. nullopt: the events of the animation and its cues.
    std::optional<Data::Monsters::MonsterSoundMoment> moment;
    bool deathEffectArmed = false;
    // Within 500 units of the hero and on the set's maps; an event that is
    // not heard anywhere plays only then.
    bool heard = true;
};

// The sound set the character plays: the one its monster (MonsterIndex) names
// for the model it appears with (OBJECT::Type, GetSoundSetWithModel). Only the
// entry's model and its random variants' models play it; nullptr for a
// character without a monster number (players), another model, or no set.
const Data::Monsters::MonsterSoundSet* FindSoundSet(const CHARACTER& character);

// Plays the sound events of the character's set that its animation and frame
// call for (PlayMonsterSound). False, with nothing done, when the set has no
// sound events.
bool Play(CHARACTER& character);

// Plays the sound events of the character's set at `moment`, whatever its
// animation. False, with nothing done, when the set has no sound events.
bool PlayMoment(CHARACTER& character, Data::Monsters::MonsterSoundMoment moment);

// The set's sound events that play in `situation`, in order, each event's
// chance drawn and its pick picked; `cues` are those of the model the
// monster appears with. `play` gets the number of each sound and whether it
// plays with the character as PlayBuffer's object.
void PlayEvents(const Data::Monsters::MonsterSoundSet& set, std::span<const Data::Monsters::MonsterModelCue> cues,
                const Situation& situation, const std::function<void(int sound, bool atMonster)>& play);
} // namespace GameLogic::Monsters::SoundEvents
