#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterModelValues.h"

#include "Data/GameData/MonsterData/MonsterAnimations.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"

#include <algorithm>
#include <utility>
#include <vector>

namespace MuEditor::Monsters::ModelValues
{
using Data::Monsters::MonsterModelDefinition;

float GetDefaultSpeed(int slot, int animation)
{
    return Data::Monsters::IsMonsterRangeSlot(slot) ? Data::Monsters::GetMonsterRangeDefaultSpeed(animation)
                                                    : Data::Monsters::NpcRangeDefaultSpeed;
}

bool IsHeldByDefault(int slot, int animation)
{
    return Data::Monsters::IsMonsterRangeSlot(slot) && Data::Monsters::IsHeldByDefaultInMonsterRange(animation);
}

float GetSpeed(const MonsterModelDefinition& model, int slot, int animation)
{
    for (const auto& [number, speed] : model.load.speeds)
    {
        if (number == animation)
        {
            return speed;
        }
    }
    return GetDefaultSpeed(slot, animation);
}

bool HasOwnSpeed(const MonsterModelDefinition& model, int animation)
{
    return std::ranges::any_of(model.load.speeds, [&](const auto& speed) { return speed.first == animation; });
}

bool HoldsLastFrame(const MonsterModelDefinition& model, int slot, int animation)
{
    if (!model.load.holdLastFrame)
    {
        return IsHeldByDefault(slot, animation);
    }
    return std::ranges::find(*model.load.holdLastFrame, animation) != model.load.holdLastFrame->end();
}

void SetSpeed(MonsterModelDefinition& model, int slot, int animation, float speed)
{
    std::erase_if(model.load.speeds, [&](const auto& entry) { return entry.first == animation; });
    if (speed != GetDefaultSpeed(slot, animation))
    {
        model.load.speeds.emplace_back(animation, speed);
        std::ranges::sort(model.load.speeds, {}, &Data::Monsters::MonsterAnimationSpeed::first);
    }
}

void SetHold(MonsterModelDefinition& model, int slot, int count, int animation, bool hold)
{
    std::vector<int> held;
    bool asDefault = true;
    for (int number = 0; number < count; ++number)
    {
        const bool holds = number == animation ? hold : HoldsLastFrame(model, slot, number);
        if (holds)
        {
            held.push_back(number);
        }
        asDefault = asDefault && holds == IsHeldByDefault(slot, number);
    }
    if (asDefault)
    {
        model.load.holdLastFrame.reset();
    }
    else
    {
        model.load.holdLastFrame = std::move(held);
    }
}
} // namespace MuEditor::Monsters::ModelValues

#endif // _EDITOR
