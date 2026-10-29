#include "stdafx.h"

#include "GameLogic/Monsters/MonsterModelChanges.h"

#include "Engine/Object/ZzzCharacter.h"
#include "Render/Models/ZzzBMD.h"

#include <algorithm>

namespace GameLogic::Monsters::ModelChanges
{
namespace
{
// The animation and frame, inside the model's animations.
void Clamp(unsigned short& action, float& frame, const BMD& model)
{
    if (action >= model.NumActions)
    {
        action = 0;
        frame = 0.0f;
    }
    if (model.NumActions <= 0 || frame >= static_cast<float>(model.Actions[action].NumAnimationKeys))
    {
        frame = 0.0f;
    }
}
} // namespace

void ClampAnimation(OBJECT& object, const BMD& model)
{
    Clamp(object.CurrentAction, object.AnimationFrame, model);
    Clamp(object.PriorAction, object.PriorAnimationFrame, model);
}

void ClampAnimations(std::span<const int> slots)
{
    if (CharactersClient == nullptr || Models == nullptr)
    {
        return;
    }
    for (int index = 0; index < MAX_CHARACTERS_CLIENT; ++index)
    {
        OBJECT& object = CharactersClient[index].Object;
        if (object.Live && std::find(slots.begin(), slots.end(), object.Type) != slots.end())
        {
            ClampAnimation(object, Models[object.Type]);
        }
    }
}
} // namespace GameLogic::Monsters::ModelChanges
