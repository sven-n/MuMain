#include "stdafx.h"

#include "GameLogic/Monsters/MonsterSoundEvents.h"

#include "Audio/DSPlaySound.h"
#include "Data/DataHandler/MonsterData/MonsterModelLoader.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
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
using Data::Monsters::MonsterModelDefinition;
using Data::Monsters::MonsterSoundDraw;
using Data::Monsters::MonsterSoundEvent;
using Data::Monsters::MonsterSoundMoment;

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

void PlaySound(const MonsterModelDefinition& model, const MonsterSoundEvent& event, const std::string& sound,
               const std::function<void(int, bool)>& play)
{
    if (const std::optional<int> number = Data::Monsters::ModelLoader::FindSoundNumber(model, sound))
    {
        play(*number, event.atMonster);
    }
}

bool IsWhen(const MonsterSoundEvent& event, const Situation& situation)
{
    if (situation.moment)
    {
        return std::find(event.moments.begin(), event.moments.end(), *situation.moment) != event.moments.end();
    }
    return std::find(event.animations.begin(), event.animations.end(), situation.animation) != event.animations.end();
}

bool Applies(const MonsterSoundEvent& event, const Situation& situation)
{
    if (!IsWhen(event, situation))
    {
        return false;
    }
    if (event.frames && (situation.frame < event.frames->first || situation.frame >= event.frames->second))
    {
        return false;
    }
    if (event.deathEffectArmed && !situation.deathEffectArmed)
    {
        return false;
    }
    if ((event.subType && situation.subType != *event.subType) ||
        (event.notSubType && situation.subType == *event.notSubType))
    {
        return false;
    }
    return event.heardAnywhere || situation.heard;
}

void PlayEvent(const MonsterModelDefinition& model, const MonsterSoundEvent& event,
               const std::function<void(int, bool)>& play)
{
    if (event.chance && !Draw(event.chance->draw, event.chance->oneIn))
    {
        return;
    }
    if (event.pick)
    {
        if (Draw(*event.pick, PickChance))
        {
            PlaySound(model, event, event.sounds[0], play);
        }
        else if (event.sounds.size() > 1)
        {
            PlaySound(model, event, event.sounds[1], play);
        }
        return;
    }
    for (const std::string& sound : event.sounds)
    {
        PlaySound(model, event, sound, play);
    }
}

// Plays the events of the object's model in the situation of its animation,
// or at `moment`.
bool PlayFor(OBJECT& object, std::optional<MonsterSoundMoment> moment)
{
    const MonsterModelDefinition* model = g_MonsterModelDatabase.Find(object.Type);
    if (model == nullptr || model->soundEvents.empty())
    {
        return false;
    }
    Situation situation;
    situation.animation = object.CurrentAction;
    situation.frame = object.AnimationFrame;
    situation.moment = moment;
    situation.deathEffectArmed = object.LifeTime == ArmedDeathEffect;
    situation.subType = object.SubType;
    const std::vector<Data::Maps::MapRef>& maps = model->soundEventMaps;
    situation.heard = !IsFarFromHero(object) && (maps.empty() || Data::Maps::Names(maps, gMapManager.GetServerMap(),
                                                                                   gMapManager.GetServerStage()));
    PlayEvents(*model, situation, [&object](int sound, bool atMonster)
               { PlayBuffer(static_cast<ESound>(sound), atMonster ? &object : nullptr); });
    return true;
}
} // namespace

bool Play(OBJECT& object)
{
    return PlayFor(object, std::nullopt);
}

bool PlayMoment(OBJECT& object, MonsterSoundMoment moment)
{
    return PlayFor(object, moment);
}

void PlayEvents(const MonsterModelDefinition& model, const Situation& situation,
                const std::function<void(int sound, bool atMonster)>& play)
{
    for (const MonsterSoundEvent& event : model.soundEvents)
    {
        if (Applies(event, situation))
        {
            PlayEvent(model, event, play);
        }
    }
}
} // namespace GameLogic::Monsters::SoundEvents
