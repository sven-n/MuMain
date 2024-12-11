// Timer.h: interface for the CTimer class.
//////////////////////////////////////////////////////////////////////

#pragma once

#include <chrono>

class CTimer
{
public:
    CTimer();
    ~CTimer() = default;

    double GetTimeElapsed(); // Time elapsed since the last reset
    double GetAbsTime();     // Absolute time since timer creation
    void ResetTimer();       // Resets the start time to now

private:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    TimePoint m_startTime;    // Start time for relative measurements
    TimePoint m_absStartTime; // Absolute start time of the timer
};

class CTimer2
{
public:
    CTimer2() : m_startTickCount(0), m_delay(0), m_timeReached(false) {}
    ~CTimer2() = default;

    void SetTimer(unsigned int delay);
    unsigned int GetDelay() const;
    void ResetTimer();
    void UpdateTime();
    bool IsTime() const;

private:
    unsigned int m_startTickCount;
    unsigned int m_delay;
    bool m_timeReached;
};
