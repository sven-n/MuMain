#include "stdafx.h"

#include "GameLogic/Monsters/MonsterSoundEvents.h"

#include "Audio/DSPlaySound.h"
#include "Data/DataHandler/MonsterData/MonsterModelLoader.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelDefinition.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "World/MapInfra/MapManager.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <functional>
#include <string>

namespace GameLogic::Monsters::SoundEvents
{
namespace
{
using Data::Monsters::MonsterModelDefinition;
using Data::Monsters::MonsterSoundDraw;
using Data::Monsters::MonsterSoundEvent;

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

void PlaySound(const MonsterModelDefinition& model, const std::string& sound, const std::function<void(int)>& play)
{
    if (const std::optional<int> number = Data::Monsters::ModelLoader::FindSoundNumber(model, sound))
    {
        play(*number);
    }
}

bool Applies(const MonsterSoundEvent& event, int animation, float frame, bool deathEffectArmed)
{
    if (std::find(event.animations.begin(), event.animations.end(), animation) == event.animations.end())
    {
        return false;
    }
    if (event.frames && (frame < event.frames->first || frame >= event.frames->second))
    {
        return false;
    }
    return !event.deathEffectArmed || deathEffectArmed;
}

void PlayEvent(const MonsterModelDefinition& model, const MonsterSoundEvent& event,
               const std::function<void(int)>& play)
{
    if (event.chance && !Draw(event.chance->draw, event.chance->oneIn))
    {
        return;
    }
    if (event.pick)
    {
        PlaySound(model, Draw(*event.pick, PickChance) ? event.sounds[0] : event.sounds[1], play);
        return;
    }
    for (const std::string& sound : event.sounds)
    {
        PlaySound(model, sound, play);
    }
}
} // namespace

bool Play(OBJECT& object)
{
    const MonsterModelDefinition* model = g_MonsterModelDatabase.Find(object.Type);
    if (model == nullptr || model->soundEvents.empty())
    {
        return false;
    }
    if (IsFarFromHero(object))
    {
        return true;
    }
    const std::vector<Data::Maps::MapRef>& maps = model->soundEventMaps;
    if (!maps.empty() && !Data::Maps::Names(maps, gMapManager.GetServerMap(), gMapManager.GetServerStage()))
    {
        return true;
    }
    PlayEvents(*model, object.CurrentAction, object.AnimationFrame, object.LifeTime == ArmedDeathEffect,
               [](int sound) { PlayBuffer(static_cast<ESound>(sound)); });
    return true;
}

void PlayEvents(const MonsterModelDefinition& model, int animation, float frame, bool deathEffectArmed,
                const std::function<void(int)>& play)
{
    for (const MonsterSoundEvent& event : model.soundEvents)
    {
        if (Applies(event, animation, frame, deathEffectArmed))
        {
            PlayEvent(model, event, play);
        }
    }
}
} // namespace GameLogic::Monsters::SoundEvents
