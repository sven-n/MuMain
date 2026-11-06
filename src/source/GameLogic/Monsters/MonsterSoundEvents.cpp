#include "stdafx.h"
#include "Core/Utilities/Random.h"

#include "GameLogic/Monsters/MonsterSoundEvents.h"
#include "GameLogic/Monsters/MonsterModelTimers.h"

#include "Audio/DSPlaySound.h"
#include "Audio/SoundOverlay.h"
#include "Audio/SoundSource.h"
#include "Data/DataHandler/MonsterData/MonsterModelLoader.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
#include "Data/GameData/MonsterData/MonsterSoundSetLinks.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "World/MapInfra/MapManager.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <string>

namespace GameLogic::Monsters::SoundEvents
{
namespace
{
using Data::Monsters::MonsterModelCue;
using Data::Monsters::MonsterModelDefinition;
using Data::Monsters::MonsterSoundDraw;
using Data::Monsters::MonsterSoundEvent;
using Data::Monsters::MonsterSoundMoment;
using Data::Monsters::MonsterSoundSet;

// The hooks' cut-off: no extra sounds of monsters farther from the hero.
constexpr float HearingDistance = 500.0f;
// OBJECT::LifeTime of a character whose death effect is armed.
constexpr int ArmedDeathEffect = 100;
// A pick draws one in two.
constexpr int PickChance = 2;

bool IsFarFromHero(const OBJECT& object)
{
    const float x = object.Position[0] - Hero->Object.Position[0];
    const float y = object.Position[1] - Hero->Object.Position[1];
    return std::sqrt(x * x + y * y) > HearingDistance;
}

// The old hooks' draws: rand_fps_check, or Random::Rand().
bool Draw(MonsterSoundDraw draw, int oneIn)
{
    if (draw == MonsterSoundDraw::RandFps)
    {
        return rand_fps_check(oneIn);
    }
    return Random::Rand() % oneIn == 0;
}

void PlaySound(const MonsterSoundSet& set, const MonsterSoundEvent& event, const std::string& sound,
               const std::function<void(int, bool)>& play)
{
    if (const std::optional<int> number = Data::Monsters::ModelLoader::FindSoundNumber(set, sound))
    {
        play(*number, event.atMonster);
    }
}

// Whether one of `names` is a cue of the model in the animation, at the frame
// (Q-MN79, Q-MN90).
bool IsOnCue(const std::vector<std::string>& names, std::span<const MonsterModelCue> cues, const Situation& situation)
{
    return std::any_of(names.begin(), names.end(),
                       [&](const std::string& name)
                       {
                           return std::any_of(cues.begin(), cues.end(),
                                              [&](const MonsterModelCue& cue)
                                              {
                                                  return cue.name == name && cue.animation == situation.animation &&
                                                         situation.frame >= cue.frames.first &&
                                                         situation.frame < cue.frames.second;
                                              });
                       });
}

bool Contains(const std::vector<int>& animations, int animation)
{
    return std::find(animations.begin(), animations.end(), animation) != animations.end();
}

bool IsWhen(const MonsterSoundEvent& event, std::span<const MonsterModelCue> cues, const Situation& situation)
{
    if (situation.drawn)
    {
        return Contains(event.during, situation.animation) || IsOnCue(event.duringCues, cues, situation) ||
               std::any_of(event.timers.begin(), event.timers.end(),
                           [&](const std::string& timer)
                           {
                               return std::find(situation.tickedTimers.begin(), situation.tickedTimers.end(), timer) !=
                                      situation.tickedTimers.end();
                           }) ||
               (!situation.oncePlayed && Contains(event.once, situation.animation));
    }
    if (situation.moment)
    {
        return std::find(event.moments.begin(), event.moments.end(), *situation.moment) != event.moments.end();
    }
    return std::find(event.animations.begin(), event.animations.end(), situation.animation) != event.animations.end() ||
           IsOnCue(event.cues, cues, situation);
}

bool Applies(const MonsterSoundEvent& event, std::span<const MonsterModelCue> cues, const Situation& situation)
{
    if (!IsWhen(event, cues, situation))
    {
        return false;
    }
    if (event.deathEffectArmed && !situation.deathEffectArmed)
    {
        return false;
    }
    return event.heardAnywhere || situation.heard;
}

// False when the event's chance did not come up.
bool PlayEvent(const MonsterSoundSet& set, const MonsterSoundEvent& event, const Situation& situation,
               const std::function<void(int, bool)>& play)
{
    if (event.chance && !situation.chanceDrawn && !Draw(event.chance->draw, event.chance->oneIn))
    {
        return false;
    }
    if (event.pick)
    {
        if (Draw(*event.pick, PickChance))
        {
            PlaySound(set, event, event.sounds[0], play);
        }
        else if (event.sounds.size() > 1)
        {
            PlaySound(set, event, event.sounds[1], play);
        }
        return true;
    }
    for (const std::string& sound : event.sounds)
    {
        PlaySound(set, event, sound, play);
    }
    return true;
}

std::span<const MonsterModelCue> CuesOf(const OBJECT& object)
{
    const MonsterModelDefinition* model = g_MonsterModelDatabase.Find(object.Type);
    return model != nullptr ? std::span<const MonsterModelCue>(model->cues) : std::span<const MonsterModelCue>();
}

// The situation of the character's animation; heard within 500 units and on
// the set's maps.
Situation SituationOf(const OBJECT& object, const MonsterSoundSet& set)
{
    Situation situation;
    situation.animation = object.CurrentAction;
    situation.frame = object.AnimationFrame;
    situation.deathEffectArmed = object.LifeTime == ArmedDeathEffect;
    situation.heard =
        !IsFarFromHero(object) &&
        (set.maps.empty() || Data::Maps::Names(set.maps, gMapManager.GetServerMap(), gMapManager.GetServerStage()));
    return situation;
}

// Plays the events of the character's set in the situation of its animation,
// or at `moment`; `chanceDrawn` as PlayMoment.
bool PlayFor(CHARACTER& character, std::optional<MonsterSoundMoment> moment, bool chanceDrawn)
{
    const MonsterSoundSet* set = FindSoundSet(character);
    if (set == nullptr || set->soundEvents.empty())
    {
        return false;
    }
    OBJECT& object = character.Object;
    const Audio::ScopedSoundSource source(Audio::SoundKind::Monster, set->name,
                                          static_cast<int>(character.MonsterIndex), object.Position);
    Situation situation = SituationOf(object, *set);
    situation.moment = moment;
    situation.chanceDrawn = chanceDrawn;
    PlayEvents(*set, CuesOf(object), situation, [&object](int sound, bool atMonster)
               { PlayBuffer(static_cast<ESound>(sound), atMonster ? &object : nullptr); });
    return true;
}

#ifdef _EDITOR
// The Sound Mixer's overlay marks a drawn monster with a sound set and its
// hearing ring.
void NoteForOverlay(const CHARACTER& character)
{
    if (!Audio::Overlay::IsOn())
    {
        return;
    }
    const MonsterSoundSet* set = FindSoundSet(character);
    if (set == nullptr)
    {
        return;
    }
    Audio::Overlay::DrawnSource drawn;
    drawn.kind = Audio::SoundKind::Monster;
    drawn.name = set->name;
    drawn.sourceNumber = static_cast<int>(character.MonsterIndex);
    drawn.position = {character.Object.Position[0], character.Object.Position[1], character.Object.Position[2]};
    // A set heard anywhere (Kanturu's) has no hearing ring: a ring would say
    // its sounds stop there.
    const bool limited = std::any_of(set->soundEvents.begin(), set->soundEvents.end(),
                                     [](const MonsterSoundEvent& event) { return !event.heardAnywhere; });
    drawn.inner = limited ? HearingDistance : 0.0f;
    Audio::Overlay::NoteDrawn(drawn);
}
#endif
} // namespace

const MonsterSoundSet* FindSoundSet(const CHARACTER& character)
{
    return Data::Monsters::SoundSetLinks::FindSet(static_cast<int>(character.MonsterIndex), character.Object.Type);
}

bool Play(CHARACTER& character)
{
    return PlayFor(character, std::nullopt, false);
}

bool PlayMoment(CHARACTER& character, MonsterSoundMoment moment, bool chanceDrawn)
{
    return PlayFor(character, moment, chanceDrawn);
}

bool PlayDrawn(CHARACTER& character, bool* played)
{
    character.SoundsDrawnThisFrame = true;
    if (played != nullptr)
    {
        *played = false;
    }
    const MonsterSoundSet* set = FindSoundSet(character);
    if (set == nullptr || !HasDrawnEvents(*set))
    {
        return false;
    }
    OBJECT& object = character.Object;
    const Audio::ScopedSoundSource source(Audio::SoundKind::Monster, set->name,
                                          static_cast<int>(character.MonsterIndex), object.Position);
    Situation situation = SituationOf(object, *set);
    situation.drawn = true;
    situation.oncePlayed = character.SoundOncePlayed;
    situation.tickedTimers = ModelTimers::TickedNames(character);
    // The flag is set after the frame's events, so that once events of one
    // animation play together (the Witch Queen's thunder and attack).
    if (PlayEvents(
            *set, CuesOf(object), situation, [&object](int sound, bool atMonster)
            { PlayBuffer(static_cast<ESound>(sound), atMonster ? &object : nullptr); }, played))
    {
        character.SoundOncePlayed = true;
    }
    // One rule for every set (Q-MN89): standing or walking clears the flag.
    if (object.CurrentAction == MONSTER01_STOP1 || object.CurrentAction == MONSTER01_STOP2 ||
        object.CurrentAction == MONSTER01_WALK)
    {
        character.SoundOncePlayed = false;
    }
    return true;
}

void FinishDrawnFrame(CHARACTER& character)
{
#ifdef _EDITOR
    NoteForOverlay(character);
#endif
    if (!character.SoundsDrawnThisFrame)
    {
        PlayDrawn(character);
    }
    character.SoundsDrawnThisFrame = false;
    ModelTimers::FinishFrame(character);
}

bool HasDrawnEvents(const MonsterSoundSet& set)
{
    return std::any_of(
        set.soundEvents.begin(), set.soundEvents.end(), [](const MonsterSoundEvent& event)
        { return !event.during.empty() || !event.once.empty() || !event.duringCues.empty() || !event.timers.empty(); });
}

bool PlayEvents(const MonsterSoundSet& set, std::span<const MonsterModelCue> cues, const Situation& situation,
                const std::function<void(int sound, bool atMonster)>& play, bool* played)
{
    bool oncePlayed = false;
    bool anyPlayed = false;
    for (const MonsterSoundEvent& event : set.soundEvents)
    {
        if (!Applies(event, cues, situation) || !PlayEvent(set, event, situation, play))
        {
            continue;
        }
        anyPlayed = true;
        if (situation.drawn && Contains(event.once, situation.animation))
        {
            oncePlayed = true;
        }
    }
    if (played != nullptr)
    {
        *played = anyPlayed;
    }
    return oncePlayed;
}
} // namespace GameLogic::Monsters::SoundEvents
