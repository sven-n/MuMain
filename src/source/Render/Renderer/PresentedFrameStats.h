#pragma once

// Frames the renderer really submitted to the swapchain. The game loop's rate is not that: where frames may not wait,
// a loop turn without a free swapchain texture draws nothing.

#include <cstdint>
#include <optional>
#include <vector>

namespace Render
{

struct PresentedFrameReport
{
    double seconds;
    std::uint32_t presented;
    std::uint32_t skipped;
    double fps;
    // The rate of the slowest 1 % of the intervals between presented frames.
    double lowFps;
    // Mean main-thread time from the frame's begin to its submission.
    double workMs;
};

class PresentedFrameStats
{
public:
    void Begin(double nowMs);
    void Skipped();
    // Returns a report once every reportPeriodMs of presented frames.
    std::optional<PresentedFrameReport> Presented(double nowMs);

    // Frames presented per second over the last full second; 0 before the first one.
    [[nodiscard]] double Rate() const
    {
        return m_rate;
    }

    static constexpr double RatePeriodMs = 1000.0;
    static constexpr double ReportPeriodMs = 5000.0;

private:
    double m_beginMs = 0.0;
    double m_previousMs = -1.0;
    double m_rate = 0.0;
    double m_rateStartMs = 0.0;
    std::uint32_t m_rateFrames = 0;
    double m_reportStartMs = 0.0;
    double m_workSumMs = 0.0;
    std::uint32_t m_skipped = 0;
    std::vector<double> m_intervalsMs;
};

// The renderer's.
PresentedFrameStats& FrameStatsOfRenderer();

} // namespace Render
