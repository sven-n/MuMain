// MuTimer.h: Frame time instrumentation for performance diagnostics.
// Story 7.2.1 — Flow Code: VS0-QUAL-FRAMETIMER
//
// Provides per-frame timing, rolling FPS average, hitch detection (>50ms),
// and periodic logging to g_ErrorReport every 60 seconds.
// Uses std::chrono::steady_clock exclusively — no Win32 timing APIs.
#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <limits>

namespace mu
{

class MuTimer
{
public:
    MuTimer();

    void FrameStart();
    void FrameEnd();

    [[nodiscard]] double GetFrameTimeMs() const;
    [[nodiscard]] double GetFPS() const;
    [[nodiscard]] uint64_t GetHitchCount() const;
    void Reset();

private:
    using Clock = std::chrono::steady_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    static constexpr double k_hitchThresholdMs = 50.0;
    static constexpr double k_logIntervalS = 60.0;
    static constexpr size_t k_fpsRingSize = 60;

    TimePoint m_frameStart;
    TimePoint m_sessionStart;
    TimePoint m_lastLogTime;

    double m_lastFrameMs;
    double m_minFrameMs;
    double m_maxFrameMs;

    uint64_t m_frameCount;
    uint64_t m_hitchCount;

    std::array<double, k_fpsRingSize> m_fpsRingBuffer;
    size_t m_fpsRingIndex;

    void LogStats();
};

} // namespace mu
