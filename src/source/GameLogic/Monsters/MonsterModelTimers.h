#pragma once

#include "Data/GameData/MonsterData/MonsterModelDefinition.h"

#include <array>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

class CHARACTER;

// The timers of the monsters' models (monsters design, Q-MN91): a model's
// map code and its sound events tick on the same timer, so that Twintail's
// hair swings and its walk sound stay in step, and the interval is data
// (Data/Monsters/Models.json "timers").
namespace GameLogic::Monsters::ModelTimers
{
// The last tick of each timer of a model, in milliseconds.
using LastTicks = std::array<std::uint32_t, Data::Monsters::MaxModelTimers>;

// Advances the timers at `now` for a character in `animation`: a timer of the
// animation whose last tick is more than its interval ago ticks, and `now`
// becomes its last tick. Returns the timers that tick: bit i for timer i.
std::uint32_t Advance(std::span<const Data::Monsters::MonsterModelTimer> timers, int animation, std::uint32_t now,
                      LastTicks& lastTicks);

// The names of the timers of `ticks` (Advance's bits), in the model's order.
std::vector<std::string> NamesOf(std::span<const Data::Monsters::MonsterModelTimer> timers, std::uint32_t ticks);

// Whether the timer `name` of the character's model ticks in this drawn
// frame. The first ask of a frame, by the map's code or the sound events,
// advances the model's timers; later asks of the frame get the same answer
// until FinishFrame.
bool Ticked(CHARACTER& character, std::string_view name);

// The names of the character's model timers that tick in this drawn frame.
std::vector<std::string> TickedNames(CHARACTER& character);

// After the frame (SoundEvents::FinishDrawnFrame, from
// UpdateDrawnCharacterEffects): the next ask is a new frame.
void FinishFrame(CHARACTER& character);

// The clock of the timers in milliseconds: timeGetTime. Tests set their own;
// nullptr goes back to timeGetTime.
void SetClock(std::uint32_t (*clock)());
} // namespace GameLogic::Monsters::ModelTimers
