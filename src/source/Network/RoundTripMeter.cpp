#include "Network/RoundTripMeter.h"

namespace Network
{
void RoundTripMeter::Show(Clock::time_point now)
{
    m_shownSince = now;
    m_shownUntil = now + ShowTime;
    m_nextRequest = now;
    m_outstanding = {};
    m_latest.reset();
}

std::optional<uint32_t> RoundTripMeter::NextRequest(Clock::time_point now)
{
    if (now >= m_shownUntil || now < m_nextRequest)
    {
        return std::nullopt;
    }

    m_nextRequest = now + RequestInterval;
    const uint32_t token = m_nextToken++;
    m_outstanding[m_nextSlot] = {token, now};
    m_nextSlot = (m_nextSlot + 1) % m_outstanding.size();
    return token;
}

void RoundTripMeter::Answered(uint32_t token, Clock::time_point now)
{
    for (Request& request : m_outstanding)
    {
        if (request.token == token && request.token != 0)
        {
            m_latest = now - request.sent;
            request = {};
            return;
        }
    }
}

RoundTripMeter::Reading RoundTripMeter::Read(Clock::time_point now) const
{
    Reading reading;
    reading.shown = now < m_shownUntil;
    if (m_latest)
    {
        reading.milliseconds =
            static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(*m_latest).count());
    }
    else
    {
        reading.unanswered = now - m_shownSince >= AnswerTimeout;
    }
    return reading;
}

RoundTripMeter& Ping()
{
    static RoundTripMeter meter;
    return meter;
}
} // namespace Network
