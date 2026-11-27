#include "Render/Renderer/PresentedFrameStats.h"

#include <algorithm>

namespace Render
{

void PresentedFrameStats::Begin(double nowMs)
{
    m_beginMs = nowMs;
}

void PresentedFrameStats::Skipped()
{
    ++m_skipped;
}

std::optional<PresentedFrameReport> PresentedFrameStats::Presented(double nowMs)
{
    if (m_previousMs < 0.0)
    {
        m_previousMs = m_rateStartMs = m_reportStartMs = nowMs;
        m_skipped = 0;
        return std::nullopt;
    }

    m_intervalsMs.push_back(nowMs - m_previousMs);
    m_previousMs = nowMs;
    m_workSumMs += nowMs - m_beginMs;

    ++m_rateFrames;
    if (nowMs - m_rateStartMs >= RatePeriodMs)
    {
        m_rate = m_rateFrames * 1000.0 / (nowMs - m_rateStartMs);
        m_rateStartMs = nowMs;
        m_rateFrames = 0;
    }

    const double elapsedMs = nowMs - m_reportStartMs;
    if (elapsedMs < ReportPeriodMs)
    {
        return std::nullopt;
    }

    const std::size_t count = m_intervalsMs.size();
    const auto slow = m_intervalsMs.begin() + static_cast<std::ptrdiff_t>(count * 99 / 100);
    std::nth_element(m_intervalsMs.begin(), slow, m_intervalsMs.end());

    const PresentedFrameReport report{
        elapsedMs / 1000.0,         static_cast<std::uint32_t>(count),  m_skipped,
        count * 1000.0 / elapsedMs, *slow > 0.0 ? 1000.0 / *slow : 0.0, m_workSumMs / count};
    m_intervalsMs.clear();
    m_workSumMs = 0.0;
    m_skipped = 0;
    m_reportStartMs = nowMs;
    return report;
}

PresentedFrameStats& FrameStatsOfRenderer()
{
    static PresentedFrameStats stats;
    return stats;
}

} // namespace Render
