#include "stdafx.h"

#include "GameLogic/Monsters/MonsterSoundEvents.h"

#include "Audio/DSPlaySound.h"
#include "Data/DataHandler/MonsterData/MonsterModelLoader.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "Data/GameData/MonsterData/MonsterSoundSetDatabase.h"
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
using Data::Monsters::MonsterDefinition;
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

// The old hooks' draws: rand_fps_check, or rand().
bool Draw(MonsterSoundDraw draw, int oneIn)
{
    if (draw == MonsterSoundDraw::RandFps)
    {
        return rand_fps_check(oneIn);
    }
    return std::rand() % oneIn == 0;
}

void PlaySound(const MonsterSoundSet& set, const MonsterSoundEvent& event, const std::string& sound,
               const std::function<void(int, bool)>& play)
{
    if (const std::optional<int> number = Data::Monsters::ModelLoader::FindSoundNumber(set, sound))
    {
        play(*number, event.atMonster);
    }
}

// Whether one of the event's cues is a cue of the model in the animation, at
// the frame (Q-MN79).
bool IsOnCue(const MonsterSoundEvent& event, std::span<const MonsterModelCue> cues, const Situation& situation)
{
    return std::any_of(event.cues.begin(), event.cues.end(),
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

bool IsWhen(const MonsterSoundEvent& event, std::span<const MonsterModelCue> cues, const Situation& situation)
{
    if (situation.moment)
    {
        return std::find(event.moments.begin(), event.moments.end(), *situation.moment) != event.moments.end();
    }
    return std::find(event.animations.begin(), event.animations.end(), situation.animation) != event.animations.end() ||
           IsOnCue(event, cues, situation);
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

void PlayEvent(const MonsterSoundSet& set, const MonsterSoundEvent& event, const std::function<void(int, bool)>& play)
{
    if (event.chance && !Draw(event.chance->draw, event.chance->oneIn))
    {
        return;
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
        return;
    }
    for (const std::string& sound : event.sounds)
    {
        PlaySound(set, event, sound, play);
    }
}

// Whether the monster appears with the model in `slot`: its entry's, or one
// of its random variants'.
bool AppearsWith(const MonsterDefinition& monster, int slot)
{
    if (Data::Monsters::FindMonsterModelReference(monster.model) == slot)
    {
        return true;
    }
    return std::any_of(monster.randomVariants.begin(), monster.randomVariants.end(),
                       [&](const Data::Monsters::MonsterRandomVariant& variant)
                       { return Data::Monsters::FindMonsterModelReference(variant.model) == slot; });
}

// Plays the events of the character's set in the situation of its animation,
// or at `moment`.
bool PlayFor(CHARACTER& character, std::optional<MonsterSoundMoment> moment)
{
    const MonsterSoundSet* set = FindSoundSet(character);
    if (set == nullptr || set->soundEvents.empty())
    {
        return false;
    }
    OBJECT& object = character.Object;
    const MonsterModelDefinition* model = g_MonsterModelDatabase.Find(object.Type);
    const std::span<const MonsterModelCue> cues =
        model != nullptr ? std::span<const MonsterModelCue>(model->cues) : std::span<const MonsterModelCue>();
    Situation situation;
    situation.animation = object.CurrentAction;
    situation.frame = object.AnimationFrame;
    situation.moment = moment;
    situation.deathEffectArmed = object.LifeTime == ArmedDeathEffect;
    situation.heard =
        !IsFarFromHero(object) &&
        (set->maps.empty() || Data::Maps::Names(set->maps, gMapManager.GetServerMap(), gMapManager.GetServerStage()));
    PlayEvents(*set, cues, situation, [&object](int sound, bool atMonster)
               { PlayBuffer(static_cast<ESound>(sound), atMonster ? &object : nullptr); });
    return true;
}
} // namespace

const MonsterSoundSet* FindSoundSet(const CHARACTER& character)
{
    const MonsterDefinition* monster = g_MonsterDatabase.Find(static_cast<int>(character.MonsterIndex));
    const int slot = character.Object.Type;
    if (monster == nullptr || !AppearsWith(*monster, slot))
    {
        return nullptr;
    }
    const std::string& name = Data::Monsters::GetSoundSetWithModel(*monster, slot);
    return name.empty() ? nullptr : g_MonsterSoundSetDatabase.Find(name);
}

bool Play(CHARACTER& character)
{
    return PlayFor(character, std::nullopt);
}

bool PlayMoment(CHARACTER& character, MonsterSoundMoment moment)
{
    return PlayFor(character, moment);
}

void PlayEvents(const MonsterSoundSet& set, std::span<const MonsterModelCue> cues, const Situation& situation,
                const std::function<void(int sound, bool atMonster)>& play)
{
    for (const MonsterSoundEvent& event : set.soundEvents)
    {
        if (Applies(event, cues, situation))
        {
            PlayEvent(set, event, play);
        }
    }
}
} // namespace GameLogic::Monsters::SoundEvents
