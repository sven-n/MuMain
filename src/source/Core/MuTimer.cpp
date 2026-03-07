// MuTimer.cpp: implementation of mu::MuTimer frame time instrumentation.
// Story 7.2.1 — Flow Code: VS0-QUAL-FRAMETIMER
#include "stdafx.h"

#include "MuTimer.h"
#include "ErrorReport.h"

#include <algorithm>
#include <numeric>

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
    using DurationMs = std::chrono::duration<double, std::milli>;
    using DurationS = std::chrono::duration<double>;

    const auto now = Clock::now();
    m_lastFrameMs = DurationMs(now - m_frameStart).count();
    m_minFrameMs = std::min(m_minFrameMs, m_lastFrameMs);
    m_maxFrameMs = std::max(m_maxFrameMs, m_lastFrameMs);
    ++m_frameCount;

    if (m_lastFrameMs > k_hitchThresholdMs)
    {
        ++m_hitchCount;
    }

    // Update rolling FPS ring buffer with instantaneous FPS for this frame
    m_fpsRingBuffer[m_fpsRingIndex] = (m_lastFrameMs > 0.0) ? 1000.0 / m_lastFrameMs : 0.0;
    m_fpsRingIndex = (m_fpsRingIndex + 1) % k_fpsRingSize;

    // Periodic stats logging — only once every k_logIntervalS seconds (not per frame)
    if (DurationS(now - m_lastLogTime).count() >= k_logIntervalS)
    {
        LogStats();
        m_lastLogTime = now;
        // Reset per-interval min/max so next interval reflects only recent frames
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
    // Average non-zero entries in the ring buffer (zero entries are unfilled slots)
    double sum = 0.0;
    size_t count = 0;
    for (double fps : m_fpsRingBuffer)
    {
        if (fps > 0.0)
        {
            sum += fps;
            ++count;
        }
    }
    return (count > 0) ? sum / static_cast<double>(count) : 0.0;
}

uint64_t MuTimer::GetHitchCount() const
{
    return m_hitchCount;
}

void MuTimer::LogStats()
{
    using DurationS = std::chrono::duration<double>;

    const double sessionElapsedS = DurationS(Clock::now() - m_sessionStart).count();
    const double avgFrameMs = (m_frameCount > 0) ? (sessionElapsedS * 1000.0 / static_cast<double>(m_frameCount)) : 0.0;
    const double minMs = (m_minFrameMs == std::numeric_limits<double>::max()) ? 0.0 : m_minFrameMs;

    // avg/elapsed/frames are session-wide totals; min/max reflect the current interval only
    // (reset per interval in FrameEnd() so each entry captures recent frame variance).
    g_ErrorReport.Write(
        L"PERF: MuTimer -- elapsed=%.0fs frames=%llu avg=%.1fms min=%.1fms max=%.1fms hitches=%llu fps=%.1f\r\n",
        sessionElapsedS, static_cast<unsigned long long>(m_frameCount), avgFrameMs, minMs, m_maxFrameMs,
        static_cast<unsigned long long>(m_hitchCount), GetFPS());
}

} // namespace mu
