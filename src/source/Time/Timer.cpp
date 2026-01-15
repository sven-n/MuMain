// Timer.cpp: implementation of the CTimer class.
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "Timer.h"
#include <chrono>

CTimer::CTimer()
{
    m_startTime = Clock::now();
    m_absStartTime = m_startTime;
}

double CTimer::GetTimeElapsed()
{
    auto now = Clock::now();
    auto elapsed = std::chrono::duration<double, std::milli>(now - m_startTime);
    return elapsed.count(); // Return elapsed time in milliseconds
}

double CTimer::GetAbsTime()
{
    auto now = Clock::now();
    auto elapsed = std::chrono::duration<double, std::milli>(now - m_absStartTime);
    return elapsed.count(); // Return absolute time in milliseconds
}

void CTimer::ResetTimer()
{
    m_startTime = Clock::now(); // Reset start time to now
}

void CTimer2::SetTimer(unsigned int delay)
{
    m_delay = delay;
    m_startTickCount = 0;
}

unsigned int CTimer2::GetDelay() const
{
    return m_delay;
}

void CTimer2::ResetTimer()
{
    m_startTickCount = 0;
}

void CTimer2::UpdateTime()
{
    using SteadyClock = std::chrono::steady_clock;
    using TimePoint = std::chrono::time_point<SteadyClock>;

    static TimePoint startTickTime;

    if (m_delay == 0)
    {
        m_timeReached = true;
    }
    else
    {
        m_timeReached = false;
        auto now = SteadyClock::now();

        if (m_startTickCount == 0)
        {
            startTickTime = now;
            m_startTickCount = 1; // Mark initialization
            return;
        }

        if (now - startTickTime > std::chrono::milliseconds(m_delay))
        {
            startTickTime = now;
            m_timeReached = true;
        }
    }
}

bool CTimer2::IsTime() const
{
    return m_timeReached;
}
