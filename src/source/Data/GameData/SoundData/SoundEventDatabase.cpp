#include "stdafx.h"

#include "SoundEventDatabase.h"

#include <algorithm>

namespace Data::Sounds
{
SoundEventDatabase& SoundEventDatabase::GetInstance()
{
    static SoundEventDatabase instance;
    return instance;
}

void SoundEventDatabase::Build(std::span<const SoundEvent> events)
{
    m_events.assign(events.begin(), events.end());
}

const SoundEvent* SoundEventDatabase::Find(std::string_view name) const
{
    const auto found =
        std::find_if(m_events.begin(), m_events.end(), [name](const SoundEvent& event) { return event.event == name; });
    return found != m_events.end() ? &*found : nullptr;
}
} // namespace Data::Sounds
