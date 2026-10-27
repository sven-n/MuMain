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

void PlaySound(const MonsterModelDefinition& model, const std::string& sound)
{
    if (const std::optional<int> number = Data::Monsters::ModelLoader::FindSoundNumber(model, sound))
    {
        PlayBuffer(static_cast<ESound>(*number));
    }
}

bool Applies(const MonsterSoundEvent& event, const OBJECT& object)
{
    if (std::find(event.animations.begin(), event.animations.end(), object.CurrentAction) == event.animations.end())
    {
        return false;
    }
    if (event.frames && (object.AnimationFrame < event.frames->first || object.AnimationFrame >= event.frames->second))
    {
        return false;
    }
    return !event.deathEffectArmed || object.LifeTime == ArmedDeathEffect;
}

void PlayEvent(const MonsterModelDefinition& model, const MonsterSoundEvent& event)
{
    if (event.chance && !Draw(event.chance->draw, event.chance->oneIn))
    {
        return;
    }
    if (event.pick)
    {
        PlaySound(model, Draw(*event.pick, PickChance) ? event.sounds[0] : event.sounds[1]);
        return;
    }
    for (const std::string& sound : event.sounds)
    {
        PlaySound(model, sound);
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
    const std::vector<int>& worlds = model->soundEventWorlds;
    if (!worlds.empty() && std::find(worlds.begin(), worlds.end(), gMapManager.WorldActive) == worlds.end())
    {
        return true;
    }
    for (const MonsterSoundEvent& event : model->soundEvents)
    {
        if (Applies(event, object))
        {
            PlayEvent(*model, event);
        }
    }
    return true;
}
} // namespace GameLogic::Monsters::SoundEvents
