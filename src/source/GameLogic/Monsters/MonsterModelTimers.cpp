#include "stdafx.h"

#include "MonsterModelTimers.h"

#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
#include "Engine/Object/w_CharacterInfo.h"

#include <algorithm>
#include <tuple>

namespace GameLogic::Monsters::ModelTimers
{
namespace
{
using Data::Monsters::MonsterModelDefinition;
using Data::Monsters::MonsterModelTimer;

static_assert(std::tuple_size_v<decltype(CHARACTER::ModelTimerLastTicks)> == Data::Monsters::MaxModelTimers);

std::uint32_t (*g_clock)() = nullptr;

std::uint32_t Now()
{
    return g_clock != nullptr ? g_clock() : static_cast<std::uint32_t>(timeGetTime());
}

// The first ask of a frame advances the character's timers.
void Ask(CHARACTER& character)
{
    if (character.ModelTimersAsked)
    {
        return;
    }
    character.ModelTimersAsked = true;
    character.ModelTimerTicks = 0;
    const MonsterModelDefinition* model = g_MonsterModelDatabase.Find(character.Object.Type);
    if (model == nullptr || model->timers.empty())
    {
        return;
    }
    character.ModelTimerTicks =
        Advance(model->timers, character.Object.CurrentAction, Now(), character.ModelTimerLastTicks);
}
} // namespace

std::uint32_t Advance(std::span<const MonsterModelTimer> timers, int animation, std::uint32_t now, LastTicks& lastTicks)
{
    std::uint32_t ticks = 0;
    const size_t count = std::min(timers.size(), lastTicks.size());
    for (size_t index = 0; index < count; ++index)
    {
        const MonsterModelTimer& timer = timers[index];
        // As the draw code counted: unsigned, so a clock that wraps still ticks.
        if (timer.animation == animation && now - lastTicks[index] > static_cast<std::uint32_t>(timer.everyMs))
        {
            ticks |= 1u << index;
            lastTicks[index] = now;
        }
    }
    return ticks;
}

std::vector<std::string> NamesOf(std::span<const MonsterModelTimer> timers, std::uint32_t ticks)
{
    std::vector<std::string> names;
    for (size_t index = 0; index < timers.size() && index < Data::Monsters::MaxModelTimers; ++index)
    {
        if ((ticks & (1u << index)) != 0)
        {
            names.push_back(timers[index].name);
        }
    }
    return names;
}

bool Ticked(CHARACTER& character, std::string_view name)
{
    const std::vector<std::string> names = TickedNames(character);
    return std::find(names.begin(), names.end(), name) != names.end();
}

std::vector<std::string> TickedNames(CHARACTER& character)
{
    Ask(character);
    if (character.ModelTimerTicks == 0)
    {
        return {};
    }
    const MonsterModelDefinition* model = g_MonsterModelDatabase.Find(character.Object.Type);
    return model != nullptr ? NamesOf(model->timers, character.ModelTimerTicks) : std::vector<std::string>();
}

void FinishFrame(CHARACTER& character)
{
    character.ModelTimersAsked = false;
    character.ModelTimerTicks = 0;
}

void SetClock(std::uint32_t (*clock)())
{
    g_clock = clock;
}
} // namespace GameLogic::Monsters::ModelTimers
