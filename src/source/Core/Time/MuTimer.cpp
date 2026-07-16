#include "stdafx.h"

#include "MuTimer.h"
#include "Core/Utilities/Log/ErrorReport.h"

#include <algorithm>

namespace mu
{
MuTimer::MuTimer()
{
    Reset();
}

void MuTimer::Reset()
{
    const auto now = Clock::now();
    m_frameStart = now;
    m_sessionStart = now;
    m_lastLogTime = now;
    m_lastFrameMs = 0.0;
    m_minFrameMs = std::numeric_limits<double>::max();
    m_maxFrameMs = 0.0;
    m_frameCount = 0;
    m_hitchCount = 0;
    m_fpsRingBuffer.fill(0.0);
    m_fpsRingIndex = 0;
}

void MuTimer::FrameStart()
{
    m_frameStart = Clock::now();
}

void MuTimer::FrameEnd()
{
    const auto now = Clock::now();
    m_lastFrameMs = std::chrono::duration<double, std::milli>(now - m_frameStart).count();
    m_minFrameMs = std::min(m_minFrameMs, m_lastFrameMs);
    m_maxFrameMs = std::max(m_maxFrameMs, m_lastFrameMs);
    ++m_frameCount;

    if (m_lastFrameMs > k_hitchThresholdMs)
    {
        ++m_hitchCount;
    }

    m_fpsRingBuffer[m_fpsRingIndex] = m_lastFrameMs > 0.0 ? 1000.0 / m_lastFrameMs : 0.0;
    m_fpsRingIndex = (m_fpsRingIndex + 1) % k_fpsRingSize;

    if (std::chrono::duration<double>(now - m_lastLogTime).count() >= k_logIntervalS)
    {
        LogStats();
        m_lastLogTime = now;
        m_minFrameMs = std::numeric_limits<double>::max();
        m_maxFrameMs = 0.0;
    }
}

double MuTimer::GetFrameTimeMs() const
{
    return m_lastFrameMs;
}

double MuTimer::GetFPS() const
{
    double sum = 0.0;
    size_t count = 0;
    for (const double fps : m_fpsRingBuffer)
    {
        if (fps > 0.0)
        {
            sum += fps;
            ++count;
        }
    }
    return count > 0 ? sum / static_cast<double>(count) : 0.0;
}

uint64_t MuTimer::GetHitchCount() const
{
    return m_hitchCount;
}

void MuTimer::LogStats()
{
    const double elapsedSeconds = std::chrono::duration<double>(Clock::now() - m_sessionStart).count();
    const double averageFrameMs = m_frameCount > 0 ? elapsedSeconds * 1000.0 / static_cast<double>(m_frameCount) : 0.0;
    const double minimumFrameMs = m_minFrameMs == std::numeric_limits<double>::max() ? 0.0 : m_minFrameMs;

    g_ErrorReport.Write(
        L"PERF: MuTimer -- elapsed=%.0fs frames=%llu avg=%.1fms min=%.1fms max=%.1fms hitches=%llu fps=%.1f\r\n",
        elapsedSeconds, static_cast<unsigned long long>(m_frameCount), averageFrameMs, minimumFrameMs, m_maxFrameMs,
        static_cast<unsigned long long>(m_hitchCount), GetFPS());
}
}
