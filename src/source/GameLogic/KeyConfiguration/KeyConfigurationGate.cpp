#include "GameLogic/KeyConfiguration/KeyConfigurationGate.h"

namespace GameLogic::KeyConfiguration
{
void KeyConfigurationGate::Reset()
{
    *this = {};
}

void KeyConfigurationGate::Joined(Clock::time_point now)
{
    m_joined = now;
}

void KeyConfigurationGate::Received(bool everythingRestored)
{
    m_received = true;
    m_pending = m_pending && !everythingRestored;
}

bool KeyConfigurationGate::RequestSave(Clock::time_point now)
{
    if (IsKnown(now))
    {
        return true;
    }
    m_pending = true;
    return false;
}

bool KeyConfigurationGate::MaySaveNow(Clock::time_point now) const
{
    return IsKnown(now);
}

bool KeyConfigurationGate::TakeDueSave(Clock::time_point now)
{
    if (!m_pending || !IsKnown(now))
    {
        return false;
    }
    m_pending = false;
    return true;
}

bool KeyConfigurationGate::IsKnown(Clock::time_point now) const
{
    return m_received || (m_joined.has_value() && now - *m_joined >= Wait);
}
} // namespace GameLogic::KeyConfiguration
