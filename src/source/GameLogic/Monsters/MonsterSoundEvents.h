#pragma once

#include "Data/GameData/MonsterData/MonsterModelDefinition.h"
#include "Data/GameData/MonsterData/MonsterSoundSet.h"

#include <functional>
#include <optional>
#include <span>
#include <string>
#include <vector>

class CHARACTER;

// The sound events of the monsters (monsters design, MN7, Q-MN76, Q-MN78,
// Q-MN84, Q-MN85): the sounds that the game plays for a monster in an
// animation, on a cue of its model, at a moment, or each frame it is drawn,
// from the sound set its entry names (Data/Monsters/Sounds.json).
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
    // The caller drew the chance already: the events' own chance is not
    // drawn (the hero's walk, PlayMoment).
    bool chanceDrawn = false;
    // A frame the character is drawn (PlayDrawn): the events during its
    // animation play, and those once in it unless `oncePlayed`.
    bool drawn = false;
    // A once event played since the character last stood.
    bool oncePlayed = false;
    // The timers of the model that tick in the drawn frame (Q-MN91).
    std::vector<std::string> tickedTimers;
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
// animation. With `chanceDrawn` the caller drew the chance in place of the
// events' own: SetPlayerWalk draws one in 64 for the hero (a transformed
// player), as the original client did, where the sets' idle events have one
// in 16. False, with nothing done, when the set has no sound events.
bool PlayMoment(CHARACTER& character, Data::Monsters::MonsterSoundMoment moment, bool chanceDrawn = false);

// Plays the events of the character's set for a frame it is drawn: those
// during its animation, and those once in it unless one played since it
// stood or walked; drawn in stop1, stop2 or walk, the flag clears (Q-MN84,
// Q-MN85, Q-MN89; the flag is CHARACTER::SoundOncePlayed, Q-MN87). The Aida
// monsters' effects (M33Aida::UpdateAidaMonsterEffects) call it where they
// played their sounds, so the random draws around it keep their order;
// UpdateDrawnCharacterEffects calls FinishDrawnFrame for every character the
// last frame drew. False when the set has no such events.
// `played`, when given, says whether an event of the frame played (its chance
// came up): the map's effects that come with a sound follow it (Blade
// Hunter's smoke with its step, Q-MN92).
bool PlayDrawn(CHARACTER& character, bool* played = nullptr);

// UpdateDrawnCharacterEffects, after the maps' monster effects: plays the
// drawn frame's events unless that code did (PlayDrawn), and makes the next
// frame a new one.
void FinishDrawnFrame(CHARACTER& character);

// Whether the set has events that play in drawn frames (during, once, cues of
// drawn frames, timers).
bool HasDrawnEvents(const Data::Monsters::MonsterSoundSet& set);

// The set's sound events that play in `situation`, in order, each event's
// chance drawn and its pick picked; `cues` are those of the model the
// monster appears with. `play` gets the number of each sound and whether it
// plays with the character as PlayBuffer's object. True when a once event
// played (passed its chance): the caller sets the flag.
// `played`, when given, becomes whether any event played.
bool PlayEvents(const Data::Monsters::MonsterSoundSet& set, std::span<const Data::Monsters::MonsterModelCue> cues,
                const Situation& situation, const std::function<void(int sound, bool atMonster)>& play,
                bool* played = nullptr);
} // namespace GameLogic::Monsters::SoundEvents
