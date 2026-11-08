#pragma once

#include "Data/GameData/SoundData/SoundEvents.h"

#include <span>
#include <string_view>
#include <vector>

namespace Data::Sounds
{
// The sound events in memory (Data/Audio/SoundEvents.json, Q-SE1), in the
// file's order. Built at start; the Sound Editor replaces it as it edits.
class SoundEventDatabase
{
public:
    static SoundEventDatabase& GetInstance();

    void Build(std::span<const SoundEvent> events);
    const std::vector<SoundEvent>& GetAll() const
    {
        return m_events;
    }
    // The event of the name, or nullptr.
    const SoundEvent* Find(std::string_view name) const;

private:
    SoundEventDatabase() = default;

    std::vector<SoundEvent> m_events;
};
} // namespace Data::Sounds

#define g_SoundEventDatabase Data::Sounds::SoundEventDatabase::GetInstance()
